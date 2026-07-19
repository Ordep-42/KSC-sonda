/*
 * qmc.h
 *
 *  Created on: 19 de jul. de 2026
 *      Author: pedro
 */

#ifndef INC_QMC_H_
#define INC_QMC_H_

#include "main.h"

typedef enum {
    QMC_EVT_NONE       = 0,
    QMC_EVT_DATA_READY = (1<<0),
    QMC_EVT_OVERFLOW   = (1<<1),
} QMC_Event_t;

typedef struct {
	I2C_HandleTypeDef *hi2c;

	uint8_t address;
	uint8_t ctrl1_reg;
	uint8_t ctrl2_reg;
	uint8_t status;

	uint8_t events;
	uint16_t scale;
} QMC5883L_Handle_t;

typedef struct {
	int16_t x_axis;
	int16_t y_axis;
	int16_t z_axis;
} QMC5883L_Data_t;

HAL_StatusTypeDef QMC5883L_Init(QMC5883L_Handle_t *hqmc, I2C_HandleTypeDef *hi2c, uint8_t dev_address);
HAL_StatusTypeDef QMC5883L_SetCtrl1(QMC5883L_Handle_t *hqmc, uint8_t ctrl1);
HAL_StatusTypeDef QMC5883L_SetCtrl2(QMC5883L_Handle_t *hqmc, uint8_t ctrl2);
HAL_StatusTypeDef QMC5883L_ReadData(QMC5883L_Handle_t *hqmc, QMC5883L_Data_t *data);
void QMC5883L_OnDataReadyIRQ(QMC5883L_Handle_t *hqmc);
HAL_StatusTypeDef QMC5883L_Reset(QMC5883L_Handle_t *hqmc);

#endif /* INC_QMC_H_ */
