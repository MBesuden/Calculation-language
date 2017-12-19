/*
 * Copyright (C) 2017 Mattes Besuden
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @brief       PID controller impelementation which operates on the shared memory.
 * 				The PID implementation is based on Arduino PID Library - Version 1.1.1 by Brett Beauregard <br3ttb@gmail.com> brettbeauregard.com (This Library is licensed under a GPLv3 License)
 *				The implementation was chenged to work on shared memory and to use the decimal_t datatype.
 *				Also a lazy initialization method was introduced.
 *
 * @author      Mattes Besuden <besuden@uni-bremen.de>
 */
#ifndef PID_H_
#define PID_H_


#include "Memory.h"

class PID {

public:

	//define Constants
	///Manual Mode, output is constant
	#define PID_MANUAL 0
	///Automatic mode, output will be computed
	#define PID_AUTOMATIC 1
	///PID direction direct, output signedness equals PID parameter signedness
	#define PID_DIRECTION_DIRECT 0
	///PID direction reverse, output signedness is reversed with parameter signedness
	#define PID_DIRECTION_REVERSE 1

	PID();

	/**
	 * PID instances
	 * @return Array of shared PID instances
	 */
	static PID* instances(void)
	{
		//shared static pids(not initialized)
		static PID vm_pids[VM_PID_NUM_AVAILABLE];
		return vm_pids;
	}

	bool init(Memory* _memory, uint16_t _inputaddress, uint16_t _outputaddress, uint16_t _setpointaddress,
			rational_t _kp, rational_t _ki, rational_t _kd,
			uint32_t _sampleTime, uint8_t _direction,
			rational_t _lowerLimit, rational_t _upperLimit);
	void clear(void);

	void setMode(uint8_t mode); //PID Mode (MANUAL|AUTOMATIC)
	bool compute(void); //computes output
	void setOutputLimits(rational_t, rational_t); //limits pid output

	void setTunings(rational_t, rational_t, rational_t);
	void setControllerDirection(uint8_t);
	void setSampleTime(uint8_t);

	rational_t getKp(void);
	rational_t getKi(void);
	rational_t getKd(void);

	uint8_t getMode(void);
	uint8_t getDirection(void);
	rational_t getUpperLimit(void);
	rational_t getLowerLimit(void);

	bool isInitialized(void);


private:

	void initialize(void);

	Memory* memory;
	uint8_t map_id;

	rational_t kp;
	rational_t ki;
	rational_t kd;

	rational_t outMin, outMax;

	uint16_t inputaddress;
	uint16_t outputaddress;
	uint16_t setpointaddress;

	rational_t iSum, lastInput;

	uint8_t direction;
	bool inAuto;

	uint32_t lastTime;
	uint32_t sampleTime;

	bool initialized;

};

#endif /* PID_H_ */
