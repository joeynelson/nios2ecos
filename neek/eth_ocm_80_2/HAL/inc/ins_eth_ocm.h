#ifndef __INS_ETH_OCM_H__
#define __INS_ETH_OCM_H__
//Altera includes
#include <io.h>             //Altera IOWR, IORD macros
#include <errno.h>          //Error return codes
#include "alt_types.h"      //Altera variable types 
#include <sys/alt_cache.h>  //For cache bypassing
#include <sys/alt_irq.h>    //for ISR registering

//Interniche includes
#include "alt_iniche_dev.h" //For alt_iniche_dev struct
#include "ether.h"          //For ethhdr struct

//UCOSII includes
#ifdef UCOS_II
#include <ucos_ii.h>
#include <os_cpu.h>
#endif

//Opencores MAC includes
#include "eth_ocm.h"
#include "eth_ocm_regs.h"
#include "eth_ocm_desc.h"

#include "system.h"         //Macro defs for your system

//****************************************************************************
//************************** Compilation options *****************************

// Set debug print statement level.
// Level 0 - No debugging)
// Level 1 - Harmless debugging (initialization) will not harm performance.
// Level 2 - Runtime debugging Transmit errors. High level of
//           errors may impact performance due to excessive printing.
// Level 3 - Runtimine debugging transmit and receive errors. High level of TX
//           or RX errors may impact performance due to excessive printing.
// Level 4 - Low level debugging. Will impact performance.
#ifndef ETH_OCM_DBG_LVL
    #define ETH_OCM_DBG_LVL 0
#endif

// Define ETH_OCM_SYNC_TX to enable synchronous transmit operations
// Whether or not this makes sense depends on your system. With syncronous
// transmits, all transmits block until the frame as been transmitted. This
// obviously implies a performance hit as nothing else can be done until the
// frame has been tranferred.
//      On the other hand, asynchronous transfers incurr a performance hit
//  due to increased interrupt activity. Try both to see which gives you higher
//  performance.
// #define ETH_OCM_SYNC_TX

// Total number of available descriptors
#define ETH_OCM_TOTAL_DESC_COUNT    128

// Define the number of descriptors used for Transmit
#ifndef ETH_OCM_TX_DESC_COUNT
    #define ETH_OCM_TX_DESC_COUNT       1 
#endif //ifndef ETH_OCM_TX_DESC_COUNT

// Validate Transmit descriptor count
#if (ETH_OCM_TX_DESC_COUNT >= ETH_OCM_TOTAL_DESC_COUNT)
    #error [ins_eth_ocm.h] Defined TX descriptor count not supported
#endif

// Define the number of descriptors used for Receive
#ifndef ETH_OCM_RX_DESC_COUNT
    #define ETH_OCM_RX_DESC_COUNT       1   //Requires sufficient Iniche Buffers 
#endif //ifndef ETH_OCM_TX_DESC_COUNT

// Validate Receive descriptor count
#if (ETH_OCM_RX_DESC_COUNT > (128 - ETH_OCM_TX_DESC_COUNT))
    #error  [ins_eth_ocm.h] Defined RX descriptor count not supported
#endif

//************************ End Compilation options ***************************
//****************************************************************************
// Define the size of Buffers the driver will request for RX.
#define ETH_OCM_BUF_ALLOC_SIZE      1536

// Define the flags that will trigger an interrupt.
#ifdef ETH_OCM_TX_SYNC
#define ETH_OCM_DEFAULT_INTERRUPT_MASK  \
    (ETH_OCM_INT_MASK_BUSY_MSK  |       \
     ETH_OCM_INT_MASK_RXE_MSK   |       \
     ETH_OCM_INT_MASK_RXB_MSK )

#else

#define ETH_OCM_DEFAULT_INTERRUPT_MASK  \
    (ETH_OCM_INT_MASK_BUSY_MSK  |       \
     ETH_OCM_INT_MASK_TXE_MSK   |       \
     ETH_OCM_INT_MASK_TXB_MSK   |       \
     ETH_OCM_INT_MASK_RXE_MSK   |       \
     ETH_OCM_INT_MASK_RXB_MSK )

#endif // !defined ETH_OCM_TX_SYNC
    

// Define the number of times to poll the Transmit descriptor to determine if
// a synchronous transmit has finished (Not used in regular asynchronous mode)
#define ETH_OCM_TRANSMIT_TIMEOUT    1000000

/**
 * eth_ocm_regs struct is used to enumerate the registers within
 * the Opencores MAC. It is custom tuned for the number of TX and RX
 * Descriptors 
 */
typedef struct{
    alt_u32         moder;
    alt_u32         int_source;
    alt_u32         int_mask;
    alt_u32         ipgt;
    alt_u32         ipgr1;
    alt_u32         ipgr2;
    alt_u32         packetlen;
    alt_u32         collconf;
    alt_u32         tx_bd_num;
    alt_u32         ctrlmoder;
    alt_u32         miimoder;
    alt_u32         miicommand;
    alt_u32         miiaddress;
    alt_u32         miitx_data;
    alt_u32         miirx_data;
    alt_u32         miistatus;
    alt_u32         mac_addr0;
    alt_u32         mac_addr1;
    alt_u32         eth_hash0;
    alt_u32         eth_hash1;
    alt_u32         eth_ctrl;
    alt_u32         reserved[235];
    eth_ocm_desc    txdescs[ETH_OCM_TX_DESC_COUNT];
    eth_ocm_desc    rxdescs[ETH_OCM_RX_DESC_COUNT];
} volatile eth_ocm_regs;

/**
 * eth_ocm_info contains information needed for runtime operation of the MAC in
 * conjunction with the InterNiche Stack. It is created and initialized if and
 * when the InterNiche Stack calls the MAC driver's "prep" function.
 */ 
typedef struct{
    unsigned char   mac_addr[6];    //MAC address
    NET             netp;           //Pointer to associated interface
    alt_u8          sem;            //semaphore
    alt_u8          cur_tx_desc;    //Current TX descriptor (currently transferring)
    alt_u8          cur_rx_desc;    //Current RX descriptor
    PACKET          *rx_pkts;       //To be alloced and used as array
#ifndef ETH_OCM_SYNC_TX
    queue           tosend;         //Packets to be sent queue
    queue           sending;        //Packets being sent queue
    alt_u8          next_tx_desc;   //Next TX descriptor (to schedule a transfer)
    alt_u8          next_tx_desc_rdy; //Next TX descriptor can be used
#endif
} eth_ocm_info;

/**
 * eth_ocm_dev contains information aquired at initialization time that is 
 * required for further access to the device. This struct is initialized
 * by the alt_sys_init routine for each instance of the OpenCores MAC in the
 * system.
 */
typedef struct{
    alt_iniche_dev  ins_dev;
    alt_u32         base;
    eth_ocm_regs    *regs;
    alt_u8          irq;
    char            name[20];
    eth_ocm_info    *info;
} eth_ocm_dev;


#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

error_t eth_ocm_prep(alt_iniche_dev *ins_dev);
int     eth_ocm_init(int iface);
int     eth_ocm_close(int iface);
void    eth_ocm_stats(void *pio, int iface);

#ifdef  __cplusplus
}
#endif  // __cplusplus

//Information that can't be obtained later
#define ETH_OCM_INSTANCE(name, dev)     \
eth_ocm_dev dev =                       \
{                                       \
    {                                   \
        ALT_LLIST_ENTRY,                \
        name##_NAME,                    \
        eth_ocm_prep                    \
    },                                  \
    name##_BASE,                        \
    (eth_ocm_regs*)(name##_BASE | 0x80000000), \
    name##_IRQ,                         \
    name##_NAME,                        \
    NULL                                \
}

//Opencores ethernet mac init function
#define ETH_OCM_INIT(name, dev_inst)    \
    alt_iniche_dev_reg(&(dev_inst.ins_dev))

#endif  //__INS_ETH_OCM_H__
