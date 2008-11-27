//==========================================================================
//
//      altera_avalon_lcd_16207.c
//
//      Device driver for the Altera Avalon LCD 16207 controller.
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
 * This file provides the implementation of the functions used to drive a
 * LCD panel.
 *
 * Characters written to the device will appear on the LCD panel as though
 * it is a very small terminal. If the lines written to the terminal are
 * longer than the number of characters on the terminal then it will scroll
 * the lines of text automatically to display them all.
 *
 * If more lines are written than will fit on the terminal then it will scroll
 * when characters are written to the line "below" the last displayed one - 
 * the cursor is allowed to sit below the visible area of the screen providing
 * that this line is entirely blank.
 *
 * The following control sequences may be used to move around and do useful
 * stuff:
 *    CR    Moves back to the start of the current line
 *    LF    Moves down a line and back to the start
 *    BS    Moves back a character without erasing
 *    ESC   Starts a VT100 style escape sequence
 *
 * The following escape sequences are recognised:
 *    ESC [ <row> ; <col> H   Move to row and column specified (positions are
 *                            counted from the top left which is 1;1)
 *    ESC [ K                 Clear from current position to end of line
 *    ESC [ 2 J               Clear screen and go to top left
 *
 * Note: the automatic scrolling when lines are longer than 80 columns is
 * not yet implemented.
 *
 */

#include <string.h>
#include <ctype.h>

#include <pkgconf/system.h>
#include <pkgconf/io.h>

#include <cyg/io/io.h>
#include <cyg/io/devtab.h>
#include <cyg/infra/diag.h>
#include <cyg/hal/hal_diag.h>

#include <altera_avalon_lcd_16207_regs.h>
#include <altera_avalon_lcd_16207.h> 

/* ---------------------------------------------------------------------
 *
 * Commands which can be written to the COMMAND register 
 */

enum /* Write to character RAM */
{
  LCD_CMD_WRITE_DATA    = 0x80
  /* Bits 6:0 hold character RAM address */
};

enum /* Write to character generator RAM */
{
  LCD_CMD_WRITE_CGR     = 0x40
  /* Bits 5:0 hold character generator RAM address */
};

enum /* Function Set command */
{
  LCD_CMD_FUNCTION_SET  = 0x20,
  LCD_CMD_8BIT          = 0x10,
  LCD_CMD_TWO_LINE      = 0x08,
  LCD_CMD_BIGFONT       = 0x04
};

enum /* Shift command */
{
  LCD_CMD_SHIFT         = 0x10,
  LCD_CMD_SHIFT_DISPLAY = 0x08,
  LCD_CMD_SHIFT_RIGHT   = 0x04
};

enum /* On/Off command */
{
  LCD_CMD_ONOFF         = 0x08,
  LCD_CMD_ENABLE_DISP   = 0x04,
  LCD_CMD_ENABLE_CURSOR = 0x02,
  LCD_CMD_ENABLE_BLINK  = 0x01
};

enum /* Entry Mode command */
{
  LCD_CMD_MODES         = 0x04,
  LCD_CMD_MODE_INC      = 0x02,
  LCD_CMD_MODE_SHIFT    = 0x01
};

enum /* Home command */
{
  LCD_CMD_HOME          = 0x02
};

enum /* Clear command */
{
  LCD_CMD_CLEAR         = 0x01
};

/* Where in LCD character space do the rows start */
static char colstart[4] = { 0x00, 0x40, 0x20, 0x60 };

/*
 * Only build the driver if this component has been selected by the user. 
 */

#ifdef CYGPKG_ALTERA_AVALON_LCD_16207

#include <pkgconf/io_serial.h>

/*--------------------------------------------------------------------- 
 * lcd_write_command
 */

static void lcd_write_command(alt_lcd_16207_dev * dev, unsigned char command)
{
  cyg_uint32 base = dev->base;

  /* We impose a timeout on the driver in case the LCD panel isn't connected.
   * The first time we call this function the timeout is approx 25ms 
   * (assuming 5 cycles per loop and a 200MHz clock).  Obviously systems
   * with slower clocks, or debug builds, or slower memory will take longer.
   */
  int i = 1000000;

  /* Don't bother if the LCD panel didn't work before */
  if (dev->broken)
    return;

  /* Wait until LCD isn't busy. */
  while (IORD_ALTERA_AVALON_LCD_16207_STATUS(base) & ALTERA_AVALON_LCD_16207_STATUS_BUSY_MSK)
    if (--i == 0)
    {
      dev->broken = 1;
      return;
    }

  /* Despite what it says in the datasheet, the LCD isn't ready to accept
   * a write immediately after it returns BUSY=0.  Wait for 100us more.
   */

  HAL_DELAY_US(100);

  IOWR_ALTERA_AVALON_LCD_16207_COMMAND(base, command);
}

/*--------------------------------------------------------------------- 
 * lcd_write_data
 */

static void lcd_write_data(alt_lcd_16207_dev * dev, unsigned char data)
{
  cyg_uint32 base = dev->base;

  /* We impose a timeout on the driver in case the LCD panel isn't connected.
   * The first time we call this function the timeout is approx 25ms 
   * (assuming 5 cycles per loop and a 200MHz clock).  Obviously systems
   * with slower clocks, or debug builds, or slower memory will take longer.
   */
  int i = 1000000;

  /* Don't bother if the LCD panel didn't work before */
  if (dev->broken)
    return;

  /* Wait until LCD isn't busy. */
  while (IORD_ALTERA_AVALON_LCD_16207_STATUS(base) & ALTERA_AVALON_LCD_16207_STATUS_BUSY_MSK)
    if (--i == 0)
    {
      dev->broken = 1;
      return;
    }

  /* Despite what it says in the datasheet, the LCD isn't ready to accept
   * a write immediately after it returns BUSY=0.  Wait for 100us more.
   */

  HAL_DELAY_US(100);

  IOWR_ALTERA_AVALON_LCD_16207_DATA(base, data);

  dev->address++;
}

/*--------------------------------------------------------------------- 
 * lcd_clear_screen
 */

static void lcd_clear_screen(alt_lcd_16207_dev * dev)
{
  int y;

  lcd_write_command(dev, LCD_CMD_CLEAR);

  dev->x = 0;
  dev->y = 0;
  dev->address = 0;

  for (y = 0 ; y < ALT_LCD_HEIGHT ; y++)
  {
    memset(dev->line[y].data, ' ', sizeof(dev->line[0].data));
    memset(dev->line[y].visible, ' ', sizeof(dev->line[0].visible));
    dev->line[y].width = 0;
  }
}

/*--------------------------------------------------------------------- 
 * lcd_repaint_screen
 */

static void lcd_repaint_screen(alt_lcd_16207_dev * dev)
{
  int y, x;

  /* scrollpos controls how much the lines have scrolled round.  The speed
   * each line scrolls at is controlled by its speed variable - while
   * scrolline lines will wrap at the position set by width
   */

  int scrollpos = dev->scrollpos;

  for (y = 0 ; y < ALT_LCD_HEIGHT ; y++)
  {
    int width  = dev->line[y].width;
    int offset = (scrollpos * dev->line[y].speed) >> 8;
    if (offset >= width)
      offset = 0;

    for (x = 0 ; x < ALT_LCD_WIDTH ; x++)
    {
      char c = dev->line[y].data[(x + offset) % width];

      /* Writing data takes 40us, so don't do it unless required */
      if (dev->line[y].visible[x] != c)
      {
        unsigned char address = x + colstart[y];

        if (address != dev->address)
        {
          lcd_write_command(dev, LCD_CMD_WRITE_DATA | address);
          dev->address = address;
        }

        lcd_write_data(dev, c);
        dev->line[y].visible[x] = c;
      }
    }
  }
}

/*--------------------------------------------------------------------- 
 * lcd_scroll_up
 */

static void lcd_scroll_up(alt_lcd_16207_dev * dev)
{
  int y;

  for (y = 0 ; y < ALT_LCD_HEIGHT ; y++)
  {
    if (y < ALT_LCD_HEIGHT-1)
      memcpy(dev->line[y].data, dev->line[y+1].data, ALT_LCD_VIRTUAL_WIDTH);
    else
      memset(dev->line[y].data, ' ', ALT_LCD_VIRTUAL_WIDTH);
  }

  dev->y--;
}

/*--------------------------------------------------------------------- 
 * lcd_handle_escape
 */

static void lcd_handle_escape(alt_lcd_16207_dev * dev, char c)
{
  int parm1 = 0, parm2 = 0;

  if (dev->escape[0] == '[')
  {
    char * ptr = dev->escape+1;
    while (isdigit(*ptr))
      parm1 = (parm1 * 10) + (*ptr++ - '0');

    if (*ptr == ';')
    {
      ptr++;
      while (isdigit(*ptr))
        parm2 = (parm2 * 10) + (*ptr++ - '0');
    }
  }
  else
    parm1 = -1;

  switch (c)
  {
  case 'H': /* ESC '[' <y> ';' <x> 'H'  : Move cursor to location */
  case 'f': /* Same as above */
    if (parm2 > 0)
      dev->x = parm2 - 1;
    if (parm1 > 0)
    {
      dev->y = parm1 - 1;
      if (dev->y > ALT_LCD_HEIGHT * 2)
        dev->y = ALT_LCD_HEIGHT * 2;
      while (dev->y > ALT_LCD_HEIGHT)
        lcd_scroll_up(dev);
    }
    break;

  case 'J':
    /*   ESC J      is clear to beginning of line    [unimplemented]
     *   ESC [ 0 J  is clear to bottom of screen     [unimplemented]
     *   ESC [ 1 J  is clear to beginning of screen  [unimplemented]
     *   ESC [ 2 J  is clear screen
     */
    if (parm1 == 2)
      lcd_clear_screen(dev);
    break;

  case 'K':
    /*   ESC K      is clear to end of line
     *   ESC [ 0 K  is clear to end of line
     *   ESC [ 1 K  is clear to beginning of line    [unimplemented]
     *   ESC [ 2 K  is clear line                    [unimplemented]
     */
    if (parm1 < 1)
    {
      if (dev->x < ALT_LCD_VIRTUAL_WIDTH)
        memset(dev->line[(int)dev->y].data + dev->x, ' ', ALT_LCD_VIRTUAL_WIDTH - dev->x);
    }
    break;
  }
}

/*--------------------------------------------------------------------- 
 * alt_lcd_16207_putc
 *
 * Send a character to the lcd. 
 */

static bool alt_lcd_16207_putc(serial_channel *chan, unsigned char c)
{
  alt_lcd_16207_dev *dev = (alt_lcd_16207_dev *)chan->dev_priv;

  int y;
  int widthmax;

  /* Tell the routine which is called off the timer interrupt that the
   * foreground routines are active so it must not repaint the display. */
  dev->active = 1;

  if (dev->esccount >= 0)
  {
    /* Single character escape sequences can end with any character
     * Multi character escape sequences start with '[' and contain
     * digits and semicolons before terminating
     */

    if ((dev->esccount == 0 && c != '[') ||
        (dev->esccount > 0 && !isdigit(c) && c != ';'))
    {
      dev->escape[(int)(dev->esccount)] = 0;

      lcd_handle_escape(dev, c);

      dev->esccount = -1;
    }
    else if (dev->esccount < sizeof(dev->escape)-1)
      dev->escape[(int) (dev->esccount++)] = c;
  }
  else if (c == 27) /* ESC */
  {
    dev->esccount = 0;
  }
  else if (c == '\r')
  {
    dev->x = 0;
  }
  else if (c == '\n')
  {
    dev->x = 0;
    dev->y++;

    /* Let the cursor sit at X=0, Y=HEIGHT without scrolling so the user
     * can print two lines of data without losing one.
     */
    if (dev->y > ALT_LCD_HEIGHT)
      lcd_scroll_up(dev);
  }
  else if (c == '\b')
  {
    if (dev->x > 0)
      dev->x--;
  }
  else if (isprint(c))
  {
    /* If we didn't scroll on the last linefeed then we might need to do
     * it now. */
    if (dev->y >= ALT_LCD_HEIGHT)
      lcd_scroll_up(dev);

    if (dev->x < ALT_LCD_VIRTUAL_WIDTH)
      dev->line[(int) (dev->y)].data[(int) (dev->x)] = c;

    dev->x++;
  }

  /* Recalculate the scrolling parameters */
  widthmax = ALT_LCD_WIDTH;
  for (y = 0 ; y < ALT_LCD_HEIGHT ; y++)
  {
    int width;
    for (width = ALT_LCD_VIRTUAL_WIDTH ; width > 0 ; width--)
      if (dev->line[y].data[width-1] != ' ')
        break;

    /* The minimum width is the size of the LCD panel.  If the real width
     * is long enough to require scrolling then add an extra space so the
     * end of the message doesn't run into the beginning of it.
     */
    if (width <= ALT_LCD_WIDTH)
      width = ALT_LCD_WIDTH;
    else
      width++;

    dev->line[y].width = width;
    if (widthmax < width)
      widthmax = width;
    dev->line[y].speed = 0; /* By default lines don't scroll */
  }

  if (widthmax <= ALT_LCD_WIDTH)
    dev->scrollmax = 0;
  else
  {
    widthmax *= 2;
    dev->scrollmax = widthmax;

    /* Now calculate how fast each of the other lines should go */
    for (y = 0 ; y < ALT_LCD_HEIGHT ; y++)
      if (dev->line[y].width > ALT_LCD_WIDTH)
      {
        /* You have three options for how to make the display scroll, chosen
         * using the preprocessor directives below
         */
#if 1
        /* This option makes all the lines scroll round at different speeds
         * which are chosen so that all the scrolls finish at the same time.
         */
        dev->line[y].speed = 256 * dev->line[y].width / widthmax;
#elif 1
        /* This option pads the shorter lines with spaces so that they all
         * scroll together.
         */
        dev->line[y].width = widthmax / 2;
        dev->line[y].speed = 256/2;
#else
        /* This option makes the shorter lines stop after they have rotated
         * and waits for the longer lines to catch up
         */
        dev->line[y].speed = 256/2;
#endif
      }
  }

  /* Repaint once, then check whether there has been a missed repaint
   * (because active was set when the timer interrupt occurred).  If there
   * has been a missed repaint then paint again.  And again.  etc.
   */

  for ( ; ; )
  {
    int old_scrollpos = dev->scrollpos;

    lcd_repaint_screen(dev);

    /* Let the timer routines repaint the display again */
    dev->active = 0;

    /* Have the timer routines tried to scroll while we were painting?
     * If not then we can exit */
    if (dev->scrollpos == old_scrollpos)
      break;

    /* We need to repaint again since the display scrolled while we were
     * painting last time */
    dev->active = 1;
  }
  return true;
}

/*--------------------------------------------------------------------- 
 * alt_lcd_16207_getc
 *
 * Read from the device. This isn't possible in our case, so we'd like to
 * return an error, but this isn't possible either. Therefore always return 0.
 */

static unsigned char alt_lcd_16207_getc(serial_channel *chan)
{
  return 0;
}

/*--------------------------------------------------------------------- 
 * alt_lcd_16207_set_config
 *
 * Function called by the higher layers to modify the behaviour of the device.
 * No configurable options are supported by this device.
 */

static Cyg_ErrNo alt_lcd_16207_set_config(serial_channel *chan, 
                                          cyg_uint32     key,
                                          const void     *xbuf, 
                                          cyg_uint32     *len)
{
  return -EINVAL;
}

/*--------------------------------------------------------------------- 
 * alt_lcd_16207_start_xmit
 *
 * Function used to start transmission. 
 */

static void alt_lcd_16207_start_xmit(serial_channel *chan)
{
}

/*--------------------------------------------------------------------- 
 * alt_lcd_16207_stop_xmit
 *
 * Function used to stop transmission.
 */

static void alt_lcd_16207_stop_xmit(serial_channel *chan)
{
}

/*--------------------------------------------------------------------- 
 * alt_lcd_16207_timeout
 */

static void alt_lcd_16207_timeout(cyg_handle_t alarmH, cyg_addrword_t context)
{
  alt_lcd_16207_dev * dev = (alt_lcd_16207_dev *) context;

  /* Update the scrolling position */
  if (dev->scrollpos + 1 >= dev->scrollmax)
    dev->scrollpos = 0;
  else
    dev->scrollpos = dev->scrollpos + 1;

  /* Repaint the panel unless the foreground will do it again soon */
  if (dev->scrollmax > 0 && !dev->active)
    lcd_repaint_screen(dev);
}

/*--------------------------------------------------------------------- 
 * alt_lcd_16207_init
 *
 * Initialise the device. 
 */

bool alt_lcd_16207_init(struct cyg_devtab_entry *tab)
{
  serial_channel *chan = (serial_channel *)tab->priv;
  alt_lcd_16207_dev *dev = (alt_lcd_16207_dev *)chan->dev_priv;

  cyg_handle_t counter;
  cyg_handle_t alarmH;
  cyg_handle_t sys_clock;

  cyg_resolution_t clock_res;

  cyg_uint32 period;
  cyg_uint32 base = dev->base;

  /* Mark the device as functional */
  dev->broken = 0;

  /* The initialisation sequence below is copied from the datasheet for
   * the 16207 LCD display.  The first commands need to be timed because
   * the BUSY bit in the status register doesn't work until the display
   * has been reset three times.
   */

  /* Wait for 15 ms then reset */
  HAL_DELAY_US(15000);
  IOWR_ALTERA_AVALON_LCD_16207_COMMAND(base, LCD_CMD_FUNCTION_SET | LCD_CMD_8BIT);

  /* Wait for another 4.1ms and reset again */
  HAL_DELAY_US(4100);  
  IOWR_ALTERA_AVALON_LCD_16207_COMMAND(base, LCD_CMD_FUNCTION_SET | LCD_CMD_8BIT);

  /* Wait a further 1 ms and reset a third time */
  HAL_DELAY_US(1000);
  IOWR_ALTERA_AVALON_LCD_16207_COMMAND(base, LCD_CMD_FUNCTION_SET | LCD_CMD_8BIT);

  /* Setup interface parameters: 8 bit bus, 2 rows, 5x7 font */
  lcd_write_command(dev, LCD_CMD_FUNCTION_SET | LCD_CMD_8BIT | LCD_CMD_TWO_LINE);
  
  /* Turn display off */
  lcd_write_command(dev, LCD_CMD_ONOFF);

  /* Clear display */
  lcd_clear_screen(dev);
  
  /* Set mode: increment after writing, don't shift display */
  lcd_write_command(dev, LCD_CMD_MODES | LCD_CMD_MODE_INC);

  /* Turn display on */
  lcd_write_command(dev, LCD_CMD_ONOFF | LCD_CMD_ENABLE_DISP);

  dev->esccount = -1;
  memset(dev->escape, 0, sizeof(dev->escape));

  dev->scrollpos = 0;
  dev->scrollmax = 0;
  dev->active = 0;

  sys_clock = cyg_real_time_clock();
  clock_res = cyg_clock_get_resolution(sys_clock);

  cyg_clock_to_counter(sys_clock, &counter);

  cyg_alarm_create(counter, 
                   alt_lcd_16207_timeout,
                  (cyg_addrword_t) dev,
                   &alarmH,
                   &dev->alarm);

  /* call every 100 ms. */

  if (clock_res.dividend >= 1000000000)
  {
    period = clock_res.divisor/(clock_res.dividend/100000000);   
  }
  else
  {
    period = (100000000*clock_res.divisor)/clock_res.dividend;
  }

  cyg_alarm_initialize(alarmH, cyg_current_time()+period, period);

  return true;
}

/*--------------------------------------------------------------------- 
 * alt_lcd_16207_lookup
 *
 * This is called to initialise a device upon first access.
 */

Cyg_ErrNo alt_lcd_16207_lookup(struct cyg_devtab_entry **tab, 
                               struct cyg_devtab_entry *sub_tab,
                               const char *name)
{
  serial_channel *chan = (serial_channel *)(*tab)->priv;
  (chan->callbacks->serial_init)(chan);
  return ENOERR;
}

/*--------------------------------------------------------------------- 
 *
 * Function table used by the device drivers.
 */

SERIAL_FUNS(alt_lcd_16207_funs,
            alt_lcd_16207_putc,
            alt_lcd_16207_getc,
            alt_lcd_16207_set_config,
            alt_lcd_16207_start_xmit,
            alt_lcd_16207_stop_xmit);

#endif /* CYGPKG_ALTERA_AVALON_LCD_16207 */
