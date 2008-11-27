#ifdef  ALT_INICHE

#include "ins_eth_ocm.h"

/**
 * Function for obtaining MAC address. Located externally in application code.
 *
 * @param net  NET interface (Interniche typedef)
 * @param mac_addr Character array into which MAC address should be written.
 *
 * @return zero on success, nonzero otherwise.
 */ 
extern int get_mac_addr(NET net, unsigned char mac_addr[6]);

/**
 * Performs PHY initialization and determines link duplex.
 * This is fully vendor specific depending on the PHY you are using.
 *
 * @param  dev Pointer to eth_ocm_dev struct which contains needed base address
 * @return 1 if Link is established in Full duplex.
 *         0 if Link is established in Half duplex.
 */
extern int eth_ocm_phy_init(eth_ocm_dev *dev);

static void eth_ocm_isr(void *context, alt_u32 id);
static int  eth_ocm_read_init(eth_ocm_dev *dev);
#ifndef ETH_OCM_SYNC_TX
static int  eth_ocm_pkt_send(PACKET pkt);
static int  eth_ocm_low_send(NET net, char *data, unsigned data_bytes);
static void eth_ocm_tx_isr(eth_ocm_dev *dev);
#else
static int  eth_ocm_raw_send(NET net, char *data, unsigned data_bytes);
#endif // else ifndef ETH_OCM_SYNC_TX
static int  eth_ocm_rx_isr(eth_ocm_dev *dev);

/**
 * Prepare ethernet MAC.
 *
 * @param ins_dev   Pointer to associated alt_iniche_dev struct
 * @return
 */
error_t eth_ocm_prep(alt_iniche_dev *ins_dev){
    NET ifp;
    int index;
    eth_ocm_dev *dev;

    index = ins_dev->if_num;
    dev = (eth_ocm_dev *)ins_dev;

    #if (ETH_OCM_DBG_LVL >= 1)
    //Status message
    dprintf("eth_ocm_prep\n");
    #endif // if ETH_OCM_DBG_LVL

    //create eth_ocm_info struct
    dev->info = (eth_ocm_info *)malloc(sizeof(eth_ocm_info));
    dev->info->sem = 0; //initialize semaphore
    dev->info->rx_pkts = (PACKET *)(malloc(sizeof(PACKET) * ETH_OCM_RX_DESC_COUNT));


    ifp = nets[index];
    ifp->n_mib->ifAdminStatus = ETH_OCM_STATUS_DOWN;    
    ifp->n_mib->ifOperStatus =  ETH_OCM_STATUS_DOWN;
    ifp->n_mib->ifLastChange =  cticks * (100/TPS);     //timestamp
    ifp->n_mib->ifPhysAddress = (u_char*)dev->info->mac_addr;
    ifp->n_mib->ifDescr =       (u_char*)"Opencores 10/100 ethernet MAC";
    ifp->n_lnh =                ETHHDR_SIZE;            /* ethernet header size. */
    ifp->n_hal =                ETH_OCM_MAC_ADDR_LEN;   /* MAC address length */
    ifp->n_mib->ifType =        ETHERNET;               /* device type */
    ifp->n_mtu =                ETH_OCM_MAX_MTU;        /* max frame size */

    /* install our hardware driver routines */
    ifp->n_init =       eth_ocm_init;
    #ifndef ETH_OCM_SYNC_TX
    ifp->pkt_send =     eth_ocm_pkt_send;
    ifp->raw_send =     NULL;
    #else
    ifp->pkt_send =     NULL;
    ifp->raw_send =     eth_ocm_raw_send;
    #endif // ifndef ETH_OCM_SYNC_TX
    ifp->n_close =      eth_ocm_close;
    ifp->n_stats =      eth_ocm_stats;

#ifdef IP_V6
      ifp->n_flags |= (NF_NBPROT | NF_IPV6);
#else
      ifp->n_flags |= NF_NBPROT;
#endif

    /* set cross-pointers between iface and eth_ocm structs */
    dev->info->netp = ifp;
    ifp->n_local = (void*)(dev);

    /* get the MAC address. */
    get_mac_addr(ifp, dev->info->mac_addr);

    index++;
    return index;
}
//End of function eth_ocm_prep

/**
 * Initializes the Opencores ethernet MAC. Called by InterNiche stack
 *
 * @param 
 */
int eth_ocm_init(int iface){
    int status = SUCCESS;
    int duplex;
    int temp;
    NET ifp;
    eth_ocm_dev *dev;
    eth_ocm_info *info;
    eth_ocm_regs *regs;

    #if (ETH_OCM_DBG_LVL >= 1)
    dprintf("[eth_ocm_init]\n");
    #endif
    //get the ifp first
    ifp = nets[iface];
    //now get the info pointer
    dev = (eth_ocm_dev *)ifp->n_local;
    info = dev->info;
    regs = dev->regs;

    //Reset Descriptors (supposedly this can be done while in reset)
    for(temp=ETH_OCM_DESC_START;temp<ETH_OCM_DESC_END;temp++)
        IOWR(dev->base, temp, 0);

    //Let's disable the MAC until everything else is set up
    regs->moder = 0;

    //Determine the number of RX descriptors
    regs->tx_bd_num = ETH_OCM_TX_DESC_COUNT; //Set TX descriptor count in MAC    
    info->cur_tx_desc = 0;
    info->cur_rx_desc = 0;
    #ifndef ETH_OCM_SYNC_TX
    info->next_tx_desc = 0;
    info->next_tx_desc_rdy = 1;
    //Initialize queues
    dev->info->tosend.q_head = dev->info->tosend.q_tail = NULL;
    dev->info->tosend.q_max = dev->info->tosend.q_min = dev->info->tosend.q_len = 0;
    dev->info->sending.q_head = dev->info->sending.q_tail = NULL;
    dev->info->sending.q_max = dev->info->sending.q_min = dev->info->sending.q_len = 0;
    #endif

    /* perform any necessary PHY setup */
    //Let's set the MDIO interface up to run at 4MHz.
    temp = (ALT_CPU_FREQ / 1000000);
    temp += 2;
    temp &= 0xFFFFFFFE;   //only even numbers allowed)
    regs->miimoder = temp;
    regs->miicommand = 0;
    //Find out if we should run in duplex or not
    duplex = eth_ocm_phy_init(dev);

    if(duplex)
        duplex = ETH_OCM_MODER_FULLD_MSK;

    // Configure MAC options
    // Interrupt sources
    regs->int_mask = ETH_OCM_DEFAULT_INTERRUPT_MASK; //Interrupt on receive
    // Clear any existing interrupts
    regs->int_source = 0xFFFFFFFF;

    // Inter-packet gap 
    if(duplex)
        regs->ipgt = ETH_OCM_FULL_DUPLEX_IPGT; 
    else
        regs->ipgt = ETH_OCM_HALF_DUPLEX_IPGT;

    //Let's set the defaults just because they've bitten us before
    regs->ipgr2     = 0x0000000C;
    regs->ipgr2     = 0x00000012;
    regs->packetlen = 0x00400600;  //Min and Max frame sizes
    regs->collconf  = 0x000F003F;
    regs->ctrlmoder = 0x00000000;

    #if (ETH_OCM_DBG_LVL >= 1)
    dprintf("[eth_ocm_init] Configuring MAC address "
            "%02x:%02x:%02x:%02x:%02x:%02x\n",
            info->mac_addr[0],info->mac_addr[1],info->mac_addr[2],
            info->mac_addr[3],info->mac_addr[4],info->mac_addr[5]);
    #endif // if ETH_OCM_DBG_LVL
    //Configure the MAC address
    regs->mac_addr0 =
            ( ((int)info->mac_addr[5])         |
             (((int)info->mac_addr[4]) << 8)   |
             (((int)info->mac_addr[3]) << 16)  |
             (((int)info->mac_addr[2]) << 24)  );

    regs->mac_addr1 =
            ( ((int)((unsigned char)info->mac_addr[1]))         |
             (((int)((unsigned char)info->mac_addr[0])) << 8)   );


    //Enable MAC
    regs->moder = (
           ETH_OCM_MODER_PAD_MSK    |   //Enable padding of small packets
           ETH_OCM_MODER_CRCEN_MSK  |   //Append CRC to TX packets
           ETH_OCM_MODER_RXEN_MSK   |   //Enable receive
           ETH_OCM_MODER_TXEN_MSK   |   //Enable transmit
           duplex                       //Discovered duplex
           );

    #if (ETH_OCM_DBG_LVL >= 1)
    dprintf("\nOpencores MAC post-init: MODER = 0x%08x\n", (int)regs->moder);
    #endif // if ETH_OCM_DBG

   /* status = UP */
   nets[iface]->n_mib->ifAdminStatus = ETH_OCM_STATUS_UP;
   nets[iface]->n_mib->ifOperStatus = ETH_OCM_STATUS_UP;

   //register ISR interrupt handler
   temp = alt_irq_register(dev->irq, dev, eth_ocm_isr);
   if(temp)
       dprintf("[eth_ocm_init] Failed to register RX ISR\n");
   //Setup the first read transfer
   eth_ocm_read_init(dev);

   return status;   //MAC is ready to rock and roll
}
//End of eth_ocm_init function


#ifndef ETH_OCM_SYNC_TX
int eth_ocm_pkt_send(PACKET pkt){
    eth_ocm_dev *dev;
    eth_ocm_info *info;
    eth_ocm_regs *regs;
    int result;
#ifdef UCOS_II
    int cpu_sr;
#endif

    dev = (eth_ocm_dev *)pkt->net->n_local;
    info = dev->info;
    regs = dev->regs;
    result = SUCCESS;

    OS_ENTER_CRITICAL();    //disable interrupts

    putq(&info->tosend, (qp)pkt);
    eth_ocm_tx_isr(dev);
    /*
    //If there is an available descriptor, and it's not busy
    if(info->next_tx_desc_rdy && (info->sending.q_len < ETH_OCM_TX_DESC_COUNT)){//!(regs->txdescs[info->next_tx_desc].ctrl & ETH_OCM_TXDESC_READY_MSK)){
        result = eth_ocm_low_send(pkt->net, pkt->nb_prot, pkt->nb_plen);
        //If setup failed, free the packet and move on.
        if(result != SUCCESS){
            pkt->net->n_mib->ifOutDiscards++; //increment TX discard counter
            pk_free(pkt);
            return result;
        }
        else{ //Transfer was successfully setup
            info->next_tx_desc++;
            if(info->next_tx_desc == ETH_OCM_TX_DESC_COUNT)
                info->next_tx_desc = 0;
            //See if all descriptor are in use
            if(info->next_tx_desc == info->cur_tx_desc)
                info->next_tx_desc_rdy = 0;
            //Put the packet in the sending queue
            putq(&info->sending, (qp)pkt);
        }
    }
    else{ //Unable to send packet right now so queue it
        putq(&info->tosend, (qp)pkt);
    }
    */

    OS_EXIT_CRITICAL();     //reenable interrupts
    return SUCCESS;
}

/** This is the asynchronous raw send function. It sets up a transfer
 *  but does not wait for it to conclude. It sets the MAC to interrupt
 *  when the transfer has finished. This is not threadsafe.
 *
 *  @param NET
 *  @param data
 *  @param data_bytes
 *
 *  @return 0 if Successful, negative otherwise
 */
int eth_ocm_low_send(NET net, char *data, unsigned data_bytes){
    int result;
    unsigned len;
    eth_ocm_dev *dev;
    eth_ocm_info *info;
    eth_ocm_regs *regs;
    alt_u8 *buf;

    dev = (eth_ocm_dev *)net->n_local;
    info = dev->info;
    regs = dev->regs;
    len = data_bytes - ETHHDR_BIAS;
    result = SUCCESS;

    //Check to see if someone is nesting send calls (BAD!)
    if(info->sem){
       dprintf("[eth_ocm_low_send] ERROR: Nested low send call\n");
       return ENP_RESOURCE;
    }
    //Grab the semaphore
    info->sem = 1;
    // clear bit-31 before passing it to SGDMA Driver
    buf = (alt_u8 *)alt_remap_cached( (volatile void *)data, 4);
    //advance the pointer beyond the header bias
    buf = (alt_u8 *)((unsigned int)buf + ETHHDR_BIAS);

    //Some error checks first
    if(len < ETH_OCM_MIN_MTU)
        result = -1;        //packet too small
    if(len > ETH_OCM_MAX_MTU)
        result = -EFBIG;    //packet too big
    if(regs->txdescs[info->next_tx_desc].ctrl & ETH_OCM_TXDESC_READY_MSK)
        result = -EBUSY;    //DMA not available

    if(result == SUCCESS){
        //Write pointer to descriptor
        regs->txdescs[info->next_tx_desc].ptr = (unsigned int)buf;
        //Write length and setup transfer
        result = ((len << ETH_OCM_TXDESC_LEN_OFST)  |
                 ETH_OCM_TXDESC_READY_MSK           |
                 ETH_OCM_TXDESC_IRQ_MSK             |
                 ETH_OCM_TXDESC_PAD_MSK             |
                 ETH_OCM_TXDESC_CRC_MSK);
        //See if wrap flag should be set
        if(info->next_tx_desc == (ETH_OCM_TX_DESC_COUNT - 1))
            result |= ETH_OCM_TXDESC_WRAP_MSK;
        //Write descriptor
        regs->txdescs[info->next_tx_desc].ctrl = result;
        result = SUCCESS;
    }

    info->sem = 0;
    return result;
}

void    eth_ocm_tx_isr(eth_ocm_dev *dev){
    eth_ocm_info *info;
    eth_ocm_regs *regs;
    int result;
    PACKET pkt;

    info = dev->info;
    regs = dev->regs;

    //First we need to process all finished descriptors
    while(  info->sending.q_len>0 
            && ((info->cur_tx_desc != info->next_tx_desc) || !info->next_tx_desc_rdy)
            && !(regs->txdescs[info->cur_tx_desc].ctrl & ETH_OCM_TXDESC_READY_MSK)){ 
    
        //Get the packet
        pkt = (PACKET)getq(&info->sending);

        //Get transmit result from descriptor
        result = regs->txdescs[info->cur_tx_desc].ctrl;

        //Check for errors
        if(result & 
            (ETH_OCM_TXDESC_UR_MSK      |
            ETH_OCM_TXDESC_RL_MSK       |
            ETH_OCM_TXDESC_LC_MSK       |
            ETH_OCM_TXDESC_CS_MSK)){
            #if (ETH_OCM_DBG_LVL >= 2)
            dprintf("[eth_ocm_tx_isr] Transmit error 0x%x\n", result);
            #endif // if ETH_OCM_DBG_LVL
            pkt->net->n_mib->ifOutDiscards++; //increment TX discard counter
            } 
        else{
            #if (ETH_OCM_DBG_LVL >= 5)
            if(result & ETH_OCM_TXDESC_RTRY_MSK)
                dprintf("[eth_ocm_tx_isr] Transmit retries: %d\n", (result & ETH_OCM_TXDESC_RTRY_MSK)>>ETH_OCM_TXDESC_RTRY_OFST);
            #endif
            pkt->net->n_mib->ifOutOctets += pkt->nb_plen;
            pkt->net->n_mib->ifOutUcastPkts++;
            result = 0;
        }

        //free the packet
        pk_free(pkt);

        //Increment the current descriptor pointer
        info->cur_tx_desc++;
        if(info->cur_tx_desc == ETH_OCM_TX_DESC_COUNT)
            info->cur_tx_desc = 0;
        //Whatever the next descriptor is it's ready now
        info->next_tx_desc_rdy = 1;
    }

    //Now we can send any queued packets
    while( info->next_tx_desc_rdy
            && info->tosend.q_len>0
            && !(regs->txdescs[info->next_tx_desc].ctrl & ETH_OCM_TXDESC_READY_MSK) ){

        //Get the packet to be send
        pkt = (PACKET)getq(&info->tosend);

        result = eth_ocm_low_send(pkt->net, pkt->nb_prot, pkt->nb_plen);
        //If setup failed, free the packet and move on.
        if(result != SUCCESS){
            pkt->net->n_mib->ifOutDiscards++;
            pk_free(pkt);
        }
        else{ //Transfer was successfully setup
            info->next_tx_desc++;
            if(info->next_tx_desc == ETH_OCM_TX_DESC_COUNT)
                info->next_tx_desc = 0;
            //See if all descriptor are in use
            if(info->next_tx_desc == info->cur_tx_desc)
                info->next_tx_desc_rdy = 0;
            //Put the packet in the sending queue
            putq(&info->sending, (qp)pkt);
        }
    }
}

#endif // ifndef ETH_OCM_SYNC_TX


#ifdef ETH_OCM_SYNC_TX
/**
 *  Raw send function to initiate a transfer to the mac 
 *
 * @param  net  - NET structure associated with the Opencores MAC instance
 * @param  data - pointer to the data payload
 *
 * @return SUCCESS if success, else a negative value
 */
int eth_ocm_raw_send(NET net, char * data, unsigned data_bytes){
    int result;
    int i;
    unsigned len;
    eth_ocm_dev *dev;
    eth_ocm_info *info;
    eth_ocm_regs *regs;
    alt_u8 *buf;
#ifdef UCOS_II
    int cpu_sr;
#endif

    OS_ENTER_CRITICAL(); //disable interrupts

    dev = (eth_ocm_dev *)net->n_local;
    info = dev->info;
    regs = dev->regs;
    len = data_bytes - ETHHDR_BIAS;
    result = 0;

    //Check to see if someone is nesting send calls (BAD!)
    if(info->sem){
       dprintf("[eth_ocm_raw_send] ERROR: Nested raw send call\n");
       OS_EXIT_CRITICAL();
       return ENP_RESOURCE;
    }

    //Grab the semaphore
    info->sem = 1;
    // clear bit-31 before passing it to SGDMA Driver
    buf = (alt_u8 *)alt_remap_cached( (volatile void *)data, 4);
    //advance the pointer beyond the header bias
    buf = (alt_u8 *)((unsigned int)buf + ETHHDR_BIAS);

    //Some error checks first
    if(len < ETH_OCM_MIN_MTU)
        result = -1;        //packet too small
    if(len > ETH_OCM_MAX_MTU)
        result = -EFBIG;    //packet too big
    if(regs->txdescs[0].ctrl & ETH_OCM_TXDESC_READY_MSK)
        result = -EBUSY;    //DMA not available

    if(!result){
        //Write pointer to descriptor
        regs->txdescs[0].ptr = (unsigned int)buf;
        //Write length and setup transfer
        regs->txdescs[0].ctrl =
                ((len << ETH_OCM_TXDESC_LEN_OFST)    |
                 ETH_OCM_TXDESC_READY_MSK            |
                 ETH_OCM_TXDESC_WRAP_MSK             |
                 ETH_OCM_TXDESC_PAD_MSK              |
                 ETH_OCM_TXDESC_CRC_MSK);
        //Wait for transfer to complete
        i=0;
        do{
            result = regs->txdescs[0].ctrl;
            i++;
        }while((result & ETH_OCM_TXDESC_READY_MSK) && i<ETH_OCM_TRANSMIT_TIMEOUT);
        //Make sure no timeout occurred
        if(i<ETH_OCM_TRANSMIT_TIMEOUT){
            if(result &
                    (ETH_OCM_TXDESC_UR_MSK      |
                    ETH_OCM_TXDESC_RL_MSK       |
                    ETH_OCM_TXDESC_LC_MSK       |
                    ETH_OCM_TXDESC_CS_MSK)){
                #if (ETH_OCM_DBG_LVL >= 2)
                dprintf("[eth_ocm_raw_send] Transmit error 0x%x\n", result);
                #endif // if ETH_OCM_DBG_LVL
                result = -EIO;  //Some error occured
            } 
            else{
                #if (ETH_OCM_DBG_LVL >= 5)
                if(result & ETH_OCM_TXDESC_RTRY_MSK)
                    dprintf("[eth_ocm_raw_send] Transmit retries: %d\n", (result & ETH_OCM_TXDESC_RTRY_MSK)>>ETH_OCM_TXDESC_RTRY_OFST);
                #endif
                result = 0;
            }
        } 
        else{   //Timeout
            result = -ETIMEDOUT;
        }
    }   //End of if(!result) Transmit branch

    //Check final result
    if(!result){    //Successfull transfer
       net->n_mib->ifOutOctets += data_bytes;   //Increment TX data counter
       // we dont know whether it was unicast or not, we count both in <ifOutUcastPkts>
       net->n_mib->ifOutUcastPkts++;
       result = SUCCESS;
    }
    else{   //Failed transfer
        #if (ETH_OCM_DBG_LVL >= 2)
        dprintf("[eth_ocm_raw_send] Transmit failed, "
                "ret=%u, len=%d\n",
                result,
                len);
        #endif // if ETH_OCM_DBG_LVL
       net->n_mib->ifOutDiscards++; //increment TX discard counter
       result = SEND_DROPPED;   // ENP_RESOURCE and SEND_DROPPED have the same value! 
    }
    
    info->sem = 0;              //release semaphore
    OS_EXIT_CRITICAL();         //reenable interrupts
    return result;              //success 
}
//End of function eth_ocm_raw_send
#endif // ifdef ETH_OCM_SYNC_TX


/**
 * Receive ISR (interrupt service routine)
 *
 * @param  context  - context of the Opencores MAC instance
 * @param  id       - IRQ number 
 */
void eth_ocm_isr(void *context, alt_u32 id){
    eth_ocm_dev *dev;
    eth_ocm_regs *regs;
    int result;

    dev = (eth_ocm_dev *)context;
    regs = dev->regs;

    //Read the interrupt source
    result = regs->int_source;
    while(result){
        //Clear interrupt flags immediately. Only clear the ones that
        //have been set. We do this in case another one has occured since
        //we read it.
        regs->int_source = result;  //clear interrupts

        //Check for receive flags
        if(result & (ETH_OCM_INT_MASK_RXB_MSK | ETH_OCM_INT_MASK_RXE_MSK)){
            //Call the receive function. This will set up a new transfer
            eth_ocm_rx_isr(dev);
            //Check to see if there is something in the stack's received queue
            if ((rcvdq.q_len) > 0){
                SignalPktDemux(); 
            }
        }
       
        //Check for busy flag
        if(result & ETH_OCM_INT_MASK_BUSY_MSK){
        #if (ETH_OCM_DBG_LVL >= 3)        
            dprintf("Frame dropped: too busy to receive\n");
        #endif
            dev->info->netp->n_mib->ifInDiscards++;
        }

        #ifndef ETH_OCM_SYNC_TX
        //Check for transmit flags
        if(result & (ETH_OCM_INT_MASK_TXE_MSK | ETH_OCM_INT_MASK_TXB_MSK)){
            eth_ocm_tx_isr(dev);
        }
        #endif //ifndef ETH_OCM_SYNC_TX


        //See if any interrupts have been set
        result = regs->int_source;
    }
}


/**
 *  Set up the first receive transfer
 */
static int eth_ocm_read_init(eth_ocm_dev *dev){
    eth_ocm_info *info;
    eth_ocm_regs *regs;
    alt_u8 *buf_ptr;
    PACKET *pkts;
    alt_u32 temp;
    int i;

    info = dev->info;
    regs = dev->regs;
    pkts = info->rx_pkts;


    //allocate a packet for every descriptor
    for(i=0;i<ETH_OCM_RX_DESC_COUNT;i++){
        pkts[i] = pk_alloc(ETH_OCM_BUF_ALLOC_SIZE);
        if (!pkts[i]){  // couldn't get a free buffer for rx 
            dprintf("[eth_ocm_read_init] Fatal error: Unable to allocte ETH_OCM_RX_DESC_COUNT buffers\n");
            return ENP_NOBUFFER;
        }

        // ensure bit-31 of pkt_array[i]->nb_buff is clear before passing
        buf_ptr = (alt_u8*)alt_remap_cached ((volatile void*) pkts[i]->nb_buff, 4);
        //shift the actual write location over by ETHHDR_BIAS (see ipport.h)
        buf_ptr = (alt_u8*)(((unsigned int)buf_ptr) + ETHHDR_BIAS);

        if(!(regs->rxdescs[i].ctrl & ETH_OCM_RXDESC_EMPTY_MSK)){
            //Write pointer
            regs->rxdescs[i].ptr = (alt_u32)buf_ptr;
            //Write the control register to start the transfer
            temp = ETH_OCM_RXDESC_EMPTY_MSK | ETH_OCM_RXDESC_IRQ_MSK;
            if(i == (ETH_OCM_RX_DESC_COUNT - 1))
                temp |= ETH_OCM_RXDESC_WRAP_MSK;
            regs->rxdescs[i].ctrl = temp; 
        }
        else{
            dprintf("[eth_ocm_read_init] Fatal error: RX descriptor unavailable.\n");
            dprintf("[eth_ocm_read_init] Descriptor %u = 0x%08x\n", i, (int)regs->rxdescs[i].ctrl);
            return ENP_RESOURCE;
        }
    }

    return SUCCESS;
}
//End of function eth_ocm_read_init


/**
 * Receive operation. Checks the status of the received frame.
 * Attempt to obtain a new buffer from the InterNiche stack.
 * Schedules another RX transfer
 *
 * @return SUCCESS on success
 */
static int eth_ocm_rx_isr(eth_ocm_dev *dev)
{
    eth_ocm_info *info;
    eth_ocm_regs *regs;
    struct ethhdr *eth;
    int pklen;
    alt_u32 stat;
    alt_u8 cur;
    PACKET pkt;
    PACKET *pkts;
    alt_u8 *buf_ptr;

    info = dev->info;
    regs = dev->regs;
    pkts = info->rx_pkts;
    cur = info->cur_rx_desc;

    stat = regs->rxdescs[cur].ctrl;

    //We'll process as many descriptors as are ready
    while(!(stat & ETH_OCM_RXDESC_EMPTY_MSK)){
        pklen = stat & ETH_OCM_RXDESC_LEN_MSK;
        pklen = pklen >> ETH_OCM_RXDESC_LEN_OFST;

        //Increment received byte count
        info->netp->n_mib->ifInOctets += (u_long)pklen;

        pkts[cur]->nb_prot = pkts[cur]->nb_buff + ETHHDR_SIZE;
        pkts[cur]->nb_plen = pklen - (14 + 4); //Packet length minus (header + CRC)
        pkts[cur]->nb_tstamp = cticks;
        pkts[cur]->net = info->netp;

        // set packet type for demux routine
        eth = (struct ethhdr *)(pkts[cur]->nb_buff + ETHHDR_BIAS);
        pkts[cur]->type = eth->e_type;

        if (!(stat & ETH_OCM_RXDESC_ERROR_MSK)){
            pkt = pk_alloc(ETH_OCM_BUF_ALLOC_SIZE);
            if (!pkt){  // couldn't get a free buffer for rx 
              #if (ETH_OCM_DBG_LVL >= 4)
              dprintf("[eth_ocm_rx_isr] No free RX buffers (Swamping the NicheStack)\n");
              #endif // if ETH_OCM_DBG_LVL
              info->netp->n_mib->ifInDiscards++;
            }
            else{
              putq(&rcvdq, pkts[cur]);
              pkts[cur] = pkt;
            }
        }
        else{
            #if (ETH_OCM_DBG_LVL >= 3)
            dprintf("[eth_ocm_rx_isr] Frame discarded due to errors: 0x%08x!\n", (unsigned)stat);
            #endif // if ETH_OCM_DBG_LVL
            info->netp->n_mib->ifInDiscards++;
        }

        // ensure bit-31 of pkt_array[]->nb_buff is clear before passing
        // to DMA Driver
        buf_ptr = (alt_u8*)alt_remap_cached ((volatile void*) pkts[cur]->nb_buff, 4);
        //shift the actual write location over by ETHHDR_BIAS (see ipport.h)
        buf_ptr = (alt_u8*)((unsigned int)buf_ptr + ETHHDR_BIAS);

        //Write pointer
        regs->rxdescs[cur].ptr = (unsigned int)buf_ptr; 
        //Write the control register to start the transfer
        stat = ETH_OCM_RXDESC_EMPTY_MSK | ETH_OCM_RXDESC_IRQ_MSK;
        if(cur == (ETH_OCM_RX_DESC_COUNT - 1))
            stat |= ETH_OCM_RXDESC_WRAP_MSK;
        regs->rxdescs[cur].ctrl = stat;
        
        //increment current descriptor counter
        cur++;
        if(cur == ETH_OCM_RX_DESC_COUNT)
            cur = 0;
        //get new descriptors status
        stat = regs->rxdescs[cur].ctrl;
    }
    info->cur_rx_desc = cur;        

    return SUCCESS;
}

void eth_ocm_stats(void *pio, int iface) {
    NET ifp;
    eth_ocm_dev *dev;
    eth_ocm_regs *regs;
    int i;

    //get the ifp first
    ifp = nets[iface];
    dev = (eth_ocm_dev *)ifp->n_local;
    regs = dev->regs;

    #ifndef ETH_OCM_SYNC_TX
    ns_printf(pio, "ToSend queue: max:%d, current:%d\n", 
            dev->info->tosend.q_max,
            dev->info->tosend.q_len);

    ns_printf(pio, "Sendng queue: max:%d, current:%d\n", 
            dev->info->sending.q_max,
            dev->info->sending.q_len);
    #endif //ifndef ETH_OCM_SYNC_TX

    ns_printf(pio, "TX Descriptor status:\n");
    for(i=0;i<ETH_OCM_TX_DESC_COUNT;i++){
        ns_printf(pio,"     %3d: 0x%08x\n", i, regs->txdescs[i].ctrl);
    }

    ns_printf(pio, "RX Descriptor status:\n");
    for(i=0;i<ETH_OCM_RX_DESC_COUNT;i++){
        ns_printf(pio,"     %3d: 0x%08x\n", i, regs->rxdescs[i].ctrl);
    }


}

/**
 * Closes the opencores mac interface
 *
 * @param  iface    index of the NET interface associated with the Opencores MAC.
 * @return SUCCESS
 */
int eth_ocm_close(int iface)
{
  int err;
  NET ifp;
  eth_ocm_dev *dev;

  /* status = down */
  ifp = nets[iface];
  dev = (eth_ocm_dev *)ifp->n_local;

  ifp->n_mib->ifAdminStatus = ETH_OCM_STATUS_DOWN;

  /* disable the interrupt in the OS*/
  err = alt_irq_register (dev->irq, 0, NULL);
  if (err){
    dprintf("[eth_ocm_close] Could not unregister interrupt, error = %d\n",err);
    return err;
  }

  // Shut down the MAC
  IOWR_ETH_OCM_MODER(dev->base, 0);

  /* status = down */
  ifp->n_mib->ifOperStatus = ETH_OCM_STATUS_DOWN;
  //deallocate memory for the eth_ocm_info struct allocated in eth_ocm_prep
  free(dev->info->rx_pkts);
  free(dev->info);

  return SUCCESS;
}

#endif // ifdef ALT_INICHE
//End of file ins_eth_ocm.h
