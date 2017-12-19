/*
 * Copyright (C) 2017 Mattes Besuden
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @brief       Stack implementation used in the calculation VM. The stack is used to store jump addresses in CALL and RETURN instructions.
 *
 * @author      Mattes Besuden <besuden@uni-bremen.de>
 */
#ifndef STACK_H_
#define STACK_H_

#include "calculationconfig.h"

///Defines Stack Size
#define STACK_SIZE VM_STACK_SIZE

class Stack
{
public:
	Stack(void);
	~Stack(void) { }

	void push(uint16_t value);
	uint16_t pop(void);
	uint16_t peek(void);
	const uint16_t* dump(void) const;
	uint8_t getStackSize(void);
	bool isEmpty(void);
	bool isFull(void);
	void clear(void);
private:
	uint8_t size;
	uint16_t stack[STACK_SIZE];
	uint8_t pointer;
};


#endif /* STACK_H_ */
