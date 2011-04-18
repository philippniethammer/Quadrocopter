/*
 * motors.c
 *
 *  Created on: 18.04.2011
 *      Author: philipp
 */

#include <avr/io.h>
#include <stdint.h>
#include "motors.h"

/*
 * initializes the counters, the motor values (= 0) and starts the counters
 * result: PWM with ON-time = 0%
 *
 */
void initializeMotors(void){

	// setting Pins as output
	DDRB |= (1 << PB3);
	DDRD |= (1 << PD4) | (1 << PD5) | (1 << PD7);



	// set initial value of output compare = 0
	OCR0 = 0xff; // since inverted
	OCR1AL = 0;
	OCR1BL = 0;
	OCR2 = 0xff; // since inverted


	/*
	 * set counter 0
	 * set PWM-mode + prescaler (and start counter by that)
	 * (COM01:0 = 2 => Set OC0 on Compare Match when
	 * up-counting. Clear OC0 on Compare Match when down-counting.)
	 * (WGM01:0 = 1 => PWM)
	 * counter/timer0 and counter/timer 2 are inverted
	 * prescaler 256:
	 * (CS02:1:0 = 5, CS02 = 1, CS00 = 1)
	 */
	TCCR0 |= ((1 << COM00) | (1 << COM01) | (1 << WGM00) | (1 << CS02));


	/*
	 * set  counter 1
	 * same as counter 0 but extra definition as 8 bit
	 * and non-inverted (setting when downcounting, clearing when upcounting)
	 */
	TCCR1A |= ((1 << COM1A1) | (1 << COM1B1) | (1 << WGM10));
	TCCR1B |= (1 << CS12);



	/*
	 * set counter 2 just like counter 0
	 */
	TCCR2 |= ((1 << COM20) | (1 << COM21) | (1 << WGM20) | (1 << CS22));

}

/*
 * Left Motor on PB3 (OC0)
 * inverting value and setting OCR0 to it
 */
void setMotorLeft(uint8_t speed){
	OCR0 = 0xff - speed;
}

/*
 * Right Motor on PD4 (OC1B)
 * setting OCR1BL to it
 */
void setMotorRight(uint8_t speed){
	OCR1BL = speed;
}

/*
 * Front Motor on PD5 (OC1A)
 * setting OCR1AL to it
 */
void setMotorFront(uint8_t speed){
	OCR1AL = speed;
}

/*
 * Back Motor on PD7 (OC2)
 * inverting value and setting OCR2 to it
 */
void setMotorBack(uint8_t speed){
	OCR2 = 0xff - speed;
}

/*
 * returning inverted value of OCR0;
 */
uint8_t getMotorvalueLeft(void){

	return 0xff - OCR0;
}

/*
 * returning value of OCR1B low-byte
 */
uint8_t getMotorvalueRight(void){

	return OCR1BL;
}

/*
 * returning value of OCR1A low-byte
 */
uint8_t getMotorvalueFront(void){

	return OCR1AL;
}

/*
 * returning inverted value of OCR2;
 */
uint8_t getMotorvalueBack(void){

	return 0xff - OCR2;
}
