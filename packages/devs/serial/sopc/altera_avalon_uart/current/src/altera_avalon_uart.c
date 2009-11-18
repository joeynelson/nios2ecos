//==========================================================================
//
//      altera_avalon_uart.c
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
 * This file provides the full serial device driver for the UART. This
 * driver is only used if the device is not being used for either the 
 * diagnostic console or GDB connection. 
 */

#include <string.h>

#include <pkgconf/system.h>
#include <cyg/infra/diag.h>

#include <altera_avalon_uart_regs.h>
#include <altera_avalon_uart.h> 

/*
 * Only build the driver if this component has been selected by the user. 
 */

#ifdef CYGPKG_ALTERA_AVALON_UART
#ifdef CYGPKG_IO_SERIAL

#include <pkgconf/io_serial.h>
#include <pkgconf/io.h>

/*
 * Macro used to convert the CDL supplied device name into a string.
 */

#define _DEV_NAME(dev) #dev
#define DEV_NAME(dev) _DEV_NAME(dev)


/*
 * Table used to translate the UART baud rate enumeration into an actual baud
 * rate. 
 */

static cyg_uint32 altera_avalon_uart_speed[] =
{
  50,
  75,
  110,
  134,
  150,
  200,
  300,
  600,
  1200,
  1800,
  2400,
  3600,
  4800,
  7200,
  9600,
  14400,
  19200,
  38400,
  57600,
  115200,
  230400
};

/*--------------------------------------------------------------------- 
 * altera_avalon_uart_putc
 *
 * Send a character to the UART. This function will fail if the device is
 * busy (i.e. it is already transmitting a character), or if hardware flow
 * control is being used and the remote end is not ready to receive. 
 */

static bool altera_avalon_uart_putc(serial_channel *chan, unsigned char c)
{
  altera_avalon_uart_dev *uart_chan = (altera_avalon_uart_dev *)chan->dev_priv;
  cyg_addrword_t port = uart_chan->base;
  cyg_uint32 status;

  status = IORD_ALTERA_AVALON_UART_STATUS(port);
  
  if (!(uart_chan->flags & ALT_AVALON_UART_FC)              ||
      !(chan->config.flags & CYGNUM_SERIAL_FLOW_RTSCTS_TX)  ||
      (status & ALTERA_AVALON_UART_STATUS_CTS_MSK))
  {
    if (status & ALTERA_AVALON_UART_STATUS_TRDY_MSK)
    {
      IOWR_ALTERA_AVALON_UART_TXDATA(port, c);
      return true;
    }
  }

  return false;
}

/*--------------------------------------------------------------------- 
 * altera_avalon_uart_getc
 *
 * Get a character from the UART, blocking if necessary.
 */

static unsigned char altera_avalon_uart_getc(serial_channel *chan)
{
  altera_avalon_uart_dev *uart_chan = (altera_avalon_uart_dev *)chan->dev_priv;
  cyg_addrword_t port = uart_chan->base;
  cyg_uint32 status;
  unsigned char data;

  do
  {
    do
    {
      status = IORD_ALTERA_AVALON_UART_STATUS(port);

      /* clear any error flags */

      IOWR_ALTERA_AVALON_UART_STATUS(port, 0);
    }
    while (!(status & ALTERA_AVALON_UART_CONTROL_RRDY_MSK));

    data = (unsigned char) IORD_ALTERA_AVALON_UART_RXDATA(port);
  }
  while (status & (ALTERA_AVALON_UART_STATUS_PE_MSK | 
                   ALTERA_AVALON_UART_STATUS_FE_MSK));
    
  return data;
}

/*--------------------------------------------------------------------- 
 * altera_avalon_uart_config_port
 *
 * Internal function to configure the hardware to desired baud rate, etc.
 */

static bool altera_avalon_uart_config_port(serial_channel    *chan, 
                                           cyg_serial_info_t *new_config, 
                                           bool              init)
{
  altera_avalon_uart_dev *uart_chan = (altera_avalon_uart_dev *)chan->dev_priv;

  /* 
   * There's no software control over the number of stop bits, parity or word 
   * length, so make sure it's not these that we're attempting to change.
   */

  if ((new_config->stop != chan->config.stop)     ||
      (new_config->parity != chan->config.parity) || 
      (new_config->word_length != chan->config.word_length)) 
  {
    return false;
  }

  /* 
   * Changing the Baud rate is only possible if the hardware has been 
   * configured to support it.
   */

  if ((uart_chan->flags & ALT_AVALON_UART_FB) &&
      (new_config->baud != chan->config.baud))
  {
    return false;
  }

  /*
   * Similarly we can only change the CTS/RTS settings if there's support for 
   * hardware flow control - and even in that case case, we can only change the
   * transmit, not the receive behaviour.
   */

  if ((!(uart_chan->flags & ALT_AVALON_UART_FC) &&
      (new_config->flags != chan->config.flags)) ||
      ((new_config->flags & ~CYGNUM_SERIAL_FLOW_RTSCTS_TX) != 
       (chan->config.flags & ~CYGNUM_SERIAL_FLOW_RTSCTS_TX))) 
  {
    return false;
  }

  /*
   * Given that the request is good, update the hardware and the status in 
   * software.
   */
  
  IOWR_ALTERA_AVALON_UART_DIVISOR(uart_chan->base, 
    ((uart_chan->freq/altera_avalon_uart_speed[new_config->baud]) - 1));

  if (&chan->config != new_config)
  {
    chan->config = *new_config;
  }

  return true;
}

/*--------------------------------------------------------------------- 
 * altera_avalon_uart_set_config
 *
 * Function called by the higher layers to modify the behaviour of the device.
 * This is translated into a call to the function 
 * altera_avalon_uart_config_port(), which is given above.
 */

static Cyg_ErrNo altera_avalon_uart_set_config(serial_channel *chan, 
                                               cyg_uint32     key,
                                               const void     *xbuf, 
                                               cyg_uint32     *len)
{
  cyg_serial_info_t *config = (cyg_serial_info_t *)xbuf;

  switch (key) 
  {
    case CYG_IO_SET_CONFIG_SERIAL_INFO:
      if ( *len < sizeof(cyg_serial_info_t) ) 
      {
        return -EINVAL;
      }
      *len = sizeof(cyg_serial_info_t);
      if ( true != altera_avalon_uart_config_port(chan, config, false) )
      {
        return -EINVAL;
      }
      break;
    default:
      return -EINVAL;
  }
    return ENOERR;
}

/*--------------------------------------------------------------------- 
 * altera_avalon_uart_start_xmit
 *
 * Function used to start transmission. 
 */

static void altera_avalon_uart_start_xmit(serial_channel *chan)
{
  cyg_uint32 ctrl;

  altera_avalon_uart_dev *uart_chan = (altera_avalon_uart_dev *)chan->dev_priv;

  (chan->callbacks->xmt_char)(chan);

  ctrl = IORD_ALTERA_AVALON_UART_CONTROL(uart_chan->base);
  IOWR_ALTERA_AVALON_UART_CONTROL(uart_chan->base, ctrl | 
                                    ALTERA_AVALON_UART_CONTROL_TRDY_MSK |
                                    ALTERA_AVALON_UART_CONTROL_DCTS_MSK);
}

/*--------------------------------------------------------------------- 
 * altera_avalon_uart_stop_xmit
 *
 * Function used to stop transmission when in interrupt driven mode. Since
 * this is a polled driver, a dummy implementation is provided here. 
 */

static void altera_avalon_uart_stop_xmit(serial_channel *chan)
{
  cyg_uint32 ctrl;

  altera_avalon_uart_dev *uart_chan = (altera_avalon_uart_dev *)chan->dev_priv;

  ctrl = IORD_ALTERA_AVALON_UART_CONTROL(uart_chan->base);
  IOWR_ALTERA_AVALON_UART_CONTROL(uart_chan->base, ctrl & 
                                    ~(ALTERA_AVALON_UART_CONTROL_TRDY_MSK |
                                    ALTERA_AVALON_UART_CONTROL_DCTS_MSK));
}

/*--------------------------------------------------------------------- 
 * altera_avalon_uart_ISR
 *
 * low level interrupt handler (ISR)
 */

static cyg_uint32 altera_avalon_uart_ISR(cyg_vector_t   vector, 
                                         cyg_addrword_t data)
{
    cyg_drv_interrupt_mask(vector);
    return CYG_ISR_CALL_DSR;  // Cause DSR to be run
}

/*--------------------------------------------------------------------- 
 * altera_avalon_uart_DSR
 *
 * high level interrupt handler (DSR)
 */

static void altera_avalon_uart_DSR(cyg_vector_t   vector, 
                                   cyg_ucount32   count, 
                                   cyg_addrword_t handle)
{
  cyg_uint32 status;

  serial_channel *chan              = (serial_channel *)handle;
  altera_avalon_uart_dev *uart_chan = (altera_avalon_uart_dev *)chan->dev_priv;
  cyg_addrword_t port               = uart_chan->base;
  cyg_uint32     data;

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
      (chan->callbacks->rcv_char)(chan, data);
    }
  }

  /* process a write irq */

  if (status & (ALTERA_AVALON_UART_STATUS_TRDY_MSK | 
                  ALTERA_AVALON_UART_STATUS_DCTS_MSK))
  {
    (chan->callbacks->xmt_char)(chan);
  }

  cyg_drv_interrupt_unmask(vector);
}

/*--------------------------------------------------------------------- 
 * altera_avalon_uart_init
 *
 * Initialize the device.
 *
 * The UART can safely be used as a diag device up until it is initialized.
 */

bool altera_avalon_uart_init(struct cyg_devtab_entry *tab)
{
  serial_channel *chan = (serial_channel *)tab->priv;
  altera_avalon_uart_dev *uart_chan = (altera_avalon_uart_dev *)chan->dev_priv;

	(chan->callbacks->serial_init)(chan);

	/* enable interrupts at the device */

	if (chan->out_cbuf.len != 0)
	{
	  cyg_drv_interrupt_create(uart_chan->irq,
							   99,                     /* Priority - unused */
							   (cyg_addrword_t)chan,   /* Data item passed to interrupt handler */
							   altera_avalon_uart_ISR,
							   altera_avalon_uart_DSR,
							   &uart_chan->serial_interrupt_handle,
							   &uart_chan->serial_interrupt);

	  cyg_drv_interrupt_attach(uart_chan->serial_interrupt_handle);

	  IOWR_ALTERA_AVALON_UART_CONTROL(uart_chan->base,
					  ALTERA_AVALON_UART_CONTROL_RTS_MSK  |
					  ALTERA_AVALON_UART_CONTROL_RRDY_MSK |
					  ALTERA_AVALON_UART_CONTROL_DCTS_MSK);

	  cyg_drv_interrupt_unmask(uart_chan->irq);
	}
	return true;
}

/*--------------------------------------------------------------------- 
 * altera_avalon_uart_lookup
 *
 * This is called to initialise a device upon first access.
 */

Cyg_ErrNo altera_avalon_uart_lookup(struct cyg_devtab_entry **tab, 
                                    struct cyg_devtab_entry *sub_tab,
                                    const char *name)
{
	return ENOERR;
}

/*--------------------------------------------------------------------- 
 *
 * Function table used by the device drivers.
 */

SERIAL_FUNS(altera_avalon_uart_funs,
            altera_avalon_uart_putc,
            altera_avalon_uart_getc,
            altera_avalon_uart_set_config,
            altera_avalon_uart_start_xmit,
            altera_avalon_uart_stop_xmit);

#endif /* CYGPKG_IO_SERIAL */
#endif /* CYGPKG_ALTERA_AVALON_UART */
