/*
 * aht.h
 *
 *  Created on: 29 de mai. de 2026
 *      Author: pedro
 */

#ifndef INC_AHT_H_
#define INC_AHT_H_

#include "main.h"

typedef enum {
	AHT_IDLE = 0,
	AHT_BUSY,
	AHT_ERROR
} AHT_State_t;

typedef enum {
	AHT_EVT_NONE = 0,
	AHT_EVT_TRIGGER = (1<<0),
	AHT_EVT_DAVAIL = (1<<1),
	AHT_EVT_DREADY = (1<<2),
} AHT_Event_t;

typedef struct {
	int16_t temp;
	uint16_t humi;
} AHT_Data_t;

typedef struct {
	uint8_t addr;
	I2C_HandleTypeDef *hi2c;
	AHT_State_t state;
	volatile uint32_t events;
	TIM_HandleTypeDef *htim;

	uint8_t rx_buf[6];
} AHT_Handle_t;

void AHT_Init(AHT_Handle_t *haht, I2C_HandleTypeDef *hi2c, uint8_t dev_addr, TIM_HandleTypeDef *htim);
HAL_StatusTypeDef AHT_ReadData(AHT_Handle_t *haht, AHT_Data_t *data);
HAL_StatusTypeDef AHT_TriggerMeasurement(AHT_Handle_t *haht);

#endif /* INC_AHT_H_ */
