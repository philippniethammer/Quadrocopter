/*
 * driver.c
 *
 *  Created on: 04.07.2012
 *      Author: alki
 *
 *
 *      pin functions:
 *
 *      PC0 - ADC0 - motor1 Vcoil2
 *      PC1 - ADC1 - motor1 Vcoil1
 *      PC2 - ADC2 - motor2 Vaverage
 *      PC3 - ADC3 - motor2 Vcoil3
 *      PC4 - ADC4 - motor2 Vcoil2
 *      PC5 - ADC5 - motor2 Vcoil1
 *      ADC6 - motor1 Vaverage
 *      ADC7 - motor1 Vcoil3
 *
 *      PB0 - motor1 coil 2 11,1V (PB0 = 1 => coil 2 = 11,1V)
 *      PB1 - motor1 coil 3 11,1V (PB1 = 1 => coil 3 = 11,1V)
 *      PB2 - motor1 coil 2 ground (PB2 = 1 => coil 2 grounded)
 *      PB3 - motor1 coil 3 ground (PB3 = 1 => coil 3 grounded)
 *      PB4 - free on isp-connector
 *      PB5 - free on isp-connector
 *      PB6 - motor2 coil 2 ground (PB6 = 1 => coil 2 grounded)
 *      PB7 - motor2 coil 3 11,1V (PB7 = 1 => coil 3 = 11,1V)
 *
 *      PD0 - motor2 coil 1 ground (PD0 = 0 => coil 1 grounded)
 *      PD1 - motor2 coil 1 11,1V (PD1 = 1 => coil 1 = 11,1V)
 *		PD2 - pseudo-SCL - used as SCL in software (INT0)
 *		PD3 - pseudo-SDA - used as SDA in software (INT1)
 *		PD4 - motor2 coil 2 11,1V
 *		PD5 - motor2 coil 3 ground
 *		PD6 - motor1 coil 1 11,1V
 *		PD7 - motor1 coil 1 ground
 */


#include "avr/io.h"
#include "stdbool.h"
#include "avr/interrupt.h"

#include "driver.h"


#define MOTORRATIOMAX 255

// states of the motors (0 to 5)
static unsigned char motor1_status = 0;
static unsigned char motor2_status = 0;




static bool motor1_running = false;
static bool motor2_running = false;
// direction: true => forward, false => backward
static bool motor1_forward = true;
static bool motor2_forward = true;

// ratio of motorpower IMPORTANT! 0 = maxPower!!! MOTORRATIOMAX = minPower
int motor1_ratio = MOTORRATIOMAX;
static int motor2_ratio = MOTORRATIOMAX;

bool motor1_tempoff = true;
bool motor2_tempoff = true;

// counting variables for motor-switch-delay
static int motor1_counter = 0;
static int motor2_counter = 0;


// ======= initialization routines =========

void initializeMotors(){
	// set all pins driving the motors as output:
	DDRB |= ((1 << PB0) | (1 << PB1) | (1 << PB2) | (1 << PB3) | (1 << PB6) | (1 << PB7));
	DDRD |= ((1 << PD0) | (1 << PD1) | (1 << PD4) | (1 << PD5) | (1 << PD6) | (1 << PD7));
	// set all pins driving the motors to 0/GND:
	PORTB &= ~((1 << PB0) | (1 << PB1) | (1 << PB2) | (1 << PB3) | (1 << PB6) | (1 << PB7));
	PORTD &= ~((1 << PD0) | (1 << PD1) | (1 << PD4) | (1 << PD5) | (1 << PD6) | (1 << PD7));
}


// ======= subroutines =====================


// turns off all motor connections to V_Motor (11,1V) of motor 1
void motor1_positive_off(void){
	PORTB &= ~((1 << PB0) | (1 << PB1) | (1 << PB2) | (1 << PB3));
	PORTD &= ~((1 << PD6) | (1 << PD7));
}


// turns back on the motor connections to V_Motor (position specific) of motor 1
void motor1_positive_on(void){
	switch(motor1_status){
			case 1:{
				MOTOR1_PHASE1_HIGH;
				break;
				}
			case 2:{
				MOTOR1_PHASE3_HIGH;
				break;
				}
			case 3:{
				MOTOR1_PHASE3_HIGH;
				break;
				}
			case 4:{
				MOTOR1_PHASE2_HIGH;
				break;
				}
			case 5:{
				MOTOR1_PHASE2_HIGH;
				break;
				}
			case 0:{
				MOTOR1_PHASE1_HIGH;
				break;
				}
			}
}


// turns off all motor connections to V_Motor of motor 2
void motor2_positive_off(void){
	PORTB &= ~((1 << PB6) | (1 << PB7));
	PORTD &= ~((1 << PD0) | (1 << PD1) | (1 << PD4) | (1 << PD5));
}


// turns back on the motor connections to V_Motor (position specific) of motor 2
void motor2_positive_on(void){
	switch(motor2_status){
			case 1:{
				MOTOR2_PHASE1_HIGH;
				break;
				}
			case 2:{
				MOTOR2_PHASE3_HIGH;
				break;
				}
			case 3:{
				MOTOR2_PHASE3_HIGH;
				break;
				}
			case 4:{
				MOTOR2_PHASE2_HIGH;
				break;
				}
			case 5:{
				MOTOR2_PHASE2_HIGH;
				break;
				}
			case 0:{
				MOTOR2_PHASE1_HIGH;
				break;
				}
			}
}



// makes motor 1 turn one step forward/backward
void motor1_turn(bool forward){
	if(forward){
		switch(motor1_status){
		case 0:{
			MOTOR1_OFF;
			MOTOR1_PHASE1_HIGH;
			MOTOR1_PHASE2_LOW;
			motor1_status = 1;
			break;
			}
		case 1:{
			MOTOR1_OFF;
			MOTOR1_PHASE3_HIGH;
			MOTOR1_PHASE2_LOW;
			motor1_status = 2;
			break;
			}
		case 2:{
			MOTOR1_OFF;
			MOTOR1_PHASE3_HIGH;
			MOTOR1_PHASE1_LOW;
			motor1_status = 3;
			break;
			}
		case 3:{
			MOTOR1_OFF;
			MOTOR1_PHASE2_HIGH;
			MOTOR1_PHASE1_LOW;
			motor1_status = 4;
			break;
			}
		case 4:{
			MOTOR1_OFF;
			MOTOR1_PHASE2_HIGH;
			MOTOR1_PHASE3_LOW;
			motor1_status = 5;
			break;
			}
		case 5:{
			MOTOR1_OFF;
			MOTOR1_PHASE1_HIGH;
			MOTOR1_PHASE3_LOW;
			motor1_status = 0;
			break;
			}
		}
	}else{
		switch(motor1_status){
		case 1:{
			MOTOR1_OFF;
			MOTOR1_PHASE1_HIGH;
			MOTOR1_PHASE3_LOW;
			motor1_status = 0;
			break;
			}
		case 0: {
			MOTOR1_OFF;
			MOTOR1_PHASE2_HIGH;
			MOTOR1_PHASE3_LOW;
			motor1_status = 5;
			break;
			}
		case 5:{
			MOTOR1_OFF;
			MOTOR1_PHASE2_HIGH;
			MOTOR1_PHASE1_LOW;
			motor1_status = 4;
			break;
			}
		case 4:{
			MOTOR1_OFF;
			motor1_status = 3;
			MOTOR1_PHASE3_HIGH;
			MOTOR1_PHASE1_LOW;
			break;
			}
		case 3:{
			MOTOR1_OFF;
			motor1_status = 2;
			MOTOR1_PHASE3_HIGH;
			MOTOR1_PHASE2_LOW;
			break;
			}
		case 2:{
			MOTOR1_OFF;
			motor1_status = 1;
			MOTOR1_PHASE1_HIGH;
			MOTOR1_PHASE2_LOW;
			break;
			}
		}
	}
}


// makes motor 2 turn one step forward/backward
void motor2_turn(bool forward){
	if(forward){
		switch(motor2_status){
		case 0:{
			MOTOR2_OFF;
			MOTOR2_PHASE1_HIGH;
			MOTOR2_PHASE2_LOW;
			motor2_status = 1;
			break;
			}
		case 1:{
			MOTOR2_OFF;
			MOTOR2_PHASE3_HIGH;
			MOTOR2_PHASE2_LOW;
			motor2_status = 2;
			break;
			}
		case 2:{
			MOTOR2_OFF;
			MOTOR2_PHASE3_HIGH;
			MOTOR2_PHASE1_LOW;
			motor2_status = 3;
			break;
			}
		case 3:{
			MOTOR2_OFF;
			MOTOR2_PHASE2_HIGH;
			MOTOR2_PHASE1_LOW;
			motor2_status = 4;
			break;
			}
		case 4:{
			MOTOR2_OFF;
			MOTOR2_PHASE2_HIGH;
			MOTOR2_PHASE3_LOW;
			motor2_status = 5;
			break;
			}
		case 5:{
			MOTOR2_OFF;
			MOTOR2_PHASE1_HIGH;
			MOTOR2_PHASE3_LOW;
			motor2_status = 0;
			break;
			}
		}
	}else{
		switch(motor2_status){
		case 1:{
			MOTOR2_OFF;
			MOTOR2_PHASE1_HIGH;
			MOTOR2_PHASE3_LOW;
			motor2_status = 0;
			break;
			}
		case 0: {
			MOTOR2_OFF;
			MOTOR2_PHASE2_HIGH;
			MOTOR2_PHASE3_LOW;
			motor2_status = 5;
			break;
			}
		case 5:{
			MOTOR2_OFF;
			MOTOR2_PHASE2_HIGH;
			MOTOR2_PHASE1_LOW;
			motor2_status = 4;
			break;
			}
		case 4:{
			MOTOR2_OFF;
			motor2_status = 3;
			MOTOR2_PHASE3_HIGH;
			MOTOR2_PHASE1_LOW;
			break;
			}
		case 3:{
			MOTOR2_OFF;
			motor2_status = 2;
			MOTOR2_PHASE3_HIGH;
			MOTOR2_PHASE2_LOW;
			break;
			}
		case 2:{
			MOTOR2_OFF;
			motor2_status = 1;
			MOTOR2_PHASE1_HIGH;
			MOTOR2_PHASE2_LOW;
			break;
			}
		}
	}
}

