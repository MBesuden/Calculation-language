/*
 * Copyright (C) 2017 Mattes Besuden
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @brief       Implementation of Memory.h
 *
 * @author      Mattes Besuden <besuden@uni-bremen.de>
 */



#include <stdio.h>
#include <stdexcept>

#include "Memory.h"

/**
 * @brief Implementation of a Memory which can be used by multiple threads. Load and store operations are protected via mutex. Values will be passed by value to ensure data integrity between threads.
 */
Memory::Memory()
{
	mutex_init(&mutex);
	Memory::clear();
}

/**
 * Stores uint8_t value at address.
 * @param address Address where to store value.
 * @param value Value to store.
 */
void Memory::store(uint16_t address, uint8_t value)
{
	if(checkmemoryaddress(&address, sizeof(uint8_t)))
	{
		throw std::range_error("Memory access violation (store)");
	}
	this->memory[address] = value;
}

/**
 * stores uint16_t value at baseaddress.
 * @param baseaddress Address where to store value.
 * @param value Value to store.
 */
void Memory::storeaddress(uint16_t baseaddress, uint16_t value)
{
	if(checkmemoryaddress(&baseaddress, sizeof(uint16_t)))
	{
		throw std::range_error("Memory access violation (storeaddress)");
	}
	memcpy(memory + baseaddress, &value, sizeof(uint16_t));
}

/**
 * Stores decimal_t at baseaddress.
 * @param baseaddress Address where to store value.
 * @param value Value to store.
 */
void Memory::storerational(uint16_t baseaddress, rational_t value)
{
	if(checkmemoryaddress(&baseaddress, sizeof(rational_t)))
	{
		throw std::range_error("Memory access violation (storedecimal)");
	}
	memcpy(memory + baseaddress, &value, sizeof(rational_t));
}

/**
 * Stores uint32_t at baseaddress.
 * @param baseaddress Address where to store value.
 * @param value Value to store.
 */
void Memory::storeunsigned(uint16_t baseaddress, uint32_t value)
{
	if(checkmemoryaddress(&baseaddress, sizeof(uint32_t)))
	{
		throw std::range_error("Memory access violation (storeunsigned)");
	}
	memcpy(memory + baseaddress, &value, sizeof(uint32_t));
}

/**
 * Loads uint8_t from address.
 * @param address Address of the value to load.
 * @return Unsigned value from memory.
 */
uint8_t Memory::load(uint16_t address)
{
	if(checkmemoryaddress(&address, sizeof(uint8_t)))
	{
		throw std::range_error("Memory access violation (load)");
	}
	return this->memory[address];
}

/**
 * Loads uint16_t from baseaddress.
 * @param baseaddress Address of the value to load.
 * @return Unsigned value from memory.
 */
uint16_t Memory::loadaddress(uint16_t baseaddress)
{
	if(checkmemoryaddress(&baseaddress, sizeof(uint16_t)))
	{
		throw std::range_error("Memory access violation (loadaddress)");
	}
	uint16_t value;
	memcpy(&value, memory+baseaddress, sizeof(uint16_t));
	return value;
}

/**
 * Loads decimal_t from baseaddress.
 * @param baseaddress Address of the value to load.
 * @return Value from memory.
 */
rational_t Memory::loadrational(uint16_t baseaddress)
{
	if(checkmemoryaddress(&baseaddress, sizeof(rational_t)))
	{
		throw std::range_error("Memory access violation (loaddecimal)");
	}
	rational_t value;
	memcpy(&value, memory+baseaddress, sizeof(rational_t));
	return value;
}

/**
 * Loads uint32_t from baseaddress.
 * @param baseaddress Address of the value to load.
 * @return Unsigned value from memory.
 */
uint32_t Memory::loadunsigned(uint16_t baseaddress)
{
	if(checkmemoryaddress(&baseaddress, sizeof(uint32_t)))
	{
		throw std::range_error("Memory access violation (loadunsigned)");
	}
	uint32_t value;
	memcpy(&value, memory+baseaddress, sizeof(uint32_t));
	return value;
}

/**
 * Copies memory at srcaddress to destaddres with length of len.
 * @param srcaddress	Address where to start copying.
 * @param len			Length to copy.
 * @param destaddress	Destination address.
 */
void Memory::copy(uint16_t srcaddress, uint8_t len, uint16_t destaddress)
{
	if(checkmemoryaddress(&srcaddress, sizeof(uint16_t)) || checkmemoryaddress(&destaddress, sizeof(uint16_t)))
	{
		throw std::range_error("Memory access violation");
	}
	memmove(memory+destaddress, memory+srcaddress, len);
}

/**
 *
 * Maps a URL and Resource to a memory address. Defines OPtype and Map-Options.
 * @param id				ID of the URL mapping to use.
 * @param optype			Operand type of the mapping (what datatype is the value).
 * @param map_options		Mapping options (Coap Method, handled by server/client, ...)
 * @param value_address		Memory address where the value is located (for storage or loading).
 * @param port
 * @param url_address		Address of URL where the value can be requested (for server: empty, for client: address (e.g. affe::1)). Length is resticted by memory size, must be NULL terminated (cstring).
 * @param resource_address	Address od resource where the value can be requested (for server and for client: resource (e.g. /sensor)). Length is resticted by memory size, must be NULL terminated (cstring).
 */
void Memory::map(uint8_t id, uint8_t optype, uint8_t map_options, uint16_t value_address, uint16_t port, uint16_t url_address, uint16_t resource_address)
{
	if(checkMapId(&id))
	{
		throw std::range_error("Map Id Access violation (map)");
	}
	mutex_lock(&mutex);
	this->mappings[id].optype = optype;
	this->mappings[id].map_options = map_options;
	this->mappings[id].value_address = value_address;
	this->mappings[id].port = port;
	this->mappings[id].url_address = url_address;
	this->mappings[id].resource_address = resource_address;
	this->mappings[id].status = 0;
	mutex_unlock(&mutex);
}

/**
 * Checks if a URL-Map status is VM_MAP_STATUS_DONE. Unmaps value if VM_MAP_OPTION_LIFETIME_ONCE and mappings status is done.
 * @param id ID of the URL mapping to check.
 * @param deleteafter Defines if the map should be deleted if checkmap return VM_MAP_STATUS_DONE
 * @return Returns 0x00 if no error but map-value was not set, 0x01 if map value was set, error code if an error occurred.
 */
uint8_t Memory::checkmap(uint8_t id, bool deleteafter)
{
	if(checkMapId(&id))
	{
		throw std::range_error("Map Id Access violation (checkmap)");
	}
	if(this->mappings[id].value_address == NO_MAPPING || this->mappings[id].url_address == NO_MAPPING || this->mappings[id].resource_address == NO_MAPPING || this->mappings[id].status == VM_MAP_STATUS_NO_MAPPING)
	{
		return 0;
	}
	uint8_t temp = mappings[id].status;
	if(deleteafter && temp == VM_MAP_STATUS_DONE && (mappings[id].map_options & VM_MAP_OPTION_LIFETIME_MASK) == VM_MAP_OPTION_LIFETIME_ONCE)
	{//unmap ^ ONLY when done and no error
		unmap(id);
	}
	return temp;
}

/**
 * Sets done flag in URL-Map
 * @param id ID of URL-Map
 */
void Memory::map_done(uint8_t id)
{
	if(checkMapId(&id))
	{
		throw std::range_error("Map Id Access violation (map_done)");
	}
	if(this->mappings[id].value_address == NO_MAPPING || this->mappings[id].url_address == NO_MAPPING || this->mappings[id].resource_address == NO_MAPPING || this->mappings[id].status == VM_MAP_STATUS_NO_MAPPING)
	{
		return;
	}
	mappings[id].status = VM_MAP_STATUS_DONE;
}

/**
 * Sets errorcode in URL-Map
 * @param id ID of URL-Map
 * @param code errorcode
 */
void Memory::map_error(uint8_t id, uint8_t code)
{
	if(checkMapId(&id))
	{
		throw std::range_error("Map Id Access violation (map_error)");
	}
	if(this->mappings[id].value_address == NO_MAPPING || this->mappings[id].url_address == NO_MAPPING || this->mappings[id].resource_address == NO_MAPPING || this->mappings[id].status == VM_MAP_STATUS_NO_MAPPING)
	{
		return;
	}
	mappings[id].status |= (code | VM_MAP_STATUS_DONE);
}

/**
 * Unmaps a URL-Map.
 * @param id ID of the URL map which should be deleted (set to NO_MAPPING).
 */
void Memory::unmap(uint8_t id)
{
	if(checkMapId(&id))
	{
		throw std::range_error("Map Id Access violation (unmap)");
	}
	mutex_lock(&mutex);
	this->mappings[id].optype = 0;
	this->mappings[id].map_options = 0;
	this->mappings[id].value_address = NO_MAPPING;
	this->mappings[id].url_address = NO_MAPPING;
	this->mappings[id].resource_address = NO_MAPPING;
	this->mappings[id].port = 0;
	this->mappings[id].status = VM_MAP_STATUS_NO_MAPPING;
	mutex_unlock(&mutex);
}

/**
 * Loads a URL from memory (cstring).
 * @param address Start address of the URL to load.
 * @return Readonly pointer to the URL.
 */
const unsigned char* Memory::loadurl(uint16_t address) const
{
	return memory+address;//returns pointer to CString
}

/**
 * Dumps Memory.
 * @return Readonly pointer to memory array.
 */
const uint8_t* Memory::dump() const
{
	return this->memory;
}

/**
 * Dumps URL-Maps.
 * @return Readonly pointer to URL mapping array.
 */
const url_map_t* Memory::dumpMap() const
{
	return this->mappings;
}

/**
 * Size of memory.
 * @return Size of the memory array.
 */
uint16_t Memory::getMemorySize()
{
	return MEMORY_SIZE;
}

/**
 * Size of URL-Maps.
 * @return Size of the URL mapping array.
 */
uint8_t Memory::getMapSize()
{
	return MEMORY_MAP_SIZE;
}

/**
 * Checks if there is a URL-Map for the specified address.
 * @param address The Memory address to check if a mapping exists.
 * @return Id of the mapping, 0xff if no mapping exists (possible because we use 4Bit map-ids in VM INstructions -> max 16 mappings)
 */
uint8_t Memory::getMapForAddress(uint16_t address)
{
	for(uint8_t i = 0; i < MEMORY_MAP_SIZE; i++)
	{
		if(this->mappings[i].value_address == address)
		{
			return i;
		}
	}
	return 0xff;
}

/**
 *	Clears memory and sets all mappings to NO_MAPPING.
 */
void Memory::clear()
{
	mutex_lock(&mutex);
	memset(memory, 0, MEMORY_SIZE);
	mutex_unlock(&mutex);
	for(uint8_t i = 0; i < MEMORY_MAP_SIZE; i++)
	{
		Memory::unmap(i);
	}
}

/**
 * Safety check to protect memory from access violation.
 * @param address	Address of the requested value.
 * @param typesize	Size of the requested value.
 * @return True if address is not in the available address size and the requested type can be read.
 */
inline bool Memory::checkmemoryaddress(uint16_t* address, uint8_t typesize)
{
	return *address > (MEMORY_SIZE - typesize);
}

/**
 * Safety check to protect URL-Maps from access violation.
 * @param id URL-Map id to access.
 * @return True if id is not valid.
 */
inline bool Memory::checkMapId(uint8_t* id)
{
	return *id >= MEMORY_MAP_SIZE;
}
