#ifndef CYGONCE_ALTERA_AVALON_LCD_16207_H
#define CYGONCE_ALTERA_AVALON_LCD_16207_H

//=============================================================================
//
//      altera_avalon_lcd_16207.h
//
//      Device driver for the Altera Avalon LCD 16207 controller.
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

#include <pkgconf/altera_avalon_lcd_16207.h>

#include <cyg/io/io.h>
#include <cyg/io/devtab.h>
#include <cyg/infra/diag.h>
#include <cyg/infra/cyg_type.h>
#include <cyg/hal/hal_if.h>
#include <cyg/hal/hal_misc.h>
#include <cyg/hal/drv_api.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_io.h>

#include <cyg/io/serial.h>
#include <cyg/hal/hal_intr.h>

#include <cyg/kernel/kapi.h>

#define ALT_LCD_HEIGHT         2
#define ALT_LCD_WIDTH         16
#define ALT_LCD_VIRTUAL_WIDTH 80

/*
 * Functions defined in altera_avalon_lcd_16207.c
 */

extern bool alt_lcd_16207_init(struct cyg_devtab_entry *tab);

extern Cyg_ErrNo alt_lcd_16207_lookup(struct cyg_devtab_entry **tab, 
                                      struct cyg_devtab_entry *sub_tab,
                                      const char              *name);

extern serial_funs alt_lcd_16207_funs;

/*
 * Structure used to store the per device private data for this driver.
 */

typedef struct {
  CYG_ADDRWORD        base;  /* Base address of the device */
  char                broken;

  char                x;
  char                y;
  char                address;
  char                esccount;

  char                scrollpos;
  char                scrollmax;
  char                active;    /* If non-zero then the foreground routines are
                                  * active so the timer call must not update the
                                  * display. */

  char                escape[8];

  cyg_alarm           alarm;

  struct
  {
    char         visible[ALT_LCD_WIDTH];
    char         data[ALT_LCD_VIRTUAL_WIDTH+1];
    char         width;
    unsigned char speed;

  } line[ALT_LCD_HEIGHT];
} alt_lcd_16207_dev;

#endif /* CYGONCE_ALTERA_AVALON_LCD_16207_H */
