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
#include <cyg/io/serialio.h>

#include <cyg/compress/zlib.h>

#include <cyg/firmwareutil/firmwareutil.h>

#include "addresses.h"
#include "bootloader.h"

 /* nios2-gdb-server will go ga-ga when we invoke the remote
  * update stuff, disable while debugging */
#define DEBUG_NO_RESET() 0

#define UNCACHED_EXT_FLASH_BASE (0x80000000 + EXT_FLASH_BASE)

static FILE *ser_fp;

/* naive conversion of \n to \r\n for serial terminal */
static void report_info(void *data, const char * format, va_list args)
{
	char buffer[1024];
	vsnprintf(buffer, sizeof(buffer), format, args);
	int i;
	for (i = 0; i < strlen(buffer); i++)
	{
		if (buffer[i] == '\n')
		{
			fprintf(ser_fp, "\r");
		}
		fprintf(ser_fp, "%c", buffer[i]);
	}
}

struct cyg_upgrade_info bootloader =
{
	(cyg_uint8 *)UNCACHED_EXT_FLASH_BASE,
	"/ram/bootloader.phi",
	"Bootloader",
	FACTORY_FPGA_OFFSET,
	APPLICATION_FPGA_OFFSET - FACTORY_FPGA_OFFSET,
	"ZylinNiosBootloader\n",
	report_info
};

struct cyg_upgrade_info firmware =
{
	(cyg_uint8 *)UNCACHED_EXT_FLASH_BASE,
	"/ram/firmware.phi",
	"Firmware",
	APPLICATION_FPGA_OFFSET,
	JFFS2_OFFSET - APPLICATION_FPGA_OFFSET,
	"ZylinNiosFirmware\n",
	report_info
};


static char IP_FILE[] = "/config/ip";
static const int WRITE_BUF_SIZE = 4096;
static char WRITE_BUF[WRITE_BUF_SIZE];

int ser = -1;

void cleaning()
{
	fclose(ser_fp);
	close(ser);

	/* 1000ms is necessary to let everything "calm down" */
	cyg_thread_delay(100);
}

bool getChar(char *key)
{
	fd_set rfds;
	int retval;

	/* Watch stdin (fd 0) to see when it has input. */
	FD_ZERO(&rfds);
	FD_SET(ser, &rfds);

	retval = select(1, &rfds, NULL, NULL, NULL);
	/* Don't rely on the value of tv now! */
	if (retval)
	{
		if (FD_ISSET(ser, &rfds))
		{
			if (read(ser, key, 1) == 1)
			{
				return true;
			}
		}
	}
	return false;
}

bool waitChar1(int seconds, char *key)
{
	fd_set rfds;
	struct timeval tv;
	int retval;
	static char chr[1024];

	/* Watch stdin (fd 0) to see when it has input. */
	FD_ZERO(&rfds);
	FD_SET(ser, &rfds);
	/* Wait this long seconds. */
	tv.tv_sec = seconds;
	tv.tv_usec = 0;

	retval = select(1, &rfds, NULL, NULL, &tv);
	/* Don't rely on the value of tv now! */
	if (retval)
	{
		if (FD_ISSET(ser, &rfds))
		{ // linux, for win see getChar
			if (read(ser, chr, 1024) > 0)
			{
				*key = chr[0];
				return true;
			}
		}
		return true;

	}

	return false;
}

bool waitChar(int seconds, char *key)
{
	fd_set rfds;
	struct timeval tv;
	int retval;

	/* Watch stdin (fd 0) to see when it has input. */
	FD_ZERO(&rfds);
	FD_SET(ser, &rfds);

	/* Wait this long seconds. */
	tv.tv_sec = seconds;
	tv.tv_usec = 0;

	retval = select(1, &rfds, NULL, NULL, &tv);
	/* Don't rely on the value of tv now! */
	if (retval)
	{
		if (FD_ISSET(ser, &rfds))
		{
			if (read(ser, key, 1) == 1)
			{
				return true;
			}
		}
	}
	return false;
}

void reset(void)
{
	fprintf(ser_fp, "Resetting\r\n");
	umount("/config");
	cleaning();
#if DEBUG_NO_RESET()
	/* DEBUG: should have reconfigured here, disabled to make debugging easier.
	 * We no longer have serial output capability */
	for (;;);
#else
	IOWR(REMOTE_UPDATE_BASE, 0x20, 0x1);
#endif
}

void openSerial()
{
	ser = open(UART_0_NAME, O_RDWR|O_SYNC|O_NONBLOCK);
	if (ser < 0)
	{
		diag_printf("Error: serial device problems %s\r\n", UART_0_NAME);
		reset();
	}

	ser_fp = fdopen(ser, "r+");

	if (ser_fp == NULL)
	{
		diag_printf("Error: serial device problems %s\r\n", UART_0_NAME);
		reset();
	}
}


/* Try to mount jffs2, if it fails print error message and return.
 * Failing is "normal" in that formatting jffs2 might fix it for
 * instance.
 */
void mountJFFS2()
{
	Cyg_ErrNo err = 0;
	err = cyg_flash_init(NULL);
	if (err)
	{
		fprintf(ser_fp, "Error: could not init flash\r\n");
		return;
	}

	cyg_flashaddr_t err_address;

#ifdef CYGHWR_IO_FLASH_BLOCK_LOCKING
	if ((err = flash_unlock((void *) UNCACHED_EXT_FLASH_BASE, EXT_FLASH_SPAN,
			(void **) &err_address)) != 0)
	{
		fprintf(ser_fp, "Error: could not unlock flash\r\n");
		return;
	}
#endif

	if ((err = mount(CYGDAT_IO_FLASH_BLOCK_DEVICE_NAME_1, "/config", "jffs2")) < 0)
	{
		fprintf(ser_fp, "Error: could not mount flash %d\r\n", err);
		return;
	}
}

/* mount ramfs, print error message and return in case of error.
 *
 * This fn can not really fail, except if we run out of memory. */
void mountRamFS()
{
	Cyg_ErrNo err = 0;

	if ((err = mount("", "/ram", "ramfs")) < 0)
	{
		fprintf(ser_fp, "Error: could not mount RAMFS %d\r\n", err);
		return;
	}
}

/* erase all sectors in this address range */
static void flash_erase_range(void *address, int length)
{
	int stat;
	void *err_addr;

	if ((stat = flash_init(0)) != 0)
	{
		fprintf(ser_fp, "Error: %s\r\n", "Initializing flash failed");
		reset();
	}

#ifdef CYGHWR_IO_FLASH_BLOCK_LOCKING
	if ((stat = flash_unlock((void *) UNCACHED_EXT_FLASH_BASE, EXT_FLASH_SPAN,
			(void **) &err_addr)) != 0)
	{
		fprintf(ser_fp, "Error: %s\r\n", "Unlocking flash failed");
		reset();
	}
#endif

	fprintf(ser_fp, "Erasing flash at %p, 0x%08x bytes\r\n", address, length);
	if ((stat = flash_erase((void *)address, length,
			(void **) &err_addr)) != 0)
	{
		fprintf(ser_fp, "Error: %s\r\n", "Erasing flash failed");
		reset();
	}
}


/* Format jffs2 area and reset */
void format(void)
{
	fprintf(ser_fp, "Formatting JFFS2...\r\n");

	flash_erase_range((void *)(UNCACHED_EXT_FLASH_BASE + JFFS2_OFFSET), JFFS2_LENGTH);

	fprintf(ser_fp, "/config formatted successfully\r\n");
	reset();
}

int firmwareFile;

// Read a string in from serial port and reset if
// anything goes wrong.
static void readLine(char *buffer,
// Including terminating \0
		int maxLen)
{
	int index = 0;
	for (;;)
	{
		char c = 0;
		getChar(&c);
		switch (c)
		{
		case 0x3:
			fprintf(ser_fp, "%s\r\n", "\r\nCtrl-c pressed");
			reset();
		case '\n':
		case '\r':
			if (index == 0)
			{
				fprintf(ser_fp, "%s\r\n", "\r\nEmpty string not allowed");
				reset();
			}
			buffer[index] = '\0';
			fprintf(ser_fp, "\r\n");
			return;
			// backspace
		case 0x08:
			if (index > 0)
			{
				index--;
				fprintf(ser_fp, "%c %c", c, c);
			}
			break;
		default:
			if (index >= (maxLen - 1))
			{
				fprintf(ser_fp, "%s\r\n", "\r\nString too long");
				reset();
			}
			fprintf(ser_fp, "%c", c);
			buffer[index] = c;
			index++;
			break;
		}
	}
}

static void getFileName(char *name, int maxLen)
{
	readLine(name, maxLen);
}

/* write config file based on 0 terminated string */
static void writeFile(const char *fileName, const char *string)
{
	if (strlen(string) == 0)
	{
		unlink(fileName);
	}
	else
	{
		int fd = creat(fileName, O_CREAT | O_TRUNC);
		if (fd < 0)
		{
			fprintf(ser_fp, "unable to create %s\r\n", fileName);
		}
		else
		{
			write(fd, string, strlen(string));
			close(fd);
		}
	}
}

static void enterParameter()
{
	char name[81];
	char param[128];
	//get IP
	fprintf(ser_fp, "Enter file name: ");
	readLine(name, sizeof(name));
	fprintf(ser_fp, "Enter parameter: ");
	readLine(param, sizeof(param));
	writeFile(name, param);

}

static void showParameter()
{
	char name[81];
	//get IP
	fprintf(ser_fp, "Enter filename: ");
	readLine(name, sizeof(name));

	int param;
	if ((param = open(name, O_RDONLY)) < 0)
	{
		fprintf(ser_fp, "Could not open %s\r\n", name);
	}

	fprintf(ser_fp, "Displaying up to 1024 bytes of that parameter\r\n");
	for (int i = 0; i < 1024; i++)
	{
		char c;
		int actual;
		actual = read(param, &c, 1);
		if (actual < 0)
		{
			fprintf(ser_fp, "\r\nFailed while reading %s", name);
			reset();
		}
		if (actual != 1)
			break;
		fprintf(ser_fp, "%c", c);
	}
	fprintf(ser_fp, "\r\n");

}

static void wrongMAC()
{
	fprintf(ser_fp, "Error: wrong MAC address syntax\r\n");
	reset();
}

static void transformMacAddress(char* buffer, cyg_uint8 mac_addr[6])
{
	memset(mac_addr, 0, 6);

	if (strlen(buffer) != 12)
	{
		wrongMAC();
	}

	for (size_t i = 0; i < strlen(buffer); i++)
	{
		char c = toupper(buffer[i]);
		if ('0' <= c && c <= '9')
		{
			cyg_uint8 val = c - '0';
			mac_addr[i / 2] += (val << (4 * ((i + 1) % 2)));
		}
		else
		if ('A' <= c && c <= 'F')
		{
			cyg_uint8 val = c - 'A' + 10;
			mac_addr[i / 2] += (val << (4 * ((i + 1) % 2)));
		}
		else
		{
			wrongMAC();
		}
	}
}

static cyg_uint8 * const macAddr = (cyg_uint8 *) (UNCACHED_EXT_FLASH_BASE + FACTORY_FPGA_OFFSET - 6);
static const int macAddrLen = 6;

static bool hasMacAddress()
{
	cyg_uint8 ret = 0xFF;
	for(int i = 0; i < macAddrLen; i++)
	{
		ret = ret & macAddr[i];
	}
	return ret != 0xFF;
}

static void printMACAddress()
{
	if (hasMacAddress())
	{
		cyg_uint8* mac = (cyg_uint8 *) (UNCACHED_EXT_FLASH_BASE + FACTORY_FPGA_OFFSET - 6);
		fprintf(ser_fp, "Mac address %02x:%02x:%02x:%02x:%02x:%02x\r\n", mac[0],
				mac[1], mac[2], mac[3], mac[4], mac[5]);
	}
	else
	{
		fprintf(ser_fp, "Mac address not set\r\n");
	}
}

static void changeMac(void)
{
	char mac[13];
	cyg_uint8 ui_mac[6];

	fprintf(ser_fp, "Enter the mac address in the format XXXXXXXXXXXX\r\n");
	readLine(mac, sizeof(mac));

	transformMacAddress(mac, ui_mac);
	fprintf(ser_fp, "New MAC address: %02x:%02x:%02x:%02x:%02x:%02x\r\n", ui_mac[0],
			ui_mac[1], ui_mac[2], ui_mac[3], ui_mac[4], ui_mac[5]);

	int stat;
	void *err_addr;

#ifdef CYGHWR_IO_FLASH_BLOCK_LOCKING
	if ((stat = flash_unlock((void *) macAddr, macAddrLen,
			(void **) &err_addr)) != 0)
	{
		fprintf(ser_fp, "Error: %s\r\n", "Unlocking flash failed");
		reset();
	}
#endif

	if ((stat = flash_erase((void *) (macAddr), macAddrLen, (void **) &err_addr)) != 0)
	{
		fprintf(ser_fp, "Error: %s\r\n", "Erasing flash failed");
		reset();
	}
	printf("erasing done\n");

	if ((stat = FLASH_PROGRAM(macAddr, ui_mac, macAddrLen, (void **)&err_addr))
			!= 0)
	{
		fprintf(ser_fp, "Error: %s\r\n", "Programming flash failed");
		reset();
	}

}

static void changeIP()
{
	char ip[81];
	//get IP
	fprintf(ser_fp,
			"\r\nEnter ip, mask and gateway(optional) (x.x.x.x,y.y.y.y[,z.z.z.z]):\r\n");
	readLine(ip, sizeof(ip));
	writeFile(IP_FILE, ip);
	reset();
}


/* load app into ram and run it. The application will need
 * a memory independent piece of code to begin with that
 * can be used to launch itself.
 */
static void runfile(const char *name)
{
	struct stat results;
	if (stat(name, &results) != 0)
	{
		fprintf(ser_fp, "Error: could not get length of file");
		reset();
	}

	int runfile_fd;
	if ((runfile_fd = open(name, O_RDONLY)) <= 0)
	{
		fprintf(ser_fp, "Error: failed to open file");
		reset();
	}
	void * mem = malloc(results.st_size);

	read(runfile_fd, mem, results.st_size);

	cyg_interrupt_disable();
	((void(*)(void)) (mem))();
	for (;;)
	{

	}
	/* never reached */
}


static void ymodemUpload(const char *fileName)
{
	int err = 0;

	fprintf(ser_fp, "Start Ymodem upload of %s\r\n", fileName);

	connection_info_t connection;
	memset(&connection, 0, sizeof(connection));
	connection.mode = xyzModem_ymodem;

	fprintf(ser_fp, "Connection over %s\r\n", fileName);

	if (xyzModem_stream_open(&connection, &err) != 0)
	{
		fprintf(ser_fp, "Could not open Ymodem connection %d\r\n", err);
		reset();
	}

	firmwareFile = creat(fileName, O_TRUNC | O_CREAT);
	if (firmwareFile < 0)
	{
		// close yModem connection so we can see error message in HyperTerminal
		int moreError;
		xyzModem_stream_close(&moreError);
		fprintf(ser_fp, "Error: could not create firmware file\r\n");
		reset();
	}

	bool ok = false;
	bool abortedByWrite = false;

	/* make sure we don't write too small blocks as this will
	 * increase memory usage catastrophically when reading the file
	 */
	int actual;
	int pos = 0;
	for (;;)
	{
		err = 0;
		actual = xyzModem_stream_read(WRITE_BUF + pos, sizeof(WRITE_BUF)
				- pos, &err);
		if (actual < 0)
		{
			break;
		}
		pos += actual;

		/* avoid lots of tiny writes, by flushing 0x100 bytes from
		 * end of buffer */
		if (((actual == 0) && (pos > 0)) || (pos > (WRITE_BUF_SIZE - 0x100)))
		{
			int written = write(firmwareFile, WRITE_BUF, pos);
			if (written < pos)
			{
				fprintf(ser_fp, "Writing %s failed\r\n", fileName);
				abortedByWrite = true;
				break;
			}
			pos = 0;
		}

		if (actual == 0)
		{
			break;
		}
	}

	int moreError;
	xyzModem_stream_close(&moreError);

	if ((!abortedByWrite) && (actual == 0) && (err == 0))
	{
		fprintf(ser_fp, "\r\nYmodem transfer complete\r\n");
		ok = true;
	}

	close(firmwareFile);

	if (!ok)
	{
		remove(fileName);
		if (!abortedByWrite)
		{
			fprintf(ser_fp, "\r\nFirmware upload failed: %s\r\n", xyzModem_error(err));
			reset();
		}
	}
}

void printAvailableRAM()
{
	struct mallinfo info;
	info = mallinfo();
	fprintf(ser_fp, "Available RAM: %d\r\n", info.fordblks);
}

static void set115200(void)
{
	cyg_serial_baud_rate_t baud;
	baud = CYGNUM_SERIAL_BAUD_115200;

	cyg_serial_info_t buf;
	cyg_uint32 len;
	//get existing serial configuration
	len = sizeof(cyg_serial_info_t);
	int err;
	cyg_io_handle_t serial_handle;

	err = cyg_io_lookup(UART_0_NAME, &serial_handle);
	if (err != ENOERR)
	{
		fprintf(ser_fp, "Error: could not get port handle\r\n");
		return;
	}

	err = cyg_io_get_config(serial_handle,
			CYG_IO_GET_CONFIG_SERIAL_OUTPUT_DRAIN, &buf, &len);
	err = cyg_io_get_config(serial_handle, CYG_IO_GET_CONFIG_SERIAL_INFO, &buf,
			&len);
	if (err != ENOERR)
	{
		fprintf(ser_fp, "Error: could not get port settings\r\n");
		return;
	}
	buf.baud = baud;

	err = cyg_io_set_config(serial_handle, CYG_IO_SET_CONFIG_SERIAL_INFO, &buf,
			&len);
	if (err != ENOERR)
	{
		fprintf(ser_fp, "Error: could not set baud rate\r\n");
		return;
	}
}


void menu(void)
{
	char fileName[NAME_MAX];

	fprintf(ser_fp, "Bootloader.\r\nCopyright FSF 2006-2010 All rights reserved\r\n");
	fprintf(ser_fp, "eCos license (GPL with exception)\r\n");
	fprintf(ser_fp, "Build date %s %s\r\n", __DATE__, __TIME__);

	mountJFFS2();
	mountRamFS();

	printMACAddress();

	start_menu:

	fprintf(ser_fp, "Press <space> for advanced help\r\n");
	if (hasMacAddress())
	{
		fprintf(ser_fp, "Press <i> to set static IP address\r\n");
		fprintf(ser_fp, "Press <enter> to start Ymodem upload of firmware\r\n");
	}
	else
	{
		fprintf(ser_fp, "Press <m> to set MAC address\r\n");
	}

	//use default firmware file name
	strcpy(fileName, firmware.file);
	char key;
	/* 5 second wait is a wee bit long here for normal execution,
	 * but it makes the bootloader a lot easier to use and debug
	 */
	waitMoreChar: if (waitChar(5, &key))
	{
		switch (key)
		{
		case 'F':
			format();
			/* never reached */
			break;
		case 'i':
		case 'I':
			changeIP();
			/* never reached */
			break;
		case 'P':
			enterParameter();
			goto start_menu;
		case 'D':
			showParameter();
			goto start_menu;
		case 'B':
			set115200();
			goto start_menu;
		case 'X':
			fprintf(ser_fp, "Erasing MAC address...");
			flash_erase_range(macAddr, macAddrLen);
			fprintf(ser_fp, "done\r\n");
			goto start_menu;
		case 'E':
			fprintf(ser_fp, "File name: ");
			getFileName(fileName, sizeof(fileName));
			ymodemUpload(fileName);
			goto start_menu;
		case '\r':
			fprintf(ser_fp, "Default firmware file update\r\n");
			ymodemUpload(firmware.file);
			cyg_firmware_upgrade(NULL, firmware);
			reset();
			break;
		case 'Y':
			fprintf(ser_fp, "Single shot bootloader update\r\n");
			ymodemUpload(bootloader.file);
			cyg_firmware_upgrade(NULL, bootloader);
			reset();
			break;
		case 'R':
			fprintf(ser_fp, "Upload and run file from RAM\r\n");
			ymodemUpload("/ram/run");
			runfile("/ram/run");
			break;
		case ' ':
			fprintf(ser_fp, "\r\nAdvanced menu:\r\n\r\n");
			fprintf(ser_fp, "Press <F> format flash\r\n");
			fprintf(ser_fp,
					"Press <E> to start Ymodem upload of a file to a specified file name\r\n");
			fprintf(ser_fp, "Press <R> run file from RAM\r\n");
			fprintf(ser_fp, "Press <Y> to start single shot update of bootloader\r\n");
			fprintf(ser_fp, "Press <P> set parameter\r\n");
			fprintf(ser_fp, "Press <D> show parameter\r\n");
			fprintf(ser_fp, "Press <B> set 115200 serial speed\r\n");
			fprintf(ser_fp, "Press <X> erase MAC address\r\n");
			goto waitMoreChar;

		default:
			/* ignore unknown keys... */
			break;
		}

		/* FIX!!!! we should only be able to change mac address if we don't
		 * have one already.
		 */
		//if(!hasMacAddress())
		{
			if ((key == 'M') || (key == 'm'))
			{
				changeMac();
			}
		}
	}

	if (!hasMacAddress())
	{
		/* do not allow running application without mac address */
		fprintf(ser_fp, "Error: MAC address not set\r\n");
		reset();
	}

	umount("/config");
	umount("/ram");
}

/********************************************************************************
 * Function: CycloneIII_Reconfig
 * Purpose: Uses the ALT_REMOTE_UPDATE megafunction to reconfigure a Cyclone III FPGA.
 * Parameters:
 * remote_update_base - base address of the remote update controller
 * flash_base - base address of flash device
 * reconfig_offset - offset in flash from which to reconfigure
 * watchdog_timeout - 29-bit watchdog timeout value
 * width_of_flash - data-width of flash device
 * Returns: 0 ( but never exits since it reconfigures the FPGA )
 ****************************************************************************/
void CycloneIIIReconfig(int remote_update_base, int flash_base,
		int reconfig_offset, int watchdog_timeout, int width_of_flash)
{
	int offset_shift, addr;
	int tmp;
	// Obtain upper 12 bits of 29-bit watchdog timeout value
	watchdog_timeout = watchdog_timeout >> 17;
	// Only enable the watchdog timer if its timeout value is greater than 0.
	if (watchdog_timeout > 0)
	{
		// Set the watchdog timeout value
		IOWR( remote_update_base, 0x2, watchdog_timeout );
	}
	else
	{
		// Disable the watchdog timer
		IOWR( remote_update_base, 0x3, 0 );
	}

	tmp = IORD( remote_update_base, 0x0 );

	tmp = IORD( remote_update_base, 0xF );

	// Calculate how much to shift the reconfig offset location:
	// width_of_flash == 8->offset_shift = 2.
	// width_of_flash == 16->offset_shift = 3
	offset_shift = ((width_of_flash / 8) + 1);
	// Write the offset of the desired reconfiguration image in flash
	IOWR( remote_update_base, 0x4, reconfig_offset >> offset_shift );

	addr = IORD( remote_update_base, 0x4);
	// Perform the reconfiguration by setting bit 0 in the
	// control/status register
	IOWR( remote_update_base, 0x20, 0x1 );
}

int needReset()
{
	int tmp = IORD(REMOTE_UPDATE_BASE, 0x0);
	return !tmp;
}

int main()
{
	openSerial();

#if DEBUG_NO_RESET()
	menu();
	/* DEBUG: should have reconfigured here, disabled to make debugging easier.
	 * We no longer have serial output capability */
	for (;;);
#else
	if (needReset())
	{
		menu();

		fprintf(ser_fp, "Start application image\r\n");
		cleaning();
		CycloneIIIReconfig(REMOTE_UPDATE_BASE, UNCACHED_EXT_FLASH_BASE,
				APPLICATION_FPGA_OFFSET, 0, 16);
	}
#endif
	fprintf(ser_fp, "Jump to application\r\n");
	cleaning();
	cyg_interrupt_disable();
	((void(*)(void)) (UNCACHED_EXT_FLASH_BASE + APPLICATION_OFFSET))();
	for (;;)
		; // never reached
}

