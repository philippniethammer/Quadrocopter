/*
 * main.c
 *
 *  Created on: 05.04.2011
 *      Author: Philipp Niethammer
 *      last edited by AlKi 13. 04. 2011
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>
#include "motors.h"
#include "sensors.h"
#include "general.h"
#include "serialcom.h"
#include "usart.h"

#define ENABLE_USART 1
#define ENABLE_SENSORS 1

/*
 * executing startup routines
 * executes all initializations and routines needed to *boot*
 */
void initializeAll(void){
	initializeMotors();
	SerialCom_init();

	/**
	if (!Sensors_Init()) {
		debug_blink(2);
	}*/


}


int main(void)
{

	DDRA = 0xff;

	delay_ms(1000);

	debug_blink(3);

	initializeAll();
	uint8_t count = 0;

	setMotorBack(128);

    while (1)
    {
    	SerialCom_LoopStep();

    	//Motor PWM testing.

		if (count == 0) {
			USART_transmit(0x88);
		}
    	count++;


    	/**
    	// Acc testing.
    	if (!Sensors_AccPullData()) {
    		debug_blink(3);
    	}
    	setMotorFront(128 + Sensors_AccGetX());
    	setMotorLeft(128 + Sensors_AccGetY());
    	setMotorRight(128 + Sensors_AccGetZ());
    	*/

    }

    return (1);	// should never happen
}

