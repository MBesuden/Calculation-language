/*
 * Copyright (C) 2017 Mattes Besuden
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @brief       Implementation of gcoap_shared_memory_functions.h
 *
 * @author      Mattes Besuden <besuden@uni-bremen.de>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <stdexcept>

#include "gcoap_shared_memory_functions.h"
#include "Opcodes.h"

#include "ThreadVM.h"
#include "PID.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "msg.h"
#include "xtimer.h"

	/**
	 * @see Memory::store(uint16_t, uint8_t) from Memory.h
	 * @param address Address to write data to
	 * @param value Value to write
	 */
	void gcoap_store(uint16_t address, uint8_t value)
	{
		try
		{
			Memory::instance().store(address, value);
		}
		catch(std::range_error &e)
		{
			printf("Memory access violation at %d\n", address);
		}
	}
//	uint8_t gcoap_load(uint16_t address)
//	{
//		return Memory::instance().load(address);
//	}
//	void gcoap_storeaddress(uint16_t address, uint16_t value)
//	{
//		Memory::instance().store(address, value);
//	}
//	uint16_t gcoap_loadaddress(uint16_t address)
//	{
//		return Memory::instance().loadaddress(address);
//	}
//	void gcoap_storeunsigned(uint16_t address, uint32_t value)
//	{
//		Memory::instance().storeunsigned(address, value);
//	}
//	uint32_t gcoap_loadunsigned(uint16_t address)
//	{
//		return Memory::instance().loadunsigned(address);
//	}
//	void gcoap_storedecimal(uint16_t address, uint32_t value)
//	{
//		decimal_t temp;
//		memcpy(&temp, &value, sizeof(decimal_t));
//		Memory::instance().storedecimal(address, temp);
//	}
//	uint32_t gcoap_loaddecimal(uint16_t address)
//	{
//		uint32_t temp;
//		decimal_t value = Memory::instance().loaddecimal(address);
//		memcpy(&temp, &value, sizeof(decimal_t));
//		return temp;
//	}
//	const url_map_t* gcoap_getMappings()
//	{
//		return Memory::instance().dumpMap();
//	}
//	uint8_t gcoap_getMapSize()
//	{
//		return Memory::instance().getMapSize();
//	}
//
	/**
	 * @see Memory::loadurl(uint16_t) from Memory.h
	 * @param address Address to load from
	 * @return Pointer to URL (cstring)
	 */
	const unsigned char* gcoap_loadurl(uint16_t address)
	{
		return Memory::instance().loadurl(address);
	}

	/**
	 * Loads value from shared memory into buffer, if requested value is mapped via URL-Map
	 * @param content_type Requested content format
	 * @param payload Payload data which includes requested url. Also buffer to write into if mapping was found.
	 * @param payload_len Length of payload data
	 * @param max_len Max length of data to write
	 * @return Written bytes
	 */
	uint8_t gcoap_check_server_value(uint16_t content_type, uint8_t* payload, unsigned payload_len, size_t max_len)
	{
		const url_map_t* mappings = Memory::instance().dumpMap();
		for(uint8_t i = 0; i < Memory::instance().getMapSize(); i++)
		{
			const char* url = (const char*)Memory::instance().loadurl(mappings[i].resource_address);
			if(strlen(url) == payload_len && strncmp(url, (const char*)payload, payload_len) == 0)
			{
				return gcoap_load_value(content_type, payload, max_len, i);
			}
		}
		return 0;
	}

	/**
	 * Loads value from shared memory into payload buffer.
	 * @param content_type requested data Format
	 * @param payload Payload buffer to write into
	 * @param max_len Max length to write
	 * @param map_id ID of URL-Map
	 * @return Written bytes
	 */
	uint8_t gcoap_load_value(uint16_t content_type, uint8_t* payload, size_t max_len, uint8_t map_id)
	{
		switch(content_type)
		{
		case 42://COAP_FORMAT_OCTET
			return gcoap_load_value_octet(payload, max_len, map_id);
		case 0://COAP_FORMAT_TEXT
		default://no known format specified (use text)
			return gcoap_load_value_text(payload, max_len, map_id);
		}
		return 0;
	}

	/**
	 * Loads data from shared memory into buffer (in text format).
	 * @param payload Payload buffer to write into
	 * @param max_len Max length to write
	 * @param map_id ID of URL-Map
	 * @return Written bytes
	 */
	uint8_t gcoap_load_value_text(uint8_t* payload, size_t max_len, uint8_t map_id)
	{
		uint8_t len = 0;
		const url_map_t* mappings = Memory::instance().dumpMap();
		switch(mappings[map_id].optype & VM_OPTYPE_MASK)
		{
		case VM_OPERAND_TYPE_UINT8:
			len = snprintf((char*)payload, max_len, "%" PRIu8 , Memory::instance().load(mappings[map_id].value_address));
			break;
		case VM_OPERAND_TYPE_UINT16:
			len = snprintf((char*)payload, max_len, "%" PRIu16 , Memory::instance().loadaddress(mappings[map_id].value_address));
			break;
		case VM_OPERAND_TYPE_UINT32:
			len = snprintf((char*)payload, max_len, "%" PRIu32 , Memory::instance().loadunsigned(mappings[map_id].value_address));
			break;
		case VM_OPERAND_TYPE_DEC:
		{
			rational_t value = Memory::instance().loadrational(mappings[map_id].value_address);
			len = snprintf((char*)payload, max_len, "%d.%04d", (int32_t)value, (int32_t)(value * 10000) % 10000);
		}
		break;
		default:
			break;
		}
		if((mappings[map_id].status <= 1 /* no error in mapping */) && (mappings[map_id].map_options & VM_MAP_OPTION_LIFETIME_MASK) == VM_MAP_OPTION_LIFETIME_ONCE)
		{
			Memory::instance().unmap(map_id);
		}
		return len;
	}

	/**
	 * Loads octet value from shared memory.
	 * @param payload Payload buffer to write into
	 * @param max_len Max length to write
	 * @param map_id ID of URL-Map
	 * @return Written bytes
	 */
	uint8_t gcoap_load_value_octet(uint8_t* payload, size_t max_len, uint8_t map_id)
	{
		(void) max_len;
		uint8_t len = 0;
		const url_map_t* mappings = Memory::instance().dumpMap();
		switch(mappings[map_id].optype & VM_OPTYPE_MASK)
		{
		case VM_OPERAND_TYPE_UINT8:
		{
			uint8_t value = Memory::instance().load(mappings[map_id].value_address);
			memcpy(payload, &value, sizeof(uint8_t));
			len = sizeof(uint8_t);
		}
		break;
		case VM_OPERAND_TYPE_UINT16:
		{
			uint16_t value = Memory::instance().loadaddress(mappings[map_id].value_address);
			memcpy(payload, &value, sizeof(uint16_t));
			len = sizeof(uint16_t);
		}
		break;
		case VM_OPERAND_TYPE_UINT32:
		{
			uint32_t value = Memory::instance().loadunsigned(mappings[map_id].value_address);
			memcpy(payload, &value, sizeof(uint32_t));
			len = sizeof(uint32_t);
		}
		break;
		case VM_OPERAND_TYPE_DEC:
		{
			rational_t value = Memory::instance().loadrational(mappings[map_id].value_address);
			memcpy(payload, &value, sizeof(rational_t));
			len = sizeof(rational_t);
		}
		break;
		default:
			break;
		}
		if((mappings[map_id].map_options & VM_MAP_OPTION_LIFETIME_MASK) == VM_MAP_OPTION_LIFETIME_ONCE)
		{
			Memory::instance().unmap(map_id);
		}
		return len;
	}

	/**
	 * Stores text data in shared memory.
	 * @param payload		Data to be stored. Will be a copied into CString buffer due to atoi/atof functions (stores 0 if string is not valid).
	 * @param payload_len	Length of incoming data.
	 * @param mapping		URL-Map which determines memory address.
	 * @return 0 if store was successful, -1 on error
	 */
	int8_t gcoap_store_value_text(uint8_t* payload, unsigned payload_len, const url_map_t* mapping)
	{
		if(payload_len >= 15)
		{
			return -1;
		}
		char buf[15] = {0};
		snprintf(buf, payload_len, "%s", (char*)payload);
		switch(mapping->optype & VM_OPTYPE_MASK)
		{
		case VM_OPERAND_TYPE_UINT8:
			Memory::instance().store(mapping->value_address, (uint8_t)atoi((const char*)payload));
			break;
		case VM_OPERAND_TYPE_UINT16:
			Memory::instance().storeaddress(mapping->value_address, (uint16_t)atoi((const char*)payload));
			break;
		case VM_OPERAND_TYPE_UINT32:
			Memory::instance().storeunsigned(mapping->value_address, (uint32_t)atoi((const char*)payload));
			break;
		case VM_OPERAND_TYPE_DEC:
			Memory::instance().storerational(mapping->value_address, (rational_t)atof((const char*)payload));
			break;
		default:
			break;
		}
		return 0;
	}

	/**
	 * Stores octet value in shared memory
	 * @param payload Payload data
	 * @param payload_len Payload length
	 * @param mapping URL-Map
	 * @return 0 if store was successful, -1 on error
	 */
	int8_t gcoap_store_value_octet(uint8_t* payload, unsigned payload_len, const url_map_t* mapping)
	{
		(void)payload_len;
		switch(mapping->optype & VM_OPTYPE_MASK)
		{
		case VM_OPERAND_TYPE_UINT8:
			Memory::instance().store(mapping->value_address, *payload);
			break;
		case VM_OPERAND_TYPE_UINT16:
			Memory::instance().storeaddress(mapping->value_address, *((uint16_t*)payload));
			break;
		case VM_OPERAND_TYPE_UINT32:
			Memory::instance().storeunsigned(mapping->value_address, *((uint32_t*)payload));
			break;
		case VM_OPERAND_TYPE_DEC:
			Memory::instance().storerational(mapping->value_address, *((rational_t*)payload));
			break;
		default:
			break;
		}
		return 0;
	}

	/**
	 * Stores Value in shared memory
	 * @param content_type Content Format of payload data
	 * @param map_id ID of URL-Map
	 * @param payload Payload data
	 * @param payload_len Payload length
	 * @return 0 if store was successful, -1 on error
	 */
	int8_t gcoap_store_value(uint16_t content_type, uint8_t map_id, uint8_t* payload, unsigned payload_len)
	{
		int8_t return_val = 0;
		const url_map_t* mappings = Memory::instance().dumpMap();
		switch(content_type)
		{
		case 42: //octet
			return_val = gcoap_store_value_octet(payload, payload_len, &mappings[map_id]);
			break;
		case 0: //format text
		default: //unknown assume text
			return_val = gcoap_store_value_text(payload, payload_len, &mappings[map_id]);
		}

		if((mappings[map_id].map_options & VM_MAP_OPTION_LIFETIME_MASK) == VM_MAP_OPTION_LIFETIME_ONCE)
		{
			Memory::instance().unmap(map_id);
		}
		return return_val;
	}

	/**
	 * @see Memory::dumpMap() from Memory.h
	 * @return Pointer to URL-Maps
	 */
	const url_map_t* gcoap_get_mappings()
	{
		return Memory::instance().dumpMap();
	}

	/**
	 * @see Memory::getMapSize() from Memory.h
	 * @return Number of URL-Maps
	 */
	uint8_t gcoap_get_map_size()
	{
		return Memory::instance().getMapSize();
	}

	/**
	 * Writes host string defined by URL-Map from shared memory into buffer.
	 * @param mapping URL-Map to get resource from.
	 * @param buf Buffer to write resource string into.
	 */
	void gcoap_get_resource(const url_map_t* mapping, char* buf)
	{
		const char* resource = (const char*)gcoap_loadurl(mapping->resource_address);
		size_t str_len = strlen(resource);
		memcpy(buf, resource, str_len);
		buf[str_len] = 0;
		return ;
	}

	/**
	 * Writes host string defined by URL-Map from shared memory into buffer.
	 * @param mapping URL-Map to get host from.
	 * @param buf Buffer to write host string into.
	 */
	void gcoap_get_host(const url_map_t* mapping, char* buf)
	{
		const char* url = (const char*)gcoap_loadurl(mapping->url_address);
		size_t str_len = strlen(url);//gcoap_parse_resource_offset(url);
		memcpy(buf, url, str_len);
		buf[str_len] = 0;
		return ;
	}

	/**
	 * @see Memory::map_done(uint8_t) in Memory.h
	 * @param id ID of URL-map
	 */
	void gcoap_done(uint8_t id)
	{
		Memory::instance().map_done(id);
	}

	/**
	 * @see Memory::map_error(uint8_t, uint8_t) in Memory.h
	 * @param id ID of URL-Map
	 * @param errorcode Error code of URL-Map
	 */
	void gcoap_error(uint8_t id, uint8_t errorcode)
	{
		Memory::instance().map_error(id, errorcode);
	}

	/**
	 * Converts 2 char hex representation into the value it represents.
	 * @param first A single hex char (first of 2).
	 * @param second A single hex char (second of 2).
	 * @return Value of a 2 char hex string
	 */
	uint8_t gcoap_fromHex(char first, char second)
	{
		return ascii2hex(first) << 4 | ascii2hex(second);
	}

	/**
	 * Converts a hex char to the value it represents.
	 * @param inChar A single hex char.
	 * @return Value of a single hex char.
	 */
	uint8_t ascii2hex(char inChar)
	{
		uint8_t retHex=0xff;
		if( (inChar>=48) && (inChar<=57) )//zahlen
			retHex = inChar-48;
		else if( (inChar>=65) && (inChar<=70) )//kleinbuchstaben
			retHex = inChar-55;
		else if( (inChar>=97) && (inChar<=102) )//großbuchstaben
			retHex = inChar-87;
		return retHex;
	}

	/**
	 * @see Memory::dump() in Memory.h
	 * @return Pointer to memory dump.
	 */
	const uint8_t* gcoap_dumpMemory(void)
	{
		return Memory::instance().dump();
	}

	/**
	 * @see Memory::getMemorySize() in Memory.h
	 * @return Size of shared memory
	 */
	uint16_t gcoap_dumpSize(void)
	{
		return Memory::instance().getMemorySize();
	}

	/**
	 * Writes Status Bytes of VM instance into buffer. Needs Process ID of VM thread to get status flags.
	 * @param buf Buffer to write status information
	 * @param buf_len Length of buffer
	 * @param vm_thread_pid Process ID of VM thread
	 * @return Bytes written
	 */
	size_t gcoap_statusVM(uint8_t* buf, size_t buf_len, kernel_pid_t vm_thread_pid)
	{
		msg_t m;
		m.content.value = VM_THREAD_STATUS;
		msg_try_send(&m, vm_thread_pid);
		int received = 0;
		size_t payload_len = 0;
		for(uint8_t i = 0; i < 3; i++)
		{
			received = msg_try_receive(&m);
			if(received == 1)
			{
				payload_len = sizeof(uint32_t);
				if(payload_len > buf_len)
				{
					return 0;
				}
				memcpy(buf, &m.content.value, sizeof(uint32_t));
				break;
			}
			xtimer_usleep(5000);
		}
		return payload_len;
	}

	/**
	 * Writes Status Bytes of Memory instance into buffer (not implemented).
	 * @param buf Buffer to write status information
	 * @param buf_len Length of buffer
	 * @return Bytes written
	 */
	size_t gcoap_statusMemory(uint8_t* buf, size_t buf_len)
	{
		(void) buf;
		(void) buf_len;
		return 0;
	}

	/**
	 * Writes Status Bytes of URL-Mappings into buffer.
	 * @param buf Buffer to write status information
	 * @param buf_len Length of buffer
	 * @return Bytes written
	 */
	size_t gcoap_statusMappings(uint8_t* buf, size_t buf_len)
	{
		const url_map_t* mappings = gcoap_get_mappings();
		uint8_t mapsize = gcoap_get_map_size();
		size_t written = sizeof(uint8_t);
		if(written > buf_len)
		{
			return 0;
		}
		memcpy(buf, &mapsize, sizeof(uint8_t));
		for(uint8_t i = 0; i < mapsize; i++)
		{
			if(written + sizeof(uint8_t) > buf_len)
			{
				return written;
			}
			memcpy(buf+written, &mappings[i].status, sizeof(uint8_t));
			written += sizeof(uint8_t);
		}
		return written;
	}

	/**
	 * Writes Status Bytes of PID instances into buffer.
	 * @param buf Buffer to write status information
	 * @param buf_len Length of buffer
	 * @return Bytes written
	 */
	size_t gcoap_statusPID(uint8_t* buf, size_t buf_len)
	{
		PID* pids = PID::instances();
		uint8_t pid_num = VM_PID_NUM_AVAILABLE;
		size_t written = sizeof(uint8_t);
		if(written > buf_len)
		{
			return 0;
		}
		memcpy(buf, &pid_num, sizeof(uint8_t));
		for(uint8_t i = 0; i < pid_num; i++)
		{
			if(written + sizeof(uint8_t) > buf_len)
			{
				return written;
			}
			uint8_t status = pids[i].getMode() << 4 | (pids[i].isInitialized() ? 1 : 0);
			memcpy(buf+written, &status, sizeof(uint8_t));
			written += sizeof(uint8_t);
		}
		return written;
	}

#ifdef __cplusplus
}
#endif
