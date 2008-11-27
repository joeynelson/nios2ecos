//==========================================================================
//
//      altera_avalon_lcd_16207.c
//
//      SOPC builder aware driver for the LCD 16207 controller.
//
//==========================================================================
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

#include <pkgconf/system.h>
#include <pkgconf/io.h>

#include <cyg/io/io.h>
#include <cyg/io/devtab.h>
#include <cyg/infra/diag.h>

#include <altera_avalon_lcd_16207.h>

/*
 * This file creates the device instances for the Altera Avalon LCD 16207 driver.
 * This is done using the auto-generated macro file cyg/hal/devices.h. This
 * macro file contains an instance of the ALTERA_AVALON_LCD_16207_INSTANCE macro
 * for every Altera Avalon UART found in the system.
 *
 * The definition of this macro given below, causes an instance of this device
 * diver to be associated with each of these physical devices.
 */

/*
 * Only build the driver if this component has been selected by the user. 
 */

#ifdef CYGPKG_ALTERA_AVALON_LCD_16207

#include <pkgconf/io_serial.h>

/*
 * Create all necessary device instances using the auto-generated devices.h. The
 * settings used here for baud rate etc. are dummy values, since these settings are
 * meaningless for this device.
 */

#define ALTERA_AVALON_LCD_16207_INSTANCE(name, device)                             \
                                                                                   \
  static alt_lcd_16207_dev device##_info = {                                       \
    name##_BASE,                                                                   \
    };                                                                             \
                                                                                   \
  static SERIAL_CHANNEL(device##_channel,                                          \
                        alt_lcd_16207_funs,                                        \
                        device##_info,                                             \
                        CYG_SERIAL_BAUD_DEFAULT,                                   \
                        CYG_SERIAL_STOP_DEFAULT,                                   \
                        CYG_SERIAL_PARITY_DEFAULT,                                 \
                        CYG_SERIAL_WORD_LENGTH_DEFAULT,                            \
                        CYG_SERIAL_FLAGS_DEFAULT);                                 \
                                                                                   \
  DEVTAB_ENTRY(device##_io,                                                        \
               name##_NAME,                                                        \
               0,                                                                  \
               &cyg_io_serial_devio,                                               \
               alt_lcd_16207_init,                                                 \
               alt_lcd_16207_lookup,                                               \
               &device##_channel);

#include <cyg/hal/devices.h>

#endif /* CYGPKG_ALTERA_AVALON_LCD_16207 */
