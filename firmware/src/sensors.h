/*
 * sensors.h
 *
 *  Created on: 16.04.2011
 *      Author: philipp
 */

#ifndef SENSORS_H_
#define SENSORS_H_


uint8_t Sensors_Init();

uint8_t Sensors_AccInit();

uint8_t Sensors_AccPullData();

uint8_t Sensors_AccGetX();
uint8_t Sensors_AccGetY();
uint8_t Sensors_AccGetZ();

uint8_t Sensors_AccGetAll();

#endif /* SENSORS_H_ */
