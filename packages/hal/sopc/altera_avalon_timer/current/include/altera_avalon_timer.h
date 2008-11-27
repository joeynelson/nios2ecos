#ifndef CYGONCE_ALTERA_AVALON_TIMER_H
#define CYGONCE_ALTERA_AVALON_TIMER_H

//=============================================================================
//
//      altera_avalon_timer.h
//
//      Device driver for the Altera Avalon timer.
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
//===========================================================================

/*
 * This file supplies the system clock constants used to calibrate the system
 * timer facilities. These parameters are more ususally suplied with the 
 * processor HAL package. In the case of SOPC builder based systems, they are
 * instead supplied with a seperate timer driver. This allows the timer device
 * type to be changed, without requiring modification of the processor HAL.
 */

/*
 * Construct some intermediate defines that are used to extract the timer 
 * configuration from the auto-generated system.h header file.
 */

#define ALT_AVALON_TIMER_PERIOD_UNITS_S      0
#define ALT_AVALON_TIMER_PERIOD_UNITS_MS     1
#define ALT_AVALON_TIMER_PERIOD_UNITS_US     2
#define ALT_AVALON_TIMER_PERIOD_UNITS_CLOCKS 3

#define __ALT_CLK_BASE(name) name##_BASE
#define _ALT_CLK_BASE(name) __ALT_CLK_BASE(name)
#define ALT_SYS_CLK_BASE _ALT_CLK_BASE(CYGHWR_HAL_SYSCLK_DEV)

#define __ALT_CLK_IRQ(name) name##_IRQ
#define _ALT_CLK_IRQ(name) __ALT_CLK_IRQ(name)
#define ALT_SYS_CLK_IRQ _ALT_CLK_IRQ(CYGHWR_HAL_SYSCLK_DEV)

#define __ALT_CLK_PERIOD(name) name##_PERIOD
#define _ALT_CLK_PERIOD(name) __ALT_CLK_PERIOD(name)
#define ALT_SYS_CLK_PERIOD _ALT_CLK_PERIOD(CYGHWR_HAL_SYSCLK_DEV)

#define __ALT_CLK_PERIOD_UNITS(name) name##_ALT_AVALON_TIMER_PERIOD_UNITS
#define _ALT_CLK_PERIOD_UNITS(name) __ALT_CLK_PERIOD_UNITS(name)
#define ALT_SYS_CLK_PERIOD_UNITS _ALT_CLK_PERIOD_UNITS(CYGHWR_HAL_SYSCLK_DEV)

#define __ALT_CLK_FREQ(name) name##_FREQ
#define _ALT_CLK_FREQ(name) __ALT_CLK_FREQ(name)
#define ALT_SYS_CLK_FREQ _ALT_CLK_FREQ(CYGHWR_HAL_SYSCLK_DEV)

/*
 * Having extracted the timer constants, use these to contruct the real time clock constants
 * required for eCos.
 */

#define CYGNUM_HAL_RTC_CONSTANTS (1)
#define CYGNUM_HAL_RTC_NUMERATOR (1000000000)
#define CYGNUM_HAL_RTC_DENOMINATOR (((ALT_SYS_CLK_PERIOD_UNITS != ALT_AVALON_TIMER_PERIOD_UNITS_S) ? \
                                      ((ALT_SYS_CLK_PERIOD_UNITS != ALT_AVALON_TIMER_PERIOD_UNITS_MS) ? \
                                        ((ALT_SYS_CLK_PERIOD_UNITS != ALT_AVALON_TIMER_PERIOD_UNITS_US ) ? \
                                          ALT_SYS_CLK_FREQ \
                                        : 1000000) \
                                      : 1000) \
                                    : 1)/ALT_SYS_CLK_PERIOD)
#define CYGNUM_HAL_RTC_PERIOD (ALT_SYS_CLK_FREQ / CYGNUM_HAL_RTC_DENOMINATOR)
#define CYGNUM_HAL_INTERRUPT_RTC ALT_SYS_CLK_IRQ

//---------------------------------------------------------------------------
// end of hal_diag.h                                                         
#endif /* CYGONCE_ALTERA_AVALON_TIMER_H */
