#ifndef CYGONCE_ALTERA_AVALON_CF_PRIV_H
#define CYGONCE_ALTERA_AVALON_CF_PRIV_H
//==========================================================================
//
//      altera_avalon_cf_private.h
//
//      private header for the Altera Avalon compact flash driver
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//==========================================================================

#include <cyg/infra/cyg_type.h>

// IDE Register Indices
#define IDE_REG_DATA      0
#define IDE_REG_ERROR     1
#define IDE_REG_FEATURES  1
#define IDE_REG_COUNT     2
#define IDE_REG_REASON    2  // ATAPI
#define IDE_REG_LBALOW    3
#define IDE_REG_LBAMID    4
#define IDE_REG_LBAHI     5
#define IDE_REG_DEVICE    6
#define IDE_REG_STATUS    7
#define IDE_REG_COMMAND   7

#define IDE_STAT_BSY      0x80
#define IDE_STAT_DRDY     0x40
#define IDE_STAT_SERVICE  0x10
#define IDE_STAT_DRQ      0x08
#define IDE_STAT_CORR     0x04
#define IDE_STAT_ERR      0x01

#define IDE_REASON_REL    0x04
#define IDE_REASON_IO     0x02
#define IDE_REASON_COD    0x01

/* flag values */

#define IDE_DEV_PRESENT  1  // Device is present
#define IDE_DEV_PACKET   2  // Supports packet interface
#define IDE_DEV_ADDR48   3  // Supports 48bit addressing

#include <cyg/infra/cyg_type.h>

// Initialize the IDE controller(s).

#define ALT_AVALON_CF_READ_UINT8( __ctlr, __regno, __val )  \
  HAL_READ_UINT8 (((__ctlr) + (__regno)), __val)

#define ALT_AVALON_CF_READ_UINT16( __ctlr, __regno, __val )  \
  HAL_READ_UINT16 (((__ctlr) + (__regno)), __val)

#define ALT_AVALON_CF_READ_ALTSTATUS( __ctlr, __val )  \
  HAL_READ_UINT32 (((__ctlr) + 14), __val)
  
#define ALT_AVALON_CF_WRITE_UINT8( __ctlr, __regno, __val )  \
  HAL_WRITE_UINT8(((__ctlr) + (__regno)), __val) 

#define ALT_AVALON_CF_WRITE_UINT16( __ctlr, __regno, __val )  \
  HAL_WRITE_UINT16(((__ctlr) + (__regno)), __val) 

#define ALT_AVALON_CF_WRITE_CONTROL( __ctlr, __val )  \
  HAL_WRITE_UINT16(((__ctlr) + 14), __val) 

typedef struct ide_identify_data_t_ {        
    cyg_uint16 general_conf;         // 00    : general configuration   
    cyg_uint16 num_cylinders;        // 01    : number of cylinders (default CHS trans) 
    cyg_uint16 reserved0;            // 02    : reserved 
    cyg_uint16 num_heads;            // 03    : number of heads (default CHS trans) 
    cyg_uint16 num_ub_per_track;     // 04    : number of unformatted bytes per track 
    cyg_uint16 num_ub_per_sector;    // 05    : number of unformatted bytes per sector 
    cyg_uint16 num_sectors;          // 06    : number of sectors per track (default CHS trans) 
    cyg_uint16 num_card_sectors[2];  // 07-08 : number of sectors per card 
    cyg_uint16 reserved1;            // 09    : reserved 
    cyg_uint16 serial[10];           // 10-19 : serial number (string) 
    cyg_uint16 buffer_type;          // 20    : buffer type (dual ported) 
    cyg_uint16 buffer_size;          // 21    : buffer size in 512 increments 
    cyg_uint16 num_ECC_bytes;        // 22    : number of ECC bytes passed on R/W Long cmds 
    cyg_uint16 firmware_rev[4];      // 23-26 : firmware revision (string)
    cyg_uint16 model_num[20];        // 27-46 : model number (string)
    cyg_uint16 rw_mult_support;      // 47    : max number of sectors on R/W multiple cmds
    cyg_uint16 reserved2;            // 48    : reserved 
    cyg_uint16 capabilities;         // 49    : LBA, DMA, IORDY support indicator 
    cyg_uint16 reserved3;            // 50    : reserved 
    cyg_uint16 pio_xferc_timing;     // 51    : PIO data transfer cycle timing mode 
    cyg_uint16 dma_xferc_timing;     // 52    : single word DMA data transfer cycle timing mode 
    cyg_uint16 cur_field_validity;   // 53    : words 54-58 validity (0 == not valid) 
    cyg_uint16 cur_cylinders;        // 54    : number of current cylinders 
    cyg_uint16 cur_heads;            // 55    : number of current heads 
    cyg_uint16 cur_spt;              // 56    : number of current sectors per track 
    cyg_uint16 cur_capacity[2];      // 57-58 : current capacity in sectors 
    cyg_uint16 mult_sectors;         // 59    : multiple sector setting 
    cyg_uint16 lba_total_sectors[2]; // 60-61 : total sectors in LBA mode 
    cyg_uint16 sw_dma;               // 62    : single word DMA support 
    cyg_uint16 mw_dma;               // 63    : multi word DMA support 
    cyg_uint16 apio_modes;           // 64    : advanced PIO transfer mode supported 
    cyg_uint16 min_dma_timing;       // 65    : minimum multiword DMA transfer cycle 
    cyg_uint16 rec_dma_timing;       // 66    : recommended multiword DMA cycle 
    cyg_uint16 min_pio_timing;       // 67    : min PIO transfer time without flow control 
    cyg_uint16 min_pio_iordy_timing; // 68    : min PIO transfer time with IORDY flow control 
//  cyg_uint16 reserved4[187];       // 69-255: reserved 
} ide_identify_data_t;

// ----------------------------------------------------------------------------

struct alt_avalon_cf_interface;

typedef struct {
  cyg_uint32* ide_base;
  cyg_uint8   chan;
  struct alt_avalon_cf_interface* interface;
} alt_avalon_cf_info_t;

// ----------------------------------------------------------------------------

#define ALT_CF_NCHANNELS (2)

typedef struct alt_avalon_cf_interface 
{
  int                     irq;
  int                     init_done;
  cyg_uint32*             ctl_base;
  cyg_interrupt           cf_interrupt;
  cyg_handle_t            cf_interrupt_handle;
  struct cyg_devtab_entry *dev[ALT_CF_NCHANNELS];
} alt_avalon_cf_interface_t;

extern disk_funs alt_avalon_cf_funs;

extern cyg_bool 
alt_avalon_cf_init(struct cyg_devtab_entry *tab);

extern Cyg_ErrNo 
alt_avalon_cf_lookup(struct cyg_devtab_entry **tab, 
		     struct cyg_devtab_entry  *sub_tab,
		     const char *name);
// ----------------------------------------------------------------------------

#endif // CYGONCE_ALTERA_AVALON_CF_PRIV_H
