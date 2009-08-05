#ifndef CYGONCE_DEVS_ETH_NIOS2_TSE_TSE_H
#define CYGONCE_DEVS_ETH_NIOS2_TSE_TSE_H
//==========================================================================
//
//      tse.h
//
//      NIOS2 TSE Ethernet IP
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2003 Nick Garnett 
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
//####BSDCOPYRIGHTBEGIN####
//
// -------------------------------------------
//
// Portions of this software may have been derived from OpenBSD or other sources,
// and are covered by the appropriate copyright disclaimers included herein.
//
// -------------------------------------------
//
//####BSDCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    jskov
// Contributors: jskov, hmt, jco, nickg
// Date:         2001-01-22
// Purpose:      Hardware description of Altera Nios-II Triple Speed Ethernet IP Core.
// Description:  
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <cyg/hal/hal_io.h>
#include <cyg/hal/hal_endian.h>

#include <cyg/io/triple_speed_ethernet_regs.h>
#include <cyg/hal/sopc/altera_avalon_sgdma.h>

/* System Constant Definition Used in the TSE Driver Code */
#define ALTERA_TSE_SW_RESET_TIME_OUT_CNT		10000
#define ALTERA_TSE_SGDMA_BUSY_TIME_OUT_CNT		1000000	
#define ALTERA_TSE_FIRST_TX_SGDMA_DESC_OFST		2
#define ALTERA_TSE_SECOND_TX_SGDMA_DESC_OFST    3
#define ALTERA_TSE_FIRST_RX_SGDMA_DESC_OFST		0
#define ALTERA_TSE_SECOND_RX_SGDMA_DESC_OFST    1
#define ALTERA_TSE_QOS_TIMER_OPTION				0
#define ALTERA_TSE_MAC_MAX_FRAME_LENGTH			1518
#define ALTERA_TSE_PKT_INIT_LEN					1528

#if	ALTERA_TSE_QOS_TIMER_OPTION	
#define ALARMTICKS(x) ((alt_ticks_per_second()*(x))/1000000)
#define	ALTERA_TSE_QOS_TIMER_PERIOD_IN_US		50000
#define TIME_OUT                                0x3E80
#endif 

#define ALTERA_TSE_SGDMA_RX_DESC_CHAIN_SIZE		16

//#define ALTERA_TSE_IRQ_R                        fake_tse_isr
#if ALTERA_TSE_SGDMA_RX_DESC_CHAIN_SIZE > 1
	#define ALTERA_TSE_SGDMA_INTR_MASK              ALTERA_AVALON_SGDMA_CONTROL_IE_DESC_COMPLETED_MSK | ALTERA_AVALON_SGDMA_CONTROL_IE_CHAIN_COMPLETED_MSK | ALTERA_AVALON_SGDMA_CONTROL_IE_GLOBAL_MSK
#else
	#define ALTERA_TSE_SGDMA_INTR_MASK              ALTERA_AVALON_SGDMA_CONTROL_IE_CHAIN_COMPLETED_MSK | ALTERA_AVALON_SGDMA_CONTROL_IE_GLOBAL_MSK
#endif
                 
#define ALTERA_TSE_ADMIN_STATUS_DOWN			2
#define ALTERA_TSE_ADMIN_STATUS_UP				1
#define ALTERA_TSE_MAX_MTU_SIZE					1514
#define ALTERA_TSE_MIN_MTU_SIZE					14
#define ALTERA_TSE_HAL_ADDR_LEN					6

#define ALTERA_TSE_DUPLEX_MODE_DEFAULT			1
#define ALTERA_TSE_MAC_SPEED_DEFAULT			0
#define ALTERA_AUTONEG_TIMEOUT_THRESHOLD		7000000
#define ALTERA_NOMDIO_TIMEOUT_THRESHOLD 		1000000
#define ALTERA_DISGIGA_TIMEOUT_THRESHOLD		5000000 		

#define NTL848PHY_ID	0x20005c90  /* National 83848, 10/100 */
#define MTIPPCS_ID		0x00010000  /* MTIP 1000 Base-X PCS */
#define TDKPHY_ID		0x0300e540  /* TDK 78Q2120 10/100 */
#define NTLPHY_ID       0x20005c7a  /* National DP83865 */
#define MVLPHY_ID		0x0141		/* Marvell 88E1111 */

/*** Debug Definition *********/

// change ENABLE_PHY_LOOPBACK to 1 to enable PHY loopback for debug purpose 
#define ENABLE_PHY_LOOPBACK		0

#ifdef KEEP_STATISTICS
struct nios2_tse_stats {
    unsigned int tx_good             ;
    unsigned int tx_max_collisions   ;
    unsigned int tx_late_collisions  ;
    unsigned int tx_underrun         ;
    unsigned int tx_carrier_loss     ;
    unsigned int tx_deferred         ;
    unsigned int tx_sqetesterrors    ;
    unsigned int tx_single_collisions;
    unsigned int tx_mult_collisions  ;
    unsigned int tx_total_collisions ;
    unsigned int rx_good             ;
    unsigned int rx_crc_errors       ;
    unsigned int rx_align_errors     ;
    unsigned int rx_resource_errors  ;
    unsigned int rx_overrun_errors   ;
    unsigned int rx_collisions       ;
    unsigned int rx_short_frames     ;
    unsigned int rx_too_long_frames  ;
    unsigned int rx_symbol_errors    ;
    unsigned int interrupts          ;
    unsigned int rx_count            ;
    unsigned int rx_deliver          ;
    unsigned int rx_resource         ;
    unsigned int rx_restart          ;
    unsigned int tx_count            ;
    unsigned int tx_complete         ;
    unsigned int tx_dropped          ;
};
#endif

#define TX_DESC_NUM 8
#define RX_DESC_NUM 8

struct tse_priv_data;
typedef cyg_bool (*provide_esa_t)(struct tse_priv_data* cpd);

#ifdef PACKET_MEMORY_BASE
#define BUFFER_NO (((PACKET_MEMORY_SIZE_VALUE / (2 *(( 1528 + 16) / 4 + 2))) < (DESCRIPTOR_MEMORY_SIZE_VALUE / (2 * sizeof(alt_sgdma_descriptor)))) ?
	(PACKET_MEMORY_SIZE_VALUE / (2 *(( 1528 + 16) / 4 + 2))) : (DESCRIPTOR_MEMORY_SIZE_VALUE / (2 * sizeof(alt_sgdma_descriptor))))
#else //PACKET_MEMORY_BASE
#define BUFFER_NO  (DESCRIPTOR_MEMORY_SIZE_VALUE / (2 * sizeof(alt_sgdma_descriptor)))
#endif

typedef struct tse_priv_data {
//	volatile cyg_uint32      rx_buffer[( 1528 + 16) / 4 + 2]; 	//keep it first so it's aligned at the DCACHE line size
	volatile cyg_uint32      *rx_buffer;
	volatile cyg_uint32      *tx_buffer;
    int 			txbusy;             				// A packet has been sent

    unsigned long 	txkey[BUFFER_NO];					// Used to ack when packet sent
    unsigned short* base;               				// Base I/O address of controller (as it comes out of reset)
    int interrupt;                      				// Interrupt vector used by controller
    unsigned char 	enaddr[6];         					// Controller ESA (MAC ADDRESS)
    void (*config_enaddr)(struct tse_priv_data* cpd);	// function to configure ESA
    provide_esa_t 	provide_esa;
    bool 			hardwired_esa;
    int 			txpacket;
    int 			rxpacket;
    int 			within_send;
    int 			addrsh;                         	// Address bits to shift
    cyg_uint32 		data_buf;
    int   			data_pos;
	alt_sgdma_dev   tx_sgdma;
	alt_sgdma_dev   rx_sgdma;
	cyg_uint32      cfgflags;  // flags or'ed during initialization of COMMAND_CONFIG

	cyg_uint32      rx_buffer_uncached;
	cyg_int32       bytesReceived;

#ifdef KEEP_STATISTICS
    struct nios2_tse_stats stats;
#endif
    cyg_uint32 speed;
} tse_priv_data __attribute__ ((aligned (NIOS2_DCACHE_LINE_SIZE)));

// ------------------------------------------------------------------------

#include CYGDAT_DEVS_ETH_NIOS2_TSE_INL


// ------------------------------------------------------------------------
#endif // CYGONCE_DEVS_ETH_NIOS2_TSE_TSE_H
// EOF nios2_tse.h
