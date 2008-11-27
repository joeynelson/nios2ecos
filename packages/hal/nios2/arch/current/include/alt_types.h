#ifndef CYGONCE_ALT_TYPES_H
#define CYGONCE_ALT_TYPES_H

//==========================================================================
//
//      alt_types.h
//
//      Standard type definitions
//
//==========================================================================
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

// This header provides the standard type definitions used by some of the device 
// drivers. These defines are similar, but distinct, from those defined through 
// basetypes.h. They are necessary for drivers that wish to make use of the 
// generic device headers that are supplied with some SOPC builder peripherals.
//
// These headers define register offset, bit masks etc.

typedef char           alt_8;
typedef unsigned char  alt_u8;
typedef short          alt_16;
typedef unsigned short alt_u16;
typedef long           alt_32;
typedef unsigned long  alt_u32;

#define ALT_INLINE        __inline__
#define ALT_ALWAYS_INLINE __attribute__ ((always_inline))
#define ALT_WEAK          __attribute__((weak))

#endif // CYGONCE_ALT_TYPES_H
// End of alt_types.h
