/*
 * qmc.c
 *
 *  Created on: 19 de jul. de 2026
 *      Author: pedro
 */

#include "qmc.h"
#include "i2c.h"

#include <string.h>

#define QMC5883L_REG_DATA 0x00
#define QMC5883L_REG_STATUS 0x06
#define QMC5883L_REG_TEMP_DATA 0x07
#define QMC5883L_REG_CTRL1 0x09
#define QMC5883L_REG_CTRL2 0x0A
#define QMC5883L_REG_RESET 0x0B

#define QMC5883L_INIT 0x01
#define QMC5883L_SOFT_RST 0x80

#define QMC5883L_STATUS_DRDY (1<<0)
#define QMC5883L_STATUS_OVL  (1<<1)
#define QMC5883L_STATUS_DOR  (1<<2)

#define QMC5883L_RNG_BIT_POS 4
#define QMC5883L_RNG_MASK (0x01 << QMC5883L_RNG_BIT_POS)
#define QMC5883L_RNG_2G 0
#define QMC5883L_RNG_8G 1
#define QMC5883L_RNG_2G_SCALE 12000
#define QMC5883L_RNG_8G_SCALE 3000
#define QMC5883L_RAW_DATA_LEN 6

static uint16_t qmc_u16_le(const uint8_t *buf) {
	return (uint16_t)buf[0] |
			((uint16_t)buf[1] << 8);
}

static int16_t qmc_s16_le(const uint8_t *buf) {
	return (int16_t)qmc_u16_le(buf);
}

static int16_t qmc_to_uT(int16_t raw, uint16_t sensitivity)
{
    return (int16_t)((int32_t)raw * 100 / sensitivity);
}

static HAL_StatusTypeDef qmc_read_reg(QMC5883L_Handle_t *hqmc, uint8_t reg, uint8_t *data, const uint16_t len) {
	HAL_StatusTypeDef status;
	status = HAL_I2C_Master_Transmit(hqmc->hi2c, hqmc->address, &reg, 1, hqmc->timeout);

	if (status != HAL_OK) return status;

	return HAL_I2C_Master_Receive(hqmc->hi2c, hqmc->address, data, len, hqmc->timeout);
}

static HAL_StatusTypeDef qmc_write_reg(QMC5883L_Handle_t *hqmc, const uint8_t reg, uint8_t *data, const uint16_t len) {
	uint8_t tx[len + 1];
	tx[0] = reg;
	memcpy(&tx[1], data, len);

	return HAL_I2C_Master_Transmit(hqmc->hi2c, hqmc->address, tx, len+1, hqmc->timeout);
}

HAL_StatusTypeDef QMC5883L_Init(QMC5883L_Handle_t *hqmc, I2C_HandleTypeDef *hi2c, uint8_t dev_address, uint32_t timeout) {
	if (hqmc == NULL || hi2c == NULL || timeout == 0) {
		return HAL_ERROR;
	}

	memset(hqmc, 0, sizeof(*hqmc));
	hqmc->hi2c = hi2c;
	hqmc->address = dev_address;
	hqmc->timeout;

	HAL_StatusTypeDef status;
	uint8_t init_cmd = QMC5883L_INIT;
	status = qmc_write_reg(hqmc, QMC5883L_REG_RESET, &init_cmd, 1);

	if (status != HAL_OK) return status;

	return HAL_OK;
}

HAL_StatusTypeDef QMC5883L_SetCtrl1(QMC5883L_Handle_t *hqmc, uint8_t ctrl1) {
	if (hqmc == NULL) return HAL_ERROR;

	HAL_StatusTypeDef status;
	status = qmc_write_reg(hqmc, QMC5883L_REG_CTRL1, &ctrl1, 1);
	if (status != HAL_OK) return status;

	uint8_t ctrl1_reg = 0;
	status = qmc_read_reg(hqmc, QMC5883L_REG_CTRL1, &ctrl1_reg, 1);
	if (status != HAL_OK) return status;
	if (ctrl1_reg != ctrl1) return HAL_ERROR;

	hqmc->ctrl1_reg = ctrl1_reg;
	switch ((ctrl1_reg & QMC5883L_RNG_MASK) >> QMC5883L_RNG_BIT_POS) {
	case QMC5883L_RNG_2G:
		hqmc->scale = QMC5883L_RNG_2G_SCALE;
		break;
	case QMC5883L_RNG_8G:
		hqmc->scale = QMC5883L_RNG_8G_SCALE;
		break;
	default:
		return HAL_ERROR;
	}
	return HAL_OK;
}

HAL_StatusTypeDef QMC5883L_SetCtrl2(QMC5883L_Handle_t *hqmc, uint8_t ctrl2) {
	if (hqmc == NULL) return HAL_ERROR;

	HAL_StatusTypeDef status;
	status = qmc_write_reg(hqmc, QMC5883L_REG_CTRL2, &ctrl2, 1);
	if (status != HAL_OK) return status;

	uint8_t ctrl2_reg = 0;
	status = qmc_read_reg(hqmc, QMC5883L_REG_CTRL2, &ctrl2_reg, 1);
	if (status != HAL_OK) return status;
	if (ctrl2_reg != ctrl2) return HAL_ERROR;

	hqmc->ctrl2_reg = ctrl2_reg;
	return HAL_OK;
}

uint8_t QMC5883L_Ctrl1Encode(const QMC5883L_Ctrl1_t *cfg) {
	if (cfg == NULL) return 0;

	return ((uint8_t)cfg->osr   << 6) |
		   ((uint8_t)cfg->range << 4) |
		   ((uint8_t)cfg->odr   << 2) |
		   ((uint8_t)cfg->mode);
}

uint8_t QMC5883L_Ctrl2Encode(const QMC5883L_Ctrl2_t *cfg) {
    if (cfg == NULL) return 0;

    return ((uint8_t)cfg->roll_pointer << 6) |
           ((uint8_t)cfg->interrupt);
}

HAL_StatusTypeDef QMC5883L_ReadRaw(QMC5883L_Handle_t *hqmc, QMC5883L_Data_t *data) {
	if (hqmc == NULL || data == NULL) {
		return HAL_ERROR;
	}
	HAL_StatusTypeDef status;

	uint8_t status_reg;
	status = qmc_read_reg(hqmc, QMC5883L_REG_STATUS, &status_reg, 1);
	if (status != HAL_OK) return status;

	hqmc->status = status_reg;
	if (!(status_reg & QMC5883L_STATUS_DRDY)) return HAL_BUSY;
	if (status_reg & QMC5883L_STATUS_OVL) hqmc->events |= QMC_EVT_OVERFLOW;

	uint8_t adc_raw[QMC5883L_RAW_DATA_LEN];
	status = qmc_read_reg(hqmc, QMC5883L_REG_DATA, adc_raw, QMC5883L_RAW_DATA_LEN);
	if (status != HAL_OK) return status;

	data->x_axis = qmc_s16_le(&adc_raw[0]);
	data->y_axis = qmc_s16_le(&adc_raw[2]);
	data->z_axis = qmc_s16_le(&adc_raw[4]);

	return HAL_OK;
}

HAL_StatusTypeDef QMC5883L_ReadData(QMC5883L_Handle_t *hqmc, QMC5883L_Data_t *data) {
	if (hqmc == NULL || data == NULL) {
		return HAL_ERROR;
	}
	HAL_StatusTypeDef status;
	uint8_t status_reg;
	status = qmc_read_reg(hqmc, QMC5883L_REG_STATUS, &status_reg, 1);
	if (status != HAL_OK) return status;

	hqmc->status = status_reg;
	if (!(status_reg & QMC5883L_STATUS_DRDY)) return HAL_BUSY;
	if (status_reg & QMC5883L_STATUS_OVL) hqmc->events |= QMC_EVT_OVERFLOW;

	QMC5883L_Data_t raw_data;

	status = QMC5883L_ReadRaw(hqmc, &raw_data);
	if (status != HAL_OK) return status;

	data->x_axis = qmc_to_uT(raw_data.x_axis, hqmc->scale);
	data->y_axis = qmc_to_uT(raw_data.y_axis, hqmc->scale);
	data->z_axis = qmc_to_uT(raw_data.z_axis, hqmc->scale);

	return HAL_OK;
}

HAL_StatusTypeDef QMC5883L_ReadTemp(QMC5883L_Handle_t *hqmc, int16_t *temp) {
	if (hqmc == NULL || temp == NULL) {
		return HAL_ERROR;
	}
	HAL_StatusTypeDef status;
	uint8_t raw_temp[2];
	status = qmc_read_reg(hqmc, QMC5883L_REG_TEMP_DATA, raw_temp, 2);
	if (status != HAL_OK) return status;

	*temp = qmc_s16_le(raw_temp);
	return HAL_OK;
}

void QMC5883L_OnDataReadyIRQ(QMC5883L_Handle_t *hqmc)
{
    if (hqmc == NULL)
        return;

    hqmc->events |= QMC_EVT_DATA_READY;
}

HAL_StatusTypeDef QMC5883L_Reset(QMC5883L_Handle_t *hqmc) {
	if (hqmc == NULL) return HAL_ERROR;

	uint8_t cmd = QMC5883L_SOFT_RST;
	return qmc_write_reg(hqmc, QMC5883L_REG_CTRL2, &cmd, 1);
}

HAL_StatusTypeDef QMC5883L_DeInit(QMC5883L_Handle_t *hqmc) {
    if (hqmc == NULL)
        return HAL_ERROR;

    (void)QMC5883L_Reset(hqmc);

    memset(hqmc, 0, sizeof(*hqmc));

    return HAL_OK;
}
