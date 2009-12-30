//========================================================================
// ####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003 Free Software Foundation, Inc.
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
//========================================================================
#ifndef _CONSTANTS_H_
#define _CONSTANTS_H_

#define BOOT_OK					0
#define BOOT_FLASH_INIT			(BOOT_OK + 1)
#define BOOT_FLASH_UNLOCK		(BOOT_FLASH_INIT + 1)
#define BOOT_MOUNT_JFFS			(BOOT_FLASH_UNLOCK + 1)
#define BOOT_MOUNT_RAMFS		(BOOT_MOUNT_JFFS + 1)
#define BOOT_FLASH_ERASE		(BOOT_MOUNT_RAMFS + 1)
#define BOOT_FLASH_PROGRAM		(BOOT_FLASH_ERASE + 1)
#define BOOT_LENGTH				(BOOT_FLASH_PROGRAM	+ 1)

static const char * error_messages[] =
		{
				"",
				"Initializing flash failed",
				"Unlocking flash failed",
				"Mounting JFFS2 failed",
				"Mounting RAMFS failed",
				"Erasing flash failed",
				"Programming flash failed"
		};

struct upgrade_info
{
	char* file;
	char * name;
	int start_address;
	size_t length;
};

upgrade_info bootloader = {"/ram/bootloader.phi", "Bootloader", FACTORY_FPGA_OFFSET, APPLICATION_FPGA_OFFSET
		- FACTORY_FPGA_OFFSET};
upgrade_info firmware = {"/ram/firmware.phi", "Firmware", APPLICATION_FPGA_OFFSET, MAIN_APPLICATION_END
		- APPLICATION_FPGA_OFFSET};

#endif // _CONSTANTS_H_
