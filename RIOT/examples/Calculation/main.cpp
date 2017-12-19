/*
 * Copyright (C) 2017 Mattes Besuden
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @brief       Proof of Concept implementation of a Calculation engine using CoAP, a Calculation VM and PID-Support
 *
 * @author      Mattes Besuden <mattes@besuden.de>
 */
#include <stdio.h>

extern "C" { //Alle C Funktionen über extern C einbinden (z.B. alle RIOT Funktionen siehe Riot und cpp example)
#include "thread.h"
#include "xtimer.h"
#include "msg.h"

#include "net/gnrc.h"
#include "net/ipv6/addr.h"
#include "net/gnrc/ipv6/netif.h"

extern void gcoap_s_init(kernel_pid_t vm_thread);
extern void *coap_c_thread(void *args);
extern int _netif_config(int argc, char **argv);
}

extern void *pid_thread(void *arg);
extern void *vm_thread(void *arg);

char pid_thread_stack[THREAD_STACKSIZE_MAIN];
char vm_thread_stack[THREAD_STACKSIZE_MAIN];
char coap_c_thread_stack[THREAD_STACKSIZE_MAIN];
#if defined (__arm__)
///handle which is required to cross compile for arm (workaround).
void* __dso_handle;
#endif

inline void add_global_address(void)
{
	kernel_pid_t interfaces[GNRC_NETIF_NUMOF];
	gnrc_netif_get(interfaces);
	char addr_str[] = "affe::2";
	ipv6_addr_t addr;
	uint8_t prefix_len = 128;
	if (ipv6_addr_from_str(&addr, addr_str) == NULL) {
		puts("error: unable to parse IPv6 address.");
		return;
	}
	if(gnrc_ipv6_netif_add_addr(interfaces[0], &addr, prefix_len, GNRC_IPV6_NETIF_ADDR_FLAGS_NON_UNICAST) == NULL)
	{
		puts("error: unable to add IPv6 address.");
	}
}

#ifndef TESTING
/**
 * Entry point for calculation VM device. Starts VM and PID thread, initializes gcoap client and server.
 * @return exit code
 */
int main(void)
{
    puts("Calculation VM Proof of Concept");
    printf("You are running RIOT on a(n) %s board.\n", RIOT_BOARD);
    printf("This board features a(n) %s MCU.\n", RIOT_MCU);

    add_global_address();

    //create VM Thread store process id for CoAP Server (needs to communicate for bytecode upload and vm start/restart)
    kernel_pid_t vm_pid = thread_create(vm_thread_stack, sizeof(vm_thread_stack),
    	                                    THREAD_PRIORITY_MAIN - 1, THREAD_CREATE_STACKTEST,
    	    								vm_thread, NULL, "vm");

    //create PID Thread
	thread_create(pid_thread_stack, sizeof(pid_thread_stack),
										THREAD_PRIORITY_MAIN - 2, THREAD_CREATE_STACKTEST,
										pid_thread, NULL, "pid");

	//create CoAP Client Thread
	thread_create(coap_c_thread_stack, sizeof(coap_c_thread_stack),
										THREAD_PRIORITY_MAIN - 3, THREAD_CREATE_STACKTEST,
										coap_c_thread, NULL, "coap client");

	//initialize CoAP Server
	gcoap_s_init(vm_pid);

	//work is done
	return 0;
}
#else
#include "Opcodes.h"
#include "tests/Tests.h"
/**
 * Main function for testing
 * @return exit code
 */
int main(void)
{
	puts("Calculation VM Proof of Concept Tests");
	printf("You are running RIOT on a(n) %s board.\n", RIOT_BOARD);
	printf("This board features a(n) %s MCU.\n", RIOT_MCU);
//
//	add_global_address();

	run_tests();
	//clear Memory after tests
	Memory::instance().clear();
	exit(0);
}
#endif
