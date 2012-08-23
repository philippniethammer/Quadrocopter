/*
 * rs232charsend.h
 *
 *  Created on: 12.06.2012
 *      Author: alki
 */

#ifndef RS232CHARSEND_H_
#define RS232CHARSEND_H_

//===========================
//
// sendet ein char per RS232
//
// Ben�tigt: usartinit.c
//
// Bsp:
// #include "subs/initusart.c"
//
#include "avr/io.h"


void rs232charsend(unsigned char x)
{
  #ifdef UCSRA                                    // wenn USART-Register vorhanden
    while( ! (UCSRA & (1 << UDRE))){asm volatile("nop");};             // solange Sender noch arbeitet warte

  #else
    while(! USR & ( 1 << UDRE)){asm volatile("nop");}                // dasselbe wenn nur UART-Register vorhanden
  #endif

  UDR = x;                                         // char zum Senden in Senderegister schreiben
}



#endif /* RS232CHARSEND_H_ */
