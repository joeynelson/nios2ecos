//# ====================================================================
//#
//#      altera_avalon_28f256p30t.c
//#
//#      28F256P30T flash driver.
//#
//# ====================================================================
//#####ECOSGPLCOPYRIGHTBEGIN####
//## -------------------------------------------
//## This file is part of eCos, the Embedded Configurable Operating System.
//##
//## eCos is free software; you can redistribute it and/or modify it under
//## the terms of the GNU General Public License as published by the Free
//## Software Foundation; either version 2 or (at your option) any later version.
//##
//## eCos is distributed in the hope that it will be useful, but WITHOUT ANY
//## WARRANTY; without even the implied warranty of MERCHANTABILITY or
//## FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
//## for more details.
//##
//## You should have received a copy of the GNU General Public License along
//## with eCos; if not, write to the Free Software Foundation, Inc.,
//## 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//##
//## As a special exception, if other files instantiate templates or use macros
//## or inline functions from this file, or you compile this file and link it
//## with other works to produce a work based on this file, this file does not
//## by itself cause the resulting work to be covered by the GNU General Public
//## License. However the source code for this file must still be made available
//## in accordance with section (3) of the GNU General Public License.
//##
//## This exception does not invalidate any other reasons why a work based on
//## this file might be covered by the GNU General Public License.
//##
//## -------------------------------------------
//#####ECOSGPLCOPYRIGHTEND####

#include <pkgconf/system.h>

#ifdef CYGPKG_DEVS_FLASH_STRATA_V2

#include <cyg/io/flash.h>
#include <cyg/io/strata_dev.h>
#include <cyg/hal/system.h>

//28F256P30T

static const CYG_FLASH_FUNS(
	altera_avalon_28f256p30t_flash_strata_funs,
    &cyg_strata_init_check_devid_16,
    &cyg_flash_devfn_query_nop,
    &cyg_strata_erase_16,
    &cyg_strata_program_16,
    (int (*)(struct cyg_flash_dev*, const cyg_flashaddr_t, void*, size_t))0,
    &cyg_strata_lock_k3_16,
    &cyg_strata_unlock_k3_16);



static const cyg_strata_dev altera_avalon_28f256p30t_flash_priv = {
    .manufacturer_code = CYG_FLASH_STRATA_MANUFACTURER_INTEL,
    .device_code = 0x8919,
    .bufsize    = 32,
    .block_info = {
        { 0x00020000, 255	},
        { 0x00008000, 4 	}
    }
};

CYG_FLASH_DRIVER(altera_avalon_28f256p30t_flash,
                 &altera_avalon_28f256p30t_flash_strata_funs,
                 0,
                 EXT_FLASH_1_BASE,
                 (EXT_FLASH_1_BASE + EXT_FLASH_1_SPAN - 1),
                 2,
                 altera_avalon_28f256p30t_flash_priv.block_info,
                 &altera_avalon_28f256p30t_flash_priv
);


#endif //CYGPKG_DEVS_FLASH_STRATA_V2
