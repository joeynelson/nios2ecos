#ifndef CYGONCE_HAL_IO_H
#define CYGONCE_HAL_IO_H

//=============================================================================
//
//      io.h
//
//      Additional HAL device IO register support.
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
//=============================================================================

// This file defines the hardware I/O macros, and serves a similar function to
// hal_io.h. It is maintained as a seperate file for the benefit of the generic
// headers provided with some SOPC builder components.

#include "cyg/hal/alt_types.h"

#ifndef SYSTEM_BUS_WIDTH
#error SYSTEM_BUS_WIDTH undefined
#endif

// Dynamic bus access functions 

#define __IO_CALC_ADDRESS_DYNAMIC(BASE, OFFSET) \
  ((void *)(((alt_u8*)BASE) + (OFFSET)))

#define IORD_32DIRECT(BASE, OFFSET) \
  __builtin_ldwio (__IO_CALC_ADDRESS_DYNAMIC ((BASE), (OFFSET)))
#define IORD_16DIRECT(BASE, OFFSET) \
  __builtin_ldhuio (__IO_CALC_ADDRESS_DYNAMIC ((BASE), (OFFSET)))
#define IORD_8DIRECT(BASE, OFFSET) \
  __builtin_ldbuio (__IO_CALC_ADDRESS_DYNAMIC ((BASE), (OFFSET)))

#define IOWR_32DIRECT(BASE, OFFSET, DATA) \
  __builtin_stwio (__IO_CALC_ADDRESS_DYNAMIC ((BASE), (OFFSET)), (DATA))
#define IOWR_16DIRECT(BASE, OFFSET, DATA) \
  __builtin_sthio (__IO_CALC_ADDRESS_DYNAMIC ((BASE), (OFFSET)), (DATA))
#define IOWR_8DIRECT(BASE, OFFSET, DATA) \
  __builtin_stbio (__IO_CALC_ADDRESS_DYNAMIC ((BASE), (OFFSET)), (DATA))

// Native bus access functions

#define __IO_CALC_ADDRESS_NATIVE(BASE, REGNUM) \
  ((void *)(((alt_u8*)BASE) + ((REGNUM) * (SYSTEM_BUS_WIDTH/8))))

#define IORD(BASE, REGNUM) \
  __builtin_ldwio (__IO_CALC_ADDRESS_NATIVE ((BASE), (REGNUM)))
#define IOWR(BASE, REGNUM, DATA) \
  __builtin_stwio (__IO_CALC_ADDRESS_NATIVE ((BASE), (REGNUM)), (DATA))

#endif // CYGONCE_HAL_IO_H
