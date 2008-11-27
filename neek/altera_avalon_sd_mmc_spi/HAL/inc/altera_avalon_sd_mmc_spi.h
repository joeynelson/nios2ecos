///////////////////////////////////////////////////////////////////////////////
//
//       ******                          Module Name: altera_avalon_sd_mmc_spi.h
//   **************                      Date       : 1. Jun. 2005
// ******************                    Author     : Klaus Brunnbauer
//*******************                    Company    : El Camino GmbH
//*********   ***  ***                   Tel.       : +49 (0)8751-8787-0
//******** ***  * *****    ***           WWW        : http://www.elca.de
// ****** ***** * *****   *****          e-mail     : info@elca.de
//  ***** ***** * *****   *****
//     ** ***** * *****   *****          Revision   : 1.1
//        *****   *************
//        *****   ************
//        *************
//         ************
//                *****
//                *****
//                *****
//         *******************
//      *************************        Copyright
//          ******************           (c) 1999-2005 El Camino GmbH
//
//-- Description: SD/MMC SPI Interface
//--
//--
// History    : 1. Jun. 2005    - Initial Release, not tested (KB)
//              29.Jun. 2005    - Final, tested release       (KB)
//              30.Jan. 2008    - added information for high speed card support
//
//
///////////////////////////////////////////////////////////////////////////////
//  This code is licensed to, the licensee:
//  NOT LICENSED, EVALUATION VERSION
//  El Camino grants to the licensee a non-transferable, non-exclusive,
//  paid-up license to use this code as follows:
//  - design with, parameterize, synthesize, simulate, compile and implement
//    in ASIC or PLD/FPGA technologies
//  - distribute, sell and otherwise market programming files, executables
//    and/or devices based on this code
//  - you may NOT use this source code except as expressly provided for above
//    or sublicense or transfer this code or rights with respect thereto.

#ifndef __ALT_AVALON_SD_MMC_SPI_H__
#define __ALT_AVALON_SD_MMC_SPI_H__

#ifndef ALTERA_AVALON_SD_MMC_SPI_USE_CRC
#define ALTERA_AVALON_SD_MMC_SPI_USE_CRC 1
#endif

#include "alt_types.h"
#include "sys/alt_flash_dev.h"
#include "sys/alt_llist.h"


typedef struct alt_sd_mmc_spi_dev alt_sd_mmc_spi_dev;


typedef struct
{
  alt_u16 cmd_response;
  alt_u8  rx_byte;
  alt_u8  tx_byte;
  alt_u8  crc7;
  alt_u16 crc16;
  alt_u8  use_crc;
  alt_u16 rw_blocklength;
  alt_u8  data_response_token;
}sd_mmc_control;


typedef struct
{
  alt_u16 status_bytes;
  alt_u8  last_error;
  alt_u8  wp;
  alt_u8  cd;
  alt_u32 i;
}sd_mmc_status;


typedef struct
{
    alt_u32   device_size;
    alt_u16   read_block_length;
    alt_u16   write_block_length;
    alt_u8    tran_speed;
    alt_u8    sd_spec;
}sd_mmc_info;


struct  alt_sd_mmc_spi_dev
{
  alt_flash_dev  dev;
  sd_mmc_control control;
  sd_mmc_status  status;
  sd_mmc_info    info;
};



int alt_sd_mmc_spi_init( alt_sd_mmc_spi_dev* sd_mmc,alt_u8 use_crc);


alt_flash_dev* alt_sd_mmc_spi_open(alt_flash_dev* fd,const char* name);

int alt_sd_mmc_spi_close(alt_flash_dev* fd);


int alt_sd_mmc_spi_write(alt_flash_dev* fd, int offset,
                          const void* src_addr, int length);

int alt_sd_mmc_spi_read(alt_flash_dev* fd, int offset,
                        void* dest_addr, int length);


int alt_sd_mmc_spi_write_block(alt_flash_dev* fd, int block_offset,
                                      int data_offset, const void* data,
                                      int length);


int alt_sd_mmc_spi_get_info(alt_flash_dev* fd, flash_region** info,
                            int* number_of_regions  );

int alt_sd_mmc_spi_erase_block ( alt_flash_dev* fd, int offset);



#define ALTERA_AVALON_SD_MMC_SPI_INSTANCE(name, dev)                       \
static alt_sd_mmc_spi_dev dev =                                            \
{                                                                          \
  {                                                                        \
    ALT_LLIST_ENTRY,                                                       \
    name##_NAME,                                                           \
    alt_sd_mmc_spi_open,                                                   \
    alt_sd_mmc_spi_close,                                                  \
    alt_sd_mmc_spi_write,                                                  \
    alt_sd_mmc_spi_read,                                                   \
    alt_sd_mmc_spi_get_info,                                               \
    alt_sd_mmc_spi_erase_block,                                            \
    alt_sd_mmc_spi_write_block,                                            \
    ((void*)( name##_BASE))                                                \
  }                                                                        \
}

#define ALTERA_AVALON_SD_MMC_SPI_INIT(name, dev)  alt_sd_mmc_spi_init (&dev, ALTERA_AVALON_SD_MMC_SPI_USE_CRC)

#endif /* __ALT_AVALON_SD_MMC_SPI_H__ */
