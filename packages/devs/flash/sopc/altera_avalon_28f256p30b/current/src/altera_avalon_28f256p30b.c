#include <pkgconf/system.h>

#ifdef CYGPKG_DEVS_FLASH_STRATA_V2

#include <cyg/io/flash.h>
#include <cyg/io/strata_dev.h>
#include <cyg/hal/system.h>

//28F256P30B

static const CYG_FLASH_FUNS(
	altera_avalon_28f256p30b_flash_strata_funs,
    &cyg_strata_init_check_devid_16,
    &cyg_flash_devfn_query_nop,
    &cyg_strata_erase_16,
    &cyg_strata_bufprogram_16,
    (int (*)(struct cyg_flash_dev*, const cyg_flashaddr_t, void*, size_t))0,
    &cyg_strata_lock_k3_16,
    &cyg_strata_unlock_k3_16);

static const cyg_strata_dev altera_avalon_28f256p30b_flash_priv = {
    .manufacturer_code = CYG_FLASH_STRATA_MANUFACTURER_INTEL,
    .device_code = 0x891C,
    .bufsize    = 16,
    .block_info = {
        { 0x00008000, 4 	},
        { 0x00020000, 255	}
    }
};

CYG_FLASH_DRIVER(altera_avalon_28f256p30b_flash,
                 &altera_avalon_28f256p30b_flash_strata_funs,
                 0,
                 EXT_FLASH_BASE,
                 (EXT_FLASH_BASE + EXT_FLASH_SPAN - 1),
                 2,
                 altera_avalon_28f256p30b_flash_priv.block_info,
                 &altera_avalon_28f256p30b_flash_priv
);

#endif //CYGPKG_DEVS_FLASH_STRATA_V2
