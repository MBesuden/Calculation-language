/*
 * Copyright (C) 2017 Mattes Besuden
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @brief       Implementation of gcoap client used by the Calculation VM to handle GET and POST requests.
 *
 * @author      Mattes Besuden <besuden@uni-bremen.de>
 */


#ifdef __cplusplus
extern "C" {
#endif

#include "net/gnrc/coap.h"
#include "od.h"
#include "fmt.h"

#include "gcoap_shared_memory_functions.h"
#include "Opcodes.h"
#include "xtimer.h"

///Utility array to store which mapping (array index) has pending requests (CoAP packet token).
static uint16_t active_requests[VM_MEMORY_MAP_SIZE];
static int32_t get_active_index(coap_pkt_t* pdu);
static void _resp_handler(unsigned req_state, coap_pkt_t* pdu);

/**
 * Callback handler for CoAP responses.
 * @param req_state
 * @param pdu
 */
static void _resp_handler(unsigned req_state, coap_pkt_t* pdu)
{
	int32_t index = get_active_index(pdu);
	if(index < 0)
	{
		return;
	}
	active_requests[index] = 0; //remove active request
    if (req_state == GCOAP_MEMO_TIMEOUT) {
    	gcoap_error(index, VM_MAP_STATUS_ERROR_TIMEOUT);
        return;
    }
    else if (req_state == GCOAP_MEMO_ERR) {
        return;
    }

    if (pdu->payload_len) {
        if (coap_get_code_class(pdu) == COAP_CLASS_CLIENT_FAILURE
                || coap_get_code_class(pdu) == COAP_CLASS_SERVER_FAILURE) {
        	//failure
        	//set failure code...
        	gcoap_error(index, VM_MAP_STATUS_ERROR_SERVER);
        }
        else {
            //handle response
        	if(gcoap_store_value(pdu->content_type, index, pdu->payload, pdu->payload_len))
        	{
        		//store error
        		gcoap_error(index, VM_MAP_STATUS_ERROR_STORE);
        	}
        	else
        	{
        		gcoap_done(index);
        	}
//        	printf("Got answer: %.*s\n", pdu->payload_len, pdu->payload);
        }
    }
    else {
        //empty payload (must have been a post or put or a GET error)
    	if(coap_get_code_class(pdu) == COAP_CLASS_SERVER_FAILURE)
    	{
    		gcoap_error(index, VM_MAP_STATUS_ERROR_SERVER);
    	}
    	else
    	{
    		//no error -> done
    		gcoap_done(index);
    	}
    }
}

/**
 * Utility function to get the index of an active CoAP request.
 * @param pdu
 * @return
 */
static int32_t get_active_index(coap_pkt_t* pdu)
{
	for(uint8_t i = 0; i < VM_MEMORY_MAP_SIZE; i++)
	{
		if(ntohs(pdu->hdr->id) == active_requests[i])
		{
			return i;
		}
	}
	return -1;
}

/**
 * Sends a CoAP request.
 * @param buf
 * @param len
 * @param mapping
 * @return
 */
static size_t _send(uint8_t *buf, size_t len, const url_map_t* mapping)
{
    size_t bytes_sent;
    uint16_t port = mapping->port;
    ipv6_addr_t addr;
    /* parse destination address */
    char host[40];
    gcoap_get_host(mapping, host);
    if (ipv6_addr_from_str(&addr, host) == NULL) {
        printf("gcoap_c: invalid address\n");
        return 0;
    }
    /* parse port */
    if (port == 0) {
        port = GCOAP_PORT;//assume standard port
    }

    bytes_sent = gcoap_req_send(buf, len, &addr, port, _resp_handler);
    return bytes_sent;
}

/**
 * Function to check for CoAP client mappings. Sends CoAP requests to the host and resource specified in the mapping.
 */
void check_clientmappings(void);
void check_clientmappings(void)
{
	const url_map_t* mappings = gcoap_get_mappings();
	uint8_t mapsize = gcoap_get_map_size();
	for(uint8_t i = 0; i < mapsize; i++)
	{
#ifdef __arm__
		xtimer_usleep(2000000 / VM_MEMORY_MAP_SIZE); //check every mapping about every 2s
#else
		xtimer_usleep(1000000 / VM_MEMORY_MAP_SIZE); //check every mapping about every 1s
#endif
		if(mappings[i].url_address == NO_MAPPING || mappings[i].value_address == NO_MAPPING || (mappings[i].status > 1 && (mappings[i].map_options & VM_MAP_OPTION_LIFETIME_ONCE)))
		{
			continue;
		}
		if(mappings[i].map_options & VM_MAP_OPTION_DIRECTION_CLIENT)
		{
			if(active_requests[i] != 0)
			{
				continue;
			}
			uint8_t buf[GCOAP_PDU_BUF_SIZE];
			coap_pkt_t pdu;
			size_t len = 0;
			size_t new_payload_len;
			char ressource[64];
			gcoap_get_resource(&mappings[i], ressource);
			if(strlen(ressource) < 1)
			{
				printf("gcoap_c: no resource specified");
				continue;
			}
			switch(mappings[i].map_options & VM_MAP_OPTION_METHOD)
			{
			case VM_MAP_OPTION_METHOD_GET:
				len = gcoap_request(&pdu, &buf[0], GCOAP_PDU_BUF_SIZE, COAP_GET, ressource);
				break;
			case VM_MAP_OPTION_METHOD_POST:
				gcoap_req_init(&pdu, &buf[0], GCOAP_PDU_BUF_SIZE, COAP_POST, ressource);
				new_payload_len = gcoap_load_value(pdu.content_type, pdu.payload, pdu.payload_len, i);
				len = gcoap_finish(&pdu, new_payload_len, COAP_FORMAT_TEXT);
				break;
			case VM_MAP_OPTION_METHOD_PUT:
				gcoap_req_init(&pdu, &buf[0], GCOAP_PDU_BUF_SIZE, COAP_PUT, ressource);
				new_payload_len = gcoap_load_value(pdu.content_type, pdu.payload, pdu.payload_len, i);
				len = gcoap_finish(&pdu, new_payload_len, COAP_FORMAT_TEXT);
				break;
			default:
				break;
			}
			if (!_send(&buf[0], len, &mappings[i]))
			{
                //error
			}
			else
			{
				active_requests[i] = ntohs(pdu.hdr->id);
			}

		}
	}
}

/**
 * Thread to check URL mappings.
 * @param args
 */
void *coap_c_thread(void *args)
{
	(void) args;
	while(1)
	{
		check_clientmappings();
	}
}

#ifdef __cplusplus
}
#endif
