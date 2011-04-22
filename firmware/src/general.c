/*
 * general.c
 *
 *  Created on: 18.04.2011
 *      Author: philipp
 */


#include <avr/io.h>
#include <util/delay.h>
#include "motors.h"
#include "general.h"

void delay_ms(int length) {
	for (int i=0; i<length; i++) {
		_delay_ms(1.0);
	}
}

void debug_blink(uint8_t times) {
	uint8_t tmp;
	tmp = PORTA;
	for (int i=0; i < times; i++) {
		PORTA = 0xff;
		delay_ms(200);
		PORTA = 0x00;
		delay_ms(200);
	}
	delay_ms(200);
	PORTA = tmp;
}
