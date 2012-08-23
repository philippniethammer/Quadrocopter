/*
 * initusart.h
 *
 *  Created on: 12.06.2012
 *      Author: alki
 */

#ifndef INITUSART_H_
#define INITUSART_H_

//==============
//
// initusart.c
//
// aktiviert U(S)ART
//
//
// Ben�tigt:   MC-Taktrate "TAKT" und gew�nschte Baudrate "BAUD"
//
// ACHTUNG: "TAKT" UND "BAUD" MÜSSEN DEFINIERT WERDEN !VOR! "#include <subs/initusart.c>"
//
// Bsp:
//

// sagt zwar "redefinition of ..." aber ohne ist es prötzlich unbekannt - dafuq?!?!?
#define TAKT 8000000
#define BAUD 9600
//
// #include <subs/initusart.c>
//
//
// auch benötigt:
//
#include <avr/io.h>




void initusart(void)
{
  unsigned char x;                                  // Hilfsvar um sp�ter UDR zu leeren

  #ifndef UBRRL                                     // wenn nur UART-Register vorhanden
    UBRR = ( TAKT / ( 16UL * BAUD )) -1;           // Teiler für Baudrate bei Taktrate
	UCR |=  (1 << TXEN) | (1 << RXEN);            // TX und RX aktiviert

  #else                                             // wenn USART-Register vorhanden
    UBRRL = ( TAKT / ( 16UL * BAUD )) -1;          // Teiler f�r Baudrate bei Taktrate
	UCSRB |= (1 << TXEN) | (1 << RXEN);           // TX und RX aktiviert
	UCSRC |= (1 << URSEL) | (1 << UCSZ1) | (1 << UCSZ0);    // asyncron und 8bit-�bertragung
  #endif

  x = UDR;                                         // Empfangspuffer geleert

}




#endif /* INITUSART_H_ */
