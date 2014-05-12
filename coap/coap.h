#ifndef COAP_H
#define COAP_H

#include "ipstack.h"
#include <stdlib.h>

//CoAP option
typedef struct {
    uint16_t  optionLength;
    uint8_t *optionValue;
}CoapOption;

//CoAP options supported
typedef struct  {
	CoapOption uriPath;
	CoapOption uriQuery;
	CoapOption contentFormat;
	CoapOption accept;
}CoapOptions;

//CoAP header
typedef struct
{
  UDPhdr udp;
  uint8_t tokenLength : 4;
  uint8_t type : 2;
  uint8_t version : 2;
  uint8_t code;
  uint16_t msgID;
}CoapHdr;

//CoAP message
typedef struct
{
  CoapHdr *coapHdr;
  uint8_t* token;
  CoapOptions options;
}CoapMsg;

//CoAP options
# define COAP_OPTIONS_INITIALISATION { { 0, NULL }, { 0, NULL }, { 0, NULL }, { 0, NULL } }

# define COAP_OPTION_IF_MATCH        1
# define COAP_OPTION_URI_HOST        3
# define COAP_OPTION_ETAG            4
# define COAP_OPTION_IF_NONE         5
# define COAP_OPTION_URI_PORT        7
# define COAP_OPTION_LOCATION_PATH   8
# define COAP_OPTION_URI_PATH       11
# define COAP_OPTION_CONTENT_FORMAT 12
# define COAP_OPTION_MAX_AGE        14
# define COAP_OPTION_URI_QUERY      15
# define COAP_OPTION_ACCEPT         17
# define COAP_OPTION_LOCATION_QUERY 20
# define COAP_OPTION_PROXY_URI      35
# define COAP_OPTION_PROXY_SCHEME   39
# define COAP_OPTION_SIZE1          60

//CoAP codes
# define DEFINE_COAP_CODE(h, l)   (((h) & 0x7) << 5 | ((l) & 0x1f))

# define COAP_CODE_CLASS_REQUEST  DEFINE_COAP_CODE(0, 0)
# define COAP_CODE_CLASS_SUCCESS  DEFINE_COAP_CODE(2, 0)
# define COAP_CODE_CLASS_C_ERROR  DEFINE_COAP_CODE(4, 0)
# define COAP_CODE_CLASS_S_ERROR  DEFINE_COAP_CODE(5, 0)
# define COAP_CODE_CLASS_MASK     DEFINE_COAP_CODE(~0,0)

# define COAP_CODE_EMPTY          DEFINE_COAP_CODE(0, 00)
# define COAP_CODE_GET            DEFINE_COAP_CODE(0, 01)
# define COAP_CODE_POST           DEFINE_COAP_CODE(0, 02)
# define COAP_CODE_PUT            DEFINE_COAP_CODE(0, 03)
# define COAP_CODE_DELETE         DEFINE_COAP_CODE(0, 04)

# define COAP_CODE_CREATED        DEFINE_COAP_CODE(2, 01)
# define COAP_CODE_DELETED        DEFINE_COAP_CODE(2, 02)
# define COAP_CODE_VALID          DEFINE_COAP_CODE(2, 03)
# define COAP_CODE_CHANGED        DEFINE_COAP_CODE(2, 04)
# define COAP_CODE_CONTENT        DEFINE_COAP_CODE(2, 05)

# define COAP_CODE_BAD_REQUEST    DEFINE_COAP_CODE(4, 00)
# define COAP_CODE_UNAUTHORIZED   DEFINE_COAP_CODE(4, 01)
# define COAP_CODE_BAD_OPTION     DEFINE_COAP_CODE(4, 02)
# define COAP_CODE_FORBIDDEN      DEFINE_COAP_CODE(4, 03)
# define COAP_CODE_NOT_FOUND      DEFINE_COAP_CODE(4, 04)
# define COAP_CODE_NOT_ALLOWED    DEFINE_COAP_CODE(4, 05)
# define COAP_CODE_NOT_ACCEPTABLE DEFINE_COAP_CODE(4, 06)
# define COAP_CODE_PRECOND_FAILED DEFINE_COAP_CODE(4, 12)
# define COAP_CODE_TOO_LARGE      DEFINE_COAP_CODE(4, 13)
# define COAP_CODE_UNSUPP_FORMAT  DEFINE_COAP_CODE(4, 15)

# define COAP_CODE_SERVER_ERROR    DEFINE_COAP_CODE(5, 00)
# define COAP_CODE_NOT_IMPLEMENTED DEFINE_COAP_CODE(5, 01)
# define COAP_CODE_BAD_GATEWAY     DEFINE_COAP_CODE(5, 02)
# define COAP_CODE_SERV_UNAVAIL    DEFINE_COAP_CODE(5, 03)
# define COAP_CODE_GATEWAY_TIMEOUT DEFINE_COAP_CODE(5, 04)
# define COAP_CODE_PROXY_NOT_SUPP  DEFINE_COAP_CODE(5, 05)

//CoAP types
# define COAP_TYPE_CONFIRMABLE      0
# define COAP_TYPE_NON_CONFIRMABLE  1
# define COAP_TYPE_ACKNOWLEDGEMENT  2
# define COAP_TYPE_RESET            3

int receiveCoapMsg(uint8_t* packet, uint16_t port, CoapMsg *coapMsgPtr);
void printCoapMsg(CoapMsg coapMsg);
void sendCoapMsg(uint8_t* packet, CoapMsg* coapMsg, char *payload);

#endif
