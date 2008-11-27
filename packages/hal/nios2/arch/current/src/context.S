/*=============================================================================##
 * context.S
 *
 * Nios2 context switch code
 *
 *=============================================================================
 *###ECOSGPLCOPYRIGHTBEGIN####
 * -------------------------------------------
 * This file is part of eCos, the Embedded Configurable Operating System.
 *
 * eCos is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 or (at your option) any later version.
 *
 * eCos is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with eCos; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 *
 * As a special exception, if other files instantiate templates or use macros
 * or inline functions from this file, or you compile this file and link it
 * with other works to produce a work based on this file, this file does not
 * by itself cause the resulting work to be covered by the GNU General Public
 * License. However the source code for this file must still be made available
 * in accordance with section (3) of the GNU General Public License.
 *
 * This exception does not invalidate any other reasons why a work based on
 * this file might be covered by the GNU General Public License.
 *
 * -------------------------------------------
 *###ECOSGPLCOPYRIGHTEND####
 *=============================================================================

#include <pkgconf/hal.h>

    /*
     * Explicitly allow the use of r1 (the assembler temporary register)
     * within this code. This register is normally reserved for the use of
     * the compiler.
     */

    .set noat

/*
 *------------------------------------------------------------------------------
 * hal_thread_switch_context
 *
 * Switch thread contexts
 *
 * r4 = address of sp of next thread to execute
 * r5 = address of sp save location of current thread
 */

    .global hal_thread_switch_context
hal_thread_switch_context:

#ifndef CYGDBG_HAL_COMMON_CONTEXT_SAVE_MINIMUM
    /*
     * When using the GDB stubs, it can be useful to save all registers on the stack
     * to allow inspection of the saved thread state in the debugger. However this does 
     * increase the context switch overhead. Therefore the otherwise unecessary 
     * registers are only saved if the option CYGDBG_HAL_COMMON_CONTEXT_SAVE_MINIMUM
     * is disabled.
     */

    addi sp, sp, -140

    /*
     * Store the first few of the caller saved registers, in order
     * to establish some scratch registers.
     */
 
    stw ra,  124(sp)
    stw r15, 120(sp)
    stw r14, 116(sp)
    stw r13, 112(sp)
    stw r12, 108(sp)

    /* 
     * Read the control registers long before using the results.
     * This avoid pipeline stalls.
     */

    rdctl r14, ienable
    rdctl r13, ipending
    rdctl r12, estatus

    /*
     * Store the remainder of the caller saved registers.
     */

    stw r11, 104(sp)
    stw r10, 100(sp)
    stw r9,  96(sp)
    stw r8,  92(sp)
    stw r7,  88(sp)
    stw r6,  84(sp)
    stw r5,  80(sp)
    stw r4,  76(sp)
    stw r3,  72(sp)
    stw r2,  68(sp)
    stw r1,  64(sp)
    stw gp,  60(sp)
    stw et,  56(sp)
    stw ea,  52(sp)

    /*
     * Write out the control register contents which were obtained above.
     */

    stw r14, 136(sp)
    stw r13, 132(sp)
    stw r12, 48(sp)

    /*
     * Write out the value of the stack pointer upon entry
     */

    addi r2, sp, 140
    stw r2, 128(sp)

#else
    addi sp, sp, -48
#endif /* CYGDBG_HAL_COMMON_CONTEXT_SAVE_MINIMUM */

    /* 
     * Save the remaining registers to the stack. 
     */

    rdctl r6, status

    stw r23, 36(sp)
    stw r22, 32(sp)
    stw r21, 28(sp)
    stw r20, 24(sp)
    stw r19, 20(sp)
    stw r18, 16(sp)
    stw r17, 12(sp)
    stw r16, 8(sp)
    stw fp,  4(sp)
    stw ra,  0(sp)

    stw r6, 40(sp)

    /*
     * Save the current stack pointer
     */

    stw sp, (r5)

    /*
     * Now load the destination thread by dropping through
     * to hal_thread_load_context
     */

/*
 *------------------------------------------------------------------------------
 * hal_thread_load_context
 * Load thread context
 * r4 = address of sp of next thread to execute
 * Note that this function is also the second half of hal_thread_switch_context
 * and is simply dropped into from it.
 */
    .global hal_thread_load_context
hal_thread_load_context:

    /*
     * Set the stack pointer to point to the new thread's stack
     */

    ldw sp, (r4) 

    /*
     * Restore R4, this is only really useful for task init.
     */

    ldw r4, 44(sp)

    /*
     * Restore the saved registers for the new task.
     */

#ifndef CYGDBG_HAL_COMMON_CONTEXT_SAVE_MINIMUM
    ldw ra,  124(sp)
    ldw r15, 120(sp)
    ldw r14, 116(sp)
    ldw r13, 112(sp)
    ldw r12, 108(sp)
    ldw r11, 104(sp)
    ldw r10, 100(sp)
    ldw r9,  96(sp)
    ldw r8,  92(sp)
    ldw r7,  88(sp)
    ldw r6,  84(sp)
    ldw r5,  80(sp)
    ldw r4,  76(sp)
    ldw r3,  72(sp)
    ldw r2,  68(sp)
    ldw r1,  64(sp)
    ldw gp,  60(sp)
#endif
    ldw r23, 36(sp)
    ldw r22, 32(sp)
    ldw r21, 28(sp)
    ldw r20, 24(sp)
    ldw r19, 20(sp)
    ldw r18, 16(sp)
    ldw r17, 12(sp)
    ldw r16, 8(sp)
    ldw fp,  4(sp)

    /* disable interrupts */

    wrctl status, r0

    /* restore the exception status registers */

    ldw ea,  0(sp)

    ldw at, 40(sp)
    wrctl estatus, at

#ifndef CYGDBG_HAL_COMMON_CONTEXT_SAVE_MINIMUM	
  ldw sp, 128(sp)
#else
   addi sp, sp, 48
#endif /* CYGDBG_HAL_COMMON_CONTEXT_SAVE_MINIMUM */
 
    /*
     * resume execution of the new task.
     */

    eret

/*------------------------------------------------------------------------------
 * HAL longjmp, setjmp implementations
 * hal_setjmp saves only to callee save registers 16-23, 28, 30, 31[ra], 29[sp]
 * into buffer supplied in a0[arg0]
 * Note: These definitions are repeated in hal_arch.h. If changes are required
 * remember to update both sets.
 */
    .globl hal_setjmp
hal_setjmp:

    stw sp, 40(r4)
    stw r16, 36(r4)
    stw r17, 32(r4)
    stw r18, 28(r4)
    stw r19, 24(r4)
    stw r20, 20(r4)
    stw r21, 16(r4)
    stw r22, 12(r4)
    stw r23, 8(r4)
    stw fp,  4(r4)
    stw ra,  0(r4)
    movi r2, 0
    ret

    .globl hal_longjmp
hal_longjmp:

    /*
     * Restore the saved registers 
     */

    ldw sp, 40(r4)
    ldw r16, 36(r4)
    ldw r17, 32(r4)
    ldw r18, 28(r4)
    ldw r19, 24(r4)
    ldw r20, 20(r4)
    ldw r21, 16(r4)
    ldw r22, 12(r4)
    ldw r23, 8(r4)
    ldw fp,  4(r4)
    ldw ra,  0(r4)

    mov r2, r5

    ret

/*------------------------------------------------------------------------------
 * end of context.S
 */
