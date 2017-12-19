/*
 * Copyright (C) 2017 Mattes Besuden
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @brief       Implementation of Stack
 *
 * @author      Mattes Besuden <besuden@uni-bremen.de>
 */

#include <string.h>
#include <stdexcept>
#include "Stack.h"

/**
 *	@brief Stack is a class used by the VM to store addresses in case of CALL and RETURN instructions.
 */
Stack::Stack()
{
	pointer = 0;
	size = STACK_SIZE;
}

/**
 * @param address Pushes address on stack and incements stackpointer.
 */
void Stack::push(uint16_t address)
{
	if(pointer == size)
	{
		throw std::overflow_error("Stack Overflow");
	}
	this->stack[pointer++] = address;
}

/**
 *
 * @return Value on top of stack (value will be deleted after pop()). Decrements stackpointer.
 */
uint16_t Stack::pop(void)
{
	if(pointer == 0) {
		throw std::underflow_error("Stack Underflow");
	}
	return this->stack[--pointer];
}

/**
 *
 * @return Value on top of stack. Stackpointer will be untouched.
 */
uint16_t Stack::peek(void)
{
	if(pointer == 0) {
		throw std::underflow_error("Stack Underflow");
	}
	return this->stack[pointer - 1];
}

/**
 *
 * @return Readonly pointer to stack.
 */
const uint16_t* Stack::dump(void) const
{
	return this->stack;
}

/**
 *
 * @return Stack size.
 */
uint8_t Stack::getStackSize()
{
	return size;
}

/**
 *
 * @return True if stack is empty (stackpointer equals zero).
 */
bool Stack::isEmpty()
{
	return pointer == 0;
}

/**
 *
 * @return True if stack is full (stackpointer equals stacksize).
 */
bool Stack::isFull()
{
	return pointer == size;
}

/**
 * Clears stack.
 */
void Stack::clear()
{
	pointer = 0;
	memset(stack, 0, size * sizeof(uint16_t));
}
