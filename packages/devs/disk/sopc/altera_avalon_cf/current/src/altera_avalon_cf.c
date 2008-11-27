//==========================================================================
//
//      altera_avalon_cf.c
//
//      Altera Avalon compact flash driver 
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
//==========================================================================

// This is a driver for the altera_avalon_cf compact flash device. This is 
// heavily based on the polled mode IDE driver originally written by IZ.

#include <pkgconf/altera_avalon_cf.h>

#include <cyg/infra/cyg_type.h>
#include <cyg/infra/cyg_ass.h>
#include <cyg/infra/diag.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_io.h>
#include <cyg/hal/hal_if.h>
#include <cyg/hal/drv_api.h>
#include <cyg/io/io.h>
#include <cyg/io/devtab.h>
#include <cyg/io/disk.h>

#include <cyg/hal/system.h>
#include <altera_avalon_cf_regs.h>

#include "altera_avalon_cf_priv.h"

// ----------------------------------------------------------------------------
// Macros used for debug

//#define DEBUG 1

#ifdef DEBUG
# define D(fmt,args...) diag_printf(fmt, ## args)
#else
# define D(fmt,args...)
#endif

// ----------------------------------------------------------------------------
// id_strcpy
//
// Copy a "string" from the devices identity data into a proper null teminated 
// string. 

static void id_strcpy(char *dest, cyg_uint16 *src, cyg_uint16 size)
{
  int i;

  for (i = 0; i < size; i+=2)
  {
    *dest++ = (char)(*src >> 8);
    *dest++ = (char)(*src & 0x00FF);
    src++;
  }
  *dest = '\0';
}

// ----------------------------------------------------------------------------
// wait_for_idle
//
// Wait until any outstanding IDE operations have completed. This will return 
// 1 for success, or zero in the event of a timeout.

static inline int wait_for_idle(cyg_uint32* ctlr)
{
  cyg_uint8 status;
  cyg_ucount32 tries;

  for (tries=0; tries<1000000; tries++) 
  {
    ALT_AVALON_CF_READ_UINT8(ctlr, IDE_REG_STATUS, status);
    if (!(status & IDE_STAT_BSY)) 
    {
      return 1;
    }
  }
  return 0;
}

// ----------------------------------------------------------------------------
// wait_for_drq
//
// Wait until the device reports that data is ready. This will return 1 for
// success, or zero in the event of a timeout.

static inline int wait_for_drq(cyg_uint32* ctlr)
{
  cyg_uint8 status;
  cyg_ucount32 tries;

  CYGACC_CALL_IF_DELAY_US(10);
  for (tries=0; tries<1000000; tries++) 
  {
    ALT_AVALON_CF_READ_UINT8(ctlr, IDE_REG_STATUS, status);
    if (!(status & IDE_STAT_BSY)) 
    {
      if (status & IDE_STAT_DRQ)
	return 1;
      else 
      {
	return 0;
      }
    }
  }
  return 0;
}

// ----------------------------------------------------------------------------
// ide_presence_detect
//
// Return true if a device is attached to the controller on a given channel 

static int ide_presence_detect(cyg_uint32* ctlr, int chan)
{
  cyg_uint16 sel, val;
  int i;
  
  sel = (chan << 4) | 0xA0;
  CYGACC_CALL_IF_DELAY_US((cyg_uint32)50000);
  ALT_AVALON_CF_WRITE_UINT16(ctlr, IDE_REG_DEVICE, sel);
  CYGACC_CALL_IF_DELAY_US((cyg_uint32)50000);
  ALT_AVALON_CF_READ_UINT16(ctlr, IDE_REG_DEVICE, val);

  if ((val & 0xff) == sel) 
  {
    return 1;
  }

  return 0;
}

// ----------------------------------------------------------------------------
// ide_ident
//
// Read out the contents of the devices identity buffer into the location
// pointed to by "buf". This function returns true for success, and false 
// otherwise.

static int ide_ident(cyg_uint32* ctlr, int dev, cyg_uint16 *buf)
{
  int i;
  
  wait_for_drq(ctlr);

  ALT_AVALON_CF_WRITE_UINT8(ctlr, IDE_REG_DEVICE, dev << 4);
  ALT_AVALON_CF_WRITE_UINT8(ctlr, IDE_REG_COMMAND, 0xEC);
  CYGACC_CALL_IF_DELAY_US((cyg_uint32)50000);

  if (!wait_for_drq(ctlr))
  {
    return 0;
  }

  for (i = 0; 
       i < (CYGDAT_ALTERA_AVALON_CF_SECTOR_SIZE / sizeof(cyg_uint16));
       i++, buf++)
  {
    ALT_AVALON_CF_READ_UINT16(ctlr, IDE_REG_DATA, *buf);
  }
  return 1;
}

// ----------------------------------------------------------------------------
// ide_read_sector
//
// read data from a specified sector into "buf". Upto "len" bytes of data will
// be read. This function returns true for success, and false otherwise. 

static int ide_read_sector(cyg_uint32* ctlr, int dev, cyg_uint32 start, 
			   cyg_uint8 *buf, cyg_uint32 len)
{
  int j, c;
  cyg_uint16 p;
  cyg_uint8 * b=buf;

  if (!wait_for_idle(ctlr))
  {
    return 0;
  }

  ALT_AVALON_CF_WRITE_UINT8(ctlr, IDE_REG_COUNT, 1);    // count =1
  ALT_AVALON_CF_WRITE_UINT8(ctlr, IDE_REG_LBALOW, start & 0xff);
  ALT_AVALON_CF_WRITE_UINT8(ctlr, IDE_REG_LBAMID, (start >>  8) & 0xff);
  ALT_AVALON_CF_WRITE_UINT8(ctlr, IDE_REG_LBAHI,  (start >> 16) & 0xff);
  ALT_AVALON_CF_WRITE_UINT8(ctlr, IDE_REG_DEVICE,
			    ((start >> 24) & 0xf) | (dev << 4) | 0x40);
  ALT_AVALON_CF_WRITE_UINT8(ctlr, IDE_REG_COMMAND, 0x20);

  if (!wait_for_drq(ctlr))
  {
    return 0;
  }

  // It would be fine if all buffers were word aligned, but who knows
  
  for (j = 0, c=0 ; j < (CYGDAT_ALTERA_AVALON_CF_SECTOR_SIZE / sizeof(cyg_uint16));
       j++) 
  {
    ALT_AVALON_CF_READ_UINT16(ctlr, IDE_REG_DATA, p);
    if (c++<len) *b++=p&0xff;
    if (c++<len) *b++=(p>>8)&0xff;
  }
  return 1;
}

// ----------------------------------------------------------------------------
// ide_write_sector
//
// write data from the input buffer "buf" to a specified sector within the
// device. This function returns true for success and false otherwise.

static int ide_write_sector(cyg_uint32* ctlr, int dev, cyg_uint32 start, 
			    cyg_uint8 *buf, cyg_uint32 len)
{
  int j, c;
  cyg_uint16 p;
  cyg_uint8 * b=buf;
  
  if (!wait_for_idle(ctlr))
  {
    return 0;
  }

  ALT_AVALON_CF_WRITE_UINT8(ctlr, IDE_REG_COUNT, 1);    // count =1
  ALT_AVALON_CF_WRITE_UINT8(ctlr, IDE_REG_LBALOW, start & 0xff);
  ALT_AVALON_CF_WRITE_UINT8(ctlr, IDE_REG_LBAMID, (start >>  8) & 0xff);
  ALT_AVALON_CF_WRITE_UINT8(ctlr, IDE_REG_LBAHI,  (start >> 16) & 0xff);
  ALT_AVALON_CF_WRITE_UINT8(ctlr, IDE_REG_DEVICE,
			    ((start >> 24) & 0xf) | (dev << 4) | 0x40);
  ALT_AVALON_CF_WRITE_UINT8(ctlr, IDE_REG_COMMAND, 0x30);
  
  if (!wait_for_drq(ctlr))
  {
    return 0;
  }
  
  // It would be fine if all buffers were word aligned, but who knows.

  for (j = 0, c=0 ; j < (CYGDAT_ALTERA_AVALON_CF_SECTOR_SIZE / sizeof(cyg_uint16));
       j++) 
  {
    p = (c++<len) ? *b++ : 0;
    p |= (c++<len) ? (*b++<<8) : 0; 
    ALT_AVALON_CF_WRITE_UINT16(ctlr, IDE_REG_DATA, p);
  }
  return 1;
}

// ----------------------------------------------------------------------------
// alt_avalon_cf_insert
//
// This function is called in response to an insert interrupt. It attempts to
// initialise the specified device. It returns true uponse success, and false
// otherwise. 

static cyg_bool alt_avalon_cf_insert(struct cyg_devtab_entry *tab)
{
  disk_channel *chan         = (disk_channel *) tab->priv;
  alt_avalon_cf_info_t *info = (alt_avalon_cf_info_t *) chan->dev_priv;
  
  cyg_uint32 id_buf[CYGDAT_ALTERA_AVALON_CF_SECTOR_SIZE/sizeof(cyg_uint32)];
  cyg_disk_identify_t ident;
  ide_identify_data_t *ide_idData=(ide_identify_data_t*)id_buf;
    
  // Check to see if there's a device available
  
  if (!ide_presence_detect(info->ide_base, info->chan))
  {
    return false;
  }

  // Having found a device, interogate it for its parameters 

  D("IDE %d identify drive\n", info->chan);
    
  if (!ide_ident(info->ide_base, info->chan, (cyg_uint16 *)id_buf)) 
  {
    return false;
  }

  id_strcpy(ident.serial, ide_idData->serial,       20);
  id_strcpy(ident.firmware_rev, ide_idData->firmware_rev, 8);
  id_strcpy(ident.model_num, ide_idData->model_num,    40);

  ident.cylinders_num  = ide_idData->num_cylinders;
  ident.heads_num = ide_idData->num_heads;
  ident.sectors_num = ide_idData->num_sectors;
  ident.lba_sectors_num = ide_idData->lba_total_sectors[1] << 16 | 
    ide_idData->lba_total_sectors[0];
    
  D("\tSerial : %s\n", ident.serial);
  D("\tFirmware rev. : %s\n", ident.firmware_rev);
  D("\tModel : %s\n", ident.model_num);
  D("\tC/H/S : %d/%d/%d\n", ident.cylinders_num, 
    ident.heads_num, ident.sectors_num);
  D("\tKind : %x\n", (ide_idData->general_conf>>8)&0x1f);
  
  // Now do the generic initialisation

  if (!(chan->callbacks->disk_init)(tab))
    return false;

  if (ENOERR != (chan->callbacks->disk_connected)(tab, &ident))
  {
    return false;
  }
  return true;
}

// ----------------------------------------------------------------------------
// altera_avalon_cf_reset
//
// Reset the interface when it's first connected

static void alt_avalon_cf_reset(void* ctl_base)
{
  IOWR_ALTERA_AVALON_CF_CTL_CONTROL(ctl_base, 
				    ALTERA_AVALON_CF_CTL_STATUS_POWER_MSK |
				    ALTERA_AVALON_CF_CTL_STATUS_RESET_MSK);
  CYGACC_CALL_IF_DELAY_US(5000);
  IOWR_ALTERA_AVALON_CF_CTL_CONTROL(ctl_base, 
				    ALTERA_AVALON_CF_CTL_STATUS_POWER_MSK);
  CYGACC_CALL_IF_DELAY_US((cyg_uint32)1000000);
}

// ----------------------------------------------------------------------------
// altera_avalon_cf_ISR
//
// Interrupt service routine for the insert and remove interrupt. All processing
// for this interrupt is defered to the associated DSR.

static cyg_uint32 altera_avalon_cf_ISR(cyg_vector_t   vector, 
				       cyg_addrword_t data)
{
  cyg_drv_interrupt_mask(vector);
  return CYG_ISR_CALL_DSR;  // Cause DSR to be run
}

// ----------------------------------------------------------------------------
// altera_avalon_cf_DSR
//
// DSR for the insert and remove interrupt. This updates the devices current
// state based on the eject status.

static void altera_avalon_cf_DSR(cyg_vector_t   vector, 
				 cyg_ucount32   count, 
				 cyg_addrword_t handle)
{
  alt_avalon_cf_interface_t* interface = (alt_avalon_cf_interface_t*) handle;
  struct cyg_devtab_entry *dev;
  alt_avalon_cf_info_t *info;
  disk_channel *chan;
  int i;

  if (IORD_ALTERA_AVALON_CF_CTL_STATUS(interface->ctl_base)
            & ALTERA_AVALON_CF_CTL_STATUS_PRESENT_MSK)
  {
    alt_avalon_cf_reset(interface->ctl_base);

    // insert channels

    for (i = 0; i < ALT_CF_NCHANNELS; i++)
    {
      if (interface->dev[i])
      {
	dev  = interface->dev[i];
	chan = (disk_channel *) dev->priv;
	info = (alt_avalon_cf_info_t *) chan->dev_priv;

	if (!alt_avalon_cf_insert(interface->dev[i]))
        {
	  (chan->callbacks->disk_disconnected)(dev);
	}
      }
    }
  }
  else
  {
    // remove channels

    for (i = 0; i < ALT_CF_NCHANNELS; i++)
    {
      if (interface->dev[i])
      {
	dev  = interface->dev[i];
	chan = (disk_channel *) dev->priv;
	info = (alt_avalon_cf_info_t *) chan->dev_priv;
	
	(chan->callbacks->disk_disconnected)(dev);
      }
    }
  }

  // re-enable the interrupt

  cyg_drv_interrupt_unmask(vector);
}

// ----------------------------------------------------------------------------
// alt_avalon_cf_init
//
// Initialise the interface. This will return true if the device is currently
// present, otherwise it will return false.

cyg_bool alt_avalon_cf_init(struct cyg_devtab_entry *tab)
{
  disk_channel *chan         = (disk_channel *) tab->priv;
  alt_avalon_cf_info_t *info = (alt_avalon_cf_info_t *) chan->dev_priv;

  alt_avalon_cf_interface_t* interface = info->interface;
  cyg_uint32* ctl_base                 = interface->ctl_base;

  cyg_uint32 id_buf[CYGDAT_ALTERA_AVALON_CF_SECTOR_SIZE/sizeof(cyg_uint32)];
  cyg_disk_identify_t ident;
  ide_identify_data_t *ide_idData=(ide_identify_data_t*)id_buf;

  int rval;
    
  if (chan->init) 
  {
    return true;
  }

  D("IDE(%d:%d) hw init\n", info->ide_base, info->chan);

  // turn the power on

  IOWR_ALTERA_AVALON_CF_CTL_CONTROL(ctl_base, 
				    ALTERA_AVALON_CF_CTL_STATUS_POWER_MSK);

  // Reset the interface at boot time.

  if (interface->init_done == 0)
  {
    alt_avalon_cf_reset(ctl_base);
  }

  // record the device table entry for future use

  interface->dev[info->chan] = tab;

  // clear any pending interrupts
    
  if (interface->init_done == 0)
  {
    IORD_ALTERA_AVALON_CF_CTL_STATUS(ctl_base);
  }

  // attempt to insert the device

  rval = alt_avalon_cf_insert(interface->dev[info->chan]);

  // enable insert detect irq

  if (interface->init_done == 0)
  {
    // clear any pending interrupts
    
    IORD_ALTERA_AVALON_CF_CTL_STATUS(ctl_base);

    // enable the interrupt

    cyg_drv_interrupt_create(interface->irq,
			     99,                         /* Priority - unused */
			     (cyg_addrword_t)interface, 
			     altera_avalon_cf_ISR,
			     altera_avalon_cf_DSR,
			     &interface->cf_interrupt_handle,
			     &interface->cf_interrupt);

    cyg_drv_interrupt_attach(interface->cf_interrupt_handle);

    IOWR_ALTERA_AVALON_CF_CTL_CONTROL(ctl_base, 
				      ALTERA_AVALON_CF_CTL_STATUS_POWER_MSK
				      + ALTERA_AVALON_CF_CTL_STATUS_IRQ_EN_MSK);
    cyg_drv_interrupt_unmask(interface->irq);
    interface->init_done = 1;
  }

  return rval;
}

// ----------------------------------------------------------------------------
// alt_avalon_cf_lookup
//
// Lookup an instance of this device. This operation is handled by the higher
// layers.

Cyg_ErrNo alt_avalon_cf_lookup(struct cyg_devtab_entry **tab, 
			       struct cyg_devtab_entry  *sub_tab,
			       const char *name)
{
  disk_channel *chan = (disk_channel *) (*tab)->priv;
  return (chan->callbacks->disk_lookup)(tab, sub_tab, name);
}

// ----------------------------------------------------------------------------
// alt_avalon_cf_read
//
// Read data from the specified block.

static Cyg_ErrNo alt_avalon_cf_read(disk_channel *chan, 
				    void         *buf,
				    cyg_uint32    len, 
				    cyg_uint32    block_num)
{
  alt_avalon_cf_info_t *info = (alt_avalon_cf_info_t *)chan->dev_priv;
  
  D("IDE %d read block %d\n", info->chan, block_num);
  
  if (!ide_read_sector(info->ide_base, info->chan, block_num, 
		       (cyg_uint8 *)buf, len)) 
  {
    return -EIO; 
  }

  return ENOERR;
}

// ----------------------------------------------------------------------------
// alt_avalon_cf_write
//
// Write data to the specified block number

static Cyg_ErrNo alt_avalon_cf_write(disk_channel *chan, 
				     const void   *buf,
				     cyg_uint32    len, 
				     cyg_uint32    block_num)
{
  alt_avalon_cf_info_t *info = (alt_avalon_cf_info_t *)chan->dev_priv;
  
  D("IDE %d write block %d\n", info->chan, block_num);
  
  if (!ide_write_sector(info->ide_base, info->chan, block_num, 
			(cyg_uint8 *)buf, len)) 
  {
    return -EIO; 
  }
        
  return ENOERR;
}

// ----------------------------------------------------------------------------
// alt_avalon_cf_get_config
//
// This is an empty stub required by the device function table.

static Cyg_ErrNo
alt_avalon_cf_get_config(disk_channel *chan, 
			 cyg_uint32    key,
			 const void   *xbuf, 
			 cyg_uint32   *len)
{
  return -EINVAL;
}

// ----------------------------------------------------------------------------
// alt_avalon_cf_set_config
//
// This is an empty stub required by the device function table.

static Cyg_ErrNo
alt_avalon_cf_set_config(disk_channel *chan, 
			 cyg_uint32    key,
			 const void   *xbuf, 
			 cyg_uint32   *len)
{
  return -EINVAL;
}

// ----------------------------------------------------------------------------
// Device function table. This is used by all instances of this device.

DISK_FUNS(alt_avalon_cf_funs, 
	  alt_avalon_cf_read, 
	  alt_avalon_cf_write, 
	  alt_avalon_cf_get_config,
	  alt_avalon_cf_set_config
);

//EOF altera_avalon_cf.c
