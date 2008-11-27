/**************************************************************************
 * Copyright (c) 2004 Altera Corporation, San Jose, California, USA.      *
 * All rights reserved. All use of this software and documentation is     *
 * subject to the License Agreement located at the end of this file below.*
 *************************************************************************/
/**************************************************************************
 *
 * 
 * Description
 *************** 
 * This is a test program which tests RAM and flash memory. 
 *
 * 
 * Requirements
 ****************
 * This is a "Hosted" application. According to the ANSI C standard, hosted 
 * applications can rely on numerous system-services (including properly-
 * initialized device drivers and, in this case, STDOUT).  
 * 
 * When this program is compiled, code is added before main(), so that all 
 * devices are properly-initialized and all system-services (e.g. the <stdio>
 * library) are ready-to-use. In this hosted environment, all standard C 
 * programs will run.
 * 
 * A hosted application (like this example) does not need to concern itself 
 * with initializing devices. As long as it only calls C Standard Library 
 * functions, a hosted application can run "as if on a workstation."
 * 
 * An application runs in a hosted environment if it declares the function 
 * main(), which this application does.
 * 
 * This software example requires a STDOUT component such as a UART or 
 * JTAG UART, a CFI flash component, and 2 RAM components (one for running
 * the program, and one for testing)  Therefore it can run on the following
 * hardware examples:
 * 
 * Nios Development Board, Stratix II Edition:
 * -  Standard (DMA RAM test will not run)
 * -  Full Featured
 *
 * DSP Development Board, Stratix II Edition:
 * -  Standard (DMA RAM test will not run)
 * -  Full Featured
 *
 * Nios Development Board, Stratix Edition:
 * -  Standard (DMA RAM test will not run)
 * -  Full Featured
 * 
 * Nios Development Board, Stratix Professional Edition:
 * -  Standard (DMA RAM test will not run)
 * -  Full Featured
 * 
 * Nios Development Board, Cyclone Edition:
 * -  Standard (DMA RAM test will not run)
 * -  Full Featured
 *
 * Nios Development Board, Cyclone II Edition:
 * -  Standard (DMA RAM test will not run)
 * -  Full Featured
 *
 * Note: This example will not run on the Nios II Instruction Set Simulator
 * 
 * Peripherals Exercised by SW
 *******************************
 * The example's purpose is to test RAM and flash, as well as demonstrate the 
 * use of the DMA controller and flash API in NiosII.
 * 
 * The RAM test routine performs the following operations:
 * 1.) Tests the address and data lines for shorts and opens. 
 * 2.) Tests byte and half-word access.
 * 3.) Tests every bit in the memory to store both '1' and '0'. 
 * 4.) Tests DMA access to the memory.
 *
 * IMPORTANT: The RAM test is destructive to the contents of the RAM.  For this
 * reason, you MUST assure that none of the software sections are located in 
 * the RAM being tested.  This requires that code, data, and exception 
 * locations must all be in a memory seperate from the one being tested.
 * These locations can be adjusted in Nios II IDE and SOPC Builder.
 *
 *  
 * The flash tests demonstrate the use of the flash programming API.  After the
 * flash device specified is opened, the test routine searches for a block in 
 * the device that is already erased.  This prevents any overwriting of 
 * important data that may be programmed in flash.  When an erased block is 
 * found, the routine performs a test of the flash API calls on that block.
 *
 * The following API functions are then run to test the flash interface:
 * 
 * - alt_get_flash_info
 *    This function queries the flash device and collects various information 
 *    about it.  In the example, the results of this query are compared to what
 *    is expected, and an error is reported in the event of a mismatch.
 * - alt_write_flash
 *    This function writes a specified number of bytes to the flash device.  
 *    In the example, this function is called repeatedly in a loop to write a 
 *    lengthy amount of data.
 * - alt_read_flash
 *    This function reads a specified number of bytes of data from the flash 
 *    device.  In the example, alt_read_flash is used to read back and test 
 *    all of the writing routines.
 * - alt_erase_flash_block
 *    This function performs a block erase on the flash device. 
 * - alt_write_flash_block
 *    This function writes an erase block of data to the flash device.  
 * 
 * During the test, status and error information is passed to the user via 
 * printf's.
 * 
 * Software Files
 ******************
 * memtest.c - Main C file that contains all memory testing code in this 
 *             example.
 * 
 **************************************************************************/


#include <stdio.h>
#include <alt_types.h>
#include <io.h>
#include <system.h>
#include <string.h>

#include "sys/alt_dma.h"
#include "system.h"
#include "sys/alt_flash.h"
#include "sys/alt_flash_dev.h"

/* Mode parameters for Flash Test */
#define TEST 1
#define SHOWMAP 2
#define CFI 3
#define EPCS 4
#define QUIT_WITHOUT_TESTING -1

/* One nice define for going to menu entry functions. */
#define MenuCase(letter,proc) case letter:proc; break;

/* Global DMA "transaction finished" flag */
#ifdef DMA_NAME  
static volatile int rx_done = 0;
#endif /* DMA_NAME */  

/******************************************************************
*  Function: MenuHeader
*
*  Purpose: Prints the menu header.
*
******************************************************************/
static void MenuHeader(void)
{
  printf("\n\n");
  printf("             <---->   Nios II Memory Test.   <---->\n");
  printf("This software example tests the memory in your system to assure it\n");
  printf("is working properly.  This test is destructive to the contents of\n");
  printf("the memory it tests. Assure the memory being tested does not contain\n");
  printf("the executable or data sections of this code or the exception address\n");
  printf("of the system.\n");
}

/******************************************************************
*  Function: MenuBegin
*
*  Purpose: Prints the top portion of the menu.
*
******************************************************************/
static void MenuBegin( char *title )
{
  printf("\n\n");
  printf("----------------------------------\n");
  printf("%s\n",title);
  printf("----------------------------------\n");
}

/******************************************************************
*  Function: MenuItem
*
*  Purpose: Prints selection items in the menu, enumerated by the 
*           specified letter.
*
******************************************************************/
static void MenuItem( char letter, char *name )
{
  printf("     %c:  %s\n" ,letter, name);
}

/******************************************************************
*  Function: GetInputString
*
*  Purpose: Parses an input string for the character '\n'.  Then
*           returns the string, minus any '\r' characters it 
*           encounters.
*
******************************************************************/
void GetInputString( char* entry, int size, FILE * stream )
{
  int i;
  int ch = 0;
  
  for(i = 0; (ch != '\n') && (i < size); )
  {
    if( (ch = getc(stream)) != '\r')
    {
      entry[i] = ch;
      i++;
    }
  }
}

/******************************************************************
*  Function: MenuEnd
*
*  Purpose: Prints the end of the menu, then captures and returns
*           the user's selection.
*
******************************************************************/
static int MenuEnd( char lowLetter, char highLetter )
{
  static char entry[4];
  static char ch;

  printf("     q:  Exit\n");
  printf("----------------------------------\n");
  printf("\nSelect Choice (%c-%c): [Followed by <enter>]",lowLetter,highLetter);

  GetInputString( entry, sizeof(entry), stdin );
  if(sscanf(entry, "%c\n", &ch))
  {
    if( ch >= 'A' && ch <= 'Z' )
      ch += 'a' - 'A';
    if( ch == 27 )
      ch = 'q';
    if(ch != 'q' && ( ch < lowLetter && ch > highLetter ))
    {
      printf("\n -ERROR: %c is an invalid entry.  Please try again\n", ch);
    }
  }
  return ch;
}

/******************************************************************
*  Function: MemGetAddressRange
*
*  Purpose: Gathers a range of memory from the user.
*
******************************************************************/
static int MemGetAddressRange(int* base_address, int* end_address)
{

  char line[12];

  while(1)
  {
    /* Get the base address */
    printf("Base address to start memory test: (i.e. 0x800000)\n");
    printf(">");

    GetInputString( line, sizeof(line), stdin );
   
    /* Check the format to make sure it was entered as hex */
    if(sscanf(line, "0x%X", base_address) != 1)
    {
      printf("%s\n", line);
      printf(" -ERROR: Invalid base address entered.  Address must be in the form '0x800000'\n\n");
      continue;
    }
    
    /* Get the end address */
    printf("End Address:\n");
    printf(">");

    GetInputString( line, sizeof(line), stdin );
    
    /* Check the format to make sure it was entered as hex */
    if(sscanf(line, "0x%X", end_address) != 1)
    {
      printf(" -ERROR: Invalid end address entered.  Address must be in the form '0x8FFFFF'\n\n");
      continue;
    }
    
    /* Make sure end address is greater than base address. */
    if (end_address <= base_address)
    {
      printf(" -ERROR: End address must be greater than the start address\n\n");

      continue;
    }
    break;
  }

  return(0);
}

/******************************************************************
*  Function: MemTestDataBus
*
*  Purpose: Tests that the data bus is connected with no 
*           stuck-at's, shorts, or open circuits.
*
******************************************************************/
static int MemTestDataBus(unsigned int address)
{
  unsigned int pattern;
  unsigned int ret_code = 0x0;

  /* Perform a walking 1's test at the given address. */
  for (pattern = 1; pattern != 0; pattern <<= 1)
  {
    /* Write the test pattern. */
    IOWR_32DIRECT(address, 0, pattern);

    /* Read it back (immediately is okay for this test). */
    if (IORD_32DIRECT(address, 0) != pattern)
    {
      ret_code = pattern;
      break;
    }
  }
  return ret_code;
}


/******************************************************************
*  Function: MemTestAddressBus
*
*  Purpose: Tests that the address bus is connected with no 
*           stuck-at's, shorts, or open circuits.
*
******************************************************************/
static int MemTestAddressBus(unsigned int memory_base, unsigned int nBytes)
{
  unsigned int address_mask = (nBytes - 1);
  unsigned int offset;
  unsigned int test_offset;

  unsigned int pattern     = 0xAAAAAAAA;
  unsigned int antipattern  = 0x55555555;

  unsigned int ret_code = 0x0;

  /* Write the default pattern at each of the power-of-two offsets. */
  for (offset = sizeof(unsigned int); (offset & address_mask) != 0; offset <<= 1)
  {
    IOWR_32DIRECT(memory_base, offset, pattern);
  }

  /* Check for address bits stuck high. */
  test_offset = 0;
  IOWR_32DIRECT(memory_base, test_offset, antipattern);
  for (offset = sizeof(unsigned int); (offset & address_mask) != 0; offset <<= 1)
  {
     if (IORD_32DIRECT(memory_base, offset) != pattern)
     {
        ret_code = (memory_base+offset);
        break;
     }
  }

  /* Check for address bits stuck low or shorted. */
  IOWR_32DIRECT(memory_base, test_offset, pattern);
  for (test_offset = sizeof(unsigned int); (test_offset & address_mask) != 0; test_offset <<= 1)
  {
    if (!ret_code)
    {
      IOWR_32DIRECT(memory_base, test_offset, antipattern);
      for (offset = sizeof(unsigned int); (offset & address_mask) != 0; offset <<= 1)
      {
        if ((IORD_32DIRECT(memory_base, offset) != pattern) && (offset != test_offset))
        {
          ret_code = (memory_base + test_offset);
          break;
        }
      }
      IOWR_32DIRECT(memory_base, test_offset, pattern);
    }
  }

  return ret_code;
}


/******************************************************************
*  Function: MemTest8_16BitAccess
*
*  Purpose: Tests that the memory at the specified base address
*           can be read and written in both byte and half-word 
*           modes.
*
******************************************************************/
static int MemTest8_16BitAccess(unsigned int memory_base)
{
  int ret_code = 0x0;

  /* Write 4 bytes */
  IOWR_8DIRECT(memory_base, 0, 0x0A);
  IOWR_8DIRECT(memory_base, 1, 0x05);
  IOWR_8DIRECT(memory_base, 2, 0xA0);
  IOWR_8DIRECT(memory_base, 3, 0x50);

  /* Read it back as one word */
  if(IORD_32DIRECT(memory_base, 0) != 0x50A0050A)
  {
    ret_code = memory_base;
  }

  /* Read it back as two half-words */
  if (!ret_code)
  {
    if ((IORD_16DIRECT(memory_base, 2) != 0x50A0) &&
        (IORD_16DIRECT(memory_base, 0) != 0x050A))
    {
      ret_code = memory_base;
    }
  }

  /* Read it back as 4 bytes */
  if (!ret_code)
  {
    if ((IORD_8DIRECT(memory_base, 3) != 0x50) &&
        (IORD_8DIRECT(memory_base, 2) != 0xA0) &&
        (IORD_8DIRECT(memory_base, 1) != 0x05) &&
        (IORD_8DIRECT(memory_base, 0) != 0x0A))
    {
    ret_code = memory_base;
    }
  }

  /* Write 2 half-words */
  if (!ret_code)
  {
    IOWR_16DIRECT(memory_base, 0, 0x50A0);
    IOWR_16DIRECT(memory_base, 2, 0x050A);

    /* Read it back as one word */
    if(IORD_32DIRECT(memory_base, 0) != 0x050A50A0)
    {
      ret_code = memory_base;
    }
  }

  /* Read it back as two half-words */
  if (!ret_code)
  {
    if ((IORD_16DIRECT(memory_base, 2) != 0x050A) &&
        (IORD_16DIRECT(memory_base, 0) != 0x50A0))
    {
      ret_code = memory_base;
    }
  }

  /* Read it back as 4 bytes */
  if (!ret_code)
  {
    if ((IORD_8DIRECT(memory_base, 3) != 0x05) &&
        (IORD_8DIRECT(memory_base, 2) != 0x0A) &&
        (IORD_8DIRECT(memory_base, 1) != 0x50) &&
        (IORD_8DIRECT(memory_base, 0) != 0xA0))
    {
      ret_code = memory_base;
    }
  }

  return(ret_code);
}


/******************************************************************
*  Function: MemTestDevice
*
*  Purpose: Tests that every bit in the memory device within the 
*           specified address range can store both a '1' and a '0'.
*
******************************************************************/
static int MemTestDevice(unsigned int memory_base, unsigned int nBytes)
{
  unsigned int offset;
  unsigned int pattern;
  unsigned int antipattern;
  unsigned int ret_code = 0x0;

  /* Fill memory with a known pattern. */
  for (pattern = 1, offset = 0; offset < nBytes; pattern++, offset+=4)
  {
    IOWR_32DIRECT(memory_base, offset, pattern);
  }

  printf(" .");

  /* Check each location and invert it for the second pass. */
  for (pattern = 1, offset = 0; offset < nBytes; pattern++, offset+=4)
  {
    if (IORD_32DIRECT(memory_base, offset) != pattern)
    {
      ret_code = (memory_base + offset);
      break;
    }
    antipattern = ~pattern;
    IOWR_32DIRECT(memory_base, offset, antipattern);
  }

  printf(" .");

  /* Check each location for the inverted pattern and zero it. */
  for (pattern = 1, offset = 0; offset < nBytes; pattern++, offset+=4)
  {
    antipattern = ~pattern;
    if (IORD_32DIRECT(memory_base, offset) != antipattern)
    {
      ret_code = (memory_base + offset);
      break;
    }
    IOWR_32DIRECT(memory_base, offset, 0x0);
  }
  return ret_code;
}

/******************************************************************
*  Function: dma_done
*
*  Purpose: Called when a DMA recieve transaction is complete.
*           Increments rx_done to signal to the main program that
*           the transaction is done.
*
******************************************************************/
#ifdef DMA_NAME  
static void dma_done (void* handle, void* data)
{
  rx_done++;
}
#endif /* DMA_NAME */  

/******************************************************************
*  Function: MemDMATest
*
*  Purpose: Tests every bit in the memory device within the 
*  specified address range using DMA.  The DMA controller provides 
*  a more rigourous test of the memory since it performs back-to-
*  back memory accesses at full system speed.
*
******************************************************************/
#ifdef DMA_NAME  
static int MemDMATest(unsigned int memory_base, unsigned int nBytes)
{
  int rc;
  int ret_code = 0;
  int pattern, offset;
  alt_dma_txchan txchan;
  alt_dma_rxchan rxchan;
  void* data_written;
  void* data_read;
  
  /* Get a couple buffers for the test */
  data_written = (void*)alt_uncached_malloc(0x1000);
  data_read = (void*)alt_uncached_malloc(0x1000);
  
  
  /* Fill write buffer with known values */
  for (pattern = 1, offset = 0; offset < 0x1000; pattern++, offset+=4)
  {
    IOWR_32DIRECT((int)data_written, offset, pattern);
  }

  /* Create the transmit channel */
  if ((txchan = alt_dma_txchan_open("/dev/dma")) == NULL)
  {
    printf ("Failed to open transmit channel\n");
    exit (1);
  }
  
  /* Create the receive channel */
  if ((rxchan = alt_dma_rxchan_open("/dev/dma")) == NULL)
  {
    printf ("Failed to open receive channel\n");
    exit (1);
  }
  
  for(offset = memory_base; offset < (memory_base + nBytes); offset += 0x1000)
  {
    /* Use DMA to transfer from write buffer to memory under test */
    /* Post the transmit request */
    if ((rc = alt_dma_txchan_send (txchan, data_written, 0x1000, NULL, NULL)) < 0)
    {
      printf ("Failed to post transmit request, reason = %i\n", rc);
      exit (1);
    }

    /* Post the receive request */
    if ((rc = alt_dma_rxchan_prepare (rxchan, (void*)offset, 0x1000, dma_done, NULL)) < 0)
    {
      printf ("Failed to post read request, reason = %i\n", rc);
      exit (1);
    }
  
    /* Wait for transfer to complete */
    while (!rx_done);
    rx_done = 0;
    
    /* Clear the read buffer before we fill it */
    memset(data_read, 0, 0x1000);
    
    /* Use DMA to read data back into read buffer from memory under test */
    /* Post the transmit request */
    if ((rc = alt_dma_txchan_send (txchan, (void*)offset, 0x1000, NULL, NULL)) < 0)
    {
      printf ("Failed to post transmit request, reason = %i\n", rc);
      exit (1);
    }

    /* Post the receive request */
    if ((rc = alt_dma_rxchan_prepare (rxchan, data_read, 0x1000, dma_done, NULL)) < 0)
    {
      printf ("Failed to post read request, reason = %i\n", rc);
      exit (1);
    }

    /* Wait for transfer to complete */
    while (!rx_done);
    rx_done = 0;
    
    if (memcmp(data_written, data_read, 0x1000))
    {
      ret_code = offset;
      break;
    }
  }
  alt_uncached_free(data_written);
  alt_uncached_free(data_read);
  return ret_code;
}
#endif /* DMA_NAME */  


/******************************************************************
*  Function: TestRam
*
*  Purpose: Performs a full-test on the RAM specified.  The tests
*           run are:
*             - MemTestDataBus
*             - MemTestAddressBus
*             - MemTest8_16BitAccess
*             - MemTestDevice
*             - MemDMATest
*
******************************************************************/
static void TestRam(void)
{
  
  int memory_base, memory_end, memory_size;
  int ret_code = 0x0;

  /* Find out what range of memory we are testing */
  MemGetAddressRange(&memory_base, &memory_end);
  memory_size = (memory_end - memory_base);

  printf("\n");
  printf("Testing RAM from 0x%X to 0x%X\n", memory_base, (memory_base + memory_size));

  /* Test Data Bus. */
  ret_code = MemTestDataBus(memory_base);

  if (ret_code)
   printf(" -Data bus test failed at bit 0x%X", (int)ret_code);
  else
    printf(" -Data bus test passed\n");

  /* Test Address Bus. */
  if (!ret_code)
  {
    ret_code  = MemTestAddressBus(memory_base, memory_size);
    if  (ret_code)
      printf(" -Address bus test failed at address 0x%X", (int)ret_code);
    else
      printf(" -Address bus test passed\n");
  }

  /* Test byte and half-word access. */
  if (!ret_code)
  {
    ret_code = MemTest8_16BitAccess(memory_base);
    if  (ret_code)
      printf(" -Byte and half-word access test failed at address 0x%X", (int)ret_code);
    else
      printf(" -Byte and half-word access test passed\n");
  }

  /* Test that each bit in the device can store both 1 and 0. */
  if (!ret_code)
  {
    printf(" -Testing each bit in memory device.");
    ret_code = MemTestDevice(memory_base, memory_size);
    if  (ret_code)
      printf("  failed at address 0x%X", (int)ret_code);
    else
      printf("  passed\n");
  }
  
  /* Test DMA access to the RAM if DMA exists */
#ifdef DMA_NAME  
  if (!ret_code)
  {
    printf(" -Testing memory using DMA.");
    ret_code = MemDMATest(memory_base, memory_size);
    if  (ret_code)
      printf("  failed at address 0x%X", (int)ret_code);
    else
      printf("  passed\n");
  }
#endif /* DMA_NAME */
      
  if (!ret_code)
    printf("Memory at 0x%X Okay\n", memory_base);
}


/******************************************************************
*  Function: FlashCheckIfBlockErased
*
*  Purpose: Checks the specified flash block to see if it is 
*           completely erased (all 0xFFFFFFFF).
*
******************************************************************/
static int FlashCheckIfBlockErased(alt_flash_fd* fd, int block, flash_region* regions)
{
  int i, j;
  int ret_code = 0x0;
  char block_is_erased = 0x1;
  alt_u8 *data_read;
 
  /* Get a buffer */
  data_read = malloc(64);
  
  /* Initialize the flag */
  block_is_erased = 0x1;

  for(i = 0; i < regions->block_size; i += 64)
  {
    ret_code = alt_read_flash(fd, ((block * regions->block_size) + i), data_read, 64);

    for(j=0; j < 64; j+=1)
    {
      if(*(data_read+j) != 0xFF)
      {
        /* If this byte isn't erased, then neither is the block */
        block_is_erased = 0x0;
        break;
      }
    }
    if (block_is_erased == 0x0)
      break;
  }
  /* Block is erased if we indexed through all block locations */
  if(i == regions->block_size)
    ret_code = 1;
  else
    ret_code = 0;
  
  free(data_read);
 
  return ret_code;
}


/******************************************************************
*  Function: FlashTestBlockWrite
*
*  Purpose: Tests that the function alt_write_flash_block is
*           is working properly.
*
******************************************************************/
static int FlashTestBlockWrite(int block, int *error, alt_flash_fd* fd, flash_region* regions)
{
  int i;
  int ret_code = 0x0;
  int test_offset;

  alt_u8 *data_written;
  alt_u8 *data_read;


  /* Get a couple buffers for the test */
  data_written = malloc(100);
  data_read = malloc(100);

  test_offset = (regions->offset + (block * regions->block_size));

  /* Fill write buffer with 100 values (incremented by 3) */
  for(i=0; i < 100; i++)
    *(data_written + i) = (i * 3);

  /* Write the buffer to flash starting 0x40 bytes from the beginning of the block. */
  printf(" -Testing \"alt_write_flash_block\".");
  ret_code = alt_write_flash_block(fd, test_offset, (test_offset + 0x40), data_written, 100);
  if (!ret_code)
  {
    /* Now read it back into the read_buffer */
    ret_code = alt_read_flash(fd, (test_offset + 0x40), data_read, 100);
    if(!ret_code)
    {
      /* See if they match */
      if (memcmp(data_written, data_read, 100))
      {
        printf("  FAILED.\n");
        *error++;
      }
      else
        printf("  passed.\n");
    }
  }

  /* Test unaligned writes */
  if(!ret_code)
  {
    /* Erase the block */
    ret_code = alt_erase_flash_block(fd, test_offset, regions->block_size);
  
    /* Write the buffer to flash on an unaligned address. */
    printf(" -Testing unaligned writes.");
    ret_code = alt_write_flash_block(fd, test_offset, (test_offset + 0x43), data_written, 100);
    if (!ret_code)
    {
      /* Now read it back into the read_buffer */
      ret_code = alt_read_flash(fd, (test_offset + 0x43), data_read, 100);
      if(!ret_code)
      {
        /* See if they match */
        if (memcmp(data_written, data_read, 100))
        {
          printf("  FAILED.\n");
          *error++;
        }
        else
          printf("  passed.\n");
      }
    }
  }

  /* Free up the buffers we allocated. */
  free(data_written);
  free(data_read);
  
  return ret_code;
}


/******************************************************************
*  Function: FlashTestReadWrite
*
*  Purpose: Tests that the functions alt_write_flash and
*           alt_read_flash are working properly, as well as tests
*           that every bit in the specified block can store both
*           a '1' and '0'.
*
******************************************************************/
static int FlashTestReadWrite(int block, int *error, alt_flash_fd* fd, flash_region* regions)
{
  int i;
  int ret_code = 0x0;
  int test_offset;

  alt_u8 *data_written;
  alt_u8 *data_read;
 

  /* Get a couple buffers for the tests */
  data_written = malloc(regions->block_size);
  data_read = malloc(regions->block_size);
 
  /* Calculate the offset at which the block lives */
  test_offset = (regions->offset + (block * regions->block_size));

  printf("\n -Starting Flash Test.\n");
 
  printf(" -Testing \"alt_write_flash\" and \"alt_read_flash\".\n");
  /* Fill buffer with incrementing values */
  for(i=0; i < regions->block_size; i++)
    *(data_written + i) = i;

  /* Write the buffer to flash block */
  ret_code = alt_write_flash(fd, test_offset, data_written, regions->block_size);
     
  if (!ret_code)
  {
    /* Read flash block into read buffer */
    ret_code = alt_read_flash(fd, test_offset, data_read, regions->block_size);
    if(!ret_code)
    {
      /* See if they match */
      if (memcmp(data_written, data_read, regions->block_size))
      {
        printf("    pass 1 - FAILED.\n");
        *error++;
      }
      else
        printf("    pass 1 - passed.\n");
    }
  
    /* Now fill the buffer with decrementing values (invert the incrementing ones) */
    for(i=0; i < regions->block_size; i++)
      *(data_written + i) = ~((alt_u8)(i));
 
    /* Write the buffer to flash block */
    ret_code = alt_write_flash(fd, test_offset, data_written, regions->block_size);
    
    if (!ret_code)
    {
      /* Read flash block into read buffer */
      ret_code = alt_read_flash(fd, test_offset, data_read, regions->block_size);
      if(!ret_code)
      {
        /* See if they match */
        if (memcmp(data_written, data_read, regions->block_size))
        {
          printf("    pass 2 - FAILED.\n");
          *error++;
        }
        else
          printf("    pass 2 - passed.\n");
      }
    }
    if (*error)
      ret_code = 1;
  }

  /* Free up the buffers we allocated */
  free(data_written);
  free(data_read);
  
  return ret_code;
}


/******************************************************************
*  Function: FlashTestBlockErase
*
*  Purpose: Tests that the function alt_erase_flash_block is
*           is working properly.  Assumes that the specified
*           flash block contains some non-0xFFFFFFFF data before
*           this function is called.
*
******************************************************************/
static int FlashTestBlockErase(int block, int *error, alt_flash_fd* fd, flash_region* regions)
{

  int ret_code = 0x0;
  int test_offset;

  /* Calculate the offset of the block */
  test_offset = (regions->offset + (block * regions->block_size));

  printf(" -Testing \"alt_erase_flash_block\".");
  ret_code = alt_erase_flash_block(fd, test_offset, regions->block_size);
  /* Check that the erase was successful. */
  if (!ret_code)
  {
    if(FlashCheckIfBlockErased(fd, block, regions))
      printf("  passed.\n");
    else
    {
      printf("  FAILED\n");  
      *error++;
    }
  }
  
  return ret_code;
}


/******************************************************************
*  Function: FlashRunTests
*
*  Purpose: Performs a full-test on the Flash specified.  The tests
*           run are:
*             - alt_write_flash
*             - alt_read_flash
*             - alt_erase_flash_block
*             - alt_write_flash_block
* 
******************************************************************/
static void FlashRunTests(alt_flash_fd* fd, int block, flash_region* regions)
{
  int ret_code = 0x0;
  int error = 0x0;
  int test_offset;

  /* Calculate the offset of the block */
  test_offset = (regions->offset + (block * regions->block_size));
  
  /* Test reading and writing functions */
  ret_code = FlashTestReadWrite(block, &error, fd, regions);
 
  /* Test the erase function */
  if (!ret_code)
  {
    ret_code = FlashTestBlockErase(block, &error, fd, regions);
  }
  /* Test the block write function */
  if (!ret_code)
  {
    ret_code = FlashTestBlockWrite(block, &error, fd, regions);
  }

  /* Erase the block so we dont fill one up each time we run the test */
  printf(" -Returning block %d to its erased state.\n", block);
  alt_erase_flash_block(fd, test_offset, regions->block_size);
 
  printf(" -Flash tests complete.\n");
  if(ret_code || error)
  {
    printf(" -At least one test failed.\n\n");
  }
}


/******************************************************************
*  Function: GetFlashName
*
*  Purpose: Gets the name of the flash to test from the user
*           Defaults to "/dev/ext_flash", the name of the flash
*           component in the Nios II example designs.
* 
******************************************************************/
static int GetFlashName(char line[30], int flash_type)
{

  char ch = 0x0;
  int i;

  if (flash_type == CFI)
  { 
    printf("\nEnter the name of the CFI flash device to be opened,\n");
    printf("or just press <enter> to open \"/dev/ext_flash\")\n");
    printf(">");
  }
  else if (flash_type == EPCS)
  {
    printf("\nEnter the name of the EPCS flash device to be opened,\n");
    printf("or just press <enter> to open \"/dev/epcs_controller\")\n");
    printf(">");
  }
 
  for(i = 0; ch != '\n'; i++)
  {
    ch = getc(stdin);
    if(ch == '\r' || ch == '\n')
    {
      /* Hitting <enter> defaults to the standard component name */
      if( i <= 1 )
      {
        if (flash_type == CFI)
          strcpy(line, "/dev/ext_flash\0");
        else if (flash_type == EPCS)
          strcpy(line, "/dev/epcs_controller\0");
      }
         
      else
        /* Properly terminate the string. */
        line[i] = '\0';
    }
    else
     line[i] = ch;
  }
 
  return 0;
}



/******************************************************************
*  Function: FlashErase
*
*  Purpose: Erases 1 or all blocks in the specified flash device.
* 
******************************************************************/
static void FlashErase(int flash_type)
{
  alt_flash_fd* fd;
  int test_offset;
  int ret_code;
  flash_region* regions;
  int number_of_regions;
  alt_u8 entry[4];
  alt_u8 flashname[30];
  unsigned int block;
 
  /* Get the name of the flash we are erasing */
  ret_code = GetFlashName(flashname, flash_type);
 
  fd = alt_flash_open_dev(flashname);
  if (fd)
  {
    /* Find out some useful stuff about the flash */
    ret_code = alt_get_flash_info(fd, &regions, &number_of_regions);
    if (!ret_code)
    {
      printf(" -Region has %d blocks.\n", regions->number_of_blocks);
      printf(" -Which block would you like to erase?\n");
      printf(" -> ");
      
      GetInputString( entry, sizeof(entry), stdin );

      if(entry[0] == 'a')
      {
        printf(" -Erase ALL blocks? (y/n) ");

        GetInputString( entry, sizeof(entry), stdin );
        
        if(entry[0] == 'y')
        {
          /* Erase all blocks */
          printf(" -Erasing %d blocks.  Please Wait.\n", (regions->number_of_blocks));
          for(block = 0; block < regions->number_of_blocks; block++)
          {
            /* Dont erase it if it's already erased silly. */
            if ((FlashCheckIfBlockErased(fd, block, regions)) == 0)
            {
              test_offset = (regions->offset + (block * regions->block_size));
              alt_erase_flash_block(fd, test_offset, regions->block_size);
            }
            /* Just a simple progress meter so we dont get bored waiting for the flash to erase. */
            printf(".");
            if(((block + 1) % 80) == 0)
            {
              printf("\n");
            }
          }
          printf("\n -All Blocks Erased.\n");
        }
        else
        {
          printf("Erased zero blocks.\n");
        }
      }
      /* Just erase one block */
      if(sscanf(entry, "%d\n", &block))
      {
        if ((block >= 0) && (block <= (regions->number_of_blocks - 1)))
        {
          test_offset = (regions->offset + (block * regions->block_size));
          alt_erase_flash_block(fd, test_offset, regions->block_size);
          printf(" -Block %d erased.\n", block);
        }
        else
        {
          printf(" -Block number entered is %d\n", block);
          printf(" -Block number must be between 0 and %d.\n", (regions->number_of_blocks - 1));
        }
      }
    }
    printf(" -Closing flash \"%s\".\n", flashname);
    alt_flash_close_dev(fd);
  }
}

  
/******************************************************************
*  Function: FlashFindErasedBlocks
*
*  Purpose: Looks through the specified flash for blocks which 
*           are completely erased.  If the mode parameter is 
*           TEST, this function simply returns the index of the 
*           first block which is completely erased.  If the mode
*           parameter is SHOWMAP, the function prints a list of 
*           all blocks, indicating which ones are erased.
* 
******************************************************************/
static int FlashFindErasedBlocks(alt_flash_fd* fd, flash_region* regions, int number_of_regions, int mode)
{ 
  int region_index, block_index;
  int block_erased = 0x0;
  alt_u8 entry[5];
  unsigned int block;

  /* Currently only supports flashes with 1 region, but region loop is left here for possible */
  /* future implementation */
  for(region_index = 0; region_index < number_of_regions; region_index++)
  {
    printf(" -Checking Region %d for erased blocks.\n", region_index);
    /* SHOWMAP mode has a legend reminding us what little plus and minus signs mean */
    if(mode == SHOWMAP)
    {
      printf("            erased block = '-'\n");     
      printf("          unerased block = '+'\n\n");     
    }
    /* Check those blocks. */
    for(block_index = 0; block_index < (regions->number_of_blocks); block_index++)
    {
      block_erased = FlashCheckIfBlockErased(fd, block_index, regions);
      /* If it's erased and were running in TEST mode, we're done */
      if(block_erased && (mode == TEST))
        break;
      /* If in SHOWMAP mode, mark block as either erased or not-erased. */
      else if(block_erased && (mode == SHOWMAP))
        printf("  Block %3d @ 0x%8.8X:\t-\n", block_index, (regions->offset + (block_index * regions->block_size)));           
      else if(!block_erased && (mode == SHOWMAP))
        printf("  Block %3d @ 0x%8.8X:\t+\n", block_index, (regions->offset + (block_index * regions->block_size)));           
    }
    /* Special case if no blocks are erased (TEST mode only)*/
    if(( block_index == ( regions->number_of_blocks )) && ( mode == TEST ))
    {
      printf(" -Found no erased blocks.  Please enter the number of the block\n");
      printf("  you would like to test.  Enter 'q' to quit without testing flash.\n");
      printf(" -> ");

      GetInputString( entry, sizeof(entry), stdin );

      if(entry[0] == 'q')
      {
    		block_index = QUIT_WITHOUT_TESTING;
    		break;
    	}
      else if(sscanf(entry, "%d\n", &block))
      {
        if ((block >= 0) && (block <= (regions->number_of_blocks - 1)))
        {
        	block_index = block;
        	break;
        }
        else 
        {
          printf(" -Block number entered is %d\n", block);
          printf(" -Block number must be between 0 and %d.\n", (regions->number_of_blocks - 1));
        }
      }    	
    }
    /* Break out of the region loop if we've found an erased block to test. */
    if(block_erased && (mode == TEST))
      break;
  }

  return block_index;
}


/******************************************************************
*  Function: TestFlash
*
*  Purpose: Opens the specified flash device.  If the mode
*           parameter is TEST, the function finds an erased 
*           block, then tests it.  If the mode parameter is 
*           SHOWMAP, the function lists all blocks in the flash and
*           indicates which ones are erased.  The flash is closed
*           at the end of the function.
* 
******************************************************************/
static void TestFlash(int mode, int flash_type)
{
  alt_flash_fd* fd;
  int number_of_regions;
  int block;
  flash_region* regions;
  int ret_code = 0x0;
  alt_u8 entry[4];
  alt_u8 flashname[30];
  
  ret_code = GetFlashName(flashname, flash_type);

  fd = alt_flash_open_dev(flashname);
  if (fd)
  {
    printf(" -Successfully opened %s\n", flashname);
    
    /* Get some useful info about the flash */
    ret_code = alt_get_flash_info(fd, &regions, &number_of_regions);
      
    if (!ret_code)
    {
      printf(" -Region 0 contains %d blocks.\n", regions->number_of_blocks);
      
      block = FlashFindErasedBlocks(fd, regions, number_of_regions, mode);

      /* If we're in TEST mode, ask if this block is okay to test. */
      if(( mode == TEST ) && ( block != QUIT_WITHOUT_TESTING ))
      {
        printf(" -Block %d, at address 0x%X identified.\n", block, (regions->offset + (block * regions->block_size)));
        printf(" -Would you like to test this block? (y/n)");

        GetInputString(entry, sizeof(entry), stdin);

        if ( entry[0] == 'y' && entry[1] == '\n' )
        {
          /* Test that Flash! */
          FlashRunTests(fd, block, regions);
          printf(" -Closing flash device \"%s\".\n", flashname);
          alt_flash_close_dev(fd);
        }       
      }
    }
  }
  else
  {
    printf(" -ERROR: Could not open %s\n", flashname);   
  }
}


/******************************************************************
*  Function: TopMenu
*
*  Purpose: Generates the top level menu.
* 
******************************************************************/
static int TopMenu( void )
{
  char ch;

  /* Print the top-level menu to stdout */
  while (1)
  {
    MenuBegin("      Memory Test Main Menu");
    MenuItem( 'a', "Test RAM" );
    MenuItem( 'b', "Test Flash");
#ifdef EPCS_CONTROLLER_NAME    
    MenuItem( 'c', "Test EPCS Serial Flash");
#endif /* EPCS_CONTROLLER_NAME */
    ch = MenuEnd( 'a', 'b' );

    switch(ch)
    {
      MenuCase('a',TestRam());
      MenuCase('b',TestFlash(TEST, CFI));
#ifdef EPCS_CONTROLLER_NAME    
      MenuCase('c',TestFlash(TEST, EPCS));
#endif /* EPCS_CONTROLLER_NAME */
      MenuCase('e',FlashErase(CFI));       /* hidden option */
      MenuCase('f',FlashErase(EPCS));       /* hidden option */
      MenuCase('m',TestFlash(SHOWMAP, CFI)); /* hidden option */
      MenuCase('s',TestFlash(SHOWMAP, EPCS)); /* hidden option */
    }
    if (ch == 'q')
      break;
    printf("\nPress enter to continue...\n");
    while( (( ch = getc(stdin)) != '\n' ) && ( ch != EOF ));

  }
  return (ch);
}


/******************************************************************
*  Function: main
*
*  Purpose: Continually prints the menu and performs the actions
*           requested by the user.
* 
******************************************************************/
int main(void)
{

  int ch;

  /* Print the Header */
  MenuHeader();
  /* Print the menu and do what the user requests, until they hit 'q' */
  while (1)
  {
    ch = TopMenu();
    if (ch == 'q')
    {
      printf( "\nExiting from Memory Test.\n");
      break;
    }
  }
  return (0);
}


/******************************************************************************
*                                                                             *
* License Agreement                                                           *
*                                                                             *
* Copyright (c) 2004 Altera Corporation, San Jose, California, USA.           *
* All rights reserved.                                                        *
*                                                                             *
* Permission is hereby granted, free of charge, to any person obtaining a     *
* copy of this software and associated documentation files (the "Software"),  *
* to deal in the Software without restriction, including without limitation   *
* the rights to use, copy, modify, merge, publish, distribute, sublicense,    *
* and/or sell copies of the Software, and to permit persons to whom the       *
* Software is furnished to do so, subject to the following conditions:        *
*                                                                             *
* The above copyright notice and this permission notice shall be included in  *
* all copies or substantial portions of the Software.                         *
*                                                                             *
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR  *
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,    *
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE *
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER      *
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING     *
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER         *
* DEALINGS IN THE SOFTWARE.                                                   *
*                                                                             *
* This agreement shall be governed in all respects by the laws of the State   *
* of California and by the laws of the United States of America.              *
* Altera does not recommend, suggest or require that this reference design    *
* file be used in conjunction or combination with any other product.          *
******************************************************************************/
