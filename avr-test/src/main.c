/*
 * main.c
 *
 *  Created on: 05.04.2011
 *      Author: Philipp Niethammer
 *      edited by AlKi for some needed tests (actual: simple motortest)
 */
#include "motors.h"
#include <avr/io.h>



void delayX(long delay){

	for(long i = 0; i < delay; i++){
		__asm__ volatile("nop");
	}
}


int main(void){

	initializeMotors();
	setMotorBack(100);
	setMotorFront(1);
	setMotorLeft(2);
	setMotorRight(3);



	while(1){
		for(unsigned char i = 0; i <= 0xff; i++){
			setMotorFront(i);
			setMotorBack(0xff - i);
			delayX(0xfff);
		}
	}

    while (1){
        __asm__ volatile("nop");		// so the endless loop isn't optimized away
    }

    return (1);	// should never happen
}
