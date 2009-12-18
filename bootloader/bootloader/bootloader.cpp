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

//#include <phi_network_support.h>

static char FIRMWARE_FILE[] = "/ram/firmware.phi";
static char BOOTLOADER_FILE[] = "/ram/bootloader.phi";
static char FPGA_FILE[] = "/ram/fpga.phi";
static char IP_FILE[] = "/config/ip";
static char MAC_FILE[] = "/config/mac";
static const int WRITE_BUF_SIZE = 4096;
static char WRITE_BUF[WRITE_BUF_SIZE];

int ser = -1;
FILE *ser_fp;

static void writeMac(cyg_uint8 mac[6]);
static void writeFile(const char *fileName, const char *string);
static bool hasMacAddress();
static void printMACAddress();

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
	IOWR(REMOTE_UPDATE_BASE, 0x20, 0x1);
}

void openSerial()
{
	ser = open(UART_0_NAME, O_RDWR|O_SYNC|O_NONBLOCK);
	if (ser < 0)
	{
		diag_printf("Serial device problems %s\r\n", UART_0_NAME);
		reset();
	}

	ser_fp = fdopen(ser, "r+");

	if (ser_fp == NULL)
	{
		diag_printf("Serial device problems %s\r\n", UART_0_NAME);
		reset();
	}
}

void format(void)
{
	fprintf(ser_fp, "Formatting JFFS2...\r\n");
	int stat;
	void *err_addr;

	if ((stat = flash_init(0)) != 0)
	{
		fprintf(ser_fp, "Flash Error flash_init: \r\n");
	}

#ifdef CYGHWR_IO_FLASH_BLOCK_LOCKING
	if ((stat = flash_unlock((void *) EXT_FLASH_BASE, EXT_FLASH_SPAN,
			(void **) &err_addr)) != 0)
	{
		fprintf(ser_fp, "Flash Error flash_unlock: %d\r\n", stat);
	}
#endif

	fprintf(ser_fp, "Formatting 0x%08x bytes\r\n", JFFS2_LENGTH - JFFS2_OFFSET);
	if ((stat = flash_erase((void *) JFFS2_OFFSET, JFFS2_LENGTH - JFFS2_OFFSET,
			(void **) &err_addr)) != 0)
	{
		fprintf(ser_fp, "Flash Error flash_erase: %d\r\n", stat);
	}

	fprintf(ser_fp, "Flash formatted successfully\r\n");
	reset();
}

int firmwareFile, fpgaFile;

static bool expect(const char *string, bool resetOnFailure = true)
{
	//	fprintf(ser_fp, "Expecting \"%s\"\r\n", string);
	for (size_t i = 0; i < strlen(string); i++)
	{
		char t;
		if (read(firmwareFile, &t, 1) != 1)
		{
			fprintf(ser_fp, "Error: reading firmware file %d, expecting \"%s\"\r\n",
					errno, string);
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
		if (t != string[i])
		{
			if (resetOnFailure)
			{
				fprintf(ser_fp,
						"Unexpected data in firmware file while expecting \"%s\"\r\n",
						string);
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

/* read integer which is terminated by whitespace or eof */
int readInt(void)
{
	char buf[32];
	size_t i;
	i = 0;
	for (;;)
	{
		if (i >= sizeof(buf))
		{
			fprintf(ser_fp, "Error: reading string. Too long %d\r\n", (int) i);
			reset();
		}

		char t;
		int actual;
		actual = read(firmwareFile, &t, 1);
		if (actual < 0)
		{
			fprintf(ser_fp, "Error: reading integer %d\r\n", errno);
			reset();
		}
		if (actual == 1)
		{
			if (!isspace((int) t))
			{
				buf[i++] = t;
			}
			else
			{
				break;
			}
		}
		else if (actual == 0)
		{
			break;
		}
		else
		{
			fprintf(ser_fp, "Error: reading integer\r\n");
			reset();
		}
	}
	buf[i] = 0;
	return atoi(buf);
}

void appendPadding(char* buffer, int start, int length)
{
	int i;
	for (i = 0; i < length; i++)
	{
		buffer[start + i] = 0xFF;
	}
}

static void upgradeBootloader()
{
	/* Do we have a pending bootloader update? */
	if ((firmwareFile = open(BOOTLOADER_FILE, O_RDONLY)) > 0)
	{
		fprintf(ser_fp, "Single shot bootloader update in progress\r\n");

		/*    	if (!expect("ZylinPhiBootloader\r\n", false))
		 {
		 close(firmwareFile);
		 fprintf(ser_fp, "Corrupt bootloader image uploaded. Safely aborting bootloader update.\r\n");
		 remove(BOOTLOADER_FILE);
		 reset();
		 }*/

		cyg_uint8 *bootloaderAddr = (cyg_uint8 *) (EXT_FLASH_BASE
				+ FACTORY_FPGA_OFFSET);
		struct stat results;

		if (stat(BOOTLOADER_FILE, &results) == 0)
		{
			fprintf(ser_fp, "size %ld\r\n", results.st_size);
		}

		printMACAddress();
		bool hasMac = hasMacAddress();

		int stat;
		void *err_addr;
		fprintf(ser_fp, "Erasing flash...");
		if ((stat = flash_erase((void *) (bootloaderAddr), APPLICATION_FPGA_OFFSET
				- FACTORY_FPGA_OFFSET, (void **) &err_addr)) != 0)
		{
			fprintf(ser_fp, "Error: erasing bootloader %p: %d\r\n", err_addr, stat);
			reset();
		}
		fprintf(ser_fp, "done\r\n");

		char buf[1024];
		int actual;
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
					= FLASH_PROGRAM(bootloaderAddr, buf, actual + rem, (void **)&err_addr))
					!= 0)
			{
				fprintf(ser_fp, "Error: writing bootloader data at %p: %d\r\n", err_addr,
						stat);
				reset();
			}

			bootloaderAddr += actual;
		}

		close(firmwareFile);

		if (actual < 0)
		{
			fprintf(ser_fp, "Error: catastrophic failure. Bootloader corrupt %d.\r\n",
					errno);
			remove(BOOTLOADER_FILE);
			reset();
		}

		if (hasMac)
		{
			// FIX!!! This won't work. If we ever type the wrong mac number, we're screwed.
			//			fprintf(ser_fp, "Updating mac address");
			//			writeMac(mac);
		}

		fprintf(ser_fp, "Bootloader successfully updated.\r\n");
		remove(BOOTLOADER_FILE);
		reset();
	}
}

static void upgradeFirmware()
{
	/* Do we have a pending firmware update? */
	if ((firmwareFile = open(FIRMWARE_FILE, O_RDONLY)) > 0)
	{
		fprintf(ser_fp, "Firmware update in progress\r\n");

		/*    	if (!expect("ZylinPhiBootloader\r\n", false))
		 {
		 close(firmwareFile);
		 fprintf(ser_fp, "Corrupt bootloader image uploaded. Safely aborting bootloader update.\r\n");
		 remove(BOOTLOADER_FILE);
		 reset();
		 }*/

		cyg_uint8 *firmwareAddr = (cyg_uint8 *) (EXT_FLASH_BASE
				+ APPLICATION_FPGA_OFFSET);
		struct stat results;

		if (stat(FIRMWARE_FILE, &results) == 0)
		{
			fprintf(ser_fp, "size %ld\r\n", results.st_size);
		}

		printMACAddress();
		bool hasMac = hasMacAddress();

		int stat;
		void *err_addr;
		fprintf(ser_fp, "Erasing flash...");
		if ((stat = flash_erase((void *) (firmwareAddr), BOOTLOADER_OFFSET
				- APPLICATION_FPGA_OFFSET, (void **) &err_addr)) != 0)
		{
			fprintf(ser_fp, "Error: erasing firmware %p: %d\r\n", err_addr, stat);
			reset();
		}
		fprintf(ser_fp, "done\r\n");

		char buf[1024];
		int actual;
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
					= FLASH_PROGRAM(firmwareAddr, buf, actual + rem, (void **)&err_addr))
					!= 0)
			{
				fprintf(ser_fp, "Error: writing bootloader data at %p: %d\r\n", err_addr,
						stat);
				reset();
			}

			firmwareAddr += actual;
		}

		close(firmwareFile);

		if (actual < 0)
		{
			fprintf(ser_fp, "Error: catastrophic failure. Bootloader corrupt %d.\r\n",
					errno);
			remove(FIRMWARE_FILE);
			reset();
		}

		if (hasMac)
		{
			// FIX!!! This won't work. If we ever type the wrong mac number, we're screwed.
			//			fprintf(ser_fp, "Updating mac address");
			//			writeMac(mac);
		}

		fprintf(ser_fp, "Firmware successfully updated.\r\n");
		remove(FIRMWARE_FILE);
		reset();
	}
}

static void upgradeFPGA(int start, int stop)
{
	/* Do we have a pending FPGA update? */
	if ((fpgaFile = open(FPGA_FILE, O_RDONLY)) > 0)
	{
		fprintf(ser_fp, "FPGA update in progress\r\n");

		/*    	if (!expect("ZylinPhiBootloader\r\n", false))
		 {
		 close(firmwareFile);
		 fprintf(ser_fp, "Corrupt bootloader image uploaded. Safely aborting bootloader update.\r\n");
		 remove(BOOTLOADER_FILE);
		 reset();
		 }*/

		cyg_uint8 *fpgaAddr = (cyg_uint8 *) (EXT_FLASH_BASE + start);
		struct stat results;

		if (stat(FPGA_FILE, &results) == 0)
		{
			fprintf(ser_fp, "size %ld\r\n", results.st_size);
		}

		printMACAddress();
		bool hasMac = hasMacAddress();

		int stat;
		void *err_addr;
		fprintf(ser_fp, "Erasing flash...from %0x size %0x", fpgaAddr, stop - start);
		if ((stat = flash_erase((void *) (fpgaAddr), stop - start,
				(void **) &err_addr)) != 0)
		{
			fprintf(ser_fp, "Error: erasing fpga %p: %d\r\n", err_addr, stat);
			reset();
		}
		fprintf(ser_fp, "done\r\n");

		char buf[1024];
		int actual;
		while ((actual = read(fpgaFile, buf, sizeof(buf))) > 0)
		{
			int stat;
			void *err_addr;

			int rem = actual % LENGTH;
			if (rem != 0)
			{
				rem = LENGTH - rem;
				appendPadding(buf, actual, rem);
			}

			fprintf(ser_fp, "%d\r\n", fpgaAddr);
			if ((stat
					= FLASH_PROGRAM(fpgaAddr, buf, actual + rem, (void **)&err_addr))
					!= 0)
			{
				fprintf(ser_fp, "Error: writing fpga data at %p: %d\r\n", err_addr, stat);
				reset();
			}

			fpgaAddr += actual;
		}

		close(fpgaFile);

		if (actual < 0)
		{
			fprintf(ser_fp, "Error: catastrophic failure. fpga corrupt %d.\r\n", errno);
			remove(FPGA_FILE);
			reset();
		}

		if (hasMac)
		{
			// FIX!!! This won't work. If we ever type the wrong mac number, we're screwed.
			//			fprintf(ser_fp, "Updating mac address");
			//			writeMac(mac);
		}

		fprintf(ser_fp, "FPGA successfully updated. %d\r\n", actual);
		remove(FPGA_FILE);
		reset();
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
			fprintf(ser_fp, "\r\nCtrl-c pressed\r\n");
			reset();
		case '\n':
		case '\r':
			if (index == 0)
			{
				fprintf(ser_fp, "\r\nEmpty string not allowed\r\n");
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
				fprintf(ser_fp, "\r\nString too long\r\n");
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

static void wrongMAC()
{
	fprintf(ser_fp, "Wrong MAC address syntax\r\n");
	reset();
}

static int getMacAddress(char* buffer)
{
	buffer[0] = 0;
	buffer[12] = 0;

	int fd = open(MAC_FILE, O_RDONLY);
	if (fd < 0)
	{
		fprintf(ser_fp, "Could not open %s\r\n", MAC_FILE);
		return fd;
	}

	for (int i = 0; i < 12; i++)
	{
		char c;
		int actual;
		actual = read(fd, &c, 1);
		if (actual < 0)
		{
			fprintf(ser_fp, "\r\nFailed while reading %s\r\n", MAC_FILE);
			reset();
		}
		if (actual != 1)
			break;
		buffer[i] = c;
	}
	close(fd);
	fprintf(ser_fp, "\r\n");
	return fd;
}


static cyg_uint8 * transformMacAddress(char* buffer, cyg_uint8 mac_addr[6])
{
	mac_addr[0] = 0;
	mac_addr[1] = 0;
	mac_addr[2] = 0;
	mac_addr[3] = 0;
	mac_addr[4] = 0;
	mac_addr[5] = 0;

	if (strlen(buffer) != 12)
	{
		wrongMAC();
	}

	for (size_t i = 0; i < strlen(buffer); i++)
	{
		char c = buffer[i];
		if (('0' <= c && c <= '9') || ('a' <= c && c <= 'f') || ('A' <= c && c
				<= 'F'))
		{
			cyg_uint32 val;
			sscanf(&c, "%x", &val);
			mac_addr[i / 2] += (val << (4 * ((i + 1) % 2)));

		}
		else
		{
			wrongMAC();
		}
	}
	return mac_addr;

}

static bool hasMacAddress()
{

	cyg_uint8* mac = (cyg_uint8 *) (EXT_FLASH_BASE + FACTORY_FPGA_OFFSET - 6);
	int i;
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
		cyg_uint8* mac = (cyg_uint8 *) (EXT_FLASH_BASE + FACTORY_FPGA_OFFSET - 6);
		fprintf(ser_fp, "Mac address 2 %02x:%02x:%02x:%02x:%02x:%02x\r\n", mac[0],
				mac[1], mac[2], mac[3], mac[4], mac[5]);
	}
	else
		fprintf(ser_fp, "Mac address not set\r\n");
}

static void changeMac()
{
	char mac[13];
	cyg_uint8 ui_mac[6];

	fprintf(ser_fp, "Enter the mac address in the format XXXXXXXXXXXX  \r\n");
	readLine(mac, sizeof(mac));

	transformMacAddress(mac, ui_mac);
	fprintf(ser_fp, "Mac address %02x:%02x:%02x:%02x:%02x:%02x\r\n", ui_mac[0],
			ui_mac[1], ui_mac[2], ui_mac[3], ui_mac[4], ui_mac[5]);

	int stat;
	void *err_addr;

	cyg_uint8 *macAddr = (cyg_uint8 *) (EXT_FLASH_BASE + FACTORY_FPGA_OFFSET - 6);

#ifdef CYGHWR_IO_FLASH_BLOCK_LOCKING
	if ((stat = flash_unlock((void *) macAddr, 6,
			(void **) &err_addr)) != 0)
	{
		fprintf(ser_fp, "Flash Error flash_unlock: %d\r\n", stat);
	}
#endif

	if ((stat = flash_erase((void *) (macAddr), 6, (void **) &err_addr)) != 0)
	{
		printf("Error: erasing bootloader %p: %d\n", err_addr, stat);
		reset();
	}
	printf("erasing done\n");

	if ((stat = FLASH_PROGRAM(macAddr, ui_mac, 6, (void **)&err_addr))
			!= 0)
	{
		printf("Error: writing bootloader data at %p: %d\n", err_addr, stat);
		reset();
	}

}

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
			fprintf(ser_fp, "\r\nFailed while reading %s\r\n", name);
			reset();
		}
		if (actual != 1)
			break;
		fprintf(ser_fp, "%c", c);
	}
	fprintf(ser_fp, "\r\n");

}

static void changeIP()
{
	char ip[81];
	//get IP
	fprintf(ser_fp,
			"\r\nEnter ip, mask and gateway(optional) (x.x.x.x,y.y.y.y[,z.z.z.z]): ");
	readLine(ip, sizeof(ip));
	writeFile(IP_FILE, ip);
}

static void ymodemUpload(const char *fileName)
{
	int err = 0;

	fprintf(ser_fp, "Start Ymodem upload of %s\r\n", fileName);

	connection_info_t connection;
	memset(&connection, 0, sizeof(connection));
	connection.mode = xyzModem_ymodem;

	fprintf(ser_fp, "connection over %s\r\n", fileName);

	if (xyzModem_stream_open(&connection, &err) == 0)
	{
		firmwareFile = creat(fileName, O_TRUNC | O_CREAT);
		if (firmwareFile < 0)
		{
			int t = errno;

			// close yModem connection so we can see error message in HyperTerminal
			int moreError;
			xyzModem_stream_close(&moreError);

			fprintf(ser_fp, "Could not create firmware file %d\r\n", t);
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
		int ii = 0;
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
					fprintf(ser_fp, "Writing %s failed %d\r\n", fileName, errno);
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
				fprintf(ser_fp, "ERROR, not enough memory. Resetting...");
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
		fprintf(ser_fp, "No such app\r\n");
		reset();
	}
	fprintf(ser_fp, "\r\n");

	strcpy(fileName, "/config/");
	strcat(fileName, names[key - '0']);
}

void mountJFFS2()
{
	Cyg_ErrNo err = 0;
	err = cyg_flash_init(NULL);
	if (err)
	{
		fprintf(ser_fp, "cyg_flash_init error %d\r\n ", err);
	}

	cyg_flashaddr_t err_address;

#ifdef CYGHWR_IO_FLASH_BLOCK_LOCKING
	if ((err = flash_unlock((void *) EXT_FLASH_BASE, EXT_FLASH_SPAN,
			(void **) &err_address)) != 0)
	{
		fprintf(ser_fp, "Flash Error flash_unlock: %d\r\n", err);
		reset();
	}
#endif

	//	std::ostringstream s;
	//	s << "/dev/flash/0/" << JFFS2_OFFSET << "," << JFFS2_LENGTH;

	std::string fis = "/dev/flash/0/";
	char number[9];
	number[8] = 0;
	sprintf(number, "0X%0x", JFFS2_OFFSET);
	fis += number;
	fis += ",";
	sprintf(number, "0X%0x", JFFS2_LENGTH);
	fis += number;
	fprintf(ser_fp, "%s\r\n", fis.c_str());

	err = mount(fis.c_str(), "/config", "jffs2");

	if (err < 0)
	{
		fprintf(ser_fp, "JFFS2 mounting error %d\r\n", err);
		format();
	}
	fprintf(ser_fp, "mounted jffs2 just fine\r\n");
	fprintf(ser_fp, "testing file write/read/delete\r\n");
	FILE* pf = fopen("/config/tralala.txt", "w+");
	if (pf == NULL)
	{
		fprintf(ser_fp, "unable to create/open file\r\n");
		format();
	}
	else
	{
		fprintf(ser_fp, "File opened OK\r\n");
		int e = fprintf(pf, "tralala\n");
		if (e < 0)
		{
			fprintf(ser_fp, "Unable to fprintf\r\n");
			format();
		}
		else
		{
			fprintf(ser_fp, "Printfed OK\r\n");
		}
		fclose(pf);
	}
}

void mountRamFS()
{
	Cyg_ErrNo err = 0;

	err = mount("", "/ram", "ramfs");

	if (err < 0)
	{
		fprintf(ser_fp, "RAMFS mounting error %d\r\n", err);
		format();
	}
	fprintf(ser_fp, "mounted ramfs just fine\r\n");
	fprintf(ser_fp, "testing file write/read/delete\r\n");
	FILE* pf = fopen("/ram/tralala.txt", "w+");
	if (pf == NULL)
	{
		fprintf(ser_fp, "unable to create/open file\r\n");
		reset();
	}
	fclose(pf);
}

void printAvailableRAM()
{
	struct mallinfo info;
	info = mallinfo();
	fprintf(ser_fp, "Available RAM: %d\r\n", info.fordblks);
}

int menu()
{
	char fileName[NAME_MAX];

	fprintf(ser_fp, "Bootloader. Copyright 2006-2010 All rights reserved\r\n");
	fprintf(ser_fp, "Version unknown %s %s\r\n", __DATE__, __TIME__);

	//	cyg_exception_handler_t *old;
	//	cyg_addrword_t oldData;
	//	cyg_exception_set_handler(CYGNUM_HAL_VECTOR_UNDEF_INSTRUCTION, exception_handler, 0, &old, &oldData);
	//	cyg_exception_set_handler(CYGNUM_HAL_VECTOR_ABORT_PREFETCH, exception_handler, 0, &old, &oldData);
	//	cyg_exception_set_handler( CYGNUM_HAL_VECTOR_ABORT_DATA, exception_handler, 0, &old, &oldData);

	mountJFFS2();
	mountRamFS();

	printMACAddress();

	printAvailableRAM();

	//extern void phi_init_all_network_interfaces(void);
	// CYGPKG_PHI_NET
	// phi_init_all_network_interfaces();


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
	strcpy(fileName, FIRMWARE_FILE);
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
			reset();
			break;
		case 'i':
		case 'I':
			changeIP();
			reset();
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
			reset();
		case '\r':
			fprintf(ser_fp, "Default firmware file update\r\n");
			ymodemUpload(FIRMWARE_FILE); //fall through
			break;
		case 'Y':
			fprintf(ser_fp, "Single shot bootloader update\r\n");
			ymodemUpload(BOOTLOADER_FILE); //fall through
			break;
		case 'A':
			fprintf(ser_fp, "FPGA image update\r\n");
			ymodemUpload(FPGA_FILE); //fall through
			break;
		case ' ':
			fprintf(ser_fp, "Press <F> format flash\r\n");
			fprintf(ser_fp,
					"Press <E> to start Ymodem upload of firmware to a specified file name\r\n");
			fprintf(ser_fp, "Press <Y> to start single shot update of bootloader\r\n");
			fprintf(ser_fp, "Press <A> to start update of FPGA image\r\n");
			fprintf(ser_fp, "Press <P> set parameter\r\n");
			fprintf(ser_fp, "Press <D> show parameter\r\n");
			goto waitMoreChar;

		default:
			/* ignore unknown keys... */
			break;
		}
		if(!hasMacAddress())
		{
			if ((key == 'M') || (key == 'm'))
			{
				changeMac();
			}
		}
	}

	upgradeFPGA(FACTORY_FPGA_OFFSET, APPLICATION_FPGA_OFFSET);
	upgradeBootloader();
	upgradeFirmware();

	if (!hasMacAddress())
	{
		/* do not allow running application without mac address */
		//reset();
	}

	umount("/config");
	umount("/ram");

	return 0;
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
int CycloneIIIReconfig(int remote_update_base, int flash_base,
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
	return (0);
}

int needReset()
{
	int tmp = IORD(REMOTE_UPDATE_BASE, 0x0);
	return !tmp;
}

int main()
{
	openSerial();

	fprintf(ser_fp, "Main\r\n");

	if (needReset())
	{
		fprintf(ser_fp, "Reconfigure and reboot\r\n");
		diag_printf("Reconfigure and reboot %d\r\n", APPLICATION_FPGA_OFFSET);
		cleaning();
		CycloneIIIReconfig(REMOTE_UPDATE_BASE, EXT_FLASH_BASE,
				APPLICATION_FPGA_OFFSET, 0, 16);
	}
	menu();
	// launch application!
	fprintf(ser_fp, "Jump to deflate application\r\n");
	cleaning();
	cyg_interrupt_disable();
	((void(*)(void)) (EXT_FLASH_BASE + DEFLATOR_OFFSET))();
	for (;;)
		; // never reached
}

