/*
 * Copyright (C) 2017 Mattes Besuden
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @brief       Provides instruction definitions used by the calculation VM. Instructions and instruction format refers to RFC 3320 SigComp.
 *
 * @author      Mattes Besuden <besuden@uni-bremen.de>
 */
#ifndef OPCODES_H_
#define OPCODES_H_

//Use RFC3320 as Opcode Reference (UDVM Instruction format)

//Operand Type can be uint32_t or decimal (floating point or fixed point) (fixpoint library https://github.com/manuelbua/fixedpoint-math)
//Addresses are 16Bit = 65536 Adressen/Byte
///Mask for OPTYPE
#define VM_OPTYPE_MASK			0x0e
///Operand is unsigned int (32Bit)
#define VM_OPERAND_TYPE_UINT32	0x00
///Operand is decimal number (decimal_t)
#define VM_OPERAND_TYPE_DEC		0x02
///Operand is unsigned int (16Bit), address
#define VM_OPERAND_TYPE_UINT8	0x04
///Operand is unsigned int (8Bit), status codes, flags, etc
#define VM_OPERAND_TYPE_UINT16	0x06

///Mask for Literal/Address
#define VM_ADDRESS_MASK			0x01
///Operand is at an address specified in the instruction
#define VM_ADDRESS				0x00
///Operand is directly coded in the instruction
#define VM_LITERAL				0x01

///ID coded inside OPTYPE (used for URL-Map and PID IDs)
#define VM_OPTYPE_ID(x)			(x >> 4)

//Instructions (see OPCODE Details for details)
///Addition
#define VM_INSTRUCTION_ADD					0x01
///Substraktion
#define VM_INSTRUCTION_SUB					0x02
///Multiplikation
#define VM_INSTRUCTION_MUL					0x03
///Division
#define VM_INSTRUCTION_DIV					0x04
///Modulus
#define VM_INSTRUCTION_MOD					0x05

///AND (not defined for decimal_t)
#define VM_INSTRUCTION_AND					0x10
///OR (not defined for decimal_t)
#define VM_INSTRUCTION_OR					0x11
///NOT (no Operand, not defined for decimal_t)
#define VM_INSTRUCTION_NOT					0x12
///XOR (not defined for decimal_t)
#define VM_INSTRUCTION_XOR					0x13
///Left Shift (not defined for decimal_t)
#define VM_INSTRUCTION_LSHIFT				0x14
///Rigth Shift (not defined for decimal_t)
#define VM_INSTRUCTION_RSHIFT				0x15

///Loads value into Memory
#define VM_INSTRUCTION_LOAD					0x20

///Loads n values into memory
#define	VM_INSTRUCTION_MULTILOAD			0x21

///Pushes value on Stack (not implemented)
#define VM_INSTRUCTION_PUSH					0x30
///Pops value from stack to memory (not implemented)
#define VM_INSTRUCTION_POP					0x31

///Copies LENGTH Bytes From SRC Address to DEST Address whereas LENGTH can be a literal or an address
#define VM_INSTRUCTION_COPY					0x40
//#define COPY_LITERAL		0x41
//#define COPY_OFFSET		0x42

///Moves Program execution to specified memory address
#define VM_INSTRUCTION_JUMP					0x51

///Compares first two operands and jumps to address1 if a < b, address2 if a == b, address3 if a > b
#define VM_INSTRUCTION_COMPARE				0x52

///Pushes next instruction counter on stack, moves program counter to value specified in address
#define VM_INSTRUCTION_CALL					0x53
///Pops value from stack and moves instruction counter to popped value (ONLY OPCODE REQUIRED)
#define VM_INSTRUCTION_RETURN				0x54

//#define SWITCH			0x55//conditional jump

///Stores CPU time (uint32_t) in specified memory address
#define VM_INSTRUCTION_TIME					0x60
///Compares CPU time with memory value (Address) if NOW - LAST > TIMEOUT Jump to Address else Jump to Address2
#define VM_INSTRUCTION_COMPARETIME			0x61

///Maps a Memory Address with an URL (uses map options defined in URL_Mapping.h)
#define VM_INSTRUCTION_URLMAP				0x70
///Checks if Mapped value was handled at least once
#define VM_INSTRUCTION_URLMAPCHECK			0x71
///Deletes URL Mapping
#define VM_INSTRUCTION_URLMAPDELETE			0x72

///Initializes PID x with parameters (doesn't start PID) can be used to change PID x (needs to be cleared first).
#define VM_INSTRUCTION_PIDINIT				0x80
///Deletes PID x
#define VM_INSTRUCTION_PIDCLEAR				0x82
///Stops PID x (set to MANUAL)
#define VM_INSTRUCTION_PIDSTOP				0x83
///Starts PID x (set to AUTOMATIC)
#define VM_INSTRUCTION_PIDRUN				0x84

//Only OPCODE instructions
///Stops execution, sets halt flag true
#define VM_INSTRUCTION_HALT					0x00
///Stops execution, clears stack (and memory), resets everything
#define VM_INSTRUCTION_RESET				0xff

#endif /* OPCODES_H_ */
