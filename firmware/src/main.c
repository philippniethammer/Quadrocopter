/*
 * main.c
 *
 *  Created on: 05.04.2011
 *      Author: Philipp Niethammer
 *      last edited by AlKi 13. 04. 2011
 */

#include "motors.h"
#include "util/delay.h"


/*
 * executing startuproutines
 * executes all initializations and routines needed to *boot*
 */
void initializeAll(void){
	initializeMotors();


}

void delay_ms(int length) {
	for (int i=0; i<length; i++) {
		_delay_ms(1.0);
	}
}


int main(void)
{

	initializeAll();


    while (1)
    {
    	//Motor PWM testing.
        for (int i = 0; i < 255; i++) {
        	setMotorBack(255-i);
        	setMotorFront(255-i);
        	setMotorLeft(i);
        	setMotorRight(i);
        	delay_ms(500);
        }
    }

    return (1);	// should never happen
}

