/*
 * Copyright (C) 2017 Mattes Besuden
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 *
 *
 *
 * Implementation of PID Controller based on Arduino PID Library - Version 1.1.1 by Brett Beauregard (GPLv3)
 */

/**
 * @brief       Implementation of PID Controller based on Arduino PID Library - Version 1.1.1 by Brett Beauregard (GPLv3)
 *
 * @author      Mattes Besuden <besuden@uni-bremen.de>
 */
#include <stdio.h>
#include "PID.h"
extern "C" {
#include "xtimer.h"
}

/**
 *	Creates a PID Controller (uninitialized) with zero values.
 *	Can only be used after initialization.
 */
PID::PID()
{
	lastTime = 0;
	direction = 0;
	lastInput = 0;
	iSum = 0;
	inputaddress = 0;
	outputaddress = 0;
	setpointaddress = 0;
	kp = 0;
	ki = 0;
	kd = 0;
	outMax = 0;
	outMin = 0;
	sampleTime = 0;
	inAuto = 0;
	memory = 0;
	initialized = false;
	map_id = 0xff;
}

/**
 * Public function which initializes the PID controller with the specified values.
 * @param _memory			Memory which is used for input, output and setpoint values.
 * @param _inputaddress		Address of the input value in _memory.
 * @param _outputaddress	Address of the output value in _memory.
 * @param _setpointaddress	Address of the setpint value in _memory.
 * @param _kp				Proportional gain value to use.
 * @param _ki				Incremental gain value to use.
 * @param _kd				Differential gain value to use.
 * @param _sampleTime		Sample time to use. New value is only computed if sample time has passed.
 * @param _direction		Direction of the PID (PID_DIRECTION_DIRECT or PID_DIRECTION_REVERSE). Determines if output value has same sign as error.
 * @param _lowerLimit		Lower limit of the PID output.
 * @param _upperLimit		Upper Limit of the PID output.
 */
bool PID::init(Memory* _memory, uint16_t _inputaddress, uint16_t _outputaddress, uint16_t _setpointaddress, rational_t _kp, rational_t _ki, rational_t _kd, uint32_t _sampleTime, uint8_t _direction, rational_t _lowerLimit, rational_t _upperLimit) {
	if(initialized)
	{
		return false;
	}
	memory = _memory;
	inputaddress = _inputaddress;
	outputaddress = _outputaddress;
	setpointaddress = _setpointaddress;

	inAuto = false;

	PID::setOutputLimits(_lowerLimit,_upperLimit);

	sampleTime = _sampleTime;
	if(sampleTime < 100)
	{
		sampleTime = 100;
	}

	PID::setControllerDirection(_direction);
	PID::setTunings(_kp,_ki,_kd);

	lastTime = (xtimer_now() / 1000) - sampleTime;
	initialized = true;
	map_id = memory->getMapForAddress(inputaddress);//check if we hit a mapped value so we can react on map errors during computation
	return true;
}

/**
 *	Clears the PID by setting the initialized flag to false (init(...) needs to be called again).
 */
void PID::clear()
{
	initialized = false;
}

/**
 * Computes a new output value if PID is initialized, in automatic mode and sample time is passed.
 * @return True if a new output value was computed, false if in PID_AUTOMATIC mode or not initialized.
 */
bool PID::compute(void) {
	if(!initialized || !inAuto)
	{
		return false;
	}
	if(map_id != 0xff && memory->checkmap(map_id, false) > 1)
	{
		//there is an error in the mapping, do not compute a new value
		return false;
	}
	uint32_t now = xtimer_now();
	uint32_t timeChange = (now - lastTime) / 1000;
	if(timeChange >= sampleTime)
	{
		/*Compute all the working error variables*/
		rational_t in = memory->loadrational(inputaddress);
		rational_t setpoint = memory->loadrational(setpointaddress);
		rational_t error = setpoint - in;
		iSum += (ki * error);
		if(iSum > outMax)
		{
		  iSum = outMax;//anti-windup
		}
		else if(iSum < outMin)
		{
		  iSum = outMin;
		}
		rational_t dInput = (in - lastInput);

		/*Compute PID Output*/
		rational_t out = kp * error + iSum - kd * dInput;

		if(out > outMax)
		{
		  out = outMax;
		}
		else if(out < outMin)
		{
		  out = outMin;
		}
		memory->storerational(outputaddress, out);

		/*Remember some variables for next time*/
		lastInput = in;
		lastTime = now;
#ifdef TESTING
		printf("in(%d): %f; set(%d): %f; err: %f; out(%d): %f;\n", inputaddress, (float)in, setpointaddress, (float)setpoint, (float)error, outputaddress, (float)out);
#endif
		return true;
	}
	else
	{
		return false;
	}
}

/**
 * Sets new PID parameters.
 * @param _kp New Kp value (will be recalculated with sampleTime).
 * @param _ki New Ki value (will be recalculated with sampleTime).
 * @param _kd New Kd value (will be recalculated with sampleTime).
 */
void PID::setTunings(rational_t _kp, rational_t _ki, rational_t _kd) {
	if(_kp < 0 || _ki < 0 || _kd < 0)
	{
		return;
	}
	double sampleTimeInSec = ((double)sampleTime)/1000;
	kp = _kp;
	ki = _ki * sampleTimeInSec;
	kd = _kd / sampleTimeInSec;

	if(direction == PID_DIRECTION_REVERSE)
	{
		kp = (0 - kp);
	    ki = (0 - ki);
	    kd = (0 - kd);
	}
}

/**
 * Sets a new sample time.
 * @param newSampleTime New sample time of the PID.
 */
void PID::setSampleTime(uint8_t newSampleTime)
{
   if (newSampleTime > 0)
   {
      double ratio  = (double)newSampleTime / (double)sampleTime;
      ki *= ratio;
      kd /= ratio;
      sampleTime = (uint32_t)newSampleTime;
   }
}

/**
 * Sets new output limits.
 * @param min New minimal output value.
 * @param max New maximal output value.
 */
void PID::setOutputLimits(rational_t min, rational_t max)
{
   if(min >= max) return;
   outMin = min;
   outMax = max;

   if(inAuto)
   {
	   if(memory->loadrational(outputaddress) > outMax)
	   {
		   memory->storerational(outputaddress, outMax);
	   }
	   else if(memory->loadrational(outputaddress) < outMin)
	   {
		   memory->storerational(outputaddress, outMin);
	   }

	   if(iSum > outMax) iSum= outMax;
	   else if(iSum < outMin) iSum= outMin;
   }
}

/**
 * Sets PID mode.
 * @param mode New mode of the PID (PID_AUTOMATIC or PID_MANUAL).
 */
void PID::setMode(uint8_t mode)
{
    bool newAuto = (mode == PID_AUTOMATIC);
    if(newAuto == !inAuto)
    {  /*we just went from manual to auto*/
        PID::initialize();
    }
    inAuto = newAuto;
}

/**
 *	Private function which initializes the PID.
 */
void PID::initialize(void)
{
   iSum = memory->loadrational(outputaddress);
   lastInput = memory->loadrational(inputaddress);
   if(iSum > outMax)
   {
	   iSum = outMax;
   }
   else if(iSum < outMin)
   {
	   iSum = outMin;
   }
}

/**
 * Sets PID controller direction.
 * @param _direction New direction of the PID (PID_DIRECTION_DIRECT or PID_DIRECTION_REVERSE).
 */
void PID::setControllerDirection(uint8_t _direction)
{
   if(inAuto && _direction != direction)
   {
	  kp = (0 - kp);
      ki = (0 - ki);
      kd = (0 - kd);
   }
   direction = _direction;
}

/**
 *
 * @return The actual Kp value (calculated with sampleTime).
 */
rational_t PID::getKp(void) { return  kp;}

/**
 *
 * @return The actual Ki value (calculated with sampleTime).
 */
rational_t PID::getKi(void) { return  ki;}

/**
 *
 * @return The actual Kd value (calculated with sampleTime).
 */
rational_t PID::getKd(void) { return  kd;}

/**
 *
 * @return Mode of the PID (PID_AUTOMATIC or PID_MANUAL).
 */
uint8_t PID::getMode(void) { return  inAuto ? PID_AUTOMATIC : PID_MANUAL;}

/**
 *
 * @return Direction of the PID (PID_DIRECTION_DIRECT or PID_DIRECTION_REVERSE).
 */
uint8_t PID::getDirection(void) { return direction;}

/**
 *
 * @return Upper Limit of the PID.
 */
rational_t PID::getUpperLimit(void) { return outMax;}

/**
 *
 * @return Lower Limit of the PID.
 */
rational_t PID::getLowerLimit(void) { return outMin;}

/**
 *
 * @return True if PID is initialized.
 */
bool PID::isInitialized(void) {return initialized;}
