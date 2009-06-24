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

#define ALTERA_TSE_SGDMA_RX_DESC_CHAIN_SIZE		1

//#define ALTERA_TSE_IRQ_R                        fake_tse_isr
#define ALTERA_TSE_SGDMA_INTR_MASK              ALTERA_AVALON_SGDMA_CONTROL_IE_CHAIN_COMPLETED_MSK | ALTERA_AVALON_SGDMA_CONTROL_IE_GLOBAL_MSK
                 
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


#ifndef pnull
#define pnull ((void *)0)
#endif



// ------------------------------------------------------------------------

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

struct tse_priv_data;
typedef cyg_bool (*provide_esa_t)(struct tse_priv_data* cpd);

typedef struct tse_priv_data {
    int 			txbusy;             				// A packet has been sent
    unsigned long 	txkey;              				// Used to ack when packet sent
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
	cyg_uint32     *rx_sgdma_desc_ram;
	cyg_uint32      cfgflags;  // flags or'ed during initialization of COMMAND_CONFIG
	cyg_uint32      rx_buffer[( 1528 + 16) / 4];
	cyg_int32       bytesReceived;

#ifdef KEEP_STATISTICS
    struct nios2_tse_stats stats;
#endif
} tse_priv_data;

// ------------------------------------------------------------------------

#include CYGDAT_DEVS_ETH_NIOS2_TSE_INL

//#ifdef LAN91CXX_32BIT_RX
//typedef cyg_uint32 rxd_t;
//#else
//typedef cyg_uint16 rxd_t;
//#endif

//#ifndef SMSC_PLATFORM_DEFINED_GET_REG
//static __inline__ unsigned short
//get_reg(struct eth_drv_sc *sc, int regno)
//{
//    struct tse_priv_data *cpd =
//        (struct tse_priv_data *)sc->driver_private;
//    unsigned short val;
//    
//    HAL_WRITE_UINT16(cpd->base+(LAN91CXX_BS << cpd->addrsh), CYG_CPU_TO_LE16(regno>>3));
//    HAL_READ_UINT16(cpd->base+((regno&0x7) << cpd->addrsh), val);
//    val = CYG_LE16_TO_CPU(val);
//
//#if DEBUG & 2
//    diag_printf("read reg %d val 0x%04x\n", regno, val);
//#endif
//    return val;
//}
//#endif // SMSC_PLATFORM_DEFINED_GET_REG

#ifndef SMSC_PLATFORM_DEFINED_PUT_REG
static __inline__ void
put_reg(struct eth_drv_sc *sc, int regno, unsigned short val)
{
    struct tse_priv_data *cpd =
        (struct tse_priv_data *)sc->driver_private;
	
//    HAL_WRITE_UINT16(cpd->base+(LAN91CXX_BS << cpd->addrsh), CYG_CPU_TO_LE16(regno>>3));
//    HAL_WRITE_UINT16(cpd->base+((regno&0x7) << cpd->addrsh), CYG_CPU_TO_LE16(val));

#if DEBUG & 2
    diag_printf("write reg %d val 0x%04x\n", regno, val); 
#endif
}
#endif // SMSC_PLATFORM_DEFINED_PUT_REG

#ifndef SMSC_PLATFORM_DEFINED_PUT_DATA
// ------------------------------------------------------------------------
// Assumes bank2 has been selected
static __inline__ void
put_data(struct eth_drv_sc *sc, unsigned short val)
{
    struct tse_priv_data *cpd =
        (struct tse_priv_data *)sc->driver_private;
	
//    HAL_WRITE_UINT16(cpd->base+((LAN91CXX_DATA & 0x7) << cpd->addrsh), val);

#if DEBUG & 2
    diag_printf("write data 0x%04x\n", val);
#endif
}
#endif // SMSC_PLATFORM_DEFINED_PUT_DATA

#ifndef SMSC_PLATFORM_DEFINED_GET_DATA
// Assumes bank2 has been selected
//static __inline__ rxd_t
static __inline__ int
get_data(struct eth_drv_sc *sc)
{
//    rxd_t val;
    struct tse_priv_data *cpd =  (struct tse_priv_data *)sc->driver_private;
	
#ifdef LAN91CXX_32BIT_RX
//    HAL_READ_UINT32(cpd->base+((LAN91CXX_DATA_HIGH & 0x7) << cpd->addrsh), val);
#else
//    HAL_READ_UINT16(cpd->base+((LAN91CXX_DATA & 0x7) << cpd->addrsh), val);
#endif

#if DEBUG & 2
//    diag_printf("read data 0x%x\n", val);
#endif
//    return val;
	return 0;
}
#endif // SMSC_PLATFORM_DEFINED_GET_DATA

// ------------------------------------------------------------------------
// Read the bank register (this one is bank-independent)
#ifndef SMSC_PLATFORM_DEFINED_GET_BANKSEL
static __inline__ unsigned short
get_banksel(struct eth_drv_sc *sc)
{
    struct tse_priv_data *cpd = (struct tse_priv_data *)sc->driver_private;
    unsigned short val;
    
//    HAL_READ_UINT16(cpd->base+(LAN91CXX_BS << cpd->addrsh), val);
    val = CYG_LE16_TO_CPU(val);
#if DEBUG & 2
    diag_printf("read bank val 0x%04x\n", val);
#endif
    return val;
}
#endif



// ------------------------------------------------------------------------
#endif // CYGONCE_DEVS_ETH_NIOS2_LAN91CXX_LAN91CXX_H
// EOF smsc_tse.h
