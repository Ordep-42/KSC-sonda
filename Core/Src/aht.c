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

void AHT_Init(AHT_Handle_t *haht, I2C_HandleTypeDef *hi2c, uint8_t dev_addr, TIM_HandleTypeDef *htim) {
	haht->hi2c = hi2c;
	haht->addr = dev_addr;
	haht->state = AHT_IDLE;
	haht->events = 0;
	haht->htim = htim;
	memset(haht->rx_buf, 0, sizeof(haht->rx_buf));
}

HAL_StatusTypeDef AHT_ReadData(AHT_Handle_t *haht, AHT_Data_t *data) {
	uint32_t raw;

	if (haht == NULL || data == NULL) return HAL_ERROR;

	if (!(haht->events & AHT_EVT_DREADY)) return HAL_BUSY;

	haht->events &= ~AHT_EVT_DREADY; // Limpa os eventos

	if (haht->rx_buf[0] & AHT10_BUSY_BIT) return HAL_BUSY; // Verifica o bit busy do sensor

	// Temperatura
	raw =
		(((uint32_t)haht->rx_buf[3] & 0x0F) << 16) |
		((uint32_t) haht->rx_buf[4] << 8) |
		haht->rx_buf[5];

	float raw_temp = (((float)raw * 200 / AHT10_SCALE) - 50.0f);
	data->temp = (int16_t)(raw_temp * 100 +
			(raw_temp >= 0 ? 0.5f :  -0.5f));

	// Umidade
	raw =
		((uint32_t)haht->rx_buf[1] << 12) |
		((uint32_t) haht->rx_buf[2] << 4) |
		(haht->rx_buf[3] >> 4);

	data->humi = (int16_t)(((float)raw * 100 / AHT10_SCALE) * 100.0f + 0.5f);

	return HAL_OK;
}

HAL_StatusTypeDef AHT_TriggerMeasurement(AHT_Handle_t *haht)
{
    return HAL_I2C_Master_Transmit_IT(
        haht->hi2c,
        haht->addr,
        (uint8_t*)aht10_measure_cmd,
        sizeof(aht10_measure_cmd)
    );
}
