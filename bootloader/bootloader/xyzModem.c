//==========================================================================
//
//      xyzModem.c
//
//      RedBoot stream handler for xyzModem protocol
//
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
//#####DESCRIPTIONBEGIN####
//
// Author(s):    gthomas
// Contributors: gthomas, tsmith, Yoshinori Sato
// Date:         2000-07-14
// Purpose:      
// Description:  
//              
// This code is part of RedBoot (tm).
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include "bootloader.h"

void putChar(int handle, char t)
{
	write(handle, &t, 1);
}

static int readpos;
static int writepos;
static const int bufsize = 4096;
static cyg_uint8 buffer[bufsize];

/* classic circular buffer to reduce overhead */
static int getCharTimeout(int handle, cyg_uint8 *t)
{
	if (readpos==writepos)
	{
		fd_set rfds;
	   struct timeval tv;
	   int retval;

	   /* Watch stdin (fd 0) to see when it has input. */
	   FD_ZERO(&rfds);
	   FD_SET(handle, &rfds);
	   /* Wait this 2 seconds. */
	   tv.tv_sec = 2;
	   tv.tv_usec = 0;

	   retval = select(1, &rfds, NULL, NULL, &tv);
	   /* Don't rely on the value of tv now! */
	   if (retval > 0)
	   {
		if (FD_ISSET(handle, &rfds))
		{
			int remainder;
			if (writepos < readpos)
			{
				remainder = readpos - writepos;
			} else
			{
				remainder = bufsize - writepos;
			}
			int actual = read(handle, buffer+writepos, remainder);

			if (actual <= 0)
			{
				return 0;
			}

			writepos = (writepos + actual) % bufsize;

		} else
		{
			return 0;
		}
	   } else
	   {
		   return 0;
	   }
	}

	*t=buffer[readpos];
	readpos=(readpos+1)%bufsize;
	return 1;
}


// Assumption - run xyzModem protocol over the console port

// Values magic to the protocol
#define SOH 0x01
#define STX 0x02
#define EOT 0x04
#define ACK 0x06
#define BSP 0x08
#define NAK 0x15
#define CAN 0x18
#undef EOF
#define EOF 0x1A  // ^Z for DOS officionados

#define nUSE_YMODEM_LENGTH

// Data & state local to the protocol
static struct {
    int  __chan;
    unsigned char pkt[1024], *bufp;
    unsigned char blk,cblk,crc1,crc2;
    unsigned char next_blk;  // Expected block
    int len, mode, total_retries;
    int total_SOH, total_STX, total_CAN;
    bool crc_mode, at_eof, tx_ack;
#ifdef USE_YMODEM_LENGTH
    unsigned long file_length, read_length;
#endif
} xyz;

#define xyzModem_CHAR_TIMEOUT            2000  // 2 seconds
#define xyzModem_MAX_RETRIES             20
#define xyzModem_MAX_RETRIES_WITH_CRC    10
#define xyzModem_CAN_COUNT                3    // Wait for 3 CAN before quitting

#ifdef DEBUG
#ifndef USE_SPRINTF
//
// Note: this debug setup only works if the target platform has two serial ports
// available so that the other one (currently only port 1) can be used for debug
// messages.
//
static int
zm_dprintf(const char *fmt, ...)
{
    int cur_console;
    va_list args;

    va_start(args, fmt);
    cur_console = CYGACC_CALL_IF_SET_CONSOLE_COMM(CYGNUM_CALL_IF_SET_COMM_ID_QUERY_CURRENT);
    CYGACC_CALL_IF_SET_CONSOLE_COMM(1);
    diag_vprintf(fmt, args);
    CYGACC_CALL_IF_SET_CONSOLE_COMM(cur_console);
}

static void
zm_flush(void)
{
}

#else
//
// Note: this debug setup works by storing the strings in a fixed buffer
//
static char *zm_out = (char *)0x00380000;
static char *zm_out_start = (char *)0x00380000;

static int
zm_dprintf(const char *fmt, ...)
{
    int len;
    va_list args;

    va_start(args, fmt);
    len = diag_vsprintf(zm_out, fmt, args);
    zm_out += len;
    return len;
}

static void
zm_flush(void)
{
    char *p = zm_out_start;
    while (*p) mon_write_char(*p++);
    zm_out = zm_out_start;
}
#endif

static void
zm_dump_buf(void *buf, int len)
{
    diag_vdump_buf_with_offset(zm_dprintf, (cyg_uint8 *)buf, len, 0);
}

static unsigned char zm_buf[2048];
static unsigned char *zm_bp;

static void
zm_new(void)
{
    zm_bp = zm_buf;
}

static void
zm_save(unsigned char c)
{
    *zm_bp++ = c;
}

static void
zm_dump(int line)
{
    zm_dprintf("Packet at line: %d\n", line);
    zm_dump_buf(zm_buf, zm_bp-zm_buf);
}

#define ZM_DEBUG(x) x
#else
#define ZM_DEBUG(x)
#endif




// Wait for the line to go idle
static void
xyzModem_flush(void)
{
    int res;
    cyg_uint8 c;
    while (true) {
        res = getCharTimeout(xyz.__chan, &c);
        if (!res) return;
    }
}


static int
xyzModem_get_hdr(void)
{
    char c;
    int res;
    bool hdr_found = false;
    int i, can_total, hdr_chars;
    unsigned short cksum;

    ZM_DEBUG(zm_new());
    // Find the start of a header
    can_total = 0;
    hdr_chars = 0;

    if (xyz.tx_ack) {
         putChar(xyz.__chan, ACK);
        xyz.tx_ack = false;
    }
    while (!hdr_found) {
        res = getCharTimeout(xyz.__chan, (cyg_uint8 *) &c);
        ZM_DEBUG(zm_save(c));
        if (res) {
            hdr_chars++;
            switch (c) {
            case SOH:
                xyz.total_SOH++;
            case STX:
                if (c == STX) xyz.total_STX++;
                hdr_found = true;
                break;
            case CAN:
                xyz.total_CAN++;
                ZM_DEBUG(zm_dump(__LINE__));
                if (++can_total == xyzModem_CAN_COUNT) {
                    return xyzModem_cancel;
                } else {
                    // Wait for multiple CAN to avoid early quits
                    break;
                }
            case EOT:
                // EOT only supported if no noise
                if (hdr_chars == 1) {
                     putChar(xyz.__chan, ACK);
                    ZM_DEBUG(zm_dprintf("ACK on EOT #%d\n", __LINE__));
                    ZM_DEBUG(zm_dump(__LINE__));
                    return xyzModem_eof;
                }
            default:
                // Ignore, waiting for start of header
                ;
            }
        } else {
            // Data stream timed out
            xyzModem_flush();  // Toss any current input
            ZM_DEBUG(zm_dump(__LINE__));
            HAL_DELAY_US(250000);
            return xyzModem_timeout;
        }
    }

    // Header found, now read the data
    res = getCharTimeout(xyz.__chan, &xyz.blk);
    ZM_DEBUG(zm_save(xyz.blk));
    if (!res) {
        ZM_DEBUG(zm_dump(__LINE__));
        return xyzModem_timeout;
    }
    res = getCharTimeout(xyz.__chan, &xyz.cblk);
    ZM_DEBUG(zm_save(xyz.cblk));
    if (!res) {
        ZM_DEBUG(zm_dump(__LINE__));
        return xyzModem_timeout;
    }
    xyz.len = (c == SOH) ? 128 : 1024;
    xyz.bufp = xyz.pkt;
    for (i = 0;  i < xyz.len;  i++) {
        res = getCharTimeout(xyz.__chan, (cyg_uint8 *)&c);
        ZM_DEBUG(zm_save(c));
        if (res) {
            xyz.pkt[i] = c;
        } else {
            ZM_DEBUG(zm_dump(__LINE__));
            return xyzModem_timeout;
        }
    }
    res = getCharTimeout(xyz.__chan, &xyz.crc1);
    ZM_DEBUG(zm_save(xyz.crc1));
    if (!res) {
        ZM_DEBUG(zm_dump(__LINE__));
        return xyzModem_timeout;
    }
    if (xyz.crc_mode) {
        res = getCharTimeout(xyz.__chan, &xyz.crc2);
        ZM_DEBUG(zm_save(xyz.crc2));
        if (!res) {
            ZM_DEBUG(zm_dump(__LINE__));
            return xyzModem_timeout;
        }
    }
    ZM_DEBUG(zm_dump(__LINE__));
    // Validate the message
    if ((xyz.blk ^ xyz.cblk) != (unsigned char)0xFF) {
        ZM_DEBUG(zm_dprintf("Framing error - blk: %x/%x/%x\n", xyz.blk, xyz.cblk, (xyz.blk ^ xyz.cblk)));
        ZM_DEBUG(zm_dump_buf(xyz.pkt, xyz.len));
        xyzModem_flush();
        return xyzModem_frame;
    }
    // Verify checksum/CRC
    if (xyz.crc_mode) {
        cksum = cyg_crc16(xyz.pkt, xyz.len);
        if (cksum != ((xyz.crc1 << 8) | xyz.crc2)) {
            ZM_DEBUG(zm_dprintf("CRC error - recvd: %02x%02x, computed: %x\n", 
                                xyz.crc1, xyz.crc2, cksum & 0xFFFF));
            return xyzModem_cksum;
        }
    } else {
        cksum = 0;
        for (i = 0;  i < xyz.len;  i++) {
            cksum += xyz.pkt[i];
        }
        if (xyz.crc1 != (cksum & 0xFF)) {
            ZM_DEBUG(zm_dprintf("Checksum error - recvd: %x, computed: %x\n", xyz.crc1, cksum & 0xFF));
            return xyzModem_cksum;
        }
    }
    // If we get here, the message passes [structural] muster
    return 0;
}

int 
xyzModem_stream_open(connection_info_t *info, int *err)
{
    int stat;
    int retries = xyzModem_MAX_RETRIES;
    int crc_retries = xyzModem_MAX_RETRIES_WITH_CRC;

//    ZM_DEBUG(zm_out = zm_out_start);
#ifdef xyzModem_zmodem
    if (info->mode == xyzModem_zmodem) {
        *err = xyzModem_noZmodem;
        return -1;
    }
#endif

    // Set up the I/O channel.  Note: this allows for using a different port in the future
    xyz.__chan = open(UART_0_NAME,  O_RDWR|O_SYNC|O_NONBLOCK);
    if(xyz.__chan < 0)
    {
    	return xyz.__chan;
    }
    xyz.len = 0;
    xyz.crc_mode = true;
    xyz.at_eof = false;
    xyz.tx_ack = false;
    xyz.mode = info->mode;
    xyz.total_retries = 0;
    xyz.total_SOH = 0;
    xyz.total_STX = 0;
    xyz.total_CAN = 0;
#ifdef USE_YMODEM_LENGTH
    xyz.read_length = 0;
    xyz.file_length = 0;
#endif
    
     putChar(xyz.__chan, (xyz.crc_mode ? 'C' : NAK));

    if (xyz.mode == xyzModem_xmodem) {
	    // X-modem doesn't have an information header - exit here
            xyz.next_blk = 1;
            return 0;
    }

    while (retries-- > 0) {
        stat = xyzModem_get_hdr();
        if (stat == 0) {
            // Y-modem file information header
            if (xyz.blk == 0) {
#ifdef USE_YMODEM_LENGTH
                // skip filename
                while (*xyz.bufp++);
                // get the length
                parse_num(xyz.bufp, &xyz.file_length, NULL, " ");
#endif
                // The rest of the file name data block quietly discarded
                xyz.tx_ack = true;
            }
            xyz.next_blk = 1;
            xyz.len = 0;
            return 0;
        } else 
        if (stat == xyzModem_timeout) {
            if (--crc_retries <= 0) xyz.crc_mode = false;
            HAL_DELAY_US(5*100000);   // Extra delay for startup
             putChar(xyz.__chan, (xyz.crc_mode ? 'C' : NAK));
            xyz.total_retries++;
            ZM_DEBUG(zm_dprintf("NAK (%d)\n", __LINE__));
        }
        if (stat == xyzModem_cancel) {
            break;
        }
    }
    *err = stat;
    ZM_DEBUG(zm_flush());
    return -1;
}

int 
xyzModem_stream_read(char *buf, int size, int *err)
{
    int stat, total, len;
    int retries;

    total = 0;
    stat = xyzModem_cancel;
    // Try and get 'size' bytes into the buffer
    while (!xyz.at_eof && (size > 0)) {
        if (xyz.len == 0) {
            retries = xyzModem_MAX_RETRIES;
            while (retries-- > 0) {
                stat = xyzModem_get_hdr();
                if (stat == 0) {
                    if (xyz.blk == xyz.next_blk) {
                        xyz.tx_ack = true;
                        ZM_DEBUG(zm_dprintf("ACK block %d (%d)\n", xyz.blk, __LINE__));
                        xyz.next_blk = (xyz.next_blk + 1) & 0xFF;
                        // Data blocks can be padded with ^Z (EOF) characters
                        // This code tries to detect and remove them
#ifdef xyzModem_zmodem
                        if (xyz.mode != xyzModem_zmodem) {
#else
                        if (1) {
#endif
                            if ((xyz.bufp[xyz.len-1] == EOF) &&
                                (xyz.bufp[xyz.len-2] == EOF) &&
                                (xyz.bufp[xyz.len-3] == EOF)) {
                                while (xyz.len && (xyz.bufp[xyz.len-1] == EOF)) {
                                    xyz.len--;
                                }
                            }
                        }

#ifdef USE_YMODEM_LENGTH
                        // See if accumulated length exceeds that of the file.
                        // If so, reduce size (i.e., cut out pad bytes)
                        // Only do this for Y-modem (and Z-modem should it ever
                        // be supported since it can fall back to Y-modem mode).
                        if (xyz.mode != xyzModem_xmodem && 0 != xyz.file_length) {
                            xyz.read_length += xyz.len;
                            if (xyz.read_length > xyz.file_length) {
                                xyz.len -= (xyz.read_length - xyz.file_length);
                            }
                        }
#endif
                        break;
                    } else if (xyz.blk == ((xyz.next_blk - 1) & 0xFF)) {
                        // Just re-ACK this so sender will get on with it
                         putChar(xyz.__chan, ACK);
                        continue;  // Need new header
                    } else {
                        stat = xyzModem_sequence;
                    }
                }
                if (stat == xyzModem_cancel) {
                    break;
                }
                if (stat == xyzModem_eof) {
                     putChar(xyz.__chan, ACK);
                    ZM_DEBUG(zm_dprintf("ACK (%d)\n", __LINE__));
                    if (xyz.mode == xyzModem_ymodem) {
                         putChar(xyz.__chan, (xyz.crc_mode ? 'C' : NAK));
                        xyz.total_retries++;
                        ZM_DEBUG(zm_dprintf("Reading Final Header\n"));
                        stat = xyzModem_get_hdr();                        
                         putChar(xyz.__chan, ACK);
                        ZM_DEBUG(zm_dprintf("FINAL ACK (%d)\n", __LINE__));
                    }
                    xyz.at_eof = true;
                    break;
                }
                 putChar(xyz.__chan, (xyz.crc_mode ? 'C' : NAK));
                xyz.total_retries++;
                ZM_DEBUG(zm_dprintf("NAK (%d)\n", __LINE__));
            }
            if (stat < 0) {
                *err = stat;
                xyz.len = -1;
                return total;
            }
        }
        // Don't "read" data from the EOF protocol package
        if (!xyz.at_eof) {
            len = xyz.len;
            if (size < len) len = size;
            memcpy(buf, xyz.bufp, len);
            size -= len;
            buf += len;
            total += len;
            xyz.len -= len;
            xyz.bufp += len;
        }
    }
    return total;
}

void
xyzModem_stream_close(int *err)
{
    diag_printf("xyzModem - %s mode, %d(SOH)/%d(STX)/%d(CAN) packets, %d retries\n", 
                xyz.crc_mode ? "CRC" : "Cksum",
                xyz.total_SOH, xyz.total_STX, xyz.total_CAN,
                xyz.total_retries);
	close(xyz.__chan);
//    ZM_DEBUG(zm_flush());
}

// Need to be able to clean out the input buffer, so have to take the
// getc
void xyzModem_stream_terminate(bool abort, int (*getc)(void))
{
  int c;

  if (abort) {
      ZM_DEBUG(zm_dprintf("!!!! TRANSFER ABORT !!!!\n"));
      switch (xyz.mode) {
	case xyzModem_xmodem:
	case xyzModem_ymodem:
	  // The X/YMODEM Spec seems to suggest that multiple CAN followed by an equal
	  // number of Backspaces is a friendly way to get the other end to abort.
	   putChar(xyz.__chan,CAN);
	   putChar(xyz.__chan,CAN);
	   putChar(xyz.__chan,CAN);
	   putChar(xyz.__chan,CAN);
	   putChar(xyz.__chan,BSP);
	   putChar(xyz.__chan,BSP);
	   putChar(xyz.__chan,BSP);
	   putChar(xyz.__chan,BSP);
	  // Now consume the rest of what's waiting on the line.
	  ZM_DEBUG(zm_dprintf("Flushing serial line.\n"));
	  xyzModem_flush();
          xyz.at_eof = true;
	break;
#ifdef xyzModem_zmodem
	case xyzModem_zmodem:
	  // Might support it some day I suppose.
#endif
	break;
      }
  } else {
      ZM_DEBUG(zm_dprintf("Engaging cleanup mode...\n"));
      // Consume any trailing crap left in the inbuffer from
      // previous recieved blocks. Since very few files are an exact multiple
      // of the transfer block size, there will almost always be some gunk here.
      // If we don't eat it now, RedBoot will think the user typed it.
      ZM_DEBUG(zm_dprintf("Trailing gunk:\n"));
      while ((c = (*getc)()) > -1) ;
      ZM_DEBUG(zm_dprintf("\n"));
      // Make a small delay to give terminal programs like minicom
      // time to get control again after their file transfer program
      // exits.
      HAL_DELAY_US(250000);
  }
}

char *
xyzModem_error(int err)
{
    switch (err) {
    case xyzModem_access:
        return "Can't access file";
        break;
    case xyzModem_noZmodem:
        return "Sorry, zModem not available yet";
        break;
    case xyzModem_timeout:
        return "Timed out";
        break;
    case xyzModem_eof:
        return "End of file";
        break;
    case xyzModem_cancel:
        return "Cancelled";
        break;
    case xyzModem_frame:
        return "Invalid framing";
        break;
    case xyzModem_cksum:
        return "CRC/checksum error";
        break;
    case xyzModem_sequence:
        return "Block sequence error";
        break;
    default:
        return "Unknown error";
        break;
    }
}

//
// RedBoot interface
//
GETC_IO_FUNCS(xyzModem_io, xyzModem_stream_open, xyzModem_stream_close,
              xyzModem_stream_terminate, xyzModem_stream_read, xyzModem_error);
RedBoot_load(xmodem, xyzModem_io, false, false, xyzModem_xmodem);
RedBoot_load(ymodem, xyzModem_io, false, false, xyzModem_ymodem);
