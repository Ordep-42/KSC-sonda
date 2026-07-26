/*
 * ebyte.c
 *
 *  Created on: 25 de jul. de 2026
 *      Author: pedro
 */

#include "ebyte.h"
#include "usart.h"

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

	return EBYTE_OK;
}

EBYTE_Status_t EBYTE_Init(EBYTE_Handle_t *dev) {
	if (dev == NULL || dev->huart = NULL)
		return EBYTE_ERROR_NULL_PTR;

	radio_wake_up(dev);
	return wait_aux(dev);
}

EBYTE_Status_t EBYTE_WakeUp(EBYTE_Handle_t *dev) {
	radio_wake_up(dev);
	return wait_aux(dev);
}

EBYTE_Status_t EBYTE_Sleep(EBYTE_Handle_t *dev) {
	radio_sleep(dev);
	return EBYTE_OK;
}

EBYTE_Status_t EBYTE_Transmit();

