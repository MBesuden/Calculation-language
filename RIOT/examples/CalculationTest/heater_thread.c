/*
 * heater_thread.c
 *
 *  Created on: 10.03.2017
 *      Author: mbesuden
 */

#include <math.h>
#include <stdlib.h>
#include "thread.h"
#include "msg.h"
#include "xtimer.h"

void print_report(void);

#define cp		(1005)			//J/(kg*K) Wärmekapazität Luft
#define m		(22.5 * 1.293)	//Masse Luft in kg Luftdichte (20° 1.293kg/m^3)//Ballon 2.974 m^3 //Raum 3*3*2.5 = 22.5
#define P		(4000)			//max Heizleistung in W (Standard Heizkörper 1.2kW)
#define k		(0.32)			//Außenwand aus Mauerziegeln (17,5 cm) mit Wärmedämmverbundsystem (PUR) 	30 cm 	ca. 0,32//Außenwand aus Beton ohne Wärmedämmung 	25 cm 	3,3;//8.5;//Ballonwand //Wärmedurchgangskoeff. W/(m^2*K)
#define a		(30)			//Wandfläche m^2 4 * (3 * 2.5)m^2 //Ballonfläche 10m^2
#define epsilon	(0.92)			//Beton 0.92-0.97 //Ballon 0.8; //Emissionskoeffizient
#define sigma	(5.67051E-8)	//SB Konstante W/(m^2*K^4)

#define KELVIN	(273.15)
#define TOKELVIN(x) (x + KELVIN)
#define TOCELSIUS(x) (x - KELVIN)
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))
#define BETWEEN(min, max, val) (((val)<(min))?(min):(((val)>(max))?(max):(val)))

static int percent = 0;
static double temp_out = 283.15;	//10°C
static double temp = 283.15;		//10°C

static double setpoint = 10;

void *heater_thread(void *args)
{

	printf("Heater thread started, pid: %" PRIkernel_pid "\n", thread_getpid());
	(void) args;
	while(1)
	{
		double delta_T = temp - temp_out;
		temp += 1/(cp * m) * ((P * percent / 100.0) - k * a * delta_T - epsilon * sigma * a * (pow(temp, 4) - pow(temp_out, 4)));
		xtimer_usleep(200000);
		print_report();
	}
	return 0;
}

double getTemperature(void)
{
	return TOCELSIUS(temp);
}

double getSetpoint(void)
{
	return setpoint;
}

void setSetpoint(double _setpoint)
{
	setpoint = _setpoint;
}

void setOutsideTemp(double _temp_out)
{
	temp_out = TOKELVIN(_temp_out);
}

double getOutsideTemp(void)
{
	return TOCELSIUS(temp_out);
}

void heater_on(int _percent) {
	percent = BETWEEN(0, 100, _percent);
}

void print_report(void)
{
	printf("out:\t%d.%04d\tset:\t%d.%04d\tP:\t%d\ttemp:\t%d.%04d\n",
			(int)getOutsideTemp(), abs(((int)(getOutsideTemp() * 10000))) % 10000,
			(int)getSetpoint(), abs(((int)(getSetpoint() * 10000))) % 10000,
			percent,
			(int)getTemperature(), abs(((int)(getTemperature() * 10000))) % 10000);
}

