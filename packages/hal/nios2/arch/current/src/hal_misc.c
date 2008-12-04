//==========================================================================
//
//      hal_misc.c
//
//      HAL miscellaneous functions
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

#include <cyg/infra/cyg_type.h>         // Base types
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros

#define CYGARC_HAL_COMMON_EXPORT_CPU_MACROS
#include <cyg/hal/hal_arch.h>           // architectural definitions

#include <cyg/hal/hal_intr.h>           // Interrupt handling

#include <cyg/hal/hal_cache.h>          // Cache handling
#include <cyg/hal/hal_if.h>             // hal_ctrlc_isr()
#include <cyg/hal/system.h>

#include <limits.h>
#include <string.h>

#include CYGHWR_MEMORY_LAYOUT_H

#include <pkgconf/hal.h>

extern void _interrupt_handler;
extern void _software_exception_handler;

//------------------------------------------------------------------------

void hal_platform_init(void)
{
  // Set up eCos/ROM interfaces

  hal_if_init();

  // Set the ISR vector

#ifndef CYGSEM_HAL_USE_ROM_MONITOR
  hal_vsr_table[0] = (CYG_ADDRWORD) &_software_exception_handler;
#endif // CYGSEM_HAL_USE_ROM_MONITOR
  hal_vsr_table[1] = (CYG_ADDRWORD) &_interrupt_handler;

}

//------------------------------------------------------------------------

externC HAL_SavedRegisters *_hal_registers;

cyg_uint32 cyg_hal_exception_handler(HAL_SavedRegisters *regs)
{
#if defined(CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS)

  // Set the pointer to the registers of the current exception
  // context. At entry the GDB stub will expand the
  // HAL_SavedRegisters structure into a (bigger) register array.

  _hal_registers = regs;
  __handle_exception();

#elif defined(CYGFUN_HAL_COMMON_KERNEL_SUPPORT) && defined(CYGPKG_HAL_EXCEPTIONS)

  // We should decode the vector and pass a more appropriate
  // value as the second argument. For now we simply pass a
  // pointer to the saved registers. We should also divert
  // breakpoint and other debug vectors into the debug stubs.

  cyg_hal_deliver_exception( 0, (CYG_ADDRWORD)regs );

#else

  CYG_FAIL("Exception!!!");

#endif
  return 0;
}

//------------------------------------------------------------------------
// default ISR

#ifndef CYGSEM_HAL_VIRTUAL_VECTOR_SUPPORT
cyg_uint32 hal_default_isr(CYG_ADDRWORD vector, CYG_ADDRWORD data)
{

  CYG_TRACE1(true, "Interrupt: %d", vector);
  CYG_FAIL("Spurious Interrupt!!!");
  return 0;
}

#else // CYGSEM_HAL_VIRTUAL_VECTOR_SUPPORT

cyg_uint32 hal_arch_default_isr(CYG_ADDRWORD vector, CYG_ADDRWORD data)
{
  return 0;
}

#endif // CYGSEM_HAL_VIRTUAL_VECTOR_SUPPORT

//------------------------------------------------------------------------
// data copy and bss zero functions

typedef void (CYG_SYM_ADDRESS)(void);

// All these must use this type of address to stop them being given relocations
// relative to $gp (i.e. assuming they would be in .sdata)

extern CYG_SYM_ADDRESS __ram_data_start;
extern CYG_SYM_ADDRESS __ram_data_end;
extern CYG_SYM_ADDRESS __rom_data_start;
extern CYG_SYM_ADDRESS __ram_exc_start;
extern CYG_SYM_ADDRESS __ram_exc_end;
extern CYG_SYM_ADDRESS __rom_exc_start;

#ifdef CYG_HAL_STARTUP_ROM

void hal_copy_data(void)
{
  char *p = (char *)&__ram_data_start;
  char *q = (char *)&__rom_data_start;

  // Copy the data section

  while( p != (char *)&__ram_data_end )
  {
    *p++ = *q++;
  }

  // Copy the exception handler if necessary

  p = (char *)&__ram_exc_start;
  q = (char *)&__rom_exc_start;

  if (p != q)
  {
    while( p != (char *)&__ram_exc_end )
    {
      *p++ = *q++;
    }
  }
}
#endif

//------------------------------------------------------------------------
// Determine the index of the ls bit of the supplied mask.

cyg_uint32 hal_lsbit_index(cyg_uint32 mask)
{
  cyg_uint32 n = mask;

  static const signed char tab[64] =
  {
    -1, 0, 1, 12, 2, 6, 0, 13, 3, 0, 7, 0, 0, 0, 0, 14, 10,
     4, 0, 0, 8, 0, 0, 25, 0, 0, 0, 0, 0, 21, 27 , 15, 31, 11,
     5, 0, 0, 0, 0, 0, 9, 0, 0, 24, 0, 0 , 20, 26, 30, 0, 0, 0,
     0, 23, 0, 19, 29, 0, 22, 18, 28, 17, 16, 0
  };

  n &= ~(n-1UL);
  n = (n<<16)-n;
  n = (n<<6)+n;
  n = (n<<4)+n;

  return tab[n>>26];
}

//------------------------------------------------------------------------
// Determine the index of the ms bit of the supplied mask.

cyg_uint32 hal_msbit_index(cyg_uint32 mask)
{
  cyg_uint32 x = mask;
  cyg_uint32 w;

  // Phase 1: make word with all ones from that one to the right

  x |= x >> 16;
  x |= x >> 8;
  x |= x >> 4;
  x |= x >> 2;
  x |= x >> 1;

  // Phase 2: calculate number of "1" bits in the word

  w = (x & 0x55555555) + ((x >> 1) & 0x55555555);
  w = (w & 0x33333333) + ((w >> 2) & 0x33333333);
  w = w + (w >> 4);
  w = (w & 0x000F000F) + ((w >> 8) & 0x000F000F);
  return (cyg_uint32)((w + (w >> 16)) & 0xFF) - 1;
}

//------------------------------------------------------------------------
// Delay for some number of usecond

void hal_delay_us(int us)
{
  int i;
  int big_loops;
  unsigned int cycles_per_loop;

  if (!__builtin_strcmp(NIOS2_CPU_IMPLEMENTATION,"tiny"))
  {
    cycles_per_loop = 9;
  }
  else
  {
    cycles_per_loop = 3;
  }

  big_loops = us / (INT_MAX/
  (ALT_CPU_FREQ/(cycles_per_loop * 1000000)));

  if (big_loops)
  {
    for(i=0;i<big_loops;i++)
    {
      //
      // Do NOT Try to single step the asm statement below
      // (single step will never return)
      // Step out of this function or set a breakpoint after the asm statements
      //
      __asm__ volatile ("usleep_delay_loop1: addi %0,%0, -1\n\tbne %0,zero,usleep_delay_loop1" :: "r" (INT_MAX));
      us -= (INT_MAX/(ALT_CPU_FREQ/
      (cycles_per_loop * 1000000)));
    }

    //
    // Do NOT Try to single step the asm statement below
    // (single step will never return)
    // Step out of this function or set a breakpoint after the asm statements
    //
    __asm__ volatile ("usleep_delay_loop2: addi %0,%0, -1\n\tbne %0,zero,usleep_delay_loop2" :: "r" (us*(ALT_CPU_FREQ/(cycles_per_loop * 1000000))));
  }
  else
  {
    //
    // Do NOT Try to single step the asm statement below
    // (single step will never return)
    // Step out of this function or set a breakpoint after the asm statements
    //
    __asm__ volatile ("usleep_delay_loop3: addi %0,%0, -1\n\tbgt %0,zero,usleep_delay_loop3" :: "r" (us*(ALT_CPU_FREQ/(cycles_per_loop * 1000000))));
  }
}

//------------------------------------------------------------------------
// Call the C++ static constructors.

#ifdef CYGSEM_HAL_STOP_CONSTRUCTORS_ON_FLAG
cyg_bool cyg_hal_stop_constructors;
#endif

typedef void (*pfunc) (void);
extern pfunc __CTOR_LIST__[];
extern pfunc __CTOR_END__[];

void
cyg_hal_invoke_constructors(void)
{
#ifdef CYGSEM_HAL_STOP_CONSTRUCTORS_ON_FLAG
  static pfunc *p = &__CTOR_END__[-1];

  cyg_hal_stop_constructors = 0;
  for (; p >= __CTOR_LIST__; p--) {
      (*p) ();
      if (cyg_hal_stop_constructors) {
          p--;
          break;
      }
  }
#else
  pfunc *p;

  for (p = &__CTOR_END__[-1]; p >= __CTOR_LIST__; p--)
      (*p) ();
#endif

} // cyg_hal_invoke_constructors()

//------------------------------------------------------------------------
// Idle thread action

#include <cyg/infra/diag.h>

void hal_idle_thread_action( cyg_uint32 count )
{
}

//------------------------------------------------------------------------
// Platform comms initialisation

void cyg_hal_plf_comms_init(void)
{
}


#ifdef CYGPKG_PROFILE_GPROF
//--------------------------------------------------------------------------
//
// Profiling support - uses a separate high-speed timer
//

#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/hal/system.h>
#include <cyg/profile/profile.h>
#include <altera_avalon_timer_regs.h>




/*---------------------------------------------------------------------
 * hal_clock_initialize
 *
 * Initialise the timer device. The period used is the period defined in the
 * SOPC builder project.
 */

void hal_clock_initialize1 (cyg_uint32 _period_)
{
  IOWR_ALTERA_AVALON_TIMER_CONTROL(HIGH_RES_TIMER_BASE,
            ALTERA_AVALON_TIMER_CONTROL_ITO_MSK  |
            ALTERA_AVALON_TIMER_CONTROL_CONT_MSK |
            ALTERA_AVALON_TIMER_CONTROL_START_MSK);

}

/*---------------------------------------------------------------------
 * hal_clock_reset
 *
 * hal_clock_reset() is called by the interrupt service routine to clear an
 * interrupt for this device.
 */

void hal_clock_reset1 (cyg_uint32 _vector_, cyg_uint32 _period_)
{
  IOWR_ALTERA_AVALON_TIMER_STATUS (HIGH_RES_TIMER_BASE, 0);
}


// Can't rely on Cyg_Interrupt class being defined.
#define Cyg_InterruptHANDLED 1

static int  profile_period  = 0;

// Profiling timer ISR
static cyg_uint32
profile_isr(CYG_ADDRWORD vector, CYG_ADDRWORD data, HAL_SavedRegisters *regs)
{
	__profile_hit(regs->pc);

	/* we don't want to starve normal execution completely, so
	 * restart timer after we've done the work.
	 */
	hal_clock_reset1(HIGH_RES_TIMER_IRQ, profile_period);
    HAL_INTERRUPT_ACKNOWLEDGE(HIGH_RES_TIMER_IRQ);

	return Cyg_InterruptHANDLED;
}

int
hal_enable_profile_timer(int resolution)
{
    // Run periodic timer interrupt for profile
    // The resolution is specified in us


	// Set period and enable timer interrupts
    hal_clock_initialize1(0);

    // Attach ISR.
    HAL_INTERRUPT_ATTACH(HIGH_RES_TIMER_IRQ, &profile_isr, 0x1111, 0);
    HAL_INTERRUPT_UNMASK(HIGH_RES_TIMER_IRQ);


    return resolution;
}
#endif

//------------------------------------------------------------------------
// End of hal_misc.
