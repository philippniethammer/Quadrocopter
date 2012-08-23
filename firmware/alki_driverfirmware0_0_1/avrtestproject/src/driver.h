/*
 * driver.h
 *
 *  Created on: 21.03.2012
 *      Author: alki
 */

#ifndef DRIVER_H_
#define DRIVER_H_

#define MOTOR1_OFF PORTB &= ~((1 << PB0) | (1 << PB1) | (1 << PB2) | (1 << PB3)); PORTD &= ~((1 << PD6) | (1 << PD7))
#define MOTOR2_OFF PORTB &= ~((1 << PB6) | (1 << PB7)); PORTD &= ~((1 << PD0) | (1 << PD1) | (1 << PD4) | (1 << PD5))

#define MOTOR1_PHASE1_LOW	PORTD |= (1 << PD7)
#define MOTOR1_PHASE1_HIGH	PORTD |= (1 << PD6)
#define MOTOR1_PHASE2_LOW	PORTB |= (1 << PB2)
#define MOTOR1_PHASE2_HIGH	PORTB |= (1 << PB0)
#define MOTOR1_PHASE3_LOW	PORTB |= (1 << PB3)
#define MOTOR1_PHASE3_HIGH	PORTB |= (1 << PB1)

#define MOTOR2_PHASE1_LOW	PORTD |= (1 << PD0)
#define MOTOR2_PHASE1_HIGH	PORTD |= (1 << PD1)
#define MOTOR2_PHASE2_LOW	PORTB |= (1 << PB6)
#define MOTOR2_PHASE2_HIGH	PORTD |= (1 << PD4)
#define MOTOR2_PHASE3_LOW	PORTD |= (1 << PD5)
#define MOTOR2_PHASE3_HIGH	PORTB |= (1 << PB7)


#endif /* DRIVER_H_ */
