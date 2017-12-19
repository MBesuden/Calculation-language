/*
 * Copyright (C) 2017 Mattes Besuden
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @brief       Configuration file for the calculation VM. Sets memory size, URL map size, stack size, available PID controllers and the decimal_t type.
 *
 * @author      Mattes Besuden <besuden@uni-bremen.de>
 */
#ifndef CALCULATIONCONFIG_H_
#define CALCULATIONCONFIG_H_


#include "fixedpoint.h"

///Use fixedpoint math
#define FIXEDTYPE
#ifdef FIXEDTYPE
	///Defines decimal_t as fixed8_t (24.8 fixepoint math)
	#define rational_t 			fixed8_t
#else
	///Defines decimal_t as float
	#define rational_t			float
#endif

///Defines VM-Stack sze
#define VM_STACK_SIZE			(20)

#ifdef TESTING
///Defines Memory size in Bytes for testing
#define VM_MEMORY_SIZE			(1024)
#else
#ifdef __arm__
///Defines Memory size in Bytes for arm processors
#define VM_MEMORY_SIZE			(1024)
#else
///Defines Memory size in Bytes
#define VM_MEMORY_SIZE			(UINT16_MAX)
#endif
#endif

///Thread IPC queue size
#define RCV_QUEUE_SIZE			(8)

///Defines Number of available PID Controllers
#define VM_PID_NUM_AVAILABLE	(3)

#endif /* CALCULATIONCONFIG_H_ */
