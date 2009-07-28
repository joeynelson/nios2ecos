//==========================================================================
//
//      altera_avalon_tse.c
//
//      Ethernet device driver for ALTERA Triple Speed Ethenet controller
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2003 Nick Garnett 
// Copyright (C) 2004 Andrew Lunn
// Copyright (C) 2004 eCosCentric Ltd.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
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
// Author(s):    hmt, based on lan900 (for LAN91C110) driver by jskov
//               jskov, based on CS8900 driver by Gary Thomas
// Contributors: gthomas, jskov, hmt, jco@ict.es, nickg
// Date:         2001-01-22
// Purpose:      
// Description:  hardware driver for LAN91CXX "LAN9000" ethernet
// Notes:        Pointer register is not saved/restored on receive interrupts.
//               The pointer is shared by both receive/transmit code.
//               But the net stack manages atomicity for you here.
//
//               The controller has an autorelease mode that allows TX packets
//               to be freed automatically on successful transmission - but
//               that is not used since we're only sending one packet at a
//               time anyway.
//               We may want to pingpong in future for throughput reasons.
//
//               <jco@ict.es> Added support for PCMCIA mode and shifted
//               address buses.
//
//####DESCRIPTIONEND####
//
//==========================================================================

// Based on LAN91C110 and LAN91C96

#include <pkgconf/system.h>
#include <pkgconf/devs_eth_nios2_tse.h>
#include <pkgconf/io_eth_drivers.h>

#include <cyg/infra/cyg_type.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/hal/hal_diag.h>
#include <cyg/hal/io.h>
#include <cyg/infra/cyg_ass.h>
#include <cyg/infra/diag.h>
#include <cyg/hal/drv_api.h>
#include <cyg/io/eth/netdev.h>
#include <cyg/io/eth/eth_drv.h>

#include <cyg/hal/hal_cache.h>

#ifdef CYGPKG_NET
#include <pkgconf/net.h>
#include <cyg/kernel/kapi.h>
#include <net/if.h>  /* Needed for struct ifnet */
#endif

#ifdef CYGPKG_INFRA_DEBUG
// Then we log, OOI, the number of times we get a bad packet number
// from the tx done fifo.
int tse_txfifo_good = 0;
int tse_txfifo_bad = 0;
#endif

#include <cyg/io/nios2_tse.h>

// Set to perms of:
// 0 disables all debug output
// 1 for process debug output
// 2 for added data IO output: get_reg, put_reg
// 4 for packet allocation/free output
// 8 for only startup status, so we can tell we're installed OK
#define DEBUG 0
cyg_uint32 getPHYSpeed(np_tse_mac *pmac);
cyg_uint32 marvell_cfg_gmii(np_tse_mac *pmac);

#if DEBUG
#if defined(CYGPKG_REDBOOT)
static void db_printf( char *fmt, ... )
{
	extern int start_console(void);
	extern void end_console(int);
	va_list a;
	int old_console;
	va_start( a, fmt );
	old_console = start_console();
	diag_vprintf( fmt, a );
	end_console(old_console);
	va_end( a );
}
#else
#if 0
static void db_printf( char *fmt, ... )
{
	va_list a;
	int old_console = CYGACC_CALL_IF_SET_CONSOLE_COMM(CYGNUM_CALL_IF_SET_COMM_ID_QUERY_CURRENT);
	va_start( a, fmt );
	CYGACC_CALL_IF_SET_CONSOLE_COMM( 0 );
	diag_vprintf( fmt, a );
	CYGACC_CALL_IF_SET_CONSOLE_COMM(old_console);
	va_end( a );
}
#else
#define db_printf diag_printf
#endif
#endif
#else
#if 0
static void db_printf( char *fmt, ... )
{
	va_list a;
	int old_console = CYGACC_CALL_IF_SET_CONSOLE_COMM(CYGNUM_CALL_IF_SET_COMM_ID_QUERY_CURRENT);
	va_start( a, fmt );
	CYGACC_CALL_IF_SET_CONSOLE_COMM( 0 );
	diag_vprintf( fmt, a );
	CYGACC_CALL_IF_SET_CONSOLE_COMM(old_console);
	va_end( a );
}
#else
#define db_printf( fmt, ... )
#endif
#endif

#if DEBUG & 1
#define DEBUG_FUNCTION() do { db_printf("%s\n", __FUNCTION__); } while (0)
#else
#define DEBUG_FUNCTION() do {} while(0)
#endif

#if defined(ETH_DRV_GET_IF_STATS) || defined (ETH_DRV_GET_IF_STATS_UD)
#define KEEP_STATISTICS
#endif

#ifdef KEEP_STATISTICS
#define INCR_STAT( _x_ )        (cpd->stats. _x_ ++)
#else
#define INCR_STAT( _x_ )        CYG_EMPTY_STATEMENT
#endif

#include <cyg/io/nios2_tse.h>

#ifdef LAN91CXX_IS_TSE
static void tse_write_phy(struct eth_drv_sc *sc, cyg_uint8 phyaddr,
		cyg_uint8 phyreg, cyg_uint16 value);
static cyg_uint16 tse_read_phy(struct eth_drv_sc *sc, cyg_uint8 phyaddr,
		cyg_uint8 phyreg);
#endif

static void tse_poll(struct eth_drv_sc *sc);

#ifndef CYGPKG_IO_ETH_DRIVERS_STAND_ALONE

static cyg_interrupt tse_interrupt;
static cyg_handle_t tse_interrupt_handle;

// This ISR is called when the ethernet interrupt occurs
static int tse_txd_isr(cyg_vector_t vector, cyg_addrword_t data)
/* , HAL_SavedRegisters *regs */
{
	struct eth_drv_sc *sc = (struct eth_drv_sc *) data;
	struct tse_priv_data *cpd = (struct tse_priv_data *) sc->driver_private;

	//   DEBUG_FUNCTION();

	//    INCR_STAT( interrupts );

	cyg_drv_interrupt_mask(cpd->tx_sgdma.irq);
	alt_avalon_sgdma_set_control(&cpd->tx_sgdma, 0x80000000);
	alt_avalon_sgdma_set_control(&cpd->tx_sgdma, 0x0);
	cyg_drv_interrupt_acknowledge(cpd->tx_sgdma.irq);

	return (CYG_ISR_HANDLED); // Run the DSR
}

// This ISR is called when the ethernet interrupt occurs
static int tse_rxd_isr(cyg_vector_t vector, cyg_addrword_t data)
/* , HAL_SavedRegisters *regs */
{
	struct eth_drv_sc *sc = (struct eth_drv_sc *) data;
	struct tse_priv_data *cpd = (struct tse_priv_data *) sc->driver_private;

//	DEBUG_FUNCTION();

	//    INCR_STAT( interrupts );

	cyg_drv_interrupt_mask(cpd->rx_sgdma.irq);

	alt_avalon_sgdma_set_control(&cpd->rx_sgdma, ALTERA_AVALON_SGDMA_CONTROL_CLEAR_INTERRUPT_MSK);
	alt_avalon_sgdma_set_control(&cpd->rx_sgdma, 0x0);
	alt_avalon_sgdma_status(&cpd->rx_sgdma);

	cyg_drv_interrupt_acknowledge(cpd->rx_sgdma.irq);

	return (CYG_ISR_HANDLED | CYG_ISR_CALL_DSR); // Run the DSR
}

#endif

// The deliver function (ex-DSR)  handles the ethernet [logical] processing
static void tse_deliver(struct eth_drv_sc *sc)
{
	struct tse_priv_data *cpd = (struct tse_priv_data *) sc->driver_private;

//	DEBUG_FUNCTION();

	cpd->bytesReceived = alt_avalon_sgdma_descpt_bytes_xfered(
			(alt_sgdma_descriptor *) cpd->rx_sgdma.descriptor_base) - 2;

//	diag_printf("Bytes Rxd: %d\n", cpd->bytesReceived);

	if (cpd->bytesReceived > 0)
	{
		(sc->funs->eth_drv->recv)(sc, cpd->bytesReceived);
	}

	// Allow interrupts to happen again
	cyg_drv_interrupt_unmask(cpd->rx_sgdma.irq);
}

static int tse_int_vector(struct eth_drv_sc *sc)
{
	struct tse_priv_data *cpd = (struct tse_priv_data *) sc->driver_private;

	return (cpd->rx_sgdma.irq);
}

static bool tse_init(struct cyg_netdevtab_entry *tab)
{
	struct eth_drv_sc *sc = (struct eth_drv_sc *) tab->device_instance;
	struct tse_priv_data *cpd = (struct tse_priv_data *) sc->driver_private;

	unsigned short val;
	int cc;
	cyg_bool esa_configured = false;
	alt_sgdma_descriptor *desc_pointer;

	DEBUG_FUNCTION();

	cpd->txbusy = cpd->within_send = 0;

	int dat;
	cyg_uint32 result;
	int is1000, duplex;
	int status = 0;
	int i;

	/* Get the Rx and Tx SGDMA addresses */
	cpd->rx_sgdma.base = SGDMA_RX_BASE;
	cpd->rx_sgdma.irq = SGDMA_RX_IRQ;
	cpd->rx_sgdma.descriptor_base
			= &((alt_sgdma_descriptor *) DESCRIPTOR_MEMORY_BASE)[ALTERA_TSE_FIRST_RX_SGDMA_DESC_OFST];
	cpd->rx_sgdma.isr = (cyg_ISR_t *) tse_rxd_isr; // isr
	cpd->rx_sgdma.dsr = (cyg_DSR_t *) eth_drv_dsr; // The logical driver DSR
	cpd->rx_sgdma.isr_data = (cyg_addrword_t) sc; // data item passed to interrupt handler
	cpd->rx_sgdma.chain_control = 0;

	cpd->tx_sgdma.base = SGDMA_TX_BASE;
	cpd->tx_sgdma.irq = SGDMA_TX_IRQ;
	cpd->tx_sgdma.descriptor_base
			= &((alt_sgdma_descriptor *) DESCRIPTOR_MEMORY_BASE)[ALTERA_TSE_FIRST_TX_SGDMA_DESC_OFST];
	cpd->tx_sgdma.isr = (cyg_ISR_t *) NULL; // tse_txd_isr	   ; // isr
	cpd->tx_sgdma.dsr = (cyg_DSR_t *) NULL;
	cpd->tx_sgdma.isr_data = (cyg_addrword_t) sc; // data item passed to interrupt handler
	cpd->tx_sgdma.chain_control = 0;

	cpd->txkey = 0;

	//Clearing SGDMA desc Memory - this clears 1024 KB of descriptor space
	for (i = 0; i < 256; i++)
	{
		IOWR( DESCRIPTOR_MEMORY_BASE, i, 0);
	}

//	printf("SGDMA desc memory cleared \n");

	// reset the rx sgdma   
	alt_avalon_sgdma_init2(&cpd->rx_sgdma);

	// reset the tx sgdma
	alt_avalon_sgdma_init2(&cpd->tx_sgdma);

	marvell_cfg_gmii(cpd->base);

	/* reset the PHY if necessary */
	result = getPHYSpeed(cpd->base);
	is1000 = (result >> 1) & 0x01;
	duplex = result & 0x01;

	diag_printf(
			"[triple_speed_ethernet_init] Speed is %s\t Duplex is %s\n",
			is1000 ? "1000 Mb/s" : "100 Mb/s", duplex ? "full" : "half");


	cpd->speed = result;

	// setup the mac address
	if (cpd->config_enaddr)
	{
		(*cpd->config_enaddr)(cpd);
	}

#if 0 < CYGINT_DEVS_ETH_SOCP_ALT_AVALON_TSE_STATIC_ESA
	// Use statically configured ESA from the private data
#if DEBUG & 9
	db_printf("TSE - static ESA: %02x:%02x:%02x:%02x:%02x:%02x\n", cpd->enaddr[0], cpd->enaddr[1], cpd->enaddr[2], cpd->enaddr[3], cpd->enaddr[4], cpd->enaddr[5] );
#endif // DEBUG
	IOWR_ALTERA_TSEMAC_MAC_0( cpd->base, (int)cpd->enaddr[0] | (int)( cpd->enaddr[1] << 8 ) | (int)( cpd->enaddr[2] << 16) | (int)( cpd->enaddr[3] << 24));
	IOWR_ALTERA_TSEMAC_MAC_1( cpd->base, ((int)cpd->enaddr[4] | (int)( cpd->enaddr[5] << 8 ) & 0xffff));

	//If your system does not require the use of multiple addresses, Altera recommends
	//that you configure all supplemental addresses to the primary MAC address.
	IOWR_ALTERA_TSEMAC_SMAC_0_0( cpd->base, ((int)cpd->enaddr[0] | (int)( cpd->enaddr[1] << 8 ) | (int)( cpd->enaddr[2] << 16) | (int)( cpd->enaddr[3] << 24)));
	IOWR_ALTERA_TSEMAC_SMAC_0_1( cpd->base, ((int)cpd->enaddr[4] | (int)( cpd->enaddr[5] << 8 ) & 0xffff));
	IOWR_ALTERA_TSEMAC_SMAC_1_0( cpd->base, ((int)cpd->enaddr[0] | (int)( cpd->enaddr[1] << 8 ) | (int)( cpd->enaddr[2] << 16) | (int)( cpd->enaddr[3] << 24)));
	IOWR_ALTERA_TSEMAC_SMAC_1_1( cpd->base, ((int)cpd->enaddr[4] | (int)( cpd->enaddr[5] << 8 ) & 0xffff));
	IOWR_ALTERA_TSEMAC_SMAC_2_0( cpd->base, ((int)cpd->enaddr[0] | (int)( cpd->enaddr[1] << 8 ) | (int)( cpd->enaddr[2] << 16) | (int)( cpd->enaddr[3] << 24)));
	IOWR_ALTERA_TSEMAC_SMAC_2_1( cpd->base, ((int)cpd->enaddr[4] | (int)( cpd->enaddr[5] << 8 ) & 0xffff));
	IOWR_ALTERA_TSEMAC_SMAC_3_0( cpd->base, ((int)cpd->enaddr[0] | (int)( cpd->enaddr[1] << 8 ) | (int)( cpd->enaddr[2] << 16) | (int)( cpd->enaddr[3] << 24)));
	IOWR_ALTERA_TSEMAC_SMAC_3_1( cpd->base, ((int)cpd->enaddr[4] | (int)( cpd->enaddr[5] << 8 ) & 0xffff));

#else // not CYGINT_DEVS_ETH_SMSC_LAN91CXX_STATIC_ESA
	//some other way
	IOWR_ALTERA_TSEMAC_MAC_0( cpd->base, ((int)cpd->enaddr[0] | (int)( cpd->enaddr[1] << 8 ) | (int)( cpd->enaddr[2] << 16) | (int)( cpd->enaddr[3] << 24)));
	IOWR_ALTERA_TSEMAC_MAC_1( cpd->base, ((int)cpd->enaddr[4] | (int)( cpd->enaddr[5] << 8 ) & 0xffff));

	//If your system does not require the use of multiple addresses, Altera recommends
	//that you configure all supplemental addresses to the primary MAC address.
	IOWR_ALTERA_TSEMAC_SMAC_0_0( cpd->base, ((int)cpd->enaddr[0] | (int)( cpd->enaddr[1] << 8 ) | (int)( cpd->enaddr[2] << 16) | (int)( cpd->enaddr[3] << 24)));
	IOWR_ALTERA_TSEMAC_SMAC_0_1( cpd->base, ((int)cpd->enaddr[4] | (int)( cpd->enaddr[5] << 8 ) & 0xffff));
	IOWR_ALTERA_TSEMAC_SMAC_1_0( cpd->base, ((int)cpd->enaddr[0] | (int)( cpd->enaddr[1] << 8 ) | (int)( cpd->enaddr[2] << 16) | (int)( cpd->enaddr[3] << 24)));
	IOWR_ALTERA_TSEMAC_SMAC_1_1( cpd->base, ((int)cpd->enaddr[4] | (int)( cpd->enaddr[5] << 8 ) & 0xffff));
	IOWR_ALTERA_TSEMAC_SMAC_2_0( cpd->base, ((int)cpd->enaddr[0] | (int)( cpd->enaddr[1] << 8 ) | (int)( cpd->enaddr[2] << 16) | (int)( cpd->enaddr[3] << 24)));
	IOWR_ALTERA_TSEMAC_SMAC_2_1( cpd->base, ((int)cpd->enaddr[4] | (int)( cpd->enaddr[5] << 8 ) & 0xffff));
	IOWR_ALTERA_TSEMAC_SMAC_3_0( cpd->base, ((int)cpd->enaddr[0] | (int)( cpd->enaddr[1] << 8 ) | (int)( cpd->enaddr[2] << 16) | (int)( cpd->enaddr[3] << 24)));
	IOWR_ALTERA_TSEMAC_SMAC_3_1( cpd->base, ((int)cpd->enaddr[4] | (int)( cpd->enaddr[5] << 8 ) & 0xffff));

#if DEBUG & 9
	db_printf("TSE - ESA: %02x:%02x:%02x:%02x:%02x:%02x\n", cpd->enaddr[0],
			cpd->enaddr[1], cpd->enaddr[2], cpd->enaddr[3], cpd->enaddr[4],
			cpd->enaddr[5]);
#endif // DEBUG
#endif // !CYGINT_DEVS_ETH_SOPC_ALT_AVALON_TSE_STATIC_ESA

	/* reset the mac */
	IOWR_ALTERA_TSEMAC_CMD_CONFIG(cpd->base, ALTERA_TSEMAC_CMD_SW_RESET_MSK | ALTERA_TSEMAC_CMD_TX_ENA_MSK | ALTERA_TSEMAC_CMD_RX_ENA_MSK | ALTERA_TSEMAC_CMD_ETH_SPEED_MSK);

	// reset is complete when the sw reset bit is cleared by the MAC
	i = 0;
	while (IORD_ALTERA_TSEMAC_CMD_CONFIG(cpd->base)
			& ALTERA_TSEMAC_CMD_SW_RESET_MSK)
	{
		if (i++ > 10000)
		{
//			diag_printf("ERROR: Cannot reset MAC");
			return false;
		}
	}

	// read the command/config register and verify that the tx/rx/enable bits are cleared
	dat = IORD_ALTERA_TSEMAC_CMD_CONFIG(cpd->base);

	if ((dat & 0x03) != 0)
	{
//		db_printf("WARN: RX/TX not disabled after reset... missing PHY clock? CMD_CONFIG=0x%08x\n",	dat);
		return false;
	}
	else
	{
//		db_printf("OK, CMD_CONFIG=0x%08x\n", dat);
	}

	/* Initialize MAC registers */
	IOWR_ALTERA_TSEMAC_FRM_LENGTH( cpd->base, ALTERA_TSE_MAC_MAX_FRAME_LENGTH);
	IOWR_ALTERA_TSEMAC_RX_ALMOST_EMPTY( cpd->base, 8);
	IOWR_ALTERA_TSEMAC_RX_ALMOST_FULL( cpd->base, 8);
	IOWR_ALTERA_TSEMAC_TX_ALMOST_EMPTY( cpd->base, 8);
	IOWR_ALTERA_TSEMAC_TX_ALMOST_FULL( cpd->base, 3);
	IOWR_ALTERA_TSEMAC_TX_SECTION_EMPTY(cpd->base, TSE_MAC_TRANSMIT_FIFO_DEPTH - 16); //1024/4;
	IOWR_ALTERA_TSEMAC_TX_SECTION_FULL( cpd->base, 0); //32/4; // start transmit when there are 48 bytes
	IOWR_ALTERA_TSEMAC_RX_SECTION_EMPTY(cpd->base, TSE_MAC_RECEIVE_FIFO_DEPTH - 16); //4000/4);
	IOWR_ALTERA_TSEMAC_RX_SECTION_FULL( cpd->base, 0);

//	IOWR_ALTERA_TSEMAC_RX_CMD_STAT(     cpd->base, ALTERA_TSEMAC_RX_CMD_STAT_RXSHIFT16_MSK);
//	IOWR_ALTERA_TSEMAC_TX_CMD_STAT(     cpd->base, ALTERA_TSEMAC_TX_CMD_STAT_TXSHIFT16_MSK);

	// Initialize upper level driver
	(sc->funs->eth_drv->init)(sc, cpd->enaddr);

	return true;
}

static void tse_stop(struct eth_drv_sc *sc)
{
	struct tse_priv_data *cpd = (struct tse_priv_data *) sc->driver_private;

	int state;

	DEBUG_FUNCTION();

	/* disable the interrupt for the rx*/
	cyg_drv_interrupt_mask(cpd->rx_sgdma.irq);

	alt_avalon_sgdma_reset(&(cpd->rx_sgdma));
	alt_avalon_sgdma_reset(&(cpd->tx_sgdma));

	/* disable the interrupt for the tx*/
	    cyg_drv_interrupt_mask(cpd->tx_sgdma.irq);

//		cyg_interrupt_detach( cpd->rx_sgdma.sgdma_interrupt_handle);
//	  	cyg_interrupt_detach( cpd->tx_sgdma.sgdma_interrupt_handle);

	cpd->txkey = 0;

	/* Disable Receive path on the device*/
	state = IORD_ALTERA_TSEMAC_CMD_CONFIG( cpd->base);

	IOWR_ALTERA_TSEMAC_CMD_CONFIG( cpd->base, state & ~(ALTERA_TSEMAC_CMD_RX_ENA_MSK | ALTERA_TSEMAC_CMD_TX_ENA_MSK));
}

//
// This function is called to "start up" the interface.  It may be called
// multiple times, even when the hardware is already running.  It will be
// called whenever something "hardware oriented" changes and should leave
// the hardware ready to send/receive packets.
//
static void tse_start(struct eth_drv_sc *sc, unsigned char *enaddr, int flags)
{
	struct tse_priv_data *cpd = (struct tse_priv_data *) sc->driver_private;
	cyg_uint32 dat, timeout, result;
	int is1000, duplex;

	DEBUG_FUNCTION();

	alt_sgdma_descriptor *desc_base = cpd->rx_sgdma.descriptor_base;

	/* enable MAC */
	dat = ALTERA_TSEMAC_CMD_TX_ENA_MSK | ALTERA_TSEMAC_CMD_RX_ENA_MSK |
#if ENABLE_PHY_LOOPBACK
			ALTERA_TSEMAC_CMD_PROMIS_EN_MSK | ALTERA_TSEMAC_CMD_LOOPBACK_MSK | // promiscuous mode
#endif
			ALTERA_TSEMAC_CMD_PAD_EN_MSK       |
//			ALTERA_TSEMAC_CMD_TX_ADDR_INS_MSK |
			ALTERA_TSEMAC_CMD_RX_ERR_DISC_MSK; /* automatically discard frames with CRC errors */

	is1000 = ((cpd->speed) >> 1) & 0x01;
	duplex = (cpd->speed) & 0x01;

	if (is1000)
	{
		dat |= ALTERA_TSEMAC_CMD_ETH_SPEED_MSK;
	}
	else
	{
		dat &= ~ALTERA_TSEMAC_CMD_ETH_SPEED_MSK;
	}

	if(duplex)
	{
		dat &= ~ALTERA_TSEMAC_CMD_HD_ENA_MSK;
	}
	else
	{
		dat |= ALTERA_TSEMAC_CMD_HD_ENA_MSK;
	}

	IOWR_ALTERA_TSEMAC_CMD_CONFIG(cpd->base, dat);
//	printf("\nMAC post-initialization: CMD_CONFIG=0x%08x\n", IORD_ALTERA_TSEMAC_CMD_CONFIG(cpd->base));

	/* enable the interrupt for the rx*/
	cyg_drv_interrupt_unmask(cpd->rx_sgdma.irq);

//	cyg_drv_interrupt_unmask(cpd->tx_sgdma.irq);

	// start the dma transfer
	// Make sure SGDMA controller is not busy from a former command
	timeout = 0;

	//  printf("\nWaiting while rx SGDMA is busy.........");
	//  	while ( (alt_avalon_sgdma_status( cpd->rx_sgdma.base) & ALTERA_AVALON_SGDMA_STATUS_BUSY_MSK) )
	//  	{
	//    	if(timeout++ == ALTERA_TSE_SGDMA_BUSY_TIME_OUT_CNT)
	//    	{
	//        	printf("TSE_START:Timeout");
	//        	return ;
	//		}
	//  	}
	// set the recieve descriptor
	alt_avalon_sgdma_construct_stream_to_mem_desc(desc_base, // descriptor
			desc_base + 1, // next descriptor
			cpd->rx_buffer, // starting write_address
			0, // read until EOP
			0); // don't write to constant address

	cpd->rx_sgdma.chain_control = 0x1a;
	alt_avalon_sgdma_set_control(&cpd->rx_sgdma, 0);

	//	do {

	// SGDMA operation invoked for RX (non-blocking call)
	result = alt_avalon_sgdma_do_async_transfer(&(cpd->rx_sgdma), desc_base);

	//	} while (( result != 0) && ( timeout++  < ALTERA_TSE_SGDMA_BUSY_TIME_OUT_CNT));

}

//
// This function is called for low level "control" operations
//
static int tse_control(struct eth_drv_sc *sc, unsigned long key,
		void *data, int length)
{
	int retVal = 0;
	struct tse_priv_data *cpd = (struct tse_priv_data *) sc->driver_private;


	DEBUG_FUNCTION();

	switch (key)
	{
		case ETH_DRV_SET_MAC_ADDRESS:
		{
			memcpy(cpd->enaddr, data, 6); //MAC address has 6 bytes.
			IOWR_ALTERA_TSEMAC_MAC_0( cpd->base, ((cyg_uint32)cpd->enaddr[0] | (cyg_uint32)( cpd->enaddr[1] << 8 ) | (cyg_uint32)( cpd->enaddr[2] << 16) | (cyg_uint32)( cpd->enaddr[3] << 24)) );
			IOWR_ALTERA_TSEMAC_MAC_1( cpd->base, ((cyg_uint32)cpd->enaddr[4] | (cyg_uint32)( cpd->enaddr[5] << 8 ) & 0xffff) );

			break;
		}
		case ETH_DRV_SET_MC_LIST:
		case ETH_DRV_SET_MC_ALL:
		{
			IOWR_ALTERA_TSEMAC_HASH_TABLE(cpd->base, 0, 0xffffffff);
			break;
		}
//		{
//			struct eth_drv_mc_list list = *((struct eth_drv_mc_list*)data);
//			cyg_uint32 dat;
//
//			if(list.len > 0)
//			{
//				//enable promiscuous mode
//				dat = IORD_ALTERA_TSEMAC_CMD_CONFIG(cpd->base);
//				dat |= ALTERA_TSEMAC_CMD_PROMIS_EN_MSK;
//				IOWR_ALTERA_TSEMAC_CMD_CONFIG(cpd->base, dat);
//			}
//			else
//			{
//				//disable promiscuous mode
//				dat = IORD_ALTERA_TSEMAC_CMD_CONFIG(cpd->base);
//				dat &= ~ALTERA_TSEMAC_CMD_PROMIS_EN_MSK;
//				IOWR_ALTERA_TSEMAC_CMD_CONFIG(cpd->base, dat);
//			}



		//DEBUG CODE
//			int i = 0;
//			diag_printf("addresses: \n");
//			for(; i<list.len; i++)
//			{
//				diag_printf("\t%02x:%02x:%02x:%02x:%02x:%02x\n",
//						list.addrs[i][0],
//						list.addrs[i][1],
//						list.addrs[i][2],
//						list.addrs[i][3],
//						list.addrs[i][4],
//						list.addrs[i][5]);
//			}
//END OF DEBUG CODE
//			break;
//		}

		default:
		{
			retVal = 1;
			break;
		}
	}

	return retVal;
}

static void tse_TxDone(struct eth_drv_sc *sc)
{
	struct tse_priv_data *cpd = (struct tse_priv_data *) sc->driver_private;
	alt_sgdma_descriptor *desc_base = cpd->tx_sgdma.descriptor_base;
	int i;
	int stat;

	stat = alt_avalon_sgdma_check_descriptor_status(desc_base);
	if(stat == 0)
	{
		(sc->funs->eth_drv->tx_done)( sc, cpd->txkey, 0 );
		cpd->txkey = 0;
	}

}

//
// This routine is called to see if it is possible to send another packet.
// It will return non-zero if a transmit is possible, zero otherwise.
//
static int tse_can_send(struct eth_drv_sc *sc)
{
	int retValue = 0;
	struct tse_priv_data *cpd = (struct tse_priv_data *) sc->driver_private;

	DEBUG_FUNCTION();

	alt_avalon_sgdma_status(&cpd->rx_sgdma);

	if((alt_avalon_sgdma_status(&cpd->tx_sgdma)	& ALTERA_AVALON_SGDMA_STATUS_BUSY_MSK) == 0)
	{
		retValue = 1;
	}

	return retValue;
}

//
// This routine is called to send data to the hardware.
static void tse_send(struct eth_drv_sc *sc, struct eth_drv_sg *sg_list,
		int sg_len, int total_len, unsigned long key)
{
	struct tse_priv_data *cpd = (struct tse_priv_data *) sc->driver_private;
	int i, len;
	static cyg_uint32 send_buffer[( 1528 + 16) / 4 + 1];
	cyg_uint8* mem = (cyg_uint8 *)( ((cyg_uint32)((cyg_uint8*)send_buffer)) | 0x80000000) + 2;
	alt_sgdma_descriptor *desc_base = cpd->tx_sgdma.descriptor_base;

	DEBUG_FUNCTION();

	if(cpd->txkey)
		(sc->funs->eth_drv->tx_done)( sc, cpd->txkey, 1 );

	// for all of the buf/len pairs in the scatter gather list
	len = 0;
	for (i = 0; i < sg_len; i++)
	{
		if(sg_list[i].buf == NULL || sg_list[i].len == 0)
			continue;

		memcpy(mem + len,  sg_list[i].buf, sg_list[i].len);
		len += sg_list[i].len;

	}
		/* Construct the descriptor */
		alt_avalon_sgdma_construct_mem_to_stream_desc(
				desc_base, /* Descriptor */
				desc_base+ 1, /* Next descriptor */
				send_buffer,
				len + 2,
				0, /* Don't read fixed addr */
				1, /* Generate SOP @ first desc */
				1, /* Generate EOP @ last desc */
				0 /* Streaming channel: N/A */
		);

	cpd->txkey = key;

	// Set up the SGDMA
	// Clear the status and control bits of the SGDMA descriptor
	IOWR_ALTERA_AVALON_SGDMA_CONTROL(cpd->tx_sgdma.base, 0);
	IOWR_ALTERA_AVALON_SGDMA_STATUS(cpd->tx_sgdma.base, 0xFF);

	// Start SGDMA (non-blocking call)
	alt_avalon_sgdma_do_async_transfer(&cpd->tx_sgdma, desc_base);

	/* perform cache save read to obtain actual bytes transferred for current sgdma descriptor */
	//actualBytesTransferred = IORD_ALTERA_TSE_SGDMA_DESC_ACTUAL_BYTES_TRANSFERRED( cpd->tx_sgdma.descriptor_base);
//	IOWR_ALTERA_TSEMAC_RX_CMD_STAT(     cpd->base, ALTERA_TSEMAC_RX_CMD_STAT_RXSHIFT16_MSK);
//	IOWR_ALTERA_TSEMAC_TX_CMD_STAT(     cpd->base, ALTERA_TSEMAC_TX_CMD_STAT_TXSHIFT16_MSK);
}

static void tse_TxEvent(struct eth_drv_sc *sc, int stat)
{
	DEBUG_FUNCTION();
#if 0
	unsigned short packet, ints, tcr;
	struct tse_priv_data *cpd =
	(struct tse_priv_data *)sc->driver_private;
	int success = 1;

	DEBUG_FUNCTION();

	INCR_STAT( tx_complete );

	// Ack and mask TX interrupt set
	ints = get_reg(sc, LAN91CXX_INTERRUPT) & 0xff00;
	ints |= LAN91CXX_INTERRUPT_TX_SET_ACK;
	ints &= ~LAN91CXX_INTERRUPT_TX_SET_M;
	put_reg(sc, LAN91CXX_INTERRUPT, ints);

	// Get number of completed packet and read the status word
	packet = get_reg(sc, LAN91CXX_FIFO_PORTS);
#if DEBUG & 1
	db_printf("%s:START: fifo %04x ints %04x\n", __FUNCTION__, packet, ints);
#endif

#ifdef KEEP_STATISTICS
	{
		unsigned short reg;

	}
#endif // KEEP_STATISTICS
	// We do not really care about Tx failure.  Ethernet is not a reliable
	// medium.  But we do care about the TX engine stopping.
	tcr = get_reg(sc, LAN91CXX_TCR);
	if ( 0 == (LAN91CXX_TCR_TXENA & tcr) )
	{
#if DEBUG & 1
		db_printf("%s: ENGINE RESTART: tcr 0x%04x eph 0x%04x ints 0x%04x\n", __FUNCTION__, tcr, get_reg(sc, LAN91CXX_EPH_STATUS), ints);
#endif
		tcr |= LAN91CXX_TCR_TXENA;
		put_reg(sc, LAN91CXX_TCR, tcr);
		success = 0; // And treat this as an error...
	}

	packet &= 0xff;

	// It certainly appears that occasionally the tx fifo tells lies; we
	// get the wrong packet number.  Freeing the one we allocated seems to
	// give correct operation.
#ifdef CYGPKG_INFRA_DEBUG
	// Then we log, OOI, the number of times we get a bad packet number
	// from the tx done fifo.
	if (cpd->txpacket != packet )
	tse_txfifo_bad++;
	else
	tse_txfifo_good++;
#endif
#if DEBUG & 4
	db_printf("#####Tx packet freed 0x%04x (expected 0x%04x)\n", packet, cpd->txpacket );
#endif
	// and then free the packet
	put_reg(sc, LAN91CXX_PNR, cpd->txpacket);
	put_reg(sc, LAN91CXX_MMU_COMMAND, LAN91CXX_MMU_rel_packet);

	// Ack the TX int which is supposed to clear the packet from the TX
	// completion queue.
	ints = get_reg(sc, LAN91CXX_INTERRUPT) & 0xff00;
	ints |= LAN91CXX_INTERRUPT_TX_FIFO_ACK;
	put_reg(sc, LAN91CXX_INTERRUPT, ints);

#if DEBUG & 1
	// Hm... The free doesn't seem to have the desired effect?!?
	ints = get_reg(sc, LAN91CXX_INTERRUPT);
	packet = get_reg(sc, LAN91CXX_FIFO_PORTS);
	db_printf("%s:END: fifo %04x ints %04x\n", __FUNCTION__, packet, ints);
#endif

	if ( cpd->txbusy )
	{
		cpd->txbusy = 0;
		(sc->funs->eth_drv->tx_done)(sc, cpd->txkey, success);
	}
#endif
}

//
// This function is called when a packet has been received.  Its job is
// to prepare to unload the packet from the hardware.  Once the length of
// the packet is known, the upper layer of the driver can be told.  When
// the upper layer is ready to unload the packet, the internal function
// 'tse_recv' will be called to actually fetch it from the hardware.
//
static void tse_RxEvent(struct eth_drv_sc *sc)
{
	struct tse_priv_data *cpd = (struct tse_priv_data *) sc->driver_private;
	unsigned short stat, len;

	DEBUG_FUNCTION();
#if 0
	stat = get_reg(sc, LAN91CXX_FIFO_PORTS);
#if DEBUG & 1
	db_printf("RxEvent - FIFOs: 0x%04x\n", stat);
#endif
	if ( 0x8000 & stat )
	{
		// Then the Rx FIFO is empty
#if DEBUG & 4
		db_printf("#####RxEvent with empty fifo\n");
#endif
		return;
	}

	INCR_STAT( rx_count );

#if DEBUG & 4
	db_printf("#####Rx packet allocated 0x%04x (previous 0x%04x)\n",
			0xff & (stat >> 8), cpd->rxpacket );
#endif
	// There is an Rx Packet ready
	cpd->rxpacket = 0xff & (stat >> 8);

	// Read status and (word) length
	put_reg(sc, LAN91CXX_POINTER, (LAN91CXX_POINTER_RCV | LAN91CXX_POINTER_READ |
					LAN91CXX_POINTER_AUTO_INCR | 0x0000));
	get_data_init(sc);

	stat = get_data_short(sc);
	len = get_data_short(sc);
	len = len - 6; // minus header/footer words

#ifdef KEEP_STATISTICS
	if ( stat & LAN91CXX_RX_STATUS_ALIGNERR ) INCR_STAT( rx_align_errors );
	//if ( stat & LAN91CXX_RX_STATUS_BCAST    ) INCR_STAT(  );
	if ( stat & LAN91CXX_RX_STATUS_BADCRC ) INCR_STAT( rx_crc_errors );
	if ( stat & LAN91CXX_RX_STATUS_TOOLONG ) INCR_STAT( rx_too_long_frames );
	if ( stat & LAN91CXX_RX_STATUS_TOOSHORT ) INCR_STAT( rx_short_frames );
	//if ( stat & LAN91CXX_RX_STATUS_MCAST    ) INCR_STAT(  );
#endif // KEEP_STATISTICS
	if ((stat & LAN91CXX_RX_STATUS_BAD) == 0)
	{
		INCR_STAT( rx_good );
		// Then it's OK

		if( LAN91CXX_RX_STATUS_IS_ODD(cpd,stat) )
		len++;

#if DEBUG & 1
		db_printf("RxEvent good rx - stat: 0x%04x, len: 0x%04x\n", stat, len);
#endif
		// Check for bogusly short packets; can happen in promisc mode:
		// Asserted against and checked by upper layer driver.
#ifdef CYGPKG_NET
		if ( len > sizeof( struct ether_header ) )
		// then it is acceptable; offer the data to the network stack
#endif
		(sc->funs->eth_drv->recv)(sc, len);

		return;
	}

	// Not OK for one reason or another...
#if DEBUG & 1
	db_printf("RxEvent - bad rx: stat: 0x%04x, len: 0x%04x\n", stat, len);
	db_printf("PHY %2d: %04x\n",0,tse_read_phy( sc, 0, 0));
	db_printf("PHY %2d: %04x\n",1,tse_read_phy( sc, 0, 1));
	db_printf("PHY %2d: %04x\n",2,tse_read_phy( sc, 0, 2));
	db_printf("PHY %2d: %04x\n",3,tse_read_phy( sc, 0, 3));
	db_printf("PHY %2d: %04x\n",4,tse_read_phy( sc, 0, 4));
	db_printf("PHY %2d: %04x\n",5,tse_read_phy( sc, 0, 5));
	db_printf("PHY %2d: %04x\n",16,tse_read_phy( sc, 0, 16));
	db_printf("PHY %2d: %04x\n",17,tse_read_phy( sc, 0, 17));
	db_printf("PHY %2d: %04x\n",18,tse_read_phy( sc, 0, 18));
	db_printf("PHY %2d: %04x\n",19,tse_read_phy( sc, 0, 19));
	db_printf("PHY %2d: %04x\n",20,tse_read_phy( sc, 0, 20));
#endif

	// Free packet
	put_reg(sc, LAN91CXX_MMU_COMMAND, LAN91CXX_MMU_remrel_rx_frame);
#endif
}

//
// This function is called as a result of the "eth_drv_recv()" call above.
// Its job is to actually fetch data for a packet from the hardware once
// memory buffers have been allocated for the packet.  Note that the buffers
// may come in pieces, using a scatter-gather list.  This allows for more
// efficient processing in the upper layers of the stack.
//
static void tse_recv(struct eth_drv_sc *sc, struct eth_drv_sg *sg_list,
		int sg_len)
{
	struct tse_priv_data *cpd = (struct tse_priv_data *) sc->driver_private;

	cyg_uint8 *from_addr;
	cyg_uint32 from, status, i;
	int pkt_len, total_len;

	from_addr = ((cyg_uint8 *) cpd->rx_buffer) + 2;

	DEBUG_FUNCTION();

	for (i = 0; i < sg_len; i++)
	{
		cyg_uint8 *to_addr;
		int len;

		to_addr = (cyg_uint8 *) (sg_list[i].buf);
		len = sg_list[i].len;

		if (to_addr == 0 || len <= 0)
		{
			break; // out of mbufs
		}

		///    	if ( len > pkt_len )
		//		{
		//      		len = pkt_len;
		//		}

		HAL_DCACHE_INVALIDATE(from_addr, len);
		memcpy(to_addr, (void *) from_addr, len);

		from_addr += len;
		//    	pkt_len   -= len;
	}

	alt_sgdma_descriptor *desc_base = cpd->rx_sgdma.descriptor_base;

	// set the recieve descriptor
	alt_avalon_sgdma_construct_stream_to_mem_desc(desc_base, // descriptor
			desc_base + 1, // next descriptor
			cpd->rx_buffer, // starting write_address
			0, // read until EOP
			0); // don't write to constant address

	alt_avalon_sgdma_set_control(&cpd->rx_sgdma, 0x12);

	// SGDMA operation invoked for RX (non-blocking call)
	alt_avalon_sgdma_do_async_transfer(&cpd->rx_sgdma, desc_base);

//	IOWR_ALTERA_TSEMAC_RX_CMD_STAT(     cpd->base, ALTERA_TSEMAC_RX_CMD_STAT_RXSHIFT16_MSK);
	IOWR_ALTERA_TSEMAC_TX_CMD_STAT(     cpd->base, ALTERA_TSEMAC_TX_CMD_STAT_TXSHIFT16_MSK);

}

static void tse_poll(struct eth_drv_sc *sc)
{
	unsigned short event;
	struct tse_priv_data *cpd = (struct tse_priv_data *) sc->driver_private;
	DEBUG_FUNCTION();
#if 0
	//    DEBUG_FUNCTION();
	while (1)
	{
		cyg_drv_interrupt_acknowledge(cpd->interrupt);
		// Get the (unmasked) requests
		event = get_reg(sc, LAN91CXX_INTERRUPT);
		event = event & (event >> 8) & 0xff;

		if (0 == event)
		break;
#if 0
		if (event & LAN91CXX_INTERRUPT_ERCV_INT)
		{
			// Early receive interrupt
			db_printf("Early receive interrupt\n");
		}
		else if (event & LAN91CXX_INTERRUPT_EPH_INT)
		{
			// ethernet protocol handler failures
			db_printf("Ethernet protocol handler failures\n");
		}
		else if (event & LAN91CXX_INTERRUPT_RX_OVRN_INT)
		{
			// receive overrun
			db_printf("Receive overrun\n");
		}
		else if (event & LAN91CXX_INTERRUPT_ALLOC_INT)
		{
			// allocation interrupt
			db_printf("Allocation interrupt\n");
		}
		else
#endif
		if (event & LAN91CXX_INTERRUPT_TX_SET)
		{
			tse_TxEvent(sc, event);
		}
		if (event & LAN91CXX_INTERRUPT_RCV_INT)
		{
			tse_RxEvent(sc);
		}
		if (event & ~(LAN91CXX_INTERRUPT_TX_SET | LAN91CXX_INTERRUPT_RCV_INT))
		db_printf("%s: Unknown interrupt: 0x%04x\n",
				__FUNCTION__, event);
	}
#endif
}

#ifdef LAN91CXX_IS_TSE

static cyg_uint16
tse_read_phy(struct eth_drv_sc *sc, cyg_uint8 phyaddr, cyg_uint8 phyreg)
{
	int i, mask, input_idx, clk_idx = 0;
	cyg_uint16 mii_reg, value;
	cyg_uint8 bits[64];
	DEBUG_FUNCTION();

	// 32 consecutive ones on MDO to establish sync
	for (i = 0; i < 32; ++i)
	bits[clk_idx++] = LAN91CXX_MGMT_MDOE | LAN91CXX_MGMT_MDO;

	// Start code <01>
	bits[clk_idx++] = LAN91CXX_MGMT_MDOE;
	bits[clk_idx++] = LAN91CXX_MGMT_MDOE | LAN91CXX_MGMT_MDO;

	// Read command <10>
	bits[clk_idx++] = LAN91CXX_MGMT_MDOE | LAN91CXX_MGMT_MDO;
	bits[clk_idx++] = LAN91CXX_MGMT_MDOE;

	// Output the PHY address, msb first
	for (mask = 0x10; mask; mask >>= 1)
	{
		if (phyaddr & mask)
		bits[clk_idx++] = LAN91CXX_MGMT_MDOE | LAN91CXX_MGMT_MDO;
		else
		bits[clk_idx++] = LAN91CXX_MGMT_MDOE;
	}

	// Output the phy register number, msb first
	for (mask = 0x10; mask; mask >>= 1)
	{
		if (phyreg & mask)
		bits[clk_idx++] = LAN91CXX_MGMT_MDOE | LAN91CXX_MGMT_MDO;
		else
		bits[clk_idx++] = LAN91CXX_MGMT_MDOE;
	}

	// Tristate and turnaround (1 bit times)
	bits[clk_idx++] = 0;

	// Input starts at this bit time
	input_idx = clk_idx;

	// Will input 16 bits
	for (i = 0; i < 16; ++i)
	bits[clk_idx++] = 0;

	// Final clock bit
	bits[clk_idx++] = 0;

	// Get the current MII register value
	mii_reg = get_reg(sc, LAN91CXX_MGMT);

	// Turn off all MII Interface bits
	mii_reg &= ~(LAN91CXX_MGMT_MDOE | LAN91CXX_MGMT_MCLK |
			LAN91CXX_MGMT_MDI | LAN91CXX_MGMT_MDO);

	// Clock all 64 cycles
	for (i = 0; i < sizeof(bits); ++i)
	{
		// Clock Low - output data
		put_reg(sc, LAN91CXX_MGMT, mii_reg | bits[i]);
		HAL_DELAY_US(50);

		// Clock Hi - input data
		put_reg(sc, LAN91CXX_MGMT, mii_reg | bits[i] | LAN91CXX_MGMT_MCLK);
		HAL_DELAY_US(50);

		bits[i] |= get_reg(sc, LAN91CXX_MGMT) & LAN91CXX_MGMT_MDI;
	}

	// Return to idle state
	put_reg(sc, LAN91CXX_MGMT, mii_reg);
	HAL_DELAY_US(50);

	// Recover input data
	for (value = 0, i = 0; i < 16; ++i)
	{
		value <<= 1;
		if (bits[input_idx++] & LAN91CXX_MGMT_MDI)
		value |= 1;
	}
	return value;
}

static void
tse_write_phy(struct eth_drv_sc *sc, cyg_uint8 phyaddr,
		cyg_uint8 phyreg, cyg_uint16 value)
{
	int i, mask, clk_idx = 0;
	cyg_uint16 mii_reg;
	cyg_uint8 bits[65];

	// 32 consecutive ones on MDO to establish sync
	for (i = 0; i < 32; ++i)
	bits[clk_idx++] = LAN91CXX_MGMT_MDOE | LAN91CXX_MGMT_MDO;

	// Start code <01>
	bits[clk_idx++] = LAN91CXX_MGMT_MDOE;
	bits[clk_idx++] = LAN91CXX_MGMT_MDOE | LAN91CXX_MGMT_MDO;

	// Write command <01>
	bits[clk_idx++] = LAN91CXX_MGMT_MDOE;
	bits[clk_idx++] = LAN91CXX_MGMT_MDOE | LAN91CXX_MGMT_MDO;

	// Output the PHY address, msb first
	for (mask = 0x10; mask; mask >>= 1)
	{
		if (phyaddr & mask)
		bits[clk_idx++] = LAN91CXX_MGMT_MDOE | LAN91CXX_MGMT_MDO;
		else
		bits[clk_idx++] = LAN91CXX_MGMT_MDOE;
	}

	// Output the phy register number, msb first
	for (mask = 0x10; mask; mask >>= 1)
	{
		if (phyreg & mask)
		bits[clk_idx++] = LAN91CXX_MGMT_MDOE | LAN91CXX_MGMT_MDO;
		else
		bits[clk_idx++] = LAN91CXX_MGMT_MDOE;
	}

	// Tristate and turnaround (2 bit times)
	bits[clk_idx++] = 0;
	bits[clk_idx++] = 0;

	// Write out 16 bits of data, msb first
	for (mask = 0x8000; mask; mask >>= 1)
	{
		if (value & mask)
		bits[clk_idx++] = LAN91CXX_MGMT_MDOE | LAN91CXX_MGMT_MDO;
		else
		bits[clk_idx++] = LAN91CXX_MGMT_MDOE;
	}

	// Final clock bit (tristate)
	bits[clk_idx++] = 0;

	// Get the current MII register value
	mii_reg = get_reg(sc, LAN91CXX_MGMT);

	// Turn off all MII Interface bits
	mii_reg &= ~(LAN91CXX_MGMT_MDOE | LAN91CXX_MGMT_MCLK |
			LAN91CXX_MGMT_MDI | LAN91CXX_MGMT_MDO);

	// Clock all cycles
	for (i = 0; i < sizeof(bits); ++i)
	{
		// Clock Low - output data
		put_reg(sc, LAN91CXX_MGMT, mii_reg | bits[i]);
		HAL_DELAY_US(50);

		// Clock Hi - input data
		put_reg(sc, LAN91CXX_MGMT, mii_reg | bits[i] | LAN91CXX_MGMT_MCLK);
		HAL_DELAY_US(50);

		//	bits[i] |= get_reg(sc, LAN91CXX_MGMT) & LAN91CXX_MGMT_MDI;
	}

	// Return to idle state
	put_reg(sc, LAN91CXX_MGMT, mii_reg);
	HAL_DELAY_US(50);
}
#endif // LAN91CXX_IS_TSE



/* @Function Description: Change operating mode of Marvell PHY to GMII
 * @API Type:   Internal
 * @param pmac  Pointer to the first TSE MAC Control Interface Base address within MAC group
 */
cyg_uint32 marvell_cfg_gmii(np_tse_mac *pmac)
{

	cyg_uint16 dat = IORD(&pmac->mdio1.reg1b, 0);
    dat &= 0xfff0;

//    printf(5, "MARVELL : Mode changed to GMII to copper mode\n");
    IOWR(&pmac->mdio1.reg1b, 0, dat | 0xf);

//    printf(5, "MARVELL : Disable RGMII Timing Control\n");
    dat = IORD(&pmac->mdio1.reg14, 0);
    dat &= ~0x82;
    IOWR(&pmac->mdio1.reg14, 0, dat);

//    printf(5, "MARVELL : PHY reset\n");
    dat = IORD(&pmac->mdio1.CONTROL, 0);
    IOWR(&pmac->mdio1.CONTROL, 0, dat | PCS_CTL_sw_reset);

    return 1;
}


/* @Function Description -  Determine link speed our PHY negotiated with our link partner.
 *                          This is fully vendor specific depending on the PHY you are using.
 * 
 * @API TYPE - Internal
 * @param  tse.mi.base MAC register map.
 * @return 0x11 if Gigabit link is established and full duplex,
 *         0x01 if not Gigabit link is established and full duplex, 
 *         0x00 if not Gigabit link is established and half duplex, 
 * If the link speed cannot be determined, it is fall back to 10/100.
 */
cyg_uint32 getPHYSpeed(np_tse_mac *pmac)
{

	int isKnown; /* known PHY. */
	cyg_uint32 is1000;
	cyg_uint32 duplex; /* 1 = full ; 0 = half*/
	int phyid;
	int phyid2 = 0;
	int dat;
	int isMVL = 0;
	cyg_uint32 result;
	int phyadd;

	DEBUG_FUNCTION();
	// determine PHY speed: This is PHY dependent and you need to change
	// this according to your PHY's specifications

	is1000 = 0;
	duplex = 1;
	isKnown = 0;

	result = 1;

	if (0 == TSE_MAC_USE_MDIO)
	{
		is1000 = ALTERA_TSE_MAC_SPEED_DEFAULT;
		duplex = ALTERA_TSE_DUPLEX_MODE_DEFAULT;
		usleep(ALTERA_NOMDIO_TIMEOUT_THRESHOLD);
		result = ((is1000 & 0x01) << 1) | (duplex & 0x01);
		return result;

	}

#ifndef ALT_SIM_OPTIMIZE
	// ------------------------------
	// PHY detection
	// ------------------------------

	for (phyadd = 0x00; phyadd < 0xff; phyadd++)
	{
		IOWR(&pmac->MDIO_ADDR1,0, phyadd);
		phyid = IORD(&pmac->mdio1.PHY_ID1,0); // read PHY ID
		phyid2 = IORD(&pmac->mdio1.PHY_ID2,0); // read PHY ID
		//                    dprintf("[phyID] 0x%x %x %x\n",phyadd, phyid, phyid2);

		if (phyid != phyid2)
		{
//			printf("[phyID] 0x%x %x %x\n", phyadd, phyid, phyid2);
			break;
		}
	}

	// ------------------------------
	// National PHY on PHYWORKX board
	// ------------------------------

	// set MDIO address we want to find our PHY at.

	if ((phyid == (NTLPHY_ID >> 16)) && (phyid2 == (NTLPHY_ID & 0xFFFF)))
	{
//		printf("[netif_tse] found National DP83865 PHY\n");
		isKnown = 1;
	}

	if (isKnown == 0)
	{

		// ------------------------------
		// Marvell PHY on PHYWORKX board
		// ------------------------------

		if (phyid == MVLPHY_ID)
		{
//			printf("[netif_tse] found Marvell 88E1111 PHY\n");
			isKnown = 1;
			isMVL = 1;

			// If there is no link yet, we enable auto crossover and reset the PHY

			if ((IORD(&pmac->mdio1.STATUS,0) & PCS_ST_an_done) == 0)
			{
				IOWR(&pmac->mdio1.reg10,0,0x0078); // 6:5=11 = AUTO MDIX crossover

			}
		}
	}

	if (isKnown == 0)
	{

		// --------------------------------------
		// National 10/100 PHY on PHYWORKX board
		// --------------------------------------

		if ((phyid == (NTL848PHY_ID >> 16)) && (phyid2 == (NTL848PHY_ID
				& 0xFFFF)))
		{
//			printf("[netif_tse] found National DP83848C PHY\n");
			isKnown = 1;
		}
	}

	if (isKnown)
	{

		// To enable PHY loopback
#if ENABLE_PHY_LOOPBACK
//		printf("Putting PHY in loopback\n");
		dat = IORD(&pmac->mdio1.CONTROL,0) & 0x001f; // keep bits 5:0
#ifdef GIGABIT
		if( isMVL )
		{
			IOWR(&pmac->mdio1.CONTROL,0, dat | 0xc140); // enable loopback, 1000Mbps mode, fulldup
		}
		IOWR(&pmac->mdio1.CONTROL,0, dat | 0x4140); // enable loopback, 1000Mbps mode, fulldup
#else
		if( isMVL )
		{
			IOWR(&pmac->mdio1.CONTROL,0,dat | 0xe100); // enable loopback, 100Mbps mode, fulldup
		}
		IOWR(&pmac->mdio1.CONTROL,0,dat | 0x6100); // enable loopback, 100Mbps mode, fulldup
#endif

#else
		// Issue a PHY reset here and wait for the link
		// autonegotiation complete again... this takes several SECONDS(!)
		// so be very careful not to do it frequently

		// perform this when PHY is configured in loopback or has no link yet.

		if (((IORD(&pmac->mdio1.CONTROL,0) & PCS_CTL_rx_slpbk) != 0)
				|| ((IORD(&pmac->mdio1.STATUS,0) & PCS_ST_an_done) == 0))
		{

			IOWR(&pmac->mdio1.CONTROL,0,PCS_CTL_an_enable | PCS_CTL_sw_reset); // send PHY reset command
//			printf("[netif_tse] PHY Reset ");
		}
#endif

		if ((1 == TSE_MAC_ENABLE_MACLITE) && (0 == TSE_MAC_MACLITE_GIGE))
		{
			dat = IORD(&pmac->mdio1.CONTROL,9);
			IOWR(&pmac->mdio1.CONTROL,9,dat & 0xFCFF);
			dat = IORD(&pmac->mdio1.CONTROL,0);
			IOWR(&pmac->mdio1.CONTROL,0,dat | 0x200);

			usleep(ALTERA_DISGIGA_TIMEOUT_THRESHOLD);
		}

		if ((IORD(&pmac->mdio1.STATUS,0) & PCS_ST_an_done) == 0)
		{
//			printf(" waiting on PHY link..");
			dat = 0;
			while ((IORD(&pmac->mdio1.STATUS,0) & PCS_ST_an_done) == 0)
			{
				if (dat++ > ALTERA_AUTONEG_TIMEOUT_THRESHOLD)
				{
//					printf(" Autoneg FAILED, continuing anyway ... ");
					break;
				}
			}
//			printf("OK. x=%d, PHY STATUS=%04x\n", dat,
//					IORD(&pmac->mdio1.STATUS,0));
		}

		// retrieve link speed from PHY

		if ((phyid == (NTLPHY_ID >> 16)) && (phyid2 == (NTLPHY_ID & 0xFFFF)))
		{
			dat = IORD(&pmac->mdio1.reg11,0); // read PHY status register (vendor specific)

			duplex = (dat >> 1) & 0x01; // extract connection duplex information

			dat = (dat >> 3) & 0x03; // extract speed information
			is1000 = (dat == 2) ? 1 : 0;

		}
		else if (phyid == MVLPHY_ID)
		{

			dat = IORD(&pmac->mdio1.reg11,0); // read PHY status register (vendor specific)
			if (dat & (1 << 6))
			{
//				printf("[netif_tse] PHY MDIX (crossover)\n");
			}

			duplex = (dat >> 13) & 0x01; // extract connection duplex information

			dat = (dat >> 14) & 0x03;
			is1000 = (dat == 2) ? 1 : 0;
#if 0
			printf("reg0 0x%08x\n", IORD(&pmac->mdio1.CONTROL,0));
			printf("reg1 0x%08x\n", IORD(&pmac->mdio1.STATUS,0));
			printf("reg2 0x%08x\n", IORD(&pmac->mdio1.PHY_ID1,0));
			printf("reg3 0x%08x\n", IORD(&pmac->mdio1.PHY_ID2,0));
			printf("reg4 0x%08x\n", IORD(&pmac->mdio1.ADV,0));
			printf("reg5 0x%08x\n", IORD(&pmac->mdio1.REMADV,0));
			printf("reg6 0x%08x\n", IORD(&pmac->mdio1.reg6,0));
			printf("reg7 0x%08x\n", IORD(&pmac->mdio1.reg7,0));
			printf("reg8 0x%08x\n", IORD(&pmac->mdio1.reg8,0));
			printf("reg9 0x%08x\n", IORD(&pmac->mdio1.reg9,0));
			printf("rega 0x%08x\n", IORD(&pmac->mdio1.rega,0));
			printf("regb 0x%08x\n", IORD(&pmac->mdio1.regb,0));
			printf("regc 0x%08x\n", IORD(&pmac->mdio1.regc,0));
			printf("regd 0x%08x\n", IORD(&pmac->mdio1.regd,0));
			printf("rege 0x%08x\n", IORD(&pmac->mdio1.rege,0));
			printf("regf 0x%08x\n", IORD(&pmac->mdio1.regf,0));
			printf("reg10 0x%08x\n", IORD(&pmac->mdio1.reg10,0));
			printf("reg11 0x%08x\n", IORD(&pmac->mdio1.reg11,0));
			printf("reg12 0x%08x\n", IORD(&pmac->mdio1.reg12,0));
			printf("reg13 0x%08x\n", IORD(&pmac->mdio1.reg13,0));
			printf("reg14 0x%08x\n", IORD(&pmac->mdio1.reg14,0));
			printf("reg15 0x%08x\n", IORD(&pmac->mdio1.reg15,0));
			printf("reg16 0x%08x\n", IORD(&pmac->mdio1.reg16,0));
			printf("reg17 0x%08x\n", IORD(&pmac->mdio1.reg17,0));
			printf("reg18 0x%08x\n", IORD(&pmac->mdio1.reg18,0));
			printf("reg19 0x%08x\n", IORD(&pmac->mdio1.reg19,0));
			printf("reg1a 0x%08x\n", IORD(&pmac->mdio1.reg1a,0));
			printf("reg1b 0x%08x\n", IORD(&pmac->mdio1.reg1b,0));
			printf("reg1c 0x%08x\n", IORD(&pmac->mdio1.reg1c,0));
			printf("reg1d 0x%08x\n", IORD(&pmac->mdio1.reg1d,0));
			printf("reg1e 0x%08x\n", IORD(&pmac->mdio1.reg1e,0));
			printf("reg1f 0x%08x\n", IORD(&pmac->mdio1.reg1f,0));

#endif

		}
		else if ((phyid == (NTL848PHY_ID >> 16)) && (phyid2 == (NTL848PHY_ID
				& 0xFFFF)))
		{

			dat = IORD(&pmac->mdio1.reg10,0); // read PHY status register (vendor specific)

			duplex = (dat >> 2) & 0x01; // extract connection duplex information
			is1000 = 0;

		}
		else
		{

			is1000 = ALTERA_TSE_MAC_SPEED_DEFAULT;
			duplex = ALTERA_TSE_DUPLEX_MODE_DEFAULT;

		}

	}
	else
	{
		// unknown PHY... default to 10/100
		is1000 = ALTERA_TSE_MAC_SPEED_DEFAULT;
		duplex = ALTERA_TSE_DUPLEX_MODE_DEFAULT;
	}
#else
	// for simulation purpose, default to gigabit mode
	is1000 = 1;
	duplex = 1;
#endif

	result = ((is1000 & 0x01) << 1) | (duplex & 0x01);
//	db_printf("[triple_speed_ethernet_init] Speed is 1000 is %d\t Full Duplex is %d\n", is1000, duplex);

	return result;
}

// EOF if_tse.c
