#ifndef CYGONCE_ALTERA_AVALON_UART_H
#define CYGONCE_ALTERA_AVALON_UART_H

//=============================================================================
//
//      altera_avalon_uart.h
//
//      Device driver for the Altera Avalon UART.
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

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#include <cyg/infra/diag.h>
#include <cyg/infra/cyg_type.h>
#include <cyg/hal/hal_if.h>
#include <cyg/hal/hal_misc.h>
#include <cyg/hal/drv_api.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_io.h>

#ifdef CYGPKG_IO_SERIAL

#include <pkgconf/altera_avalon_uart.h>

#include <cyg/io/devtab.h>
#include <cyg/io/serial.h>
#include <cyg/io/io.h>
#include <cyg/hal/hal_intr.h>

extern bool altera_avalon_uart_init(struct cyg_devtab_entry *tab);

extern Cyg_ErrNo altera_avalon_uart_lookup(struct cyg_devtab_entry **tab, 
                                           struct cyg_devtab_entry *sub_tab,
                                           const char *name);

extern serial_funs altera_avalon_uart_funs;

/*
 * Structure used to store the per device private data for this driver.
 */

typedef struct {
  CYG_ADDRWORD        base;  /* Base address of the device */
  cyg_uint32          irq;   /* Interrupt number */
  cyg_uint32          freq;  /* Input clock frequency of the device */
  cyg_uint32          flags; /* Flags indicating hardware configuration */
  cyg_interrupt       serial_interrupt;
  cyg_handle_t        serial_interrupt_handle;
  cyg_uint8           rx_buf[CYGDAT_ALT_AVALON_UART_BUF_LEN]; /* The receive buffer */
  cyg_uint8           tx_buf[CYGDAT_ALT_AVALON_UART_BUF_LEN]; /* The transmit buffer */
} altera_avalon_uart_dev;

/*
 * Flags that can be set in the flags field of the above structure.
 */ 

#define ALT_AVALON_UART_FB 0x1 /* baud rate is fixed in hardware */ 
#define ALT_AVALON_UART_FC 0x2 /* hardware flow control available */

#endif /* CYGPKG_IO_SERIAL */
 
#endif /* CYGONCE_ALTERA_AVALON_UART_H */
