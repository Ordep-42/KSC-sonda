/*
 * aht.h
 *
 *  Created on: 29 de mai. de 2026
 *      Author: pedro
 */

#ifndef INC_AHT_H_
#define INC_AHT_H_

#include "main.h"

typedef struct {
	int16_t temp;
	uint16_t humi;
} AHT_Data_t;

typedef struct {
	uint8_t addr;
	I2C_HandleTypeDef *hi2c;
	uint32_t timeout;
} AHT_Handle_t;

HAL_StatusTypeDef AHT_Init(AHT_Handle_t *haht, I2C_HandleTypeDef *hi2c, uint8_t dev_addr, uint32_t timeout);
HAL_StatusTypeDef AHT_ReadData(AHT_Handle_t *haht, AHT_Data_t *data);
HAL_StatusTypeDef AHT_TriggerMeasurement(AHT_Handle_t *haht);

#endif /* INC_AHT_H_ */
