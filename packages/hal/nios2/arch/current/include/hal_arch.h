#ifndef CYGONCE_HAL_HAL_ARCH_H
#define CYGONCE_HAL_HAL_ARCH_H

//==========================================================================
//
//      hal_arch.h
//
//      Architecture specific abstractions
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
//
//==========================================================================

#ifndef __ASSEMBLER__
#include <pkgconf/hal.h>
#include <cyg/infra/cyg_type.h>

#include <cyg/hal/var_arch.h>

//--------------------------------------------------------------------------
// Macros for accessing the control registers.

#define NIOS2_STATUS   status
#define NIOS2_ESTATUS  estatus
#define NIOS2_BSTATUS  bstatus
#define NIOS2_IENABLE  ienable
#define NIOS2_IPENDING ipending

#define NIOS2_READ_STATUS(dest) \
        __asm__ volatile ("rdctl %0, status" : "=r" (dest))

#define NIOS2_WRITE_STATUS(src) \
        __asm__ volatile ("wrctl status, %0" :: "r" (src))

#define NIOS2_READ_ESTATUS(dest) \
        __asm__ volatile ("rdctl %0, estatus" : "=r" (dest))

#define NIOS2_READ_BSTATUS(dest) \
        __asm__ volatile ("rdctl %0, bstatus" : "=r" (dest))

#define NIOS2_READ_IENABLE(dest) \
        __asm__ volatile ("rdctl %0, ienable" : "=r" (dest))

#define NIOS2_WRITE_IENABLE(src) \
        __asm__ volatile ("wrctl ienable, %0" :: "r" (src))

#define NIOS2_READ_IPENDING(dest) \
        __asm__ volatile ("rdctl %0, ipending" : "=r" (dest))

//--------------------------------------------------------------------------
// Processor saved states. This structure defines the layout of the 
// registers as they are saved on the stack after a context switch. If you 
// change this structure, you will need to make the equivalent changes in 
// vectors.S and context.S.

typedef struct 
{
  CYG_WORD32 pc;
  CYG_WORD32 fp;
  CYG_WORD32 d_high[8];
  CYG_WORD32 status;
  CYG_WORD32 reg_arg0;
  CYG_WORD32 estatus;
  CYG_WORD32 ea;
  CYG_WORD32 et;
  CYG_WORD32 gp;
  CYG_WORD32 d_low[15];
  CYG_WORD32 ra;
  CYG_WORD32 sp;
  CYG_WORD32 ipending;
  CYG_WORD32 ienable;
} HAL_SavedRegisters;

//--------------------------------------------------------------------------
// Bit manipulation macros

externC cyg_uint32 hal_lsbit_index(cyg_uint32 mask);
externC cyg_uint32 hal_msbit_index(cyg_uint32 mask);

#define HAL_LSBIT_INDEX(index, mask) index = hal_lsbit_index(mask);

#define HAL_MSBIT_INDEX(index, mask) index = hal_msbit_index(mask);

//--------------------------------------------------------------------------
// Context Initialization
//
// Initialize the context of a thread.
// Arguments:
// _sparg_ name of variable containing current sp, will be written with new sp
// _thread_ thread object address, passed as argument to entry point
// _entry_ entry point address.
// _id_ bit pattern used in initializing registers, for debugging.

extern int _gp;

#define HAL_THREAD_INIT_CONTEXT( _sparg_, _thread_, _entry_, _id_ )                      \
{                                                                                        \
  register CYG_WORD _sp_ = ((CYG_WORD)_sparg_) - 56;                                     \
  register HAL_SavedRegisters *_regs_;                                                   \
  int _i_;                                                                               \
  _sp_ = _sp_ & 0xFFFFFFF0;                                                              \
  _regs_ = (HAL_SavedRegisters *)(((_sp_) - sizeof(HAL_SavedRegisters))&0xFFFFFFF0);     \
  (_regs_)->d_low[3] = (CYG_WORD32)(_thread_);                                           \
  (_regs_)->reg_arg0 = (CYG_WORD32)(_thread_); /* Input argument is the thread object */ \
  (_regs_)->status  = 1;                       /* Interrupts enabled                  */ \
  (_regs_)->gp = (CYG_WORD32) &_gp;            /* GP = the current global pointer     */ \
  (_regs_)->fp = (CYG_WORD32)(_sp_);           /* FP = top of stack                   */ \
  (_regs_)->sp = (CYG_WORD32)(_sp_);           /* SP = top of stack                   */ \
  (_regs_)->pc = (CYG_WORD32)(_entry_);        /* RA(d[31]) = entry point             */ \
   _sparg_ = (CYG_ADDRESS)_regs_;              /* return the new stack pointer        */ \
}

//--------------------------------------------------------------------------
// Context switch macros.
// The arguments are pointers to locations where the stack pointer
// of the current thread is to be stored, and from where the sp of the
// next thread is to be fetched.

#ifdef __cplusplus
 externC
#endif
	void hal_thread_switch_context( CYG_ADDRESS to, CYG_ADDRESS from );
#ifdef __cplusplus
 externC
#endif
 void hal_thread_load_context( CYG_ADDRESS to )
	     __attribute__ ((noreturn));


#define HAL_THREAD_SWITCH_CONTEXT(_fspptr_,_tspptr_)                    \
        hal_thread_switch_context( (CYG_ADDRESS)_tspptr_,               \
                                   (CYG_ADDRESS)_fspptr_);

#define HAL_THREAD_LOAD_CONTEXT(_tspptr_)                               \
        hal_thread_load_context( (CYG_ADDRESS)_tspptr_ );

//--------------------------------------------------------------------------
// Execution reorder barrier.
// When optimizing the compiler can reorder code. In multithreaded systems
// where the order of actions is vital, this can sometimes cause problems.
// This macro may be inserted into places where reordering should not happen.
// The "memory" keyword is potentially unnecessary, but it is harmless to
// keep it.

#define HAL_REORDER_BARRIER() asm volatile ( "" : : : "memory" )

//--------------------------------------------------------------------------
// Breakpoint support
// HAL_BREAKPOINT() is a code sequence that will cause a breakpoint to
// happen if executed.
// HAL_BREAKINST is the value of the breakpoint instruction and
// HAL_BREAKINST_SIZE is its size in bytes.
// HAL_BREAKINST_TYPE is the type.

// In the case of Nios II, the trap instruction is used to generate a 
// breakpoint for the GDB stub. The break instruction is reserved for the 
// use of the JTAG debugger.

#define HAL_BREAKPOINT(_label_)                 \
asm volatile (" .globl  " #_label_ "\n"         \
              #_label_":"                       \
              "trap"                            \
    );

#define HAL_BREAKINST           0x003b683a

#define HAL_BREAKINST_SIZE      4

#define HAL_BREAKINST_TYPE      cyg_uint32

//--------------------------------------------------------------------------
// Thread register state manipulation for GDB support.

// Set a 32 bit register size for GDB register dumps.

#define CYG_HAL_GDB_REG CYG_WORD32

// Translate a stack pointer as saved by the thread context macros above into
// a pointer to a HAL_SavedRegisters structure.

#define HAL_THREAD_GET_SAVED_REGISTERS( _sp_, _regs_ )          \
        (_regs_) = (HAL_SavedRegisters *)(_sp_)

// Copy a set of registers from a HAL_SavedRegisters structure into a
// GDB ordered array.

#define HAL_GET_GDB_REGISTERS( _aregval_ , _regs_ )             \
{                                                               \
    CYG_HAL_GDB_REG *_regval_ = (CYG_HAL_GDB_REG *)(_aregval_); \
    int _i_;                                                    \
                                                                \
    _regval_[0] = 0;                                            \
    for( _i_ = 1; _i_ < 16; _i_++ )                             \
      _regval_[_i_] = (_regs_)->d_low[_i_-1];                   \
    for( _i_ = 16; _i_ < 24; _i_++ )                            \
        _regval_[_i_] = (_regs_)->d_high[_i_-16];               \
    _regval_[24] = (_regs_)->et;                                \
    _regval_[25] = 0xbad;                                       \
    _regval_[26] = (_regs_)->gp;                                \
    _regval_[27] = (_regs_)->sp;                                \
    _regval_[28] = (_regs_)->fp;                                \
    _regval_[29] = (_regs_)->ea;                                \
    _regval_[30] = 0xbad;                                       \
    _regval_[31] = (_regs_)->ra;                                \
    _regval_[32] = (_regs_)->pc;                                \
    _regval_[33] = (_regs_)->status;                            \
    _regval_[34] = (_regs_)->estatus;                           \
    _regval_[35] = 0xbad;                                       \
    _regval_[36] = (_regs_)->ienable;                           \
    _regval_[37] = (_regs_)->ipending;                          \
}

// Copy a GDB ordered array into a HAL_SavedRegisters structure.
// Note, not all registers can be modified. In particular the
// control registers can not be changed using the debugger.

#define HAL_SET_GDB_REGISTERS( _regs_ , _aregval_ )             \
{                                                               \
    CYG_HAL_GDB_REG *_regval_ = (CYG_HAL_GDB_REG *)(_aregval_); \
    int _i_;                                                    \
    for( _i_ = 1; _i_ < 16; _i_++ )                             \
      (_regs_)->d_low[_i_-1] = _regval_[_i_];                   \
    for( _i_ = 16; _i_ < 24; _i_++ )                            \
      (_regs_)->d_high[_i_-16] = _regval_[_i_];                 \
    (_regs_)->gp = _regval_[26];                                \
    (_regs_)->sp = _regval_[27];                                \
    (_regs_)->fp = _regval_[28];                                \
    (_regs_)->ra = _regval_[31];                                \
    (_regs_)->pc = _regval_[32];                                \
    (_regs_)->reg_arg0 = _regval_[4];				\
}

//--------------------------------------------------------------------------
// HAL setjmp

#define CYGARC_JMP_BUF_SP        0
#define CYGARC_JMP_BUF_R16       1
#define CYGARC_JMP_BUF_R17       2
#define CYGARC_JMP_BUF_R18       3
#define CYGARC_JMP_BUF_R19       4
#define CYGARC_JMP_BUF_R20       5
#define CYGARC_JMP_BUF_R21       6
#define CYGARC_JMP_BUF_R22       7
#define CYGARC_JMP_BUF_R23       8
#define CYGARC_JMP_BUF_R28       9
#define CYGARC_JMP_BUF_R31      11

#define CYGARC_JMP_BUF_SIZE     11

typedef cyg_uint32 hal_jmp_buf[CYGARC_JMP_BUF_SIZE];

externC int  hal_setjmp(hal_jmp_buf env);
externC void hal_longjmp(hal_jmp_buf env, int val);

//-------------------------------------------------------------------------
// Idle thread code.
// This macro is called in the idle thread loop, and gives the HAL the
// chance to insert code. Typical idle thread behaviour might be to halt the
// processor.

externC void hal_idle_thread_action(cyg_uint32 loop_count);

#define HAL_IDLE_THREAD_ACTION(_count_) hal_idle_thread_action(_count_)

//--------------------------------------------------------------------------
// Minimal and sensible stack sizes: the intention is that applications
// will use these to provide a stack size in the first instance prior to
// proper analysis.  Idle thread stack should be this big.

//    THESE ARE NOT INTENDED TO BE MICROMETRICALLY ACCURATE FIGURES.
//           THEY ARE HOWEVER ENOUGH TO START PROGRAMMING.
// YOU MUST MAKE YOUR STACKS LARGER IF YOU HAVE LARGE "AUTO" VARIABLES!

// This is not a config option because it should not be adjusted except
// under "enough rope" sort of disclaimers.

// Typical case stack frame size: return link + 4 pushed registers + some locals.

#define CYGNUM_HAL_STACK_FRAME_SIZE (48)

// Stack needed for a context switch:

#define CYGNUM_HAL_STACK_CONTEXT_SIZE ((32+10)*4)

// Interrupt + call to ISR, interrupt_end() and the DSR

#define CYGNUM_HAL_STACK_INTERRUPT_SIZE (4+2*CYGNUM_HAL_STACK_CONTEXT_SIZE) 

#ifdef CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK

// An interrupt stack which is large enough for all possible interrupt
// conditions (and only used for that purpose) exists.  "User" stacks
// can be much smaller

#define CYGNUM_HAL_STACK_SIZE_MINIMUM (CYGNUM_HAL_STACK_CONTEXT_SIZE+      \
                                       CYGNUM_HAL_STACK_INTERRUPT_SIZE*2+  \
                                       CYGNUM_HAL_STACK_FRAME_SIZE*16)
#define CYGNUM_HAL_STACK_SIZE_TYPICAL (CYGNUM_HAL_STACK_SIZE_MINIMUM+2024)

#else // CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK 

// No separate interrupt stack exists.  Make sure all threads contain
// a stack sufficiently large.

#define CYGNUM_HAL_STACK_SIZE_MINIMUM (4096)
#define CYGNUM_HAL_STACK_SIZE_TYPICAL (4096)

#endif

#endif /* __ASSEMBLER__ */

// Convenience macros for accessing memory cached or uncached

#define CYGARC_CACHED_ADDRESS(x) (((CYG_ADDRESS)(x)) & 0x7FFFFFFF)
#define CYGARC_UNCACHED_ADDRESS(x) (((CYG_ADDRESS)(x)) | 0x80000000) 
#define CYGARC_PHYSICAL_ADDRESS(x) ((CYG_ADDRESS)(x))

//--------------------------------------------------------------------------
// Macros for switching context between two eCos instances (jump from
// code in ROM to code in RAM or vice versa).

#define CYGARC_HAL_SAVE_GP()                              \
    CYG_MACRO_START                                       \
    register CYG_ADDRWORD __gp_save;                      \
    asm volatile ( "mov %0, gp \n\t" : "=r"(__gp_save));  \
    asm volatile ( ".extern _gp\n\t"                      \
                   "movhi gp,%hiadj(_gp)\n\t"             \
                   "addi gp, gp,%lo(_gp)");               \
                   

#define CYGARC_HAL_RESTORE_GP()                           \
    asm volatile ( "mov   gp,%0 " :: "r"(__gp_save) );    \
    CYG_MACRO_END

//--------------------------------------------------------------------------
// Macro for finding return address. 

#define CYGARC_HAL_GET_RETURN_ADDRESS(_x_, _dummy_)       \
  asm volatile ( "mov %0, ra" : "=r" (_x_) )

#define CYGARC_HAL_GET_RETURN_ADDRESS_BACKUP(_dummy_)

//--------------------------------------------------------------------------
#endif // CYGONCE_HAL_HAL_ARCH_H
// End of hal_arch.h
