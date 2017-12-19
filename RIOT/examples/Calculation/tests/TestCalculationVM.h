/*
 * Copyright (C) 2017 Mattes Besuden
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @brief       Tests for CalculationVM.h
 *
 * @author      Mattes Besuden <besuden@uni-bremen.de>
 */
#ifndef TESTCALCULATIONVM_H_
#define TESTCALCULATIONVM_H_
#include "Tests.h"
#include "CalculationVM.h"

extern "C" {
extern void* mempcpy(void * to, const void *from, size_t size);
}

PID pids[VM_PID_NUM_AVAILABLE];

inline void test_CalculationVM_setProgram()
{
	Memory* mem = &Memory::instance();
	mem->clear();
	VM vm(mem, pids);
	uint8_t program[] = {0xff, 0xff, 0xff};
	vm.setProgram(program, 3);
	ASSERT(mem->load(0x0000) == 0xff, "setProgram address 0x0000 != 0xff");
	ASSERT(mem->load(0x0001) == 0xff, "setProgram address 0x0001 != 0xff");
	ASSERT(mem->load(0x0002) == 0xff, "setProgram address 0x0002 != 0xff");
	ASSERT(mem->load(0x0003) == 0x00, "setProgram address 0x0003 != 0x00");
	ASSERT(vm.getProgramcounter() == 0, "programcounter wrong");

	ASSERT((vm.getStatuscode() & VM_ERROR_MASK) == 0, "VM shouldnt have an error");
}

inline void test_CalculationVM_ADD_unsigned()
{
	Memory* mem = &Memory::instance();
	mem->clear();
	VM vm(mem, pids);
	uint8_t program[] = {VM_INSTRUCTION_ADD, VM_OPERAND_TYPE_UINT32 | VM_LITERAL, 0x10, 0x00, 0x00, 0x01, 0x00, 0x00, VM_INSTRUCTION_HALT};
	vm.setProgram(program, 9);
	vm.executeStep();
	ASSERT(mem->loadunsigned(0x0010) == 0x00000100, "Add unsigned wrong");
	ASSERT(vm.getProgramcounter() == 8, "programcounter wrong");
	vm.executeStep();
	ASSERT(vm.halted(), "VM not in HALT");
	ASSERT(vm.getProgramcounter() == 8, "programcounter wrong");

	ASSERT((vm.getStatuscode() & VM_ERROR_MASK) == 0, "VM shouldnt have an error");
}

inline void test_CalculationVM_ADD_decimal()
{
	Memory* mem = &Memory::instance();
	mem->clear();
	VM vm(mem, pids);
	//set value in mem(32) to 1
	mem->storerational(0x0010, (rational_t)1);
	uint8_t program[] = {VM_INSTRUCTION_ADD, VM_OPERAND_TYPE_DEC | VM_LITERAL, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, VM_INSTRUCTION_HALT};
	rational_t temp(1.35);
	mempcpy(program + 4, &temp, sizeof(rational_t));
	vm.setProgram(program, 9);
	vm.executeStep();
	ASSERT(mem->loadrational(0x0010) == (rational_t)2.35, "Add decimal wrong");
	ASSERT(vm.getProgramcounter() == 8, "programcounter wrong");
	vm.executeStep();
	ASSERT(vm.halted(), "VM not in HALT");
	ASSERT(vm.getProgramcounter() == 8, "programcounter wrong");

	ASSERT((vm.getStatuscode() & VM_ERROR_MASK) == 0, "VM shouldnt have an error");
}

inline void test_CalculationVM_SUB_unsigned()
{

	Memory* mem = &Memory::instance();
	mem->clear();
	VM vm(mem, pids);
	uint8_t program[] = {VM_INSTRUCTION_SUB, VM_OPERAND_TYPE_UINT32 | VM_LITERAL, 0x10, 0x00, 0x01, 0x00, 0x00, 0x00, VM_INSTRUCTION_HALT};
	vm.setProgram(program, 9);
	vm.executeStep();
	ASSERT(mem->loadunsigned(0x0010) == 0xffffffff, "Sub unsigned wrong");
	ASSERT(vm.getProgramcounter() == 8, "programcounter wrong");
	vm.executeStep();
	ASSERT(vm.halted(), "VM not in HALT");
	ASSERT(vm.getProgramcounter() == 8, "programcounter wrong");

	ASSERT((vm.getStatuscode() & VM_ERROR_MASK) == 0, "VM shouldnt have an error");
}

inline void test_CalculationVM_SUB_decimal()
{

	Memory* mem = &Memory::instance();
	mem->clear();
	VM vm(mem, pids);
	uint8_t program[] = {VM_INSTRUCTION_SUB, VM_OPERAND_TYPE_DEC | VM_LITERAL, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, VM_INSTRUCTION_HALT};
	rational_t temp(1);
	mempcpy(program + 4, &temp, sizeof(rational_t));
	vm.setProgram(program, 9);
	vm.executeStep();
	ASSERT(mem->loadrational(0x0010) == (rational_t)-1, "Sub decimal wrong");
	ASSERT(vm.getProgramcounter() == 8, "programcounter wrong");

	ASSERT((vm.getStatuscode() & VM_ERROR_MASK) == 0, "VM shouldnt have an error");
}

inline void test_CalculationVM_MUL_unsigned()
{

	Memory* mem = &Memory::instance();
	mem->clear();
	VM vm(mem, pids);
	//load value in memory(32)
	mem->storeunsigned(0x0010, 2);
	uint8_t program[] = {VM_INSTRUCTION_MUL, VM_OPERAND_TYPE_UINT32 | VM_LITERAL, 0x10, 0x00, 0x02, 0x00, 0x00, 0x00, VM_INSTRUCTION_HALT};
	vm.setProgram(program, 9);
	vm.executeStep();
	ASSERT(mem->loadunsigned(0x0010) == 4, "Mul unsigned wrong");
	ASSERT(vm.getProgramcounter() == 8, "programcounter wrong");
	vm.executeStep();
	ASSERT(vm.halted(), "VM not in HALT");
	ASSERT(vm.getProgramcounter() == 8, "programcounter wrong");

	ASSERT((vm.getStatuscode() & VM_ERROR_MASK) == 0, "VM shouldnt have an error");
}

inline void test_CalculationVM_MUL_decimal()
{

	Memory* mem = &Memory::instance();
	mem->clear();
	VM vm(mem, pids);
	mem->storerational(0x0010, (rational_t)2.5);
	uint8_t program[] = {VM_INSTRUCTION_MUL, VM_OPERAND_TYPE_DEC | VM_LITERAL, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, VM_INSTRUCTION_HALT};
	rational_t temp(2);
	mempcpy(program + 4, &temp, sizeof(rational_t));
	vm.setProgram(program, 9);
	vm.executeStep();
	ASSERT(mem->loadrational(0x0010) == (rational_t)5, "Mul decimal wrong");
	ASSERT(vm.getProgramcounter() == 8, "programcounter wrong");
	vm.executeStep();
	ASSERT(vm.halted(), "VM not in HALT");
	ASSERT(vm.getProgramcounter() == 8, "programcounter wrong");

	ASSERT((vm.getStatuscode() & VM_ERROR_MASK) == 0, "VM shouldnt have an error");
}

inline void test_CalculationVM_DIV_unsigned()
{

	Memory* mem = &Memory::instance();
	mem->clear();
	VM vm(mem, pids);
	//load value in memory(32)
	mem->storeunsigned(0x0010, 2);
	uint8_t program[] = {VM_INSTRUCTION_DIV, VM_OPERAND_TYPE_UINT32 | VM_LITERAL, 0x10, 0x00, 0x02, 0x00, 0x00, 0x00, VM_INSTRUCTION_HALT};
	vm.setProgram(program, 9);
	vm.executeStep();
	ASSERT(mem->loadunsigned(0x0010) == 1, "Div unsigned wrong");
	ASSERT(vm.getProgramcounter() == 8, "programcounter wrong");
	vm.executeStep();
	ASSERT(vm.halted(), "VM not in HALT");
	ASSERT(vm.getProgramcounter() == 8, "programcounter wrong");

	ASSERT((vm.getStatuscode() & VM_ERROR_MASK) == 0, "VM shouldnt have an error");
}

inline void test_CalculationVM_DIV_decimal()
{

	Memory* mem = &Memory::instance();
	mem->clear();
	VM vm(mem, pids);
	mem->storerational(0x0010, (rational_t)2.5);
	uint8_t program[] = {VM_INSTRUCTION_DIV, VM_OPERAND_TYPE_DEC | VM_LITERAL, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, VM_INSTRUCTION_HALT};
	rational_t temp(2);
	mempcpy(program + 4, &temp, sizeof(rational_t));
	vm.setProgram(program, 9);
	vm.executeStep();
	ASSERT(mem->loadrational(0x0010) == (rational_t)1.25, "Div Zero decimal wrong");
	ASSERT(vm.getProgramcounter() == 8, "programcounter wrong");
	vm.executeStep();
	ASSERT(vm.halted(), "VM not in HALT");
	ASSERT(vm.getProgramcounter() == 8, "programcounter wrong");

	ASSERT((vm.getStatuscode() & VM_ERROR_MASK) == 0, "VM shouldnt have an error");
}

inline void test_CalculationVM_DIV_unsigned_zero()
{

	Memory* mem = &Memory::instance();
	mem->clear();
	VM vm(mem, pids);
	//load value in memory(32)
	mem->storeunsigned(0x0010, 2);
	uint8_t program[] = {VM_INSTRUCTION_DIV, VM_OPERAND_TYPE_UINT32 | VM_LITERAL, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, VM_INSTRUCTION_HALT};
	vm.setProgram(program, 9);
	vm.executeStep();
	ASSERT(mem->loadunsigned(0x0010) == 2, "Div zero unsigned wrong");
	ASSERT(vm.getProgramcounter() == 8, "programcounter wrong");
	vm.executeStep();
	ASSERT(vm.halted(), "VM not in HALT");
	ASSERT(vm.getProgramcounter() == 8, "programcounter wrong");
	ASSERT(vm.errorFlag() == true, "VM should have an error");

	ASSERT(vm.getStatuscode() & VM_ERROR_DIVIDEZERO, "VM should have an errorcode ERROR_DEVIDEZERO");
}

inline void test_CalculationVM_DIV_decimal_zero()
{

	Memory* mem = &Memory::instance();
	mem->clear();
	VM vm(mem, pids);
	mem->storerational(0x0010, (rational_t)2.5);
	uint8_t program[] = {VM_INSTRUCTION_DIV, VM_OPERAND_TYPE_DEC | VM_LITERAL, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, VM_INSTRUCTION_HALT};
	rational_t temp(0);
	mempcpy(program + 4, &temp, sizeof(rational_t));
	vm.setProgram(program, 9);
	vm.executeStep();
	ASSERT(mem->loadrational(0x0010) == (rational_t)2.5, "Div zero decimal wrong");
	ASSERT(vm.getProgramcounter() == 8, "programcounter wrong");
	vm.executeStep();
	ASSERT(vm.halted(), "VM not in HALT");
	ASSERT(vm.getProgramcounter() == 8, "programcounter wrong");
	ASSERT(vm.errorFlag() == true, "VM should have an error");

	ASSERT(vm.getStatuscode() & VM_ERROR_DIVIDEZERO, "VM should have an errorcode ERROR_DEVIDEZERO");
}

inline void test_CalculationVM_MOD_unsigned()
{

	Memory* mem = &Memory::instance();
	mem->clear();
	VM vm(mem, pids);
	//load value in memory(32)
	mem->storeunsigned(0x0010, 2);
	uint8_t program[] = {VM_INSTRUCTION_MOD, VM_OPERAND_TYPE_UINT32 | VM_LITERAL, 0x10, 0x00, 0x02, 0x00, 0x00, 0x00, VM_INSTRUCTION_HALT};
	vm.setProgram(program, 9);
	vm.executeStep();
	ASSERT(mem->loadunsigned(0x0010) == 0, "Mod unsigned wrong");
	ASSERT(vm.getProgramcounter() == 8, "programcounter wrong");
	vm.executeStep();
	ASSERT(vm.halted(), "VM not in HALT");
	ASSERT(vm.getProgramcounter() == 8, "programcounter wrong");

	ASSERT((vm.getStatuscode() & VM_ERROR_MASK) == 0, "VM shouldnt have an error");
}

inline void test_CalculationVM_MOD_decimal()
{

	Memory* mem = &Memory::instance();
	mem->clear();
	VM vm(mem, pids);
	mem->storerational(0x0010, (rational_t)2.5);
	uint8_t program[] = {VM_INSTRUCTION_MOD, VM_OPERAND_TYPE_DEC | VM_LITERAL, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, VM_INSTRUCTION_HALT};
	rational_t temp(2);
	mempcpy(program + 4, &temp, sizeof(rational_t));
	vm.setProgram(program, 9);
	vm.executeStep();
	ASSERT(mem->loadrational(0x0010) == (rational_t)0.5, "Mod decimal wrong");
	ASSERT(vm.getProgramcounter() == 8, "programcounter wrong");
	vm.executeStep();
	ASSERT(vm.halted(), "VM not in HALT");
	ASSERT(vm.getProgramcounter() == 8, "programcounter wrong");

	ASSERT((vm.getStatuscode() & VM_ERROR_MASK) == 0, "VM shouldnt have an error");
}

inline void test_CalculationVM_AND()
{
	Memory* mem = &Memory::instance();
	mem->clear();
	VM vm(mem, pids);

	uint8_t program[] = {VM_INSTRUCTION_AND, VM_OPERAND_TYPE_UINT32 | VM_LITERAL, 0x20, 0x00, 0xff, 0xff, 0xff, 0xff, VM_INSTRUCTION_HALT};
	mem->storeunsigned(0x0020, 0x01010101);
	vm.setProgram(program, 9);
	vm.executeStep();
	ASSERT(mem->loadunsigned(0x0020) == (0x01010101), "AND unsigned wrong");
	ASSERT(vm.getProgramcounter() == 8, "programcounter wrong");

	mem->clear();
	uint8_t program2[] = {VM_INSTRUCTION_AND, VM_OPERAND_TYPE_UINT16 | VM_LITERAL, 0x20, 0x00, 0xff, 0xff, VM_INSTRUCTION_HALT};
	mem->storeaddress(0x0020, 0x0101);
	vm.setProgram(program2, 7);
	vm.executeStep();
	ASSERT(mem->loadaddress(0x0020) == (0x0101), "AND address wrong");
	ASSERT(vm.getProgramcounter() == 6, "programcounter wrong");

	mem->clear();
	uint8_t program3[] = {VM_INSTRUCTION_AND, VM_OPERAND_TYPE_UINT8 | VM_LITERAL, 0x20, 0x00, 0xff, VM_INSTRUCTION_HALT};
	mem->store(0x0020, 0x01);
	vm.setProgram(program3, 6);
	vm.executeStep();
	ASSERT(mem->load(0x0020) == (0x01), "AND uin8_t wrong");
	ASSERT(vm.getProgramcounter() == 5, "programcounter wrong");

	ASSERT((vm.getStatuscode() & VM_ERROR_MASK) == 0, "VM shouldnt have an error");
}

inline void test_CalculationVM_OR()
{
	Memory* mem = &Memory::instance();
	mem->clear();
	VM vm(mem, pids);

	uint8_t program[] = {VM_INSTRUCTION_OR, VM_OPERAND_TYPE_UINT32 | VM_LITERAL, 0x20, 0x00, 0xff, 0xff, 0xff, 0xff, VM_INSTRUCTION_HALT};
	mem->storeunsigned(0x0020, 0x01010101);
	vm.setProgram(program, 9);
	vm.executeStep();
	ASSERT(mem->loadunsigned(0x0020) == (0xffffffff), "OR unsigned wrong");
	ASSERT(vm.getProgramcounter() == 8, "programcounter wrong");

	mem->clear();
	uint8_t program2[] = {VM_INSTRUCTION_OR, VM_OPERAND_TYPE_UINT16 | VM_LITERAL, 0x20, 0x00, 0xff, 0xff, VM_INSTRUCTION_HALT};
	mem->storeaddress(0x0020, 0x0101);
	vm.setProgram(program2, 7);
	vm.executeStep();
	ASSERT(mem->loadaddress(0x0020) == (0xffff), "OR address wrong");
	ASSERT(vm.getProgramcounter() == 6, "programcounter wrong");

	mem->clear();
	uint8_t program3[] = {VM_INSTRUCTION_OR, VM_OPERAND_TYPE_UINT8 | VM_LITERAL, 0x20, 0x00, 0xff, VM_INSTRUCTION_HALT};
	mem->store(0x0020, 0x01);
	vm.setProgram(program3, 6);
	vm.executeStep();
	ASSERT(mem->load(0x0020) == (0xff), "OR uin8_t wrong");
	ASSERT(vm.getProgramcounter() == 5, "programcounter wrong");

	ASSERT((vm.getStatuscode() & VM_ERROR_MASK) == 0, "VM shouldnt have an error");
}

inline void test_CalculationVM_NOT()
{
	Memory* mem = &Memory::instance();
	mem->clear();
	VM vm(mem, pids);

	uint8_t program[] = {VM_INSTRUCTION_NOT, VM_OPERAND_TYPE_UINT32 | VM_LITERAL, 0x20, 0x00, VM_INSTRUCTION_HALT};
	mem->storeunsigned(0x0020, 0x01010101);
	vm.setProgram(program, 5);
	vm.executeStep();
	ASSERT(mem->loadunsigned(0x0020) == (0xfefefefe), "NOT unsigned wrong");
	ASSERT(vm.getProgramcounter() == 4, "programcounter wrong");

	mem->clear();
	uint8_t program2[] = {VM_INSTRUCTION_NOT, VM_OPERAND_TYPE_UINT16 | VM_LITERAL, 0x20, 0x00, VM_INSTRUCTION_HALT};
	mem->storeaddress(0x0020, 0x0101);
	vm.setProgram(program2, 5);
	vm.executeStep();
	ASSERT(mem->loadaddress(0x0020) == (0xfefe), "NOT address wrong");
	ASSERT(vm.getProgramcounter() == 4, "programcounter wrong");

	mem->clear();
	uint8_t program3[] = {VM_INSTRUCTION_NOT, VM_OPERAND_TYPE_UINT8 | VM_LITERAL, 0x20, 0x00, VM_INSTRUCTION_HALT};
	mem->store(0x0020, 0x01);
	vm.setProgram(program3, 5);
	vm.executeStep();
	ASSERT(mem->load(0x0020) == (0xfe), "NOT uin8_t wrong");
	ASSERT(vm.getProgramcounter() == 4, "programcounter wrong");

	ASSERT((vm.getStatuscode() & VM_ERROR_MASK) == 0, "VM shouldnt have an error");
}

inline void test_CalculationVM_XOR()
{
	Memory* mem = &Memory::instance();
	mem->clear();
	VM vm(mem, pids);

	uint8_t program[] = {VM_INSTRUCTION_XOR, VM_OPERAND_TYPE_UINT32 | VM_LITERAL, 0x20, 0x00, 0xff, 0x00, 0xff, 0x00, VM_INSTRUCTION_HALT};
	mem->storeunsigned(0x0020, 0x01010101);
	vm.setProgram(program, 9);
	vm.executeStep();
	ASSERT(mem->loadunsigned(0x0020) == (0x01fe01fe), "XOR unsigned wrong");
	ASSERT(vm.getProgramcounter() == 8, "programcounter wrong");

	mem->clear();
	uint8_t program2[] = {VM_INSTRUCTION_XOR, VM_OPERAND_TYPE_UINT16 | VM_LITERAL, 0x20, 0x00, 0xff, 0x00, VM_INSTRUCTION_HALT};
	mem->storeaddress(0x0020, 0x0101);
	vm.setProgram(program2, 7);
	vm.executeStep();
	ASSERT(mem->loadaddress(0x0020) == (0x01fe), "XOR address wrong");
	ASSERT(vm.getProgramcounter() == 6, "programcounter wrong");

	mem->clear();
	uint8_t program3[] = {VM_INSTRUCTION_XOR, VM_OPERAND_TYPE_UINT8 | VM_LITERAL, 0x20, 0x00, 0xf0, VM_INSTRUCTION_HALT};
	mem->store(0x0020, 0x11);
	vm.setProgram(program3, 6);
	vm.executeStep();
	ASSERT(mem->load(0x0020) == (0xe1), "XOR uin8_t wrong");
	ASSERT(vm.getProgramcounter() == 5, "programcounter wrong");

	ASSERT((vm.getStatuscode() & VM_ERROR_MASK) == 0, "VM shouldnt have an error");
}

inline void test_CalculationVM_LSHIFT()
{
	Memory* mem = &Memory::instance();
	mem->clear();
	VM vm(mem, pids);

	uint8_t program[] = {VM_INSTRUCTION_LSHIFT, VM_OPERAND_TYPE_UINT32 | VM_LITERAL, 0x20, 0x00, 0x02, 0x00, 0x00, 0x00, VM_INSTRUCTION_HALT};
	mem->storeunsigned(0x0020, 0x00000001);
	vm.setProgram(program, 9);
	vm.executeStep();
	ASSERT(mem->loadunsigned(0x0020) == (0x00000004), "LSHIFT unsigned wrong");
	ASSERT(vm.getProgramcounter() == 8, "programcounter wrong");

	ASSERT((vm.getStatuscode() & VM_ERROR_MASK) == 0, "VM shouldnt have an error");

	mem->clear();
	uint8_t program2[] = {VM_INSTRUCTION_LSHIFT, VM_OPERAND_TYPE_UINT16 | VM_LITERAL, 0x20, 0x00, 0x02, 0x00, VM_INSTRUCTION_HALT};
	mem->storeaddress(0x0020, 0x0001);
	vm.setProgram(program2, 7);
	vm.executeStep();
	ASSERT(mem->loadaddress(0x0020) == (0x0004), "LSHIFT address wrong");
	ASSERT(vm.getProgramcounter() == 6, "programcounter wrong");

	ASSERT((vm.getStatuscode() & VM_ERROR_MASK) == 0, "VM shouldnt have an error");

	mem->clear();
	uint8_t program3[] = {VM_INSTRUCTION_LSHIFT, VM_OPERAND_TYPE_UINT8 | VM_LITERAL, 0x20, 0x00, 0x02, VM_INSTRUCTION_HALT};
	mem->store(0x0020, 0x01);
	vm.setProgram(program3, 6);
	vm.executeStep();
	ASSERT(mem->load(0x0020) == (0x04), "LSHIFT uin8_t wrong");
	ASSERT(vm.getProgramcounter() == 5, "programcounter wrong");

	ASSERT((vm.getStatuscode() & VM_ERROR_MASK) == 0, "VM shouldnt have an error");
}

inline void test_CalculationVM_RSHIFT()
{
	Memory* mem = &Memory::instance();
	mem->clear();
	VM vm(mem, pids);

	uint8_t program[] = {VM_INSTRUCTION_RSHIFT, VM_OPERAND_TYPE_UINT32 | VM_LITERAL, 0x20, 0x00, 0x02, 0x00, 0x00, 0x00, VM_INSTRUCTION_HALT};
	mem->storeunsigned(0x0020, 0x00000004);
	vm.setProgram(program, 9);
	vm.executeStep();
	ASSERT(mem->loadunsigned(0x0020) == (0x00000001), "RSHIFT unsigned wrong");
	ASSERT(vm.getProgramcounter() == 8, "programcounter wrong");

	ASSERT((vm.getStatuscode() & VM_ERROR_MASK) == 0, "VM shouldnt have an error");

	mem->clear();
	uint8_t program2[] = {VM_INSTRUCTION_RSHIFT, VM_OPERAND_TYPE_UINT16 | VM_LITERAL, 0x20, 0x00, 0x02, 0x00, VM_INSTRUCTION_HALT};
	mem->storeaddress(0x0020, 0x0004);
	vm.setProgram(program2, 7);
	vm.executeStep();
	ASSERT(mem->loadaddress(0x0020) == (0x0001), "RSHIFT address wrong");
	ASSERT(vm.getProgramcounter() == 6, "programcounter wrong");

	ASSERT((vm.getStatuscode() & VM_ERROR_MASK) == 0, "VM shouldnt have an error");

	mem->clear();
	uint8_t program3[] = {VM_INSTRUCTION_RSHIFT, VM_OPERAND_TYPE_UINT8 | VM_LITERAL, 0x20, 0x00, 0x02, VM_INSTRUCTION_HALT};
	mem->store(0x0020, 0x04);
	vm.setProgram(program3, 6);
	vm.executeStep();
	ASSERT(mem->load(0x0020) == (0x01), "RSHIFT uin8_t wrong");
	ASSERT(vm.getProgramcounter() == 5, "programcounter wrong");

	ASSERT((vm.getStatuscode() & VM_ERROR_MASK) == 0, "VM shouldnt have an error");
}

inline void test_CalculationVM_LOAD()
{

	Memory* mem = &Memory::instance();
	mem->clear();
	VM vm(mem, pids);
	uint8_t program[] = {VM_INSTRUCTION_LOAD,  VM_OPERAND_TYPE_UINT8 | VM_LITERAL, 0x20, 0x00, 0xff,
			VM_INSTRUCTION_LOAD, VM_OPERAND_TYPE_UINT16 | VM_LITERAL, 0x21, 0x00, 0x11, 0x11,
			VM_INSTRUCTION_LOAD, VM_OPERAND_TYPE_UINT32 | VM_LITERAL, 0x23, 0x00, 0x22, 0x22, 0x22, 0x22,
			VM_INSTRUCTION_LOAD, VM_OPERAND_TYPE_DEC | VM_LITERAL, 0x27, 0x00, 0x00, 0x00, 0x00, 0x00, VM_INSTRUCTION_HALT};
	rational_t temp(1.5);
	memcpy(program + 23, &temp, sizeof(rational_t));
	vm.setProgram(program, 28);
	vm.executeStep();
	ASSERT(mem->load(0x0020) == 0xff, "unt8_t load wrong");
	ASSERT(mem->loadaddress(0x0021) == 0, "uint16_t load wrong");
	ASSERT(mem->loadunsigned(0x0023) == 0, "uint32_t load wrong");
	ASSERT(mem->loadrational(0x0027) == 0, "decimal_t load wrong");
	ASSERT(vm.getProgramcounter() == 5, "programcounter wrong");
	vm.executeStep();
	ASSERT(mem->load(0x0020) == 0xff, "unt8_t load wrong");
	ASSERT(mem->loadaddress(0x0021) == 0x1111, "uint16_t load wrong");
	ASSERT(mem->loadunsigned(0x0023) == 0, "uint32_t load wrong");
	ASSERT(mem->loadrational(0x0027) == 0, "decimal_t load wrong");
	ASSERT(vm.getProgramcounter() == 11, "programcounter wrong");
	vm.executeStep();
	ASSERT(mem->load(0x0020) == 0xff, "unt8_t load wrong");
	ASSERT(mem->loadaddress(0x0021) == 0x1111, "uint16_t load wrong");
	ASSERT(mem->loadunsigned(0x0023) == 0x22222222, "uint32_t load wrong");
	ASSERT(mem->loadrational(0x0027) == 0x0000, "decimal_t load wrong");
	ASSERT(vm.getProgramcounter() == 19, "programcounter wrong");
	vm.executeStep();
	ASSERT(mem->load(0x0020) == 0xff, "unt8_t load wrong");
	ASSERT(mem->loadaddress(0x0021) == 0x1111, "uint16_t load wrong");
	ASSERT(mem->loadunsigned(0x0023) == 0x22222222, "uint32_t load wrong");
	ASSERT(mem->loadrational(0x0027) == (rational_t)1.5, "decimal_t load wrong");
	ASSERT(vm.getProgramcounter() == 27, "programcounter wrong");

	ASSERT((vm.getStatuscode() & VM_ERROR_MASK) == 0, "VM shouldnt have an error");
}

inline void test_CalculationVM_MULTILOAD_decimal()
{

	Memory* mem = &Memory::instance();
	mem->clear();
	VM vm(mem, pids);
	uint8_t program[] = {VM_INSTRUCTION_MULTILOAD, VM_OPERAND_TYPE_DEC | VM_LITERAL, 0x20, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, VM_INSTRUCTION_HALT};
	rational_t temp1(1.234), temp2(100.4);
	memcpy(program + 5, &temp1, sizeof(rational_t));
	memcpy(program + 9, &temp2, sizeof(rational_t));
	vm.setProgram(program, 14);
	vm.executeStep();
	ASSERT(mem->loadrational(0x20) == (rational_t)1.234, "Multiload decimal first value wrong");
	ASSERT(mem->loadrational(0x20 + sizeof(rational_t)) == (rational_t)100.4, "Multiload decimal second value wrong");
	ASSERT(vm.getProgramcounter() == 13, "programcounter wrong");

	ASSERT((vm.getStatuscode() & VM_ERROR_MASK) == 0, "VM shouldnt have an error");
}

inline void test_CalculationVM_COPY()
{
	Memory* mem = &Memory::instance();
	mem->clear();
	VM vm(mem, pids);
	uint8_t program[] = {VM_INSTRUCTION_COPY, 0x10, 0x00, 0x04, 0x14, 0x00, VM_INSTRUCTION_HALT};
	mem->storeunsigned(0x0010, 0xabcdef01);
	vm.setProgram(program, 7);
	ASSERT(mem->loadunsigned(0x0010) == 0xabcdef01, "COPY init before wrong");
	ASSERT(mem->loadunsigned(0x0014) == 0x00000000, "COPY before wrong (Memory error)");
	vm.executeStep();
	ASSERT(mem->loadunsigned(0x0010) == 0xabcdef01, "COPY init after wrong");
	ASSERT(mem->loadunsigned(0x0014) == 0xabcdef01, "COPY wrong");
	ASSERT(vm.getProgramcounter() == 6, "programcounter wrong");

	ASSERT((vm.getStatuscode() & VM_ERROR_MASK) == 0, "VM shouldnt have an error");
}

inline void test_CalculationVM_JUMP()
{
	Memory* mem = &Memory::instance();
	mem->clear();
	VM vm(mem, pids);
	uint8_t program[] = {VM_INSTRUCTION_JUMP, VM_LITERAL, 0x05, 0x00, VM_INSTRUCTION_HALT, VM_INSTRUCTION_JUMP, VM_ADDRESS, 0x0a, 0x00, VM_INSTRUCTION_HALT, 0x09, 0x00};
	vm.setProgram(program, 12);
	vm.executeStep();//jumps to address 5
	ASSERT(vm.getProgramcounter() == 5, "programcounter wrong");

	vm.executeStep();//jumps to address specified in 0x0010 -> 0x0009
	ASSERT(vm.getProgramcounter() == 9, "programcounter wrong");
	vm.executeStep();
	ASSERT(vm.halted(), "VM not in halt state");
	ASSERT(vm.getProgramcounter() == 9, "programcounter wrong");

	ASSERT((vm.getStatuscode() & VM_ERROR_MASK) == 0, "VM shouldnt have an error");
}

inline void test_CalculationVM_COMPARE()
{
	Memory* mem = &Memory::instance();
	mem->clear();
	VM vm(mem, pids);
	uint8_t program[] = {VM_INSTRUCTION_COMPARE, VM_OPERAND_TYPE_DEC | VM_LITERAL, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x00, 0x22, 0x00, 0x33, 0x00, VM_INSTRUCTION_HALT};
	rational_t temp(12.3);
	memcpy(program + 4, &temp, sizeof(rational_t));
	mem->storerational(0x0010, temp);
	vm.setProgram(program, 15);
	vm.executeStep();
	ASSERT(vm.getProgramcounter() == 0x0022, "COMPARE, wrong jump address (equal)");

	ASSERT((vm.getStatuscode() & VM_ERROR_MASK) == 0, "VM shouldnt have an error");

	uint8_t program2[] = {VM_INSTRUCTION_COMPARE, VM_OPERAND_TYPE_DEC | VM_LITERAL, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x00, 0x22, 0x00, 0x33, 0x00, VM_INSTRUCTION_HALT};
	memcpy(program2 + 4, &temp, sizeof(rational_t));
	mem->storerational(0x0010, temp + 1);
	vm.setProgram(program2, 15);
	vm.executeStep();
	ASSERT(vm.getProgramcounter() == 0x0033, "COMPARE, wrong jump address (value @address should be greater than @operand)");

	ASSERT((vm.getStatuscode() & VM_ERROR_MASK) == 0, "VM shouldnt have an error");

	uint8_t program3[] = {VM_INSTRUCTION_COMPARE, VM_OPERAND_TYPE_DEC | VM_LITERAL, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x00, 0x22, 0x00, 0x33, 0x00, VM_INSTRUCTION_HALT};
	memcpy(program3 + 4, &temp, sizeof(rational_t));
	mem->storerational(0x0010, temp - 1);
	vm.setProgram(program3, 15);
	vm.executeStep();
	ASSERT(vm.getProgramcounter() == 0x0011, "COMPARE, wrong jump address (value @address should be smaller than @operand)");

	ASSERT((vm.getStatuscode() & VM_ERROR_MASK) == 0, "VM shouldnt have an error");
}

inline void test_CalculationVM_CALL_RETURN()
{
	Memory* mem = &Memory::instance();
	mem->clear();
	VM vm(mem, pids);
	uint8_t program[] = {VM_INSTRUCTION_CALL, 0x07, 0x00, VM_INSTRUCTION_CALL, 0x07, 0x00, VM_INSTRUCTION_HALT, VM_INSTRUCTION_RETURN};
	vm.setProgram(program, 8);
	ASSERT(vm.getStack()->isEmpty(), "Stack should be empty");
	vm.executeStep();//first call
	ASSERT(vm.getProgramcounter() == 0x0007, "CALL to wrong address");
	ASSERT(vm.getStack()->peek() == 0x0003, "Wrong Return address on stack");


	vm.executeStep();//first return
	ASSERT(vm.getProgramcounter() == 0x0003, "CALL to wrong address");
	ASSERT(vm.getStack()->isEmpty(), "Stack should be empty");


	vm.executeStep();//second call
	ASSERT(vm.getProgramcounter() == 0x0007, "CALL to wrong address");
	ASSERT(vm.getStack()->peek() == 0x0006, "Wrong Return address on stack");


	vm.executeStep();//second return
	ASSERT(vm.getStack()->isEmpty(), "Stack should be empty");

	vm.executeStep();//halt
	ASSERT(vm.halted(), "VM should be halted");
	ASSERT(vm.getProgramcounter() == 6, "programcounter wrong");

	ASSERT((vm.getStatuscode() & VM_ERROR_MASK) == 0, "VM shouldnt have an error");
}

inline void test_CalculationVM_TIME()
{
	Memory* mem = &Memory::instance();
	mem->clear();
	VM vm(mem, pids);
	uint8_t program[] = {VM_INSTRUCTION_TIME, 0x10, 0x00, VM_INSTRUCTION_HALT};
	vm.setProgram(program, 4);
	uint32_t before = xtimer_now();//we µs
	vm.executeStep();
	uint32_t after = xtimer_now();//again µs
	uint32_t memoryvalue = mem->loadunsigned(0x0010);
	ASSERT(memoryvalue >= before && memoryvalue <= after, "TIME stored wrong value");
	ASSERT(vm.getProgramcounter() == 3, "programcounter wrong");

	ASSERT((vm.getStatuscode() & VM_ERROR_MASK) == 0, "VM shouldnt have an error");
}

inline void test_CalculationVM_COMPARETIME()
{

	Memory* mem = &Memory::instance();
	mem->clear();
	VM vm(mem, pids);
	uint8_t program[] = {VM_INSTRUCTION_TIME, 0x20, 0x00, VM_INSTRUCTION_COMPARETIME, 0x20, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x0e, 0x00, VM_INSTRUCTION_COMPARETIME, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x19, 0x00, 0x00, 0x00, VM_INSTRUCTION_HALT};
	vm.setProgram(program, 26);
	vm.executeStep();//store systemtime in 0x0020
	xtimer_usleep(10000);
	vm.executeStep();//since we slept 10ms timeout of comparetime1 in not over -> jump to jaddress2 (comparetime2)
	ASSERT(vm.getProgramcounter() == 14, "programcounter wrong");

	vm.executeStep();//since timeout in comparetime2 is 0 we always jump to jaddress1
	ASSERT(vm.getProgramcounter() == 25, "programcounter wrong");

	ASSERT((vm.getStatuscode() & VM_ERROR_MASK) == 0, "VM shouldnt have an error");
}

inline void test_CalculationVM_PID_init()
{

	Memory* mem = &Memory::instance();
	mem->clear();
	VM vm(mem, pids);
	uint8_t program[] = {VM_INSTRUCTION_PIDINIT, 0x00,
			0x10, 0x00, 0x14, 0x00, 0x18, 0x00,
			0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
			0x64, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, VM_INSTRUCTION_HALT};
	rational_t upperlimit(255);
	memcpy(program + 28, &upperlimit, sizeof(rational_t));
	vm.setProgram(program, 34);
	vm.executeStep();
	ASSERT((vm.getStatuscode() & VM_ERROR_MASK) == 0, "Error on execute pidinit");
	ASSERT(pids[0].getLowerLimit() == (rational_t)0, "lower Limit wrong");
	ASSERT(pids[0].getUpperLimit() == (rational_t)255, "upper Limit wrong");
	ASSERT(pids[0].getDirection() == PID_DIRECTION_DIRECT, "direction wrong");
	ASSERT(vm.getProgramcounter() == 33, "programcounter wrong");
	ASSERT((vm.getStatuscode() & VM_ERROR_MASK) == 0, "VM shouldnt have an error");
}

inline void test_CalculationVM_PID_run()
{

	Memory* mem = &Memory::instance();
	mem->clear();
	VM vm(mem, pids);
	uint8_t program[] = {VM_INSTRUCTION_PIDRUN, 0x00, VM_INSTRUCTION_HALT};
	vm.setProgram(program, 3);
	vm.executeStep();
	ASSERT(pids[0].getMode() == PID_AUTOMATIC, "Pid not startet");
	ASSERT(vm.getProgramcounter() == 2, "programcounter wrong");
	ASSERT((vm.getStatuscode() & VM_ERROR_MASK) == 0, "VM shouldnt have an error");
}

inline void test_CalculationVM_PID_stop()
{

	Memory* mem = &Memory::instance();
	mem->clear();
	VM vm(mem, pids);
	uint8_t program[] = {VM_INSTRUCTION_PIDSTOP, 0x00, VM_INSTRUCTION_HALT};
	vm.setProgram(program, 3);
	vm.executeStep();
	ASSERT(pids[0].getMode() == PID_MANUAL, "Pid not stopped");
	ASSERT(vm.getProgramcounter() == 2, "programcounter wrong");
	ASSERT((vm.getStatuscode() & VM_ERROR_MASK) == 0, "VM shouldnt have an error");
}

inline void test_CalculationVM_PID_clear()
{

	Memory* mem = &Memory::instance();
	mem->clear();
	VM vm(mem, pids);
	uint8_t program[] = {VM_INSTRUCTION_PIDCLEAR, 0x00, VM_INSTRUCTION_HALT};
	vm.setProgram(program, 3);
	ASSERT(pids[0].isInitialized() == true, "PID is not initialized (not running)");
	vm.executeStep();
	ASSERT(pids[0].isInitialized() == false, "PID is still initialized (still running)");
	ASSERT(vm.getProgramcounter() == 2, "programcounter wrong");
	ASSERT((vm.getStatuscode() & VM_ERROR_MASK) == 0, "VM shouldnt have an error");
}

inline void test_CalculationVM_URLMAP()
{

	Memory* mem = &Memory::instance();
	mem->clear();
	VM vm(mem, pids);
	uint8_t program[] = {VM_INSTRUCTION_URLMAP, 0x00 | VM_OPERAND_TYPE_UINT32, VM_MAP_OPTION_RESOURCE_LITERAL | VM_MAP_OPTION_URL_LITERAL | VM_MAP_OPTION_METHOD_GET | VM_MAP_OPTION_DIRECTION_CLIENT, 0x30, 0x00, 0x00, 0x00, 0x66, 0x64, 0x30, 0x30, 0x3a, 0x3a, 0x66,
			0x65, 0x61, 0x61, 0x3a, 0x31, 0x32, 0x33, 0x34, 0x3a, 0x31, 0x32, 0x33, 0x00, 0x2f, 0x73, 0x65, 0x6e, 0x73, 0x6f, 0x72, 0x00, VM_INSTRUCTION_HALT
	};
	vm.setProgram(program, 36);
	vm.executeStep();
	const unsigned char* temp = mem->loadurl((mem->dumpMap())[0].url_address);
	const char* address = "fd00::feaa:1234:123";
	const unsigned char* temp2 = mem->loadurl((mem->dumpMap())[0].resource_address);
	const char* resource = "/sensor";
	ASSERT(strcmp((const char*)temp, (const char*)address) == 0, "URL mapping wrong");
	ASSERT(strcmp((const char*)temp2, (const char*)resource) == 0, "Resource mapping wrong");
	ASSERT(vm.getProgramcounter() == 35, "programcounter wrong");

	ASSERT((vm.getStatuscode() & VM_ERROR_MASK) == 0, "VM shouldnt have an error");
}

inline void test_CalculationVM_URLMAPCHECK()
{
	Memory* mem = &Memory::instance();
	mem->clear();
	VM vm(mem, pids);
	uint8_t program[] = {VM_INSTRUCTION_URLMAPCHECK, 0x00, 0x06, 0x00, VM_INSTRUCTION_HALT};
	vm.setProgram(program, 5);
	//we assume there is a mapping in memory
	mem->map(0, VM_OPERAND_TYPE_UINT16, VM_MAP_OPTION_LIFETIME_ONCE | VM_MAP_OPTION_URL_LITERAL | VM_MAP_OPTION_DIRECTION_CLIENT | VM_MAP_OPTION_METHOD_GET, 0x0020, 0, 0x0030, 0x0032);//actually not working, but irrelevant for MAPCHECK
	vm.executeStep();
	ASSERT(mem->load(0x0006) == 0, "Mapcheck should be false");
	ASSERT(vm.getProgramcounter() == 4, "programcounter wrong");

	ASSERT((vm.getStatuscode() & VM_ERROR_MASK) == 0, "VM shouldnt have an error");

	mem->map_error(0, VM_MAP_STATUS_ERROR_404);
	vm.setProgram(program, 5);
	vm.executeStep();
	ASSERT(mem->load(0x0006) > 1, "Mapcheck should return error");
	ASSERT(mem->load(0x0006) & VM_MAP_STATUS_ERROR_404, "Mapcheck should return error 404");
	ASSERT(vm.getProgramcounter() == 4, "programcounter wrong");

	ASSERT((vm.getStatuscode() & VM_ERROR_MASK) == 0, "VM shouldnt have an error");

	mem->map_done(0);
	vm.setProgram(program, 5);
	vm.executeStep();
	ASSERT(mem->load(0x0006) & VM_MAP_STATUS_DONE, "Mapcheck should be true");
	ASSERT(vm.getProgramcounter() == 4, "programcounter wrong");
	ASSERT(mem->dumpMap()[0].url_address == NO_MAPPING, "Map should be deleted after done when lifetime is once");

	ASSERT((vm.getStatuscode() & VM_ERROR_MASK) == 0, "VM shouldnt have an error");
}

inline void test_CalculationVM_URLMAPDELETE()
{
	Memory* mem = &Memory::instance();
	mem->clear();
	VM vm(mem, pids);
	uint8_t program[] = {VM_INSTRUCTION_URLMAPDELETE, 0x00, VM_INSTRUCTION_HALT};
	vm.setProgram(program, 3);
	//we assume there is a mapping in memory
	mem->map(0, VM_OPERAND_TYPE_UINT16, VM_MAP_OPTION_LIFETIME_ONCE | VM_MAP_OPTION_URL_LITERAL | VM_MAP_OPTION_DIRECTION_CLIENT | VM_MAP_OPTION_METHOD_GET, 0x0020, 0, 0x0030, 0x0032);//actually not working, but irrelevant for MAPCHECK
	vm.executeStep();
	ASSERT(mem->dumpMap()[0].url_address == NO_MAPPING, "Map should be deleted");
	ASSERT(vm.getProgramcounter() == 2, "programcounter wrong");

	ASSERT((vm.getStatuscode() & VM_ERROR_MASK) == 0, "VM shouldnt have an error");
}

inline void test_CalculationVM_HALT()
{
	//Although implicitlydone in all other test here is an explicit HALT instruction test
	Memory* mem = &Memory::instance();
	mem->clear();
	VM vm(mem, pids);
	uint8_t program[] = {VM_INSTRUCTION_HALT, VM_INSTRUCTION_HALT};
	vm.setProgram(program, 2);
	vm.executeStep();
	ASSERT(vm.getProgramcounter() == 0, "programcounter wrong");
	ASSERT(vm.halted(), "VM should be in halt state");

	vm.executeStep();
	ASSERT(vm.getProgramcounter() == 0, "programcounter wrong");
	ASSERT(vm.halted(), "VM should be in halt state");

	ASSERT((vm.getStatuscode() & VM_ERROR_MASK) == 0, "VM shouldnt have an error");
}

inline void test_CalculationVM_RESET()
{
	Memory* mem = &Memory::instance();
	mem->clear();
	VM vm(mem, pids);
	uint8_t program[] = {VM_INSTRUCTION_RESET};
	vm.setProgram(program, 1);
	vm.executeStep();
	ASSERT(vm.getProgramcounter() == 0, "programcounter wrong");
	ASSERT(vm.halted(), "VM should be in halt state");

	ASSERT(vm.getStatuscode() & VM_ERROR_RESET, "VM have a suicide error");
}

/**
 * @brief Runs all test functions specified. Acts as a test-suite.
 */
inline void test_CalculationVM()
{
#ifndef TEST_CALCULATION_OFF
	test_CalculationVM_setProgram();

	test_CalculationVM_ADD_unsigned();
	test_CalculationVM_ADD_decimal();
	test_CalculationVM_SUB_unsigned();
	test_CalculationVM_SUB_decimal();
	test_CalculationVM_MUL_unsigned();
	test_CalculationVM_MUL_decimal();
	test_CalculationVM_DIV_unsigned();
	test_CalculationVM_DIV_decimal();
	test_CalculationVM_DIV_unsigned_zero();
	test_CalculationVM_DIV_decimal_zero();
	test_CalculationVM_MOD_unsigned();
	test_CalculationVM_MOD_decimal();

	test_CalculationVM_AND();
	test_CalculationVM_OR();
	test_CalculationVM_NOT();
	test_CalculationVM_XOR();

	test_CalculationVM_LSHIFT();
	test_CalculationVM_RSHIFT();

	test_CalculationVM_LOAD();
	test_CalculationVM_MULTILOAD_decimal();

	test_CalculationVM_COPY();

	test_CalculationVM_JUMP();
	test_CalculationVM_COMPARE();

	test_CalculationVM_CALL_RETURN();

	test_CalculationVM_TIME();
	test_CalculationVM_COMPARETIME();

	test_CalculationVM_URLMAP();
	test_CalculationVM_URLMAPCHECK();
	test_CalculationVM_URLMAPDELETE();

	test_CalculationVM_PID_init();
	test_CalculationVM_PID_run();//must be after pid init
	test_CalculationVM_PID_stop();//mus be after pid run
	test_CalculationVM_PID_clear();

	test_CalculationVM_HALT();
	test_CalculationVM_RESET();

#else
	TESTINFO("Test CalculationVM off");
#endif
}

#endif /* TESTCALCULATIONVM_H_ */
