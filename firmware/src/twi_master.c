/*******************************************************
 Author:					Manfred Langemann
 mailto:					Manfred.Langemann ät t-online.de
 Begin of project:			04.01.2008
 Latest version generated:	04.01.2008
 Filename:					TWI_Master.c
 Description:    			TWI Master functions

 Master code adapted form Peter Fleury <pfleury@gmx.ch>
 http://jump.to/fleury
 --------------------------------------------------------
 Program Example
 --------------------------------------------------------

 int main (void)
 {
 uint8_t		i=0;
 uint8_t		j=0;
 uint8_t		Data[8];
 uint8_t		SlaveAddress = 15;

 // Clear any interrupt
 cli ();

 // Wait 1 second for POR
 Delay_ms (1000);

 // Initiate RS232
 RS232_Init ();
 printf ("Hello world...\n");

 // Initiate TWI Master with bitrate of 100000 Hz
 TWIM_Init (100000);

 // Endless loop
 while (1)
 {
 // Read byte(s) from the slave.
 // It is implicitely assumed, that the slave will send
 // 8 bytes.
 if (!TWIM_Start (SlaveAddress, TWIM_READ))
 {
 TWIM_Stop ();
 printf ("Could not start TWI Bus for READ\n");
 }
 else
 {
 for (i=0;i<7;i++)
 {
 Data[i] = TWIM_ReadAck ();
 }
 Data[7] = TWIM_ReadNack ();
 printf ("Reading Byte %d: %d\n", i, Data[i]);
 TWIM_Stop ();
 Delay_ms (1000);
 }

 // Write byte(s) to the slave.
 // It is implicitely assumed, that the slave will
 // accepts 8 bytes

 if (!TWIM_Start (SlaveAddress, TWIM_WRITE))
 {
 TWIM_Stop ();
 printf ("Could not start TWI Bus for WRITE\n");
 }
 else
 {
 for (i=0;i<8;i++)
 {
 TWIM_Write (j++);
 printf ("Byte %d sent\n", j);
 }
 TWIM_Stop ();
 Delay_ms (1000);
 }

 // Do something else
 i++;
 }

 return 0;
 }

 ********************************************************/
#include <stdio.h>
#include <stdint.h>
#include <avr/interrupt.h>

#include "twi_master.h"

/*******************************************************
 Public Function: TWIM_Init

 Purpose: Initialise the TWI Master Interface

 Input Parameter:
 - uint16_t	TWI_Bitrate (Hz)

 Return Value: uint8_t
 - FALSE:	Bitrate too high
 - TRUE:		Bitrate OK

 *******************************************************/
uint8_t TWIM_Init(uint32_t TWI_Bitrate) {
	/*
	 ** Set TWI bitrate
	 ** If bitrate is too high, then error return
	 */
	TWBR = ((F_CPU / TWI_Bitrate) - 16) / 2;
	if (TWBR < 11)
		return 0;

	return 1;
}

/*******************************************************
 Public Function: TWIM_Start

 Purpose: Start the TWI Master Interface

 Input Parameter:
 - uint8_t	Device address
 - uint8_t	Type of required Operation:
 TWIM_READ: Read data from the slave
 TWIM_WRITE: Write data to the slave

 Return Value: uint8_t
 - TRUE:		OK, TWI Master accessible
 - FALSE:	Error in starting TWI Master

 *******************************************************/
uint8_t TWIM_Start(uint8_t Address, uint8_t TWIM_Type) {
	/*
	 ** Send START condition
	 */
	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
	/*
	 ** Wait until transmission completed
	 */
	while (!(TWCR & (1 << TWINT)))
		;
	/*
	 ** Check value of TWI Status Register. Mask prescaler bits.
	 */
	if ((TW_STATUS != TW_START) && (TW_STATUS != TW_REP_START))
		return 0;

	/*
	 ** Send device address
	 */
	TWDR = (Address << 1) + TWIM_Type;
	TWCR = (1 << TWINT) | (1 << TWEN);
	/*
	 ** Wait until transmission completed and ACK/NACK has been received
	 */
	while (!(TWCR & (1 << TWINT)))
		;
	/*
	 ** Check value of TWI Status Register. Mask prescaler bits.
	 */
	if ((TW_STATUS != TW_MT_SLA_ACK) && (TW_STATUS != TW_MR_SLA_ACK))
		return 0;

	return 1;
}

/*******************************************************
 Public Function: TWIM_Stop

 Purpose: Stop the TWI Master

 Input Parameter: None

 Return Value: None

 *******************************************************/
void TWIM_Stop(void) {
	/*
	 ** Send stop condition
	 */
	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
	/*
	 ** Wait until stop condition is executed and bus released
	 */
	while (TWCR & (1 << TWINT))
		;
}

/*******************************************************
 Public Function: TWIM_Write

 Purpose: Write a byte to the slave

 Input Parameter:
 - uint8_t	Byte to be sent

 Return Value: uint8_t
 - TRUE:		OK, Byte sent
 - FALSE:	Error in byte transmission

 *******************************************************/
uint8_t TWIM_Write(uint8_t byte) {
	/*
	 ** Send data to the previously addressed device
	 */
	TWDR = byte;
	TWCR = (1 << TWINT) | (1 << TWEN);
	/*
	 ** Wait until transmission completed
	 */
	while (!(TWCR & (1 << TWINT)))
		;
	/*
	 ** Check whether ACK was received.
	 */
	if (TW_STATUS != TW_MT_DATA_ACK)
		return 0;

	return 1;
}

uint8_t TWIM_SetRegister(uint8_t deviceAddr, uint8_t regAddr, uint8_t byte) {
	if (!TWIM_Start(deviceAddr, TW_WRITE)
		|| !TWIM_Write(regAddr)
		|| !TWIM_Write(byte)) {
		TWIM_Stop();
		return 0;
	}
	TWIM_Stop();
	return 1;
}

/*******************************************************
 Public Function: TWIM_ReadAck

 Purpose: Read a byte from the slave and request next byte

 Input Parameter: None

 Return Value: uint8_t
 - uint8_t	Read byte

 *******************************************************/
uint8_t TWIM_ReadAck(void) {
	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
	while (!(TWCR & (1 << TWINT)))
		;

	return TWDR;
}

/*******************************************************
 Public Function: TWIM_ReadAck

 Purpose: Read the last byte from the slave

 Input Parameter: None

 Return Value: uint8_t
 - uint8_t	Read byte

 *******************************************************/
uint8_t TWIM_ReadNack(void) {
	TWCR = (1 << TWINT) | (1 << TWEN);
	while (!(TWCR & (1 << TWINT)))
		;

	return TWDR;
}
