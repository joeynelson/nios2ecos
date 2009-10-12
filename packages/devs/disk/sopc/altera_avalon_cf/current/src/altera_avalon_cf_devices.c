//==========================================================================
//
//      altera_avalon_cf_devices.c
//
//      Altera Avalon compact flash driver 
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
//==========================================================================

#include <cyg/io/devtab.h>
#include <cyg/io/disk.h>

#include <cyg/hal/system.h>
#include <altera_avalon_cf_priv.h>

#include <pkgconf/altera_avalon_cf.h>

// Create device table entries for all instances of the 
// altera_avalon_cf component found in the system. 
//
// Note that two devices are created for each component instance.
// These represent the two devices available per interface. For
// example, if you have a component instance named "my_cf" in 
// SOPC builder, then this will create the devices: "/dev/my_cfa"
// and "/dev/my_cfb". One for each device that could be connected
// to the interface.

#ifdef CYGDAT_ALTERA_AVALON_CF_USE_MBR
#define CYGDAT_ALTERA_AVALON_CF_MBR_FLAG 1
#else
#define CYGDAT_ALTERA_AVALON_CF_MBR_FLAG 0
#endif    

#define ALTERA_AVALON_CF_DEVICE_INSTANCE(_name_, _device_, _ident_, _chan_) \
extern alt_avalon_cf_interface_t alt_avalon_cf_interface_##_name_; \
                                                                   \
static alt_avalon_cf_info_t alt_avalon_cf_info_##_name_##_chan_ =  \
{						           \
 ide_base:  (cyg_uint32*) _name_##_IDE_BASE,               \
 chan:      _chan_,					   \
 interface: &alt_avalon_cf_interface_##_name_              \
};                                                         \
                                                           \
DISK_CONTROLLER( alt_avalon_cf_disk_controller_##_name_##_chan_, alt_avalon_cf_disk_controller_##_name_##_chan_ ); \
DISK_CHANNEL(alt_avalon_cf_channel_##_name_##_chan_,       \
             alt_avalon_cf_funs,                           \
             alt_avalon_cf_info_##_name_##_chan_,          \
			 alt_avalon_cf_disk_controller_##_name_##_chan_,		\
             CYGDAT_ALTERA_AVALON_CF_MBR_FLAG,   	   \
             4                                             \
	     );                                            \
                                                           \
BLOCK_DEVTAB_ENTRY(alt_avalon_cf_io_##_name_##_chan_,      \
                   _name_##_IDE_NAME _ident_ "/",          \
                   0,                                      \
                   &cyg_io_disk_devio,                     \
                   alt_avalon_cf_init,                     \
                   alt_avalon_cf_lookup,                   \
                   &alt_avalon_cf_channel_##_name_##_chan_ \
		   )

#define ALTERA_AVALON_CF_INSTANCE(_name_, _device_)        \
  ALTERA_AVALON_CF_DEVICE_INSTANCE(_name_, _device_, "a", 0); \
  ALTERA_AVALON_CF_DEVICE_INSTANCE(_name_, _device_, "b", 1); \
                                                           \
alt_avalon_cf_interface_t alt_avalon_cf_interface_##_name_ = \
{                                                   \
  irq: _name_##_CTL_IRQ,			    \
  init_done: 0,                                     \
  ctl_base:  (cyg_uint32*) _name_##_CTL_BASE,	    \
};

// This header provides the table of devices, which is expanded out 
// using the macro given above.

#include <cyg/hal/devices.h>

