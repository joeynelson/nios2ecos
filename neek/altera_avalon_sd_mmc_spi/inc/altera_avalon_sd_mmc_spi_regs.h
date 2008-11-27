/////////////////////////////////////////////////////////////////////////////////////
//
//       ******                          Module Name: altera_avalon_sd_mmc_spi_regs.h
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
//          ******************           (c) 1999-2008 El Camino GmbH
//
//Description: SD/MMC SPI Interface
//
//
// History    : 1. Jun. 2005    - Initial Release, not tested (KB)
//              29.Jun. 2005    - Final, tested release       (KB)
//              30.Jan. 2008    - added clock control register
//
//
/////////////////////////////////////////////////////////////////////////////////////
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


#ifndef __ALTERA_AVALON_SD_MMC_SPI_REGS_H__
#define __ALTERA_AVALON_SD_MMC_SPI_REGS_H__

#include <io.h>

#define IOADDR_ALTERA_AVALON_SD_MMC_SPI_RXDATA(base)         __IO_CALC_ADDRESS_NATIVE(base, 0)
#define IORD_ALTERA_AVALON_SD_MMC_SPI_RXDATA(base)           IORD(base, 0)
#define IOWR_ALTERA_AVALON_SD_MMC_SPI_RXDATA(base, data)     IOWR(base, 0, data)

#define IOADDR_ALTERA_AVALON_SD_MMC_SPI_TXDATA(base)         __IO_CALC_ADDRESS_NATIVE(base, 1)
#define IORD_ALTERA_AVALON_SD_MMC_SPI_TXDATA(base)           IORD(base, 1)
#define IOWR_ALTERA_AVALON_SD_MMC_SPI_TXDATA(base, data)     IOWR(base, 1, data)

#define IOADDR_ALTERA_AVALON_SD_MMC_SPI_STATUS(base)         __IO_CALC_ADDRESS_NATIVE(base, 2)
#define IORD_ALTERA_AVALON_SD_MMC_SPI_STATUS(base)           IORD(base, 2)
#define IOWR_ALTERA_AVALON_SD_MMC_SPI_STATUS(base, data)     IOWR(base, 2, data)


#define ALTERA_AVALON_SD_MMC_SPI_STATUS_MS_MSK               (0x1)
#define ALTERA_AVALON_SD_MMC_SPI_STATUS_MS_OFST			     (0)
#define ALTERA_AVALON_SD_MMC_SPI_STATUS_WP_MSK			     (0x2)
#define ALTERA_AVALON_SD_MMC_SPI_STATUS_WP_OFST			     (1)
#define ALTERA_AVALON_SD_MMC_SPI_STATUS_CD_MSK			     (0x800)
#define ALTERA_AVALON_SD_MMC_SPI_STATUS_CD_OFST			     (11)


#define ALTERA_AVALON_SD_MMC_SPI_STATUS_ROE_MSK              (0x8)
#define ALTERA_AVALON_SD_MMC_SPI_STATUS_ROE_OFST             (3)
#define ALTERA_AVALON_SD_MMC_SPI_STATUS_TOE_MSK              (0x10)
#define ALTERA_AVALON_SD_MMC_SPI_STATUS_TOE_OFST             (4)
#define ALTERA_AVALON_SD_MMC_SPI_STATUS_TMT_MSK              (0x20)
#define ALTERA_AVALON_SD_MMC_SPI_STATUS_TMT_OFST             (5)
#define ALTERA_AVALON_SD_MMC_SPI_STATUS_TRDY_MSK             (0x40)
#define ALTERA_AVALON_SD_MMC_SPI_STATUS_TRDY_OFST            (6)
#define ALTERA_AVALON_SD_MMC_SPI_STATUS_RRDY_MSK             (0x80)
#define ALTERA_AVALON_SD_MMC_SPI_STATUS_RRDY_OFST            (7)
#define ALTERA_AVALON_SD_MMC_SPI_STATUS_E_MSK                (0x100)
#define ALTERA_AVALON_SD_MMC_SPI_STATUS_E_OFST               (8)

#define IOADDR_ALTERA_AVALON_SD_MMC_SPI_CONTROL(base)        __IO_CALC_ADDRESS_NATIVE(base, 3)
#define IORD_ALTERA_AVALON_SD_MMC_SPI_CONTROL(base)          IORD(base, 3)
#define IOWR_ALTERA_AVALON_SD_MMC_SPI_CONTROL(base, data)    IOWR(base, 3, data)

#define ALTERA_AVALON_SD_MMC_SPI_CONTROL_CSTART_MSK			 (0x1)
#define ALTERA_AVALON_SD_MMC_SPI_CONTROL_CSTART_OFST		 (0)
#define ALTERA_AVALON_SD_MMC_SPI_CONTROL_CRCC_MSK			 (0x2)
#define ALTERA_AVALON_SD_MMC_SPI_CONTROL_CRCC_OFST		     (1)
#define ALTERA_AVALON_SD_MMC_SPI_CONTROL_CRCTX_MSK           (0x4)
#define ALTERA_AVALON_SD_MMC_SPI_CONTROL_CRC_OFST            (2)

#define ALTERA_AVALON_SD_MMC_SPI_CONTROL_ICD_MSK			 (0x800)
#define ALTERA_AVALON_SD_MMC_SPI_CONTROL_ICD_OFST			 (11)

#define ALTERA_AVALON_SD_MMC_SPI_CONTROL_IROE_MSK            (0x8)
#define ALTERA_AVALON_SD_MMC_SPI_CONTROL_IROE_OFST           (3)
#define ALTERA_AVALON_SD_MMC_SPI_CONTROL_ITOE_MSK            (0x10)
#define ALTERA_AVALON_SD_MMC_SPI_CONTROL_ITOE_OFST           (4)
#define ALTERA_AVALON_SD_MMC_SPI_CONTROL_ITRDY_MSK           (0x40)
#define ALTERA_AVALON_SD_MMC_SPI_CONTROL_ITRDY_OFS           (6)
#define ALTERA_AVALON_SD_MMC_SPI_CONTROL_IRRDY_MSK           (0x80)
#define ALTERA_AVALON_SD_MMC_SPI_CONTROL_IRRDY_OFS           (7)
#define ALTERA_AVALON_SD_MMC_SPI_CONTROL_IE_MSK              (0x100)
#define ALTERA_AVALON_SD_MMC_SPI_CONTROL_IE_OFST             (8)
#define ALTERA_AVALON_SD_MMC_SPI_CONTROL_SSO_MSK             (0x400)
#define ALTERA_AVALON_SD_MMC_SPI_CONTROL_SSO_OFST            (10)

#define IOADDR_ALTERA_AVALON_SD_MMC_SPI_SLAVE_SEL(base)      __IO_CALC_ADDRESS_NATIVE(base, 5)
#define IORD_ALTERA_AVALON_SD_MMC_SPI_SLAVE_SEL(base)        IORD(base, 5)
#define IOWR_ALTERA_AVALON_SD_MMC_SPI_SLAVE_SEL(base, data)  IOWR(base, 5, data)

#define IOADDR_ALTERA_AVALON_SD_MMC_SPI_CRC7(base)          __IO_CALC_ADDRESS_NATIVE(base, 4)
#define IORD_ALTERA_AVALON_SD_MMC_SPI_CRC7(base)            IORD(base, 4)
#define IOWR_ALTERA_AVALON_SD_MMC_SPI_CRC7(base, data)      IOWR(base, 4,data)

#define IOADDR_ALTERA_AVALON_SD_MMC_SPI_CLOCK_CONTROL(base)      __IO_CALC_ADDRESS_NATIVE(base, 6)
#define IORD_ALTERA_AVALON_SD_MMC_SPI_CLOCK_CONTROL(base)        IORD(base, 6)
#define IOWR_ALTERA_AVALON_SD_MMC_SPI_CLOCK_CONTROL(base, data)  IOWR(base, 6,data)

#define IOADDR_ALTERA_AVALON_SD_MMC_SPI_CRC16(base)          __IO_CALC_ADDRESS_NATIVE(base, 7)
#define IORD_ALTERA_AVALON_SD_MMC_SPI_CRC16(base)            IORD(base, 7)
#define IOWR_ALTERA_AVALON_SD_MMC_SPI_CRC16(base, data)      IOWR(base, 7,data)



#endif /* __ALTERA_AVALON_SD_MMC_SPI_REGS_H__ */
