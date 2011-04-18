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
#include "general.h"
#include "serialcom.h"
#include "usart.h"

/*
 * executing startup routines
 * executes all initializations and routines needed to *boot*
 */
void initializeAll(void){
	initializeMotors();
	SerialCom_init();
	/*
	if (!Sensors_Init()) {
		debug_blink(2);
	}
	*/

}


int main(void)
{

	initializeAll();
	uint8_t count = 0;

    while (1)
    {
    	SerialCom_LoopStep();

    	//Motor PWM testing.

		for (int i = 0; i < 255; i++) {
			setMotorFront(255-i);
			setMotorLeft(i);
			setMotorRight(i);
		}
		if (count == 0) {
			USART_transmit(0x88);
		}
    	count++;
        /*
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

