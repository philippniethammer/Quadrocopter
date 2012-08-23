/*
 * driver_test1.c
 *
 *  Created on: 17.02.2012
 *      Author: alki
 *
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
#include "avr/signal.h"
#include "avr/interrupt.h"

#include "driver.h"
#include "adc.h"

// =================== defining global static variables ===================

#define TESTMODE TRUE
#define SWITCHFUNCTION 2 // 1 => original " difference < X", 2 => switch greater <=> smaller
// PWM-concerning
// maximum off-clocks: 256
// actual off-clocks: 256 - motorX_ratio
// minimum on-clocks: 256 - MOTORMINPWMFACTOR
// actual on-clocks: motorX_ratio+MOTORMINPWMFACTOR
#define MOTORMINPWMFACTOR 256 - 100

#define MOTORCOUNTERMAX 1000 //65500
#define MOTORCOUNTERMIN 200
#define MOTORCOUNTERSTEP 50

#define MOTOROFFDELAYSTEPS 25 // steps to wait for the FETs to turn off
// max. difference of phase-voltage to common-voltage for triggering a commutation
#define MOTORADCDIFF 15

//================ initializing variables ===================

// states of the motors (0 to 2 [or 5])
static uint8_t motor1_status = 0;
static uint8_t motor2_status = 0;

// direction: true => forward, false => backward
static bool motor1_forward = true;
static bool motor2_forward = true;

// variables for driving motors blindly
static uint16_t motor1_counter = 0;
static uint16_t motor1_counter_max = MOTORCOUNTERMAX;
static uint16_t motor2_counter = 0;
static uint16_t motor2_counter_max = MOTORCOUNTERMAX;

// ratio of motorpower
uint8_t motor1_ratio = 0;
uint8_t motor2_ratio = 0;

// fast enough to use ADC?
uint8_t motor1_started = 0;
uint8_t motor2_started = 0;

//variables for motor-pwm and adc
static uint8_t motor1_pwm_status = 0; // 0 => actually off, 1 => actually on
static uint8_t motor2_pwm_status = 0;

// for SWITCHFUNCTION 1
static uint16_t motor1_common_adc = 0;
static uint16_t motor1_phase_adc = 0;

static uint16_t motor2_common_adc = 0;
static uint16_t motor2_phase_adc = 0;

// for SWITCHFUNCTION 2
static uint8_t motor1_lastadcstatus = 0; // 0 => no old data available, 1 => phase > common, 2=> phase <= common
static uint8_t motor2_lastadcstatus = 0;

static uint16_t motor1_switchtime; // next time the motor has to be switched (counter 1 value)
static uint16_t motor1_lastswitchtime; // last time the motor has to be switched (counter 1 value)
static uint16_t motor1_lastswitchdelay; // clocks between last commutation and the commutation before
static uint16_t motor1_zerotime;

static uint16_t motor2_switchtime;
static uint16_t motor2_lastswitchtime;
static uint16_t motor2_lastswitchdelay;
static uint16_t motor2_zerotime;

// ======= initialization routines =========

void initializeMotors() {
	// set all pins driving the motors as output:
	DDRB |= ((1 << PB0) | (1 << PB1) | (1 << PB2) | (1 << PB3) | (1 << PB6)
			| (1 << PB7));
	DDRD |= ((1 << PD0) | (1 << PD1) | (1 << PD4) | (1 << PD5) | (1 << PD6)
			| (1 << PD7));
	// set all pins driving the motors to 0/GND: (just for sure)
	PORTB &= ~((1 << PB0) | (1 << PB1) | (1 << PB2) | (1 << PB3) | (1 << PB6)
			| (1 << PB7));
	PORTD &= ~((1 << PD0) | (1 << PD1) | (1 << PD4) | (1 << PD5) | (1 << PD6)
			| (1 << PD7));

	// initialize interrupts used for motor-PWM

	// TIMER0
	// TODO TCCR0 |= (1 << CS01); // start timer0 by setting prescaler, prescaler = 8
	TCNT0 = 0;

	// TIMER2
	// TODO TCCR2 |= (1 << CS01); // start timer2 by setting prescaler, prescaler = 8
	TCNT2 = 0;


	// initialize timer1 for commutation-computing
	TCCR1B |= (1 << CS11); // prescaler = 8

	// start timers
	// TODO TIMSK = TIMSK | (1 << TOIE0) | (1 << TOIE2); // Timer Overflow Interrupt Enable of timer 0 + 2

}

// ======= subroutines =====================


// turns back on the motor connections to V_Motor (position specific) of motor 1
void motor1_positive_on(void) {
	switch (motor1_status) {
	case 1: {
		MOTOR1_PHASE1_HIGH;
		break;
	}
	case 2: {
		MOTOR1_PHASE3_HIGH;
		break;
	}
	case 3: {
		MOTOR1_PHASE3_HIGH;
		break;
	}
	case 4: {
		MOTOR1_PHASE2_HIGH;
		break;
	}
	case 5: {
		MOTOR1_PHASE2_HIGH;
		break;
	}
	case 0: {
		MOTOR1_PHASE1_HIGH;
		break;
	}
	}
}

// turns back on the motor connections to V_Motor (position specific) of motor 2
void motor2_positive_on(void) {
	switch (motor2_status) {
	case 1: {
		MOTOR2_PHASE1_HIGH;
		break;
	}
	case 2: {
		MOTOR2_PHASE3_HIGH;
		break;
	}
	case 3: {
		MOTOR2_PHASE3_HIGH;
		break;
	}
	case 4: {
		MOTOR2_PHASE2_HIGH;
		break;
	}
	case 5: {
		MOTOR2_PHASE2_HIGH;
		break;
	}
	case 0: {
		MOTOR2_PHASE1_HIGH;
		break;
	}
	}
}

// makes motor 1 turn one step forward/backward
void motor1_turn(void) {
	// turn off interrupts
	// TODO cli();
	// turn off all motor connections
	MOTOR1_OFF;
	/*for (uint8_t i = 0; i < MOTOROFFDELAYSTEPS; i++) {
		asm volatile ("nop");
	}*/
	if (motor1_forward) {
		switch (motor1_status) {
		case 0: {
			MOTOR1_PHASE1_HIGH;
			MOTOR1_PHASE2_LOW;
			motor1_status = 1;
			break;
		}
		case 1: {
			MOTOR1_PHASE3_HIGH;
			MOTOR1_PHASE2_LOW;
			motor1_status = 2;
			break;
		}
		case 2: {
			MOTOR1_PHASE3_HIGH;
			MOTOR1_PHASE1_LOW;
			motor1_status = 3;
			break;
		}
		case 3: {
			MOTOR1_PHASE2_HIGH;
			MOTOR1_PHASE1_LOW;
			motor1_status = 4;
			break;
		}
		case 4: {
			MOTOR1_PHASE2_HIGH;
			MOTOR1_PHASE3_LOW;
			motor1_status = 5;
			break;
		}
		case 5: {
			MOTOR1_PHASE1_HIGH;
			MOTOR1_PHASE3_LOW;
			motor1_status = 0;
			break;
		}
		}
	} else {
		switch (motor1_status) {
		case 1: {
			MOTOR1_PHASE1_HIGH;
			MOTOR1_PHASE3_LOW;
			motor1_status = 0;
			break;
		}
		case 0: {
			MOTOR1_PHASE2_HIGH;
			MOTOR1_PHASE3_LOW;
			motor1_status = 5;
			break;
		}
		case 5: {
			MOTOR1_PHASE2_HIGH;
			MOTOR1_PHASE1_LOW;
			motor1_status = 4;
			break;
		}
		case 4: {
			motor1_status = 3;
			MOTOR1_PHASE3_HIGH;
			MOTOR1_PHASE1_LOW;
			break;
		}
		case 3: {
			motor1_status = 2;
			MOTOR1_PHASE3_HIGH;
			MOTOR1_PHASE2_LOW;
			break;
		}
		case 2: {
			motor1_status = 1;
			MOTOR1_PHASE1_HIGH;
			MOTOR1_PHASE2_LOW;
			break;
		}
		}
	}
	// turn back on interrupts
	// TODO sei();
}

// makes motor 2 turn one step forward/backward
void motor2_turn(void) {
	// turn off interrupts
	// TODO cli();
	// turn off all motor connections
	MOTOR2_OFF;

	/*for (uint8_t i = 0; i < MOTOROFFDELAYSTEPS; i++) {
		asm volatile ("nop");
	}*/
	if (motor2_forward) {
		switch (motor2_status) {
		case 0: {
			MOTOR2_PHASE1_HIGH;
			MOTOR2_PHASE2_LOW;
			motor2_status = 1;
			break;
		}
		case 1: {
			MOTOR2_PHASE3_HIGH;
			MOTOR2_PHASE2_LOW;
			motor2_status = 2;
			break;
		}
		case 2: {
			MOTOR2_PHASE3_HIGH;
			MOTOR2_PHASE1_LOW;
			motor2_status = 3;
			break;
		}
		case 3: {
			MOTOR2_PHASE2_HIGH;
			MOTOR2_PHASE1_LOW;
			motor2_status = 4;
			break;
		}
		case 4: {
			MOTOR2_PHASE2_HIGH;
			MOTOR2_PHASE3_LOW;
			motor2_status = 5;
			break;
		}
		case 5: {
			MOTOR2_PHASE1_HIGH;
			MOTOR2_PHASE3_LOW;
			motor2_status = 0;
			break;
		}
		}
	} else {
		switch (motor2_status) {
		case 1: {
			MOTOR2_PHASE1_HIGH;
			MOTOR2_PHASE3_LOW;
			motor2_status = 0;
			break;
		}
		case 0: {
			MOTOR2_PHASE2_HIGH;
			MOTOR2_PHASE3_LOW;
			motor2_status = 5;
			break;
		}
		case 5: {
			MOTOR2_PHASE2_HIGH;
			MOTOR2_PHASE1_LOW;
			motor2_status = 4;
			break;
		}
		case 4: {
			motor2_status = 3;
			MOTOR2_PHASE3_HIGH;
			MOTOR2_PHASE1_LOW;
			break;
		}
		case 3: {
			motor2_status = 2;
			MOTOR2_PHASE3_HIGH;
			MOTOR2_PHASE2_LOW;
			break;
		}
		case 2: {
			motor2_status = 1;
			MOTOR2_PHASE1_HIGH;
			MOTOR2_PHASE2_LOW;
			break;
		}
		}
	}
	// turn back on interrupts
	// TODO sei();
}

void motor1_setratio(uint8_t ratio) {
	motor1_ratio = ratio;
	if (ratio == 0) {
		motor1_started = false;
		motor1_counter_max = MOTORCOUNTERMAX;
		motor1_lastadcstatus = 0;
		motor1_pwm_status = 0;
	}
}

void motor2_setratio(uint8_t ratio) {
	motor2_ratio = ratio;
	if (ratio == 0) {
		motor2_started = false;
		motor2_counter_max = MOTORCOUNTERMAX;
		motor2_lastadcstatus = 0;
		motor1_pwm_status = 0;
	}
}

// ====================== INTERUPT ROUTINES ================================

// Timer 0 overflow (for motor1 pwm)
ISR( TIMER0_OVF_vect ) {
	// disable interrupts
	cli();
	if (motor1_ratio > 0) {
		//If PWM was just off
		if (motor1_pwm_status == 0) {
			motor1_positive_on();
			TCNT0 = MOTORMINPWMFACTOR;
		} else if (motor1_pwm_status == 1) {
			motor1_pwm_status = 1;
			// count up #[motorratio] steps
			TCNT0 = 0xff - motor1_ratio;
		} else {
			// turn off all motor connections to positive voltage
			PORTB &= ~((1 << PB0) | (1 << PB1));
			PORTD &= ~(1 << PD6);
			motor1_pwm_status = 0;
			// count up #[256 - motorratio] steps
			TCNT0 = motor1_ratio;
		}
	} else {
		MOTOR1_OFF
		;
	}
	// enable interrupts
	sei();
}

// Timer 2 overflow (for motor2 pwm
ISR( TIMER2_OVF_vect ) {
	// disable interrupts
	cli();
	if (motor2_ratio > 0) {
		//If PWM was just off
		if (motor2_pwm_status == 0) {
			motor2_positive_on();
			TCNT2 = MOTORMINPWMFACTOR;
		} else if (motor2_pwm_status == 1) {
			motor2_positive_on();
			motor2_pwm_status = 1;
			// count up #[motorratio] steps
			TCNT2 = 0xff - motor2_ratio;
		} else {
			// turn off all motor connections to positive voltage
			PORTB &= ~((1 << PB7));
			PORTD &= ~((1 << PD1) | (1 << PD4));
			motor2_pwm_status = 0;
			// count up #[256 - motorratio] steps
			TCNT2 = motor2_ratio;
		}
	} else {
		// turn off motor
		MOTOR2_OFF
		;
	}
	// enable interrupts
	sei();
}

// ====================== MAIN ROUTINE ================================

int main(void) {

	initializeMotors(); // for safety reasons first!
	//initADC();

#if TESTMODE == TRUE
	//motor1_ratio = 200; // motor1 is the one with a jumper
	motor2_ratio = 10;
	//motor2_forward = false;

	// TODO
	while (true) {
	 for (uint16_t i = 0; i < 6550; i++) {
		 for (uint16_t i = 0; i < 5; i++) {
		 	 	 asm volatile ("nop");
		 	 }
	 }
	 motor2_turn();
	}
#endif

	// finally turn on interrupts
	sei();
	while (1) {

		// work on motor1
		if ((motor1_ratio > 0) && (motor1_started)) {

			ADMUX = 6; // ADC6
			ADCSR |= ADSC; // start converting motor1 common
			while (ADCSR & (ADSC)) {
				asm volatile ("nop");
			}
			// conversion complete, start next conversion and save last result
			motor1_common_adc = ADC;
			switch (motor1_status) {
			case 0:
				// phase 2 (ADC0)
				ADMUX = 0;
				break;
			case 1:
				// phase 3 (ADC7)
				ADMUX = 7;
				break;
			case 2:
				// phase 1 (ADC1)
				ADMUX = 1;
				break;
			case 3:
				// phase 2 (ADC0)
				ADMUX = 0;
				break;
			case 4:
				// phase 3 (ADC7)
				ADMUX = 7;
				break;
			case 5:
				// phase 1 (ADC1)
				ADMUX = 1;
				break;
			}
			ADCSR |= ADSC;
			// wait until conversion completes
			while (ADCSR & (ADSC)) {
				asm volatile ("nop");
			}
			motor1_counter++;

			// conversion complete, save result
			motor1_phase_adc = ADC;

			// TODO: compare results and turn motor1 if ready

#if SWITCHFUNCTION == 1
			// prototype 1:
			int phasemaxvalue = 0;
			int phaseminvalue = 0;

			phasemaxvalue = motor1_phase_adc + MOTORADCDIFF;

			// handle potential 'underflow'
			if ((motor1_common_adc - MOTORADCDIFF) > motor1_common_adc) {
				phaseminvalue = 0;
			} else {
				phaseminvalue = motor1_common_adc - MOTORADCDIFF;
			}

			if ((motor1_phase_adc < phasemaxvalue)
					&& (motor1_phase_adc > phasemaxvalue)) {
				// comutate motor1 if ready
				motor1_turn();
			}

#elif SWITCHFUNCTION == 2
			// prototype 2:
			switch (motor1_lastadcstatus) {
			case 0:
				if (motor1_phase_adc > motor1_common_adc) {
					motor1_lastadcstatus = 1;
				} else {
					motor1_lastadcstatus = 2;
				}
				break;
			case 1:
				if (motor1_phase_adc <= motor1_common_adc) {
					motor1_zerotime = TCNT1;
					motor1_switchtime = motor1_zerotime
							+ (motor1_zerotime - motor1_lastswitchtime); // zerotime + time taken to zero-crossing

					if (motor1_switchtime < motor1_zerotime) { // overflow happened
						motor1_lastadcstatus = 3;
					} else {
						motor1_lastadcstatus = 4;
					}
				}
				break;
			case 2:
				if (motor1_phase_adc > motor1_common_adc) {
					motor1_zerotime = TCNT1;
					motor1_switchtime = motor1_zerotime
							+ (motor1_zerotime - motor1_lastswitchtime); // zerotime + time taken to zero-crossing
					if (motor1_switchtime < motor1_zerotime) { // overflow happened
						motor1_lastadcstatus = 3;
					} else {
						motor1_lastadcstatus = 4;
					}
				}
				break;
			case 3:
				if ((TCNT1 > motor1_switchtime) && (TCNT1 < motor1_zerotime)) {
					motor1_turn();
					motor1_lastswitchtime = TCNT1;
					motor1_lastadcstatus = 0;
				}
				break;
			case 4:
				if(TCNT1 > motor1_switchtime){
					motor1_turn();
					motor1_lastswitchtime = TCNT1;
					motor1_lastadcstatus = 0;
				}
				break;
			default:
				break;
			}

#endif
			// if motor1 not yet fast enough: drive it blindly
		} else if (motor1_ratio > 0) {

			if (motor1_counter > motor1_counter_max) {
				motor1_counter_max = motor1_counter_max - MOTORCOUNTERSTEP;
				motor1_counter = 0;
				motor1_turn();
				if (motor1_counter < MOTORCOUNTERMIN) {
					// get actual value of timer/counter1
					motor1_switchtime = TCNT1;
					motor1_counter_max = MOTORCOUNTERMAX;
					motor1_started = 1;
				}
			}

			motor1_counter++;
		}


		// work on motor2 =======================================
		if (motor2_ratio > 0) {

			ADMUX = 2; // ADC2
			ADCSR |= ADSC; // start converting motor2 common
			while (ADCSR & (ADSC)) {
				asm volatile ("nop");
			}
			// conversion complete, start next conversion and save last result
			motor2_common_adc = ADC;
			switch (motor2_status) {
			case 0:
				// phase 2 (ADC4)
				ADMUX = 4;
				break;
			case 1:
				// phase 3 (ADC3)
				ADMUX = 3;
				break;
			case 2:
				// phase 1 (ADC5)
				ADMUX = 5;
				break;
			case 3:
				// phase 2 (ADC4)
				ADMUX = 4;
				break;
			case 4:
				// phase 3 (ADC3)
				ADMUX = 3;
				break;
			case 5:
				// phase 1 (ADC5)
				ADMUX = 5;
				break;
			}
			ADCSR |= ADSC; // start conversion
			// wait until conversion completes
			while (ADCSR & (ADSC)) {
				asm volatile ("nop");
			}

			// conversion complete, save result
			motor2_phase_adc = ADC;

			// TODO: compare results and turn motor2 if ready

#if SWITCHFUNCTION == 1

			// prototype:
			int phasemaxvalue = 0;
			int phaseminvalue = 0;

			// no overflow possible (ADC-values are 10bit, not 16bit)
			phasemaxvalue = motor2_phase_adc + MOTORADCDIFF;

			// handle potential 'underflow'
			if ((motor2_common_adc - MOTORADCDIFF) > motor2_common_adc) {
				phaseminvalue = 0;
			} else {
				phaseminvalue = motor2_common_adc - MOTORADCDIFF;
			}

			if ((motor2_phase_adc < phasemaxvalue)
					&& (motor2_phase_adc > phasemaxvalue)) {
				// commutate motor2 if ready
				motor2_turn();
			}

#elif SWITCHFUNCTION == 2

			// prototype 2:
			switch (motor2_lastadcstatus) {
			case 0:
							if (motor2_phase_adc > motor2_common_adc) {
								motor2_lastadcstatus = 1;
							} else {
								motor2_lastadcstatus = 2;
							}
							break;
						case 1:
							if (motor2_phase_adc <= motor2_common_adc) {
								motor2_zerotime = TCNT1;
								motor2_lastswitchtime = motor2_switchtime;
								motor2_switchtime = motor2_zerotime
										+ (motor2_zerotime - motor2_lastswitchtime); // zerotime + time taken to zero-crossin
								if (motor2_switchtime < motor2_zerotime) { // overflow happened
									motor2_lastadcstatus = 3;
								} else {
									motor2_lastadcstatus = 4;
								}
							}
							break;
						case 2:
							if (motor2_phase_adc > motor2_common_adc) {
								motor2_zerotime = TCNT1;
								motor2_lastswitchtime = motor2_switchtime;
								motor2_switchtime = motor2_zerotime
										+ (motor2_zerotime - motor2_lastswitchtime); // zerotime + time taken to zero-crossing
								if (motor2_switchtime < motor2_zerotime) { // overflow happened
									motor2_lastadcstatus = 3;
								} else {
									motor2_lastadcstatus = 4;
								}
							}
							break;
						case 3:
							if ((TCNT1 > motor2_switchtime) && (TCNT1 < motor2_zerotime)) {
								motor2_turn();
								motor2_lastswitchtime = TCNT1;
								motor2_lastadcstatus = 0;
							}
							break;
						case 4:
							if(TCNT1 > motor2_switchtime){
								motor2_turn();
								motor2_lastswitchtime = TCNT1;
								motor2_lastadcstatus = 0;
							}
							break;
						default:
							break;
						}

#endif
			// if motor2 not yet fast enough: drive it blindly
		} else if (motor2_ratio > 0) {

			if (motor2_counter > motor2_counter_max) {
				motor2_counter_max = motor2_counter_max - MOTORCOUNTERSTEP;
				motor2_counter = 0;
				motor2_turn();
				if (motor2_counter < MOTORCOUNTERMIN) {
					motor2_started = 1;
					motor2_counter_max = MOTORCOUNTERMAX;
				}
			}

			motor2_counter++;

		}
	}

// just for the compiler, so "int main(void)" 'returns' some value
	return 0;
}
