/*
 * Copyright (C) 2017 Mattes Besuden
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @brief       Memory implementation to provide shared memory usable by multiple threads. Protects memory access via mutexes.
 *
 * @author      Mattes Besuden <besuden@uni-bremen.de>
 */
#ifndef MEMORY_H_
#define MEMORY_H_

#include <string.h>
#include <mutex.h>

#include "calculationconfig.h"
#include "URL_Mapping.h"

///Defines memory size
#define MEMORY_SIZE		VM_MEMORY_SIZE
///Defines number of availabe URL-Mappings
#define MEMORY_MAP_SIZE	VM_MEMORY_MAP_SIZE

class Memory
{
public:
	/**
	 * Memory instance
	 * @return Instance of shared memory
	 */
	static Memory& instance()
	{
	   static Memory shared_memory;
	   return shared_memory;
	}
	Memory(void);
	~Memory(void) { }

	void store(uint16_t address, uint8_t value);
	void storeaddress(uint16_t address, uint16_t value);
	void storerational(uint16_t baseaddress, rational_t value);
	void storeunsigned(uint16_t baseaddress, uint32_t value);

	uint8_t load(uint16_t address);
	uint16_t loadaddress(uint16_t address);
	rational_t loadrational(uint16_t baseaddress);
	uint32_t loadunsigned(uint16_t baseaddress);

	void copy(uint16_t src, uint8_t len, uint16_t dest);

	void map(uint8_t id, uint8_t optype, uint8_t map_options, uint16_t value_address, uint16_t port, uint16_t url_address, uint16_t resource_address);
	uint8_t checkmap(uint8_t i) {return checkmap(i, true);}
	uint8_t checkmap(uint8_t i, bool deleteafter);
	void map_done(uint8_t id);
	void map_error(uint8_t id, uint8_t code);
	void unmap(uint8_t id);

	const unsigned char* loadurl(uint16_t address) const;

	const uint8_t* dump(void) const;
	const url_map_t* dumpMap(void) const;

	uint16_t getMemorySize(void);
	uint8_t getMapSize(void);
	uint8_t getMapForAddress(uint16_t address);

	void clear(void);
private:

	mutex_t mutex;
	uint8_t memory[MEMORY_SIZE];
	url_map_t mappings[MEMORY_MAP_SIZE];
	inline bool checkmemoryaddress(uint16_t* address, uint8_t typesize);
	inline bool checkMapId(uint8_t* id);
};

#endif /* MEMORY_H_ */
