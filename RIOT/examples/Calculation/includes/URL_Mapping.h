/*
 * Copyright (C) 2017 Mattes Besuden
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @brief       Defines the URL mapping used in Memory and the gcoap server/client, memory and by the Calculation VM.
 *
 * @author      Mattes Besuden <besuden@uni-bremen.de>
 */
#ifndef INCLUDES_URL_MAPPING_H_
#define INCLUDES_URL_MAPPING_H_

///Number of possible URL-Mapings
#define VM_MEMORY_MAP_SIZE	(16)		//4 Bit id im OPTYPE, könnte auf 5 erweitert werden

///Address used to show that value is not mapped (0xffff is max memory size, it is not useful to place a cstring there)
#define NO_MAPPING 0xffff

//MAPOTIONS
///Map option mask to determine if map URL is an address or directly coded in instruction
#define VM_MAP_OPTION_URL_MASK				VM_ADDRESS_MASK
///Map option, URL is directly coded inside instruction
#define VM_MAP_OPTION_URL_LITERAL			VM_LITERAL
///Map option, URL is placed at an address in memory
#define VM_MAP_OPTION_URL_ADDRESS			VM_ADDRESS

///Map option mask to dertermine if map resource is an address or directly coded in instruction
#define VM_MAP_OPTION_RESOURCE_MASK			(VM_ADDRESS_MASK << 1)
///Map option, resource is directly coded inside instruction
#define VM_MAP_OPTION_RESOURCE_LITERAL		(VM_LITERAL << 1)
///Map option, resource is placed at an address in memory
#define VM_MAP_OPTION_RESOURCE_ADDRESS		(VM_ADDRESS << 1)

///Map option mask to determine the lifetime of a mapping
#define VM_MAP_OPTION_LIFETIME_MASK			0x04
///Map option, URL-Map should be used once
#define VM_MAP_OPTION_LIFETIME_ONCE			0x00
///Map option, URL-Map should be used until unmapped (or ever)
#define VM_MAP_OPTION_LIFETIME_EVER			0x04

///Map option mask to determine the direction of the mapping
#define VM_MAP_OPTION_DIRECTION_MASK		0x08
///Map option, value is accessible via coap server
#define VM_MAP_OPTION_DIRECTION_SERVER		0x00
///Map option, URL-Map is handled via coap client
#define VM_MAP_OPTION_DIRECTION_CLIENT		0x08

///Map option mask to determine which coap method to use
#define VM_MAP_OPTION_METHOD				0xf0
///Map option, client has to use GET method
#define VM_MAP_OPTION_METHOD_GET			0x10
///Map option, client has to use POST method
#define VM_MAP_OPTION_METHOD_POST			0x20
///Map option, client has to use PUT method
#define VM_MAP_OPTION_METHOD_PUT			0x30

///Map status, URL-Map is done
#define VM_MAP_STATUS_DONE					0x01
///Map status, a timeout occurred
#define VM_MAP_STATUS_ERROR_TIMEOUT			0x02
///Map status, requested resource not found
#define VM_MAP_STATUS_ERROR_404				0x04
///Map status, server error
#define VM_MAP_STATUS_ERROR_SERVER			0x08
///Map status, value could not be stored in memory
#define VM_MAP_STATUS_ERROR_STORE			0x10
///Map status, value could not be loaded from memory
#define VM_MAP_STATUS_ERROR_LOAD			0x20
///Map status, URL-Map is not used (not mapped)
#define VM_MAP_STATUS_NO_MAPPING			0x80

typedef struct {
	///OPTYPE: 4Bit id, 4Bit Optype (1Bit unused, 2Bit type, 1Bit literal or address(unused))
	uint8_t optype;
	///Map-Options: 4Bit CoAP Method, 1Bit server or client (server serves value on request (GET only), client handles value via CoAP Method), 1Bit Lifetime, 1Bit URL adresse im befehl(Literal) oder als adresse (Address), 1Bit URL resource im befehl(Literal) oder als adresse (Address)
	uint8_t map_options;
	///Memory address for value
	uint16_t value_address;
	///Memory address for url
	uint16_t url_address;
	///Memory address for resource
	uint16_t resource_address;
	///Port
	uint16_t port;
	///Statuscode
	uint8_t status;
} url_map_t;


#endif /* INCLUDES_URL_MAPPING_H_ */
