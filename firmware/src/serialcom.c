/*
 * serialcom.c
 *
 *  Created on: 18.04.2011
 *      Author: philipp
 */

#include <stdint.h>
#include "general.h"
#include "usart.h"
#include "motors.h"
#include "sensors.h"
#include "serialcom.h"

#define UBRR (F_CPU/(16UL*USART_BAUD))-1

#define SER_COMMAND_BLINK 0x01

uint8_t command = 0;

void SerialCom_init(void)
{
	USART_Init(UBRR);
	debug_blink(4);
}

void SerialCom_LoopStep(void)
{
	uint8_t status = USART_LoopStep();

	if (!(status & USART_STATUS_OK)) {
		debug_blink(3);
	} else {
		if (status & USART_STATUS_DATA_AVAILABLE) {
			debug_blink(2);
			command = USART_getc();

				switch (command) {
				case SER_COMMAND_BLINK:
					debug_blink(1);
					command = 0;
					break;
				}

		}
	}
}
