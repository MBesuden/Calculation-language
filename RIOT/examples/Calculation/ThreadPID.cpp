/*
 * Copyright (C) 2017 Mattes Besuden
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @brief       Implementation of PID Thread
 *
 * @author      Mattes Besuden <besuden@uni-bremen.de>
 */
extern "C" {
#include "thread.h"
#include "msg.h"
#include "xtimer.h"
}
#include "PID.h"

//static msg_t pid_rcv_queue[RCV_QUEUE_SIZE];

/**
 * PID Thread function
 * @param arg
 */
void *pid_thread(void *arg)
{
	(void) arg;

	printf("PID thread started, pid: %" PRIkernel_pid "\n", thread_getpid());
	//msg_t m;
	//msg_init_queue(pid_rcv_queue, RCV_QUEUE_SIZE);
	while (1) {
		for(uint8_t i = 0; i < VM_PID_NUM_AVAILABLE; i++)
		{
			PID::instances()[i].compute();
		}
		xtimer_usleep(10000);
	}
	return 0;
}
