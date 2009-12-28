//===========================================================================
//
// MLT linker script for Nios II
//
//===========================================================================
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
//===========================================================================

#include <pkgconf/system.h>
#include <pkgconf/hal_nios2.h>
#define __ASSEMBLER__
#include <pkgconf/mlt_nios2.h>
#undef __ASSEMBLER__

STARTUP(vectors.o)
#ifdef EXTRAS
INPUT(extras.o)
#endif
#if (__GNUC__ >= 3)
GROUP(libtarget.a libgcc.a libsupc++.a)
#else
GROUP(libtarget.a libgcc.a)
#endif

OUTPUT_FORMAT( "elf32-littlenios2",
               "elf32-littlenios2",
               "elf32-littlenios2" )

ENTRY( _start )

#define ALIGN_LMA 8
#define FOLLOWING(_section_) AT ((LOADADDR (_section_) + SIZEOF (_section_) + ALIGN_LMA - 1) & ~ (ALIGN_LMA - 1))
#define LMA_EQ_VMA
#define CURRENT_LMA
#define FORCE_OUTPUT . = .

#define SECTIONS_BEGIN

#define SECTIONS_entry(_region_, _vma_, _lma_) \
    .entry _vma_ : _lma_                       \
    {                                          \
        KEEP (*(.entry));                      \
    } > _region_

#ifdef CYGSEM_HAL_USE_ROM_MONITOR

#define SECTIONS_exceptions(_region_,_vma_, _lma_)  \
    .rom_monitor_exceptions _vma_ (NOLOAD) : _lma_  \
    {                                               \
        . = ALIGN(0x20);                            \
        . += CYGNUM_ROM_MONITOR_EXCEPTIONS_LEN;     \
        . = ALIGN(32 / 8);                          \
    } > _region_

#else /* !CYGSEM_HAL_USE_ROM_MONITOR */

#define SECTIONS_exceptions(_region_,_vma_, _lma_)  \
    .exceptions _vma_ : _lma_                       \
    {                                               \
        PROVIDE (__ram_exc_start = ABSOLUTE(.));    \
        . = ALIGN(0x20);                            \
        KEEP(*(.irq));                              \
        KEEP (*(.exceptions));                      \
        PROVIDE (__ram_exc_end = ABSOLUTE(.));      \
    } > _region_                                    \
    PROVIDE (__rom_exc_start = LOADADDR(.exceptions));

#endif /* CYGSEM_HAL_USE_ROM_MONITOR */

#define SECTIONS_text(_vma_, _lma_)                             \
    .text _vma_ : _lma_                                         \
    {                                                           \
        PROVIDE (stext = ABSOLUTE(.));                          \
        PROVIDE (_stext = ABSOLUTE(.));                         \
                                                                \
        *(.interp);                                             \
        *(.hash);                                               \
        *(.dynsym);                                             \
        *(.dynstr);                                             \
        *(.gnu.version);                                        \
        *(.gnu.version_d);                                      \
        *(.gnu.version_r);                                      \
        *(.rel.init);                                           \
        *(.rela.init);                                          \
        *(.rel.text .rel.text.* .rel.gnu.linkonce.t.*);         \
        *(.rela.text .rela.text.* .rela.gnu.linkonce.t.*);      \
        *(.rel.fini);                                           \
        *(.rela.fini);                                          \
        *(.rel.rodata .rel.rodata.* .rel.gnu.linkonce.r.*);     \
        *(.rela.rodata .rela.rodata.* .rela.gnu.linkonce.r.*);  \
        *(.rel.data .rel.data.* .rel.gnu.linkonce.d.*);         \
        *(.rela.data .rela.data.* .rela.gnu.linkonce.d.*);      \
        *(.rel.tdata .rel.tdata.* .rel.gnu.linkonce.td.*);      \
        *(.rela.tdata .rela.tdata.* .rela.gnu.linkonce.td.*);   \
        *(.rel.tbss .rel.tbss.* .rel.gnu.linkonce.tb.*);        \
        *(.rela.tbss .rela.tbss.* .rela.gnu.linkonce.tb.*);     \
        *(.rel.ctors);                                          \
        *(.rela.ctors);                                         \
        *(.rel.dtors);                                          \
        *(.rela.dtors);                                         \
        *(.rel.got);                                            \
        *(.rela.got);                                           \
        *(.rel.sdata .rel.sdata.* .rel.gnu.linkonce.s.*);       \
        *(.rela.sdata .rela.sdata.* .rela.gnu.linkonce.s.*);    \
        *(.rel.sbss .rel.sbss.* .rel.gnu.linkonce.sb.*);        \
        *(.rela.sbss .rela.sbss.* .rela.gnu.linkonce.sb.*);     \
        *(.rel.sdata2 .rel.sdata2.* .rel.gnu.linkonce.s2.*);    \
        *(.rela.sdata2 .rela.sdata2.* .rela.gnu.linkonce.s2.*); \
        *(.rel.sbss2 .rel.sbss2.* .rel.gnu.linkonce.sb2.*);     \
        *(.rela.sbss2 .rela.sbss2.* .rela.gnu.linkonce.sb2.*);  \
        *(.rel.bss .rel.bss.* .rel.gnu.linkonce.b.*);           \
        *(.rela.bss .rela.bss.* .rela.gnu.linkonce.b.*);        \
        *(.rel.plt);                                            \
        *(.rela.plt);                                           \
                                                                \
        KEEP (*(.init));                                        \
        *(.plt);                                                \
        *(.text .stub .text.* .gnu.linkonce.t.*);               \
        *(.gnu.warning.*);                                      \
        KEEP (*(.fini));                                        \
        PROVIDE (__etext = ABSOLUTE(.));                        \
        PROVIDE (_etext = ABSOLUTE(.));                         \
        PROVIDE (etext = ABSOLUTE(.));                          \
                                                                \
        *(.eh_frame_hdr);                                       \
        . = ALIGN(32 / 8);                                      \
        PROVIDE (__preinit_array_start = ABSOLUTE(.));          \
        *(.preinit_array);                                      \
        PROVIDE (__preinit_array_end = ABSOLUTE(.));            \
        PROVIDE (__init_array_start = ABSOLUTE(.));             \
        *(.init_array);                                         \
        PROVIDE (__init_array_end = ABSOLUTE(.));               \
        PROVIDE (__fini_array_start = ABSOLUTE(.));             \
        *(.fini_array);                                         \
        PROVIDE (__fini_array_end = ABSOLUTE(.));               \
        SORT(CONSTRUCTORS)                                      \
        KEEP (*(.eh_frame));                                    \
        *(.gcc_except_table);                                   \
        *(.dynamic);                                            \
        PROVIDE (__CTOR_LIST__ = ABSOLUTE(.));                  \
        KEEP (*(.ctors))                                        \
        KEEP (*(SORT(.ctors.*)));                               \
        PROVIDE (__CTOR_END__ = ABSOLUTE(.));                   \
        PROVIDE (__DTOR_LIST__ = ABSOLUTE(.));                  \
        KEEP (*(.dtors))                                        \
        KEEP (*(SORT(.dtors.*)));                               \
        PROVIDE (__DTOR_END__ = ABSOLUTE(.));                   \
         KEEP (*(.jcr));                                        \
    } > CYGHWR_HAL_NIOS2_TEXT_REGION =0x0001883a

#define SECTIONS_rodata(_vma_, _lma_)                \
    .rodata _vma_ : _lma_                            \
    {                                                \
        . = ALIGN(32 / 8);                           \
        *(.rodata .rodata.* .gnu.linkonce.r.*);      \
        *(.rodata1);                                 \
    } > CYGHWR_HAL_NIOS2_RODATA_REGION

#define SECTIONS_rwdata(_vma_, _lma_)                \
    .rwdata _vma_ : _lma_                            \
    {                                                \
        PROVIDE (__ram_data_start = ABSOLUTE(.));    \
        . = ALIGN(32 / 8);                           \
        *(.got.plt) *(.got);                         \
        *(.data1);                                   \
        *(.2ram.*);                                  \
        *(.data .data.* .gnu.linkonce.d.*);          \
       . = ALIGN (0x4);                              \
        KEEP(*( SORT (.ecos.table.*)));              \
       . = ALIGN (0x4);                              \
                                                     \
        _gp = ABSOLUTE(. + 0x7ff0);                  \
        PROVIDE(gp = _gp);                           \
                                                     \
        *(.sdata .sdata.* .gnu.linkonce.s.*);        \
        *(.sdata2 .sdata2.* .gnu.linkonce.s2.*);     \
        . = ALIGN(32 / 8);                           \
        _edata = ABSOLUTE(.);                        \
        PROVIDE (edata = ABSOLUTE(.));               \
        PROVIDE (__ram_data_end = ABSOLUTE(.));      \
    } > CYGHWR_HAL_NIOS2_RWDATA_REGION               \
    PROVIDE (__rom_data_start = LOADADDR(.rwdata));

#define SECTIONS_bss(_vma_, _lma_)                \
    .bss _vma_ : _lma_                            \
    {                                             \
        __bss_start = ABSOLUTE(.);                \
        PROVIDE (__sbss_start = ABSOLUTE(.));     \
        PROVIDE (___sbss_start = ABSOLUTE(.));    \
                                                  \
        *(.dynsbss);                              \
        *(.sbss .sbss.* .gnu.linkonce.sb.*);      \
        *(.sbss2 .sbss2.* .gnu.linkonce.sb2.*);   \
        *(.scommon);                              \
                                                  \
        PROVIDE (__sbss_end = ABSOLUTE(.));       \
        PROVIDE (___sbss_end = ABSOLUTE(.));      \
                                                  \
        *(.dynbss);                               \
        *(.bss .bss.* .gnu.linkonce.b.*);         \
        *(COMMON);                                \
                                                  \
        . = ALIGN(32 / 8);                        \
        PROVIDE (__bss_end = ABSOLUTE(.));        \
        PROVIDE (__end   = ABSOLUTE(.));          \
        PROVIDE (end     = ABSOLUTE(.));          \
    } > CYGHWR_HAL_NIOS2_RWDATA_REGION

#define SECTIONS_END                                               \
    .stab          0 : { *(.stab) }                                \
    .stabstr       0 : { *(.stabstr) }                             \
    .stab.excl     0 : { *(.stab.excl) }                           \
    .stab.exclstr  0 : { *(.stab.exclstr) }                        \
    .stab.index    0 : { *(.stab.index) }                          \
    .stab.indexstr 0 : { *(.stab.indexstr) }                       \
    .comment       0 : { *(.comment) }                             \
    .debug          0 : { *(.debug) }                              \
    .line           0 : { *(.line) }                               \
    .debug_srcinfo  0 : { *(.debug_srcinfo) }                      \
    .debug_sfnames  0 : { *(.debug_sfnames) }                      \
    .debug_aranges  0 : { *(.debug_aranges) }                      \
    .debug_pubnames 0 : { *(.debug_pubnames) }                     \
    .debug_info     0 : { *(.debug_info .gnu.linkonce.wi.*) }      \
    .debug_abbrev   0 : { *(.debug_abbrev) }                       \
    .debug_line     0 : { *(.debug_line) }                         \
    .debug_frame    0 : { *(.debug_frame) }                        \
    .debug_str      0 : { *(.debug_str) }                          \
    .debug_loc      0 : { *(.debug_loc) }                          \
    .debug_macinfo  0 : { *(.debug_macinfo) }                      \
    .debug_weaknames 0 : { *(.debug_weaknames) }                   \
    .debug_funcnames 0 : { *(.debug_funcnames) }                   \
    .debug_typenames 0 : { *(.debug_typenames) }                   \
    .debug_varnames  0 : { *(.debug_varnames) }

#ifdef CYGSEM_HAL_USE_ROM_MONITOR

#define SECTIONS_rom_monitor_text(_vma_, _lma_)                    \
    .rom_monitor_text _vma_ (NOLOAD) : _lma_                       \
    {                                                              \
        . += CYGNUM_ROM_MONITOR_TEXT_LEN;                          \
        . = ALIGN(32 / 8);                                         \
    } > CYGSEM_ROM_MONITOR_TEXT_REGION

#define SECTIONS_rom_monitor_rodata(_vma_, _lma_)                  \
    .rom_monitor_rodata _vma_ (NOLOAD) : _lma_                     \
    {                                                              \
        . += CYGNUM_ROM_MONITOR_RODATA_LEN;                        \
        . = ALIGN(32 / 8);                                         \
    } > CYGSEM_ROM_MONITOR_RODATA_REGION

#define SECTIONS_rom_monitor_rwdata(_vma_, _lma_)                  \
    .rom_monitor_rwdata _vma_ (NOLOAD) : _lma_                     \
    {                                                              \
		. = . + CYGBLD_RAM_OFFSET;                   \
        . = ALIGN(32 / 8);                                         \
        PROVIDE (hal_virtual_vector_table = ABSOLUTE(.));          \
        . += 256;                                                  \
        PROVIDE (hal_vsr_table = ABSOLUTE(.));	                   \
        . += CYGNUM_ROM_MONITOR_RWDATA_LEN;                        \
        . = ALIGN(32 / 8);                                         \
    } > CYGSEM_ROM_MONITOR_RWDATA_REGION

#else /* !CYGSEM_HAL_USE_ROM_MONITOR */

#define SECTIONS_rom_monitor_text(_vma_, _lma_) 
#define SECTIONS_rom_monitor_rodata(_vma_, _lma_)       
#define SECTIONS_rom_monitor_rwdata(_vma_, _lma_)                  \
    .rom_monitor_rwdata _vma_ : _lma_                              \
    {                                                              \
		. = . + CYGBLD_RAM_OFFSET;                   \
        . = ALIGN(32 / 8);                                         \
        PROVIDE (hal_virtual_vector_table = ABSOLUTE(.));          \
        . += 256;                                                  \
        PROVIDE (hal_vsr_table = ABSOLUTE(.));                     \
        . += 8;                                                    \
    } > CYGHWR_HAL_NIOS2_RWDATA_REGION

#endif /* CYGSEM_HAL_USE_ROM_MONITOR */

#include CYGHWR_MEMORY_LAYOUT_LDI


