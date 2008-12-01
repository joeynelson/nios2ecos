#ifndef CYGONCE_ALTERA_AVALON_SGDMA_H
#define CYGONCE_ALTERA_AVALON_SGDMA_H

/******************************************************************************
*                                                                             *
* License Agreement                                                           *
*                                                                             *
* Copyright (c) 2006 Altera Corporation, San Jose, California, USA.           *
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
*                                                                             *
******************************************************************************/

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#include <cyg/infra/diag.h>
#include <cyg/infra/cyg_type.h>
#include <cyg/hal/hal_if.h>
#include <cyg/hal/hal_misc.h>
#include <cyg/hal/drv_api.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_io.h>

#ifdef CYGPKG_ALTERA_AVALON_SGDMA

#include <cyg/io/devtab.h>
#include <cyg/io/io.h>
#include <cyg/hal/hal_intr.h>


/* Each Scatter-gather DMA buffer descriptor spans 0x20 of memory */
#define ALTERA_AVALON_SGDMA_DESCRIPTOR_SIZE (0x20)

#define IOADDR_ALTERA_AVALON_SGDMA_STATUS(base)       __IO_CALC_ADDRESS_DYNAMIC(base, 0)
#define IORD_ALTERA_AVALON_SGDMA_STATUS(base)         IORD(base, 0)
#define IOWR_ALTERA_AVALON_SGDMA_STATUS(base, data)   IOWR(base, 0, data)

#define ALTERA_AVALON_SGDMA_STATUS_ERROR_MSK           (0x1)
#define ALTERA_AVALON_SGDMA_STATUS_ERROR_OFST          (0)
#define ALTERA_AVALON_SGDMA_STATUS_EOP_ENCOUNTERED_MSK           (0x2)
#define ALTERA_AVALON_SGDMA_STATUS_EOP_ENCOUNTERED_OFST          (1)
#define ALTERA_AVALON_SGDMA_STATUS_DESC_COMPLETED_MSK           (0x4)
#define ALTERA_AVALON_SGDMA_STATUS_DESC_COMPLETED_OFST          (2)
#define ALTERA_AVALON_SGDMA_STATUS_CHAIN_COMPLETED_MSK           (0x8)
#define ALTERA_AVALON_SGDMA_STATUS_CHAIN_COMPLETED_OFST          (3)
#define ALTERA_AVALON_SGDMA_STATUS_BUSY_MSK            (0x10)
#define ALTERA_AVALON_SGDMA_STATUS_BUSY_OFST           (4)

#define IOADDR_ALTERA_AVALON_SGDMA_CONTROL(base)     __IO_CALC_ADDRESS_DYNAMIC(base, 4)
#define IORD_ALTERA_AVALON_SGDMA_CONTROL(base)        IORD(base, 4)
#define IOWR_ALTERA_AVALON_SGDMA_CONTROL(base, data)  IOWR(base, 4, data)
#define ALTERA_AVALON_SGDMA_CONTROL_IE_ERROR_MSK  (0x1)
#define ALTERA_AVALON_SGDMA_CONTROL_IE_ERROR_OFST  (0)
#define ALTERA_AVALON_SGDMA_CONTROL_IE_EOP_ENCOUNTERED_MSK  (0x2)
#define ALTERA_AVALON_SGDMA_CONTROL_IE_EOP_ENCOUNTERED_OFST  (1)
#define ALTERA_AVALON_SGDMA_CONTROL_IE_DESC_COMPLETED_MSK  (0x4)
#define ALTERA_AVALON_SGDMA_CONTROL_IE_DESC_COMPLETED_OFST  (2)
#define ALTERA_AVALON_SGDMA_CONTROL_IE_CHAIN_COMPLETED_MSK  (0x8)
#define ALTERA_AVALON_SGDMA_CONTROL_IE_CHAIN_COMPLETED_OFST  (3)
#define ALTERA_AVALON_SGDMA_CONTROL_IE_GLOBAL_MSK  (0x10)
#define ALTERA_AVALON_SGDMA_CONTROL_IE_GLOBAL_OFST  (4)
#define ALTERA_AVALON_SGDMA_CONTROL_RUN_MSK  (0x20)
#define ALTERA_AVALON_SGDMA_CONTROL_RUN_OFST  (5)
#define ALTERA_AVALON_SGDMA_CONTROL_STOP_DMA_ER_MSK  (0x40)
#define ALTERA_AVALON_SGDMA_CONTROL_STOP_DMA_ER_OFST  (6)
#define ALTERA_AVALON_SGDMA_CONTROL_IE_MAX_DESC_PROCESSED_MSK  (0x80)
#define ALTERA_AVALON_SGDMA_CONTROL_IE_MAX_DESC_PROCESSED_OFST  (7)
#define ALTERA_AVALON_SGDMA_CONTROL_MAX_DESC_PROCESSED_MSK  (0xFF00)
#define ALTERA_AVALON_SGDMA_CONTROL_MAX_DESC_PROCESSED_OFST  (8)
#define ALTERA_AVALON_SGDMA_CONTROL_SOFTWARERESET_MSK (0X10000)
#define ALTERA_AVALON_SGDMA_CONTROL_SOFTWARERESET_OFST (16)
#define ALTERA_AVALON_SGDMA_CONTROL_PARK_MSK (0X20000)
#define ALTERA_AVALON_SGDMA_CONTROL_PARK_OFST (17)
#define ALTERA_AVALON_SGDMA_CONTROL_CLEAR_INTERRUPT_MSK (0X80000000)
#define ALTERA_AVALON_SGDMA_CONTROL_CLEAR_INTERRUPT_OFST (31)

#define IOADDR_ALTERA_AVALON_SGDMA_NEXT_DESC_POINTER(base)     __IO_CALC_ADDRESS_DYNAMIC(base, 8)
#define IORD_ALTERA_AVALON_SGDMA_NEXT_DESC_POINTER(base)        IORD(base, 8)
#define IOWR_ALTERA_AVALON_SGDMA_NEXT_DESC_POINTER(base, data)  IOWR(base, 8, data)



/*
 * Descriptor control bit masks & offsets
 *
 * Note: The control byte physically occupies bits [31:24] in memory.
 *       The following bit-offsets are expressed relative to the LSB of
 *       the control register bitfield.
 */
#define ALTERA_AVALON_SGDMA_DESCRIPTOR_CONTROL_GENERATE_EOP_MSK (0x1)
#define ALTERA_AVALON_SGDMA_DESCRIPTOR_CONTROL_GENERATE_EOP_OFST (0)
#define ALTERA_AVALON_SGDMA_DESCRIPTOR_CONTROL_READ_FIXED_ADDRESS_MSK (0x2)
#define ALTERA_AVALON_SGDMA_DESCRIPTOR_CONTROL_READ_FIXED_ADDRESS_OFST (1)
#define ALTERA_AVALON_SGDMA_DESCRIPTOR_CONTROL_WRITE_FIXED_ADDRESS_MSK (0x4)
#define ALTERA_AVALON_SGDMA_DESCRIPTOR_CONTROL_WRITE_FIXED_ADDRESS_OFST (2)
#define ALTERA_AVALON_SGDMA_DESCRIPTOR_CONTROL_ATLANTIC_CHANNEL_MSK (0x8)
#define ALTERA_AVALON_SGDMA_DESCRIPTOR_CONTROL_ATLANTIC_CHANNEL_OFST (3)
#define ALTERA_AVALON_SGDMA_DESCRIPTOR_CONTROL_OWNED_BY_HW_MSK (0x80)
#define ALTERA_AVALON_SGDMA_DESCRIPTOR_CONTROL_OWNED_BY_HW_OFST (7)

/*
 * Descriptor status bit masks & offsets
 *
 * Note: The status byte physically occupies bits [23:16] in memory.
 *       The following bit-offsets are expressed relative to the LSB of
 *       the status register bitfield.
 */
#define ALTERA_AVALON_SGDMA_DESCRIPTOR_STATUS_E_CRC_MSK (0x1)
#define ALTERA_AVALON_SGDMA_DESCRIPTOR_STATUS_E_CRC_OFST (0)
#define ALTERA_AVALON_SGDMA_DESCRIPTOR_STATUS_E_PARITY_MSK (0x2)
#define ALTERA_AVALON_SGDMA_DESCRIPTOR_STATUS_E_PARITY_OFST (1)
#define ALTERA_AVALON_SGDMA_DESCRIPTOR_STATUS_E_OVERFLOW_MSK (0x4)
#define ALTERA_AVALON_SGDMA_DESCRIPTOR_STATUS_E_OVERFLOW_OFST (2)
#define ALTERA_AVALON_SGDMA_DESCRIPTOR_STATUS_E_SYNC_MSK (0x8)
#define ALTERA_AVALON_SGDMA_DESCRIPTOR_STATUS_E_SYNC_OFST (3)
#define ALTERA_AVALON_SGDMA_DESCRIPTOR_STATUS_E_UEOP_MSK (0x10)
#define ALTERA_AVALON_SGDMA_DESCRIPTOR_STATUS_E_UEOP_OFST (4)
#define ALTERA_AVALON_SGDMA_DESCRIPTOR_STATUS_E_MEOP_MSK (0x20)
#define ALTERA_AVALON_SGDMA_DESCRIPTOR_STATUS_E_MEOP_OFST (5)
#define ALTERA_AVALON_SGDMA_DESCRIPTOR_STATUS_E_MSOP_MSK (0x40)
#define ALTERA_AVALON_SGDMA_DESCRIPTOR_STATUS_E_MSOP_OFST (6)
#define ALTERA_AVALON_SGDMA_DESCRIPTOR_STATUS_TERMINATED_BY_EOP_MSK (0x80)
#define ALTERA_AVALON_SGDMA_DESCRIPTOR_STATUS_TERMINATED_BY_EOP_OFST (7)

#define ALTERA_AVALON_SGDMA_CONTROL_SOFTWARERESET_MSK (0X10000)
#define ALTERA_AVALON_SGDMA_STATUS_BUSY_MSK            (0x10)

/*
 * To ensure that a descriptor is created without spaces
 * between the struct members, we call upon GCC's ability
 * to pack to a byte-aligned boundary.
 */
#define alt_avalon_sgdma_packed __attribute__ ((packed,aligned(1)))

/*
 * Buffer Descriptor data structure
 *
 * The SGDMA controller buffer descriptor allocates
 * 64 bits for each address. To support ANSI C, the
 * struct implementing a descriptor places 32-bits
 * of padding directly above each address; each pad must
 * be cleared when initializing a descriptor.
 */
typedef struct {
    cyg_uint32   *read_addr;
    cyg_uint32   read_addr_pad;

    cyg_uint32   *write_addr;
    cyg_uint32   write_addr_pad;

    cyg_uint32   *next;
    cyg_uint32   next_pad;
    
    cyg_uint16   bytes_to_transfer;
    cyg_uint8    read_burst;
    cyg_uint8    write_burst;

    cyg_uint16   actual_bytes_transferred;
    cyg_uint8    status;
    cyg_uint8    control;

} alt_avalon_sgdma_packed alt_sgdma_descriptor;




/* Callback routine type definition */
typedef void (*alt_avalon_sgdma_callback)(void *context);


/* SGDMA Device Structure */
typedef struct 
{
  CYG_ADDRWORD              base;               // Base address of SGDMA
  cyg_uint32          		irq;                 /* Interrupt number */
  const char                *name;               // Name of SGDMA in SOPC System
  cyg_interrupt       		sgdma_interrupt;
  cyg_handle_t        		sgdma_interrupt_handle;
  cyg_ISR_t                 *isr;
  cyg_DSR_t                 *dsr;
  cyg_addrword_t             isr_data;	
  cyg_uint32                *descriptor_base;    // reserved
  cyg_uint32                next_index;          // reserved
  cyg_uint32                num_descriptors;     // reserved
  alt_sgdma_descriptor      *current_descriptor; // reserved
  alt_sgdma_descriptor      *next_descriptor;    // reserved
  alt_avalon_sgdma_callback callback;            // Callback routine pointer
  void                      *callback_context;   // Callback context pointer
  cyg_uint32                chain_control;       // Value OR'd into control reg
} alt_sgdma_dev;

/*
 * Flags that can be set in the flags field of the above structure.
 */ 

/*
 * The function alt_find_dev() is used to search the device list "list" to
 * locate a device named "name". If a match is found, then a pointer to the
 * device is returned, otherwise NULL is returned.
 */
//extern alt_dev* alt_find_dev (const char* name, alt_llist* list);




/*******************************************************************************
 *  Public API
 ******************************************************************************/

/* API for "application managed" operation */
int alt_avalon_sgdma_do_async_transfer(
  alt_sgdma_dev *dev,
  alt_sgdma_descriptor *desc);

cyg_uint8 alt_avalon_sgdma_do_sync_transfer(
  alt_sgdma_dev *dev,
  alt_sgdma_descriptor *desc);

void alt_avalon_sgdma_construct_mem_to_mem_desc(
  alt_sgdma_descriptor *desc,
  alt_sgdma_descriptor *next,
  cyg_uint32              *read_addr,
  cyg_uint32              *write_addr,
  cyg_uint16               length,
  int                   read_fixed,
  int                   write_fixed);

void alt_avalon_sgdma_construct_stream_to_mem_desc(
  alt_sgdma_descriptor *desc,
  alt_sgdma_descriptor *next,
  cyg_uint32              *write_addr,
  cyg_uint16               length_or_eop,
  int                   write_fixed);

void alt_avalon_sgdma_construct_mem_to_stream_desc(
  alt_sgdma_descriptor *desc,
  alt_sgdma_descriptor *next,
  cyg_uint32              *read_addr,
  cyg_uint16               length,
  int                   read_fixed,
  int                   generate_sop,
  int                   generate_eop,
  cyg_uint8                atlantic_channel);

void alt_avalon_sgdma_register_callback(
  alt_sgdma_dev *dev,
  alt_avalon_sgdma_callback callback,
  cyg_uint32 chain_control,
  void *context);

void alt_avalon_sgdma_start(
  alt_sgdma_dev *dev);

void alt_avalon_sgdma_stop(
  alt_sgdma_dev *dev);

void alt_avalon_sgdma_park( 
  alt_sgdma_dev *dev);

cyg_uint32 alt_avalon_sgdma_status( 
  alt_sgdma_dev *dev);

int alt_avalon_sgdma_check_descriptor_status( 
  alt_sgdma_descriptor *desc);

alt_sgdma_dev* alt_avalon_sgdma_open (const char* name);

void alt_avalon_sgdma_validate_descriptor_loop_for_parking (alt_sgdma_descriptor *desc);

void alt_avalon_sgdma_reset (alt_sgdma_dev *dev);


/* Private API */
void alt_avalon_sgdma_construct_descriptor(
  alt_sgdma_descriptor *desc,
  alt_sgdma_descriptor *next,
  cyg_uint32              *read_addr,
  cyg_uint32              *write_addr,
  cyg_uint16               length_or_eop,
  int                   generate_eop,
  int                   read_fixed,
  int                   write_fixed_or_sop,
  cyg_uint8                atlantic_channel);

void alt_avalon_sgdma_init (alt_sgdma_dev *dev);
void alt_avalon_sgdma_init2 (alt_sgdma_dev *dev);
cyg_uint32 alt_avalon_sgdma_descpt_bytes_xfered(alt_sgdma_descriptor *desc);
//cyg_uint32 alt_avalon_sgdma_statuuget_control( alt_sgdma_dev *dev);
cyg_uint32 alt_avalon_sgdma_get_control( alt_sgdma_dev *dev);
void       alt_avalon_sgdma_set_control( alt_sgdma_dev *dev, cyg_uint32 val);


void alt_avalon_sgdma_manage_circular_buffer(alt_sgdma_dev *dev);
int alt_avalon_sgdma_check_desc_chain(alt_sgdma_dev *dev);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CYGPKG_ALTERA_AVALON_SGDMA */

#endif /* __ALTERA_AVALON_SGDMA_H__ */
