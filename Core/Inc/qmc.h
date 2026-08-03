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
    QMC5883L_OSR_512 = 0,
    QMC5883L_OSR_256,
    QMC5883L_OSR_128,
    QMC5883L_OSR_64,
} QMC5883L_OSR_t;

typedef enum {
    QMC5883L_ODR_10HZ = 0,
    QMC5883L_ODR_50HZ,
    QMC5883L_ODR_100HZ,
    QMC5883L_ODR_200HZ,
} QMC5883L_ODR_t;

typedef enum {
    QMC5883L_RANGE_2G = 0,
    QMC5883L_RANGE_8G = 1,
} QMC5883L_Range_t;

typedef enum {
    QMC5883L_STANDBY = 0,
    QMC5883L_CONTINUOUS = 1,
} QMC5883L_Mode_t;

typedef struct
{
    QMC5883L_OSR_t osr;
    QMC5883L_Range_t range;
    QMC5883L_ODR_t odr;
    QMC5883L_Mode_t mode;
} QMC5883L_Ctrl1_t;

typedef enum {
    QMC5883L_INT_ENABLE  = 0,
    QMC5883L_INT_DISABLE = 1,
} QMC5883L_Int_t;

typedef enum {
    QMC5883L_ROL_DISABLE = 0,
    QMC5883L_ROL_ENABLE  = 1,
} QMC5883L_Rol_t;

typedef struct {
    QMC5883L_Rol_t roll_pointer;
    QMC5883L_Int_t interrupt;
} QMC5883L_Ctrl2_t;

typedef enum {
    QMC_EVT_NONE       = 0,
    QMC_EVT_DATA_READY = (1<<0),
    QMC_EVT_OVERFLOW   = (1<<1),
} QMC_Event_t;

typedef struct {
	I2C_HandleTypeDef *hi2c;

	uint32_t timeout;
	uint8_t address;
	uint8_t ctrl1_reg;
	uint8_t ctrl2_reg;
	volatile uint8_t status;

	volatile uint8_t events;
	uint16_t scale;
} QMC5883L_Handle_t;

typedef struct {
	int16_t x_axis;
	int16_t y_axis;
	int16_t z_axis;
} QMC5883L_Data_t;

HAL_StatusTypeDef QMC5883L_Init(QMC5883L_Handle_t *hqmc, I2C_HandleTypeDef *hi2c, uint8_t dev_address, uint32_t timeout);
HAL_StatusTypeDef QMC5883L_SetCtrl1(QMC5883L_Handle_t *hqmc, uint8_t ctrl1);
HAL_StatusTypeDef QMC5883L_SetCtrl2(QMC5883L_Handle_t *hqmc, uint8_t ctrl2);
uint8_t QMC5883L_Ctrl1Encode(const QMC5883L_Ctrl1_t *cfg);
uint8_t QMC5883L_Ctrl2Encode(const QMC5883L_Ctrl2_t *cfg);
HAL_StatusTypeDef QMC5883L_ReadRaw(QMC5883L_Handle_t *hqmc, QMC5883L_Data_t *data);
HAL_StatusTypeDef QMC5883L_ReadData(QMC5883L_Handle_t *hqmc, QMC5883L_Data_t *data);
HAL_StatusTypeDef QMC5883L_ReadTemp(QMC5883L_Handle_t *hqmc, int16_t *temp);
void QMC5883L_OnDataReadyIRQ(QMC5883L_Handle_t *hqmc);
HAL_StatusTypeDef QMC5883L_Reset(QMC5883L_Handle_t *hqmc);
HAL_StatusTypeDef QMC5883L_DeInit(QMC5883L_Handle_t *hqmc);

#endif /* INC_QMC_H_ */
