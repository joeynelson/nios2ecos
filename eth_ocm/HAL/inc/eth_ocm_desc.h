#ifndef __ETH_OCM_DESCRIPTORS_H__
#define __ETH_OCM_DESCRIPTORS_H__

#include <errno.h>
#include "eth_ocm_regs.h"

#define ETH_OCM_DESC_SIZE       2   //descriptors span two word
#define ETH_OCM_DESC_CTRL_WRD   0   //Control word is first word
#define ETH_OCM_DESC_PTR_WRD    1   //Pointer is second word
#define ETH_OCM_MAX_DESCRIPTORS 128

#define IOWR_ETH_OCM_DESC_CTRL(base, desc, dat) \
    IOWR(base, (ETH_OCM_DESC_START + (desc<<1)), dat)
#define IORD_ETH_OCM_DESC_CTRL(base, desc)  \
    IORD(base, (ETH_OCM_DESC_START + (desc<<1)))

#define IOWR_ETH_OCM_DESC_PTR(base, desc, dat) \
    IOWR(base, ((ETH_OCM_DESC_START + (desc<<1)) | 1), dat)
#define IORD_ETH_OCM_DESC_PTR(base, desc) \
    IORD(base, ((ETH_OCM_DESC_START + (desc<<1)) | 1))

//Transmit descriptor bit masks
#define ETH_OCM_TXDESC_LEN_MSK      0xFFFF0000
#define ETH_OCM_TXDESC_LEN_OFST     16
#define ETH_OCM_TXDESC_READY_MSK    0x00008000
#define ETH_OCM_TXDESC_READY_OFST   15
#define ETH_OCM_TXDESC_IRQ_MSK      0x00004000
#define ETH_OCM_TXDESC_IRQ_OFST     14
#define ETH_OCM_TXDESC_WRAP_MSK     0x00002000
#define ETH_OCM_TXDESC_WRAP_OFST    13
#define ETH_OCM_TXDESC_PAD_MSK      0x00001000
#define ETH_OCM_TXDESC_PAD_OFST     12
#define ETH_OCM_TXDESC_CRC_MSK      0x00000800
#define ETH_OCM_TXDESC_CRC_OFST     11
#define ETH_OCM_TXDESC_UR_MSK       0x00000100
#define ETH_OCM_TXDESC_UR_OFST      8
#define ETH_OCM_TXDESC_RTRY_MSK     0x000000F0
#define ETH_OCM_TXDESC_RTRY_OFST    4
#define ETH_OCM_TXDESC_RL_MSK       0x00000008
#define ETH_OCM_TXDESC_RL_OFST      3
#define ETH_OCM_TXDESC_LC_MSK       0x00000004
#define ETH_OCM_TXDESC_LC_OFST      2
#define ETH_OCM_TXDESC_DF_MSK       0x00000002
#define ETH_OCM_TXDESC_DF_OFST      1
#define ETH_OCM_TXDESC_CS_MSK       0x00000001
#define ETH_OCM_TXDESC_CS_OFST      0
//End transmit descriptor bit masks

//Receive descriptor bit masks
#define ETH_OCM_RXDESC_LEN_MSK      0xFFFF0000
#define ETH_OCM_RXDESC_LEN_OFST     16
#define ETH_OCM_RXDESC_EMPTY_MSK    0x00008000
#define ETH_OCM_RXDESC_EMPTY_OFST   15
#define ETH_OCM_RXDESC_IRQ_MSK      0x00004000
#define ETH_OCM_RXDESC_IRQ_OFST     14
#define ETH_OCM_RXDESC_WRAP_MSK     0x00002000
#define ETH_OCM_RXDESC_WRAP_OFST    13
#define ETH_OCM_RXDESC_CF_MSK       0x00000100
#define ETH_OCM_RXDESC_CF_OFST      8
#define ETH_OCM_RXDESC_M_MSK        0x00000080
#define ETH_OCM_RXDESC_M_OFST       7
#define ETH_OCM_RXDESC_OR_MSK       0x00000040
#define ETH_OCM_RXDESC_OR_OFST      6
#define ETH_OCM_RXDESC_IS_MSK       0x00000020
#define ETH_OCM_RXDESC_IS_OFST      5
#define ETH_OCM_RXDESC_DN_MSK       0x00000010
#define ETH_OCM_RXDESC_DN_OFST      4
#define ETH_OCM_RXDESC_TL_MSK       0x00000008
#define ETH_OCM_RXDESC_TL_OFST      3
#define ETH_OCM_RXDESC_SF_MSK       0x00000004
#define ETH_OCM_RXDESC_SF_OFST      2
#define ETH_OCM_RXDESC_CRC_MSK      0x00000002
#define ETH_OCM_RXDESC_CRC_OFST     1
#define ETH_OCM_RXDESC_LC_MSK       0x00000001
#define ETH_OCM_RXDESC_LC_OFST      0
//End receive descriptor bit masks

#define ETH_OCM_RXDESC_ERROR_MSK    \
    (ETH_OCM_RXDESC_OR_MSK  |       \
    ETH_OCM_RXDESC_IS_MSK   |       \
    ETH_OCM_RXDESC_DN_MSK   |       \
    ETH_OCM_RXDESC_TL_MSK   |       \
    ETH_OCM_RXDESC_SF_MSK   |       \
    ETH_OCM_RXDESC_CRC_MSK  |       \
    ETH_OCM_RXDESC_LC_MSK)        \

typedef struct{
    alt_u32     ctrl;
    alt_u32     ptr;
} eth_ocm_desc;

#endif //__ETH_OCM_DESCRIPTORS_H__

