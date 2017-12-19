/*
 * Copyright (C) 2017 Mattes Besuden
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @brief       Tests for C Wrapper Functions for ggcoap_shared_memory_functions.h
 *
 * @author      Mattes Besuden <besuden@uni-bremen.de>
 */

#ifndef TESTS_TESTGCOAPSHAREDMEMORYFUNCTIONS_H_
#define TESTS_TESTGCOAPSHAREDMEMORYFUNCTIONS_H_

#include "Tests.h"
#include "Memory.h"

#include "URL_Mapping.h"

extern "C" {

void gcoap_store(uint16_t address, uint8_t value);
const unsigned char* gcoap_loadurl(uint16_t address);

uint8_t gcoap_check_server_value(uint16_t content_type, uint8_t* payload, unsigned payload_len, size_t max_len);

uint8_t gcoap_load_value(uint16_t content_type, uint8_t* payload, size_t max_len, uint8_t map_id);
uint8_t gcoap_load_value_text(uint8_t* payload, size_t max_len, uint8_t map_id);
uint8_t gcoap_load_value_octet(uint8_t* payload, size_t max_len, uint8_t map_id);

void gcoap_store_value(uint16_t content_type, uint8_t map_id, uint8_t* payload, unsigned payload_len);
void gcoap_done(uint8_t id);
void gcoap_error(uint8_t id, uint8_t errorcode);

const url_map_t* gcoap_get_mappings(void);
uint8_t gcoap_get_map_size(void);

uint8_t gcoap_parse_resource_offset(const char* url);

void gcoap_get_resource(const url_map_t* mapping, char* buf);
void gcoap_get_host(const url_map_t* mapping, char* buf);

size_t gcoap_statusVM(uint8_t* buf, size_t buf_len, kernel_pid_t vm_thread_pid);
size_t gcoap_statusMemory(uint8_t* buf, size_t buf_len);
size_t gcoap_statusMappings(uint8_t* buf, size_t buf_len);
size_t gcoap_statusPID(uint8_t* buf, size_t buf_len);
}

inline void test_gcoap_store(void)
{
	Memory::instance().clear();
	gcoap_store(0x00, 0xff);
	ASSERT(Memory::instance().load(0x00) == 0xff, "gcoap_store wrong");
}

inline void test_gcoap_loadurl(void)
{
	char url[] = "dead::beef:1";
	uint8_t len = strlen(url);
	for(uint8_t i = 0; i < len; i++)
	{
		Memory::instance().store(i, url[i]);
	}
	Memory::instance().store(len, 0);
	ASSERT(strcmp((const char*)gcoap_loadurl(0x00), "dead::beef:1") == 0, "loadurl wrong");
}

inline void test_gcoap_check_server_value(void)
{
	char resource[] = "/testvalue";//url to map
	uint8_t len = strlen(resource);
	uint16_t baseaddress = 0x0012;//baseaddress for url
	for(uint8_t i = 0; i < len; i++)//store the mapped url
	{
		Memory::instance().store(baseaddress + i, resource[i]);
	}
	Memory::instance().map(0, 0, VM_MAP_OPTION_LIFETIME_EVER, 0x0000, 0, baseaddress - 2, baseaddress);//create a URL Map
	Memory::instance().storeunsigned(0x0000, 1234);//store value in Memory which is mapped
	uint16_t content_type_text = 0;//simulate coap pdu values for content format text
	uint8_t payload[] = {'/', 't', 'e', 's', 't', 'v', 'a', 'l', 'u', 'e'};
	unsigned payload_len = 10;
	uint8_t newlen = gcoap_check_server_value(content_type_text, (uint8_t*)payload, payload_len, (size_t)payload_len);
	ASSERT(strcmp((const char*)payload, "1234") == 0, "check_server_value wrong (text)");
	ASSERT(newlen == 4, "payload_len wrong (text)");

	uint16_t content_type_octet = 42;//simulate coap pdu values
	uint8_t payload2[] = {'/', 't', 'e', 's', 't', 'v', 'a', 'l', 'u', 'e'};
	unsigned payload_len2 = 10;
	newlen = gcoap_check_server_value(content_type_octet, (uint8_t*)payload2, payload_len2, (size_t)payload_len2);
	ASSERT(payload2[0] == 0xd2 && payload2[1] == 0x04 && payload2[2] == 0x00 && payload2[3] == 0x00, "check_server_value wrong (octet)");
	ASSERT(newlen == 4, "payload_len wrong (octet)");
}

inline void test_gcoap_load_value(void)
{
	char resource[] = "/testvalue";//url to map
	uint8_t len = strlen(resource);
	uint16_t baseaddress = 0x0012;//baseaddress for url
	for(uint8_t i = 0; i < len; i++)//store the mapped url
	{
		Memory::instance().store(baseaddress + i, resource[i]);
	}
	Memory::instance().map(0, 0, VM_MAP_OPTION_LIFETIME_EVER, 0x0000, 0, baseaddress-2, baseaddress);//create a URL Map
	Memory::instance().storeunsigned(0x0000, 1234);//store value in Memory which is mapped
	uint16_t content_type_text = 0;//simulate coap pdu values for content format text
	uint8_t payload[] = {'/', 't', 'e', 's', 't', 'v', 'a', 'l', 'u', 'e'};
	unsigned payload_len = 10;
	uint8_t newlen = gcoap_load_value(content_type_text, (uint8_t*)payload, (size_t)payload_len, 0);
	ASSERT(strcmp((const char*)payload, "1234") == 0, "check_server_value wrong (text)");
	ASSERT(newlen == 4, "payload_len wrong (text)");

	uint16_t content_type_octet = 42;//simulate coap pdu values
	uint8_t payload2[] = {'/', 't', 'e', 's', 't', 'v', 'a', 'l', 'u', 'e'};
	unsigned payload_len2 = 10;
	newlen = gcoap_load_value(content_type_octet, (uint8_t*)payload2, (size_t)payload_len2, 0);
	ASSERT(payload2[0] == 0xd2 && payload2[1] == 0x04 && payload2[2] == 0x00 && payload2[3] == 0x00, "check_server_value wrong (octet)");
	ASSERT(newlen == 4, "payload_len wrong (octet)");
}

inline void test_gcoap_store_value(void)
{
	uint16_t baseaddress = 0x0012;
	uint16_t valueaddress = 0x0000;
	uint16_t valueaddress2 = 0x0004;
	Memory::instance().map(0, 0, VM_MAP_OPTION_LIFETIME_EVER, valueaddress, 0, baseaddress-2, baseaddress);//create a url map
	Memory::instance().map(1, VM_OPERAND_TYPE_DEC, VM_MAP_OPTION_LIFETIME_EVER, valueaddress2, 0, baseaddress-2, baseaddress);//create a url map
	uint8_t payload_text[] = {'1', '2', '3', '4'};
	uint16_t content_type = 0;
	unsigned payload_len = 4;
	gcoap_store_value(content_type, 0, payload_text, payload_len);
	ASSERT(Memory::instance().loadunsigned(valueaddress) == 1234, "stored value wrong");

	uint8_t payload_octet[] = {0xd2, 0x04, 0x10, 0x00};
	content_type = 42;
	payload_len = 4;
	gcoap_store_value(content_type, 0, payload_octet, payload_len);
	ASSERT(Memory::instance().loadunsigned(valueaddress) == 1049810, "stored value wrong");

	uint8_t payload_text2[] = {'1', '2', '3', '.', '4'};
	uint16_t content_type2 = 0;
	unsigned payload_len2 = 5;
	gcoap_store_value(content_type2, 1, payload_text2, payload_len2);
	ASSERT(Memory::instance().loadrational(valueaddress2) == rational_t(123.4), "stored value wrong (decimal)");

	uint8_t payload_octet2[] = {0x80, 0x16, 0x00, 0x00};//should be 22.5 in fixed8_t
	content_type2 = 42;
	payload_len2 = 4;
	gcoap_store_value(content_type2, 1, payload_octet2, payload_len2);
	ASSERT(Memory::instance().loadrational(valueaddress2) == rational_t(22.5), "stored value wrong (decimal)");
}

inline void test_gcoap_done(void)
{
	Memory::instance().map(0, 0, VM_MAP_OPTION_LIFETIME_ONCE, 0, 0, 0, 0);
	gcoap_done(0);
	ASSERT(Memory::instance().dumpMap()[0].status & VM_MAP_STATUS_DONE, "Map not marked done");
	Memory::instance().checkmap(0);
	ASSERT(Memory::instance().dumpMap()[0].value_address == NO_MAPPING, "Map should be deleted due to VM_MAP_LIFETIME_ONCE")
}

inline void test_gcoap_error(void)
{
	Memory::instance().map(0, 0, VM_MAP_OPTION_LIFETIME_ONCE, 0, 0, 0, 0);
	gcoap_error(0, VM_MAP_STATUS_ERROR_404);
	ASSERT(Memory::instance().dumpMap()[0].status > 1, "Map should have an error");
	ASSERT(Memory::instance().dumpMap()[0].status & VM_MAP_STATUS_ERROR_404, "Map should have error 404");
	Memory::instance().checkmap(0);
	ASSERT(Memory::instance().dumpMap()[0].value_address != NO_MAPPING, "Map should not be deleted due to error");
}


inline void test_gcoap_get_mappings(void)
{
	ASSERT(gcoap_get_mappings() == Memory::instance().dumpMap(), "not the mappings we are looking for");
}

inline void test_gcoap_get_map_size(void)
{
	ASSERT(gcoap_get_map_size() == VM_MEMORY_MAP_SIZE, "Mapsize wrong");
}

inline void test_gcoap_get_ressource(void)
{
	char url[] = "dead::beef:1\0/testvalue";//url to map
	uint8_t len = sizeof(url);
	uint16_t baseaddress = 0x0010;//baseaddress for url
	for(uint8_t i = 0; i < len; i++)//store the mapped url
	{
		Memory::instance().store(baseaddress + i, url[i]);
	}
	Memory::instance().map(0, 0, VM_MAP_OPTION_LIFETIME_EVER, 0x0000, 0, baseaddress, baseaddress + strlen(url) + 1);//create a URL Map
	char resourcebuffer[20];
	gcoap_get_resource(Memory::instance().dumpMap(), resourcebuffer);
	ASSERT(strcmp(resourcebuffer, "/testvalue") == 0, "wrong resource");
}

inline void test_gcoap_get_host(void)
{
	char url[] = "dead::beef:1\0/testvalue";//url to map
	uint8_t len = sizeof(url);
	uint16_t baseaddress = 0x0010;//baseaddress for url
	for(uint8_t i = 0; i < len; i++)//store the mapped url
	{
		Memory::instance().store(baseaddress + i, url[i]);
	}
	Memory::instance().map(0, 0, VM_MAP_OPTION_LIFETIME_EVER, 0x0000, 0, baseaddress, baseaddress + strlen(url) + 1);//create a URL Map
	char hostbuffer[20];
	gcoap_get_host(Memory::instance().dumpMap(), hostbuffer);
	ASSERT(strcmp(hostbuffer, "dead::beef:1") == 0, "wrong host");
}

inline void test_gcoap_statusVM(void)
{
	TESTINFO("needs vm thread to run");
}

inline void test_gcoap_statusMemory(void)
{
	TESTINFO("Method not implemented");
}

inline void test_gcoap_statusMappings(void)
{
	uint8_t buf[17];
	size_t written = gcoap_statusMappings(buf, 17);
	ASSERT(written == VM_MEMORY_MAP_SIZE + 1, "Not all bytes written");

	uint8_t buf2[4];
	written = gcoap_statusMappings(buf2, 4);
	ASSERT(written == sizeof(buf2), "Something went wrong");
}

inline void test_gcoap_statusPID(void)
{
	uint8_t buf[4];
	size_t written = gcoap_statusPID(buf, 4);
	ASSERT(written == VM_PID_NUM_AVAILABLE + 1, "Not all bytes written");

	uint8_t buf2[1];
	written = gcoap_statusPID(buf, 1);
	ASSERT(written == sizeof(buf2), "Something went wrong");
}

/**
 * @brief Runs all test functions specified. Acts as a test-suite.
 */
inline void test_Gcoap_shared()
{
#ifndef TEST_Gcoap_shared_OFF
	test_gcoap_store();

	test_gcoap_loadurl();

	test_gcoap_check_server_value();

	test_gcoap_load_value();

	test_gcoap_store_value();
	test_gcoap_done();
	test_gcoap_error();

	test_gcoap_get_mappings();
	test_gcoap_get_map_size();

	test_gcoap_get_ressource();
	test_gcoap_get_host();

	test_gcoap_statusVM();
	test_gcoap_statusMemory();
	test_gcoap_statusMappings();
	test_gcoap_statusPID();

#else
	TESTINFO("Test Gcoap_shared_memory_functions off");
#endif
}

#endif /* TESTS_TESTGCOAPSHAREDMEMORYFUNCTIONS_H_ */
