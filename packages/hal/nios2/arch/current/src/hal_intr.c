//==========================================================================
//
//      hal_intr.c
//
//      Define HAL interrupt tables
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

#include <pkgconf/hal.h>
#include <cyg/hal/hal_arch.h>           // everything...

#include <cyg/infra/cyg_type.h>
#include <cyg/hal/hal_intr.h>

// Create the interrupt handler table, with all handlers set to the 'safe'
// default.

volatile CYG_ADDRESS hal_interrupt_handlers[CYGNUM_HAL_ISR_COUNT] = 
{(CYG_ADDRESS)HAL_DEFAULT_ISR,
(CYG_ADDRESS)HAL_DEFAULT_ISR,
(CYG_ADDRESS)HAL_DEFAULT_ISR,
(CYG_ADDRESS)HAL_DEFAULT_ISR,
(CYG_ADDRESS)HAL_DEFAULT_ISR,
(CYG_ADDRESS)HAL_DEFAULT_ISR,
(CYG_ADDRESS)HAL_DEFAULT_ISR,
(CYG_ADDRESS)HAL_DEFAULT_ISR,
(CYG_ADDRESS)HAL_DEFAULT_ISR,
(CYG_ADDRESS)HAL_DEFAULT_ISR,
(CYG_ADDRESS)HAL_DEFAULT_ISR,
(CYG_ADDRESS)HAL_DEFAULT_ISR,
(CYG_ADDRESS)HAL_DEFAULT_ISR,
(CYG_ADDRESS)HAL_DEFAULT_ISR,
(CYG_ADDRESS)HAL_DEFAULT_ISR,
(CYG_ADDRESS)HAL_DEFAULT_ISR,
(CYG_ADDRESS)HAL_DEFAULT_ISR,
(CYG_ADDRESS)HAL_DEFAULT_ISR,
(CYG_ADDRESS)HAL_DEFAULT_ISR,
(CYG_ADDRESS)HAL_DEFAULT_ISR,
(CYG_ADDRESS)HAL_DEFAULT_ISR,
(CYG_ADDRESS)HAL_DEFAULT_ISR,
(CYG_ADDRESS)HAL_DEFAULT_ISR,
(CYG_ADDRESS)HAL_DEFAULT_ISR,
(CYG_ADDRESS)HAL_DEFAULT_ISR,
(CYG_ADDRESS)HAL_DEFAULT_ISR,
(CYG_ADDRESS)HAL_DEFAULT_ISR,
(CYG_ADDRESS)HAL_DEFAULT_ISR,
(CYG_ADDRESS)HAL_DEFAULT_ISR,
(CYG_ADDRESS)HAL_DEFAULT_ISR,
(CYG_ADDRESS)HAL_DEFAULT_ISR,
(CYG_ADDRESS)HAL_DEFAULT_ISR};

volatile CYG_ADDRWORD   hal_interrupt_data[CYGNUM_HAL_ISR_COUNT];
volatile CYG_ADDRESS    hal_interrupt_objects[CYGNUM_HAL_ISR_COUNT];

