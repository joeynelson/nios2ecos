/*=============================================================================
 *
 *  vectors.S
 *
 *  Nios II exception vectors
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
 */

#include <cyg/hal/system.h>
#include <pkgconf/hal_nios2.h>
#include <pkgconf/system.h>
#include <pkgconf/mlt_nios2.h>
#include <pkgconf/hal.h>

/*
 * Set up some useful defines that locate the top and the bottom of the
 * initial/interrupt stack.
 */

#define STACK_BASE (CYGMEM_REGION_ram + CYGMEM_REGION_ram_SIZE)
#define STACK_END (STACK_BASE - CYGNUM_HAL_COMMON_INTERRUPTS_STACK_SIZE)

    /*
     * Linker defined symbols used to initialise bss.
     */

    .globl __bss_start
    .globl __bss_end

    /*
     * GCC 3.4.1 defines a C++ exception handler that is incompatible with eCos.
     * To avoid pulling this in, an alternative is provided here.
     */

#if __GNUC__ == 3
#if __GNUC_MINOR__ == 4
#if __GNUC_PATCHLEVEL__ == 1
	.globl _ZN10__cxxabiv119__terminate_handlerE
#endif /* __GNUC_PATCHLEVEL__ */
#endif /* __GNUC_MINOR__ */
#endif /* __GNUC__ */
    /*
     * ========================================================================
     * _hardware_reset
     *
     * This is the reset entry point for Nios II.
     *
     * At reset, only the cache line which contain the reset vector is
     * initialized. Therefore the code within the first cache line is required
     * to initialize the instruction cache for the remainder of the code.
     *
     * Note that the automatically generated linker script requires that
     * the .init section is less than 0x20 bytes long.
     */

    .section .entry, "xa"
    .align 2

    /*
     * Some tools want to know where the reset vector is.
     */

    .globl _hardware_reset

     /*
      * Explicitly allow the use of r1 (the assembler temporary register)
      * within this code. This register is normally reserved for the use of
      * the compiler.
      */

    .set noat

#ifdef CYG_HAL_STARTUP_ROM

_hardware_reset:

    /* Initialize all cache lines within the instruction cache */

#if NIOS2_ICACHE_SIZE > 0

    /*
     * The assumption here is that the instruction cache size is always
     * a power of two.
     */

#if NIOS2_ICACHE_SIZE > 0x8000
    movhi r1, %hi(NIOS2_ICACHE_SIZE)
#else
    movui r1, NIOS2_ICACHE_SIZE
#endif

    movui r2, NIOS2_ICACHE_LINE_SIZE

0:
    initi r1
    sub r1, r1, r2
    bgt r1, zero, 0b
#endif /* NIOS2_ICACHE_SIZE > 0 */

    /*
     * Having initialised the cache, call the entry point for the .text
     * section.
     */

    movhi r1, %hiadj(_start)
    addi r1, r1, %lo(_start)

    jmp r1
#endif /* CYG_HAL_STARTUP_ROM */

    /*
     * ========================================================================
     * _start
     *
     * Start of the .text section, and also the code entry point when
     * the code is executed by a loader rather than directly from reset.
     */

    .section .text
    .align 2
    .globl _start
    .type _start, @function

_start:

    /*
     * After initializing the instruction cache, we must initialize the data
     * cache.
     */

#if NIOS2_DCACHE_SIZE > 0

    /*
     * The assumption here is that the data cache size is always
     * a power of two.
     */

#if NIOS2_DCACHE_SIZE > 0x8000
    movhi r1, %hi(NIOS2_DCACHE_SIZE)
#else
    movui r1, NIOS2_DCACHE_SIZE
#endif
    movui r2, NIOS2_DCACHE_LINE_SIZE

1:
    initd (r1)
    sub r1, r1, r2
    bgt r1, zero, 1b

#endif /* NIOS2_DCACHE_SIZE > 0 */

    /*
     * Now that the caches have been cleaned, set up the stack pointer.
     * The value is assumed to be correctly aligned.
     */

    movhi sp, %hiadj(STACK_BASE)
    addi sp, sp, %lo(STACK_BASE)

    /* set up the global pointer to the value provided by the linker script.*/

    movhi gp, %hiadj(_gp)
    addi gp, gp, %lo(_gp)

    /*
     * Clear bss.
     *
     * This uses the symbols: __bss_start and __bss_end, which are defined
     * by the linker script. They mark the begining and the end of the bss
     * region. The linker script guarantees that these values are word
     * aligned.
     */

    movhi r1, %hiadj(__bss_start)
    addi r1, r1, %lo(__bss_start)

    movhi r2, %hiadj(__bss_end)
    addi r2, r2, %lo(__bss_end)

    beq r1, r2, 3f

2:
    stw zero, (r1)
    addi r1, r1, 4
    bltu r1, r2, 2b

3:

#ifdef CYG_HAL_STARTUP_ROM

    /*
     * When running from flash, the initial values for the read/write
     * data must be copied to RAM.
     */

    .extern hal_copy_data
    call hal_copy_data
#endif /* CYG_HAL_STARTUP_ROM */

     /*
      * Call the platform initialisation. This initialises the virtual
      * vector and VSR tables.
      */

    .extern hal_platform_init
    call    hal_platform_init

    /* Call the C++ static constructors */

    .extern cyg_hal_invoke_constructors
    call    cyg_hal_invoke_constructors

     /* Initialise the GDB stub */

#if defined(CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS)
    .extern initialize_stub
    call    initialize_stub
#endif

     /*
      * Initialise ctrl-C support. This is used to implement the gdb
      * 'stop'.
      */

#if defined(CYGDBG_HAL_NIOS2_DEBUG_GDB_CTRLC_SUPPORT)
    .extern hal_ctrlc_isr_init
    call    hal_ctrlc_isr_init
#endif

    /* call the C entry point.  */

    call cyg_start

    /* spin, since there's no where left to go */

_cyg_start_end:
    br _cyg_start_end

    /*
     * ========================================================================
     * _exception_vector
     *
     * This is the exception entry point. It is responsible for determing if the
     * exception was caused by an hardware interrupt, or a software exception.
     * It then vectors accordingly using the VSR table to handle the exception.
     */

#ifndef CYG_HAL_STARTUP_RAM

    .section .irq, "xa"

_exception_vector:

    /*
     * First, re-wind so we're pointed to the instruction where the exception
     * occurred.
     */

    addi ea, ea, -4

    /*
     * Now test to determine the cause of the exception.
     */

    rdctl r24, estatus
    andi r24, r24, 1
    beq r24, zero, 4f
    rdctl r24, ipending
    beq r24, zero, 4f

    /*
     * Falling through to here means that this was a hardware interrupt.
     * In this case r24 is set to 4 to indicate that this exception
     * should be handled using the VSR table entry at offset 4.
     * In the case of a software exception, r24 will be left with the
     * value zero.
     */

    movi r24, 4

4:
    /*
     * At this point r24 contains the byte offset of the exception handler
     * in the VSR table.
     */

    .extern hal_vsr_table

    /*
     * Load the address of the exception handler, and jump to it.
     */

    orhi r24, r24, %hiadj(hal_vsr_table)
    addi r24, r24, %lo(hal_vsr_table)
    ldw r24, (r24)
    jmp r24

#endif /* CYG_HAL_STARTUP_RAM */

    /*
     * ========================================================================
     * _interrupt_handler
     *
     * This is the default handler for hardware interrupts.
     */

    .section .text

    .globl _interrupt_handler
    .type _interrupt_handler, @function
_interrupt_handler:

    /*
     * Process an external hardware interrupt.
     *
     * First, preserve all callee saved registers on
     * the stack. (See the Nios2 ABI documentation for details).
     */

    rdctl r24, estatus

    addi sp, sp, -76
    stw ra,  72(sp)
    stw r1,  68(sp)
    stw r2,  64(sp)
    stw r3,  60(sp)
    stw r4,  56(sp)
    stw r5,  52(sp)
    stw r6,  48(sp)
    stw r7,  44(sp)
    stw r8,  40(sp)
    stw r9,  36(sp)
    stw r10, 32(sp)
    stw r11, 28(sp)
    stw r12, 24(sp)
    stw r13, 20(sp)
    stw r14, 16(sp)
    stw r15, 12(sp)
    stw gp,  8(sp)
    stw r24, 4(sp)
    stw ea,  0(sp)

    /*
     * Move into r6 a pointer to the current saved registers.
     *
     * Do this here so it is accessible by both ISR and interrupt_end
     */

    mov r6, sp

    /*
     * Obtain a bitlist of the pending interrupts.
     */

    rdctl r24, ipending

    /*
     * Restore the global pointer to the expected value.
     */

    movhi gp, %hiadj(_gp)
    addi gp, gp, %lo(_gp)

#if defined(CYGDBG_HAL_NIOS2_DEBUG_GDB_CTRLC_SUPPORT)

    /*
     * If we are supporting Ctrl-C interrupts from GDB, we must squirrel
     * away a pointer to the save interrupt state here so that we can
     * plant a breakpoint at some later time.
     *
     * The state is only updated if it is a thread, rather than a DSR that
     * has been interrupted. That way it always points back to the last
     * thread to execute.
     *
     * This is a little bit shady, since the current stack frame doesn't
     * match the format expected by the debug stubs. However it is only
     * used to obtain the return address - which is stored in the expected
     * location.
     */

    .extern hal_saved_interrupt_state

    movhi r2, %hiadj(hal_saved_interrupt_state)
    addi r2, r2, %lo(hal_saved_interrupt_state)
    stw sp, (r2)
#endif

#if defined(CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK)

    /*
     * Switch to use the dedicated interrupt stack. The stack pointer
     * is only updated in the case that we're not already executing from
     * the interrupt stack.
     */

    mov r1, sp
    movhi r2, %hiadj(STACK_END)
    addi r2, r2, %lo(STACK_END)
    bgtu sp, r2, 5f
    movhi sp, %hiadj(STACK_BASE)
    addi sp, sp, %lo(STACK_BASE)
5:
    addi sp, sp, -4
    stw r1, (sp)
#endif /* defined(CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK) */

#ifdef CYGFUN_HAL_COMMON_KERNEL_SUPPORT
    /*
     * Lock the scheduler, if a scheduler is being used.
     */

    .extern _cyg_scheduler_sched_lock

    movhi r10, %hiadj(cyg_scheduler_sched_lock)
    addi r10, r10, %lo(cyg_scheduler_sched_lock)
    ldw r11,(r10)
    addi r11,r11,1
    stw r11,(r10)
#endif

    /*
     * Call the ISR to process the interrupts. At this point r24 contains
     * the contents of the ipending register.
     */

    mov r4, zero

    /*
     * Search through the bit list stored in r24 to find the first enabled
     * bit. The offset of this bit is the index of the interrupt that is
     * to be handled.
     */

6:
    andi r3, r24, 1
    bne r3, zero, 7f
    addi r4, r4, 1
    srli r24, r24, 1
    bne zero, r24, 6b				/* More bits to search*/
spurious_interrupt_site:
	/* ea or $r29 contains address site of interrupt */
    br spurious_interrupt_done

7:
    /*
     * Having located the interrupt source, r4 contains the index of the
     * interrupt to be handled.
     *
     * This is converted into an offset into the handler table,
     * and stored in r15.
     */

    slli r15, r4, 2

    /*
     * Load into r1 the address of the handler for this interrupt. This is
     * obtained from the interrupt handler table: hal_interrupt_handlers.
     */

    movhi r1, %hiadj(hal_interrupt_handlers)
    addi r1, r1, %lo(hal_interrupt_handlers)
    add r1, r1, r15
    ldw r1, (r1)

    /*
     * Load into r5 the data associated with the interrupt handler. This is
     * obtained from the table: hal_interrupt_data.
     */

    movhi r5, %hiadj(hal_interrupt_data)
    addi r5, r5, %lo(hal_interrupt_data)
    add r5, r5, r15
    ldw r5, (r5)

    /*
     * Call the interrupt handler  The input arguments are the interrupt number
     * and the associated data obtained above. Save off r15 explicitly since
     * its a caller-saved register and is used below.
     */
    addi sp, sp, -4
    stw r15, (sp)
    callr r1
    ldw r15, (sp)
    addi sp, sp, 4


	/* This is where we continue after a spurious interrupt, same as
	normal interrupt processing. */
spurious_interrupt_done:

#if defined(CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK)
    /*
     * Restore the stack pointer, in the case that we're using the common interrupt
     * stack.
     */

    ldw sp, (sp)
#endif /* defined(CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK) */

    /* Enable interrupts */

    movi r1, 1
    wrctl status, r1

    /* Begin DSR processing. */

    /*
     * Move the interrupt handlers return value into r4, so that it is the first
     * input argument to the DSR.
     */

    mov r4, r2

    /*
     * Load the interrupt object for DSR processing from the table:
     * hal_interrupt_objects. The offset into this table has previously been
     * calculated using the index of the interrupt, and is stored in r15.
     */

    movhi r5, %hiadj(hal_interrupt_objects)
    addi r5, r5, %lo(hal_interrupt_objects)
    add r5, r5, r15
    ldw r5, (r5)

    /* Call the DSR
     *
     * interrupt_end (isr_ret, intr, regs) ;
     */

    call interrupt_end

    /*
     * Now that the interrupt processing is complete, prepare to return to
     * the interrupted code.
     */

    /*
     * Restore the saved registers, so that all general purpose registers
     * have been restored to their state at the time the interrupt occured.
     */

    ldw ra,  72(sp)
    ldw r1,  68(sp)
    ldw r2,  64(sp)
    ldw r3,  60(sp)
    ldw r4,  56(sp)
    ldw r5,  52(sp)
    ldw r6,  48(sp)
    ldw r7,  44(sp)
    ldw r8,  40(sp)
    ldw r9,  36(sp)
    ldw r10, 32(sp)
    ldw r11, 28(sp)
    ldw r12, 24(sp)
    ldw r13, 20(sp)
    ldw r14, 16(sp)
    ldw r15, 12(sp)
    ldw gp,  8(sp)

    /*
     * Disable interrupts, so that we can safely update the
     * exception registers.
     */

    wrctl status, r0

    /* Restore the exception registers */

    ldw ea,  0(sp)
    ldw r24, 4(sp)
    wrctl estatus, r24

    /* Restore the stack pointer */

    addi sp, sp, 76

    /*
     * Return to the interrupted instruction.
     */
_return_interrupt_site:
    eret

    /*
     * ========================================================================
     * _software_exception_handler
     *
     * This is the default handler for software exceptions.
     */

    .globl _software_exception_handler
    .type _software_exception_handler, @function

_software_exception_handler:

    /* Store a reference to the current stack pointer in r24. */

    mov r24, sp

#if defined(CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK)

    /*
     * Switch to use the dedicated interrupt stack. The stack pointer
     * is only updated in the case that we're not already executing from
     * the interrupt stack.
     *
     * Some care needs to be taken here in that we need to preserve all
     * the general purpose registers at their current values, so that
     * they are reported correctly to GDB.
     */

    stw r2, -4(r24)
    movhi r2, %hiadj(STACK_END)
    addi r2, r2, %lo(STACK_END)
    bgtu sp, r2, 8f
    movhi sp, %hiadj(STACK_BASE)
    addi sp, sp, %lo(STACK_BASE)
8:
    ldw r2, -4(r24)

#endif /* defined(CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK) */

    /*
     * Store all the registers on the stack, so that their current
     * values are available to GDB.
     */

    addi sp, sp, -140

    stw r24, 128(sp)
    stw ra,  124(sp)

    /*
     * Store the first few of the caller saved registers, in order
     * to establish some scratch registers.
     */

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
    stw r12, 40(sp)

    /*
     * Write out the callee saved registers.
     */

    stw r23, 36(sp)
    stw r22, 32(sp)
    stw r21, 28(sp)
    stw r20, 24(sp)
    stw r19, 20(sp)
    stw r18, 16(sp)
    stw r17, 12(sp)
    stw r16, 8(sp)
    stw fp,  4(sp)
    stw ea,  0(sp)

    /*
     * Restore the global pointer.
     */

    movhi gp, %hiadj(_gp)
    addi gp, gp, %lo(_gp)

    /*
     * Pass a pointer to the stack frame as the input argument of the
     * exception handler
     */

    mov r4, sp

    /*
     * Call the exception handler.
     */

    .extern cyg_hal_exception_handler
    call    cyg_hal_exception_handler

    /*
     * Restore the saved registers, so that all general purpose registers
     * have been restored to their state at the time the interrupt occured.
     */

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

    ldw r24, 40(sp)
    wrctl estatus, r24

    ldw r23, 36(sp)
    ldw r22, 32(sp)
    ldw r21, 28(sp)
    ldw r20, 24(sp)
    ldw r19, 20(sp)
    ldw r18, 16(sp)
    ldw r17, 12(sp)
    ldw r16, 8(sp)
    ldw fp,  4(sp)
    ldw ea,  0(sp)

    /*
     * Restore the stack pointer
     */

    ldw sp, 128(sp)

    /*
     * Return from the exception.
     */

    eret

    /*
     * ========================================================================
     * hal_interrupt_stack_call_pending_DSRs
     *
     * Execute pending DSRs on the interrupt stack with interrupts enabled.
     * Note: this can only be called from code running on a thread stack.
     */

#if defined(CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK)
    .extern cyg_interrupt_call_pending_DSRs

    .section .text
    .align 2
    .globl hal_interrupt_stack_call_pending_DSRs
    .type hal_interrupt_stack_call_pending_DSRs, @function

hal_interrupt_stack_call_pending_DSRs:
    wrctl status, zero                    /* disable interrupts */
    mov r1, sp                            /* store the current stack */
    movhi sp, %hiadj(STACK_BASE - 8)      /* switch to the new stack */
    addi sp, sp, %lo(STACK_BASE - 8)
    stw r1, (sp)                          /* store the old stack location */
    stw ra, 4(sp)                         /* store the old return address */
    movi r1, 1                            /* enable interrupts */
    wrctl status, r1
    call cyg_interrupt_call_pending_DSRs  /* process the DSR's */
    ldw ra, 4(sp)                         /* restore the return address */
    ldw sp, (sp)                          /* restore the stack pointer */
    ret                                   /* return */

#endif

    /*
     * ========================================================================
     * __gccmain
     *
     * Short circuit in case any code tries to use "__gccmain()"
     */

    .section .text
    .align 2
    .globl __gccmain
    .type __gccmain, @function

__gccmain:
    jmp ra

    /*
     * GCC 3.4.1 defines a C++ exception handler that is incompatible with eCos.
     * To avoid pulling this in, an alternative is provided here.
     */

	.section .sdata
	.align 4

#if __GNUC__ == 3
#if __GNUC_MINOR__ == 4
#if __GNUC_PATCHLEVEL__ == 1
_ZN10__cxxabiv119__terminate_handlerE:
   .word abort
#endif /* __GNUC_PATCHLEVEL__ */
#endif /* __GNUC_MINOR__ */
#endif /* __GNUC__ */
