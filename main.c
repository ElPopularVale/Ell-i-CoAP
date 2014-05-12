#include "stm32f0xx.h"
#include "stm32f0xx_gpio.h"
#include "stm32f0xx_rcc.h"
#include "enc28j60.h"
#include "usart.h"
#include "ipstack.h"
#include "coap.h"

int main(void) {

	usartInit();
	IPstackInit();
	uint8_t packet[MAXPACKETLEN];
	uint16_t port = 5683;
	CoapMsg coapMsg;
	char *payload = "Hello";

	printf("CoAP Port: %u\r", port);
	while (1) {
		receiveCoapMsg(packet, port, &coapMsg);
		printCoapMsg(coapMsg);
		sendCoapMsg(packet, &coapMsg, payload);
		printCoapMsg(coapMsg);
	}
}
