#ifndef __ETH_OCM_REGS_H__
#define __ETH_OCM_REGS_H__

#include <io.h>         //Altera IOWR and IORD
#include "alt_types.h"  //Altera defined types

#define ETH_OCM_MODER       0x00
#define ETH_OCM_INT_SOURCE  0x01
#define ETH_OCM_INT_MASK    0x02
#define ETH_OCM_IPGT        0x03
#define ETH_OCM_IPGR1       0x04
#define ETH_OCM_IPGR2       0x05
#define ETH_OCM_PACKETLEN   0x06
#define ETH_OCM_COLLCONF    0x07
#define ETH_OCM_TX_BD_NUM   0x08
#define ETH_OCM_CTRLMODER   0x09
#define ETH_OCM_MIIMODER    0x0A
#define ETH_OCM_MIICOMMAND  0x0B
#define ETH_OCM_MIIADDRESS  0x0C
#define ETH_OCM_MIITX_DATA  0x0D
#define ETH_OCM_MIIRX_DATA  0x0E
#define ETH_OCM_MIISTATUS   0x0F
#define ETH_OCM_MAC_ADDR0   0x10
#define ETH_OCM_MAC_ADDR1   0x11
#define ETH_OCM_ETH_HASH0   0x12
#define ETH_OCM_ETH_HASH1   0x13
#define ETH_OCM_ETH_CTRL    0x14
#define ETH_OCM_DESC_START  0x100
#define ETH_OCM_DESC_END    0x1FF

//Mode register bit masks
#define IOWR_ETH_OCM_MODER(base, dat)   \
    IOWR(base, ETH_OCM_MODER, dat)
#define IORD_ETH_OCM_MODER(base)    \
    IORD(base, ETH_OCM_MODER)

#define IOWR_ETH_OCM_INT_SOURCE(base, dat)  \
    IOWR(base, ETH_OCM_INT_SOURCE, dat)
#define IORD_ETH_OCM_INT_SOURCE(base)   \
    IORD(base, ETH_OCM_INT_SOURCE)

#define IOWR_ETH_OCM_INT_MASK(base, dat)    \
    IOWR(base, ETH_OCM_INT_MASK, dat)
#define IORD_ETH_OCM_INT_MASK(base) \
    IORD(base, ETH_OCM_INT_MASK)

#define IOWR_ETH_OCM_IPGT(base, dat)    \
    IOWR(base, ETH_OCM_IPGT, dat)
#define IORD_ETH_OCM_IPGT(base)         \
    IORD(base, ETH_OCM_IPGT)

#define IOWR_ETH_OCM_IPGR1(base, dat)   \
    IOWR(base, ETH_OCM_IPGR1, dat)
#define IORD_ETH_OCM_IPGR1(base)    \
    IORD(base, ETH_OCM_IPGR1)

#define IOWR_ETH_OCM_IPGR2(base, dat)   \
    IOWR(base, ETH_OCM_IPGR2, dat)
#define IORD_ETH_OCM_IPGR2(base)    \
    IORD(base, ETH_OCM_IPGR2)

#define IOWR_ETH_OCM_PACKETLEN(base, dat)   \
    IOWR(base, ETH_OCM_PACKETLEN, dat)
#define IORD_ETH_OCM_PACKETLEN(base)    \
    IORD(base, ETH_OCM_PACKETLEN)

#define IOWR_ETH_OCM_COLLCONF(base, dat)    \
    IOWR(base, ETH_OCM_COLLCONF, dat)
#define IORD_ETH_OCM_COLLCONF(base) \
    IORD(base, ETH_OCM_COLLCONF)

#define IOWR_ETH_OCM_TX_BD_NUM(base, dat)   \
    IOWR(base, ETH_OCM_TX_BD_NUM, dat)
#define IORD_ETH_OCM_TX_BD_NUM(base)    \
    IORD(base, ETH_OCM_TX_BD_NUM)

#define IOWR_ETH_OCM_CTRLMODER(base, dat)   \
    IOWR(base, ETH_OCM_CTRLMODER, dat)
#define IORD_ETH_OCM_CTRLMODER(base)    \
    IORD(base, ETH_OCM_CTRLMODER)

#define IOWR_ETH_OCM_MIIMODER(base, dat) \
    IOWR(base, ETH_OCM_MIIMODER, dat)
#define IORD_ETH_OCM_MIIMODER(base)  \
    IORD(base, ETH_OCM_MIIMODER)

#define IOWR_ETH_OCM_MIICOMMAND(base, dat)  \
    IOWR(base, ETH_OCM_MIICOMMAND, dat)
#define IORD_ETH_OCM_MIICOMMAND(base)   \
    IORD(base, ETH_OCM_MIICOMMAND)

#define IOWR_ETH_OCM_MIIADDRESS(base, dat)  \
    IOWR(base, ETH_OCM_MIIADDRESS, dat)
#define IORD_ETH_OCM_MIIADDRESS(base)   \
    IORD(base, ETH_OCM_MIIADDRESS)

#define IOWR_ETH_OCM_MIITX_DATA(base, dat)  \
    IOWR(base, ETH_OCM_MIITX_DATA, dat)
#define IORD_ETH_OCM_MIITX_DATA(base)   \
    IORD(base, ETH_OCM_MIITX_DATA)

#define IOWR_ETH_OCM_MIIRX_DATA(base, dat)  \
    IOWR(base, ETH_OCM_MIIRX_DATA, dat)
#define IORD_ETH_OCM_MIIRX_DATA(base)   \
    IORD(base, ETH_OCM_MIIRX_DATA)

#define IOWR_ETH_OCM_MIISTATUS(base, dat)   \
    IOWR(base, ETH_OCM_MIISTATUS, dat)
#define IORD_ETH_OCM_MIISTATUS(base)    \
    IORD(base, ETH_OCM_MIISTATUS)

#define IOWR_ETH_OCM_MAC_ADDR0(base, dat)   \
    IOWR(base, ETH_OCM_MAC_ADDR0, dat)
#define IORD_ETH_OCM_MAC_ADDR0(base)    \
    IORD(base, ETH_OCM_MAC_ADDR0)

#define IOWR_ETH_OCM_MAC_ADDR1(base, dat)   \
    IOWR(base, ETH_OCM_MAC_ADDR1, dat)
#define IORD_ETH_OCM_MAC_ADDR1(base)    \
    IORD(base, ETH_OCM_MAC_ADDR1)

#define IOWR_ETH_OCM_ETH_HASH0(base, dat)   \
    IOWR(base, ETH_OCM_ETH_HASH0, dat)
#define IORD_ETH_OCM_ETH_HASH0(base)    \
    IORD(base, ETH_OCM_ETH_HASH0)

#define IOWR_ETH_OCM_ETH_HASH1(base, dat)   \
    IOWR(base, ETH_OCM_ETH_HASH1, dat)
#define IORD_ETH_OCM_ETH_HASH1(base)    \
    IORD(base, ETH_OCM_ETH_HASH1)

#define IOWR_ETH_OCM_ETH_CTRL(base, dat)    \
    IOWR(base, ETH_OCM_ETH_CTRL, dat)
#define IORD_ETH_OCM_ETH_CTRL(base) \
    IORD(base, ETH_OCM_ETH_CTRL)

#define ETH_OCM_MODER_RECSMALL_MSK      0x00010000
#define ETH_OCM_MODER_RECSMALL_OFST     16
#define ETH_OCM_MODER_PAD_MSK           0x00008000
#define ETH_OCM_MODER_PAD_OFST          15
#define ETH_OCM_MODER_HUGEN_MSK         0x00004000
#define ETH_OCM_MODER_HUGEN_OFST        14
#define ETH_OCM_MODER_CRCEN_MSK         0x00002000
#define ETH_OCM_MODER_CRCEN_OFST        13
#define ETH_OCM_MODER_DLYCRCEN_MSK      0x00001000
#define ETH_OCM_MODER_DLYCRCEN_OFST     12
#define ETH_OCM_MODER_FULLD_MSK         0x00000400
#define ETH_OCM_MODER_FULLD_OFST        10
#define ETH_OCM_MODER_EXDFREN_MSK       0x00000200
#define ETH_OCM_MODER_EXDFREN_OFST      9
#define ETH_OCM_MODER_NOBKOF_MSK        0x00000100
#define ETH_OCM_MODER_NOBKOF_OFST       8
#define ETH_OCM_MODER_LOOPBCK_MSK       0x00000080
#define ETH_OCM_MODER_LOOPBCK_OFST      7
#define ETH_OCM_MODER_IFG_MSK           0x00000040
#define ETH_OCM_MODER_IFG_OFST          6
#define ETH_OCM_MODER_PRO_MSK           0x00000020
#define ETH_OCM_MODER_PRO_OFST          5
#define ETH_OCM_MODER_IAM_MSK           0x00000010
#define ETH_OCM_MODER_IAM_OFST          4
#define ETH_OCM_MODER_BRO_MSK           0x00000008
#define ETH_OCM_MODER_BRO_OFST          3
#define ETH_OCM_MODER_NOPRE_MSK         0x00000004
#define ETH_OCM_MODER_NOPRE_OFST        2
#define ETH_OCM_MODER_TXEN_MSK          0x00000002
#define ETH_OCM_MODER_TXEN_OFST         1
#define ETH_OCM_MODER_RXEN_MSK          0x00000001
#define ETH_OCM_MODER_RXEN_OFST         0
//End of bit masks for MODE register

//Define bit masks for INT_SOURCE and INT_MASK registers
#define ETH_OCM_INT_MASK_RXC_MSK        0x00000040
#define ETH_OCM_INT_MASK_RXC_OFST       6
#define ETH_OCM_INT_MASK_TXC_MSK        0x00000020
#define ETH_OCM_INT_MASK_TXC_OFST       5
#define ETH_OCM_INT_MASK_BUSY_MSK       0x00000010
#define ETH_OCM_INT_MASK_BUSY_OFST      4   
#define ETH_OCM_INT_MASK_RXE_MSK        0x00000008
#define ETH_OCM_INT_MASK_RXE_OFST       3
#define ETH_OCM_INT_MASK_RXB_MSK        0x00000004
#define ETH_OCM_INT_MASK_RXB_OFST       2
#define ETH_OCM_INT_MASK_TXE_MSK        0x00000002
#define ETH_OCM_INT_MASK_TXE_OFST       1
#define ETH_OCM_INT_MASK_TXB_MSK        0x00000001
#define ETH_OCM_INT_MASK_TXB_OFST       0
//End of bit masks for INT_SOURCE register

//Bit masks for the PACKETLEN register
#define ETH_OCM_PACKETLEN_MINFL_MSK     0xFFFF0000
#define ETH_OCM_PACKETLEN_MINFL_OFST    16
#define ETH_OCM_PACKETLEN_MAXFL_MSK     0x0000FFFF
#define ETH_OCM_PACKETLEN_MAXFL_OFST    0
//End bit masks for PACKETLEN register

//Bit masks for COLLCONF register
#define ETH_OCM_COLLCONF_MAXRET_MSK     0x000F0000
#define ETH_OCM_COLLCONF_MAXRET_OFST    16
#define ETH_OCM_COLLCONF_COLLVALID_MSK  0x0000003F
#define ETH_OCM_COLLCONF_COLLVALID_OFST 0
//End bit masks for COLLCONF register

//Bit masks for CTRLMODER register
#define ETH_OCM_CTRLMODER_TXFLOW_MSK    0x00000004
#define ETH_OCM_CTRLMODER_TXFLOW_OFST   2
#define ETH_OCM_CTRLMODER_RXFLOW_MSK    0x00000002
#define ETH_OCM_CTRLMODER_RXFLOW_OFST   1
#define ETH_OCM_CTRLMODER_PASSALL_MSK   0x00000001
#define ETH_OCM_CTRLMODER_PASSALL_OFST  0
//End bit masks for CTRLMODER register

//Bit masks for MIIMODER register
#define ETH_OCM_MIIMODER_MIINOPRE_MSK   0x00000100
#define ETH_OCM_MIIMODER_MIINOPRE_OFST  8
#define ETH_OCM_MIIMODER_CLKDIV_MSK     0x000000FF
#define ETH_OCM_MIIMODER_CLKDIV_OFST    0
//End bit masks for MIIMODER register

//Bit masks for MIICOMMAND register
#define ETH_OCM_MIICOMMAND_WCTRLDATA_MSK    0x00000004
#define ETH_OCM_MIICOMMAND_WCTRLDATA_OFST   2
#define ETH_OCM_MIICOMMAND_RSTAT_MSK        0x00000002
#define ETH_OCM_MIICOMMAND_RSTAT_OFST       1
#define ETH_OCM_MIICOMMAND_SCANSTAT_MSK     0x00000001
#define ETH_OCM_MIICOMMAND_SCANSTAT_OFST    0
//End bit masks for MIICOMMAND register

//Bit masks for MIIADDRESS register
#define ETH_OCM_MIIADDRESS_RGAD_MSK         0x00001F00
#define ETH_OCM_MIIADDRESS_RGAD_OFST        8
#define ETH_OCM_MIIADDRESS_FIAD_MSK         0x0000001F
#define ETH_OCM_MIIADDRESS_FIAD_OFST        0
//End bit masks for MIIADDRESS register

//Bit masks for MIISTATUS register
#define ETH_OCM_MIISTATUS_NVALID_MSK        0x00000004
#define ETH_OCM_MIISTATUS_NVALID_OFST       2
#define ETH_OCM_MIISTATUS_BUSY_MSK          0x00000002
#define ETH_OCM_MIISTATUS_BUSY_OFST         1
#define ETH_OCM_MIISTATUS_LINKFAIL_MSK      0x00000001
#define ETH_OCM_MIISTATUS_LINKFAIL_OFST     0
//End bit masks for MIISTATUS register

//Bit masks for TXCTRL register
#define ETH_OCM_TXCTRL_TXPAUSERA_MSK        0x00010000
#define ETH_OCM_TXCTRL_TXPAUSERA_OFST       16
#define ETH_OCM_TXCTRL_TXPAUSETV_MSK        0x0000FFFF
#define ETH_OCM_TXCTRL_TXPAUSETV_OFST       0
//End bit masks for TXCTRL register

#endif //__ETH_OCM_REGS_H__
