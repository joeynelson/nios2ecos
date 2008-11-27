#ifndef CYGONCE_NIOS2_OPCODE_H
#define CYGONCE_NIOS2_OPCODE_H

//=============================================================================
//
//      nios2_opcode.h
//
//      Opcodes for Nios II.
//
//=============================================================================
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
//=============================================================================

// Provide structures and defines used for decoding Nios II instructions

// Define the instruction formats.

typedef union {
    unsigned word;

    struct {
        unsigned op: 6;
        unsigned imm: 16;
        unsigned b: 5;
        unsigned a: 5;
    } IType;

    struct {
        unsigned op: 6;
        unsigned blank: 5;
        unsigned opx: 6;
        unsigned c: 5;
        unsigned b: 5;
        unsigned a: 5;
    } RType;

    struct {
        unsigned op: 6;
        unsigned imm: 26;
    } JType;
} InstFmt;

//Values for the 'op' field.

#define OP_ADDI         0x04
#define OP_ANDHI        0x2c
#define OP_ANDI         0x0c
#define OP_BEQ          0x26
#define OP_BGE          0x0e
#define OP_BGEU         0x2e
#define OP_BLT          0x16
#define OP_BLTU         0x36
#define OP_BNE          0x1e
#define OP_BR           0x06
#define OP_CALL         0x00
#define OP_CMPEQI       0x20
#define OP_CMPGEI       0x08
#define OP_CMPGEUI      0x28
#define OP_CMPLTI       0x10
#define OP_CMPLTUI      0x30
#define OP_CMPNEI       0x18
#define OP_FLUSHD       0x3b
#define OP_LDB          0x07
#define OP_LDBIO        0x27
#define OP_LDBU         0x03
#define OP_LDBUIO       0x23
#define OP_LDH          0x0f
#define OP_LDHIO        0x2f
#define OP_LDHU         0x0b
#define OP_LDHUIO       0x2b
#define OP_LDW          0x17
#define OP_LDWIO        0x37
#define OP_MULI         0x24
#define OP_ORHI         0x34
#define OP_ORI          0x14
#define OP_STB          0x05
#define OP_STBIO        0x25
#define OP_STH          0x0d
#define OP_STHIO        0x2d
#define OP_STW          0x15
#define OP_STWIO        0x35
#define OP_USR          0x32
#define OP_XORHI        0x3c
#define OP_XORI         0x1c
#define OP_RTYPE        0x3a

// Values of the 'opx' field for when the opcode = OP_RTYPE 

#define OPX_ERET        0x01
#define OPX_ROLI        0x02
#define OPX_ROL         0x03
#define OPX_FLUSHP      0x04
#define OPX_RET         0x05
#define OPX_NOR         0x06
#define OPX_MULXUU      0x07
#define OPX_CMPGE       0x08
#define OPX_BRET        0x09
#define OPX_ROR         0x0b
#define OPX_FLUSHI      0x0c
#define OPX_JMP         0x0d
#define OPX_AND         0x0e
#define OPX_CMPLT       0x10
#define OPX_SLLI        0x12
#define OPX_SLL         0x13
#define OPX_OR          0x16
#define OPX_MULXSU      0x17
#define OPX_CMPNE       0x18
#define OPX_SRLI        0x1a
#define OPX_SRL         0x1b
#define OPX_NEXTPC      0x1c
#define OPX_CALLR       0x1d
#define OPX_XOR         0x1e
#define OPX_MULXSS      0x1f
#define OPX_CMPEQ       0x20
#define OPX_DIVU        0x24
#define OPX_DIV         0x25
#define OPX_RDCTL       0x26
#define OPX_MUL         0x27
#define OPX_CMPGEU      0x28
#define OPX_TRAP        0x2d
#define OPX_WRCTL       0x2e
#define OPX_CMPLTU      0x30
#define OPX_ADD         0x31
#define OPX_BREAK       0x34
#define OPX_SYNC        0x36
#define OPX_SUB         0x39
#define OPX_SRAI        0x3a
#define OPX_SRA         0x3b

// Instructions with specal significance to debuggers.

#define BREAK_INSTR     0x005b603a      // instruction code for break (actually trap)
#define NOP_INSTR       0x0001883a      // instruction code for no-op 

#endif  // CYGONCE_NIOS2_OPCODE_H
