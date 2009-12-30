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

/*
#define FACTORY_FPGA_OFFSET 0x20000
#define APPLICATION_FPGA_OFFSET 0x120000
#define DEFLATOR_OFFSET 0x220000
#define MAIN_APPLICATION_OFFSET 0x240000
#define BOOTLOADER_OFFSET 0x500000
#define JFFS2_OFFSET 0x700000
#define JFFS2_LENGTH 0x100000
*/



#define FACTORY_FPGA_OFFSET 0x20000
#define BOOTLOADER_OFFSET 0x120000
#define APPLICATION_FPGA_OFFSET 0x200000
#define DEFLATOR_OFFSET 0x300000
#define MAIN_APPLICATION_OFFSET 0x320000
#define MAIN_APPLICATION_END 0x500000
#define JFFS2_LENGTH 0x100000
#define JFFS2_OFFSET (EXT_FLASH_SPAN - JFFS2_LENGTH)
