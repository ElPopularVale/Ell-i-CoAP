#include "enc28j60.h"
#include "usart.h"
#include "ipstack.h"
#include "coap.h"
#include <string.h>

int receiveCoapMsg(uint8_t* packet, uint16_t port, CoapMsg *coapMsg) {
	coapMsg->coapHdr = (CoapHdr*) packet;

	//Receive UDP request from browser
	coapMsg->coapHdr->udp.destPort = 0;
	do {
		GetPacket(UDPPROTOCOL, packet);
	} while (!(coapMsg->coapHdr->udp.destPort == (uint16_t) HTONS(port)));

	//Parse Token
	uint8_t token[coapMsg->coapHdr->tokenLength];
	coapMsg->token = token;
	memcpy(coapMsg->token, packet + sizeof(CoapHdr),
			coapMsg->coapHdr->tokenLength);

	//Parse options
	uint16_t totalLen; //Total UDP packet length
	totalLen = HTONS(coapMsg->coapHdr->udp.ip.len) + sizeof(EtherNetII) - 2; //-2 Added to make it work with Copper who adds two bytes at the end of the options value
	uint8_t* inputPtr = packet + sizeof(CoapHdr)
			+ coapMsg->coapHdr->tokenLength;
	uint8_t* endPtr = packet + totalLen;
	uint16_t optionNumber = 0;
	coapMsg->options = (CoapOptions) COAP_OPTIONS_INITIALISATION;

	while (inputPtr < endPtr && inputPtr[0] != 0xFF) {
		uint8_t optionByte = inputPtr[0];
		uint16_t optionDelta = optionByte >> 4;
		uint16_t optionLength = optionByte & 0x0F;

		//Process option delta
		switch (optionDelta) {
		case 13:
			inputPtr++;
			optionDelta = (*(inputPtr) - 13) & 0xff;
			break;
		case 14:
			inputPtr++;
			optionDelta = *(inputPtr) - 269;
			inputPtr++;
			break;
		case 15:
			usartSendString("Error: payload marker detected in option delta\r");
			return -1;
			break;
		default:
			break;
		}

		//Process option length
		switch (optionLength) {
		case 13:
			inputPtr++;
			optionLength = (*(inputPtr) - 13) & 0xff;
			break;
		case 14:
			inputPtr++;
			optionLength = *(inputPtr) - 269;
			inputPtr++;
			break;
		case 15:
			usartSendString("Error: message format error\r");
			return -1;
			break;
		default:
			break;
		}

		//Process the option
		uint8_t *optionValue = ++inputPtr;
		optionNumber += optionDelta;

		switch (optionNumber) {
		case COAP_OPTION_URI_PATH:
			coapMsg->options.uriPath.optionLength = optionLength;
			coapMsg->options.uriPath.optionValue = optionValue;
			break;
		case COAP_OPTION_CONTENT_FORMAT:
			coapMsg->options.contentFormat.optionLength = optionLength;
			coapMsg->options.contentFormat.optionValue = optionValue;
		case COAP_OPTION_URI_QUERY:
			coapMsg->options.uriQuery.optionLength = optionLength;
			coapMsg->options.uriQuery.optionValue = optionValue;
		case COAP_OPTION_ACCEPT:
			coapMsg->options.accept.optionLength = optionLength;
			coapMsg->options.accept.optionValue = optionValue;
			break;
		case COAP_OPTION_IF_MATCH:
		case COAP_OPTION_URI_HOST:
		case COAP_OPTION_ETAG:
		case COAP_OPTION_IF_NONE:
		case COAP_OPTION_URI_PORT:
		case COAP_OPTION_LOCATION_PATH:
		case COAP_OPTION_MAX_AGE:
		case COAP_OPTION_LOCATION_QUERY:
		case COAP_OPTION_PROXY_URI:
		case COAP_OPTION_PROXY_SCHEME:
		case COAP_OPTION_SIZE1:
		default:
			usartSendString("Error: Option not supported\r");
			return -1;
			break;
		}

		//Update pointer to next option
		inputPtr += optionLength;
	}
	usartSendString("\rCoAP message received OK\r");
	return 0;
}

void printCoapMsg(CoapMsg coapMsg) {
	int i;

	//Print type
	usartSendString("Type: ");
	switch (coapMsg.coapHdr->type) {
	case COAP_TYPE_CONFIRMABLE:
		usartSendString("Confirmable\r");
		break;
	case COAP_TYPE_NON_CONFIRMABLE:
		usartSendString("Non-confirmable\r");
		break;
	case COAP_TYPE_ACKNOWLEDGEMENT:
		usartSendString("Acknowledgement\r");
		break;
	case COAP_TYPE_RESET:
		usartSendString("Reset\r");
		break;
	default:
		usartSendString("Print error: Type not recognized\r");
		break;
	}

	//Print code
	usartSendString("Code: ");
	switch (coapMsg.coapHdr->code) {
	case COAP_CODE_EMPTY:
		usartSendString("EMPTY\r");
		break;
	case COAP_CODE_GET:
		usartSendString("GET\r");
		break;
	case COAP_CODE_POST:
		usartSendString("POST\r");
		break;
	case COAP_CODE_PUT:
		usartSendString("PUT\r");
		break;
	case COAP_CODE_DELETE:
		usartSendString("DELETE\r");
		break;
	case COAP_CODE_CONTENT:
		usartSendString("CONTENT\r");
		break;
	default:
		usartSendString("Print error: Method code not recognized\r");
		break;
	}

	//Print message ID
	usartSendString("MID: ");
	printf("%u\r", (uint16_t) HTONS(coapMsg.coapHdr->msgID));

	//Print token
	if (coapMsg.coapHdr->tokenLength) {
		usartSendString("Token: ");
		for (i = 0; i < coapMsg.coapHdr->tokenLength; i++)
			printf("%u", coapMsg.token[i]);
		usartSendString("\r");
	}

	//Print options
	if (coapMsg.options.uriPath.optionLength) {
		usartSendString("Option: Uri-Path\r");
		usartSendString("Option value: ");
		for (i = 0; i < coapMsg.options.uriPath.optionLength; i++)
			usartSend8(coapMsg.options.uriPath.optionValue[i]);
		usartSendString("\r");
	}

	if (coapMsg.options.contentFormat.optionLength) {
		usartSendString("Option: Content-Format\r");
		usartSendString("Option value: ");
		for (i = 0; i < coapMsg.options.contentFormat.optionLength; i++)
			printf("%u", coapMsg.options.contentFormat.optionValue[i]);
		usartSendString("\r");
	}

	if (coapMsg.options.uriQuery.optionLength) {
		usartSendString("Option: Uri-Query\r");
		usartSendString("Option value: ");
		for (i = 0; i < coapMsg.options.uriQuery.optionLength; i++)
			printf("%u", coapMsg.options.uriQuery.optionValue[i]);
		usartSendString("\r");
	}

	if (coapMsg.options.accept.optionLength) {
		usartSendString("Option: Accept\r");
		usartSendString("Option value: ");
		for (i = 0; i < coapMsg.options.accept.optionLength; i++)
			printf("%u", coapMsg.options.accept.optionValue[i]);
		usartSendString("\r");
	}
}

void sendCoapMsg(uint8_t* packet, CoapMsg* coapMsg, char *payload) {
	coapMsg->coapHdr = (CoapHdr*) packet;

	//Reset options
	coapMsg->options = (CoapOptions) COAP_OPTIONS_INITIALISATION;

	//Swap source and destination MAC addresses
	memcpy(coapMsg->coapHdr->udp.ip.eth.DestAddrs,
			coapMsg->coapHdr->udp.ip.eth.SrcAddrs, sizeof(deviceMAC));
	memcpy(coapMsg->coapHdr->udp.ip.eth.SrcAddrs, deviceMAC, sizeof(deviceMAC));

	//Swap source and destination IP addresses
	memcpy(coapMsg->coapHdr->udp.ip.dest, coapMsg->coapHdr->udp.ip.source,
			sizeof(deviceIP));
	memcpy(coapMsg->coapHdr->udp.ip.source, deviceIP, sizeof(deviceIP));

	//Swap ports
	uint16_t destPort = coapMsg->coapHdr->udp.destPort;
	coapMsg->coapHdr->udp.destPort = coapMsg->coapHdr->udp.sourcePort;
	coapMsg->coapHdr->udp.sourcePort = destPort;

	//Setup CoAP response
	if (coapMsg->coapHdr->type == COAP_TYPE_CONFIRMABLE)
		coapMsg->coapHdr->type = COAP_TYPE_ACKNOWLEDGEMENT;
	coapMsg->coapHdr->code = COAP_CODE_CONTENT;

	//Payload marker
	uint8_t ff = 0xFF;
	printf("Token length: %u\r", coapMsg->coapHdr->tokenLength);
	memcpy(packet + sizeof(CoapHdr) + coapMsg->coapHdr->tokenLength, &ff, 1);

	//Payload
	uint8_t payloadLen = strlen(payload);
	memcpy(packet + sizeof(CoapHdr) + coapMsg->coapHdr->tokenLength + 1,
			payload, payloadLen);

	//Total packet length
	uint16_t len = sizeof(CoapHdr) + coapMsg->coapHdr->tokenLength + 1
			+ payloadLen;

	//Setup UDP packet
	coapMsg->coapHdr->udp.len = (uint16_t) HTONS((len-sizeof(IPhdr)));
	coapMsg->coapHdr->udp.ip.len = (uint16_t) HTONS((len-sizeof(EtherNetII)));
	coapMsg->coapHdr->udp.ip.ident = 0xdeee;

	//Calculate UDP and IP checksums
	coapMsg->coapHdr->udp.chksum = 0;
	coapMsg->coapHdr->udp.ip.chksum = 0;
	uint16_t pseudochksum = (uint16_t) chksum(UDPPROTOCOL + len - sizeof(IPhdr),
			coapMsg->coapHdr->udp.ip.source, sizeof(deviceIP) * 2);
	uint16_t chk1, chk2;
	chk1 = ~(chksum(pseudochksum, packet + sizeof(IPhdr), len - sizeof(IPhdr)));
	coapMsg->coapHdr->udp.chksum = (uint16_t) HTONS(chk1);
	chk2 = ~(chksum(0, packet + sizeof(EtherNetII),
			sizeof(IPhdr) - sizeof(EtherNetII)));
	coapMsg->coapHdr->udp.ip.chksum = (uint16_t) HTONS(chk2);

	//Send packet
	enc28j60_send_packet(packet, len);
	usartSendString("\rCoAP message sent OK\r");
}
