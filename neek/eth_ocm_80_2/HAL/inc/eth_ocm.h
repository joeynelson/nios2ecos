#ifndef __ETH_OCM_H__
#define __ETH_OCM_H__

#if defined(ALT_INICHE)
    #include "ins_eth_ocm.h"
#else
/**
 * eth_ocm_dev contains information aquired at initialization time that is 
 * required for further access to the device. This struct is initialized
 * by the alt_sys_init routine for each instance of the OpenCores MAC in the
 * system.
 */
typedef struct{
    alt_u32         base;
    alt_u8          irq;
    char            name[20];
} eth_ocm_dev;

//Information that can't be obtained later
#define ETH_OCM_INSTANCE(name, dev)     \
eth_ocm_dev dev = {                     \
    name##_BASE,                        \
    name##_IRQ,                         \
    name##_NAME                         \
}

#define ETH_OCM_INIT(name, dev_inst) 

#endif // !defined ALT_INICHE

#define ETH_OCM_STATUS_DOWN         2
#define ETH_OCM_STATUS_UP           1
#define ETH_OCM_MAC_ADDR_LEN        6
#define ETH_OCM_MIN_MTU             4
#define ETH_OCM_MAX_MTU             1518
#define ETH_OCM_FULL_DUPLEX_IPGT    0x15
#define ETH_OCM_HALF_DUPLEX_IPGT    0x12

#endif //__ETH_OCM_H__

