/*
 * general.c
 *
 *  Created on: 18.04.2011
 *      Author: philipp
 */


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
	tmp = getMotorvalueBack();
	for (int i=0; i < times; i++) {
		setMotorBack(255);
		delay_ms(500);
		setMotorBack(0);
		delay_ms(500);
	}
	delay_ms(500);
	setMotorBack(tmp);
}
