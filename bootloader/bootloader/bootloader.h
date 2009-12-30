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
#ifndef _BOOTLOADER_H_
#define _BOOTLOADER_H_

#include <sstream>
#include <string>

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <cyg/kernel/kapi.h>
#include <sys/select.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <pkgconf/io_flash.h>
#include <cyg/fileio/fileio.h>
#include <cyg/io/config_keys.h>
#include <cyg/io/flash.h>
#include <cyg/io/io.h>
#include <cyg/hal/hal_diag.h>
#include <errno.h>
#include "redboot.h"
#include "xyzModem.h"


bool getChar(char *key);
bool waitChar(int seconds, char *key);

#endif // _BOOTLOADER_H_
