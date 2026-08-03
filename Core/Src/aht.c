/*
 * aht.c
 *
 *  Created on: 29 de mai. de 2026
 *      Author: pedro
 */

#include "aht.h"
#include "i2c.h"

#include <string.h>

#define AHT10_SCALE (1UL << 20)
#define AHT10_BUSY_BIT 0x80 // 0b10000000

#define AHT10_CMD_TRIGGER_MEASUREMENT  0xAC
#define AHT10_PARAM_MODE               0x33
#define AHT10_PARAM_NOP                0x00

static const uint8_t aht10_measure_cmd[3] = {
    AHT10_CMD_TRIGGER_MEASUREMENT,
    AHT10_PARAM_MODE,
    AHT10_PARAM_NOP
};

HAL_StatusTypeDef AHT_Init(AHT_Handle_t *haht, I2C_HandleTypeDef *hi2c, const uint8_t dev_addr, uint32_t timeout) {
	if (haht == NULL || hi2c == NULL || timeout == 0) return HAL_ERROR;
	haht->hi2c = hi2c;
	haht->addr = dev_addr;
	haht->timeout = timeout;
	return HAL_OK;
}

HAL_StatusTypeDef AHT_ReadData(AHT_Handle_t *haht, AHT_Data_t *data) {
	if (haht == NULL || data == NULL) return HAL_ERROR;

	uint8_t rx_buf[6];
	HAL_StatusTypeDef status = HAL_I2C_Master_Receive(
			  haht->hi2c,
			  haht->addr,
			  rx_buf,
			  6,
			  haht->timeout);

	if (status != HAL_OK) return status;

	uint32_t raw;
	if (rx_buf[0] & AHT10_BUSY_BIT) return HAL_BUSY;

	// Temperatura
	raw =
		(((uint32_t)rx_buf[3] & 0x0F) << 16) |
		((uint32_t)rx_buf[4] << 8) |
		rx_buf[5];

	float raw_temp = (((float)raw * 200 / AHT10_SCALE) - 50.0f);
	data->temp = (int16_t)(raw_temp * 100 +
			(raw_temp >= 0 ? 0.5f :  -0.5f));

	// Umidade
	raw =
		((uint32_t)rx_buf[1] << 12) |
		((uint32_t)rx_buf[2] << 4) |
		(rx_buf[3] >> 4);

	data->humi = (int16_t)(((float)raw * 100 / AHT10_SCALE) * 100.0f + 0.5f);

	return HAL_OK;
}

HAL_StatusTypeDef AHT_TriggerMeasurement(AHT_Handle_t *haht)
{
	if (haht == NULL) return HAL_ERROR;
    HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(
        haht->hi2c,
        haht->addr,
        (uint8_t*)aht10_measure_cmd,
        sizeof(aht10_measure_cmd),
		haht->timeout
    );

   if (status != HAL_OK) return status;
   return HAL_OK;
}
