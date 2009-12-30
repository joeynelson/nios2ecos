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

#include "addresses.h"
#include "bootloader.h"

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

#define UNCACHED_EXT_FLASH_BASE (0x80000000 + EXT_FLASH_BASE)

static char IP_FILE[] = "/config/ip";
static const int WRITE_BUF_SIZE = 4096;
static char WRITE_BUF[WRITE_BUF_SIZE];

int ser = -1;
FILE *ser_fp;

#define LENGTH 8


void cleaning()
{
	fclose(ser_fp);
	close(ser);
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
	/* 1000ms is necessary to let everyting "calmn down" */
	cyg_thread_delay(100);
	IOWR(REMOTE_UPDATE_BASE, 0x20, 0x1);
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


	char buffer[256];
	sprintf(buffer, "dev/flash/0/0X%x,0X%x", JFFS2_OFFSET, JFFS2_LENGTH);
	fprintf(ser_fp, "%s\r\n", buffer);

	if ((err = mount(buffer, "/config", "jffs2")) < 0)
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
	return;
}

/* Format jffs2 area and reset */
void format(void)
{
	int stat;
	void *err_addr;

	fprintf(ser_fp, "Formatting JFFS2...\r\n");

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

	fprintf(ser_fp, "Formatting 0x%08x bytes\r\n", JFFS2_LENGTH);
	if ((stat = flash_erase((void *) (UNCACHED_EXT_FLASH_BASE + JFFS2_OFFSET), JFFS2_LENGTH,
			(void **) &err_addr)) != 0)
	{
		fprintf(ser_fp, "Error: %s\r\n", "Erasing flash failed");
		reset();
	}

	fprintf(ser_fp, "%s\r\n", "Flash formatted successfully");
	reset();
}

int firmwareFile, fpgaFile;

/* check that the firmware file contains the expected string. */
static bool expect(const char *strin, bool resetOnFailure = true)
{
	//	fprintf(ser_fp, "Expecting \"%s\"\r\n", string);
	for (size_t i = 0; i < strlen(strin); i++)
	{
		char t;
		if (read(firmwareFile, &t, 1) != 1)
		{
			fprintf(ser_fp, "Error: reading firmware file , expecting \"%s\"\r\n", strin);
			if (resetOnFailure)
			{
				reset();
			}
			else
			{
				lseek(firmwareFile, -1, SEEK_CUR);
				return false;
			}
		}
		if (t != strin[i])
		{
			if (resetOnFailure)
			{
				fprintf(ser_fp, "Unexpected data in firmware file while expecting %s\r\n", strin);
				reset();
			}
			else
			{
				lseek(firmwareFile, -1, SEEK_CUR);
				return false;
			}
		}
	}
	return true;
}

void appendPadding(char* buffer, int start, int length)
{
	int i;
	for (i = 0; i < length; i++)
	{
		buffer[start + i] = 0xFF;
	}
}

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

static bool hasMacAddress()
{
	cyg_uint8* mac = (cyg_uint8 *) (UNCACHED_EXT_FLASH_BASE + FACTORY_FPGA_OFFSET - 6);
	cyg_uint8 ret = 0xFF;
	for(int i = 0; i < 6; i++)
	{
		ret = ret & mac[i];
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

static void changeMac()
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

	cyg_uint8 *macAddr = (cyg_uint8 *) (UNCACHED_EXT_FLASH_BASE + FACTORY_FPGA_OFFSET - 6);

#ifdef CYGHWR_IO_FLASH_BLOCK_LOCKING
	if ((stat = flash_unlock((void *) macAddr, 6,
			(void **) &err_addr)) != 0)
	{
		fprintf(ser_fp, "Error: %s\r\n", "Unlocking flash failed");
		reset();
	}
#endif

	if ((stat = flash_erase((void *) (macAddr), 6, (void **) &err_addr)) != 0)
	{
		fprintf(ser_fp, "Error: %s\r\n", "Erasing flash failed");
		reset();
	}
	printf("erasing done\n");

	if ((stat = FLASH_PROGRAM(macAddr, ui_mac, 6, (void **)&err_addr))
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
			"\r\nEnter ip, mask and gateway(optional) (x.x.x.x,y.y.y.y[,z.z.z.z]): ");
	readLine(ip, sizeof(ip));
	writeFile(IP_FILE, ip);
	reset();
}

static void upgrade(upgrade_info upgraded_file)
{
	/* Do we have a pending bootloader update? */
	if ((firmwareFile = open(upgraded_file.file, O_RDONLY)) > 0)
	{
		fprintf(ser_fp, "%s update in progress\r\n", upgraded_file.name);

		/* FIX!!!! we need to check for header here!!! */
		/*    	if (!expect("ZylinPhiBootloader\r\n", false))
		 {
		 close(firmwareFile);
		 fprintf(ser_fp, "Corrupt bootloader image uploaded. Safely aborting bootloader update.\r\n");
		 remove(BOOTLOADER_FILE);
		 reset();
		 }*/

		cyg_uint8 *startAddr = (cyg_uint8 *) (UNCACHED_EXT_FLASH_BASE + upgraded_file.start_address);
		struct stat results;

		if (stat(upgraded_file.file, &results) == 0)
		{
			fprintf(ser_fp, "size %ld\r\n", results.st_size);
		}

		int stat;
		void *err_addr;

#ifdef CYGHWR_IO_FLASH_BLOCK_LOCKING
		if ((stat = flash_unlock((void *) startAddr, upgraded_file.length,
			(void **) &err_addr)) != 0)
			{
				fprintf(ser_fp, "Error: %s\r\n", "Unlocking flash failed");
				reset();
			}
#endif

		fprintf(ser_fp, "Erasing flash...");
		if ((stat = flash_erase((void *) (startAddr), upgraded_file.length, (void **) &err_addr)) != 0)
		{
			fprintf(ser_fp, "Error: %s\r\n", "Erasing flash failed");
			reset();
		}
		fprintf(ser_fp, "done.\r\n");

		char buf[1024];
		int actual;
		fprintf(ser_fp, "Programming flash...");
		while ((actual = read(firmwareFile, buf, sizeof(buf))) > 0)
		{
			int stat;
			void *err_addr;
			int rem = actual % LENGTH;
			if (rem != 0)
			{
				rem = LENGTH - rem;
				appendPadding(buf, actual, rem);
			}

			if ((stat
					= FLASH_PROGRAM(startAddr, buf, actual + rem, (void **)&err_addr))
					!= 0)
			{
				fprintf(ser_fp, "Error: %s\r\n", "Programming flash failed");
				reset();
			}

			startAddr += actual;
		}
		fprintf(ser_fp, "done.\r\n");

		close(firmwareFile);

		if (actual < 0)
		{
			remove(upgraded_file.file);
			fprintf(ser_fp, "Error: catastrophic failure. %s corrupt\r\n", upgraded_file.name);
			reset();
		}

		remove(upgraded_file.file);
		fprintf(ser_fp, "%s successfully updated.\r\n", upgraded_file.name);
		reset();
	}
}


static void ymodemUpload(const char *fileName)
{
	int err = 0;

	fprintf(ser_fp, "Start Ymodem upload of %s\r\n", fileName);

	connection_info_t connection;
	memset(&connection, 0, sizeof(connection));
	connection.mode = xyzModem_ymodem;

	fprintf(ser_fp, "Connection over %s\r\n", fileName);

	if (xyzModem_stream_open(&connection, &err) == 0)
	{
		firmwareFile = creat(fileName, O_TRUNC | O_CREAT);
		if (firmwareFile < 0)
		{
			// close yModem connection so we can see error message in HyperTerminal
			int moreError;
			xyzModem_stream_close(&moreError);
			fprintf(ser_fp, "Error: could not create firmware file\r\n");
			reset();
		}

		int err;
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

			if (((actual == 0) && (pos > 0)) || ((actual + pos)
					> (WRITE_BUF_SIZE - 0x100)))
			{
				int written = write(firmwareFile, WRITE_BUF, actual + pos);
				if (written < (actual + pos))
				{
					fprintf(ser_fp, "Writing %s failed\r\n", fileName);
					abortedByWrite = true;
					break;
				}
				pos = 0;
			}
			else
			{
				pos += actual;
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
			fprintf(ser_fp, "\r\nFirmware successfully uploaded\r\n");
			ok = true;
		}

		close(firmwareFile);

		if (!ok)
		{
			remove(fileName);
			if (!abortedByWrite)
			{
				fprintf(ser_fp, "\r\nFirmware upload failed: %s\r\n", xyzModem_error(err));
			}
		}
	}
}

static void selectFile(char *fileName)
{
	int index = 0;
	char *names[10]; //max 10 files
	DIR* fs = opendir("/config");
	char key = 0;
	for (index = 0; index < 10;)
	{
		struct dirent *entry = readdir(fs);
		int len = 0;
		if (entry == NULL)
			break;
		len = strlen(entry->d_name);
		/* geeez.... The code below just checks for.zpu and .phi extensions
		 * and although it looks ugly, it's tested, leave be until
		 * a regression test can be done after fixing it. */
		if (len > 4 && (entry->d_name[len - 4] == '.' && ((entry->d_name[len
				- 3] == 'p' && entry->d_name[len - 2] == 'h'
				&& entry->d_name[len - 1] == 'i') || (entry->d_name[len - 3]
				== 'z' && entry->d_name[len - 2] == 'p' && entry->d_name[len
				- 1] == 'u'))))
		{
			fprintf(ser_fp, "%d. %s\r\n", index, entry->d_name);
			names[index] = (char *) malloc(strlen(entry->d_name) + 1);
			if (names[index] == NULL)
			{
				fprintf(ser_fp, "Error: not enough memory\r\n");
				reset();
			}
			strcpy(names[index], entry->d_name);
			index++;
		}
	}
	fprintf(ser_fp, "Type in the index of the file you want to select (0 - %d)", index
			- 1);
	getChar(&key);
	if (!(key >= '0' && key <= '9' || key - '0' > index - 1))
	{
		fprintf(ser_fp, "Error: No such app\r\n");
		reset();
	}
	fprintf(ser_fp, "\r\n");

	strcpy(fileName, "/config/");
	strcat(fileName, names[key - '0']);
}

void printAvailableRAM()
{
	struct mallinfo info;
	info = mallinfo();
	fprintf(ser_fp, "Available RAM: %d\r\n", info.fordblks);
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
	waitMoreChar: if (waitChar(2, &key))
	{
		switch (key)
		{
		case 'S':
			selectFile(fileName);
			break;
		case 'F':
			format();
			break;
		case 'i':
		case 'I':
			changeIP();
		case 'P':
			enterParameter();
			goto start_menu;
		case 'D':
			showParameter();
			goto start_menu;
		case 'E':
			fprintf(ser_fp, "File name: ");
			getFileName(fileName, sizeof(fileName));
			ymodemUpload(fileName);
		case '\r':
			fprintf(ser_fp, "Default firmware file update\r\n");
			ymodemUpload(firmware.file); //fall through
			break;
		case 'Y':
			fprintf(ser_fp, "Single shot bootloader update\r\n");
			ymodemUpload(bootloader.file); //fall through
			break;
		case ' ':
			fprintf(ser_fp, "Press <F> format flash\r\n");
			fprintf(ser_fp,
					"Press <E> to start Ymodem upload of firmware to a specified file name\r\n");
			fprintf(ser_fp, "Press <Y> to start single shot update of bootloader\r\n");
			fprintf(ser_fp, "Press <P> set parameter\r\n");
			fprintf(ser_fp, "Press <D> show parameter\r\n");
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

	upgrade(bootloader);
	upgrade(firmware);

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

	if (needReset())
	{
		menu();

		fprintf(ser_fp, "Reconfigure and reboot\r\n");
		cleaning();
		CycloneIIIReconfig(REMOTE_UPDATE_BASE, UNCACHED_EXT_FLASH_BASE,
				APPLICATION_FPGA_OFFSET, 0, 16);
	}
	fprintf(ser_fp, "Jump to application\r\n");
	cleaning();
	cyg_interrupt_disable();
	((void(*)(void)) (UNCACHED_EXT_FLASH_BASE + DEFLATOR_OFFSET))();
	for (;;)
		; // never reached
}

