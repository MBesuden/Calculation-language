/*
 * Copyright (C) 2017 Mattes Besuden
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @brief       Tests for PID.h
 *
 * @author      Mattes Besuden <besuden@uni-bremen.de>
 */
#ifndef TESTS_TESTPID_H_
#define TESTS_TESTPID_H_

#include "Tests.h"
#include "PID.h"

inline void test_PID_init()
{
	Memory* mem = &Memory::instance();
	mem->clear();
	PID pid;
	pid.init(mem, 0, 4, 8, (rational_t)1, (rational_t)0.1, (rational_t)0.01, 100,PID_DIRECTION_DIRECT, (rational_t)0, (rational_t)255);
	ASSERT(pid.getMode() == PID_MANUAL, "Wrong PID mode");
	ASSERT(pid.getDirection() == PID_DIRECTION_DIRECT, "wrong PID direction");
	ASSERT(pid.getLowerLimit() == (rational_t)0, "wrong lower limit");
	ASSERT(pid.getUpperLimit() == (rational_t)255, "wrong upper Limit");
}

inline void test_PID_compute()
{
	TESTINFO("Testing PID, this may take a while...\n");
	Memory* mem = &Memory::instance();
	mem->clear();
	PID pid;
	pid.init(mem, 0, 4, 8, (rational_t)1, (rational_t)1, (rational_t)0.01, 100,PID_DIRECTION_DIRECT, (rational_t)-10, (rational_t)10);
	Memory::instance().storerational(8, (rational_t)1);
	pid.setMode(PID_AUTOMATIC);
	int i = 0;
	while(i<500)
	{
		pid.compute();
		Memory::instance().storerational(0,Memory::instance().loadrational(0) + Memory::instance().loadrational(4)/10);
		xtimer_usleep(30000);
		i++;
		if(i > 250)
			Memory::instance().storerational(8,(rational_t)5);
	}
	ASSERT(Memory::instance().loadrational(0) > 4 && Memory::instance().loadrational(0) < 6, "Input not in range, should be around 5");
}

inline void test_PID_MANUAL_mode()
{
	Memory* mem = &Memory::instance();
	mem->clear();
	PID pid;
	pid.init(mem, 0, 4, 8, (rational_t)1, (rational_t)1, (rational_t)0.01, 100,PID_DIRECTION_DIRECT, (rational_t)-10, (rational_t)10);
	Memory::instance().storerational(8, (rational_t)1);
	ASSERT(Memory::instance().loadrational(4) == 0, "Output value changed");
	pid.setMode(PID_AUTOMATIC);
	pid.compute();
	ASSERT(Memory::instance().loadrational(4) != 0, "Output value not computed");//should compute output value since setpoint != 0;
	rational_t temp = Memory::instance().loadrational(4);
	pid.setMode(PID_MANUAL);
	pid.compute();
	ASSERT(Memory::instance().loadrational(4) == temp, "Output value changed");
}

/**
 * @brief Runs all test functions specified. Acts as a test-suite.
 */
inline void test_PID()
{
//#define TEST_PID_OFF
#ifndef TEST_PID_OFF
	test_PID_init();
	test_PID_compute();
	test_PID_MANUAL_mode();
#else
	TESTINFO("Test PID off");
#endif
}


#endif /* TESTS_TESTPID_H_ */
