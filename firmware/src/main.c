/*
 * main.c
 *
 *  Created on: 05.04.2011
 *      Author: Philipp Niethammer
 *      last edited by AlKi 13. 04. 2011
 */

#include "motors.h"

int main(void)
{

	initializeMotors();

    while (1)
    {
        __asm__ volatile("nop");		// so the endless loop isn't optimized away
    }

    return (1);	// should never happen
}
