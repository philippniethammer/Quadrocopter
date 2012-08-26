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

// PWM-concerning
// maximum off-clocks: 256
// actual off-clocks: 256 - motorX_ratio
// minimum on-clocks: 256 - MOTORMINPWMFACTOR
// actual on-clocks: motorX_ratio+MOTORMINPWMFACTOR
#define MOTORMINPWMFACTOR 256 - 100

#define MOTORCOUNTERMAX 2000
#define MOTORCOUNTERMIN 200
#define MOTORCOUNTERSTEP 10

//================ initializing variables ===================

static bool motor2_adcIgnore = false;

static uint16_t motor2_maxadc = 0;
static uint16_t motor2_minadc = UINT16_MAX;

static uint16_t motor2_switch = 300;
static uint8_t motor2_triggerComm = 0;

// states of the motors (0 to 2 [or 5])
static uint8_t motor2_status = 0;

// direction: true => forward, false => backward
static bool motor2_forward = true;

// variables for driving motors blindly
static uint16_t motor2_counter = 0;
static uint16_t motor2_counter_max = MOTORCOUNTERMAX;
static uint16_t motor2_startcounter = 0;

// ratio of motorpower
uint8_t motor2_ratio = 0;

// fast enough to use ADC?
uint8_t motor2_started = 0;

//variables for motor-pwm and adc
static uint8_t motor2_pwm_status = 0;// 0 => actually off, 1 => actually on

// for SWITCHFUNCTION 1
static uint16_t motor2_common_adc = 0;
static uint16_t motor2_phase_adc = 0;

// for SWITCHFUNCTION 2
static uint8_t motor2_lastadcstatus = 0;// 0 => no old data available, 1 => phase > common, 2=> phase <= common

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
	TCCR0 |= (1 << CS01); // start timer0 by setting prescaler, prescaler = 8
	TCNT0 = 0;

	// TIMER2
	// FIXME Used for SUART
	//TCCR2 |= (1 << CS01); // start timer2 by setting prescaler, prescaler = 8
	//TCNT2 = 0;

	// initialize timer1 for commutation-computing
	TCCR1B |= (1 << CS11); // prescaler = 8

	// start timers
	// TODO TIMSK = TIMSK | (1 << TOIE0) | (1 << TOIE2); // Timer Overflow Interrupt Enable of timer 0 + 2

}

// ======= subroutines =====================


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


void motor2_setratio(uint8_t ratio) {
	motor2_ratio = ratio;
	if (ratio == 0) {
		motor2_started = false;
		motor2_counter_max = MOTORCOUNTERMAX;
		motor2_lastadcstatus = 0;
		motor2_pwm_status = 0;
	}
}

uint8_t getTimeDelta(uint8_t ago, uint8_t now) {
	if (now >= ago) {
		return now - ago;
	} else {
		return UINT8_MAX - ago + now;
	}
}

void motor2_switchADCChannel(uint8_t status) {
	while (status > 5) {
		status -= 6;
	}
	switch (status) {
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
}


void motor2_startup() {
	if (motor2_counter > motor2_counter_max) {
		motor2_counter_max = motor2_counter_max - MOTORCOUNTERSTEP;
		motor2_lastswitchtime = TCNT1;
		motor2_turn();
		motor2_switchADCChannel(motor2_status);
		motor2_adcIgnore = true;
		if (motor2_counter < MOTORCOUNTERMIN) {
			if (motor2_startcounter < 1000) {
				motor2_counter_max += MOTORCOUNTERSTEP;
				motor2_startcounter++;
			} else {
				motor2_startcounter = 0;
				motor2_switch = (motor2_maxadc+motor2_minadc)/2;
				motor2_started = 1;
				motor2_counter_max = MOTORCOUNTERMAX;
			}
		}
		motor2_counter = 0;
	}

	motor2_counter++;
}

// ====================== INTERUPT ROUTINES ================================

ISR( ADC_vect )
{

	if (motor2_triggerComm || motor2_adcIgnore) {
		uint16_t tmp = ADC;
		motor2_adcIgnore = false;
		return;
	}

	// lern mode
	if (!motor2_started) {
		uint16_t tmp = ADC;

		if (tmp > motor2_maxadc) {
			motor2_maxadc = tmp;
		}
		if (tmp < motor2_minadc) {
			motor2_minadc = tmp;
		}

	} else {

		// save result
		motor2_phase_adc = ADC;

		switch (motor2_lastadcstatus) {
		case 0:
			if (motor2_phase_adc > motor2_switch) {

				motor2_lastadcstatus = 1;
			} else {
				motor2_lastadcstatus = 2;
			}
			break;
		case 1:
			if (motor2_phase_adc <= motor2_switch) {

				motor2_zerotime = TCNT1;
				motor2_switchtime = motor2_zerotime + getTimeDelta(motor2_lastswitchtime, motor2_zerotime);
				if (motor2_zerotime < motor2_switchtime) {
					motor2_triggerComm = 1;
				} else {
					motor2_triggerComm = 2;
				}
				motor2_switchADCChannel(motor2_status+1);
				motor2_lastadcstatus = 0;
			}
			break;
		case 2:
			if (motor2_phase_adc >= motor2_switch) {

				motor2_zerotime = TCNT1;
				motor2_switchtime = motor2_zerotime + getTimeDelta(motor2_lastswitchtime, motor2_zerotime);
				if (motor2_zerotime < motor2_switchtime) {
					motor2_triggerComm = 1;
				} else {
					motor2_triggerComm = 2;
				}
				motor2_switchADCChannel(motor2_status+1);
				motor2_lastadcstatus = 0;
			}
			break;
		}
	}
}

// Timer 0 overflow (for motor1 pwm)
/*
ISR( TIMER0_OVF_vect )
{
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
*/

// Timer 2 overflow (for motor2 pwm
// FIXME Used for SUART
/**
ISR( TIMER2_OVF_vect )
{
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
*/

// ====================== MAIN ROUTINE ================================

int main(void) {

	initializeMotors(); // for safety reasons first!
	initADC();

#if TESTMODE == TRUE
	//motor1_ratio = 200; // motor1 is the one with a jumper
	motor2_ratio = 10;
	//motor2_forward = false;

#endif

	// finally turn on interrupts
	sei();

	while (true) {
		if (motor2_ratio > 0 && motor2_started) {
			if ((motor2_triggerComm == 1 || (motor2_triggerComm == 2 && TCNT1 < motor2_zerotime))
					&& TCNT1 >= motor2_switchtime) {
				motor2_lastswitchtime = TCNT1;
				motor2_turn();
				motor2_triggerComm = 0;
			}
		} else if (motor2_ratio > 0) { // start up.

			motor2_startup();

		}
	}

	// just for the compiler, so "int main(void)" 'returns' some value
	return 0;
}
