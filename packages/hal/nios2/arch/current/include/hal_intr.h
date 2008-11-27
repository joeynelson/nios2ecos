#ifndef CYGONCE_HAL_HAL_INTR_H
#define CYGONCE_HAL_HAL_INTR_H

//==========================================================================
//
//      hal_intr.h
//
//      HAL Interrupt and clock support
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

#include <pkgconf/hal.h>

#include <cyg/infra/cyg_type.h>
#include <cyg/hal/hal_io.h>
#include <cyg/hal/hal_arch.h>

#include <cyg/hal/system.h>

#define CYGNUM_HAL_ISR_COUNT 32

#define CYGNUM_HAL_ISR_MIN   0
#define CYGNUM_HAL_ISR_MAX   31

//--------------------------------------------------------------------------
// Nios2 vectors.

// These are the exception codes to use for HAL_VSR_GET/SET

#define CYGNUM_HAL_VECTOR_INTERRUPT            1
#define CYGNUM_HAL_VECTOR_BREAKPOINT           0

#define CYGNUM_HAL_VSR_MIN                     0
#define CYGNUM_HAL_VSR_MAX                     1
#define CYGNUM_HAL_VSR_COUNT                   2

#define CYGNUM_HAL_EXCEPTION_TRAP              0 

#define CYGNUM_HAL_EXCEPTION_MIN     CYGNUM_HAL_EXCEPTION_TRAP
#define CYGNUM_HAL_EXCEPTION_MAX     CYGNUM_HAL_EXCEPTION_TRAP
#define CYGNUM_HAL_EXCEPTION_COUNT   (CYGNUM_HAL_EXCEPTION_MAX - \
                                      CYGNUM_HAL_EXCEPTION_MIN + 1)

//--------------------------------------------------------------------------
// Static data used by HAL

// ISR tables

externC volatile CYG_ADDRESS    hal_interrupt_handlers[CYGNUM_HAL_ISR_COUNT];
externC volatile CYG_ADDRWORD   hal_interrupt_data[CYGNUM_HAL_ISR_COUNT];
externC volatile CYG_ADDRESS    hal_interrupt_objects[CYGNUM_HAL_ISR_COUNT];

// VSR table

externC volatile CYG_ADDRESS    hal_vsr_table[CYGNUM_HAL_VSR_MAX+1] __attribute ((section (".rom_monitor_rwdata")));

//--------------------------------------------------------------------------
// Default ISR
// The define is used to test whether this routine exists, and to allow
// us to call it.

externC cyg_uint32 hal_default_isr(CYG_ADDRWORD vector, CYG_ADDRWORD data);

#define HAL_DEFAULT_ISR hal_default_isr

//--------------------------------------------------------------------------
// Interrupt and VSR attachment macros

#define HAL_INTERRUPT_IN_USE( _vector_, _state_)                            \
    CYG_MACRO_START                                                         \
    cyg_uint32 _index_;                                                     \
    HAL_TRANSLATE_VECTOR ((_vector_), _index_);                             \
                                                                            \
    if( hal_interrupt_handlers[_index_] == (CYG_ADDRESS)HAL_DEFAULT_ISR )   \
        (_state_) = 0;                                                      \
    else                                                                    \
        (_state_) = 1;                                                      \
    CYG_MACRO_END

#define HAL_INTERRUPT_ATTACH( _vector_, _isr_, _data_, _object_ )           \
    CYG_MACRO_START                                                         \
    cyg_uint32 _index_;                                                     \
    HAL_TRANSLATE_VECTOR( _vector_, _index_ );                              \
                                                                            \
    if( hal_interrupt_handlers[_index_] == (CYG_ADDRESS)HAL_DEFAULT_ISR )   \
    {                                                                       \
        hal_interrupt_handlers[_index_] = (CYG_ADDRESS)_isr_;               \
        hal_interrupt_data[_index_] = (CYG_ADDRWORD)_data_;                 \
        hal_interrupt_objects[_index_] = (CYG_ADDRESS)_object_;             \
    }                                                                       \
    CYG_MACRO_END

#define HAL_INTERRUPT_DETACH( _vector_, _isr_ )                             \
    CYG_MACRO_START                                                         \
    cyg_uint32 _index_;                                                     \
    HAL_TRANSLATE_VECTOR( _vector_, _index_ );                              \
                                                                            \
    if( hal_interrupt_handlers[_index_] == (CYG_ADDRESS)_isr_ )             \
    {                                                                       \
        hal_interrupt_handlers[_index_] = (CYG_ADDRESS)HAL_DEFAULT_ISR;     \
        hal_interrupt_data[_index_] = 0;                                    \
        hal_interrupt_objects[_index_] = 0;                                 \
    }                                                                       \
    CYG_MACRO_END

#define HAL_VSR_GET( _vector_, _pvsr_ )                                     \
    *(_pvsr_) = (void (*)())hal_vsr_table[_vector_];
    

#define HAL_VSR_SET( _vector_, _vsr_, _poldvsr_ ) CYG_MACRO_START           \
    if( (void*)_poldvsr_ != NULL)                                           \
        *(CYG_ADDRESS *)_poldvsr_ = (CYG_ADDRESS)hal_vsr_table[_vector_];   \
    hal_vsr_table[_vector_] = (CYG_ADDRESS)_vsr_;                           \
CYG_MACRO_END

// This is an ugly name, but what it means is: grab the VSR back to eCos
// internal handling, or if you like, the default handler. But if
// cooperating with GDB and CygMon, the default behaviour is to pass most
// exceptions to CygMon. This macro undoes that so that eCos handles the
// exception.  So use it with care.

externC void __default_exception_vsr(void);
externC void __default_interrupt_vsr(void);
externC void __break_vsr_springboard(void);

#define HAL_VSR_SET_TO_ECOS_HANDLER( _vector_, _poldvsr_ ) CYG_MACRO_START  \
    HAL_VSR_SET( _vector_, _vector_ == CYGNUM_HAL_VECTOR_INTERRUPT          \
                              ? (CYG_ADDRESS)__default_interrupt_vsr        \
                              : _vector_ == CYGNUM_HAL_VECTOR_BREAKPOINT    \
                                ? (CYG_ADDRESS)__break_vsr_springboard      \
                                : (CYG_ADDRESS)__default_exception_vsr,     \
                 _poldvsr_ );                                               \
CYG_MACRO_END

//--------------------------------------------------------------------------
// Interrupt state storage

typedef cyg_uint32 CYG_INTERRUPT_STATE;

#define HAL_DISABLE_INTERRUPTS(_old_) \
  CYG_MACRO_START                     \
  NIOS2_READ_STATUS (_old_);          \
  NIOS2_WRITE_STATUS (0);             \
  CYG_MACRO_END

#define HAL_ENABLE_INTERRUPTS() NIOS2_WRITE_STATUS (1);

#define HAL_RESTORE_INTERRUPTS(_old_) NIOS2_WRITE_STATUS (_old_);

#define HAL_QUERY_INTERRUPTS( _state_ ) NIOS2_READ_STATUS (_state_);

//--------------------------------------------------------------------------
// Routine to execute DSRs using separate interrupt stack

#ifdef  CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK

externC void hal_interrupt_stack_call_pending_DSRs(void);

#define HAL_INTERRUPT_STACK_CALL_PENDING_DSRS() \
    hal_interrupt_stack_call_pending_DSRs()

// these are offered solely for stack usage testing
// if they are not defined, then there is no interrupt stack.

#define HAL_INTERRUPT_STACK_BASE cyg_interrupt_stack_base
#define HAL_INTERRUPT_STACK_TOP  cyg_interrupt_stack
#endif

#define HAL_TRANSLATE_VECTOR(_vector_,_index_) (_index_) = (_vector_)

//--------------------------------------------------------------------------
// Enable and disable individual interrupts

#define HAL_INTERRUPT_MASK( _vector_ )          \
  CYG_MACRO_START                               \
  cyg_uint32 _status_;                          \
  cyg_uint32 _alt_irq_active_;                  \
                                                \
  HAL_DISABLE_INTERRUPTS(_status_);             \
                                                \
  NIOS2_READ_IENABLE (_alt_irq_active_);        \
  _alt_irq_active_ &= ~(1 << _vector_);         \
  NIOS2_WRITE_IENABLE (_alt_irq_active_);       \
                                                \
  HAL_RESTORE_INTERRUPTS(_status_);             \
  CYG_MACRO_END

#define HAL_INTERRUPT_UNMASK( _vector_ )        \
  CYG_MACRO_START                               \
  unsigned int  _status_;                       \
  unsigned int _alt_irq_active_;                \
                                                \
  HAL_DISABLE_INTERRUPTS(_status_);             \
                                                \
  NIOS2_READ_IENABLE (_alt_irq_active_);        \
  _alt_irq_active_ |= (1 << _vector_);          \
  NIOS2_WRITE_IENABLE (_alt_irq_active_);       \
                                                \
  HAL_RESTORE_INTERRUPTS(_status_);             \
  CYG_MACRO_END

#define HAL_INTERRUPT_ACKNOWLEDGE( _vector_ )

#define HAL_INTERRUPT_CONFIGURE( _vector_, _level_, _up_ )

#define HAL_INTERRUPT_SET_LEVEL( _vector_, _level_ )

//----------------------------------------------------------------------------
// Reset.

#define HAL_PLATFORM_RESET()                   \
  NIOS2_WRITE_STATUS(0);                       \
  NIOS2_WRITE_IENABLE(0);                      \
  ((void (*) (void)) NIOS2_RESET_ADDR) ()

#define HAL_PLATFORM_RESET_ENTRY NIOS2_RESET_ADDR

//--------------------------------------------------------------------------
// Clock routines. 
//
// These are supplied by the timer device driver.

externC void hal_clock_initialize (cyg_uint32 _period_);
externC void hal_clock_reset (cyg_uint32 _vector_, cyg_uint32 _period_);
externC cyg_uint32 hal_clock_read (void);

#define HAL_CLOCK_INITIALIZE( _period_ ) hal_clock_initialize (_period_)

#define HAL_CLOCK_RESET( _vector_, _period_ ) hal_clock_reset( _vector_, _period_ )

#define HAL_CLOCK_READ( _pvalue_ ) *_pvalue_ = hal_clock_read ()

//--------------------------------------------------------------------------
// Microsecond delay function provided in hal_misc.c

externC void hal_delay_us(int us);

#define HAL_DELAY_US(n)          hal_delay_us(n)

//--------------------------------------------------------------------------
#endif // ifndef CYGONCE_HAL_HAL_INTR_H
// End of hal_intr.h
