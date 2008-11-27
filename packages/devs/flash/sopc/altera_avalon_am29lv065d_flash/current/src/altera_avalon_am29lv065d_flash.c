//==========================================================================
//
//      altera_avalon_am29lv065d_flash.c
//
//      SOPC builder aware driver for AM29LV065D flash memory.
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
#include <pkgconf/altera_avalon_am29lv065d_flash.h>
#include <cyg/hal/system.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/infra/cyg_type.h>

/* Device properties */

#define CYGNUM_FLASH_INTERLEAVE (1)
#define CYGNUM_FLASH_SERIES     (1)
#define CYGNUM_FLASH_WIDTH      (8)
#define CYGNUM_FLASH_BASE       altera_avalon_am29lv065d_base

#define CYGHWR_FLASH_AM29XXXXX_PLF_INIT altera_avalon_am29lv065d_init

/* local variables used to store the flash parameters */

static cyg_uint8* altera_avalon_am29lv065d_base = (void*) 0;

/* Macros used below to initialise the flash parameters */

#define _DEV_NAME(dev) "/dev/" #dev
#define DEV_NAME(dev) _DEV_NAME(dev)

#define ALTERA_AVALON_CFI_FLASH_INSTANCE(name, device)                                   \
if (!__builtin_strcmp(DEV_NAME(ALTERA_AVALON_AM29LV065D_FLASH_DEV), name##_NAME) ||      \
    (!__builtin_strcmp(DEV_NAME(ALTERA_AVALON_AM29LV065D_FLASH_DEV), "/dev/default") &&  \
     !found))                                                                            \
{                                                                                        \
  altera_avalon_am29lv065d_base = (cyg_uint8*) CYGARC_UNCACHED_ADDRESS(name##_BASE);     \
  found = 1;                                                                             \
}

/* Function used to initialise the flash parameters */

static void altera_avalon_am29lv065d_init (void)
{
  int found = 0;
 
#include <cyg/hal/devices.h>
}

/* Now include the driver code. */

#include "cyg/io/flash_am29xxxxx.inl"
