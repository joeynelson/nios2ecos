//========================================================================
//
//      nios2-stub.c
//
//      Helper functions for the GDB stub which are Nios II specific.
//
//========================================================================
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
//========================================================================

#include <stddef.h>

#include <pkgconf/hal.h>

#ifdef CYGPKG_REDBOOT
#include <pkgconf/redboot.h>
#endif

#ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS

#include <cyg/hal/hal_stub.h>
#include <cyg/hal/nios2_opcode.h>

#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/hal/hal_cache.h>

typedef unsigned long t_inst;

/*----------------------------------------------------------------------
 * Asynchronous interrupt support
 */

static struct
{
  t_inst *targetAddr;
  t_inst savedInstr;
} asyncBuffer;

// Called to asynchronously interrupt a running program.
// Must be passed address of instruction interrupted.
// This is typically called in response to a debug port
// receive interrupt.
//

void
install_async_breakpoint(void *epc)
{
  CYGARC_HAL_SAVE_GP();

  asyncBuffer.targetAddr = epc;
  asyncBuffer.savedInstr = *(t_inst *)epc;
  *(t_inst *)epc = *(t_inst *)_breakinst;

  HAL_DCACHE_SYNC();
  HAL_ICACHE_SYNC();

  CYGARC_HAL_RESTORE_GP();
}

//--------------------------------------------------------------------
// Given a trap value TRAP, return the corresponding signal. 

int __computeSignal (unsigned int trap_number)
{
  // Treat everything as a break point 

  if (asyncBuffer.targetAddr != NULL)
  {
    // BP installed by serial driver to stop running program 

    *asyncBuffer.targetAddr = asyncBuffer.savedInstr;
    HAL_DCACHE_SYNC();
    HAL_ICACHE_SYNC();
    asyncBuffer.targetAddr = NULL;
    return SIGINT;
  }
  return SIGTRAP;
}

// Return the trap number corresponding to the last-taken trap. 

int __get_trap_number (void)
{
  return 0;
}

#if defined(CYGSEM_REDBOOT_BSP_SYSCALLS)
int __is_bsp_syscall(void) 
{
    return __get_trap_number() == EXC_SYS;
}
#endif

// Set the currently-saved pc register value to PC. This also updates NPC
// as needed. 

void set_pc (target_register_t pc)
{
  put_register (REG_PC, pc);
}


//----------------------------------------------------------------------
// Single-step support
//

// Saved instruction data for single step support.

static struct
{
  t_inst *targetAddr;
  t_inst savedInstr;
} instrBuffer;


// Set things up so that the next user resume will execute one instruction.

void __single_step (void)
{
  InstFmt inst;
  t_inst *pc = (t_inst *) get_register (REG_PC);

  instrBuffer.targetAddr = pc + 1;              // set default 

  inst.word = *pc;                              // read the next instruction 

  // In the case of a branch, override the default behaviour.

  switch (inst.RType.op) 
  {
  case OP_BEQ:
    if (get_register (inst.IType.a) == get_register (inst.IType.b))
    {
      cyg_int16 offset = inst.IType.imm;
      instrBuffer.targetAddr = (t_inst *) (get_register (REG_PC) + offset + 4);
    }
    break;
  case OP_BGE:
    if ((cyg_int32) get_register (inst.IType.a) >= (cyg_int32) get_register (inst.IType.b))
    {
      cyg_int16 offset = inst.IType.imm;
      instrBuffer.targetAddr = (t_inst *) (get_register (REG_PC) + offset + 4);
    }
    break;
  case OP_BGEU:
    if (get_register (inst.IType.a) >= get_register (inst.IType.b))
    {
      cyg_int16 offset = inst.IType.imm;
      instrBuffer.targetAddr = (t_inst *) (get_register (REG_PC) + offset + 4);
    }
    break;
  case OP_BLT:
    if ((cyg_int32) get_register (inst.IType.a) < (cyg_int32) get_register (inst.IType.b))
    {
      cyg_int16 offset = inst.IType.imm;
      instrBuffer.targetAddr = (t_inst *) (get_register (REG_PC) + offset + 4);
    }
    break;
  case OP_BLTU:
    if (get_register (inst.IType.a) < get_register (inst.IType.b))
    {
      cyg_int16 offset = inst.IType.imm;
      instrBuffer.targetAddr = (t_inst *) (get_register (REG_PC) + offset + 4);
    }
    break;
  case OP_BNE:
    if (get_register (inst.IType.a) != get_register (inst.IType.b))
    {
      cyg_int16 offset = inst.IType.imm;
      instrBuffer.targetAddr = (t_inst *) (get_register (REG_PC) + offset + 4);
    }
    break;
  case OP_BR:
    {
      cyg_int16 offset = inst.IType.imm;
      instrBuffer.targetAddr = (t_inst *) (get_register (REG_PC) + offset + 4);
      break;
    }
  case OP_CALL:
    {
      cyg_uint32 offset = inst.JType.imm * 4;
      instrBuffer.targetAddr = (t_inst *) ((get_register (REG_PC) & 0xf0000000) | offset);
      break;
    }
  case OP_RTYPE:
    switch (inst.RType.opx) {
    case OPX_ERET:
      instrBuffer.targetAddr = (t_inst *) get_register (REG_EA);
      break;
    case OPX_RET:
      instrBuffer.targetAddr = (t_inst *) get_register (REG_RA);
      break;
    case OPX_JMP:
    case OPX_CALLR:
      instrBuffer.targetAddr = (t_inst *) get_register (inst.RType.a);
      break;
    default:
      break;
    }
  default:
    break;
  }
}


/* Clear the single-step state. */

void __clear_single_step (void)
{
  if (instrBuffer.targetAddr != NULL)
    {
      *instrBuffer.targetAddr = instrBuffer.savedInstr;
      instrBuffer.targetAddr = NULL;
    }
  instrBuffer.savedInstr = NOP_INSTR;
}


void __install_breakpoints ()
{
  if (instrBuffer.targetAddr != NULL)
    {
      instrBuffer.savedInstr = *instrBuffer.targetAddr;
      *instrBuffer.targetAddr = __break_opcode ();
    }

  // Ensure that any instructions that are about to be modified aren't in
  // the instruction cache.

  HAL_ICACHE_SYNC();

  // Install the breakpoints in the breakpoint list

  __install_breakpoint_list();

  // Make sure the breakpoints have been written out to memory.

  HAL_DCACHE_SYNC();
}

void __clear_breakpoints (void)
{
  __clear_breakpoint_list();
}


// If the breakpoint we hit is in the breakpoint() instruction, return a
// non-zero value.

int
__is_breakpoint_function ()
{
    return get_register (REG_PC) == (target_register_t)(unsigned long)&_breakinst;
}


// Skip the current instruction.  Since this is only called by the
// stub when the PC points to a breakpoint or trap instruction,
// we can safely just skip 4.

void __skipinst (void)
{
    put_register (REG_PC, get_register (REG_PC) + 4);
}

#endif // CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
