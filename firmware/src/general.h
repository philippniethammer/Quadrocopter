/*
 * general.h
 *
 *  Created on: 18.04.2011
 *      Author: philipp
 *
 * This file contains config defines and some general functions.
 */

#ifndef GENERAL_H_
#define GENERAL_H_

#include <stdint.h>

#define TWI_FREQ 100000
#define SENSOR_ACC_ADDRESS 0x1D
#define USART_BAUD 9600

void delay_ms(int length);

void debug_blink(uint8_t times);

#endif /* GENERAL_H_ */
