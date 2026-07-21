/*
 * bmp280.h
 *
 *  Created on: 14 de jul. de 2026
 *      Author: pedro
 */

#ifndef INC_BMP280_H_
#define INC_BMP280_H_

#include "main.h"

typedef enum {
    BMP280_OSRS_SKIP = 0,
    BMP280_OSRS_X1,
    BMP280_OSRS_X2,
    BMP280_OSRS_X4,
    BMP280_OSRS_X8,
    BMP280_OSRS_X16
} BMP280_Oversampling_t;

typedef enum {
    BMP280_FILTER_OFF = 0,
    BMP280_FILTER_2,
    BMP280_FILTER_4,
    BMP280_FILTER_8,
    BMP280_FILTER_16
} BMP280_Filter_t;

typedef enum {
    BMP280_MODE_SLEEP = 0,
    BMP280_MODE_FORCED = 1,
    BMP280_MODE_NORMAL = 3
} BMP280_Mode_t;

typedef enum {
    BMP280_STANDBY_0_5MS = 0,
    BMP280_STANDBY_62_5MS,
    BMP280_STANDBY_125MS,
    BMP280_STANDBY_250MS,
    BMP280_STANDBY_500MS,
    BMP280_STANDBY_1000MS,
    BMP280_STANDBY_2000MS,
    BMP280_STANDBY_4000MS
} BMP280_Standby_t;

typedef struct
{
    BMP280_Oversampling_t osrs_t;
    BMP280_Oversampling_t osrs_p;
    BMP280_Mode_t mode;
} BMP280_Ctrl_t;

typedef struct
{
    BMP280_Standby_t standby;
    BMP280_Filter_t filter;
    uint8_t spi3w_enable;
} BMP280_Config_t;

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

	uint32_t timeout;
	uint8_t address;
	uint8_t status;
	uint8_t ctrl_mode;
	uint8_t config;
	int32_t adc_T_raw;
	int32_t adc_P_raw;

	I2C_HandleTypeDef *hi2c;
} BMP280_Handle_t;

typedef struct {
	int32_t temp;
	uint32_t pres;
} BMP280_Data_t;

HAL_StatusTypeDef BMP280_Init(BMP280_Handle_t *hbmp, I2C_HandleTypeDef *hi2c, uint8_t dev_address, uint32_t timeout);
HAL_StatusTypeDef BMP280_SetMode(BMP280_Handle_t *hbmp, uint8_t mode);
HAL_StatusTypeDef BMP280_SetConfig(BMP280_Handle_t *hbmp, uint8_t config);
uint8_t BMP280_CtrlEncode(const BMP280_Ctrl_t *cfg);
uint8_t BMP280_ConfigEncode(const BMP280_Config_t *cfg);
HAL_StatusTypeDef BMP280_ReadData(BMP280_Handle_t *hbmp, BMP280_Data_t *data);
HAL_StatusTypeDef BMP280_Reset(BMP280_Handle_t *hbmp);
HAL_StatusTypeDef BMP280_DeInit(BMP280_Handle_t *hbmp);

#endif /* INC_BMP280_H_ */
