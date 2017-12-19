/*
 * Copyright (C) 2017 Mattes Besuden
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @brief       Wrapper for coap server written in C. Provides functions to access the shared memory written in C++.
 *
 * @author      Mattes Besuden <besuden@uni-bremen.de>
 */
#ifndef INCLUDES_GCOAP_SHARED_MEMORY_FUNCTIONS_H_
#define INCLUDES_GCOAP_SHARED_MEMORY_FUNCTIONS_H_

#include <kernel_types.h>
#include "URL_Mapping.h"

#ifdef __cplusplus
extern "C" {
#endif

void gcoap_store(uint16_t address, uint8_t value);
//uint8_t gcoap_load(uint16_t address);
//void gcoap_storeaddress(uint16_t address, uint16_t value);
//uint16_t gcoap_loadaddress(uint16_t address);
//void gcoap_storeunsigned(uint16_t address, uint32_t value);
//uint32_t gcoap_loadunsigned(uint16_t address);
//void gcoap_storedecimal(uint16_t address, uint32_t value);
//uint32_t gcoap_loaddecimal(uint16_t address);
//
//const url_map_t* gcoap_getMappings(void);
//uint8_t gcoap_getMapSize(void);

const unsigned char* gcoap_loadurl(uint16_t address);

uint8_t gcoap_check_server_value(uint16_t content_type, uint8_t* payload, unsigned payload_len, size_t max_len);

uint8_t gcoap_load_value(uint16_t content_type, uint8_t* payload, size_t max_len, uint8_t map_id);
uint8_t gcoap_load_value_text(uint8_t* payload, size_t max_len, uint8_t map_id);
uint8_t gcoap_load_value_octet(uint8_t* payload, size_t max_len, uint8_t map_id);

int8_t gcoap_store_value(uint16_t content_type, uint8_t map_id, uint8_t* payload, unsigned payload_len);
void gcoap_done(uint8_t id);
void gcoap_error(uint8_t id, uint8_t errorcode);

const url_map_t* gcoap_get_mappings(void);
uint8_t gcoap_get_map_size(void);

void gcoap_get_resource(const url_map_t* mapping, char* buf);
void gcoap_get_host(const url_map_t* mapping, char* buf);
uint8_t gcoap_fromHex(char first, char second);
uint8_t ascii2hex(char inChar);
const uint8_t* gcoap_dumpMemory(void);
uint16_t gcoap_dumpSize(void);

size_t gcoap_statusVM(uint8_t* buf, size_t buf_len, kernel_pid_t vm_thread_pid);
size_t gcoap_statusMemory(uint8_t* buf, size_t buf_len);
size_t gcoap_statusMappings(uint8_t* buf, size_t buf_len);
size_t gcoap_statusPID(uint8_t* buf, size_t buf_len);

#ifdef __cplusplus
}
#endif



#endif /* INCLUDES_GCOAP_SHARED_MEMORY_FUNCTIONS_H_ */
