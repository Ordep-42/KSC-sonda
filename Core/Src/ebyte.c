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

EBYTE_Status_t EBYTE_Init(EBYTE_Handle_t *dev) {
	if (dev == NULL || dev->huart == NULL)
		return EBYTE_ERROR_NULL_PTR;

	radio_sleep(dev);
	dev->state = EBYTE_IDLE;
	return EBYTE_OK;
}

EBYTE_Status_t EBYTE_WakeUp(EBYTE_Handle_t *dev) {
	if (dev == NULL)
		return EBYTE_ERROR_NULL_PTR;

	if(dev->state != EBYTE_SLEEP)
		return EBYTE_BUSY;

	radio_wake_up(dev);
	dev->state = EBYTE_WAIT_WAKE;
	return EBYTE_OK;
}

EBYTE_Status_t EBYTE_Sleep(EBYTE_Handle_t *dev) {
	if (dev == NULL)
		return EBYTE_ERROR_NULL_PTR;

	if(dev->state != EBYTE_IDLE)
		return EBYTE_BUSY;

	radio_sleep(dev);
	dev->state = EBYTE_WAIT_SLEEP;
	return EBYTE_OK;
}

EBYTE_Status_t EBYTE_Transmit(EBYTE_Handle_t *dev, const uint16_t target_addr, const uint8_t target_chan, const uint8_t *data, uint16_t size) {
	if (dev == NULL || dev->huart == NULL || data == NULL)
		return EBYTE_ERROR_NULL_PTR;

	if (size == 0 || size > (EBYTE_PACKET_MAX_LEN-3))
	    return EBYTE_ERROR_INVALID_PARAM;

	if (dev->state != EBYTE_IDLE)
		return EBYTE_BUSY;

	dev->tx_buffer[0] = (uint8_t)(target_addr >> 8);
	dev->tx_buffer[1] = (uint8_t)(target_addr);
	dev->tx_buffer[2] = target_chan;
	memcpy(&dev->tx_buffer[3], data, size);
	dev->tx_len = size + 3;

	HAL_StatusTypeDef status;
	status = HAL_UART_Transmit_IT(dev->huart, dev->tx_buffer, dev->tx_len);
	if (status != HAL_OK) {
	    if (status == HAL_BUSY) return EBYTE_BUSY;

	    return EBYTE_ERROR_HARDWARE;
	}
	dev->state = EBYTE_WAIT_UART;
	return EBYTE_OK;
}

void EBYTE_TxCpltCallback(EBYTE_Handle_t *dev, UART_HandleTypeDef *huart) {
	if (huart != dev->huart) return;

	if (dev->state == EBYTE_WAIT_UART)
		dev->state = EBYTE_WAIT_AUX;
}

void EBYTE_AuxCallback(EBYTE_Handle_t *dev) {
    switch(dev->state) {
    	case EBYTE_WAIT_WAKE:
			dev->state = EBYTE_IDLE;
			break;

		case EBYTE_WAIT_AUX:
			dev->state = EBYTE_IDLE;
			break;

		case EBYTE_WAIT_SLEEP:
			dev->state = EBYTE_SLEEP;
			break;

        default:
            break;
    }
}
