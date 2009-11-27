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
 * This driver is used for the UART when it is being used for either the 
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

#include <altera_avalon_uart_regs.h>

typedef struct 
{
  cyg_uint8* port;
  cyg_uint32 irq;
} altera_avalon_uart_chan;

/*--------------------------------------------------------------------- 
 * altera_avalon_uart_diag_putc
 *
 * Send a character to the UART. This function will block until the 
 * character can be sent.
 */

void altera_avalon_uart_diag_putc(void* __ch_data, cyg_uint8 __ch)
{
    cyg_uint8* port = ((altera_avalon_uart_chan*) __ch_data)->port;
    volatile cyg_uint32 status;

    CYGARC_HAL_SAVE_GP();

    do
    {
      status = IORD_ALTERA_AVALON_UART_STATUS(port);
    }
    while (!(status & ALTERA_AVALON_UART_STATUS_TRDY_MSK));
      
    IOWR_ALTERA_AVALON_UART_TXDATA(port, __ch);

    CYGARC_HAL_RESTORE_GP();
}

/*--------------------------------------------------------------------- 
 * altera_avalon_uart_diag_getc_nonblock
 *
 * Fetch a character from the UART. If there is no character available
 * this function will return immediately with the value: false. Otherwise it
 * will return true and 'ch' will be set to point to the fetched character.
 */

cyg_bool altera_avalon_uart_diag_getc_nonblock(void* __ch_data, cyg_uint8* ch)
{
    cyg_uint8* port = ((altera_avalon_uart_chan*) __ch_data)->port;
    cyg_uint32 status;

    status = IORD_ALTERA_AVALON_UART_STATUS(port);

    /* clear any error flags */

    IOWR_ALTERA_AVALON_UART_STATUS(port, 0);

    if (status & ALTERA_AVALON_UART_CONTROL_RRDY_MSK)
    {
      *ch = IORD_ALTERA_AVALON_UART_RXDATA(port);

      if (!(status & (ALTERA_AVALON_UART_STATUS_PE_MSK | 
                      ALTERA_AVALON_UART_STATUS_FE_MSK)))
      {
        return true;
      }
    }
    return false;
}

/*--------------------------------------------------------------------- 
 * altera_avalon_uart_diag_getc
 *
 * Fetch a character from the UART. This function will block until a
 * character is available.
 */

cyg_uint8 altera_avalon_uart_diag_getc(void* __ch_data)
{
    cyg_uint8 ch;

    CYGARC_HAL_SAVE_GP();
    while(!altera_avalon_uart_diag_getc_nonblock(__ch_data, &ch));
    CYGARC_HAL_RESTORE_GP();

    return ch;
}

/*--------------------------------------------------------------------- 
 * altera_avalon_uart_diag_write
 *
 * Write a block of data to the UART.
 */

void altera_avalon_uart_diag_write(void* __ch_data, const cyg_uint8* __buf, 
                                   cyg_uint32 __len)
{
    CYGARC_HAL_SAVE_GP();

    while(__len-- > 0)
    {
      altera_avalon_uart_diag_putc(__ch_data, *__buf++);
    }

    CYGARC_HAL_RESTORE_GP();
}

/*--------------------------------------------------------------------- 
 * altera_avalon_uart_diag_read
 *
 * Read a block of data from the UART. This function will block until 
 * the requested amount of data has been read.
 */

void altera_avalon_uart_diag_read(void* __ch_data, cyg_uint8* __buf, cyg_uint32 __len)
{
    CYGARC_HAL_SAVE_GP();

    while(__len-- > 0)
    {
      *__buf++ = altera_avalon_uart_diag_getc(__ch_data);
    }

    CYGARC_HAL_RESTORE_GP();
}

/*--------------------------------------------------------------------- 
 * altera_avalon_uart_diag_getc_timeout
 *
 * Fetch a character from the UART, but timeout after 10ms if no character
 * becomes available.
 */

cyg_bool altera_avalon_uart_diag_getc_timeout(void* __ch_data, cyg_uint8* ch)
{
    int delay_count;
    cyg_bool res;

    CYGARC_HAL_SAVE_GP();

    delay_count = 10; // delay in .1 ms steps

    for(;;) {
        res = altera_avalon_uart_diag_getc_nonblock(__ch_data, ch);
        if (res || 0 == delay_count--)
            break;
       HAL_DELAY_US(10);
    }

    CYGARC_HAL_RESTORE_GP();

    return res;
}

/*--------------------------------------------------------------------- 
 * altera_avalon_uart_diag_control
 *
 * Preform I/O control for the UART. In this case the only supported operations
 * relate to enabling and disabling interrupts. These are required by the 
 * debugger for ctrl-C processing.
 */

int altera_avalon_uart_diag_control(void *__ch_data, __comm_control_cmd_t __func, ...)
{
    static int irq_state = 0;
    altera_avalon_uart_chan* chan;
    int ret = 0;
    cyg_uint32 ctrl;

    CYGARC_HAL_SAVE_GP();

    chan = (altera_avalon_uart_chan*)__ch_data;

    switch (__func) {
    case __COMMCTL_IRQ_ENABLE:
        irq_state = 1;

        ctrl = IORD_ALTERA_AVALON_UART_CONTROL(chan->port);
        IOWR_ALTERA_AVALON_UART_CONTROL(chan->port, ctrl | 
                                                    ALTERA_AVALON_UART_CONTROL_RRDY_MSK);
        HAL_INTERRUPT_SET_LEVEL(chan->irq, 1);
        HAL_INTERRUPT_UNMASK(chan->irq);
        break;
    case __COMMCTL_IRQ_DISABLE:
        ret = irq_state;
        irq_state = 0;

        ctrl = IORD_ALTERA_AVALON_UART_CONTROL(chan->port);
        IOWR_ALTERA_AVALON_UART_CONTROL(chan->port, ctrl & 
                                                    ~(ALTERA_AVALON_UART_CONTROL_RRDY_MSK));
        HAL_INTERRUPT_MASK(chan->irq);
        break;
    case __COMMCTL_DBG_ISR_VECTOR:
        ret = chan->irq;
        break;

    default:
        break;
    }
    CYGARC_HAL_RESTORE_GP();
    return ret;
}

/*--------------------------------------------------------------------- 
 * altera_avalon_uart_diag_isr
 *
 * The interrupt service routine for the UART. This is used by the 
 * debugger to detect ctrl-C (stop) commands.
 */

int altera_avalon_uart_diag_isr(void *__ch_data, int* __ctrlc, 
                                CYG_ADDRWORD __vector, CYG_ADDRWORD __data)
{
  altera_avalon_uart_chan* chan = (altera_avalon_uart_chan*) __ch_data;
  cyg_uint8* port = chan->port;
  cyg_uint32 status;
  cyg_uint32 data;
  
  int res = 0;
  cyg_uint8 c;
  CYGARC_HAL_SAVE_GP();
  
  HAL_INTERRUPT_ACKNOWLEDGE(chan->irq);

  *__ctrlc = 0;

  /*
   * Read the status register in order to determine the cause of the
   * interrupt.
   */

  status = IORD_ALTERA_AVALON_UART_STATUS(port);

  /* Clear any error flags set at the device */

  IOWR_ALTERA_AVALON_UART_STATUS(port, 0);

  /* process a read irq */
 
  if (status & ALTERA_AVALON_UART_STATUS_RRDY_MSK)
  {
    data = IORD_ALTERA_AVALON_UART_RXDATA(port);

    /* pass the data to the higher layers only if there was no error */

    if (!(status & (ALTERA_AVALON_UART_STATUS_PE_MSK | 
                    ALTERA_AVALON_UART_STATUS_FE_MSK)))
    {
      if( cyg_hal_is_break( (cyg_uint8*) &data , 1 ) )
      *__ctrlc = 1;

      res = CYG_ISR_HANDLED;
    }
  }

  CYGARC_HAL_RESTORE_GP();
  return res;
}

/*---------------------------------------------------------------------*/

/*
 * Macro used to convert the supplied device name into a string.
 */

#define _DEV_NAME(dev) #dev
#define DEV_NAME(dev) _DEV_NAME(dev)

/*
 * Pointers to the console channel devices. altera_avalon_uart_chan[0] is the
 * channel used for debug communication, and altera_avalon_uart_chan[1] is the
 * channel used for console communication.
 *
 * The value NULL indicates that the channel is not connected.
 */ 

static altera_avalon_uart_chan uart_chan[2] = 
{
  {NULL, 0},
  {NULL, 0}
};

/*
 * Macro used to install the console driver if apropriate. The user defined macros:
 * CYGHWR_HAL_NIOS2_VV_DEBUG_DEV and CYGHWR_HAL_NIOS2_VV_CONSOLE_DEV name the 
 * devices to use for the debug and console channels respectively. If these names
 * are found to match a device of type altera_avalon_uart, then the channel is
 * connected to that device.
 *
 */

#define ALTERA_AVALON_UART_INSTANCE(name, device)                                  \
  if (!__builtin_strcmp (DEV_NAME(CYGHWR_HAL_NIOS2_VV_DEBUG_DEV), name##_NAME))    \
  {                                                                                \
    uart_chan[0].port = (cyg_uint8*) name##_BASE;                                  \
    uart_chan[0].irq  = name##_IRQ;                                                \
                                                                                   \
    HAL_INTERRUPT_MASK(name##_IRQ);                                                \
                                                                                   \
    CYGACC_CALL_IF_SET_CONSOLE_COMM(0);                                            \
    comm = CYGACC_CALL_IF_CONSOLE_PROCS();                                         \
    CYGACC_COMM_IF_CH_DATA_SET(*comm, &uart_chan[0]);                              \
    CYGACC_COMM_IF_WRITE_SET(*comm, altera_avalon_uart_diag_write);                \
    CYGACC_COMM_IF_READ_SET(*comm, altera_avalon_uart_diag_read);                  \
    CYGACC_COMM_IF_PUTC_SET(*comm, altera_avalon_uart_diag_putc);                  \
    CYGACC_COMM_IF_GETC_SET(*comm, altera_avalon_uart_diag_getc);                  \
    CYGACC_COMM_IF_CONTROL_SET(*comm, altera_avalon_uart_diag_control);            \
    CYGACC_COMM_IF_DBG_ISR_SET(*comm, altera_avalon_uart_diag_isr);                \
    CYGACC_COMM_IF_GETC_TIMEOUT_SET(*comm, altera_avalon_uart_diag_getc_timeout);  \
  }                                                                                \
                                                                                   \
  if (!__builtin_strcmp (DEV_NAME(CYGHWR_HAL_NIOS2_VV_CONSOLE_DEV), name##_NAME))  \
  {                                                                                \
    uart_chan[1].port = (cyg_uint8*) name##_BASE;                                  \
    uart_chan[1].irq  = name##_IRQ;                                                \
                                                                                   \
    HAL_INTERRUPT_MASK(name##_IRQ);                                                \
                                                                                   \
    CYGACC_CALL_IF_SET_CONSOLE_COMM(1);                                            \
    comm = CYGACC_CALL_IF_CONSOLE_PROCS();                                         \
    CYGACC_COMM_IF_CH_DATA_SET(*comm, &uart_chan[1]);                              \
    CYGACC_COMM_IF_WRITE_SET(*comm, altera_avalon_uart_diag_write);                \
    CYGACC_COMM_IF_READ_SET(*comm, altera_avalon_uart_diag_read);                  \
    CYGACC_COMM_IF_PUTC_SET(*comm, altera_avalon_uart_diag_putc);                  \
    CYGACC_COMM_IF_GETC_SET(*comm, altera_avalon_uart_diag_getc);                  \
    CYGACC_COMM_IF_CONTROL_SET(*comm, altera_avalon_uart_diag_control);            \
    CYGACC_COMM_IF_DBG_ISR_SET(*comm, altera_avalon_uart_diag_isr);                \
    CYGACC_COMM_IF_GETC_TIMEOUT_SET(*comm, altera_avalon_uart_diag_getc_timeout);  \
  }

#define FIFOED_AVALON_UART_INSTANCE(name, device)  ALTERA_AVALON_UART_INSTANCE(name, device)
/*--------------------------------------------------------------------- 
 * altera_avalon_uart_diag_init
 */

void altera_avalon_uart_diag_init (void) 
  {
    hal_virtual_comm_table_t* comm;
    int cur;

    cur = CYGACC_CALL_IF_SET_CONSOLE_COMM(CYGNUM_CALL_IF_SET_COMM_ID_QUERY_CURRENT);

#include <cyg/hal/devices.h>
 
    /* Restore original console */

    CYGACC_CALL_IF_SET_CONSOLE_COMM(cur);
};
