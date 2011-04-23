/*
 * usart.h
 *
 *  Created on: 18.04.2011
 *      Author: philipp
 */

#ifndef USART_H_
#define USART_H_


#include <stdint.h>

#define USART_STATUS_OK 1
#define USART_STATUS_RFIFO_OVERFLOW 2
#define USART_STATUS_WFIFO_OVERFLOW 4
#define USART_STATUS_DATA_AVAILABLE 8

/**
 * Only for testing...
 */
void USART_transmit(uint8_t data);

void USART_Init (uint16_t baud);

/**
 * Put data byte.
 */
uint8_t USART_putc(uint8_t byte);

/**
 * Get next data byte.
 *
 * returns -1 if no data is availible.
 */
uint8_t USART_getc(void);

/**
 * Get length of receive buffer.
 */
uint8_t USART_getRBufferLength(void);

/**
 * Step for main loop.
 */
uint8_t USART_LoopStep(void);

#endif /* USART_H_ */
