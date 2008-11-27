//==========================================================================
//
//      altera_avalon_timer.c
//
//      Device driver for the Altera Avalon timer.
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

#include <pkgconf/system.h>
#include <cyg/hal/hal_intr.h>
#include <altera_avalon_timer_regs.h>

/*
 * This file supplies the system clock functions. These are defined here rather 
 * than with the architecture package (as is more normal), so that alternative 
 * timer devices can be used without requiring changes to the processor port. 
 */ 

#ifdef CYGPKG_ALTERA_AVALON_TIMER

/*--------------------------------------------------------------------- 
 * hal_clock_initialize
 *
 * Initialise the timer device. The period used is the period defined in the
 * SOPC builder project.
 */

void hal_clock_initialize (cyg_uint32 _period_)
{
  IOWR_ALTERA_AVALON_TIMER_CONTROL(ALT_SYS_CLK_BASE,
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

void hal_clock_reset (cyg_uint32 _vector_, cyg_uint32 _period_)
{
  IOWR_ALTERA_AVALON_TIMER_STATUS (ALT_SYS_CLK_BASE, 0);
}

/*--------------------------------------------------------------------- 
 * hal_clock_read
 *
 * hal_clock_read() can be called to sample the current value of the
 * system clock timer. Note that this requires the device to be configured with
 * a readable snapshot register.
 */

cyg_uint32 hal_clock_read (void)
{
  cyg_uint32 lower;
  cyg_uint32 upper;

  IOWR_ALTERA_AVALON_TIMER_SNAPL (ALT_SYS_CLK_BASE, 0);

  lower = IORD_ALTERA_AVALON_TIMER_SNAPL(ALT_SYS_CLK_BASE);
  upper = IORD_ALTERA_AVALON_TIMER_SNAPH(ALT_SYS_CLK_BASE);

  return (CYGNUM_HAL_RTC_PERIOD - ((upper << 16) | lower));
}

#endif /* CYGPKG_ALTERA_AVALON_TIMER */
