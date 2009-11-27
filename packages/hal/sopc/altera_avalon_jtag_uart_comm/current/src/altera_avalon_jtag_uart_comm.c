//==========================================================================
//
//      altera_avalon_uart_comm.c
//
//      Device driver for the Altera Avalon UART.
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

/*
 * This driver is used for the JTAG UART when it is being used for either the 
 * diagnostic console or debug connection.
 */

#include <cyg/infra/diag.h>
#include <cyg/infra/cyg_type.h>
#include <cyg/hal/hal_if.h>
#include <cyg/hal/hal_misc.h>
#include <cyg/hal/drv_api.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_io.h>

#include <pkgconf/hal.h>
#include <pkgconf/system.h>
#include CYGBLD_HAL_PLATFORM_H

#include <cyg/hal/hal_intr.h>

#include <altera_avalon_jtag_uart_regs.h>

/*--------------------------------------------------------------------- 
 * altera_avalon_jtag_uart_diag_putc
 *
 * Send a character to the JTAG UART. This function will block until the 
 * character can be sent.
 */

void altera_avalon_jtag_uart_diag_putc(void* __ch_data, cyg_uint8 __ch)
{
    cyg_uint32 status;
    cyg_uint8* port = (cyg_uint8*) __ch_data;
    volatile cyg_uint32 control;

    CYGARC_HAL_SAVE_GP();

    do
    {
      control = IORD_ALTERA_AVALON_JTAG_UART_CONTROL(port);
    }
    while(!(control & ALTERA_AVALON_JTAG_UART_CONTROL_WSPACE_MSK));

    IOWR_ALTERA_AVALON_JTAG_UART_DATA(port, __ch);

    CYGARC_HAL_RESTORE_GP();

}

/*--------------------------------------------------------------------- 
 * altera_avalon_jtag_uart_diag_getc_nonblock
 *
 * Fetch a character from the JTAG UART. If there is no character available
 * this function will return immediately with the value: false. Otherwise it
 * will return true and 'ch' will be set to point to the fetched character.
 */

cyg_bool altera_avalon_jtag_uart_diag_getc_nonblock(void* __ch_data, cyg_uint8* ch)
{
    cyg_uint8* port = (cyg_uint8*) __ch_data;
    cyg_uint32 status;
    cyg_uint32 data; 

    data = IORD_ALTERA_AVALON_JTAG_UART_DATA(port);

    if (data & ALTERA_AVALON_JTAG_UART_DATA_RVALID_MSK)
    {
      *ch = (data & ALTERA_AVALON_JTAG_UART_DATA_DATA_MSK) 
                >> ALTERA_AVALON_JTAG_UART_DATA_DATA_OFST;
      return true;
    }
    else
    {
      return false;
    }
}

/*--------------------------------------------------------------------- 
 * altera_avalon_jtag_uart_diag_getc
 *
 * Fetch a character from the UART. This function will block until a
 * character is available.
 */

cyg_uint8 altera_avalon_jtag_uart_diag_getc(void* __ch_data)
{
    cyg_uint8 ch;

    CYGARC_HAL_SAVE_GP();

    while(!altera_avalon_jtag_uart_diag_getc_nonblock(__ch_data, &ch));

    CYGARC_HAL_RESTORE_GP();

    return ch;
}

/*--------------------------------------------------------------------- 
 * altera_avalon_jtag_uart_diag_write
 *
 * Write a block of data to the JTAG UART.
 */

void altera_avalon_jtag_uart_diag_write(void* __ch_data, const cyg_uint8* __buf, 
                                        cyg_uint32 __len)
{
    CYGARC_HAL_SAVE_GP();
  
    while(__len-- > 0)
    {
      altera_avalon_jtag_uart_diag_putc(__ch_data, *__buf++);
    }
    CYGARC_HAL_RESTORE_GP();

}

/*--------------------------------------------------------------------- 
 * altera_avalon_jtag_uart_diag_read
 *
 * Read a block of data from the JTAG UART. This function will block until 
 * the requested amount of data has been read.
 */

void altera_avalon_jtag_uart_diag_read(void* __ch_data, cyg_uint8* __buf, cyg_uint32 __len)
{
    CYGARC_HAL_SAVE_GP();

    while(__len-- > 0)
    {
      *__buf++ = altera_avalon_jtag_uart_diag_getc(__ch_data);
    }

    CYGARC_HAL_RESTORE_GP();
}

/*--------------------------------------------------------------------- 
 * altera_avalon_jtag_uart_diag_getc_timeout
 *
 * Fetch a character from the JTAG UART, but timeout after 10ms if no character
 * becomes available.
 */

cyg_bool altera_avalon_jtag_uart_diag_getc_timeout(void* __ch_data, cyg_uint8* ch)
{
    int delay_count;
    cyg_bool res;

    CYGARC_HAL_SAVE_GP();

    delay_count = 10; // delay in .1 ms steps

    for(;;) {
      res = altera_avalon_jtag_uart_diag_getc_nonblock(__ch_data, ch);
      if (res || 0 == delay_count--)
        break;
      HAL_DELAY_US(10);
    }

    CYGARC_HAL_RESTORE_GP();

    return res;
}

/*--------------------------------------------------------------------- 
 * altera_avalon_jtag_uart_diag_control
 *
 * Preform I/O control for the UART. In this case there are no supported
 * operations.
 */

int altera_avalon_jtag_uart_diag_control(void *__ch_data, __comm_control_cmd_t __func, ...)
{
    return true;
}

/*--------------------------------------------------------------------- 
 * altera_avalon_jtag_uart_diag_isr
 */

int altera_avalon_jtag_uart_diag_isr(void *__ch_data, int* __ctrlc, 
                                     CYG_ADDRWORD __vector, CYG_ADDRWORD __data)
{
  return 0;
}

/*---------------------------------------------------------------------*/

/*
 * Pointers to the console channel devices. altera_avalon_jtag_uart_chan[0] is the
 * channel used for debug communication, and altera_avalon_jtag_uart_chan[1] is the
 * channel used for console communication.
 *
 * The value NULL indicates that the channel is not connected.
 */ 

static cyg_uint8* altera_avalon_jtag_uart_chan[2] = {NULL, NULL};

/*
 * Macro used to convert the supplied device name into a string.
 */

#define _DEV_NAME(dev) #dev
#define DEV_NAME(dev) _DEV_NAME(dev)

/*
 * Macro used to install the console driver if apropriate. The user defined macros:
 * CYGHWR_HAL_NIOS2_VV_DEBUG_DEV and CYGHWR_HAL_NIOS2_VV_CONSOLE_DEV name the 
 * devices to use for the debug and console channels respectively. If these names
 * are found to match a device of type altera_avalon_jtag_uart, then the channel is
 * connected to that device.
 *
 */

#define ALTERA_AVALON_JTAG_UART_INSTANCE(name, device)                                  \
  if (!__builtin_strcmp (DEV_NAME(CYGHWR_HAL_NIOS2_VV_DEBUG_DEV), name##_NAME))         \
  {                                                                                     \
    altera_avalon_jtag_uart_chan[0] = (cyg_uint8*) name##_BASE;                         \
                                                                                        \
    CYGACC_CALL_IF_SET_CONSOLE_COMM(0);                                                 \
    comm = CYGACC_CALL_IF_CONSOLE_PROCS();                                              \
    CYGACC_COMM_IF_CH_DATA_SET(*comm, altera_avalon_jtag_uart_chan[0]);                 \
    CYGACC_COMM_IF_WRITE_SET(*comm, altera_avalon_jtag_uart_diag_write);                \
    CYGACC_COMM_IF_READ_SET(*comm, altera_avalon_jtag_uart_diag_read);                  \
    CYGACC_COMM_IF_PUTC_SET(*comm, altera_avalon_jtag_uart_diag_putc);                  \
    CYGACC_COMM_IF_GETC_SET(*comm, altera_avalon_jtag_uart_diag_getc);                  \
    CYGACC_COMM_IF_CONTROL_SET(*comm, altera_avalon_jtag_uart_diag_control);            \
    CYGACC_COMM_IF_DBG_ISR_SET(*comm, altera_avalon_jtag_uart_diag_isr);                \
    CYGACC_COMM_IF_GETC_TIMEOUT_SET(*comm, altera_avalon_jtag_uart_diag_getc_timeout);  \
  }                                                                                     \
                                                                                        \
  if (!__builtin_strcmp (DEV_NAME(CYGHWR_HAL_NIOS2_VV_CONSOLE_DEV), name##_NAME))       \
  {                                                                                     \
    altera_avalon_jtag_uart_chan[1] = (cyg_uint8*) name##_BASE;                         \
                                                                                        \
    CYGACC_CALL_IF_SET_CONSOLE_COMM(1);                                                 \
    comm = CYGACC_CALL_IF_CONSOLE_PROCS();                                              \
    CYGACC_COMM_IF_CH_DATA_SET(*comm, altera_avalon_jtag_uart_chan[1]);                 \
    CYGACC_COMM_IF_WRITE_SET(*comm, altera_avalon_jtag_uart_diag_write);                \
    CYGACC_COMM_IF_READ_SET(*comm, altera_avalon_jtag_uart_diag_read);                  \
    CYGACC_COMM_IF_PUTC_SET(*comm, altera_avalon_jtag_uart_diag_putc);                  \
    CYGACC_COMM_IF_GETC_SET(*comm, altera_avalon_jtag_uart_diag_getc);                  \
    CYGACC_COMM_IF_CONTROL_SET(*comm, altera_avalon_jtag_uart_diag_control);            \
    CYGACC_COMM_IF_DBG_ISR_SET(*comm, altera_avalon_jtag_uart_diag_isr);                \
    CYGACC_COMM_IF_GETC_TIMEOUT_SET(*comm, altera_avalon_jtag_uart_diag_getc_timeout);  \
  }


/*--------------------------------------------------------------------- 
 * altera_avalon_jtag_uart_diag_init
 */

void altera_avalon_jtag_uart_diag_init (void) 
  {
    hal_virtual_comm_table_t* comm;
    int cur;

    cur = CYGACC_CALL_IF_SET_CONSOLE_COMM(CYGNUM_CALL_IF_SET_COMM_ID_QUERY_CURRENT);

#include <cyg/hal/devices.h>
 
    /* Restore original console */

    CYGACC_CALL_IF_SET_CONSOLE_COMM(cur);  
};
