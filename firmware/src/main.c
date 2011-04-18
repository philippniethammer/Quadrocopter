/*
 * main.c
 *
 *  Created on: 05.04.2011
 *      Author: Philipp Niethammer
 *      last edited by AlKi 13. 04. 2011
 */

#include <util/delay.h>
#include <stdint.h>
#include "motors.h"
#include "sensors.h"

void delay_ms(int length) {
	for (int i=0; i<length; i++) {
		_delay_ms(1.0);
	}
}

void debug_blink(uint8_t times) {
	uint8_t tmp;
	tmp = getMotorvalueBack();
	for (int i=1; i < times; i++) {
		setMotorBack(255);
		delay_ms(500);
		setMotorBack(0);
		delay_ms(500);
	}
	setMotorBack(tmp);
}

/*
 * executing startup routines
 * executes all initializations and routines needed to *boot*
 */
void initializeAll(void){
	initializeMotors();
	if (!Sensors_Init()) {
		debug_blink(2);
	}

}


int main(void)
{

	initializeAll();

	setMotorBack(128);

    while (1)
    {
    	/*//Motor PWM testing.
        for (int i = 0; i < 255; i++) {
        	setMotorBack(255-i);
        	setMotorFront(255-i);
        	setMotorLeft(i);
        	setMotorRight(i);
        	delay_ms(500);
        }*/

    	// Acc testing.
    	if (!Sensors_AccPullData()) {
    		debug_blink(3);
    	}
    	setMotorFront(128 + Sensors_AccGetX());
    	setMotorLeft(128 + Sensors_AccGetY());
    	setMotorRight(128 + Sensors_AccGetZ());
    }

    return (1);	// should never happen
}

