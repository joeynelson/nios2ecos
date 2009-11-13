//==========================================================================
//
//      altera_avalon_jtag_uart.c
//
//      Device driver for the Altera Avalon JTAG UART.
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
 * This file provides the full serial device driver for the JTAG UART. This
 * driver is only used if the device is not being used for either the 
 * diagnostic console or GDB connection.
 */

#include <string.h>

#include <pkgconf/system.h>
#include <cyg/infra/diag.h>

#include <altera_avalon_jtag_uart_regs.h>
#include <altera_avalon_jtag_uart.h> 

/*
 * Only build the driver if this component has been selected by the user. 
 */

#ifdef CYGPKG_ALTERA_AVALON_JTAG_UART
#ifdef CYGPKG_IO_SERIAL

#ifdef CYGOPT_IO_SERIAL_FLOW_CONTROL_SOFTWARE
#error "The JTAG UART PC Software does not support Software flow control"
#endif

#include <pkgconf/io_serial.h>
#include <cyg/io/io.h>                  /* I/O functions */

#define _DEV_NAME(dev) #dev
#define DEV_NAME(dev) _DEV_NAME(dev)

/*--------------------------------------------------------------------- 
 * altera_avalon_jtag_uart_putc
 *
 * Send a character to the JTAG UART. This function will fail if the device is
 * busy (i.e. it is already transmitting). 
 */

static bool altera_avalon_jtag_uart_putc(serial_channel *chan, unsigned char c)
{
  altera_avalon_jtag_uart_dev *jtag_uart_chan = 
                                (altera_avalon_jtag_uart_dev *)chan->dev_priv;
  cyg_addrword_t port               = jtag_uart_chan->base;
  cyg_uint32 control;
  bool ret_code;
  
  control = IORD_ALTERA_AVALON_JTAG_UART_CONTROL(port);
  
  if (control & ALTERA_AVALON_JTAG_UART_CONTROL_WSPACE_MSK)
  {
    IOWR_ALTERA_AVALON_JTAG_UART_DATA(port, c);
    ret_code = true;
  }
  else
  {
    ret_code = false;
  }
  
  return ret_code;

}

/*--------------------------------------------------------------------- 
 * altera_avalon_jtag_uart_getc
 *
 * Get a character from the UART, blocking if necessary.
 */

static unsigned char altera_avalon_jtag_uart_getc(serial_channel *chan)
{
  altera_avalon_jtag_uart_dev *jtag_uart_chan = 
                                (altera_avalon_jtag_uart_dev *)chan->dev_priv;
  cyg_addrword_t port               = jtag_uart_chan->base;
  cyg_uint32 data;

  do 
  {
    data = IORD_ALTERA_AVALON_JTAG_UART_DATA(port);
  }
  while (!(data & ALTERA_AVALON_JTAG_UART_DATA_RVALID_MSK));

  return (data & ALTERA_AVALON_JTAG_UART_DATA_DATA_MSK) 
                 >> ALTERA_AVALON_JTAG_UART_DATA_DATA_OFST;
}

/*--------------------------------------------------------------------- 
 * altera_avalon_jtag_uart_start_xmit
 *
 * Function used to start transmission. 
 */

static void altera_avalon_jtag_uart_start_xmit(serial_channel *chan)
{
  altera_avalon_jtag_uart_dev *jtag_uart_chan = 
                                (altera_avalon_jtag_uart_dev *)chan->dev_priv;
  cyg_addrword_t port               = jtag_uart_chan->base;
  cyg_uint32     control;
  unsigned int  status;

  control = IORD_ALTERA_AVALON_JTAG_UART_CONTROL(port);
  control |= ALTERA_AVALON_JTAG_UART_CONTROL_WE_MSK;
  IOWR_ALTERA_AVALON_JTAG_UART_CONTROL(port, control);

  return;
}

/*--------------------------------------------------------------------- 
 * altera_avalon_jtag_uart_stop_xmit
 *
 * Function used to stop transmission when in interrupt driven mode. Since
 * this is a polled driver, a dummy implementation is provided here. 
 */

static void altera_avalon_jtag_uart_stop_xmit(serial_channel *chan)
{
  altera_avalon_jtag_uart_dev *jtag_uart_chan = 
                                (altera_avalon_jtag_uart_dev *)chan->dev_priv;
  cyg_addrword_t port               = jtag_uart_chan->base;
  cyg_uint32     control;
  unsigned int  status;
  
  control = IORD_ALTERA_AVALON_JTAG_UART_CONTROL(port);
  control &= ~ALTERA_AVALON_JTAG_UART_CONTROL_WE_MSK;
  IOWR_ALTERA_AVALON_JTAG_UART_CONTROL(port, control);

  return;
}

/*--------------------------------------------------------------------- 
 * altera_avalon_jtag_uart_set_config
 *
 * Function called by the higher layers to modify the behaviour of the device.
 *
 * There's nothing to configure so do nothing and return an error
 */

static Cyg_ErrNo altera_avalon_jtag_uart_set_config(serial_channel *chan, 
                                                    cyg_uint32 key,
                                                    const void *xbuf, 
                                                    cyg_uint32 *len)
{
  altera_avalon_jtag_uart_dev *jtag_uart_chan = 
                            (altera_avalon_jtag_uart_dev *)chan->dev_priv;
  cyg_addrword_t port = jtag_uart_chan->base;
  Cyg_ErrNo ret_code;
  cyg_uint32     control;

  switch (key)
  {
  case CYG_IO_SET_CONFIG_SERIAL_HW_RX_FLOW_THROTTLE:
    { 
      if(*len != 4)
      {
        ret_code = -EINVAL;
      }
      else 
      {
        if (*(char*)xbuf == 1) 
        {
          /*
           * Throttle Rx
           */
          control = IORD_ALTERA_AVALON_JTAG_UART_CONTROL(port);
          control &= ~ALTERA_AVALON_JTAG_UART_CONTROL_RE_MSK;
          IOWR_ALTERA_AVALON_JTAG_UART_CONTROL(port, control);
          ret_code = ENOERR;
        }
        else if (*(char*)xbuf == 0)
        {
          /*
           * Restart Rx
           */
          control = IORD_ALTERA_AVALON_JTAG_UART_CONTROL(port);
          control |= ALTERA_AVALON_JTAG_UART_CONTROL_RE_MSK;
          IOWR_ALTERA_AVALON_JTAG_UART_CONTROL(port, control);
          ret_code = ENOERR;
        }
        else
        {
          ret_code = -EINVAL;
        }
      }
      break;
    }
  default:
    {
      ret_code = -EINVAL;
      break;
    }
  }

  return ret_code;
}


/*--------------------------------------------------------------------- 
 * altera_avalon_jtag_uart_ISR
 *
 * low level interrupt handler (ISR)
 */

static cyg_uint32 altera_avalon_jtag_uart_ISR(cyg_vector_t   vector, 
                                              cyg_addrword_t data)
{
    cyg_drv_interrupt_mask(vector);
    return CYG_ISR_CALL_DSR;  // Cause DSR to be run
}

/*--------------------------------------------------------------------- 
 * altera_avalon_jtag_uart_DSR
 *
 * high level interrupt handler (DSR)
 */

static void altera_avalon_jtag_uart_DSR(cyg_vector_t   vector, 
                                        cyg_ucount32   count, 
                                        cyg_addrword_t handle)
{
  cyg_uint32 status;

  serial_channel *chan              = (serial_channel *)handle;
  altera_avalon_jtag_uart_dev *jtag_uart_chan = 
                            (altera_avalon_jtag_uart_dev *)chan->dev_priv;
  cyg_addrword_t port               = jtag_uart_chan->base;
  cyg_uint32     data, control;
  int            space_avail, chars_avail, i, tx_space;
  unsigned char* fifo;

  /*
   * Read the status register in order to determine the cause of the
   * interrupt.
   */

  control = IORD_ALTERA_AVALON_JTAG_UART_CONTROL(port);

  if (control & ALTERA_AVALON_JTAG_UART_CONTROL_WI_MSK)
  {
    space_avail = (control & ALTERA_AVALON_JTAG_UART_CONTROL_WSPACE_MSK) >>
                  ALTERA_AVALON_JTAG_UART_CONTROL_WSPACE_OFST;

    (chan->callbacks->data_xmt_req)(chan, space_avail, &chars_avail, &fifo);

    for (i=0;i<chars_avail;i++)
    {
      IOWR_ALTERA_AVALON_JTAG_UART_DATA(port, *(fifo+i));
    }
    (chan->callbacks->data_xmt_done)(chan, chars_avail);
  }

  if (control & ALTERA_AVALON_JTAG_UART_CONTROL_RI_MSK)
  {
    data = IORD_ALTERA_AVALON_JTAG_UART_DATA(port);
    /*
     * Pass the read character to the upper layer 
     */
    (chan->callbacks->rcv_char)(chan, (data & 
                ALTERA_AVALON_JTAG_UART_DATA_DATA_MSK) 
                >> ALTERA_AVALON_JTAG_UART_DATA_DATA_OFST);

    chars_avail = (data & ALTERA_AVALON_JTAG_UART_DATA_RAVAIL_MSK) >> 
                        ALTERA_AVALON_JTAG_UART_DATA_RAVAIL_OFST;

    if (chars_avail)
    {
#ifndef CYGOPT_IO_SERIAL_FLOW_CONTROL_HW
      for (i=0;i<chars_avail;i++)
      {
        data = IORD_ALTERA_AVALON_JTAG_UART_DATA(port);
        (chan->callbacks->rcv_char)(chan, (data & 
                ALTERA_AVALON_JTAG_UART_DATA_DATA_MSK) 
                >> ALTERA_AVALON_JTAG_UART_DATA_DATA_OFST);
      }
#else
      if (chan->callbacks->data_rcv_req(chan, chars_avail, 
                    &chars_avail, &fifo) == CYG_RCV_OK)
      {
        for(i=0;i<chars_avail;i++)
        {
          *(fifo+i) = IORD_ALTERA_AVALON_JTAG_UART_DATA(port);
        }
      }

      (chan->callbacks->data_rcv_done)(chan, chars_avail);
#endif /* CYGOPT_IO_SERIAL_FLOW_CONTROL_HW */
    }
  }

  cyg_drv_interrupt_unmask(vector);
}


/*--------------------------------------------------------------------- 
 * altera_avalon_jtag_uart_init
 *
 * Initialise the device. 
 */

bool altera_avalon_jtag_uart_init(struct cyg_devtab_entry *tab)
{
  serial_channel *chan = (serial_channel *)tab->priv;
  altera_avalon_jtag_uart_dev *jtag_uart_chan = (altera_avalon_jtag_uart_dev *)chan->dev_priv;

  (chan->callbacks->serial_init)(chan); 

  if (chan->out_cbuf.len != 0) 
  {
    cyg_drv_interrupt_create(jtag_uart_chan->irq,
                             99,                     /* Priority - unused */
                             (cyg_addrword_t)chan,   /* Data item passed to interrupt handler */
                             altera_avalon_jtag_uart_ISR,
                             altera_avalon_jtag_uart_DSR,
                             &jtag_uart_chan->serial_interrupt_handle,
                             &jtag_uart_chan->serial_interrupt);

    cyg_drv_interrupt_attach(jtag_uart_chan->serial_interrupt_handle);
  }
  return true;

}

/*--------------------------------------------------------------------- 
 * altera_avalon_jtag_uart_lookup
 *
 * This is called to initialise a device upon first access.
 */

Cyg_ErrNo altera_avalon_jtag_uart_lookup(struct cyg_devtab_entry **tab, 
                                         struct cyg_devtab_entry *sub_tab,
                                         const char *name)
{
  serial_channel *chan = (serial_channel *)(*tab)->priv;
  altera_avalon_jtag_uart_dev *jtag_uart_chan = (altera_avalon_jtag_uart_dev *)chan->dev_priv;

  if (__builtin_strcmp (DEV_NAME(CYGHWR_HAL_NIOS2_VV_DEBUG_DEV), (*tab)->name) &&
      __builtin_strcmp (DEV_NAME(CYGHWR_HAL_NIOS2_VV_CONSOLE_DEV), (*tab)->name))
  {
    (chan->callbacks->serial_init)(chan);

    /* enable interrupts at the device */
    IOWR_ALTERA_AVALON_JTAG_UART_CONTROL(jtag_uart_chan->base, 
                                ALTERA_AVALON_JTAG_UART_CONTROL_RE_MSK); 

    cyg_drv_interrupt_unmask(jtag_uart_chan->irq);

    return ENOERR;
  }
  else
  {
    return -EPERM;
  }

}

/*--------------------------------------------------------------------- 
 *
 * Function table used by the device drivers.
 */

SERIAL_FUNS(altera_avalon_jtag_uart_funs,
            altera_avalon_jtag_uart_putc,
            altera_avalon_jtag_uart_getc,
            altera_avalon_jtag_uart_set_config,
            altera_avalon_jtag_uart_start_xmit,
            altera_avalon_jtag_uart_stop_xmit);

#endif /* CYGPKG_IO_SERIAL */
#endif /* CYGPKG_ALTERA_AVALON_JTAG_UART */

