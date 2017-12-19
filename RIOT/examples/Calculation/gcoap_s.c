/*
 * Copyright (C) 2017 Mattes Besuden
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @brief       Implementation of gcoap server used as an interface for the Calculation VM (upload of Bytecode, get status information, etc).
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
	#include "ThreadVM.h"
	#include "Opcodes.h"

	static ssize_t _dump_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len);
	static ssize_t _status_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len);
	static ssize_t _status_map_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len);
	static ssize_t _status_pid_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len);
	static ssize_t _status_vm_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len);
	static ssize_t _upload_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len);
	static ssize_t _upload_multipart_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len);
	static ssize_t _value_get_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len);
	static ssize_t _value_post_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len);

	/* CoAP resources */
	static const coap_resource_t _resources[] = {
		{ "/dump", COAP_POST, _dump_handler },
		{ "/status", COAP_GET, _status_handler },
		{ "/status/map", COAP_GET, _status_map_handler },
		{ "/status/pid", COAP_GET, _status_pid_handler },
		{ "/status/vm", COAP_GET, _status_vm_handler },
		{ "/upload", COAP_POST, _upload_handler },
		{ "/upload/multipart", COAP_POST, _upload_multipart_handler },
		{ "/value/get", COAP_POST, _value_get_handler },
		{ "/value/set", COAP_POST, _value_post_handler },
	};

	static gcoap_listener_t _listener = {
		(coap_resource_t *)&_resources[0],
		sizeof(_resources) / sizeof(_resources[0]),
		NULL
	};

	kernel_pid_t vm_thread_pid;

	/**
	 * CoAP handler which returns a memory dump starting at the address specified by POST payload.
	 * @param pdu
	 * @param buf
	 * @param len
	 * @return
	 */
	static ssize_t _dump_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len)
	{
		if(pdu->content_type != COAP_FORMAT_TEXT && pdu->content_type != COAP_FORMAT_NONE)
		{
			return gcoap_response(pdu, buf, len, COAP_CODE_UNSUPPORTED_CONTENT_FORMAT);
		}
		uint16_t startaddress = gcoap_fromHex((char) pdu->payload[0], (char) pdu->payload[1]) << 8 | gcoap_fromHex((char) pdu->payload[2], (char) pdu->payload[3]);
		uint16_t dumpsize = gcoap_dumpSize();

		if(pdu->payload_len != 4 || startaddress >= dumpsize)
		{
			return gcoap_response(pdu, buf, len, COAP_CODE_BAD_REQUEST);
		}

		const uint8_t* dump = gcoap_dumpMemory();
		size_t new_payload_len = 64;//TODO rework and check for overflows
		if(new_payload_len + startaddress >= dumpsize)
		{
			new_payload_len = dumpsize - startaddress;
		}
		gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
		memcpy(pdu->payload, &dump[startaddress], new_payload_len);

		return gcoap_finish(pdu, new_payload_len, COAP_FORMAT_OCTET);
	}

	/**
	 * CoAP handler which returns the combined status information of the device (VM, mappings, pids).
	 * @param pdu
	 * @param buf
	 * @param len
	 * @return
	 */
	static ssize_t _status_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len)
	{
		//something like:
		//VM: <status>
		//Mem: <status>
		//Map: <number of mappings>
		//		id: <status>
		//PIDs: <number of active pids>
		//		id: <Mode>
		//
		//would be useful
		gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
		size_t new_payload_len = gcoap_statusVM(pdu->payload, len, vm_thread_pid);
		new_payload_len += gcoap_statusMappings(pdu->payload + new_payload_len, len - new_payload_len);
		new_payload_len += gcoap_statusPID(pdu->payload + new_payload_len, len - new_payload_len);
		return gcoap_finish(pdu, new_payload_len, COAP_FORMAT_OCTET);
	}

	/**
	 * CoAP handler which returns the mapping status.
	 * @param pdu
	 * @param buf
	 * @param len
	 * @return
	 */
	static ssize_t _status_map_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len)
	{
		gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
		size_t payload_len = gcoap_statusMappings(pdu->payload, len);
		return gcoap_finish(pdu, payload_len, COAP_FORMAT_OCTET);
	}

	/**
	 * CoAP handler which returns the pid status.
	 * @param pdu
	 * @param buf
	 * @param len
	 * @return
	 */
	static ssize_t _status_pid_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len)
	{
		gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
		size_t payload_len = gcoap_statusPID(pdu->payload, len);
		return gcoap_finish(pdu, payload_len, COAP_FORMAT_OCTET);
	}
	/**
	 * CoAP handler which returns the VM status.
	 * @param pdu
	 * @param buf
	 * @param len
	 * @return
	 */
	static ssize_t _status_vm_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len)
	{
		gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
		size_t payload_len = gcoap_statusVM(pdu->payload, len, vm_thread_pid);
		return gcoap_finish(pdu, payload_len, COAP_FORMAT_OCTET);
	}

	/**
	 * CoAP handler which stores the payload bytecode in the shared memory and restarts the VM.
	 * @param pdu
	 * @param buf
	 * @param len
	 * @return
	 */
	static ssize_t _upload_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len)
	{
		msg_t m;
		m.content.value = VM_THREAD_STOP;
		if(msg_try_send(&m, vm_thread_pid) != 1)
		{//try to stop VM before storing new bytecode;
			return gcoap_response(pdu, buf, len, COAP_CODE_PRECONDITION_FAILED);
		}
		switch(pdu->content_type)
		{
		case 42: //OCTET
			if(pdu->payload_len > gcoap_dumpSize())
			{
				return gcoap_response(pdu, buf, len, COAP_CODE_REQUEST_ENTITY_TOO_LARGE);
			}
			for(uint16_t i = 0; i < pdu->payload_len; i++)
			{
				gcoap_store(i, pdu->payload[i]);
			}
			break;
		case 0: //TEXT
		default: //unknown assume text
			//if text, hex notation is used -> 2chars for 1 Byte
			if(pdu->payload_len % 2 == 1 || pdu->payload_len / 2 > gcoap_dumpSize())//if text, payload_len must be even -> 2chars for one byte
			{
				m.content.value = VM_THREAD_RUN;
				msg_try_send(&m, vm_thread_pid);
				//set VM_Thread to run because no new bytecode was uploaded;
				return gcoap_response(pdu, buf, len, COAP_CODE_BAD_REQUEST);
			}
			for(uint16_t i = 0; i < pdu->payload_len; i = i + 2)
			{

				gcoap_store(i / 2, gcoap_fromHex((char)pdu->payload[i], (char)pdu->payload[i+1]));
			}
			break;
		}
		m.content.value = VM_THREAD_RESTART;
		if(msg_try_send(&m, vm_thread_pid) == 1)
		{
			return gcoap_response(pdu, buf, len, COAP_CODE_VALID);
		}
		else
		{
			return gcoap_response(pdu, buf, len, COAP_CODE_INTERNAL_SERVER_ERROR);
		}
	}

	/**
	 * CoAP handler which stores the payload bytecode in the shared memory starting at the address specified by the first 4 characters (interpreted as hex address).
	 * Can be used to upload large bytecode scripts, due to the missing blockwise-transfer of gcoap. By POSTing only the address ffff to this handler, the VM will be restarted.
	 * @param pdu
	 * @param buf
	 * @param len
	 * @return
	 */
	static ssize_t _upload_multipart_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len)
	{
		//text, hex notation is used -> 2chars for 1 Byte
		if(pdu->payload_len < 4 || pdu->payload_len % 2 == 1)
		{
			return gcoap_response(pdu, buf, len, COAP_CODE_BAD_REQUEST);
		}
		msg_t m;
		m.content.value = VM_THREAD_STOP;
		if(msg_try_send(&m, vm_thread_pid) != 1)
		{//try to stop VM before storing new bytecode;
			return gcoap_response(pdu, buf, len, COAP_CODE_PRECONDITION_FAILED);
		}
		uint16_t startaddress = gcoap_fromHex((char)pdu->payload[0], (char)pdu->payload[1]) << 8 | gcoap_fromHex((char)pdu->payload[2], (char)pdu->payload[3]);
		if(pdu->payload_len == 4 && startaddress == NO_MAPPING)
		{
			//start VM
			m.content.value = VM_THREAD_RESTART;
			if(msg_try_send(&m, vm_thread_pid) == 1)
			{
				return gcoap_response(pdu, buf, len, COAP_CODE_VALID);
			}
			else
			{
				return gcoap_response(pdu, buf, len, COAP_CODE_INTERNAL_SERVER_ERROR);
			}
		}
		switch(pdu->content_type)
		{
		case COAP_FORMAT_NONE: //since gcoap example only send with COAP_FORMAT_NONE
		case COAP_FORMAT_TEXT: //TEXT
			for(uint16_t i = 0; i < (pdu->payload_len - 4); i = i + 2)
			{
				gcoap_store(startaddress + i/2, gcoap_fromHex((char)pdu->payload[i + 4], (char)pdu->payload[i+5]));
			}
			return gcoap_response(pdu, buf, len, COAP_CODE_VALID);
		default: //unknown
			return gcoap_response(pdu, buf, len, COAP_CODE_UNSUPPORTED_CONTENT_FORMAT);
		}
	}

	/**
	 * CoAP handler which handles the GET server mappings specified via VM bytecode. Since gcoap doesn't support query strings,
	 * the requested value can be requested by POSTing the mapping string.
	 * The handler checks if the string is mapped and returns the value or COAP_ERROR_404 if not mapped.
	 * Since query strings are not supported, the POST mappings can not be implemented since we had to post
	 * the map string AND the value in the same message.
	 * @param pdu
	 * @param buf
	 * @param len
	 * @return
	 */
	static ssize_t _value_get_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len)
	{
		gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
		switch(pdu->content_type)
		{
		case COAP_FORMAT_TEXT:;
			ssize_t payload_len = 0;
			//check if url (post string) is mapped and write value to pdu
			payload_len = gcoap_check_server_value(pdu->content_type, pdu->payload, pdu->payload_len, len);

			if(payload_len > 0)
			{
				return gcoap_finish(pdu, payload_len, COAP_FORMAT_TEXT);
			}
			break;
		default:
			break;
		}
		return gcoap_response(pdu, buf, len, COAP_CODE_404);
	}

	static ssize_t _value_post_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len)
	{
		return gcoap_response(pdu, buf, len, COAP_CODE_NOT_IMPLEMENTED);
	}

	/**
	 * Initializes CoAP listener, sets process ID of VM thread.
	 * @param vm_thread
	 */
	void gcoap_s_init(kernel_pid_t vm_thread)
	{
		vm_thread_pid = vm_thread;
		gcoap_register_listener(&_listener);
	}

#ifdef __cplusplus
}
#endif
