/*
 * Copyright (C) 2017 Mattes Besuden
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @brief       Implementation of VM Thread
 *
 * @author      Mattes Besuden <besuden@uni-bremen.de>
 */
extern "C" {
#include "thread.h"
#include "msg.h"
}

#include "ThreadVM.h"
#include "CalculationVM.h"

void *vm_thread(void* arg);
void parse(msg_t* m, VM* vm);

bool vm_thread_run = false;
bool vm_thread_restart = false;

static msg_t vm_thread_queue[RCV_QUEUE_SIZE];

/**
 * VM thread function
 * @param arg
 */
void *vm_thread(void *arg)
{
	(void) arg;
	msg_t m;
	msg_init_queue(vm_thread_queue, RCV_QUEUE_SIZE);

	printf("VM Thread started, pid: %" PRIkernel_pid "\n", thread_getpid());
	VM vm(&Memory::instance(), PID::instances());

	while(1)
	{
		if(vm_thread_run)
		{
			if(msg_try_receive(&m) == 1)
			{
				parse(&m, &vm);
			}
			if(vm_thread_restart)
			{
				vm.clear();
				vm_thread_restart = false;
			}
			vm.executeStep();
			if(vm.halted())
			{
				vm_thread_run = false;
				if(vm.errorFlag())
				{
					printf("VM ERROR: 0x%08" PRIx32 , vm.getStatuscode());
				}
			}
		}
		else
		{
			msg_receive(&m);
			parse(&m, &vm);
		}
	}
	return 0;
}

/**
 * Parses received message and executes command
 * @param m Message
 * @param vm VM.h
 */
void parse(msg_t* m, VM* vm)
{
	switch(m->content.value)
	{
	case VM_THREAD_RUN:
		vm_thread_run = true;
		break;
	case VM_THREAD_STOP:
		vm_thread_run = false;
		break;
	case VM_THREAD_RESTART:
		vm_thread_restart = true;
		vm_thread_run = true;
		break;
	case VM_THREAD_STATUS:
		m->content.value = vm->getStatuscode();
		msg_send(m, m->sender_pid);
		break;
	default: break;
	}
}
