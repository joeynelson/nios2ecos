#ifndef CYGONCE_HAL_PLF_STUB_H
#define CYGONCE_HAL_PLF_STUB_H

//=============================================================================
//
//      plf_stub.h
//
//      Platform header for GDB stub support.
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

#ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS

#include <cyg/infra/cyg_type.h>         // CYG_UNUSED_PARAM


#include <pkgconf/system.h>
#include <pkgconf/hal_nios2.h>

#include <cyg/hal/hal_io.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NUMREGS 38
#define REGSIZE(X) 4

typedef unsigned long target_register_t;

enum regnames {
        REG_ZERO,   REG_AT,      REG_R2,      REG_R3,      REG_R4,       REG_R5,  REG_R6,  REG_R7,
        REG_R8,     REG_R9,      REG_R10,     REG_R11,     REG_R12,      REG_R13, REG_R14, REG_R15,
        REG_R16,    REG_R17,     REG_R18,     REG_R19,     REG_R20,      REG_R21, REG_R22, REG_R23,
        REG_R24,    REG_R25,     REG_GP,      REG_SP,      REG_FP,       REG_EA,  REG_BA,  REG_RA,
        REG_PC,     REG_STATUS,  REG_ESTATUS, REG_BSTATUS, REG_IENABLE,  REG_IPENDING 
};

#define USE_LONG_NAMES_FOR_ENUM_REGNAMES

typedef enum regnames regnames_t;

// Given a trap value TRAP, return the corresponding signal.

extern int __computeSignal (unsigned int trap_number);

// Return the SPARC trap number corresponding to the last-taken trap.

extern int __get_trap_number (void);

// Return the currently-saved value corresponding to register REG.

extern target_register_t get_register (regnames_t reg);

// Store VALUE in the register corresponding to WHICH.

extern void put_register (regnames_t which, target_register_t value);

// Set the currently-saved pc register value to PC.

#if !defined(SET_PC_PROTOTYPE_EXISTS) && !defined(set_pc)
#define SET_PC_PROTOTYPE_EXISTS
extern void set_pc (target_register_t pc);
#endif

// Set things up so that the next user resume will execute one instruction.
// This may be done by setting breakpoints or setting a single step flag
// in the saved user registers, for example.

#ifndef __single_step
void __single_step (void);
#endif

// Clear the single-step state.

void __clear_single_step (void);

extern int __is_bsp_syscall(void);

extern int hal_syscall_handler(void);
    
// If the breakpoint we hit is in the breakpoint() instruction, return a
// non-zero value.

#ifndef __is_breakpoint_function
extern int __is_breakpoint_function (void);
#endif

// Skip the current instruction.

extern void __skipinst (void);

extern void __install_breakpoints (void);

extern void __clear_breakpoints (void);

extern void __install_breakpoint_list (void);

extern void __clear_breakpoint_list (void);

#ifdef __cplusplus
}      // extern "C".
#endif












//----------------------------------------------------------------------------
// Define some platform specific communication details. This is mostly
// handled by hal_if now, but we need to make sure the comms tables are
// properly initialized.

externC void cyg_hal_plf_comms_init(void);

#define HAL_STUB_PLATFORM_INIT_SERIAL()       cyg_hal_plf_comms_init()

#define HAL_STUB_PLATFORM_SET_BAUD_RATE(baud) CYG_UNUSED_PARAM(int, (baud))
#define HAL_STUB_PLATFORM_INTERRUPTIBLE       0
#define HAL_STUB_PLATFORM_INIT_BREAK_IRQ()    CYG_EMPTY_STATEMENT

//----------------------------------------------------------------------------
// Stub initializer.
#define HAL_STUB_PLATFORM_INIT()              CYG_EMPTY_STATEMENT

#endif // ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS

//-----------------------------------------------------------------------------
// Syscall support.
#ifdef CYGPKG_CYGMON
// Cygmon provides syscall handling for this board
#define SIGSYSCALL SIGSYS
extern int __get_syscall_num (void);
#endif

//-----------------------------------------------------------------------------
#endif // CYGONCE_HAL_PLF_STUB_H
// End of plf_stub.h
