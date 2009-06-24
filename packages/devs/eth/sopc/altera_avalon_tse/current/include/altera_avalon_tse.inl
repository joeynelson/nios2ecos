
//=============================================================================
//
//      altera_avalon_tse.inl
//
//      Device driver for the Altera Avalon TSE device.
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
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
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//===========================================================================

/*
 * This file is included into the SMSC driver in order to define the
 * configuration of the ethernet device (e.g. base address). In this particular
 * case these parameters are extracted from the associated SOPC builder
 * project using the auto-generated file, device.h.
 */

/*
 * Provide a prototype for the function used to obtain the MAC address.
 */

extern void CYGDAT_TSE_GET_ESA(unsigned char   enaddr[],
                               unsigned short* base,
                               int             irq)
{
	unsigned char   static_esa[] = CYGDAT_TSE_MAC_DEFAULT;
	memcpy(enaddr, static_esa, 6);
};

/*
 * Nios Development Boards are programmed on the production line with a unique
 * MAC address & network settings in the last sector of flash memory.
 *
 * Stratix II based Nios Development Boards have 16 megabytes of flash memory,
 * so the last flash sector is located starting at offset 0x00FF0000 from
 * the flash base.
 *
 * For Stratix and Cyclone based Nios Development Boards, which have 8 megabytes
 * of flash memory, the last flash sector is located at offset address 0x7F0000
 * from a flash memory base address of 0 as defined in system.h.
 */

#ifdef EXT_FLASH_BASE

/* Stratix II based Nios and DSP Development Board */
#if defined(ALTERA_NIOS_DEV_BOARD_STRATIX_2S60_ES) || \
    defined(ALTERA_NIOS_DEV_BOARD_STRATIX_2S60)    || \
    defined(ALTERA_NIOS_DEV_BOARD_CYCLONE_2C35)
#define LAST_SECTOR_OFFSET  0x00FF0000
#endif

/* Stratix and Cyclone based Nios Development Board */
#if defined(ALTERA_NIOS_DEV_BOARD_CYCLONE_1C20)    || \
    defined(ALTERA_NIOS_DEV_BOARD_STRATIX_1S10)    || \
    defined(ALTERA_NIOS_DEV_BOARD_STRATIX_1S10_ES) || \
    defined(ALTERA_NIOS_DEV_BOARD_STRATIX_1S40)
#define LAST_SECTOR_OFFSET  0x007F0000
#endif

/* All Nios Development Boards */
#define LAST_FLASH_SECTOR EXT_FLASH_BASE + LAST_SECTOR_OFFSET

/*
 * genone_tse_get_esa() is the function used to
 * construct the MAC address for the device. An alternative function can be
 * invoked using the CDL parameter: CYGDAT_TSE_GET_ESA of this
 * component.
 *
 * Note that address x60is the micromonitor storage locate for the 
 * MAC address stored int ASCII xx:xx:xx:xx:xx:xx
 */

void genone_tse_get_esa(unsigned char   enaddr[],
                              unsigned short* base,
                              int             irq)
{
  unsigned char * macaddr	   = (char *)0x60;
  unsigned char   static_esa[] = CYGDAT_TSE_MAC_DEFAULT;
  unsigned char   digit;
  unsigned int    i;

  if ( (char)macaddr[0] != (char)0xff)
  {
	for( i = 0; i < 6; i++)
	{
		if ((	macaddr[(i * 3)] >= '0') && ( macaddr[(i * 3)] <= '9'))
			digit =	   macaddr[(i * 3)] - '0';

		if ((	macaddr[(i * 3)] >= 'a') && ( macaddr[(i * 3)] <= 'f'))
			digit =	(  macaddr[(i * 3)] -  'a') + 10;

		digit <<= 4;

		if ((	macaddr[(i * 3) + 1] >= '0') && ( macaddr[(i * 3) + 1] <= '9'))
			digit |=	macaddr[(i * 3) + 1] - '0';

		if ((	macaddr[(i * 3) + 1] >= 'a') && ( macaddr[(i * 3) + 1] <= 'f'))
			digit |= (  macaddr[(i * 3) + 1] - 'a') + 10;

        enaddr[i] = digit;
	}
  }
  else
  {
     memcpy(enaddr, static_esa, 6);
  }
}

void altera_avalon_tse_get_esa(unsigned char   enaddr[],
                               unsigned short* base,
                               int             irq)
{
  cyg_uint32 signature;
  unsigned char static_esa[] = CYGDAT_TSE_MAC_DEFAULT;

  int ret_code = 0;

#if defined(ALTERA_NIOS_DEV_BOARD_CYCLONE_1C20)     || \
    defined(ALTERA_NIOS_DEV_BOARD_CYCLONE_2C35)     || \
    defined(ALTERA_NIOS_DEV_BOARD_STRATIX_1S10)     || \
    defined(ALTERA_NIOS_DEV_BOARD_STRATIX_1S10_ES)  || \
    defined(ALTERA_NIOS_DEV_BOARD_STRATIX_1S40)     || \
    defined(ALTERA_NIOS_DEV_BOARD_STRATIX_2S60)     || \
    defined(ALTERA_NIOS_DEV_BOARD_STRATIX_2S60_ES)

  HAL_READ_UINT32 ((void*) (LAST_FLASH_SECTOR), signature);

  if (signature != 0x00005afe)
  {
    diag_printf("This Flash does not contain a valid MAC Address\n");
    memcpy(enaddr, static_esa, 6);
    diag_printf("Using the default of 0x%x:%x:%x:%x:%x:%x\n", enaddr[0], enaddr[1], enaddr[2],
            enaddr[3], enaddr[4], enaddr[5]);
  }
  else
  {
    HAL_READ_UINT8 ((void*) (LAST_FLASH_SECTOR + 4), enaddr[0]);
    HAL_READ_UINT8 ((void*) (LAST_FLASH_SECTOR + 5), enaddr[1]);
    HAL_READ_UINT8 ((void*) (LAST_FLASH_SECTOR + 6), enaddr[2]);
    HAL_READ_UINT8 ((void*) (LAST_FLASH_SECTOR + 7), enaddr[3]);
    HAL_READ_UINT8 ((void*) (LAST_FLASH_SECTOR + 8), enaddr[4]);
    HAL_READ_UINT8 ((void*) (LAST_FLASH_SECTOR + 9), enaddr[5]);
  }
#else
  diag_printf("This is not an Altera development board\n");
  memcpy(enaddr, static_esa, 6);
  diag_printf("Using the default of 0x%x:%x:%x:%x:%x:%x\n", enaddr[0], enaddr[1], enaddr[2],
          enaddr[3], enaddr[4], enaddr[5]);

  perror( "Not an Altera board.\n");
  perror( "You need to modify the function altera_avalon_tse_get_esa() \n");
  perror( "to set a MAC address for your board");
#endif /* (Altera Nios Dev board definition) */
}

#endif /* EXT_FLASH_BASE */

/*
 *
 */
struct tse_priv_data;
static void altera_avalon_tse_get_esa_wrapper(struct tse_priv_data* cpd)
{
  CYGDAT_TSE_GET_ESA(cpd->enaddr, cpd->base, cpd->interrupt);
}

/*
 * Create all necessary device instances using the auto-generated
 * devices.h.
 */


#define TRIPLE_SPEED_ETHERNET_INSTANCE(name, dev)					\
static tse_priv_data dev##_priv_data = {                       		\
    config_enaddr : altera_avalon_tse_get_esa_wrapper,        		\
    base : (name##_BASE),       		\
    interrupt: SGDMA_RX_IRQ                                           \
};                                                                  \
                                                                    \
ETH_DRV_SC( dev##_sc,                                               \
           &dev##_priv_data,                                        \
           "eth",                                                   \
           tse_start,                                          		\
           tse_stop,                                           		\
           tse_control,                                        		\
           tse_can_send,                                       		\
           tse_send,                                           		\
           tse_recv,                                           		\
           tse_deliver,                                        		\
           tse_poll,                                           		\
           tse_int_vector                                      		\
);                                                                  \
                                                                    \
NETDEVTAB_ENTRY(dev##_netdev,                                       \
                #dev "_eth0",                                       \
                tse_init,                                 			\
                &dev##_sc);

#include <cyg/hal/devices.h>
