/*
 * main.c
 *
 *  Created on: 05.04.2011
 *      Author: Philipp Niethammer
 *      last edited by AlKi 13. 04. 2011
 */

#include "motors.h"


/*
 * executing startuproutines
 * executes all initializations and routines needed to *boot*
 */
void initializeAll(void){
	initializeMotors();


}



int main(void)
{

	initializeAll();


    while (1)
    {
        __asm__ volatile("nop");		// so the endless loop isn't optimized away
    }

    return (1);	// should never happen
}

