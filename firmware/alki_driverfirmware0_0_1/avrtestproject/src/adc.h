/*
 * adc.h
 *
 *  Created on: 13.06.2012
 *      Author: alki
 */
#ifndef ADC_H_
#define ADC_H_


#include "avr/io.h"


void initADC(void){
	// enable ADC
	// and set Prescaler to 64 (125kHz sampling rate. (32 => 250kHz))
	ADCSRA |= (( 1 << ADEN) | (1 << ADPS0) | (1 << ADPS2));

	// clear ADC-result
	ADCSRA |= (1<<ADSC);
	while (ADCSRA & (1<<ADSC) ) { }
	uint16_t ADCwaste = ADC;
}






#endif
