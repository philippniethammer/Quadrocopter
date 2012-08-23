/*
 * driver.h
 *
 *  Created on: 21.03.2012
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

#ifndef DRIVER_H_
#define DRIVER_H_

#define MOTOR1_OFF PORTB &= ~((1 << PB0) | (1 << PB1) | (1 << PB2) | (1 << PB3)); PORTD &= ~((1 << PD6) | (1 << PD7)); motor1_tempoff = true
#define MOTOR2_OFF PORTB &= ~((1 << PB6) | (1 << PB7)); PORTD &= ~((1 << PD0) | (1 << PD1) | (1 << PD4) | (1 << PD5)); motor2_tempoff = true

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
