#ifndef CYGONCE_DEVS_ETH_OPENCORES_ETHERMAC_INFO_H
#define CYGONCE_DEVS_ETH_OPENCORES_ETHERMAC_INFO_H
/*==========================================================================
//
//        oeth_info.h
//
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003 Red Hat, Inc.
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
// Author(s):     
// Contributors:  
// Date:          2004-01-20
// Description:
//
//####DESCRIPTIONEND####
*/

#include <pkgconf/devs_eth_opencores_ethermac.h>


/* Ethernet configuration registers */
typedef struct _oeth_regs {
        cyg_uint32    moder;          /* Mode Register */
        cyg_uint32    int_src;        /* Interrupt Source Register */
        cyg_uint32    int_mask;       /* Interrupt Mask Register */
        cyg_uint32    ipgt;           /* Back to Bak Inter Packet Gap Register */
        cyg_uint32    ipgr1;          /* Non Back to Back Inter Packet Gap Register 1 */
        cyg_uint32    ipgr2;          /* Non Back to Back Inter Packet Gap Register 2 */
        cyg_uint32    packet_len;     /* Packet Length Register (min. and max.) */
        cyg_uint32    collconf;       /* Collision and Retry Configuration Register */
        cyg_uint32    tx_bd_num;      /* Transmit Buffer Descriptor Number Register */
        cyg_uint32    ctrlmoder;      /* Control Module Mode Register */
        cyg_uint32    miimoder;       /* MII Mode Register */
        cyg_uint32    miicommand;     /* MII Command Register */
        cyg_uint32    miiaddress;     /* MII Address Register */
        cyg_uint32    miitx_data;     /* MII Transmit Data Register */
        cyg_uint32    miirx_data;     /* MII Receive Data Register */
        cyg_uint32    miistatus;      /* MII Status Register */
        cyg_uint32    mac_addr0;      /* MAC Individual Address Register 0 */
        cyg_uint32    mac_addr1;      /* MAC Individual Address Register 1 */
        cyg_uint32    hash_addr0;     /* Hash Register 0 */
        cyg_uint32    hash_addr1;     /* Hash Register 1 */                           
} oeth_regs;

/* Ethernet buffer descriptor */
typedef struct _oeth_bd {
        cyg_uint32    len_status;
        cyg_uint32    addr;           /* Buffer address */
} oeth_bd;

#define OETH_REG_BASE(b)        (b)
#define OETH_BD_BASE(b)         ((b) + 0x400)
#define OETH_TOTAL_BD           128
#define OETH_MAXBUF_LEN         0x600
                                
/* Tx BD */                     
#define OETH_TX_BD_READY        0x8000 /* Tx BD Ready */
#define OETH_TX_BD_IRQ          0x4000 /* Tx BD IRQ Enable */
#define OETH_TX_BD_WRAP         0x2000 /* Tx BD Wrap (last BD) */
#define OETH_TX_BD_PAD          0x1000 /* Tx BD Pad Enable */
#define OETH_TX_BD_CRC          0x0800 /* Tx BD CRC Enable */
                                
#define OETH_TX_BD_UNDERRUN     0x0100 /* Tx BD Underrun Status */
#define OETH_TX_BD_RETRY        0x00F0 /* Tx BD Retry Status */
#define OETH_TX_BD_RETLIM       0x0008 /* Tx BD Retransmission Limit Status */
#define OETH_TX_BD_LATECOL      0x0004 /* Tx BD Late Collision Status */
#define OETH_TX_BD_DEFER        0x0002 /* Tx BD Defer Status */
#define OETH_TX_BD_CARRIER      0x0001 /* Tx BD Carrier Sense Lost Status */
#define OETH_TX_BD_STATS        (OETH_TX_BD_UNDERRUN            | \
                                OETH_TX_BD_RETRY                | \
                                OETH_TX_BD_RETLIM               | \
                                OETH_TX_BD_LATECOL              | \
                                OETH_TX_BD_DEFER                | \
                                OETH_TX_BD_CARRIER)
                                
/* Rx BD */                     
#define OETH_RX_BD_EMPTY        0x8000 /* Rx BD Empty */
#define OETH_RX_BD_IRQ          0x4000 /* Rx BD IRQ Enable */
#define OETH_RX_BD_WRAP         0x2000 /* Rx BD Wrap (last BD) */
                                
#define OETH_RX_BD_MISS         0x0080 /* Rx BD Miss Status */
#define OETH_RX_BD_OVERRUN      0x0040 /* Rx BD Overrun Status */
#define OETH_RX_BD_INVSIMB      0x0020 /* Rx BD Invalid Symbol Status */
#define OETH_RX_BD_DRIBBLE      0x0010 /* Rx BD Dribble Nibble Status */
#define OETH_RX_BD_TOOLONG      0x0008 /* Rx BD Too Long Status */
#define OETH_RX_BD_SHORT        0x0004 /* Rx BD Too Short Frame Status */
#define OETH_RX_BD_CRCERR       0x0002 /* Rx BD CRC Error Status */
#define OETH_RX_BD_LATECOL      0x0001 /* Rx BD Late Collision Status */
#define OETH_RX_BD_STATS        (OETH_RX_BD_MISS                | \
                                OETH_RX_BD_OVERRUN              | \
                                OETH_RX_BD_INVSIMB              | \
                                OETH_RX_BD_DRIBBLE              | \
                                OETH_RX_BD_TOOLONG              | \
                                OETH_RX_BD_SHORT                | \
                                OETH_RX_BD_CRCERR               | \
                                OETH_RX_BD_LATECOL)

/* MODER Register */
#define OETH_MODER_RXEN         0x00000001 /* Receive Enable  */
#define OETH_MODER_TXEN         0x00000002 /* Transmit Enable */
#define OETH_MODER_NOPRE        0x00000004 /* No Preamble  */
#define OETH_MODER_BRO          0x00000008 /* Reject Broadcast */
#define OETH_MODER_IAM          0x00000010 /* Use Individual Hash */
#define OETH_MODER_PRO          0x00000020 /* Promiscuous (receive all) */
#define OETH_MODER_IFG          0x00000040 /* Min. IFG not required */
#define OETH_MODER_LOOPBCK      0x00000080 /* Loop Back */
#define OETH_MODER_NOBCKOF      0x00000100 /* No Backoff */
#define OETH_MODER_EXDFREN      0x00000200 /* Excess Defer */
#define OETH_MODER_FULLD        0x00000400 /* Full Duplex */
#define OETH_MODER_RST          0x00000800 /* Reset MAC */
#define OETH_MODER_DLYCRCEN     0x00001000 /* Delayed CRC Enable */
#define OETH_MODER_CRCEN        0x00002000 /* CRC Enable */
#define OETH_MODER_HUGEN        0x00004000 /* Huge Enable */
#define OETH_MODER_PAD          0x00008000 /* Pad Enable */
#define OETH_MODER_RECSMALL     0x00010000 /* Receive Small */
 
/* Interrupt Source Register */
#define OETH_INT_TXB            0x00000001 /* Transmit Buffer IRQ */
#define OETH_INT_TXE            0x00000002 /* Transmit Error IRQ */
#define OETH_INT_RXF            0x00000004 /* Receive Frame IRQ */
#define OETH_INT_RXE            0x00000008 /* Receive Error IRQ */
#define OETH_INT_BUSY           0x00000010 /* Busy IRQ */
#define OETH_INT_TXC            0x00000020 /* Transmit Control Frame IRQ */
#define OETH_INT_RXC            0x00000040 /* Received Control Frame IRQ */

/* Interrupt Mask Register */
#define OETH_INT_MASK_TXB       0x00000001 /* Transmit Buffer IRQ Mask */
#define OETH_INT_MASK_TXE       0x00000002 /* Transmit Error IRQ Mask */
#define OETH_INT_MASK_RXF       0x00000004 /* Receive Frame IRQ Mask */
#define OETH_INT_MASK_RXE       0x00000008 /* Receive Error IRQ Mask */
#define OETH_INT_MASK_BUSY      0x00000010 /* Busy IRQ Mask */
#define OETH_INT_MASK_TXC       0x00000020 /* Transmit Control Frame IRQ Mask */
#define OETH_INT_MASK_RXC       0x00000040 /* Received Control Frame IRQ Mask */
 
/* Control Module Mode Register */
#define OETH_CTRLMODER_PASSALL  0x00000001 /* Pass Control Frames */
#define OETH_CTRLMODER_RXFLOW   0x00000002 /* Receive Control Flow Enable */
#define OETH_CTRLMODER_TXFLOW   0x00000004 /* Transmit Control Flow Enable */
                               
/* MII Mode Register */        
#define OETH_MIIMODER_CLKDIV    0x000000FF /* Clock Divider */
#define OETH_MIIMODER_NOPRE     0x00000100 /* No Preamble */
#define OETH_MIIMODER_RST       0x00000200 /* MIIM Reset */
 
/* MII Command Register */
#define OETH_MIICOMMAND_SCANSTAT  0x00000001 /* Scan Status */
#define OETH_MIICOMMAND_RSTAT     0x00000002 /* Read Status */
#define OETH_MIICOMMAND_WCTRLDATA 0x00000004 /* Write Control Data */
 
/* MII Address Register */
#define OETH_MIIADDRESS_FIAD    0x0000001F /* PHY Address */
#define OETH_MIIADDRESS_RGAD    0x00001F00 /* RGAD Address */
 
/* MII Status Register */
#define OETH_MIISTATUS_LINKFAIL 0x00000001 /* Link Fail */
#define OETH_MIISTATUS_BUSY     0x00000002 /* MII Busy */
#define OETH_MIISTATUS_NVALID   0x00000004 /* Data in MII Status Register is invalid */

/* Buffer number (must be 2^n)  */
#define OETH_RXBD_NUM		CYGINT_DEVS_ETH_OPENCORES_ETHERMAC_RxNUM
#define OETH_TXBD_NUM		CYGINT_DEVS_ETH_OPENCORES_ETHERMAC_TxNUM
#define OETH_RXBD_NUM_MASK	(OETH_RXBD_NUM-1)
#define OETH_TXBD_NUM_MASK	(OETH_TXBD_NUM-1)

/* Buffer size  (if not XXBUF_PREALLOC */
#define OETH_MAX_FRAME_SIZE	((1518 + 7) & ~7)

/* Buffer size  */
#define OETH_RX_BUFF_SIZE	OETH_MAX_FRAME_SIZE
#define OETH_TX_BUFF_SIZE	OETH_MAX_FRAME_SIZE

struct net_device_stats
{
	unsigned long	rx_packets;		/* total packets received	*/
	unsigned long	tx_packets;		/* total packets transmitted	*/
	unsigned long	rx_bytes;		/* total bytes received 	*/
	unsigned long	tx_bytes;		/* total bytes transmitted	*/
	unsigned long	rx_errors;		/* bad packets received		*/
	unsigned long	tx_errors;		/* packet transmit problems	*/
	unsigned long	rx_dropped;		/* no space in linux buffers	*/
	unsigned long	tx_dropped;		/* no space available in linux	*/
	unsigned long	multicast;		/* multicast packets received	*/
	unsigned long	collisions;

	/* detailed rx_errors: */
	unsigned long	rx_length_errors;
	unsigned long	rx_over_errors;		/* receiver ring buff overflow	*/
	unsigned long	rx_crc_errors;		/* recved pkt with crc error	*/
	unsigned long	rx_frame_errors;	/* recv'd frame alignment error */
	unsigned long	rx_fifo_errors;		/* recv'r fifo overrun		*/
	unsigned long	rx_missed_errors;	/* receiver missed packet	*/

	/* detailed tx_errors */
	unsigned long	tx_aborted_errors;
	unsigned long	tx_carrier_errors;
	unsigned long	tx_fifo_errors;
	unsigned long	tx_heartbeat_errors;
	unsigned long	tx_window_errors;
	
	/* for cslip etc */
	unsigned long	rx_compressed;
	unsigned long	tx_compressed;
};

/* The buffer descriptors track the ring buffers.  */
struct oeth_private {
  
  cyg_uint32 tx_next;			/* Next buffer to be sent */
  cyg_uint32 tx_last;			/* Next buffer to be checked if packet sent */
  cyg_uint32 tx_full;			/* Buffer ring fuul indicator */
  cyg_uint32 rx_cur;				/* Next buffer to be checked if packet received */
  
  oeth_regs *regs;			/* Address of controller registers. */
  oeth_bd *rx_bd_base;		        /* Address of Rx BDs. */
  oeth_bd *tx_bd_base;		        /* Address of Tx BDs. */
  cyg_uint32 irq;
  
  cyg_uint32 tx_keys[OETH_TXBD_NUM];	/* remember tx keys */
  
  
  struct net_device_stats stats;
};

// ------------------------------------------------------------------------
//
//                       STATISTICAL COUNTER STRUCTURE
//
// ------------------------------------------------------------------------
#ifdef KEEP_STATISTICS

typedef struct {
    cyg_uint32 interrupts;
    cyg_uint32 rx_count;
    cyg_uint32 rx_deliver;
    cyg_uint32 rx_resource;
    cyg_uint32 rx_restart;
    cyg_uint32 tx_count;
    cyg_uint32 tx_complete;
    cyg_uint32 tx_dropped;
} STATISTICS;

extern STATISTICS statistics[CYGNUM_DEVS_ETH_OPENCORES_ETHERMAC_DEV_COUNT];

#endif // KEEP_STATISTICS

// ------------------------------------------------------------------------
//
//                      DEVICES AND PACKET QUEUES
//
// ------------------------------------------------------------------------
// The system seems to work OK with as few as 8 of RX and TX descriptors.
// It limps very painfully with only 4.
// Performance is better with more than 8.
// But the size of non-cached (so useless for anything else)
// memory window is 1Mb, so we might as well use it all.
//
// 128 for these uses the whole 1Mb, near enough.

typedef struct _oeth_info {
  struct oeth_private cep;
  
  cyg_uint8 found:1;                        // was hardware discovered?
  cyg_uint8 mac_addr_ok:1;                  // can we bring up?
  cyg_uint8 active:1;                       // has this if been brung up?

  cyg_uint8 multicast_all:1;                  
  cyg_uint8 promisc:1;
  
  
  cyg_uint8 dev_addr[6];
  struct eth_drv_sc *sc;
  cyg_uint32 idx;
  
  // Interrupt handling stuff
  cyg_vector_t    vector;             // interrupt vector
  cyg_handle_t    interrupt_handle;   // handle for int.handler
  cyg_interrupt   interrupt_object;

#ifdef KEEP_STATISTICS
    void *p_statistics;                 // pointer to statistical counters
#endif
  
} oeth_info;



#endif 

/* EOF oeth_info.h */

