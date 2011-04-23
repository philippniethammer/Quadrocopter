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
#if (ENABLE_USART == 1)
	SerialCom_init();
#endif

#if (ENABLE_SENSORS == 1)
	if (!Sensors_Init()) {
		debug_blink(2);
	}
#endif

}


int main(void)
{

	DDRA = 0xff;

	initializeAll();

	setMotorBack(128);

    while (1)
    {
#if (ENABLE_USART == 1)
    	SerialCom_LoopStep();
#endif

#if (ENABLE_SENSORS == 1)
    	// Acc testing.
    	if (!Sensors_AccPullData()) {
    		debug_blink(3);
    	}
#endif

    }

    return (1);	// should never happen
}

