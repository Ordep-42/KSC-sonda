/*
 * mpu.h
 *
 *  Created on: 20 de jul. de 2026
 *      Author: pedro
 */

#ifndef INC_MPU_H_
#define INC_MPU_H_

#include "main.h"
#include "mpu_regs.h"

typedef struct {
    I2C_HandleTypeDef *hi2c;

    uint32_t timeout;
    uint8_t address;

    uint8_t accel_range;
    uint8_t gyro_range;
    uint8_t dlpf_cfg;

    uint16_t accel_scale;
    uint16_t gyro_scale;

    uint8_t smplrt_div_reg;
    uint8_t config_reg;
    uint8_t gyro_config_reg;
    uint8_t accel_config_reg;
    uint8_t fifo_en_reg;
    uint8_t int_pin_cfg_reg;
    uint8_t int_enable_reg;
    uint8_t user_ctrl_reg;
    uint8_t pwr_mgmt_1_reg;
    uint8_t pwr_mgmt_2_reg;

    volatile uint8_t events;
} MPU6050_Handle_t;

typedef struct {
	int16_t x_axis;
	int16_t y_axis;
	int16_t z_axis;
} MPU6050_AxisData_t;

typedef MPU6050_AxisData_t MPU6050_GyroData_t;
typedef MPU6050_AxisData_t MPU6050_AccelData_t;

typedef struct {
    int16_t temperature;
    MPU6050_AccelData_t accel;
    MPU6050_GyroData_t gyro;
} MPU6050_Data_t;

HAL_StatusTypeDef MPU6050_Init(MPU6050_Handle_t *hmpu, I2C_HandleTypeDef *hi2c, uint8_t dev_address, uint32_t i2c_timeout);
HAL_StatusTypeDef MPU6050_SetSampleRate(MPU6050_Handle_t *hmpu, uint8_t smplrt_div);
HAL_StatusTypeDef MPU6050_SetConfig(MPU6050_Handle_t *hmpu, const MPU6050_Config_t *config);
HAL_StatusTypeDef MPU6050_SetGyroConfig(MPU6050_Handle_t *hmpu, const MPU6050_GyroConfig_t *config);
HAL_StatusTypeDef MPU6050_SetAccelConfig(MPU6050_Handle_t *hmpu, const MPU6050_AccelConfig_t *config);
HAL_StatusTypeDef MPU6050_SetFifoEnable(MPU6050_Handle_t *hmpu, uint8_t flags);
HAL_StatusTypeDef MPU6050_SetIntPinConfig(MPU6050_Handle_t *hmpu, uint8_t flags);
HAL_StatusTypeDef MPU6050_SetIntEnable(MPU6050_Handle_t *hmpu, uint8_t flags);
HAL_StatusTypeDef MPU6050_SetUserCtrl(MPU6050_Handle_t *hmpu, uint8_t flags);
HAL_StatusTypeDef MPU6050_SetPwrMgmt1(MPU6050_Handle_t *hmpu, const MPU6050_PwrMgmt1_t *config);
HAL_StatusTypeDef MPU6050_SetPwrMgmt2(MPU6050_Handle_t *hmpu, const MPU6050_PwrMgmt2_t *config);

HAL_StatusTypeDef MPU6050_ReadData(MPU6050_Handle_t *hmpu, MPU6050_Data_t *data);
HAL_StatusTypeDef MPU6050_ReadAccelData(MPU6050_Handle_t *hmpu, MPU6050_AccelData_t *accel_data);
HAL_StatusTypeDef MPU6050_ReadGyroData(MPU6050_Handle_t *hmpu, MPU6050_GyroData_t *gyro_data);
HAL_StatusTypeDef MPU6050_ReadTempData(MPU6050_Handle_t *hmpu, int16_t *temp_data);

HAL_StatusTypeDef MPU6050_Reset(MPU6050_Handle_t *hmpu);
HAL_StatusTypeDef MPU6050_DeInit(MPU6050_Handle_t *hmpu);

#endif /* INC_MPU_H_ */
