//==========================================================================
//
//      dev/if_oeth.c
//
//      Ethernet device driver for Opencores ethermac
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2003 Nick Garnett
// Copyright (C) 2004 Andrew Lunn
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    Gaisler Research, (Konrad Eisele<eiselekd@web.de>)
// Contributors:
// Date:         2005-01-22
// Purpose:
// Description:
// Notes:
//####DESCRIPTIONEND####
//
//==========================================================================


#include <pkgconf/system.h>
#ifdef CYGPKG_IO_ETH_DRIVERS
#include <pkgconf/io_eth_drivers.h>
#endif
#include <pkgconf/devs_eth_opencores_ethermac.h>

#include <cyg/infra/cyg_type.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/hal/hal_diag.h>
#include <cyg/hal/hal_io.h>
#include <cyg/infra/cyg_ass.h>
#include <cyg/infra/diag.h>
#include <cyg/hal/drv_api.h>
#include <cyg/io/eth/netdev.h>
#include <cyg/io/eth/eth_drv.h>
#include <cyg/hal/hal_cache.h>
#include <stdio.h>

#ifdef CYGPKG_NET
#include <pkgconf/net.h>
#include <cyg/kernel/kapi.h>
#include <net/if.h>  /* Needed for struct ifnet */
#endif

#ifdef CYGPKG_KERNEL
#include <cyg/kernel/kapi.h>
#endif


#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#if CYGNUM_DEVS_ETH_OPENCORES_ETHERMAC_DEV_COUNT > 1
#error Only 1 ethermac at a time supported yet
#endif


#define FIAD_PHY_ADDRESS 31 // for Zylin PHI board.

//#define DEBUG_OPENETH

#ifdef DEBUG_OPENETH
#define DEBUG_RX_PACKETS 1
#define DEBUG_TX_PACKETS 1
#define os_printf diag_printf
#define db_printf diag_printf
#else
#define DEBUG_RX_PACKETS 0
#define DEBUG_TX_PACKETS 0
#define os_printf(fmt,...)
#define db_printf(fmt,...)
#endif

#define MACADDR0 macaddr[0]
#define MACADDR1 macaddr[1]
#define MACADDR2 macaddr[2]
#define MACADDR3 macaddr[3]
#define MACADDR4 macaddr[4]
#define MACADDR5 macaddr[5]

/* The transmitter timeout */
#define TX_TIMEOUT	(2*HZ)

#define ETH_ALEN	6		/* Octets in one ethernet addr	 */
#define ETH_HLEN	14		/* Total octets in header.	 */
#define ETH_ZLEN	60		/* Min. octets in frame sans FCS */

#include <oeth_info.h>
#include CYGDAT_DEVS_ETH_OPENCORES_ETHERMAC_INL

cyg_uint8 macaddr[6] = CYGPKG_DEVS_ETH_OPENCORES_ETHERMAC_ETH0_ESA;
extern volatile cyg_uint8 *macstart;


//#define OETH_REGLOAD(a,v) HAL_READ_UINT32(&(a),v); diag_printf("load %08x=%08x\n", &(a), v);
//#define OETH_REGSAVE(a,v) HAL_WRITE_UINT32(&(a),v); diag_printf("store %08x,%08x\n", &(a), v);
#define OETH_REGLOAD(a,v) HAL_READ_UINT32(&(a),v);
#define OETH_REGSAVE(a,v) HAL_WRITE_UINT32(&(a),v);

#define OETH_REGORIN(a,v)  \
    { cyg_uint32 va;       \
      OETH_REGLOAD(a,va);  \
      va |= v;             \
      OETH_REGSAVE(a,va);  \
    }
#define OETH_REGANDIN(a,v) \
    { cyg_uint32 va;       \
      OETH_REGLOAD(a,va);  \
      va &= v;             \
      OETH_REGSAVE(a,va);  \
    }

static void openeth_start( struct eth_drv_sc *sc, unsigned char *enaddr, int flags );
static void openeth_stop( struct eth_drv_sc *sc );
static cyg_uint32 eth_isr(cyg_vector_t vector, cyg_addrword_t data);
static void eth_dsr(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data);
static void oeth_tx(struct eth_drv_sc *sc);
static void openeth_rxready(struct eth_drv_sc *sc);
static void oeth_txdone(struct eth_drv_sc *sc);




/* Danger!!!! we need to align this to cache line sizes, otherwise
 * a flush/invalidate of one buffer could bleed over into another!
 */
#define ROUND_UP_BUF_SIZE(a) (((a+HAL_DCACHE_LINE_SIZE-1)/HAL_DCACHE_LINE_SIZE)*HAL_DCACHE_LINE_SIZE)
static cyg_uint8 rxbuff[OETH_RXBD_NUM][ROUND_UP_BUF_SIZE(OETH_RX_BUFF_SIZE)]
 __attribute__((aligned(HAL_DCACHE_LINE_SIZE)));
static cyg_uint8 txbuff[OETH_TXBD_NUM][ROUND_UP_BUF_SIZE(OETH_TX_BUFF_SIZE)]
__attribute__((aligned(HAL_DCACHE_LINE_SIZE)));

extern bool CYGPKG_DEVS_ETH_OPENCORES_ETHERMAC_INITFN(struct cyg_netdevtab_entry *ndp);

bool openeth_init(struct cyg_netdevtab_entry *ndp) {
  return CYGPKG_DEVS_ETH_OPENCORES_ETHERMAC_INITFN(ndp);
}

#if DEBUG_RX_PACKETS || DEBUG_TX_PACKETS
static void oeth_print_packet(unsigned long add, int len)
{
  int i;
  diag_printf("ipacket: add = %x len = %d\n", (unsigned int)add, len);
  for(i = 0; i < len; i++) {
    if(!(i % 16))
      diag_printf("\n");
    diag_printf(" %.2x", *(((unsigned char *)add) + i));
  }
  diag_printf("\n");
  diag_printf("\n");
}
#endif

static void openeth_start( struct eth_drv_sc *sc, unsigned char *enaddr, int flags ) {

  oeth_info *oi = (oeth_info *)sc->driver_private;
  oeth_regs *regs = oi->cep.regs;
  struct oeth_private *cep = (struct oeth_private *)&(oi->cep);
  volatile oeth_bd *bdp;
  int i;

  if ( oi->active )
    openeth_stop( sc );

  db_printf("openeth_start: irq %d\n",cep ->irq);

  /* Install our interrupt handler. */
  cyg_drv_interrupt_create(cep ->irq,
                           0,                  // Priority - unused
                           (CYG_ADDRWORD)sc,   // Data item passed to ISR & DSR
                           eth_isr,            // ISR
                           eth_dsr,            // DSR
                           &oi->interrupt_handle, // handle to intr obj
                           &oi->interrupt_object ); // space for int obj
  db_printf("openeth_start2: irq %d done\n",cep ->irq);
  cyg_drv_interrupt_attach(oi->interrupt_handle);
  db_printf("openeth_start3: irq %d done\n",cep ->irq);
  cyg_drv_interrupt_acknowledge(cep->irq);
  db_printf("openeth_start4: irq %d done\n",cep ->irq);
  cyg_drv_interrupt_unmask(cep ->irq);

  db_printf("openeth_start: irq %d done\n",cep ->irq);

  // Enable device
  oi->active = 1;

  /* Initialize transmit pointers. */
  cep->rx_cur = 0;
  cep->tx_next = 0;
  cep->tx_last = 0;
  cep->tx_full = 0;

  bdp = cep->rx_bd_base;
  for (i = 0; i < OETH_RXBD_NUM; i++) {
    OETH_REGORIN(bdp->len_status , OETH_RX_BD_EMPTY); //bdp->len_status |= OETH_RX_BD_EMPTY;
    bdp++;
  }

  /* Enable receiver and transmiter  */
  OETH_REGORIN(regs->moder , OETH_MODER_RXEN | ((OETH_TXBD_NUM>1)?OETH_MODER_TXEN:0));  //regs->moder |= OETH_MODER_RXEN | OETH_MODER_TXEN;
}


static void openeth_stop( struct eth_drv_sc *sc ) {

  oeth_info *oi = (oeth_info *)sc->driver_private;
  oeth_regs *regs = oi->cep.regs;
  struct oeth_private *cep = (struct oeth_private *)&(oi->cep);
  volatile oeth_bd *bdp;
  int i;

  db_printf("openeth_stop\n");

  /* Free interrupt hadler  */
  cyg_interrupt_delete(oi->interrupt_handle);

  /* Disable receiver and transmitesr */
  OETH_REGANDIN(regs->moder , ~(OETH_MODER_RXEN | OETH_MODER_TXEN)); //regs->moder &= ~(OETH_MODER_RXEN | OETH_MODER_TXEN);

  bdp = cep->rx_bd_base;
  for (i = 0; i < OETH_RXBD_NUM; i++) {
    OETH_REGANDIN(bdp->len_status , ~(OETH_RX_BD_STATS | OETH_RX_BD_EMPTY));  //bdp->len_status &= ~(OETH_RX_BD_STATS | OETH_RX_BD_EMPTY);
    bdp++;
  }

  bdp = cep->tx_bd_base;
  for (i = 0; i < OETH_TXBD_NUM; i++) {
    OETH_REGANDIN(bdp->len_status, ~(OETH_TX_BD_STATS | OETH_TX_BD_READY));  //bdp->len_status &= ~(OETH_TX_BD_STATS | OETH_TX_BD_READY);
    bdp++;
  }

  memset(cep->tx_keys,0,sizeof(cep->tx_keys));

  oi->active = 0;               // stop people tormenting it

}

static cyg_uint32 eth_isr(cyg_vector_t vector, cyg_addrword_t data) {
  struct eth_drv_sc *sc = (struct eth_drv_sc *)data;
  oeth_info *oi = (oeth_info *)sc->driver_private;
  struct oeth_private *cep = (struct oeth_private *)&(oi->cep);
  oeth_regs *regs = oi->cep.regs;
  cyg_uint32 int_events, calldsr = 0;
  //cyg_uint32 cpu = sparc_leon3_get_cpuid();

  /* Get the interrupt events that caused us to be here. */
  OETH_REGLOAD(regs->int_src, int_events); //int_events = cep->regs->int_src;
  OETH_REGSAVE(regs->int_src, int_events); //cep->regs->int_src = int_events;

  /*
	diag_printf("isr %x(cpu:%d,irq:%d)\n",int_events,cpu,cep->irq);
    diag_printf("mask(0):0x%x\n",LEON3_IrqCtrl_Regs->mask[0]);
    diag_printf("mask(1):0x%x\n",LEON3_IrqCtrl_Regs->mask[1]);
    diag_printf("ipend:0x%x\n",LEON3_IrqCtrl_Regs->ipend);
    diag_printf("force:0x%x\n",LEON3_IrqCtrl_Regs->iforce);
  */

  /* Handle receive event . */
  if (int_events & (OETH_INT_RXF | OETH_INT_RXE)) {
    calldsr = CYG_ISR_CALL_DSR;
  }

  /* Handle transmit event in its own function. */
  if (int_events & (OETH_INT_TXB | OETH_INT_TXE)) {
    calldsr = CYG_ISR_CALL_DSR;
  }

  /* Check for receive busy, i.e. packets coming but no place to put them.  */
  if (int_events & OETH_INT_BUSY) {
    if (!(int_events & (OETH_INT_RXF | OETH_INT_RXE))) {
      db_printf("openeth: RX buffer dumped!.\n"); /* All transmit buffers are full.  Bail out.*/
    }
    calldsr = CYG_ISR_CALL_DSR;
  }

  cyg_drv_interrupt_acknowledge(cep ->irq);
  if (calldsr) {
    //cyg_drv_interrupt_mask(cep ->irq);
  }

  if (!calldsr) {
    //diag_printf("dsr not called \n");
  }

  return (CYG_ISR_HANDLED|calldsr);        // schedule DSR
}

static void eth_dsr(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data) {
  struct eth_drv_sc *sc = (struct eth_drv_sc *)data;
  //diag_printf("eth_dsr\n");
  eth_drv_dsr( vector, count, (cyg_addrword_t)sc );
}

static void openeth_deliver(struct eth_drv_sc *sc) {
  //oeth_info *oi = (oeth_info *)sc->driver_private;
  //struct oeth_private *cep = (struct oeth_private *)&(oi->cep);
  //cyg_drv_dsr_lock();
  //cyg_drv_isr_lock();
  openeth_rxready(sc);
  oeth_txdone(sc);
  //cyg_drv_isr_unlock();
  //cyg_drv_dsr_unlock();
  //cyg_drv_interrupt_unmask(cep->irq);

}

static void oeth_tx(struct eth_drv_sc *sc)
{
  oeth_info *oi = (oeth_info *)sc->driver_private;
  struct oeth_private *cep = (struct oeth_private *)&(oi->cep);
  volatile oeth_bd *bdp;
  cyg_uint32 status;

  //db_printf ("oeth_tx()\n");

  for (;; cep->tx_last = (cep->tx_last + 1) % OETH_TXBD_NUM) {

    bdp = cep->tx_bd_base + cep->tx_last;
    OETH_REGLOAD(bdp->len_status,status);

    if ((status & OETH_TX_BD_READY) || ((cep->tx_last == cep->tx_next) && !cep->tx_full))
      break;

    /* Check status for errors */
    if (status & OETH_TX_BD_LATECOL)
      cep->stats.tx_window_errors++;
    if (status & OETH_TX_BD_RETLIM)
      cep->stats.tx_aborted_errors++;
    if (status & OETH_TX_BD_UNDERRUN)
      cep->stats.tx_fifo_errors++;
    if (status & OETH_TX_BD_CARRIER)
      cep->stats.tx_carrier_errors++;
    if (status & (OETH_TX_BD_LATECOL | OETH_TX_BD_RETLIM | OETH_TX_BD_UNDERRUN))
      cep->stats.tx_errors++;

    cep->stats.tx_packets++;
    cep->stats.collisions += (status >> 4) & 0x000f;

    if (cep->tx_full)
      cep->tx_full = 0;
  }
}

//pass tx-keys up the stack when packets are transmitted, called from can_send() and deliver()
static void oeth_txdone(struct eth_drv_sc *sc) {
  oeth_info *oi = (oeth_info *)sc->driver_private;
  struct oeth_private *cep = (struct oeth_private *)&(oi->cep);
  volatile oeth_bd *bdp;
  cyg_uint32 status, i;

  oeth_tx(sc);

  //db_printf ("oeth_txdone()\n");
  for (i = 0; i < OETH_TXBD_NUM; i++) {
    bdp = cep->tx_bd_base + i;
    OETH_REGLOAD(bdp->len_status,status);
    if (cep->tx_keys[i] && !(status & OETH_TX_BD_READY)) {
      unsigned long k = cep->tx_keys[i];
      cep->tx_keys[i] = 0;
//      diag_printf ("%i: key 0x%x, ready: %d\n",k ,status & OETH_TX_BD_READY ? 1 : 0);
      db_printf("tx=%d\n",i);
      (sc->funs->eth_drv->tx_done)( sc, k, 0 );
    }
  }
}

#ifdef ETH_DRV_GET_MAC_ADDRESS
static int eth_get_mac_address(struct eth_drv_sc *sc, char *addr)
{
  oeth_info *oi = (oeth_info *)sc->driver_private;
  oeth_regs *regs = oi->cep.regs;
  cyg_uint32 m0,m1;
  db_printf("eth_get_mac_address\n");
  OETH_REGLOAD(regs->mac_addr1 ,m1);
  OETH_REGLOAD(regs->mac_addr0 ,m0);
  addr[0] = (m1 >> 8) & 0xff;
  addr[1] = (m1 >> 0) & 0xff;
  addr[2] = (m0 >>24) & 0xff;
  addr[3] = (m0 >>16) & 0xff;
  addr[4] = (m0 >> 8) & 0xff;
  addr[5] = (m0 >> 0) & 0xff;
  return 0;
}
#endif

#ifdef ETH_DRV_SET_MAC_ADDRESS
static int eth_set_mac_address(struct eth_drv_sc *sc, cyg_uint8 *addr, int eeprom)
{
  oeth_info *oi = (oeth_info *)sc->driver_private;
  oeth_regs *regs = oi->cep.regs;
  db_printf("eth_set_mac_address\n");
  oi->dev_addr[0] = addr[0];
  oi->dev_addr[1] = addr[1];
  oi->dev_addr[2] = addr[2];
  oi->dev_addr[3] = addr[3];
  oi->dev_addr[4] = addr[4];
  oi->dev_addr[5] = addr[5];
  OETH_REGSAVE(regs->mac_addr1 ,
               addr[0] << 8 	|
               addr[1]);
  OETH_REGSAVE(regs->mac_addr0 ,
               addr[2] << 24 	|
               addr[3] << 16 	|
               addr[4] << 8 	|
               addr[5]);
  return 1;
}
#endif

static int calc_crc(unsigned char *mac_addr)
{
	int result = 0;
	return (result & 0x3f);
}

static void oeth_set_multicast_list(struct eth_drv_sc *sc, struct eth_drv_mc_list *mcl)
{


  oeth_info *oi = (oeth_info *)sc->driver_private;
  /* Get pointer of controller registers. */
  volatile oeth_regs *regs  = oi->cep.regs;
  int	i;

  //db_printf("oeth_set_multicast_list: promisc: %d mc_all: %d (mc_list: %d)\n",oi->promisc,oi->multicast_all,mcl->len);


  if (oi->promisc) {
    /* Log any net taps.  */
    db_printf("Promiscuous mode enabled.\n");
    OETH_REGORIN(regs->moder , OETH_MODER_PRO); //regs->moder |= OETH_MODER_PRO;
  } else {
    OETH_REGANDIN(regs->moder , ~OETH_MODER_PRO); //regs->moder &= ~OETH_MODER_PRO;
    if (oi->multicast_all) {
      /* Catch all multicast addresses, so set the filter to all 1's. */
      OETH_REGSAVE(regs->hash_addr0 , 0xffffffff); //regs->hash_addr0 = 0xffffffff
      OETH_REGSAVE(regs->hash_addr1 , 0xffffffff); //regs->hash_addr0 = 0xffffffff
    }
    else if (mcl && mcl->len) {

      //db_printf("oeth_set_multicast_list: multicastlist \n");
      OETH_REGORIN(regs->moder , OETH_MODER_IAM); //regs->moder |= OETH_MODER_IAM;

      /* Clear filter and add the addresses in the list. */
      OETH_REGSAVE(regs->hash_addr0 , 0x00000000); //regs->hash_addr0 = 0x00000000
      OETH_REGSAVE(regs->hash_addr0 , 0x00000000); //regs->hash_addr0 = 0x00000000

      for (i = 0; i < mcl->len; i++) {

        int hash_b;

        /* Only support group multicast for now. */
        if (!(mcl->addrs[i][0] & 1))
          continue;

        //db_printf("%x %x %x %x %x %x\n",mcl->addrs[i][0],mcl->addrs[i][1],mcl->addrs[i][2],
        //mcl->addrs[i][3],mcl->addrs[i][4],mcl->addrs[i][5]
        //);

        hash_b = calc_crc(mcl->addrs[i]);
        if(hash_b >= 32) {
          OETH_REGORIN(regs->hash_addr1 , 1 << (hash_b - 32)); //regs->hash_addr1 |= 1 << (hash_b - 32);
        } else {
          OETH_REGORIN(regs->hash_addr0 , 1 << hash_b); //regs->hash_addr0 |= 1 << hash_b;
        }
      }
    }
  }
}

static int openeth_ioctl(struct eth_drv_sc *sc, unsigned long key, void *data, int data_length) {
  oeth_info *oi = (oeth_info *)sc->driver_private;

  //db_printf("openeth_ioctl\n");
  //db_printf( "openeth_ioctl: device eth%d at %x; key is 0x%x, data at %x[%d]\n",
  //oi->idx, oi, key, data, data_length );

  switch ( key ) {
#ifdef ETH_DRV_SET_MC_LIST
  case ETH_DRV_SET_MC_LIST:    {
    struct eth_drv_mc_list *mcl = (struct eth_drv_mc_list *)data;
    oi->multicast_all = 0;
    oeth_set_multicast_list(sc, mcl);
    return 0;
  }
#endif // ETH_DRV_SET_MC_LIST
#ifdef ETH_DRV_SET_MC_ALL
  case ETH_DRV_SET_MC_ALL:
    oi->multicast_all = 1;
    oeth_set_multicast_list(sc, 0);
    return 0;
#endif // ETH_DRV_SET_MC_ALL

#ifdef ETH_DRV_SET_MAC_ADDRESS
  case ETH_DRV_SET_MAC_ADDRESS:
    if ( 6 != data_length )
      return -2;
    return eth_set_mac_address( sc, data, 1 /* do write eeprom */ );
#endif
#ifdef ETH_DRV_GET_MAC_ADDRESS
  case ETH_DRV_GET_MAC_ADDRESS:
    return eth_get_mac_address( sc, data );
#endif
  default:
    break;
  }
  return -1;
}


static int openeth_can_send(struct eth_drv_sc *sc) {
  oeth_info *oi = (oeth_info *)sc->driver_private;
  struct oeth_private *cep = (struct oeth_private *)&(oi->cep);
  oeth_txdone(sc);
  //db_printf("openeth: openeth_can_send: %d\n",!cep->tx_full);



  return !cep->tx_full;
}

static void openeth_send(struct eth_drv_sc *sc,
                         struct eth_drv_sg *sg_list, int sg_len, int total_len,
                         unsigned long key) {

  oeth_info *oi = (oeth_info *)sc->driver_private;
  struct oeth_private *cep = (struct oeth_private *)&(oi->cep);
  volatile oeth_bd *bdp;
  cyg_uint32 status, addr;
  int len, i, ec = 0;
  cyg_uint8 *to_addr;

  /* Fill in a Tx ring entry */
  bdp = cep->tx_bd_base + cep->tx_next;
  cep->tx_keys[cep->tx_next] = key;

  OETH_REGLOAD(bdp->len_status,status);

  if (OETH_TX_BD_READY & status) {
    diag_printf("openeth: tx counter mismatch!.\n");
    return;
  }
  if (cep->tx_full) {
    diag_printf("openeth: tx queue full!.\n"); /* All transmit buffers are full.  Bail out.*/
    return;
  }

  /* Clear all of the status flags. */
  OETH_REGANDIN(bdp->len_status , ~OETH_TX_BD_STATS); //bdp->len_status &= ~OETH_TX_BD_STATS;

  /* If the frame is short, tell CPM to pad it. */
  if (total_len <= ETH_ZLEN) {
    OETH_REGORIN(bdp->len_status , OETH_TX_BD_PAD); //bdp->len_status |= OETH_TX_BD_PAD;
  } else {
    OETH_REGANDIN(bdp->len_status , ~OETH_TX_BD_PAD);  //bdp->len_status &= ~OETH_TX_BD_PAD;
  }

  /* Copy data in preallocated buffer */
  if (total_len > OETH_TX_BUFF_SIZE) {
    diag_printf("openeth: tx frame too long!.\n");
    return ;
  }

  OETH_REGLOAD(bdp->addr, addr);
  to_addr = (cyg_uint8 *)(addr);
  for (i = 0;  i < sg_len;  i++) {
    len = sg_list[i].len;
    memcpy(to_addr, (void*)sg_list[i].buf, len);

    to_addr += len; ec += len;
  }

  if (ec != total_len) {
    diag_printf("openeth: packet length wrong!.\n");
  }

#if DEBUG_TX_PACKETS
  db_printf("TX\n");
  oeth_print_packet((unsigned long)addr, total_len);
#endif

  OETH_REGLOAD(bdp->len_status,status);
  OETH_REGSAVE(bdp->len_status , (status & 0x0000ffff) | (total_len << 16));   //bdp->len_status = (bdp->len_status & 0x0000ffff) | (skb->len << 16);

  cep->tx_next = (cep->tx_next + 1);
  if (cep->tx_next >= OETH_TXBD_NUM)
  {
	  cep->tx_next = 0;
  }

  if (cep->tx_next == cep->tx_last)
    cep->tx_full = 1;

#ifdef CYGPKG_DEVS_ETH_OPENCORES_ETHERMAC_FLUSH
	/* the DMA will read memory directly..
	 *
	 * We can invalidate here since we *know* that the bdp->addr is on
 	 * a cache line boundry and that the length of the rx/tx slot is aligned up
 	 * to the nearest cache line size.
 	 */
  HAL_DCACHE_FLUSH(bdp->addr, total_len );
#endif

  /* KLUDGE!!!! there is something wrong with buffer descriptors,
   * start/stop receive wif we are using only one descriptor */
  if (OETH_TXBD_NUM==1)
  {
	  OETH_REGANDIN(cep->regs->moder , ~OETH_MODER_TXEN);
  }

  /* Send it on its way.  Tell controller its ready, interrupt when done, and to put the CRC on the end.  */
  OETH_REGORIN(bdp->len_status , (OETH_TX_BD_READY | OETH_TX_BD_IRQ | OETH_TX_BD_CRC)); //bdp->len_status |= (OETH_TX_BD_READY | OETH_TX_BD_IRQ | OETH_TX_BD_CRC);

  if (OETH_TXBD_NUM==1)
  {
	  OETH_REGORIN(cep->regs->moder , OETH_MODER_TXEN);
  }
  return;
}

/* Why does the ethermac return 4 too much????? */
int packetLength(int status)
{
	return (status >> 16);
}

static void openeth_recv( struct eth_drv_sc *sc, struct eth_drv_sg *sg_list, int sg_len ) {
  oeth_info *oi = (oeth_info *)sc->driver_private;
  struct oeth_private *cep = (struct oeth_private *)&(oi->cep);
  volatile oeth_bd *bdp;
  cyg_uint8 *from_addr;
  cyg_uint32 from, status, i;
  int	pkt_len, total_len;

//  diag_printf ("-recv(%d)",cep->rx_cur);

  bdp = cep->rx_bd_base + cep->rx_cur;
  OETH_REGLOAD(bdp->len_status,status);
  OETH_REGLOAD(bdp->addr,from);
  from_addr = (cyg_uint8 *) (from);

  /* Process the incoming frame.
   *
   * KLUDGE!!!! why does the ethermac core report 8 bytes too many???
   *
   */
  total_len = pkt_len = packetLength(status);

  for ( i = 0; i < sg_len; i++ ) {
    cyg_uint8 *to_addr;
    int len;
    to_addr = (cyg_uint8 *)(sg_list[i].buf);
    len = sg_list[i].len;

    if (to_addr == 0 || len <= 0)
      return; // out of mbufs

    if ( len > pkt_len )
      len = pkt_len;

    //diag_printf("RX %x <- %x (%x)\n",to_addr,from_addr,len);
    memcpy( to_addr, (void *)from_addr, len );
    from_addr += len;
    pkt_len -= len;

    /*    {

      int i;
      for(i = 0; i < len; i++) {
        if(!(i % 16))
          diag_printf("\n");
        diag_printf(" %.2x", *(((unsigned char *)to_addr) + i));
      }
      diag_printf("\n");
      }*/


  }

  if (pkt_len > 0) {
    diag_printf("oeth: sglist: not all data received\n");
  }


#if DEBUG_RX_PACKETS
//  diag_printf("RX %d\n", total_len);
  oeth_print_packet((unsigned long)from, total_len);
#endif

  cep->stats.rx_packets++;

}

static void openeth_rxready(struct eth_drv_sc *sc) {
  oeth_info *oi = (oeth_info *)sc->driver_private;
  struct oeth_private *cep = (struct oeth_private *)&(oi->cep);
  volatile oeth_bd *bdp;
  cyg_uint32 status;

  int	pkt_len;
  int	bad = 0,i = 0;

  //db_printf ("openeth_rxready\n");

  /* First, grab all of the stats for the incoming packet.
     These get messed up if we get called due to a busy condition. */
  for (;;) {
    bdp = cep->rx_bd_base + cep->rx_cur;
#if 0
    /* check if any descriptors have the empty bit cleared */
    for (i=0; i<OETH_RXBD_NUM; i++)
    {
        OETH_REGLOAD(cep->rx_bd_base[i].len_status,status);
        if ((status & OETH_RX_BD_EMPTY)==0)
        {
        	break;
        }
    }
#endif

    OETH_REGLOAD(bdp->len_status,status);
    if (status & OETH_RX_BD_EMPTY) {
    	/* If we loose track of the hw rx_cur pointer, then our best bet is to wait
    	 * for it to cycle around, so do nothing here...
    	 *
    	 * Normally we keep track of the rx_cur hw pointer, and quitting here is
    	 * the right thing to do.
    	 */
#if 0
    	if (i!=OETH_RXBD_NUM)
    	{
    		diag_printf("RX buffer out of sync");
    	}
#endif
        break;
    }

    /* Check status for errors. */
    if (status & (OETH_RX_BD_TOOLONG | OETH_RX_BD_SHORT)) { //if (bdp->len_status & (OETH_RX_BD_TOOLONG | OETH_RX_BD_SHORT)) {
      diag_printf ("openeth: length error\n");
      cep->stats.rx_length_errors++;
      bad = 1;
    }
    if (status & OETH_RX_BD_DRIBBLE) { //if (bdp->len_status & OETH_RX_BD_DRIBBLE) {
      diag_printf ("openeth: dribble error\n");
      cep->stats.rx_frame_errors++;
      bad = 1;
    }
    if (status & OETH_RX_BD_CRCERR) { //if (bdp->len_status & OETH_RX_BD_CRCERR) {
      diag_printf ("openeth: crc error\n");
      cep->stats.rx_crc_errors++;
      bad = 1;
    }
    if (status & OETH_RX_BD_OVERRUN) { //if (bdp->len_status & OETH_RX_BD_OVERRUN) {
      diag_printf ("openeth: overrun error. Length %d\n", packetLength(status));
      cep->stats.rx_crc_errors++;
      bad = 1;
    }
    if (status & OETH_RX_BD_LATECOL) { //if (bdp->len_status & OETH_RX_BD_LATECOL) {
      diag_printf ("openeth: latecol error\n");
      cep->stats.rx_frame_errors++;
      bad = 1;
    }

	/* Process the incoming frame.     */
	pkt_len = packetLength(status);

	/* discard manually. Why???? The hw should do this for us!!!  */
	if (! bad && (pkt_len < 64))
	{
	    diag_printf ("openeth: manual discard error\n");
	    cep->stats.rx_crc_errors++;
		bad=1;
	}

	if (status & OETH_RX_BD_MISS)
	{
		diag_printf ("openeth: miss error\n");
	    cep->stats.rx_crc_errors++;
	    bad = 1;
	}

    if (!bad)
	{

		//call up stack
		//diag_printf("rx packet %d\n",pkt_len);

		//diag_printf("rx=%d",cep->rx_cur);

		(sc->funs->eth_drv->recv)( sc, pkt_len );
		//diag_printf("\n");
    }

    cep->rx_cur = (cep->rx_cur + 1);
    if (cep->rx_cur >= OETH_RXBD_NUM)
    {
    	cep->rx_cur = 0;
    }

#ifdef CYGPKG_DEVS_ETH_OPENCORES_ETHERMAC_FLUSH
   	/* We can invalidate here since we *know* that the bdp->addr is on
   	 * a cache line boundry and that the length of the rx/tx slot is aligned up
   	 * to the nearest cache line size.
   	 */
    HAL_DCACHE_INVALIDATE(bdp->addr, pkt_len );
#endif

    OETH_REGSAVE(bdp->len_status , OETH_RX_BD_EMPTY | OETH_RX_BD_IRQ | ((cep->rx_cur==0)?OETH_RX_BD_WRAP:0)); //bdp->len_status &= ~OETH_RX_BD_STATS;
  }
}


static void openeth_poll(struct eth_drv_sc *sc) {
  oeth_info *oi = (oeth_info *)sc->driver_private;
  struct oeth_private *cep = (struct oeth_private *)&(oi->cep);
  eth_isr(cep->irq,(cyg_addrword_t)sc);
  openeth_deliver(sc);
}

static int openeth_int_vector(struct eth_drv_sc *sc) {
  oeth_info *oi = (oeth_info *)sc->driver_private;
  struct oeth_private *cep = (struct oeth_private *)&(oi->cep);
  return cep->irq;
}

static int read_mii(int phy, int addr, volatile oeth_regs *regs)
{
  int tmp;

  do {
    OETH_REGLOAD(regs->miistatus, tmp);
  } while (tmp & OETH_MIISTATUS_BUSY);

  OETH_REGSAVE(regs->miiaddress, addr << 8 | phy);
  OETH_REGSAVE(regs->miicommand, OETH_MIICOMMAND_RSTAT);

  do {
    OETH_REGLOAD(regs->miistatus, tmp);
  } while (tmp & OETH_MIISTATUS_BUSY);

  OETH_REGLOAD(regs->miistatus, tmp);
  if (!(tmp & OETH_MIISTATUS_NVALID)) {
    OETH_REGLOAD(regs->miirx_data, tmp);
    return tmp;
  }
  else {
    diag_printf("open_eth: failed to read mii\n");
    return (0);
  }
}

static void write_mii(int phy, int addr, int data, volatile oeth_regs *regs)
{
  int tmp;
  do {
    OETH_REGLOAD(regs->miistatus, tmp);
  } while (tmp & OETH_MIISTATUS_BUSY);

  OETH_REGSAVE(regs->miiaddress, addr << 8 | phy);
  OETH_REGSAVE(regs->miitx_data, data);
  OETH_REGSAVE(regs->miicommand, OETH_MIICOMMAND_WCTRLDATA);

  do {
    OETH_REGLOAD(regs->miistatus, tmp);
  } while (tmp & OETH_MIISTATUS_BUSY);

}

bool openeth_device_init(struct eth_drv_sc *sc, cyg_uint32 idx, cyg_uint32 base, cyg_uint32 irq) {

	bool gotMac = false;

//diag_printf("OETH_RXBD_NUM=%d\n", OETH_RXBD_NUM);
//diag_printf("OETH_TXBD_NUM=%d\n", OETH_TXBD_NUM);

//diag_printf("ETHERMAC PHY address %d\n", FIAD_PHY_ADDRESS);

  struct oeth_private *cep = &openeth_priv_array[idx]->cep;
  volatile oeth_regs *regs;
  volatile oeth_bd *tx_bd, *rx_bd;
  int i;

  openeth_priv_array[idx]->found = 1;

  /* Try to get the ethernet station address from 0x1030000 - 6*/
  for(i = 0; i < 6; i++)
  {
	  macaddr[i] = *(macstart + i);
	  if(macaddr[i] != 0xff)
		  gotMac = true;
  }
  for(i = 0; i < 6; i++)
  {
	  if (macaddr[i] != 0x00)
		  break;
  }
  gotMac = gotMac && (i != 6);

  if (!gotMac)
  {
	diag_printf("Set MAC address via bootloader. ");
	return false;
  }

  /* Set default ethernet station address. */
  openeth_priv_array[idx]->dev_addr[0] = MACADDR0;
  openeth_priv_array[idx]->dev_addr[1] = MACADDR1;
  openeth_priv_array[idx]->dev_addr[2] = MACADDR2;
  openeth_priv_array[idx]->dev_addr[3] = MACADDR3;
  openeth_priv_array[idx]->dev_addr[4] = MACADDR4;
  openeth_priv_array[idx]->dev_addr[5] = MACADDR5;

  openeth_priv_array[idx]->sc = sc;
  openeth_priv_array[idx]->idx = idx;

  /* Get pointer ethernet controller configuration registers. */
  cep->regs = (oeth_regs *)(OETH_REG_BASE(base));
  regs = (oeth_regs *)(OETH_REG_BASE(base));
  cep->irq = irq;

  /* Reset the controller. */
  //OETH_REGSAVE(regs->moder , OETH_MODER_RST); //regs->moder = OETH_MODER_RST;	/* Reset ON */
  //OETH_REGANDIN(regs->moder , ~OETH_MODER_RST);	//regs->moder &= ~OETH_MODER_RST;	/* Reset OFF */
  OETH_REGSAVE(regs->ctrlmoder, 0);
  OETH_REGSAVE(regs->moder, OETH_MODER_RST);       /* Reset ON */
  OETH_REGSAVE(regs->moder, 0);

  /* Setting TXBD base to OETH_TXBD_NUM. */
  OETH_REGSAVE(regs->tx_bd_num , OETH_TXBD_NUM); //regs->tx_bd_num = OETH_TXBD_NUM;

  /* Initialize TXBD pointer*/
  cep->tx_bd_base = (oeth_bd *)OETH_BD_BASE(base);
  tx_bd = (volatile oeth_bd *)OETH_BD_BASE(base);

  /* Initialize RXBD pointer*/
  cep->rx_bd_base = ((oeth_bd *)OETH_BD_BASE(base)) + OETH_TXBD_NUM;
  rx_bd = ((volatile oeth_bd *)OETH_BD_BASE(base)) + OETH_TXBD_NUM;

  /* Initialize transmit pointers. */
  cep->rx_cur = 0;
  cep->tx_next = 0;
  cep->tx_last = 0;
  cep->tx_full = 0;

  /* Set min/max packet length  */
  OETH_REGSAVE(regs->packet_len , 0x00400600); //regs->packet_len = 0x00400600;

  /* Set IPGT register to recomended value  */
  OETH_REGSAVE(regs->ipgt , 0x00000012); //regs->ipgt = 0x00000012;

  /* Set IPGR1 register to recomended value  */
  OETH_REGSAVE(regs->ipgr1 , 0x0000000c); //regs->ipgr1 = 0x0000000c;

  /* Set IPGR2 register to recomended value  */
  OETH_REGSAVE(regs->ipgr2 , 0x00000012); //regs->ipgr2 = 0x00000012;

  /* Set COLLCONF register to recomended value */
  OETH_REGSAVE(regs->collconf , 0x000f003f); //regs->collconf = 0x000f003f;

  /* Set control module mode  */
#if 0
  OETH_REGSAVE(regs->ctrlmoder , OETH_CTRLMODER_TXFLOW | OETH_CTRLMODER_RXFLOW); //regs->ctrlmoder = OETH_CTRLMODER_TXFLOW | OETH_CTRLMODER_RXFLOW;
#else
  OETH_REGSAVE(regs->ctrlmoder , 0);//regs->ctrlmoder = 0;
#endif

#if 0
  /* FIX!!! There is no generic way of doing hardware PHY reset.
   *
   * Pray this PHY is hardwired to reset into autonegotiate.
   */
  HAL_WRITE_UINT32(0x08002000, 0xf); /* set all to 1 */
  HAL_DELAY_US(1000);
  HAL_WRITE_UINT32(0x08002004, 0x8); /* reset PHY */
  HAL_DELAY_US(1000);
  HAL_WRITE_UINT32(0x08002000, 0x8); /* come out of PHY reset... */
  HAL_DELAY_US(1000);
#endif

  /* Set PHY to show Tx status, Rx status and Link status */
  /*regs->miiaddress = 20<<8;
  regs->miitx_data = 0x1422;
  regs->miicommand = OETH_MIICOMMAND_WCTRLDATA;*/

  // The PHY is autonegotiate per defualt, we don't want to mess around with that..
#if 0
	#ifdef CYGPKG_DEVS_ETH_OPENCORES_ETHERMAC_ETH100
		OETH_REGSAVE(regs->miiaddress , 0 | FIAD_PHY_ADDRESS); //regs->miiaddress = 0;
		OETH_REGSAVE(regs->miitx_data , 0x2000); //regs->miitx_data = 0x2000;
		OETH_REGSAVE(regs->miicommand , OETH_MIICOMMAND_WCTRLDATA); //regs->miicommand = OETH_MIICOMMAND_WCTRLDATA;
	#else
		// switch to 10 mbit ethernet
		OETH_REGSAVE(regs->miiaddress , 0 | FIAD_PHY_ADDRESS); //regs->miiaddress = 0;
		OETH_REGSAVE(regs->miitx_data , 0); //regs->miitx_data = 0;
		OETH_REGSAVE(regs->miicommand , OETH_MIICOMMAND_WCTRLDATA); //regs->miicommand = OETH_MIICOMMAND_WCTRLDATA;
	#endif
#endif

#if 0
	/* This will dump all the phy registers */
	for (i=0; i<32; i++)
	{
		int val= read_mii(FIAD_PHY_ADDRESS, i, regs);
		diag_printf("reg %d 0x%08x\n", i, val);
	}
	diag_printf("Second pass - some registers are clear on read\n");
	for (i=0; i<32; i++)
	{
		int val= read_mii(FIAD_PHY_ADDRESS, i, regs);
		diag_printf("reg %d 0x%08x\n", i, val);
	}
#endif

  /* Initialize TXBDs. */
  for(i = 0; i < OETH_TXBD_NUM; i++) {
    OETH_REGSAVE(tx_bd[i].len_status , OETH_TX_BD_PAD | OETH_TX_BD_CRC | OETH_TX_BD_IRQ); //tx_bd[i].len_status = OETH_TX_BD_PAD | OETH_TX_BD_CRC | OETH_RX_BD_IRQ;
    OETH_REGSAVE(tx_bd[i].addr, txbuff+i); //tx_bd[i].addr = __pa(mem_addr);
  }
  OETH_REGORIN(tx_bd[OETH_TXBD_NUM - 1].len_status , OETH_TX_BD_WRAP); //tx_bd[OETH_TXBD_NUM - 1].len_status |= OETH_TX_BD_WRAP;

  for(i = 0; i < OETH_RXBD_NUM; i++) {
    OETH_REGSAVE(rx_bd[i].len_status , OETH_RX_BD_EMPTY | OETH_RX_BD_IRQ); //rx_bd[k].len_status = OETH_RX_BD_EMPTY | OETH_RX_BD_IRQ;
    OETH_REGSAVE(rx_bd[i].addr, rxbuff+i); //rx_bd[k].addr = __pa(mem_addr);
  }
  OETH_REGORIN(rx_bd[OETH_RXBD_NUM - 1].len_status , OETH_RX_BD_WRAP); //rx_bd[OETH_RXBD_NUM - 1].len_status |= OETH_RX_BD_WRAP;


  OETH_REGSAVE(regs->mac_addr1 , MACADDR0 << 8 | MACADDR1); //regs->mac_addr1 = MACADDR0 << 8 | MACADDR1;
  OETH_REGSAVE(regs->mac_addr0 , MACADDR2 << 24 | MACADDR3 << 16 | MACADDR4 << 8 | MACADDR5); //regs->mac_addr0 = MACADDR2 << 24 | MACADDR3 << 16 | MACADDR4 << 8 | MACADDR5;

  /* Clear all pending interrupts  */
  OETH_REGSAVE(regs->int_src , 0xffffffff); //regs->int_src = 0xffffffff;

  /* Promisc, IFG, CRCEn, do not receive small packets */
  OETH_REGSAVE(regs->moder , OETH_MODER_HUGEN | OETH_MODER_PAD | OETH_MODER_IFG | OETH_MODER_CRCEN); //regs->moder |= OETH_MODER_PAD | OETH_MODER_IFG | OETH_MODER_CRCEN;

  OETH_REGANDIN(regs->moder , ~OETH_MODER_FULLD); // ensure half duplex

  /* Enable interrupt sources. */
  OETH_REGSAVE(regs->int_mask ,
               OETH_INT_MASK_TXB  |  /* regs->int_mask = OETH_INT_MASK_TXB | OETH_INT_MASK_TXE | OETH_INT_MASK_RXF | OETH_INT_MASK_RXE | OETH_INT_MASK_BUSY | OETH_INT_MASK_TXC | OETH_INT_MASK_RXC; */
               OETH_INT_MASK_TXE  |
               OETH_INT_MASK_RXF  |
               OETH_INT_MASK_RXE  |
               OETH_INT_MASK_BUSY |
               OETH_INT_MASK_TXC  |
               OETH_INT_MASK_RXC);

  db_printf("%x: Open Ethernet Core Version 1.0 at [0x%x] irq %d\n", idx,(unsigned int)(cep->regs),(unsigned int)(cep->irq));

  // Initialize upper level driver
  (sc->funs->eth_drv->init)(sc, openeth_priv_array[idx]->dev_addr);

  return true;

}

// EOF if_oeth.c
