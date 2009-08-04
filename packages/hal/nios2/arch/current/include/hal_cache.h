#ifndef CYGONCE_HAL_CACHE_H
#define CYGONCE_HAL_CACHE_H

//=============================================================================
//
//      hal_cache.h
//
//      HAL cache control API
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

#include <pkgconf/hal.h>
#include <cyg/infra/cyg_type.h>

#include <cyg/hal/system.h>

//-----------------------------------------------------------------------------
// Cache dimensions.

// Data cache

#if NIOS2_DCACHE_SIZE != 0
#define HAL_DCACHE_SIZE         NIOS2_DCACHE_SIZE      // Size of data cache in bytes
#define HAL_DCACHE_LINE_SIZE    NIOS2_DCACHE_LINE_SIZE // Size of a data cache line
#define HAL_DCACHE_WAYS         1                      // Associativity of the cache
#endif // NIOS2_DCACHE_SIZE 

// Instruction cache

#define HAL_ICACHE_SIZE         NIOS2_ICACHE_SIZE       // Size of cache in bytes
#define HAL_ICACHE_LINE_SIZE    NIOS2_ICACHE_LINE_SIZE  // Size of a cache line
#define HAL_ICACHE_WAYS         1                       // Associativity of the cache

#if NIOS2_DCACHE_SIZE != 0
#define HAL_DCACHE_SETS (HAL_DCACHE_SIZE/(HAL_DCACHE_LINE_SIZE*HAL_DCACHE_WAYS))
#endif // NIOS2_DCACHE_SIZE
#define HAL_ICACHE_SETS (HAL_ICACHE_SIZE/(HAL_ICACHE_LINE_SIZE*HAL_ICACHE_WAYS))

//-----------------------------------------------------------------------------
// Global control of data cache

// This macro is used to enable the data cache, however the cache is always on for 
// Nios II.

#define HAL_DCACHE_ENABLE()

// Disable the data cache. This is also not possible.

#define HAL_DCACHE_DISABLE()

#if NIOS2_DCACHE_SIZE != 0
#define HAL_DCACHE_IS_ENABLED(_state_) (_state_) = 1

// Invalidate the entire cache
// We simply use HAL_DCACHE_SYNC() to do this. For writeback caches this
// is not quite what we want, but there is no index-invalidate operation
// available.

#define HAL_DCACHE_INVALIDATE_ALL() HAL_DCACHE_INVALIDATE(0, NIOS2_DCACHE_SIZE)

// Synchronize the contents of the cache with memory.
// This uses the index-writeback-invalidate operation.

#define HAL_DCACHE_SYNC() HAL_DCACHE_FLUSH(0, NIOS2_DCACHE_SIZE)
    
// It is not possible to lock entries in the Nios II data cache, so no
// implementation is provided for these macros.
// Flushing must happen on 4 byte aligned addresses

#define HAL_DCACHE_FLUSH( _base_ , _asize_ )                     \
CYG_MACRO_START                                                  \
    char* __i__;        											 \
    char* __start__ = (char*)(((cyg_uint32)_base_) & 0xffffff00);       \
    char* __end__;                                                   \
    cyg_uint32 __size__ = _asize_ + ((cyg_uint32)(_asize_ ) - (cyg_uint32)(__start__));                                   \
                                                                 \
    if (__size__ > NIOS2_DCACHE_SIZE)                                \
    {                                                            \
      __size__ = NIOS2_DCACHE_SIZE;                                  \
    }                                                            \
                                                                 \
    __end__ = ((char*)(__start__)) + __size__;                               \
                                                                 \
    for (__i__ = (__start__); __i__ < __end__; __i__+= NIOS2_DCACHE_LINE_SIZE)       \
    {                                                            \
      __asm__ volatile ("flushd (%0)" :: "r" (__i__));               \
    }                                                            \
                                                                 \
    if (((cyg_uint32)(__start__)) & (NIOS2_DCACHE_LINE_SIZE - 1))    \
    {                                                            \
      __asm__ volatile ("flushd (%0)" :: "r" (__i__));               \
    }                                                            \
CYG_MACRO_END

#define HAL_DCACHE_INVALIDATE( _base_ , _asize_ ) \
CYG_MACRO_START                                                  \
    char* __i__;        											 \
    char* __start__ = (char*)(((cyg_uint32)_base_) & 0xffffff00);       \
    char* __end__;                                                   \
    cyg_uint32 __size__ = _asize_ + ((cyg_uint32)(_asize_ )- (cyg_uint32)(__start__));                                   \
                                                                 \
    if (__size__ > NIOS2_DCACHE_SIZE)                                \
    {                                                            \
      __size__ = NIOS2_DCACHE_SIZE;                                  \
    }                                                            \
                                                                 \
    __end__ = ((char*)(__start__)) + __size__;                               \
                                                                 \
    for (__i__ = (__start__); __i__ < __end__; __i__+= NIOS2_DCACHE_LINE_SIZE)       \
    {                                                            \
      __asm__ volatile ("initd (%0)" :: "r" (__i__));               \
    }                                                            \
                                                                 \
    if (((cyg_uint32)(__start__)) & (NIOS2_DCACHE_LINE_SIZE - 1))    \
    {                                                            \
      __asm__ volatile ("initd (%0)" :: "r" (__i__));               \
    }                                                            \
CYG_MACRO_END

#define HAL_DCACHE_STORE( _base_ , _asize_ )  \
  HAL_DCACHE_FLUSH( _base_ , _asize_ )   

#else
#define HAL_DCACHE_INVALIDATE_ALL()
#define HAL_DCACHE_SYNC()
#define HAL_DCACHE_IS_ENABLED(_state_) (_state_) = 0
#define HAL_DCACHE_FLUSH( _base_ , _asize_ ) 
#endif // NIOS2_DCACHE_SIZE > 0

//-----------------------------------------------------------------------------
// Global control of Instruction cache

// Enable the instruction cache

#define HAL_ICACHE_ENABLE()

// Disable the instruction cache

#define HAL_ICACHE_DISABLE()

#define HAL_ICACHE_IS_ENABLED(_state_) (_state_) = 1;

// Invalidate the entire cache

#define HAL_ICACHE_INVALIDATE_ALL() HAL_ICACHE_INVALIDATE(0, NIOS2_ICACHE_SIZE)

// Synchronize the contents of the cache with memory.
// Simply force the cache to reload.

#define HAL_ICACHE_SYNC() HAL_ICACHE_INVALIDATE_ALL()

//-----------------------------------------------------------------------------
// Instruction cache line control

// Invalidate cache lines in the given range without writing to memory.
// This uses the hit-invalidate cache operation.


#if NIOS2_ICACHE_SIZE > 0
#define HAL_ICACHE_INVALIDATE( _base_ , _asize_ )             \
CYG_MACRO_START                                               \
    char* __i__;                                                  \
    char* __start__ = _base_;                                     \
    char* __end__;                                                \
    cyg_uint32 __size__ = _asize_;                                \
                                                              \
    if (__size__ > NIOS2_ICACHE_SIZE)                             \
    {                                                         \
      __size__ = NIOS2_ICACHE_SIZE;                               \
   }                                                          \
                                                              \
    __end__ = ((char*) __start__) + __size__;                            \
                                                              \
    for (__i__ = __start__; __i__ < __end__; __i__+= NIOS2_ICACHE_LINE_SIZE)     \
    {                                                         \
      __asm__ volatile ("initi %0" :: "r" (__i__));               \
    }                                                         \
                                                              \
    if (((cyg_uint32) __start__) & (NIOS2_ICACHE_LINE_SIZE - 1)) \
    {                                                         \
      __asm__ volatile ("initi %0" :: "r" (__i__));               \
    }                                                         \
                                                              \
    __asm__ volatile ("flushp");                              \
CYG_MACRO_END

#define HAL_ICACHE_FLUSH( _base_ , _asize_ ) \
CYG_MACRO_START                                               \
    char* __i__;                                                  \
    char* __start__ = _base_;                                     \
    char* __end__;                                                \
    cyg_uint32 __size__ = _asize_;                                \
                                                              \
    if (__size__ > NIOS2_ICACHE_SIZE)                             \
    {                                                         \
      __size__ = NIOS2_ICACHE_SIZE;                               \
   }                                                          \
                                                              \
    __end__ = ((char*) __start__) + __size__;                            \
                                                              \
    for (__i__ = __start__; __i__ < __end__; __i__+= NIOS2_ICACHE_LINE_SIZE)     \
    {                                                         \
      __asm__ volatile ("flushi %0" :: "r" (__i__));               \
    }                                                         \
                                                              \
    if (((cyg_uint32) __start__) & (NIOS2_ICACHE_LINE_SIZE - 1)) \
    {                                                         \
      __asm__ volatile ("flushi %0" :: "r" (__i__));               \
    }                                                         \
                                                              \
    __asm__ volatile ("flushp");                              \
CYG_MACRO_END

#else
#define HAL_ICACHE_INVALIDATE( _base_ , _asize_ ) 
#define HAL_ICACHE_FLUSH( _base_ , _asize_ ) 
#endif // NIOS2_ICACHE_SIZE > 0 

//-----------------------------------------------------------------------------
#endif // ifndef CYGONCE_HAL_CACHE_H
// End of hal_cache.h
