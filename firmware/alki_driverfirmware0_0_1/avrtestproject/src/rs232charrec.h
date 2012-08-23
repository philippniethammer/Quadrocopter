/*
 * rs232charrec.h
 *
 *  Created on: 12.06.2012
 *      Author: alki
 */

#ifndef RS232CHARREC_H_
#define RS232CHARREC_H_

//==========================================================
//
// wartet auf ein char von U(S)ART und gibt es zur�ck
//
// Ben�tigt: initusart.c
//
// Bsp:
// #include "subs/initusart.c"
//

#include "avr/io.h"


unsigned char rs232charrec(void)
{
  #ifdef UCSRA                                     // wenn USART-Register vorhanden
	while ( !(UCSRA & (1<<RXC)) ){asm volatile("nop");}              // warten bis Zeichen empfangen

  #else                                            // wenn nur UART-Register vorhanden
	//while( ! ( USR & (1 << RXC))){asm volatile("nop");}              // dito
  #endif

  return UDR;                                      // Zeichen zur�ckgeben
}


#endif /* RS232CHARREC_H_ */
