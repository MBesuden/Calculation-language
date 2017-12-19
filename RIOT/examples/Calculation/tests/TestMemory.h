/*
 * Copyright (C) 2017 Mattes Besuden
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @brief       Tests for Memory.h
 *
 * @author      Mattes Besuden <besuden@uni-bremen.de>
 */
#ifndef TESTS_TESTMEMORY_H_
#define TESTS_TESTMEMORY_H_

#include "Tests.h"

inline void test_Memory_store_load()
{
	for(uint16_t i = 0; i < MEMORY_SIZE; i++)
	{
		Memory::instance().store(i, (uint8_t)i);
	}
	for(uint16_t i = 0; i < MEMORY_SIZE; i++)
	{
		ASSERT(Memory::instance().load(i) == (uint8_t)i, "Memory store == load");
	}
}

inline void test_Memory_storeaddress_loadaddress()
{

	for(uint16_t i = 0; i < (MEMORY_SIZE-2); i += 2)
	{
		Memory::instance().storeaddress(i, i);
	}
	for(uint16_t i = 0; i < (MEMORY_SIZE-2); i +=2)
	{
		ASSERT(Memory::instance().loadaddress(i) == i, "Memory storeaddress == loadaddress");
	}
}

inline void test_Memory_storeunsigned_loadunsigned()
{

	for(uint16_t i = 0; i < (MEMORY_SIZE-sizeof(uint32_t)); i += sizeof(uint32_t))
	{
		Memory::instance().storeunsigned(i, i);
	}
	for(uint16_t i = 0; i < (MEMORY_SIZE-sizeof(uint32_t)); i += sizeof(uint32_t))
	{
		ASSERT(Memory::instance().loadunsigned(i) == i, "Memory storeunsigned == loadunsigned");
	}
}

inline void test_Memory_storedecimal_loaddecimal()
{

	for(uint16_t i = 0; i < (MEMORY_SIZE-sizeof(rational_t)); i += sizeof(rational_t))
	{
		Memory::instance().storerational(i, (rational_t)i);
	}
	for(uint16_t i = 0; i < (MEMORY_SIZE-sizeof(rational_t)); i += sizeof(rational_t))
	{
		ASSERT(Memory::instance().loadrational(i) == (rational_t)i, "Memory storedecimal == loaddecimal");
	}
}

inline void test_Memory_copy()
{

	Memory::instance().store(0x0001, 1);
	Memory::instance().store(0x0002, 255);
	Memory::instance().copy(0x0001, 2, 0x0020);
	ASSERT(Memory::instance().load(0x0020) == 1, "Memory copy wrong value");
	ASSERT(Memory::instance().load(0x0021) == 255, "Memory copy wrong value");
}

inline void test_Memory_clear()
{

	Memory::instance().store(0x0000, 0xff);
	Memory::instance().store(0x0001, 0x20);
	ASSERT(Memory::instance().load(0x0000) == 0xff, "Memory clear not stored");
	ASSERT(Memory::instance().load(0x0001) == 0x20, "Memory clear not stored");
	Memory::instance().clear();
	ASSERT(Memory::instance().load(0x0000) == 0x00, "Memory not cleared");
	ASSERT(Memory::instance().load(0x0001) == 0x00, "Memory not cleared");
}

inline void test_Memory_access_violation()
{

	try
	{
		Memory::instance().load(Memory::instance().getMemorySize());
	}
	catch(std::exception& e)
	{
		return;
	}
	ASSERTFALSE("Memory acces violation no exception");
}

//inline void test_Memory_readonly_dump()
//{
//
//	try
//	{
//		const uint8_t* dump = Memory::instance().dump();
//		uint8_t* writelocation = dump;
//		writelocation[0] = 0x00;
//	}
//	catch(std::exception& e)
//	{
//		return;
//	}
//	ASSERTFALSE("Memory Dump write access");
//}


/**
 * @brief Runs all test functions specified. Acts as a test-suite.
 */
inline void test_Memory()
{
#ifndef TEST_MEMORY_OFF
	test_Memory_store_load();
	test_Memory_storeaddress_loadaddress();
	test_Memory_storeunsigned_loadunsigned();
	test_Memory_storedecimal_loaddecimal();
	test_Memory_copy();
	test_Memory_clear();
	test_Memory_access_violation();
#else
	TESTINFO("Test Memory off");
#endif
}


#endif /* TESTS_TESTMEMORY_H_ */
