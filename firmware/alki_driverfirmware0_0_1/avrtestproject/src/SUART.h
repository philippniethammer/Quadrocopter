/*
 * SUART.h
 *
 *  Created on: 26.08.2012
 *      Author: philipp
 */

#ifndef SUART_H_
#define SUART_H_

#define BR_9600     //!< Desired baudrate...choose one, comment the others.
//#define BR_19200    //!< Desired baudrate...choose one, comment the others.
//#define BR_38400      //!< Desired baudrate...choose one, comment the others.

static volatile unsigned char SwUartTXData;     //!< Data to be transmitted.

static void SUART_init( void );
void suart_print( const unsigned char *);

#endif /* SUART_H_ */
