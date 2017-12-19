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
#include "mango.h"

static uint32_t after = 0;
static uint32_t before = 0;
static uint8_t memory[128];

static const uint8_t ggt_name[4] = { 0x67, 0x67, 0x74, 0x00, };
static const uint8_t ggt_code[36] = { 0xFF, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x02,
	    0x04, 0x01, 0x00, 0x00, 0x1E, 0x00, 0x66, 0x00,
	    0x33, 0xF4, 0x01, 0x00, 0x00, 0x1E, 0x01, 0x68,
	    0x00, 0x20, 0xF1, 0x00, 0x00, 0x00, 0x00, 0x01,
	0x1C, 0x08, 0x00, 0x02, };

int32_t compute(void) {

	mango_vm* vm = mango_initialize(memory, sizeof(memory), 32, NULL);
	if (!vm) {
		return -5;
	}


	if (mango_module_import(vm, ggt_name, ggt_code, sizeof(ggt_code), NULL,
			MANGO_IMPORT_TRUSTED_MODULE | MANGO_IMPORT_SKIP_VERIFICATION)) {
		return -6;
	}


	int32_t result = -2;
	before = xtimer_now();
	while (1) {
		switch (mango_execute(vm)) {
		default:
			result = -1; break;
		case MANGO_E_SUCCESS:
			result = 0; break;
		case MANGO_E_SYSCALL:
			result = -3; break;
		}
	}
	after = xtimer_now();
	return result;
}

int main(void) {
	xtimer_init();
	printf("starting ggt_Mango computation\n");
	printf("%s", (unsigned char *)ggt_name);

	int32_t return_val = 0;
	return_val = compute();

	uint32_t result = memory[0];

	printf("Ggt_Mango computation DONE\n");
	printf("return_val: %"PRId32", result: %"PRIu32" took %"PRIu32" µs\n", return_val, result, (after - before));
	return 0;
}
