/*
 * sensors.c
 *
 *  Created on: 15.04.2011
 *      Author: philipp
 */

#define TWI_FREQ 100000
#define ACC_ADDRESS 0x1D

#include <stdint.h>
#include "twi_master.h"
#include "sensors.h"

uint8_t acc_dataX = 0;
uint8_t acc_dataY = 0;
uint8_t acc_dataZ = 0;

uint8_t Sensors_Init() {
	if (!TWIM_Init(TWI_FREQ)) {
		return 0;
	}

	return Sensors_AccInit();
}

uint8_t Sensors_AccInit() {
	if (!TWIM_SetRegister(ACC_ADDRESS, 0x2A, 0x07)) {
		return 0;
	}
	return TWIM_SetRegister(ACC_ADDRESS, 0x09, 0b01000000);
}

uint8_t Sensors_LoopStep() {
	static uint8_t step = 0;

	switch (step) {
	//get data.
	case 0:

		break;
	default:
		step = 0;
	}
	return 1;
}

uint8_t Sensors_AccPullData() {
	uint8_t status;

	//tell acc the right register.
	if (!TWIM_Start(ACC_ADDRESS, TW_WRITE) || !TWIM_Write(0x00)) {
		TWIM_Stop();
		return 0;
	}

	//get status.
	if (!TWIM_Start(ACC_ADDRESS, TW_READ)) {
		TWIM_Stop();
		return 0;
	}
	status = TWIM_ReadAck();
	if (!(status & 0b00111111)) {
		TWIM_Stop();
		return 1;
	}

	acc_dataX = TWIM_ReadAck();
	acc_dataY = TWIM_ReadAck();
	acc_dataZ = TWIM_ReadNack();
	TWIM_Stop();

	return 1;
}

uint8_t Sensors_AccGetX() {
	return acc_dataX;
}

uint8_t Sensors_AccGetY() {
	return acc_dataY;
}

uint8_t Sensors_AccGetZ() {
	return acc_dataZ;
}
