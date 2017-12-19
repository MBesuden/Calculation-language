/* Copyright (C) 2017 Mattes Besuden
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @brief       Implementation of the calculation language. Instructions refer to RFC 3320 SigComp.
 * 				Processes the bytecode stored in the shared memory.
 * @author      Mattes Besuden <besuden@uni-bremen.de>
 */
#ifndef CALCULATIONVM_H_
#define CALCULATIONVM_H_


extern "C" {
#include "xtimer.h"
}

#include "Opcodes.h"
#include "Stack.h"
#include "PID.h"
#include <stdexcept>


//Referenz https://github.com/DoubangoTelecom/libsigcomp/tree/master/libsigcomp/src

class VM
{
public:

	///Debug mode, on
	#define VM_DEBUG_ON						1
	///Debug mode, off
	#define VM_DEBUG_OFF					0

	///VM flag halted
	#define VM_FLAG_HALTED					0x01
	///VM flag error
	#define VM_FLAG_ERROR					0x02
	///VM flag divide by zero error
	#define VM_FLAG_DIVIDEZERO				0x04

	///VM error code mask
	#define VM_ERROR_MASK					0xff
	///VM error code suicide occurred
	#define VM_ERROR_RESET					0x01
	///VM error code divide by zero
	#define VM_ERROR_DIVIDEZERO				0x02
	///VM error code unsupported operand
	#define VM_ERROR_UNSUPPORTED_OPERAND	0x03
	///VM error code unsupported instruction
	#define VM_ERROR_UNSUPPORTED_OPERATION	0x04
	///VM error memory error
	#define VM_ERROR_MEMORY_EXCEPTION		0x05
	///VM error code ID unavailable, ID is out of range
	#define VM_ERROR_ID_UNAVAILABLE			0x06
	///VM error code PID initialized (can not initialize again, clear before reinitialize)
	#define VM_ERROR_PID_INIT				0x07


	VM(Memory* memory, PID* pids);
	~VM(void) { };


	void setProgram(uint8_t* _program, uint16_t _size);
	void executeStep(void);

	uint16_t getProgramcounter(void);

	uint8_t getFlags(void);

	Memory* getMemory(void);
	Stack* getStack(void);

	void setDebugMode(uint8_t mode);
	bool halted(void);
	bool errorFlag(void);
	uint32_t getStatuscode(void);

	void clear();

private:
	Memory* memory;
	PID* pids;
	Stack stack;

	///8Bit flags
	uint8_t flags;
	uint16_t programcounter;

	uint8_t debugmode;
	///16Bit programcounter, 8Bit Opcode, 8Bit Errorcode
	uint32_t statuscode;
	bool execution_error;

	//instruction
	bool handleADD(void);
	bool handleSUB(void);
	bool handleMUL(void);
	bool handleDIV(void);
	bool handleMOD(void);
	bool handleAND(void);
	bool handleOR(void);
	bool handleNOT(void);
	bool handleXOR(void);
	bool handleLSHIFT(void);
	bool handleRSHIFT(void);
	bool handleLOAD(void);
	bool handleMULTILOAD(void);
	bool handlePUSH(void);
	bool handlePOP(void);
	bool handleCOPY(void);
	bool handleJUMP(void);
	bool handleCOMPARE(void);
	bool handleCALL(void);
	bool handleRETURN(void);
	bool handleTIME(void);
	bool handleCOMPARETIME(void);
	bool handleURLMAP(void);
	bool handleURLMAPCHECK(void);
	bool handleURLMAPDELETE(void);
	bool handlePIDINIT(void);
	bool handlePIDCLEAR(void);
	bool handlePIDSTOP(void);
	bool handlePIDRUN(void);
	bool handleHALT(void);
	bool handleRESET(void);

	//Utility
	inline uint8_t get_optype(void);
	inline uint8_t get_number(void);
	inline uint16_t get_address(void);
	inline uint16_t get_operandaddress(uint8_t optype);
};

#endif /* CALCULATIONVM_H_ */
