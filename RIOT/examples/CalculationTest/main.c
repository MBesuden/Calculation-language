/*
 * main.cpp
 *
 *  Created on: 10.03.2017
 *      Author: mbesuden
 */

#include "thread.h"
#include "shell.h"
#include "xtimer.h"

#include "net/gnrc.h"
#include "net/ipv6/addr.h"
#include "net/gnrc/ipv6/netif.h"

extern int _netif_config(int argc, char **argv);

char heater_thread_stack[THREAD_STACKSIZE_MAIN];

extern void* heater_thread(void* args);
extern void coap_init(void);


void add_global_address(void)
{
	kernel_pid_t interfaces[GNRC_NETIF_NUMOF];
	gnrc_netif_get(interfaces);
	char *addr_str = "affe::1";
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

int main(void)
{

	add_global_address();
	xtimer_sleep(3);
	puts("Configured network interfaces:");
	_netif_config(0, NULL);
	xtimer_sleep(1);

	thread_create(heater_thread_stack, sizeof(heater_thread_stack),
					THREAD_PRIORITY_MAIN - 1, THREAD_CREATE_STACKTEST,
					heater_thread, NULL, "Heater");

	coap_init();
//
//	char line_buf[SHELL_DEFAULT_BUFSIZE];
//	shell_run(NULL, line_buf, SHELL_DEFAULT_BUFSIZE);
	return 0;
}
