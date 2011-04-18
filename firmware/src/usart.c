/*
 * usart.c
 *
 *  Created on: 18.04.2011
 *      Author: philipp
 */

#include <stdint.h>
#include "general.h"
#include "fifo.h"
#include "usart.h"

#define USART_RBUF_SIZE 10
#define USART_WBUF_SIZE 20

#define USART_STATUS_ERROR 0xFE

uint8_t buffer_r[USART_RBUF_SIZE];
fifo_t fifo_r;

uint8_t buffer_w[USART_WBUF_SIZE];
fifo_t fifo_w;

void USART_Init (uint16_t baud)
{
	/* Set baud rate */
	UBRRH = (uint8_t) (baud>>8);
	UBRRL = (uint8_t) baud;

	/* Enable receiver and transmitter */
	UCSRB = (1<<RXEN)|(1<<TXEN);

	/* Set frame format: 8 data, 1 stop bit */
	UCSRC |= (1 << URSEL) | (1 << UCSZ1) | (1 << UCSZ0);

	/* Initialize read and write fifos */
	fifo_init(&fifo_r, buffer_r, USART_RBUF_SIZE);
	fifo_init(&fifo_w, buffer_w, USART_WBUF_SIZE);
}

void USART_transmit(uint8_t data)
{
	/* Wait for empty transmit buffer */
	while ( !( UCSRA & (1<<UDRE)) )
		;
	/* Put data into buffer, sends the data */
	UDR = data;
}

/**
 * Check whether we are ready to transmit another char.
 */
uint8_t USART_transmitReady(void)
{
	return (UCSRA & (1<<UDRE));
}

uint8_t USART_receive( void )
{
	/* Wait for data to be received */
	while ( !(UCSRA & (1<<RXC)) )
		;
	/* Get and return received data from buffer */
	return UDR;
}

/**
 * Check whether there is a character to read in.
 */
uint8_t USART_receiveReady(void)
{
	return (UCSRA & (1<<RXC));
}

/**
 * Put data byte to transmit fifo.
 */
uint8_t USART_putc(uint8_t byte)
{
	if (!fifo_put(&fifo_w, byte)) {
		return USART_STATUS_WFIFO_OVERFLOW;
	}

	return USART_STATUS_OK;
}

/**
 * Get data byte from receive fifo.
 *
 * Returns -1 if fifo is empty.
 */
int USART_getc(void)
{
	return fifo_get_nowait(&fifo_r);
}

/**
 * Step for main loop.
 */
uint8_t USART_LoopStep(void)
{
	int8_t status = USART_STATUS_OK;

	//send next char from fifo if ready.
	if (!fifo_isEmpty(&fifo_w) && USART_transmitReady()) {
		USART_transmit((uint8_t) fifo_get_wait(&fifo_w));
		debug_blink(1);
	}
	//receive next char from USART.
	if (USART_receiveReady()) {
		if (fifo_isFull(&fifo_r)) {
			status |= USART_STATUS_RFIFO_OVERFLOW;
			status &= USART_STATUS_ERROR;
		} else {
			fifo_put(&fifo_r, USART_receive());
			status |= USART_STATUS_DATA_AVAILABLE;
		}
	}

	return status;
}
