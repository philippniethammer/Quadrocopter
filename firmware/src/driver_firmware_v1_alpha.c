/*
 * driver_firmware_v1_alpha.c
 *
 *  Created on: 17.02.2012
 *      Author: alki
 *
 *
 *
 *      pin functions:
 *      PC0 - ADC
 *      PC1 - ADC
 *      PC2 - ADC
 *      PC3 - ADC
 *      PC4 - ADC
 *      PC5 - ADC
 *      PC6 - ADC
 *      PC7 - ADC
 *
 *      PB0 - motor1 coil 2 11,1V (PB0 = 1 => coil 2 = 11,1V)
 *      PB1 - motor1 coil 3 11,1V (PB1 = 1 => coil 3 = 11,1V)
 *      PB2 - motor1 coil 2 ground (PB2 = 1 => coil 2 grounded)
 *      PB3 - motor1 coil 3 ground (PB3 = 1 => coil 3 grounded)
 *      PB4 - free on isp-connector
 *      PB5 - free on isp-connector
 *      PB6 - motor2 coil 2 ground (PB6 = 1 => coil 2 grounded)
 *      PB7 - motor2 coil 2 11,1V (PB7 = 1 => coil 2 = 11,1V)
 *
 *      PD0 - motor2 coil 1 ground (PD0 = 0 => coil 1 grounded)
 *      PD1 - motor2 coil 1 11,1V (PD1 = 1 => coil 1 = 11,1V)
 *
 */

#include "avr/io.h"

// states of the motors (0 => all off, 1/2/3 => coil 1/2/3 on)
static boolean motor1_status = 0;
static boolean motor2_status = 0;

int main(void){

	// TWI address - un-/comment proper line
	static final TWI_address = 0x02;
	//static final TWI_address = 0x03

	int motor1_ratio = 0;
	int motor2_ratio = 0;


	while(true){

	}

	return 0;
}


// makes motor 1 turn one step forward/backward
void motor1_turn(boolean forward){
	if(forward){
			switch(motor1_status){
			case 1: {

				}
			case 2:{

				}
			case 3:{

				}
			}
		}else{
			switch(motor1_status){
			case 1: {

				}
			case 2:{

				}
			case 3:{

				}
			}
		}
}


// makes motor 2 turn one step forward/backward
void motor2_turn(boolean forward){
	if(forward){
		switch(motor1_status){
		case 1: {

			}
		case 2:{

			}
		case 3:{

			}
		}
	}else{
		switch(motor1_status){
		case 1: {

			}
		case 2:{

			}
		case 3:{

			}
		}
	}
}

// grounds all motor connections of motor 1
void motor1_off(void){

}

// grounds all motor connections of motor 2
void motor2_off(void){

}
