/*
 * Copyright (C) 2017 Mattes Besuden
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @brief       Defines communication codes to control the VM thread.
 *
 * @author      Mattes Besuden <besuden@uni-bremen.de>
 */
#ifndef INCLUDES_THREADVM_H_
#define INCLUDES_THREADVM_H_

///Command to run VM execution
#define VM_THREAD_RUN		0x01
///Command to stop VM execution
#define VM_THREAD_STOP		0x02
///Command to retrieve VM status
#define VM_THREAD_STATUS	0x04
///Command to restart VM
#define VM_THREAD_RESTART	0x08


#endif /* INCLUDES_THREADVM_H_ */
