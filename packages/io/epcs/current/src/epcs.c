//==========================================================================
//
//      flashiodev.c
//
//      Flash device interface to I/O subsystem
//
//==========================================================================
// ####ECOSGPLCOPYRIGHTBEGIN####                                            
// -------------------------------------------                              
// This file is part of eCos, the Embedded Configurable Operating System.   
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2004, 2007, 2009 Free Software Foundation, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under    
// the terms of the GNU General Public License as published by the Free     
// Software Foundation; either version 2 or (at your option) any later      
// version.                                                                 
//
// eCos is distributed in the hope that it will be useful, but WITHOUT      
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or    
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License    
// for more details.                                                        
//
// You should have received a copy of the GNU General Public License        
// along with eCos; if not, write to the Free Software Foundation, Inc.,    
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.            
//
// As a special exception, if other files instantiate templates or use      
// macros or inline functions from this file, or you compile this file      
// and link it with other works to produce a work based on this file,       
// this file does not by itself cause the resulting work to be covered by   
// the GNU General Public License. However the source code for this file    
// must still be made available in accordance with section (3) of the GNU   
// General Public License v2.                                               
//
// This exception does not invalidate any other reasons why a work based    
// on this file might be covered by the GNU General Public License.         
// -------------------------------------------                              
// ####ECOSGPLCOPYRIGHTEND####                                              
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    jlarmour
// Contributors: woehler, Andrew Lunn
// Date:         2004-12-03
// Purpose:      
// Description:  
//              
//####DESCRIPTIONEND####
//
//==========================================================================

// INCLUDES

#include <pkgconf/system.h>
#include <cyg/hal/system.h>
#include <stddef.h>

#include <pkgconf/io_epcs.h>
#include <errno.h>

#include <cyg/io/devtab.h>
#include <string.h> // memcpy
#include <cyg/hal/hal_if.h>
#include "sys/alt_llist.h"


#include <cyg/io/config_keys.h>

#include <cyg/io/altera_avalon_epcs_flash_controller.h>
#include <cyg/io/epcs.h>
// MACROS

#define MIN(x,y) ((x)<(y) ? (x) : (y))
#define MAX(x,y) ((x)>(y) ? (x) : (y))

// FUNCTION PROTOTYPES

static bool epcs_init( struct cyg_devtab_entry *tab );
static Cyg_ErrNo
epcs_lookup(struct cyg_devtab_entry **tab,
                  struct cyg_devtab_entry  *sub_tab,
                  const char *name) ;
static Cyg_ErrNo
epcs_bread( cyg_io_handle_t handle, void *buf, cyg_uint32 *len,
                  cyg_uint32 pos);
static Cyg_ErrNo
epcs_bwrite( cyg_io_handle_t handle, const void *buf, cyg_uint32 *len,
                   cyg_uint32 pos );
static Cyg_ErrNo
epcs_get_config( cyg_io_handle_t handle,
                       cyg_uint32 key,
                       void* buf,
                       cyg_uint32* len);
static Cyg_ErrNo
epcs_set_config( cyg_io_handle_t handle,
                       cyg_uint32 key,
                       const void* buf,
                       cyg_uint32* len);

// STATICS/GLOBALS


BLOCK_DEVIO_TABLE( cyg_io_epcs_ops,
                   &epcs_bwrite,
                   &epcs_bread,
                   NULL, // no select
                   &epcs_get_config,
                   &epcs_set_config
                   );


BLOCK_DEVTAB_ENTRY( cyg_io_epcs,
                    "/dev/epcs",
                    NULL,
                    &cyg_io_epcs_ops,
                    &epcs_init,
                    &epcs_lookup,
                    NULL );

// new EPCS instance
ALTERA_AVALON_EPCS_FLASH_CONTROLLER_INSTANCE( EPCS_FLASH_CONTROLLER, flash);

static alt_flash_fd *p_epcs_fd;
static flash_region *p_epcs_reg_info;
static int num_epcs_regs;

// FUNCTIONS
static bool epcs_init( struct cyg_devtab_entry *tab )
{
	int ret = -ENOERR;
	ret = ALTERA_AVALON_EPCS_FLASH_CONTROLLER_INIT( EPCS_FLASH_CONTROLLER, flash);
	return ret == -ENOERR;
} // epcs_init()

Cyg_ErrNo epcs_open(void)
{
	int ret = -ENOERR;
	// Get pointer to flash info structure
    p_epcs_fd = (alt_flash_fd *)alt_flash_open_dev(EPCS_FLASH_CONTROLLER_NAME);

    // Verify flash details
    ret = alt_epcs_flash_get_info(p_epcs_fd, &p_epcs_reg_info, &num_epcs_regs);

    return ret;

} // epcs open

static Cyg_ErrNo
epcs_lookup(struct cyg_devtab_entry **tab,
                  struct cyg_devtab_entry  *sub_tab,
                  const char *name)
{
	return epcs_open();
} // epcs_lookup()

static Cyg_ErrNo
epcs_bread( cyg_io_handle_t handle, void *buf, cyg_uint32 *len, cyg_uint32 pos)
{
	struct alt_flash_dev *flash_info = (alt_flash_dev *)p_epcs_fd;
	return alt_epcs_flash_read(flash_info, pos, buf, *len);
} // epcs_bread()

static Cyg_ErrNo
epcs_bwrite( cyg_io_handle_t handle, const void *buf, cyg_uint32 *len, cyg_uint32 pos )
{
	int ret = -ENOERR, j;
	struct alt_flash_dev *flash_info = (alt_flash_dev *)p_epcs_fd;

	ret = epcs_flash_write(flash_info, pos, buf, *len);

	if(ret == -ENOERR)
	{
		ret = alt_epcs_flash_memcmp(flash_info, buf, pos, *len);
	}
	return ret;
} // epcs_bwrite()

int cyg_epcs_erase_blocks(alt_flash_dev *flash_info, cyg_flashaddr_t offset,
						size_t len, cyg_flashaddr_t *err_address)
{
	int         ret_code = 0;
	int         i,j;
	int         data_to_write;
	int         current_offset;
	alt_u8		erased_sample[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
	int 		size = sizeof(erased_sample) / sizeof(alt_u8);

	/*
	 * First and foremost which sectors are affected?
	*/
	for(i = 0; i < flash_info->number_of_regions; i++)
	{
		/* Is it in this erase block region?*/
		if((offset >= flash_info->region_info[i].offset) &&
	      (offset < (flash_info->region_info[i].offset +
	      flash_info->region_info[i].region_size)))
	    {
			current_offset = flash_info->region_info[i].offset;
			while(len > 0)
			{
				if ((offset >= current_offset ) &&
	            (offset < (current_offset +
	            flash_info->region_info[i].block_size)))
				{
					bool erased = true;
					for (j = 0; j < flash_info->region_info[i].block_size;  j += size) {
						if (alt_epcs_flash_memcmp(flash_info, erased_sample, current_offset + j, size)) {
							erased = false;
							break;
						}
					}
					if (!erased) {
						//diag_printf("erasing %0xd\n", current_offset);
						ret_code = (*flash_info->erase_block)(flash_info, current_offset);
					}
					else
					{
						//diag_printf("erased %0xd\n", current_offset);
					}

					len -= flash_info->region_info[i].block_size;
					offset = current_offset + flash_info->region_info[i].block_size;
				}
				current_offset += flash_info->region_info[i].block_size;
			}
		}
	 }
	return -ENOERR;
} // cyg_epcs_erase_blocks


int epcs_flash_write(alt_flash_dev* flash_info, int offset,
                          const void* src_addr, int length)
{
	int         ret_code = 0;
	int         i, j;
	int         data_to_write;
	int         current_offset;

	/*
	 * First and foremost which sectors are affected?
	*/
	for(i = 0; i < flash_info->number_of_regions; i++)
	{
		/* Is it in this erase block region?*/
		if((offset >= flash_info->region_info[i].offset) &&
			(offset < (flash_info->region_info[i].offset +
			flash_info->region_info[i].region_size)))
		{
			current_offset = flash_info->region_info[i].offset;

			for(j = 0; j < flash_info->region_info[i].number_of_blocks; j++)
			{
				if ((offset >= current_offset ) &&
					(offset < (current_offset +
					flash_info->region_info[i].block_size)))
				{
					/*
					 * Check if the contents of the block are different
					 * from the data we wish to put there
					 */
					data_to_write = ( current_offset + flash_info->region_info[i].block_size
									- offset);
					data_to_write = MIN(data_to_write, length);

					if(alt_epcs_flash_memcmp(flash_info, src_addr, offset, data_to_write))
					{
						ret_code = (*flash_info->write_block)(
                                                  flash_info,
                                                  current_offset,
                                                  offset,
                                                  src_addr,
                                                  data_to_write);
					}

					/* Was this the last block? */
					if ((length == data_to_write) || ret_code)
					{
						goto finished;
					}

					length -= data_to_write;
					offset = current_offset + flash_info->region_info[i].block_size;
					src_addr = (alt_u8*)src_addr + data_to_write;
				}
				current_offset += flash_info->region_info[i].block_size;
			}
		}
	}
	finished:
		return ret_code;
} // epcs_flash_write


static Cyg_ErrNo
epcs_get_config( cyg_io_handle_t handle,
                       cyg_uint32 key,
                       void* buf,
                       cyg_uint32* len)
{

	struct alt_flash_dev *flash_info = (alt_flash_dev *)p_epcs_fd;
	switch (key) {
		case CYG_IO_GET_CONFIG_FLASH_ERASE:
		{
			cyg_io_flash_getconfig_erase_t *e = (cyg_io_flash_getconfig_erase_t *)buf;
			cyg_flashaddr_t startpos = e->offset;

			e->flasherr = cyg_epcs_erase_blocks(flash_info, startpos, e->len, &e->err_address );

			return e->flasherr;
		}
    
		case CYG_IO_GET_CONFIG_FLASH_DEVSIZE:
		{
			cyg_io_flash_getconfig_devsize_t *d = (cyg_io_flash_getconfig_devsize_t *)buf;

			long size = 0;
			int i;
			for(i = 0; i < flash_info->number_of_regions; i++)
			{
				size += flash_info->region_info[i].region_size;
			}
			d->dev_size = size;
			return -ENOERR;
		}

		case CYG_IO_GET_CONFIG_FLASH_DEVADDR:
		{
			#ifdef CYGPKG_INFRA_DEBUG // don't bother checking this all the time
				if ( *len != sizeof( cyg_io_flash_getconfig_devaddr_t ) )
					return -EINVAL;
			#endif
			cyg_io_flash_getconfig_devaddr_t *d = (cyg_io_flash_getconfig_devaddr_t *)buf;
			d->dev_addr = (int)flash_info->base_addr;
			return -ENOERR;
		}

		case CYG_IO_GET_CONFIG_FLASH_BLOCKSIZE:
		{
			cyg_io_flash_getconfig_blocksize_t *b =  (cyg_io_flash_getconfig_blocksize_t *)buf;
			b->block_size = flash_info->region_info[0].block_size;
			return -ENOERR;
		}
		default:
			return -EINVAL;
	}
} // epcs_get_config()

static Cyg_ErrNo
epcs_set_config( cyg_io_handle_t handle,
                       cyg_uint32 key,
                       const void* buf,
                       cyg_uint32* len)
{
	struct alt_flash_dev *flash_info = (alt_flash_dev *)p_epcs_fd;
	switch (key) {
		case CYG_IO_SET_CONFIG_CLOSE:
		{
			alt_flash_close_dev(p_epcs_fd);
			return -ENOERR;
		}
		default:
			return -EINVAL;
	}
} // epcs_set_config()

// EOF epcs.c
