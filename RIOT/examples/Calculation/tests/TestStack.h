/*
 * Copyright (C) 2017 Mattes Besuden
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @brief       Tests for Stack.h
 *
 * @author      Mattes Besuden <besuden@uni-bremen.de>
 */
#ifndef TESTS_TESTSTACK_H_
#define TESTS_TESTSTACK_H_

#include "Tests.h"
#include "Stack.h"

inline void test_Stack_push()
{
	Stack stack;
	stack.push(0x1234);
	const uint16_t* dump = stack.dump();
	ASSERT(dump[0] == 0x1234, "push(...) error");
}

inline void test_Stack_pop()
{
	Stack stack;
	stack.push(0x1234);
	ASSERT(stack.pop() == 0x1234, "pop() error");
}

inline void test_Stack_peek()
{
	Stack stack;
	stack.push(0x1234);
	ASSERT(stack.peek() == 0x1234, "peek() error");
	ASSERT(stack.pop() == 0x1234, "peek deletes value");
}

inline void test_Stack_overflow()
{
	Stack stack;
	for(uint8_t i = 0; i < stack.getStackSize(); i++)//fill stack with values
	{
		stack.push(i);
	}
	try
	{
		stack.push(0x1234);
	}
	catch(std::overflow_error& e)
	{
		return;
	}
	ASSERTFALSE("No Overflow exception");
}

inline void test_Stack_underflow()
{
	Stack stack;
	try
	{
		stack.pop();
	}
	catch(std::underflow_error& e)
	{
		return;
	}
	ASSERTFALSE("No underflow exception");
}

inline void test_Stack_full()
{
	Stack stack;
	ASSERT(!stack.isFull(), "Stack should not be full");
	for(uint8_t i = 0; i < stack.getStackSize(); i++)
	{
		stack.push(i);
	}
	ASSERT(stack.isFull(), "Stack should be full");
	stack.pop();
	ASSERT(!stack.isFull(), "Stack should not be full");
}

inline void test_Stack_empty()
{
	Stack stack;
	ASSERT(stack.isEmpty(), "Stack should be empty");
	stack.push(0x1234);
	ASSERT(!stack.isEmpty(), "Stack should not be empty");
	stack.pop();
	ASSERT(stack.isEmpty(), "Stack should be empty");
}

inline void test_Stack_clear()
{
	Stack stack;
	for(uint8_t i = 0; i < stack.getStackSize(); i++)
	{
		stack.push(0x1234);
		ASSERT(stack.peek() != 0, "Stack value is zero");
	}
	stack.clear();
	ASSERT(stack.isEmpty(), "Stack should be empty after clear");
	const uint16_t* dump = stack.dump();
	for(uint8_t i = 0; i < stack.getStackSize(); i++)
	{
		ASSERT(dump[i] == 0, "Stack values should be zero");
	}
}

inline void test_Stack()
{
#ifndef TEST_STACK_OFF
	test_Stack_push();
	test_Stack_pop();
	test_Stack_peek();
	test_Stack_overflow();
	test_Stack_underflow();
	test_Stack_full();
	test_Stack_empty();
	test_Stack_clear();

#else
	TESTINFO("Test Stack off");
#endif
}

#endif /* TESTS_TESTSTACK_H_ */
