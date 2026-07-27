/*
 * ebyte.c
 *
 *  Created on: 25 de jul. de 2026
 *      Author: pedro
 */

#include "ebyte.h"
#include "usart.h"
#include <string.h>

static inline void radio_wake_up(EBYTE_Handle_t *dev) {
	HAL_GPIO_WritePin(dev->m0_gpio_port, dev->m0_pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(dev->m1_gpio_port, dev->m1_pin, GPIO_PIN_RESET);
}

static inline void radio_sleep(EBYTE_Handle_t *dev) {
	HAL_GPIO_WritePin(dev->m0_gpio_port, dev->m0_pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(dev->m1_gpio_port, dev->m1_pin, GPIO_PIN_SET);
}

static EBYTE_Status_t wait_aux(EBYTE_Handle_t *dev) {
	uint32_t tickstart = HAL_GetTick();

	while (HAL_GPIO_ReadPin(dev->aux_gpio_port, dev->aux_pin) == GPIO_PIN_RESET) {
		if ((HAL_GetTick() - tickstart) >= dev->radio_timeout)
			return EBYTE_TIMEOUT;
	}
	HAL_Delay(EBYTE_AUX_WAIT_MS);

	return EBYTE_OK;
}

EBYTE_Status_t EBYTE_Init(EBYTE_Handle_t *dev) {
	if (dev == NULL || dev->huart == NULL)
		return EBYTE_ERROR_NULL_PTR;

	return EBYTE_WakeUp(dev);
}

EBYTE_Status_t EBYTE_WakeUp(EBYTE_Handle_t *dev) {
	if (dev == NULL)
		return EBYTE_ERROR_NULL_PTR;
	radio_wake_up(dev);
	return wait_aux(dev);
}

EBYTE_Status_t EBYTE_Sleep(EBYTE_Handle_t *dev) {
	if (dev == NULL)
		return EBYTE_ERROR_NULL_PTR;
	radio_sleep(dev);
	return wait_aux(dev);
}

EBYTE_Status_t EBYTE_Transmit(EBYTE_Handle_t *dev, const uint16_t target_addr, const uint8_t target_chan, const uint8_t *data, uint16_t size) {
	if (dev == NULL || dev->huart == NULL || data == NULL)
		return EBYTE_ERROR_NULL_PTR;

	if (size == 0 || size > (EBYTE_PACKET_MAX_LEN-3))
	    return EBYTE_ERROR_INVALID_PARAM;

	if (wait_aux(dev) != EBYTE_OK)
		return EBYTE_TIMEOUT;

	uint8_t tx_buf[3 + size];
	tx_buf[0] = (uint8_t)((target_addr >> 8) & 0xFF);
	tx_buf[1] = (uint8_t)(target_addr & 0xFF);
	tx_buf[2] = target_chan;
	memcpy(tx_buf + 3, data, size);

	HAL_StatusTypeDef status;
	status = HAL_UART_Transmit(dev->huart, tx_buf, 3+size, dev->uart_timeout);
	if (status != HAL_OK) {
	    if (status == HAL_BUSY) return EBYTE_BUSY;

	    return EBYTE_ERROR_HARDWARE;
	}

	return wait_aux(dev);
}

