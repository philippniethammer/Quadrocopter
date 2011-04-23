/*
 * serialcom.c
 *
 *  Created on: 18.04.2011
 *      Author: philipp
 */

#include <avr/io.h>
#include <stdint.h>
#include "general.h"
#include "usart.h"
#include "motors.h"
#include "sensors.h"
#include "serialcom.h"

#define UBRR (F_CPU/(16UL*USART_BAUD))-1

#define SER_COMMAND_BLINK 0x01
#define SER_COMMAND_ACCDATA 0x10
#define SER_COMMAND_SETMOTOR1 0x08
#define SER_COMMAND_SETMOTOR2 0x09
#define SER_COMMAND_SETMOTOR3 0x0a
#define SER_COMMAND_SETMOTOR4 0x0b

uint8_t command = 0;

void SerialCom_init(void)
{
	USART_Init(UBRR);
	//XXX debug only
	DDRA = 0xff;
	debug_blink(2);
	PORTA = UBRR;
}

void SerialCom_LoopStep(void)
{
	uint8_t status = USART_LoopStep();

	if (!(status & USART_STATUS_OK)) {
		debug_blink(3);
	} else {
		if (status & USART_STATUS_DATA_AVAILABLE) {
			if (command == 0) {
				command = USART_getc();
			}

			uint8_t value;
			switch (command) {
			case SER_COMMAND_SETMOTOR4:
				if (USART_getRBufferLength() >= 1) {
					value = USART_getc();
					setMotorRight(value);
					command = 0;
				}
				break;
			case SER_COMMAND_SETMOTOR3:
				if (USART_getRBufferLength() >= 1) {
					value = USART_getc();
					setMotorLeft(value);
					command = 0;
				}
				break;
			case SER_COMMAND_SETMOTOR2:
				if (USART_getRBufferLength() >= 1) {
					value = USART_getc();
					setMotorBack(value);
					command = 0;
				}
				break;
			case SER_COMMAND_SETMOTOR1:
				if (USART_getRBufferLength() >= 1) {
					value = USART_getc();
					setMotorFront(value);
					command = 0;
				}
				break;
			case SER_COMMAND_ACCDATA:
				USART_putc(command);
				USART_putc(3);
				USART_putc(Sensors_AccGetX());
				USART_putc(Sensors_AccGetY());
				USART_putc(Sensors_AccGetZ());
				command = 0;
				break;
			case SER_COMMAND_BLINK:
				debug_blink(1);
				command = 0;
				break;
			default:
				command = 0;
			}
		}
	}
}
