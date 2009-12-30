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

#include <cyg/hal/io.h>
#include <cyg/hal/system.h>
#include <sys/stat.h>
#include <sys/stat.h>
#include <cyg/io/serialio.h>
#include <cyg/io/flash.h>
#include <fcntl.h>
#include <cyg/compress/zlib.h>

#include <cyg/compress/zlib.h>

#include <cyg/firmwareutil/firmwareutil.h>

static cyg_uint8 inflateBuffer[8192];
static cyg_uint8 inputBuffer[8192];

static const char * tmp_file = "/ram/tmp";

struct info_forward
{
	void *data;
	struct cyg_upgrade_info *upgraded_file;
};


static void report_info(void *data, const char * format, ... )
{
	va_list arguments;
	struct info_forward *fw = data;
	va_start(arguments, format);
	fw->upgraded_file->print_info(fw->data, format, arguments);
	va_end(arguments);
}

/* check that the firmware file contains the expected string. */
static bool expect(void *data, int file, const char *strin)
{
	size_t i;
	for (i = 0; i < strlen(strin); i++)
	{
		char t;
		if (read(file, &t, 1) != 1)
		{
			report_info(data, "Error: reading firmware file , expecting \"%s\"\n", strin);

			lseek(file, -(i+1), SEEK_CUR);
			return false;
		}
		if (t != strin[i])
		{
			report_info(data, "Unexpected data in firmware file while expecting %s\n", strin);
			return false;
		}
	}
	return true;
}

/* Check header, guzip to temp file in ram and return actual length
 * of decompressed file. */
static bool gunzip_firmware(void *data, struct cyg_upgrade_info upgraded_file, int *actuallength)
{
	int tmpFile;
	int firmwareFile;
	if ((firmwareFile = open(upgraded_file.file, O_RDONLY)) <= 0)
	{
		report_info(data, "Error: could not open %s\n", upgraded_file.file);
		return false;
	}

	if ((tmpFile = open(tmp_file, O_RDWR | O_CREAT | O_TRUNC)) <= 0)
	{
		report_info(data, "Error: could not open temp file");

		close(firmwareFile);
		return false;
	}

	if (!expect(data, firmwareFile, upgraded_file.header))
	{
		report_info(data, "invalid image uploaded. Safely aborting upgrade.\n");

		close(tmpFile);
		close(firmwareFile);
		return false;
	}

	z_stream d_stream; /* decompression stream */
	d_stream.zalloc = Z_NULL;
	d_stream.zfree = Z_NULL;
	d_stream.opaque = Z_NULL;

	d_stream.next_in  = inputBuffer; /* We will copy 0 bytes from this area... */
	d_stream.avail_in = 0;

	int err = inflateInit(&d_stream);

	if (err == Z_MEM_ERROR)
	{
		report_info(data, "Error: Failed to deflate application %d\n", err);

		close(tmpFile);
		close(firmwareFile);
		return false;
	}

	int offset = 0;
	int actual;
	for (;;)
	{
		/* fill up as much of the buffer as possible... if we don't have
		 * "enough" data available, then inflate will fail. */
		if (d_stream.avail_in < sizeof(inputBuffer))
		{
			memmove(inputBuffer, d_stream.next_in, d_stream.avail_in);

			actual = read(firmwareFile, inputBuffer + d_stream.avail_in, sizeof(inputBuffer) - d_stream.avail_in);
			if (actual < 0)
			{
				report_info(data, "Error: failed to read from firmware file");

				inflateEnd(&d_stream);
				close(tmpFile);
				close(firmwareFile);
				return false;
			} else
			{
				/* We read some more from the input file... */
			}
			d_stream.avail_in = d_stream.avail_in + actual;
			d_stream.next_in  = inputBuffer;
		}

		d_stream.avail_out = sizeof(inflateBuffer);
		d_stream.next_out = inflateBuffer;

		int err = inflate(&d_stream, Z_NO_FLUSH);
		if ((err != Z_STREAM_END) && (err != Z_OK))
		{
			report_info(data, "Error: failed to deflate application %d\n", err);

			inflateEnd(&d_stream);
			close(tmpFile);
			close(firmwareFile);
			return false;
		}

		int len = sizeof(inflateBuffer) - d_stream.avail_out;

		actual = write(tmpFile, inflateBuffer, len);
		if (actual != len)
		{
			report_info(data, "Error: Failed to write to temp file\n");

			inflateEnd(&d_stream);
			close(tmpFile);
			close(firmwareFile);
			return false;
		}

		offset += len;

		if (err == Z_STREAM_END)
		{
			/* Done! */
			break;
		}

	}

	*actuallength = offset;

	inflateEnd(&d_stream);
	close(tmpFile);
	close(firmwareFile);
	return true;
}

static char flashbuffer[65536];

/* Load firmware file, gunzip it and program into flash */
bool cyg_firmware_upgrade(void *caller_data, struct cyg_upgrade_info upgraded_file)
{
	/* NB! we only erase/program as much as we have to! */
	int actuallength;

	struct info_forward fwd;
	fwd.data = caller_data;
	fwd.upgraded_file = &upgraded_file;
	void *data = &fwd;

	if (!gunzip_firmware(data, upgraded_file, &actuallength))
		return false;

	int firmwareFile;
	if ((firmwareFile = open(tmp_file, O_RDONLY)) <= 0)
	{
		report_info(data, "Error: could not open %s\n", tmp_file);
		return false;
	}

	report_info(data, "%s update in progress\n", upgraded_file.name);

	cyg_uint8 *saved_startAddr, *startAddr;
	saved_startAddr = startAddr = (cyg_uint8 *) (upgraded_file.flash_base + upgraded_file.start_address);

	int stat;
	void *err_addr;

#ifdef CYGHWR_IO_FLASH_BLOCK_LOCKING
	if ((stat = flash_unlock((void *) startAddr, actuallength,
		(void **) &err_addr)) != 0)
		{
			report_info(data, "Error: unlocking flash failed(%d, %p): %s\n", stat, err_addr, cyg_flash_errmsg(stat));
			close(firmwareFile);
			return false;
		}
#endif

	report_info(data, "Erasing flash...");
	if ((stat = flash_erase((void *) (startAddr), actuallength, (void **) &err_addr)) != 0)
	{
		report_info(data, "Error: erasing flash failed(%d, %p): %s\n", stat, err_addr, cyg_flash_errmsg(stat));
		close(firmwareFile);
		return false;
	}
	report_info(data, "done.\n");

	int actual;
	report_info(data, "Programming flash at %p %d bytes", startAddr, actuallength);
	int write_times = 1;
	while ((actual = read(firmwareFile, flashbuffer, sizeof(flashbuffer))) > 0)
	{
		int stat;
		void *err_addr;

		/* Leave the rest of the flash unwritten to. */
		memset(flashbuffer + actual, 0xff, sizeof(flashbuffer)-actual);

		int bytes_to_write = (actual + 3) & ~0x3; /* align to 32 bits */

		stat = flash_program(startAddr, flashbuffer, bytes_to_write, (void **)&err_addr);
		if (stat != 0)
		{
			report_info(data, "Error: programming flash failed(%d, %p): %s\n", stat, err_addr, cyg_flash_errmsg(stat));
			close(firmwareFile);
			return false;
		}
		report_info(data, ".");
		if ((write_times++%32)==0)
		{
			report_info(data, "\n");
		}

		startAddr += actual;
	}
	report_info(data, "done.\n");

	if (saved_startAddr + actuallength != startAddr)
	{
		report_info(data, "Warning: firmware file corrupt, wrote incorrect amount of data to flash.\n");
	}

	close(firmwareFile);

	remove(upgraded_file.file);

	if (actual < 0)
	{
		report_info(data, "Error: catastrophic failure. %s corrupt\n", upgraded_file.name);
		return false;
	}

	report_info(data, "%s successfully updated.\n", upgraded_file.name);

	return true;
}

