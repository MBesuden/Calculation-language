
#include "net/gnrc/coap.h"
#include "od.h"
#include "fmt.h"

#ifdef __arm__
#define int32_t int
#endif

static ssize_t _sensor_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len);
static ssize_t _setpoint_get_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len);
static ssize_t _setpoint_post_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len);
static ssize_t _heater_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len);

static ssize_t _temp_out_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len);

static ssize_t _post_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len);
static ssize_t _get_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len);
static ssize_t _get_float_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len);
static ssize_t _get_timeout_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len);
static ssize_t _get_timeout_float_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len);

static uint32_t getTimeout(uint32_t min, uint32_t max);

extern double getSetpoint(void);
extern void setSetpoint(double);
extern void setOutsideTemp(double);
extern double getTemperature(void);
extern void heater_on(int percentage);

/* CoAP resources */
static const coap_resource_t _resources[] = {
	{ "/get", COAP_GET, _get_handler },
	{ "/get/float", COAP_GET, _get_float_handler },
	{ "/get/timeout", COAP_GET, _get_timeout_handler },
	{ "/get/timeout/float", COAP_GET, _get_timeout_float_handler },
	{ "/heater", COAP_POST, _heater_handler },
	{ "/post", COAP_POST, _post_handler },
	{ "/sensor", COAP_GET, _sensor_handler },
	{ "/setpoint", COAP_GET, _setpoint_get_handler },
	{ "/setpoint", COAP_POST, _setpoint_post_handler },
	{ "/temp-out", COAP_POST, _temp_out_handler },
};

static gcoap_listener_t _listener_sensor = {
	(coap_resource_t *)&_resources[0],
	sizeof(_resources) / sizeof(_resources[0]),
	NULL
};

static ssize_t _sensor_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len)
{
	//add some timeout to simulate network rtt
	xtimer_usleep(getTimeout(30000, 50000));//timeout between 30 and 50 ms
	gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
	double temp = getTemperature();
	char payload[20];
	size_t payload_len = snprintf(payload, 20, "%d.%04d", (int)temp, (int)(abs(temp * 10000)) % 10000);
	pdu->payload = (unsigned char*)payload;

	return gcoap_finish(pdu, payload_len, COAP_FORMAT_TEXT);
}

static ssize_t _heater_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len)
{
	xtimer_usleep(getTimeout(30000, 50000));//timeout between 30 and 50 ms
//	printf("Heater:\n");
	if(pdu->payload_len < 1)
	{
		return gcoap_response(pdu, buf, len, COAP_CODE_BAD_REQUEST);
	}
	//be sure payload is null terminated for atoi();
	pdu->payload[pdu->payload_len] = 0;
	int percentage = atoi((char*)pdu->payload);
	heater_on(percentage);
//	printf("heater set to %d", percentage);
	return gcoap_response(pdu, buf, len, COAP_CODE_VALID);
}

static ssize_t _setpoint_get_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len)
{
	xtimer_usleep(getTimeout(30000, 50000));//timeout between 30 and 50 ms
	gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
	pdu->payload_len = snprintf((char*)pdu->payload, len, "%d.%04d", (int32_t)getSetpoint(), (int32_t)abs(getSetpoint() * 10000) % 10000);
	return gcoap_finish(pdu, pdu->payload_len, COAP_FORMAT_TEXT);
}

static ssize_t _setpoint_post_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len)
{
	xtimer_usleep(getTimeout(30000, 50000));//timeout between 30 and 50 ms
	pdu->payload[pdu->payload_len] = 0;
	setSetpoint(atof((char*)pdu->payload));
	return gcoap_response(pdu, buf, len, COAP_CODE_VALID);
}

static ssize_t _temp_out_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len)
{
	xtimer_usleep(getTimeout(30000, 50000));//timeout between 30 and 50 ms
	pdu->payload[pdu->payload_len] = 0;
	setOutsideTemp(atof((char*)pdu->payload));
	return gcoap_response(pdu, buf, len, COAP_CODE_VALID);
}

static ssize_t _post_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len)
{
	xtimer_usleep(getTimeout(30000, 50000));//timeout between 30 and 50 ms
	printf("POST handler: got ");
	for(unsigned i = 0; i < pdu->payload_len; i++)
	{
		switch(pdu->content_type)
		{
		case COAP_FORMAT_TEXT:
			printf("%.1s", (char*)&pdu->payload[i]);
			break;
		case COAP_FORMAT_OCTET:
			printf("  %0x02x,", pdu->payload[i]);
			break;
		default:
			printf("  %0x02x,", pdu->payload[i]);
			break;
		}
	}
	return gcoap_response(pdu, buf, len, COAP_CODE_CONTENT);
}

static ssize_t _get_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len)
{
	gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
	int value = rand();
	printf("GET Handler: sending %d\n", value);
	char payload[20];
	snprintf(payload, 20, "%d", value);
	size_t payload_len = fmt_strlen(payload);
	pdu->payload = (unsigned char*)payload;

	return gcoap_finish(pdu, payload_len, COAP_FORMAT_TEXT);
}

static ssize_t _get_float_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len)
{
	gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
	float value = ((float)rand() / RAND_MAX) * 100.0;
	printf("GET Handler: sending %d.%04d\n", (int32_t)value, (int32_t)(abs(value * 10000)) % 10000);
	char payload[20];
	snprintf(payload, 20, "%d.%04d", (int32_t)value, (int32_t)(abs(value * 10000)) % 10000);
	size_t payload_len = fmt_strlen(payload);
	pdu->payload = (unsigned char*)payload;

	return gcoap_finish(pdu, payload_len, COAP_FORMAT_TEXT);
}

static ssize_t _get_timeout_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len)
{
	uint32_t timeout = getTimeout(20000, 5500000);
	printf("sending get with timeout %" PRIu32 "ms\n", timeout / 1000);
	xtimer_usleep(timeout);
	return _get_handler(pdu, buf, len);
}

static ssize_t _get_timeout_float_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len)
{
	uint32_t timeout = getTimeout(20000, 5500000);
	printf("sending get float with timeout %" PRIu32 "ms\n", timeout / 1000);
	xtimer_usleep(timeout);
	return _get_float_handler(pdu, buf, len);
}

static uint32_t getTimeout(uint32_t min, uint32_t max)
{
	return min + rand() / (RAND_MAX / (max - min + 1) + 1);
}


void coap_init(void);
void coap_init(void)
{
	gcoap_register_listener(&_listener_sensor);
}

