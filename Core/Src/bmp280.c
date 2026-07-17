/*
 * bmp280.c
 *
 *  Created on: 14 de jul. de 2026
 *      Author: pedro
 */

#include "bmp280.h"
#include "i2c.h"

#include <string.h>

#define BMP280_REG_CHIPID 0xD0
#define BMP280_REG_CONFIG 0xF5
#define BMP280_REG_CTRL 0xF4
#define BMP280_REG_STATUS 0xF3
#define BMP280_REG_RESET 0xE0
#define BMP280_REG_CAL 0x88
#define BMP280_REG_DATA 0xF7

#define BMP280_MEASURING 0x08
#define BMP280_CHIPID 0x58
#define BMP280_RESET 0xB6

static uint16_t bmp_u16_le(const uint8_t *buf) {
	return (uint16_t)buf[0] |
			((uint16_t)buf[1] << 8);
}

static int16_t bmp_s16_le(const uint8_t *buf) {
	return (int16_t)bmp_u16_le(buf);
}

static int32_t bmp_build_adc20(const uint8_t data[3]) {
	return ((int32_t)data[0] << 12) |
		   ((int32_t)data[1] << 4 ) |
		   ((int32_t)data[2] >> 4 );
}

static int32_t compensate_temp(BMP280_CalParams_t *calib, int32_t adc_T_raw) {
	int32_t var1, var2, temp;

	var1 = ((((adc_T_raw >> 3) - ((int32_t)calib->dig_T1<<1))) * ((int32_t)calib->dig_T2)) >> 11;
	var2 = (((((adc_T_raw >> 4) - ((int32_t)calib->dig_T1)) * ((adc_T_raw>>4) - ((int32_t)calib->dig_T1))) >> 12)
			* ((int32_t)calib->dig_T3)) >> 14;
	calib->t_fine = var1 + var2;
	temp = (calib->t_fine * 5 + 128) >> 8;

	return temp;
}

static uint32_t compensate_pres(BMP280_CalParams_t *calib, int32_t adc_P_raw) {
	int32_t var1, var2;
	uint32_t pres;
	var1 = (((int32_t)calib->t_fine)>>1) - (int32_t)64000;
	var2 = (((var1>>2) * (var1>>2)) >> 11) * ((int32_t)calib->dig_P6);
	var2 = var2 + ((var1 * ((int32_t)calib->dig_P5)) << 1);
	var2 = (var2>>2)+(((int32_t)calib->dig_P4)<<16);
	var1 = (((calib->dig_P3 * (((var1>>2) * (var1>>2)) >> 13))>>3) + ((((int32_t)calib->dig_P2) * var1)>>1))>>18;
	var1 = ((((32768 + var1)) * ((int32_t)calib->dig_P1))>>15);

	if (var1 == 0) return 0;
	pres = (((uint32_t)(((int32_t)1048576)-adc_P_raw)-(var2>>12)))*3125;

	if (pres < 0x80000000) {
		pres = (pres << 1) / ((uint32_t)var1);
	} else {
		pres = (pres / (uint32_t)var1) * 2;
	}

	var1 = (((int32_t)calib->dig_P9) * ((int32_t)(((pres>>3) * (pres>>3))>>13)))>>12;
	var2 = (((int32_t)(pres>>2)) * ((int32_t)calib->dig_P8))>>13;
	pres = (uint32_t)((int32_t)pres + ((var1 + var2 + calib->dig_P7) >> 4));
	return pres;
}

HAL_StatusTypeDef bmp_read_reg(BMP280_Handle_t *hbmp, uint8_t reg, uint8_t *data, uint16_t len) {
	HAL_StatusTypeDef status;
	status = HAL_I2C_Master_Transmit(hbmp->hi2c, hbmp->address, &reg, 1, HAL_MAX_DELAY);

	if (status != HAL_OK) return status;

	return HAL_I2C_Master_Receive(hbmp->hi2c, hbmp->address, data, len, HAL_MAX_DELAY);
}

HAL_StatusTypeDef bmp_write_reg(BMP280_Handle_t *hbmp, uint8_t reg, uint8_t *data, uint16_t len) {
	uint8_t tx[len + 1];
	tx[0] = reg;
	memcpy(&tx[1], data, len);

	return HAL_I2C_Master_Transmit(hbmp->hi2c, hbmp->address, tx, len+1, HAL_MAX_DELAY);
}

HAL_StatusTypeDef bmp_read_raw_data(BMP280_Handle_t *hbmp) {
	HAL_StatusTypeDef status;
	uint8_t adc_raw[6];
	status = bmp_read_reg(hbmp, BMP280_REG_DATA, adc_raw, 6);
	if (status != HAL_OK) return status;

	hbmp->adc_P_raw = bmp_build_adc20(&adc_raw[0]);
	hbmp->adc_T_raw = bmp_build_adc20(&adc_raw[3]);

	return HAL_OK;
}

HAL_StatusTypeDef bmp_read_calibration(BMP280_Handle_t *hbmp) {
	if (hbmp == NULL) return HAL_ERROR;

	HAL_StatusTypeDef status;
	uint8_t calib_raw[26];
	status = bmp_read_reg(hbmp, BMP280_REG_CAL, calib_raw, sizeof(calib_raw));
	if (status != HAL_OK) return status;

	hbmp->calib.dig_T1 = bmp_u16_le(&calib_raw[0]);
	hbmp->calib.dig_T2 = bmp_s16_le(&calib_raw[2]);
	hbmp->calib.dig_T3 = bmp_s16_le(&calib_raw[4]);
	hbmp->calib.dig_P1 = bmp_u16_le(&calib_raw[6]);
	hbmp->calib.dig_P2 = bmp_s16_le(&calib_raw[8]);
	hbmp->calib.dig_P3 = bmp_s16_le(&calib_raw[10]);
	hbmp->calib.dig_P4 = bmp_s16_le(&calib_raw[12]);
	hbmp->calib.dig_P5 = bmp_s16_le(&calib_raw[14]);
	hbmp->calib.dig_P6 = bmp_s16_le(&calib_raw[16]);
	hbmp->calib.dig_P7 = bmp_s16_le(&calib_raw[18]);
	hbmp->calib.dig_P8 = bmp_s16_le(&calib_raw[20]);
	hbmp->calib.dig_P9 = bmp_s16_le(&calib_raw[22]);

	return HAL_OK;
}

HAL_StatusTypeDef BMP280_Init(BMP280_Handle_t *hbmp, I2C_HandleTypeDef *hi2c, uint8_t dev_address) {
	if (hbmp == NULL || hi2c == NULL) {
		return HAL_ERROR;
	}
	hbmp->hi2c = hi2c;
	hbmp->address = dev_address;
	uint8_t chipid;
	HAL_StatusTypeDef status;
	status = bmp_read_reg(hbmp, BMP280_REG_CHIPID, &chipid, 1);

	if (status != HAL_OK) return status;
	if (chipid != BMP280_CHIPID) return HAL_ERROR;

	return bmp_read_calibration(hbmp);
}

HAL_StatusTypeDef BMP280_SetMode(BMP280_Handle_t *hbmp, uint8_t mode) {
	if (hbmp == NULL) return HAL_ERROR;

	HAL_StatusTypeDef status;
	status = bmp_write_reg(hbmp, BMP280_REG_CTRL, &mode, 1);
	if (status != HAL_OK) return status;

	uint8_t ctrl_reg = 0;
	status = bmp_read_reg(hbmp, BMP280_REG_CTRL, &ctrl_reg, 1);
	if (status != HAL_OK) return status;
	if (ctrl_reg != mode) return HAL_ERROR;

	hbmp->ctrl_mode = ctrl_reg;
	return HAL_OK;
}

HAL_StatusTypeDef BMP280_SetConfig(BMP280_Handle_t *hbmp, uint8_t config) {
	if (hbmp == NULL) return HAL_ERROR;

	HAL_StatusTypeDef status;
	status = bmp_write_reg(hbmp, BMP280_REG_CONFIG, &config, 1);
	if (status != HAL_OK) return status;

	uint8_t config_reg = 0;
	status = bmp_read_reg(hbmp, BMP280_REG_CONFIG, &config_reg, 1);
	if (status != HAL_OK) return status;
	if (config_reg != config) return HAL_ERROR;

	hbmp->config = config_reg;
	return HAL_OK;
}

HAL_StatusTypeDef BMP280_ReadData(BMP280_Handle_t *hbmp, BMP280_Data_t *data) {
	if (hbmp == NULL || data == NULL) {
		return HAL_ERROR;
	}
	HAL_StatusTypeDef status;

	status = bmp_read_raw_data(hbmp);
	if (status != HAL_OK) return status;

	data->temp = compensate_temp(&hbmp->calib, hbmp->adc_T_raw);
	data->pres = compensate_pres(&hbmp->calib, hbmp->adc_P_raw);

	return HAL_OK;
}

HAL_StatusTypeDef BMP280_Reset(BMP280_Handle_t *hbmp) {
	if (hbmp == NULL) return HAL_ERROR;

	uint8_t cmd = BMP280_RESET;
	return bmp_write_reg(hbmp, BMP280_REG_RESET, &cmd, 1);
}
