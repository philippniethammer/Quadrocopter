/*
 * motors.h
 *
 *  Created on: 08.04.2011
 *      Author: alki
 *
 *      this contains all needed procedures for driving the motors
 *      initializeMotors() needs to be excecuted once to make the other
 *      functions working
 *
 *
 *  	Motors:
 *  	Left:	PB3 (OC0)
 *  	Right:	PD4 (OC1B)
 *  	Front:	PD5 (OC1A)
 *  	Back:	PD7 (OC2)
 */

#ifndef MOTORS_H_
#define MOTORS_H_
/*
 * Initializes the counters, the motor values (= 0) and starts the counters
 * result: PWM with ON-time = 0%
 */
void initializeMotors(void);

/*
 * Set left motor.
 */
void setMotorLeft(uint8_t speed);

/*
 * Set right motor.
 */
void setMotorRight(uint8_t speed);

/*
 * Set front motor.
 */
void setMotorFront(uint8_t speed);

/*
 * Set back motor.
 */
void setMotorBack(uint8_t speed);

/*
 * Get left motor value.
 */
uint8_t getMotorvalueLeft(void);

/*
 * Get right motor value.
 */
uint8_t getMotorvalueRight(void);

/*
 * Get front motor value.
 */
uint8_t getMotorvalueFront(void);

/*
 * Get back motor value.
 */
uint8_t getMotorvalueBack(void);

#endif
