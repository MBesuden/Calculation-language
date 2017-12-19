/*
 * main.c
 *
 *  Created on: 29.10.2017
 *      Author: mbesuden
 */

#include <stdio.h>
#include <inttypes.h>
#include "shell.h"
#include "xtimer.h"

int main(void)
{
	xtimer_init();
	printf("starting ggt computation\n");
	uint32_t a = 1836311903;
	uint32_t b = 1134903170;
	uint32_t after = 0;
	uint32_t before = xtimer_now();
	if(a == 0) {
		a = b;
	} else {
		while(b != 0) {
			if(a > b) {
				a = a - b;
			} else {
				b = b - a;
			}
		}
	}
	after = xtimer_now();
	printf("Ggt computation DONE\n");
	printf("%"PRIu32" after %"PRIu32" µs\n", a, (after - before));
	return 0;
}
