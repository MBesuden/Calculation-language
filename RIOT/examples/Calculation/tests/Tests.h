/*
 * Copyright (C) 2017 Mattes Besuden
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @brief       Tests for Calculation Example
 *
 * @author      Mattes Besuden <besuden@uni-bremen.de>
 */
#ifndef TESTS_H_
#define TESTS_H_
#include <stdio.h>
#include <stdexcept>

/**
 * @brief Prints information about failed tests.
 * @param severity Severity (ERROR, INFO, etc)
 * @param file File Name
 * @param line Line Number
 * @param func Function Name
 * @param msg The error message provided by assert macro.
 */
inline void aFailed(const char* severity, const char *file, int line, const char* func, const char *msg)
{
	printf("%s! File: %s:%d, %s: %s\n", severity, file, line, func, msg);
}

static uint32_t errors = 0;

/**
 * @brief counts the errors occurred during testing.
 */
inline void inc(void)
{
	errors++;
}

/**
 * @brief Macro for assertions on boolean expressions.
 */
#define ASSERT(expr, msg) \
    if (expr) \
        {} \
    else \
	{ \
        aFailed("ERROR", __FILE__, __LINE__, __PRETTY_FUNCTION__, msg); \
		inc(); \
	}

/**
 * @brief Macro for assertions on equality.
 */
#define ASSERTEQUAL(expr1, exp2, msg) \
    if (expr == expr2) \
        {} \
    else \
	{ \
        aFailed(__FILE__, __LINE__, __PRETTY_FUNCTION__, msg); \
		inc(); \
	}

/**
 * @brief Macro for assertions on inequality.
 */
#define ASSERTNEQUAL(expr1, exp2, msg) \
    if (expr != expr2) \
        {} \
    else \
	{ \
        aFailed("ERROR", __FILE__, __LINE__, __PRETTY_FUNCTION__, msg); \
		inc(); \
	}

/**
 * @brief Macro for assertion that the test ran in a wrong state.
 */
#define ASSERTFALSE(msg) \
        aFailed("ERROR", __FILE__, __LINE__, __PRETTY_FUNCTION__, msg); \
		inc()

/**
 * @brief Onyl prints out the message
 */
#define TESTINFO(msg) aFailed("INFO", __FILE__, __LINE__, __PRETTY_FUNCTION__, msg);

#include "TestCalculationVM.h"
#include "TestMemory.h"
#include "TestStack.h"
#include "TestPID.h"
#include "TestGcoapSharedMemoryFunctions.h"
#include "TestExamples.h"

/**
 * @brief runs the specified test functions. Measures time during test execution.
 */
inline void run_tests()
{
	printf("Running tests...\n");
	uint32_t time = xtimer_now();

	test_CalculationVM();
	test_Memory();
	test_Stack();
	test_PID();
	test_Gcoap_shared();
	test_examples();

	uint32_t usedtime = (xtimer_now() - time) / 1000;
	printf("Tests completed after %" PRIu32 "ms  %" PRIu32 " Errors\n", usedtime, errors);
}

#endif /* TESTS_H_ */
