/*
 * bmp280.h
 *
 *  Created on: 14 de jul. de 2026
 *      Author: pedro
 */

#ifndef INC_BMP280_H_
#define INC_BMP280_H_

#include "main.h"

typedef struct {
	uint16_t dig_T1;
	int16_t dig_T2;
	int16_t dig_T3;
	uint16_t dig_P1;
	int16_t dig_P2;
	int16_t dig_P3;
	int16_t dig_P4;
	int16_t dig_P5;
	int16_t dig_P6;
	int16_t dig_P7;
	int16_t dig_P8;
	int16_t dig_P9;
	int32_t t_fine;
} BMP280_CalParams_t;

typedef struct {
	BMP280_CalParams_t calib;

	I2C_HandleTypeDef *hi2c;
	uint8_t chip_id;
	uint8_t address;
	uint8_t status;
	uint8_t ctrl_mode;
	int32_t adc_T_raw;
	int32_t adc_P_raw;
} BMP280_Handle_t;

typedef struct {
	int32_t temp;
	uint32_t pres;
} BMP280_Data_t;

HAL_StatusTypeDef BMP280_Init(BMP280_Handle_t *hbmp, I2C_HandleTypeDef *hi2c, uint8_t dev_address);
HAL_StatusTypeDef BMP280_SetMode(BMP280_Handle_t *hbmp, uint8_t mode);
HAL_StatusTypeDef BMP280_ReadData(BMP280_Handle_t *hbmp, BMP280_Data_t *data);

#endif /* INC_BMP280_H_ */
