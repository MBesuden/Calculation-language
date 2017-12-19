/*
 * Copyright (C) 2017 Mattes Besuden
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @brief       Implementation of CalculationVM.h
 *
 * @author      Mattes Besuden <besuden@uni-bremen.de>
 */
#include <stdio.h>
#include <math.h>

extern "C" {
#include "xtimer.h"
}
#include "CalculationVM.h"


/**
 * @brief Implementation of a calculation VM which processes a bytecode program in the memory.
 * @param memory The memory on which to operate.
 * @param pids	The PID controller which to use.
 */
VM::VM(Memory* memory, PID* pids)
{
	this->memory = memory;
	this->pids = pids;
	flags = 0;
	programcounter = 0;

	debugmode = VM_DEBUG_OFF;
	statuscode = 0;
	execution_error = false;
}

/**
 * Stores Bytecode in Memory and cleares VM.
 * @param _program Bytecode program.
 * @param _size	Size of the program.
 */
void VM::setProgram(uint8_t* _program, uint16_t _size)
{
	if(_program)
	{
		for(uint16_t i = 0; i < _size; i++) {
			memory->store(i, _program[i]);
		}
	}
	VM::clear();
}

/**
 * @brief executes exactly one instruction of the program bytecode. Halts the machine if an error occurs. Instructions are defined in Opcodes.h.
 */
void VM::executeStep()
{
	//one step per execution
	if(halted())
	{
		return;
	}
	try {
		uint8_t currentop = memory->load(programcounter);
		statuscode = 0 | (programcounter << 16);//code programcounter in statuscode
		statuscode |= (currentop << 8);//code currentop in statuscode
		switch(currentop)
		{
		case VM_INSTRUCTION_ADD:			execution_error = !this->handleADD();			programcounter++;	break;
		case VM_INSTRUCTION_SUB:			execution_error = !this->handleSUB();			programcounter++;	break;
		case VM_INSTRUCTION_MUL:			execution_error = !this->handleMUL();			programcounter++;	break;
		case VM_INSTRUCTION_DIV:			execution_error = !this->handleDIV();			programcounter++;	break;
		case VM_INSTRUCTION_MOD:			execution_error = !this->handleMOD();			programcounter++;	break;
		case VM_INSTRUCTION_AND:			execution_error = !this->handleAND();			programcounter++;	break;
		case VM_INSTRUCTION_OR:				execution_error = !this->handleOR();			programcounter++;	break;
		case VM_INSTRUCTION_NOT:			execution_error = !this->handleNOT();			programcounter++;	break;
		case VM_INSTRUCTION_XOR:			execution_error = !this->handleXOR();			programcounter++;	break;
		case VM_INSTRUCTION_LSHIFT:			execution_error = !this->handleLSHIFT();		programcounter++;	break;
		case VM_INSTRUCTION_RSHIFT:			execution_error = !this->handleRSHIFT();		programcounter++;	break;
		case VM_INSTRUCTION_LOAD:			execution_error = !this->handleLOAD();			programcounter++;	break;
		case VM_INSTRUCTION_MULTILOAD:		execution_error = !this->handleMULTILOAD();		programcounter++;	break;
		case VM_INSTRUCTION_PUSH:			execution_error = !this->handlePUSH();			programcounter++;	break;
		case VM_INSTRUCTION_POP:			execution_error = !this->handlePOP();			programcounter++;	break;
		case VM_INSTRUCTION_COPY:			execution_error = !this->handleCOPY();			programcounter++;	break;
		case VM_INSTRUCTION_JUMP:			execution_error = !this->handleJUMP();			break;
		case VM_INSTRUCTION_COMPARE:		execution_error = !this->handleCOMPARE();		break;
		case VM_INSTRUCTION_CALL:			execution_error = !this->handleCALL();			break;
		case VM_INSTRUCTION_RETURN:			execution_error = !this->handleRETURN();		break;
		case VM_INSTRUCTION_TIME:			execution_error = !this->handleTIME();			programcounter++;	break;
		case VM_INSTRUCTION_COMPARETIME:	execution_error = !this->handleCOMPARETIME();	break;
		case VM_INSTRUCTION_URLMAP:			execution_error = !this->handleURLMAP();		programcounter++;	break;
		case VM_INSTRUCTION_URLMAPCHECK:	execution_error = !this->handleURLMAPCHECK();	programcounter++;	break;
		case VM_INSTRUCTION_URLMAPDELETE:	execution_error = !this->handleURLMAPDELETE();	programcounter++;	break;
		case VM_INSTRUCTION_PIDINIT:		execution_error = !this->handlePIDINIT();		programcounter++;	break;
		case VM_INSTRUCTION_PIDCLEAR:		execution_error = !this->handlePIDCLEAR();		programcounter++;	break;
		case VM_INSTRUCTION_PIDSTOP:		execution_error = !this->handlePIDSTOP();		programcounter++;	break;
		case VM_INSTRUCTION_PIDRUN:			execution_error = !this->handlePIDRUN();		programcounter++;	break;
		case VM_INSTRUCTION_HALT:			execution_error = !this->handleHALT();			break;
		case VM_INSTRUCTION_RESET:		execution_error = !this->handleRESET();		break;
		default: execution_error = true; statuscode |= VM_ERROR_UNSUPPORTED_OPERATION; break;
		}
	}
	catch (std::range_error& e) {
		execution_error = true;
		statuscode |= VM_ERROR_MEMORY_EXCEPTION;
	}
	if(execution_error)
	{
		flags |= (VM_FLAG_ERROR | VM_FLAG_HALTED);
	}

//	if(debugmode)
//	{
//		printf("\nMemory:\n");
//		const uint8_t* memdump = memory->dump();
//		for(uint8_t i = 0; i < memory->getMemorySize(); i++) {
//			if(i != 0 && i % 4 == 0)
//							printf("    ");
//			if(i != 0 && i % 24 == 0)
//							printf(" \n");
//			printf("%02x ", memdump[i]);
//		}
//		printf("\n\n");
//	}
}

/**
 *
 * @return The programcounter value.
 */
uint16_t VM::getProgramcounter()
{
	return programcounter;
}

/**
 *
 * @return VM flags.
 */
uint8_t VM::getFlags()
{
	return flags;
}

/**
 *
 * @return Pointer to the memory.
 */
Memory* VM::getMemory()
{
	return memory;
}

/**
 *
 * @return Pointer to the address stack.
 */
Stack* VM::getStack()
{
	return &stack;
}

/**
 *
 * @param mode Debug mode to use (VM_DEBUG_ON or VM_DEBUG_OFF).
 */
void VM::setDebugMode(uint8_t mode)
{
	this->debugmode = mode;
}

/**
 *
 * @return True if VM is in halt state.
 */
bool VM::halted()
{
	return flags & VM_FLAG_HALTED;
}

/**
 *
 * @return True if error flag is set.
 */
bool VM::errorFlag()
{
	return flags & VM_FLAG_ERROR;
}

/**
 *
 * @return Errorcode of the VM.
 */
uint32_t VM::getStatuscode()
{
	return statuscode;
}

/**
 * @brief Clears the VM state.
 */
void VM::clear()
{
	flags = 0;
	programcounter = 0;

	statuscode = 0;
	execution_error = false;
}

/**
 * @return True if the instruction was successful.
 */
bool VM::handleADD()//opcode optype/addresstype address operand
{
	uint8_t optype = get_optype();
	uint16_t address = get_address();
	uint16_t operandaddress = get_operandaddress(optype);
	if(debugmode)
	{
		printf("optype: %02x address: %04x operandaddress: %04x\n", optype, address, operandaddress);
	}
	switch(optype & VM_OPTYPE_MASK)
	{
	case VM_OPERAND_TYPE_UINT8:
		memory->store(address, memory->load(address) + memory->load(operandaddress));
		return true;
	case VM_OPERAND_TYPE_UINT16:
		memory->storeaddress(address, memory->loadaddress(address) + memory->loadaddress(operandaddress));
		return true;
	case VM_OPERAND_TYPE_UINT32:
		memory->storeunsigned(address, memory->loadunsigned(address) + memory->loadunsigned(operandaddress));
		return true;
	case VM_OPERAND_TYPE_DEC:
		memory->storerational(address, memory->loadrational(address) + memory->loadrational(operandaddress));
		return true;
	default:
		statuscode |= VM_ERROR_UNSUPPORTED_OPERAND;
		break;
	}
	return false;
}

/**
 * @return True if the instruction was successful.
 */
bool VM::handleSUB()
{
	uint8_t optype = get_optype();
	uint16_t address = get_address();
	uint16_t operandaddress = get_operandaddress(optype);
	switch(optype & VM_OPTYPE_MASK)
	{
	case VM_OPERAND_TYPE_UINT8:
		memory->store(address, memory->load(address) - memory->load(operandaddress));
		return true;
	case VM_OPERAND_TYPE_UINT16:
		memory->storeaddress(address, memory->loadaddress(address) - memory->loadaddress(operandaddress));
		return true;
	case VM_OPERAND_TYPE_UINT32:
		memory->storeunsigned(address, memory->loadunsigned(address) - memory->loadunsigned(operandaddress));
		return true;
	case VM_OPERAND_TYPE_DEC:
		memory->storerational(address, memory->loadrational(address) - memory->loadrational(operandaddress));
		return true;
	default:
		statuscode |= VM_ERROR_UNSUPPORTED_OPERAND;
		break;
	}
	return false;
}

/**
 * @return True if the instruction was successful.
 */
bool VM::handleMUL()
{
	uint8_t optype = get_optype();
	uint16_t address = get_address();
	uint16_t operandaddress = get_operandaddress(optype);
	switch(optype & VM_OPTYPE_MASK)
	{
	case VM_OPERAND_TYPE_UINT8:
		memory->store(address, memory->load(address) * memory->load(operandaddress));
		return true;
	case VM_OPERAND_TYPE_UINT16:
		memory->storeaddress(address, memory->loadaddress(address) * memory->loadaddress(operandaddress));
		return true;
	case VM_OPERAND_TYPE_UINT32:
		memory->storeunsigned(address, memory->loadunsigned(address) * memory->loadunsigned(operandaddress));
		return true;
	case VM_OPERAND_TYPE_DEC:
		memory->storerational(address, memory->loadrational(address) * memory->loadrational(operandaddress));
		return true;
	default:
		statuscode |= VM_ERROR_UNSUPPORTED_OPERAND;
		break;
	}
	return false;
}

/**
 * @return True if the instruction was successful.
 */
bool VM::handleDIV()
{
	uint8_t optype = get_optype();
	uint16_t address = get_address();
	uint16_t operandaddress = get_operandaddress(optype);
	switch(optype & VM_OPTYPE_MASK)
	{
	case VM_OPERAND_TYPE_UINT8:
	{
		uint8_t op1 = memory->load(address);
		uint8_t op2 = memory->load(operandaddress);
		if(op2 == 0)
		{
			flags |= VM_FLAG_DIVIDEZERO;
			statuscode |= VM_ERROR_DIVIDEZERO;
			break;
		}
		memory->store(address, op1 / op2);
		return true;
	}
	case VM_OPERAND_TYPE_UINT16:
	{
		uint16_t op1 = memory->loadaddress(address);
		uint16_t op2 = memory->loadaddress(operandaddress);
		if(op2 == 0)
		{
			flags |= VM_FLAG_DIVIDEZERO;
			statuscode |= VM_ERROR_DIVIDEZERO;
			break;
		}
		memory->storeaddress(address, op1 / op2);
		return true;
	}
	case VM_OPERAND_TYPE_UINT32:
	{
		uint32_t op1 = memory->loadunsigned(address);
		uint32_t op2 = memory->loadunsigned(operandaddress);
		if(op2 == 0)
		{
			flags |= VM_FLAG_DIVIDEZERO;
			statuscode |= VM_ERROR_DIVIDEZERO;
			break;
		}
		memory->storeunsigned(address, op1 / op2);
		return true;
	}
	case VM_OPERAND_TYPE_DEC:
	{
		rational_t op1 = memory->loadrational(address);
		rational_t op2 = memory->loadrational(operandaddress);
		if(op2 == 0)
		{
			flags |= VM_FLAG_DIVIDEZERO;
			statuscode |= VM_ERROR_DIVIDEZERO;
			break;
		}
		memory->storerational(address, op1 / op2);
		return true;
	}
	default:
		statuscode |= VM_ERROR_UNSUPPORTED_OPERAND;
		break;
	}
	return false;
}

/**
 * @return True if the instruction was successful.
 */
bool VM::handleMOD()//imperformant, da kein mod auf fixed -> convert to float modulo konvert back
{
	uint8_t optype = get_optype();
	uint16_t address = get_address();
	uint16_t operandaddress = get_operandaddress(optype);
	switch(optype & VM_OPTYPE_MASK)
	{
	case VM_OPERAND_TYPE_UINT8:
		memory->store(address, memory->load(address) % memory->load(operandaddress));
		return true;
	case VM_OPERAND_TYPE_UINT16:
		memory->storeaddress(address, memory->loadaddress(address) % memory->loadaddress(operandaddress));
		return true;
	case VM_OPERAND_TYPE_UINT32:
		memory->storeunsigned(address, memory->loadunsigned(address) % memory->loadunsigned(operandaddress));
		return true;
	case VM_OPERAND_TYPE_DEC:
#ifdef FIXEDTYPE
		memory->storerational(address, static_cast<rational_t>(fmod(static_cast<float>(memory->loadrational(address)), static_cast<float>(memory->loadrational(operandaddress)))));
#else
		memory->storerational(address, fmod(memory->loadrational(address), memory->loadrational(operandaddress)));
#endif
		return true;
	default:
		statuscode |= VM_ERROR_UNSUPPORTED_OPERAND;
		break;
	}
	return false;
}

/**
 * @return True if the instruction was successful.
 */
bool VM::handleAND()
{
	uint8_t optype = get_optype();
	uint16_t address = get_address();
	uint16_t operandaddress = get_operandaddress(optype);
	switch(optype & VM_OPTYPE_MASK)
	{
	case VM_OPERAND_TYPE_UINT8:
		memory->store(address, memory->load(address) & memory->load(operandaddress));
		return true;
	case VM_OPERAND_TYPE_UINT16:
		memory->storeaddress(address, memory->loadaddress(address) & memory->loadaddress(operandaddress));
		return true;
	case VM_OPERAND_TYPE_UINT32:
		memory->storeunsigned(address, memory->loadunsigned(address) & memory->loadunsigned(operandaddress));
		return true;
	case VM_OPERAND_TYPE_DEC:
	default:
		statuscode |= VM_ERROR_UNSUPPORTED_OPERAND;
		break;
	}
	return false;
}

/**
 * @return True if the instruction was successful.
 */
bool VM::handleOR()
{
	uint8_t optype = get_optype();
	uint16_t address = get_address();
	uint16_t operandaddress = get_operandaddress(optype);
	switch(optype & VM_OPTYPE_MASK)
	{
	case VM_OPERAND_TYPE_UINT8:
		memory->store(address, memory->load(address) | memory->load(operandaddress));
		return true;
	case VM_OPERAND_TYPE_UINT16:
		memory->storeaddress(address, memory->loadaddress(address) | memory->loadaddress(operandaddress));
		return true;
	case VM_OPERAND_TYPE_UINT32:
		memory->storeunsigned(address, memory->loadunsigned(address) | memory->loadunsigned(operandaddress));
		return true;
	case VM_OPERAND_TYPE_DEC:
	default:
		statuscode |= VM_ERROR_UNSUPPORTED_OPERAND;
		break;
	}
	return false;
}

/**
 * @return True if the instruction was successful.
 */
bool VM::handleNOT()
{
	uint8_t optype = get_optype();
	uint16_t address = get_address();

	switch(optype & VM_OPTYPE_MASK)
	{
	case VM_OPERAND_TYPE_UINT8:
		memory->store(address, memory->load(address) ^ -1);
		return true;
	case VM_OPERAND_TYPE_UINT16:
		memory->storeaddress(address, memory->loadaddress(address) ^ -1);
		return true;
	case VM_OPERAND_TYPE_UINT32:
		memory->storeunsigned(address, memory->loadunsigned(address) ^ -1);
		return true;
	case VM_OPERAND_TYPE_DEC:
	default:
		statuscode |= VM_ERROR_UNSUPPORTED_OPERAND;
		break;
	}
	return false;
}

/**
 * @return True if the instruction was successful.
 */
bool VM::handleXOR()
{
	uint8_t optype = get_optype();
	uint16_t address = get_address();
	uint16_t operandaddress = get_operandaddress(optype);

	switch(optype & VM_OPTYPE_MASK)
	{
	case VM_OPERAND_TYPE_UINT8:
		memory->store(address, memory->load(address) ^ memory->load(operandaddress));
		return true;
	case VM_OPERAND_TYPE_UINT16:
		memory->storeaddress(address, memory->loadaddress(address) ^ memory->loadaddress(operandaddress));
		return true;
	case VM_OPERAND_TYPE_UINT32:
		memory->storeunsigned(address, memory->loadunsigned(address) ^ memory->loadunsigned(operandaddress));
		return true;
	case VM_OPERAND_TYPE_DEC:
	default:
		statuscode |= VM_ERROR_UNSUPPORTED_OPERAND;
		break;
	}
	return false;
}

/**
 * @return True if the instruction was successful.
 */
bool VM::handleLSHIFT()
{
	uint8_t optype = get_optype();
	uint16_t address = get_address();
	uint16_t operandaddress = get_operandaddress(optype);

	switch(optype & VM_OPTYPE_MASK)
	{
	case VM_OPERAND_TYPE_UINT8:
		memory->store(address, memory->load(address) << memory->load(operandaddress));
		return true;
	case VM_OPERAND_TYPE_UINT16:
		memory->storeaddress(address, memory->loadaddress(address) << memory->loadaddress(operandaddress));
		return true;
	case VM_OPERAND_TYPE_UINT32:
		memory->storeunsigned(address, memory->loadunsigned(address) << memory->loadunsigned(operandaddress));
		return true;
	case VM_OPERAND_TYPE_DEC:
	default:
		statuscode |= VM_ERROR_UNSUPPORTED_OPERAND;
		break;
	}
	return false;
}

/**
 * @return True if the instruction was successful.
 */
bool VM::handleRSHIFT()
{
	uint8_t optype = get_optype();
	uint16_t address = get_address();
	uint16_t operandaddress = get_operandaddress(optype);

	switch(optype & VM_OPTYPE_MASK)
	{
	case VM_OPERAND_TYPE_UINT8:
		memory->store(address, memory->load(address) >> memory->load(operandaddress));
		return true;
	case VM_OPERAND_TYPE_UINT16:
		memory->storeaddress(address, memory->loadaddress(address) >> memory->loadaddress(operandaddress));
		return true;
	case VM_OPERAND_TYPE_UINT32:
		memory->storeunsigned(address, memory->loadunsigned(address) >> memory->loadunsigned(operandaddress));
		return true;
	case VM_OPERAND_TYPE_DEC:
	default:
		statuscode |= VM_ERROR_UNSUPPORTED_OPERAND;
		break;
	}
	return false;
}

/**
 * @return True if the instruction was successful.
 */
bool VM::handleLOAD()
{
	uint8_t optype = get_optype();
	uint16_t address = get_address();
	uint16_t operandaddress = get_operandaddress(optype);

	switch(optype & VM_OPTYPE_MASK)
	{
	case VM_OPERAND_TYPE_UINT8:
		memory->store(address, memory->load(operandaddress));
		return true;
	case VM_OPERAND_TYPE_UINT16:
		memory->storeaddress(address, memory->loadaddress(operandaddress));
		return true;
	case VM_OPERAND_TYPE_UINT32:
		memory->storeunsigned(address, memory->loadunsigned(operandaddress));
		return true;
	case VM_OPERAND_TYPE_DEC:
		memory->storerational(address, memory->loadrational(operandaddress));
		return true;
	default:
		statuscode |= VM_ERROR_UNSUPPORTED_OPERAND;
		break;
	}
	return false;
}

/**
 * @return True if the instruction was successful.
 */
bool VM::handleMULTILOAD()
{
	uint8_t optype = get_optype();
	uint16_t address = get_address();
	uint8_t count = get_number();
	uint16_t operandaddress = 0;

	switch(optype & VM_OPTYPE_MASK)
	{
	case VM_OPERAND_TYPE_UINT8:
		for(uint8_t i = 0; i < count; i++)
		{
			operandaddress = get_operandaddress(optype);
			memory->store(address + i*sizeof(uint8_t), memory->load(operandaddress));
		}
		return true;
	case VM_OPERAND_TYPE_UINT16:
		for(uint8_t i = 0; i < count; i++)
		{
			operandaddress = get_operandaddress(optype);
			memory->storeaddress(address + i*sizeof(uint16_t), memory->loadaddress(operandaddress));
		}
		return true;
	case VM_OPERAND_TYPE_UINT32:
		for(uint8_t i = 0; i < count; i++)
		{
			operandaddress = get_operandaddress(optype);
			memory->storeunsigned(address + i*sizeof(uint32_t), memory->loadunsigned(operandaddress));
		}
		return true;
	case VM_OPERAND_TYPE_DEC:
		for(uint8_t i = 0; i < count; i++)
		{
			operandaddress = get_operandaddress(optype);
			memory->storerational(address + i*sizeof(rational_t), memory->loadrational(operandaddress));
		}
		return true;
	default:
		statuscode |= VM_ERROR_UNSUPPORTED_OPERAND;
		break;
	}
	return false;
}

/**
 * @return True if the instruction was successful.
 */
bool VM::handlePUSH()
{
	return false;
}

/**
 * @return True if the instruction was successful.
 */
bool VM::handlePOP()
{
	return false;
}

/**
 * @return True if the instruction was successful.
 */
bool VM::handleCOPY()
{
	uint16_t srcaddress = get_address();
	uint8_t len = get_number();
	uint16_t destaddress = get_address();

	memory->copy(srcaddress, len, destaddress);
	return true;
}

/**
 * @return True if the instruction was successful.
 */
bool VM::handleJUMP()
{
	uint8_t optype = get_optype();//nur für addresse oder literal, immer 16bit adresse
	uint16_t operandaddress = get_operandaddress(optype | VM_OPERAND_TYPE_UINT16);
	programcounter = memory->load(operandaddress);
	return true;
}

/**
 * @return True if the instruction was successful.
 */
bool VM::handleCOMPARE()//überarbeiten
{
	uint8_t optype = get_optype();
	uint16_t address = get_address();
	uint16_t operandaddress = get_operandaddress(optype);

	//jumpaddressen immer als OPERAND_TYPE_UINT16, nur literal od address interassant
	uint16_t jumpsmaller = get_operandaddress(optype | VM_LITERAL | VM_OPERAND_TYPE_UINT16);
	uint16_t jumpequal = get_operandaddress(optype | VM_LITERAL | VM_OPERAND_TYPE_UINT16);
	uint16_t jumpgreater = get_operandaddress(optype | VM_LITERAL | VM_OPERAND_TYPE_UINT16);
	int8_t compare = 0;
	switch(optype & VM_OPTYPE_MASK)
	{
	case VM_OPERAND_TYPE_UINT8:
		{//scope for initialization
			uint8_t value1 = memory->load(address);
			uint8_t value2 = memory->load(operandaddress);
			if(value1 < value2)
			{
				compare = -1;
			}
			else if(value1 > value2)
			{
				compare = 1;
			}
		}
		break;
	case VM_OPERAND_TYPE_UINT16:
		{//scope for initialization
			uint16_t value1 = memory->loadaddress(address);
			uint16_t value2 = memory->loadaddress(operandaddress);
			if(value1 < value2)
			{
				compare = -1;
			}
			else if(value1 > value2)
			{
				compare = 1;
			}
		}
		break;
	case VM_OPERAND_TYPE_UINT32:
		{//scope for initialization
			uint32_t value1 = memory->loadunsigned(address);
			uint32_t value2 = memory->loadunsigned(operandaddress);
			if(value1 < value2)
			{
				compare = -1;
			}
			else if(value1 > value2)
			{
				compare = 1;
			}
		}
		break;
	case VM_OPERAND_TYPE_DEC:
		{
			rational_t value1 = memory->loadrational(address);
			rational_t value2 = memory->loadrational(operandaddress);
			if(value1 < value2)
			{
				compare = -1;
			}
			else if(value1 > value2)
			{
				compare = 1;
			}
		}
		break;
	default:
		statuscode |= VM_ERROR_UNSUPPORTED_OPERAND;
		return false;
	}
	switch(compare)
	{
	case -1:
		programcounter = memory->loadaddress(jumpsmaller);
		return true;
	case 0:
		programcounter = memory->loadaddress(jumpequal);
		return true;
	case 1:
		programcounter = memory->loadaddress(jumpgreater);
		return true;
	default:
		return false;
	}
}

/**
 * @return True if the instruction was successful.
 */
bool VM::handleCALL()
{
	uint16_t address = get_address();
	stack.push(++programcounter);
	programcounter = address;
	return true;
}

/**
 * @return True if the instruction was successful.
 */
bool VM::handleRETURN()
{
	uint16_t address = stack.pop();
	programcounter = address;
	return true;
}

/**
 * @return True if the instruction was successful.
 */
bool VM::handleTIME()
{
	uint16_t address = get_address();
	memory->storeunsigned(address, xtimer_now());//WARNING time is Systemtime in µs
	return true;
}

/**
 * @return True if the instruction was successful.
 */
bool VM::handleCOMPARETIME()
{
	uint16_t address = get_address();
	uint16_t timeout = get_operandaddress(VM_OPERAND_TYPE_UINT32 | VM_LITERAL);
	uint16_t jump1 = get_address();
	uint16_t jump2 = get_address();
	if(xtimer_now() - memory->loadunsigned(address) >= (memory->loadunsigned(timeout) * 1000))
	{//compare in ns because xtimer_now is in ns, stored time in ms (ns overflows after 1.19h, only for short timers)
		programcounter = jump1;
	}
	else
	{
		programcounter = jump2;
	}
	return true;
}

/**
 * @return True if the instruction was successful.
 */
bool VM::handleURLMAP()
{
	uint8_t optype = get_optype();
	uint8_t map_options = get_optype();//also a uint8_t value;
	uint8_t id = VM_OPTYPE_ID(optype);
	uint16_t value_address = get_address();//Adresse an der der wert abgerufen, gespeichet werden soll
	uint16_t port = get_address();//port als uint16_t angegeben
	uint16_t url_address = NO_MAPPING;
	if((map_options & VM_MAP_OPTION_URL_MASK) == VM_MAP_OPTION_URL_LITERAL)
	{
		url_address = get_operandaddress(VM_LITERAL | VM_OPERAND_TYPE_UINT8);//Anfang des URL strings ist im befehl
		while(memory->load(++programcounter) != 0x00);
	}
	else
	{
		url_address = get_operandaddress(VM_ADDRESS);//Anfang des URL strings ist an addresse im speicher
	}

	uint16_t resource_address = NO_MAPPING;
	if((map_options & VM_MAP_OPTION_RESOURCE_MASK) == VM_MAP_OPTION_RESOURCE_LITERAL)
	{
		resource_address = get_operandaddress(VM_LITERAL | VM_OPERAND_TYPE_UINT8);//Anfang des resource strings
		while(memory->load(++programcounter) != 0x00);
	}
	else
	{
		resource_address = get_operandaddress(VM_ADDRESS);//Anfang des resource strings
	}
	memory->map(id, optype, map_options, value_address, port, url_address, resource_address);
	return true;
//	return false;
}

/**
 * @return True if the instruction was successful.
 */
bool VM::handleURLMAPCHECK()
{
	uint8_t id = VM_OPTYPE_ID(get_optype());
	uint16_t address = get_address();
	memory->store(address, memory->checkmap(id));
	return true;
}

/**
 * @return True if the instruction was successful.
 */
bool VM::handleURLMAPDELETE()
{
	uint8_t id = VM_OPTYPE_ID(get_optype());
	memory->unmap(id);
	return true;
}

/**
 * @return True if the instruction was successful.
 */
bool VM::handlePIDINIT()
{
	uint8_t id = VM_OPTYPE_ID(get_optype());
	if(id >= VM_PID_NUM_AVAILABLE)
	{
		statuscode |= VM_ERROR_ID_UNAVAILABLE;
		return false;
	}
	uint16_t input_address = get_address();
	uint16_t output_address = get_address();
	uint16_t setpoint_address = get_address();
	uint16_t kp_address = get_operandaddress(VM_LITERAL | VM_OPERAND_TYPE_DEC);
	uint16_t ki_address = get_operandaddress(VM_LITERAL | VM_OPERAND_TYPE_DEC);
	uint16_t kd_address = get_operandaddress(VM_LITERAL | VM_OPERAND_TYPE_DEC);
	uint16_t sampleTime_address = get_operandaddress(VM_LITERAL | VM_OPERAND_TYPE_UINT32);
	uint16_t lowerLimit_address = get_operandaddress(VM_LITERAL | VM_OPERAND_TYPE_DEC);
	uint16_t upperLimit_address = get_operandaddress(VM_LITERAL | VM_OPERAND_TYPE_DEC);
	uint16_t direction_address = get_operandaddress(VM_LITERAL | VM_OPERAND_TYPE_UINT8);
	if(!pids[id].init(memory, input_address, output_address, setpoint_address,
			memory->loadrational(kp_address), memory->loadrational(ki_address), memory->loadrational(kd_address),
			memory->loadunsigned(sampleTime_address), memory->load(direction_address),
			memory->loadrational(lowerLimit_address), memory->loadrational(upperLimit_address)))
	{
		statuscode |= VM_ERROR_PID_INIT;
		return false;
	}
	return true;
}

/**
 * @return True if the instruction was successful.
 */
bool VM::handlePIDCLEAR()
{
	uint8_t id = VM_OPTYPE_ID(get_optype());
	if(id >= VM_PID_NUM_AVAILABLE)
	{
		statuscode |= VM_ERROR_ID_UNAVAILABLE;
		return false;
	}
	pids[id].clear();
	return true;
}

/**
 * @return True if the instruction was successful.
 */
bool VM::handlePIDSTOP()
{
	uint8_t id = VM_OPTYPE_ID(get_optype());
	if(id >= VM_PID_NUM_AVAILABLE)
	{
		statuscode |= VM_ERROR_ID_UNAVAILABLE;
		return false;
	}
	pids[id].setMode(PID_MANUAL);
	return true;
}

/**
 * @return True if the instruction was successful.
 */
bool VM::handlePIDRUN()
{
	uint8_t id = VM_OPTYPE_ID(get_optype());
	if(id >= VM_PID_NUM_AVAILABLE)
	{
		statuscode |= VM_ERROR_ID_UNAVAILABLE;
		return false;
	}
	pids[id].setMode(PID_AUTOMATIC);
	return true;
}

/**
 * @return True if the instruction was successful.
 */
bool VM::handleHALT()
{
	flags |= VM_FLAG_HALTED;
	return true;
}
/**
 * @return always returns false, therefore the halt flag will be set. Clears Memory and Stack.
 */

bool VM::handleRESET()
{
	statuscode |= VM_ERROR_RESET;
	memory->clear();
	stack.clear();
	for(uint8_t i = 0; i < VM_PID_NUM_AVAILABLE; i++)
	{
		pids[i].clear();
	}
	return false;
}

/**
 * @return Optype coded in the instruction bytecode.
 */
inline uint8_t VM::get_optype()
{
	return memory->load(++programcounter);
}

/**
 * @return Number coded in the instruction bytecode.
 */
inline uint8_t VM::get_number()
{
	return memory->load(++programcounter);
}

/**
 * @return Address coded in the instruction bytecode.
 */
inline uint16_t VM::get_address()
{
	uint16_t temp = memory->loadaddress(++programcounter);
	++programcounter;
	return temp;
}

/**
 * @return Address of the operand coded in the instruction bytecode. Depends on Optype (literal or address), programcounter will be set to according to the datatype.
 */
inline uint16_t VM::get_operandaddress(uint8_t optype)
{
	uint16_t operandaddress;
	switch(optype & VM_ADDRESS_MASK)
	{
	case VM_LITERAL:
		operandaddress = ++programcounter;
		switch(optype & VM_OPTYPE_MASK)
		{
		case VM_OPERAND_TYPE_UINT32:
			programcounter += sizeof(uint32_t) - 1;
			break;
		case VM_OPERAND_TYPE_DEC:
			programcounter += sizeof(rational_t) - 1;
			break;
		case VM_OPERAND_TYPE_UINT16:
			programcounter += sizeof(uint16_t) - 1;
			break;
		case VM_OPERAND_TYPE_UINT8:
			programcounter += sizeof(uint8_t) - 1;
			break;
		default:
			statuscode |= VM_ERROR_UNSUPPORTED_OPERAND;
			break;
		}
		break;
	case VM_ADDRESS:
	default:
		operandaddress = get_address();
		break;
	}
	return operandaddress;
}

