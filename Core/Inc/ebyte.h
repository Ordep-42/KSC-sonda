/*
 * ebyte.h
 *
 *  Created on: 22 de jul. de 2026
 *      Author: pedro
 */

#ifndef INC_EBYTE_H_
#define INC_EBYTE_H_

#include "stm32f1xx_hal.h"
#include "ebyte_conf.h"

typedef enum {
	EBYTE_OK = 0,

	EBYTE_ERROR_NULL_PTR,
	EBYTE_ERROR_INVALID_PARAM,
	EBYTE_ERROR_HARDWARE,

	EBYTE_BUSY,
	EBYTE_TIMEOUT
} EBYTE_Status_t;

typedef struct {
	UART_HandleTypeDef *huart;
	uint32_t radio_timeout;
	uint32_t uart_timeout;

	GPIO_TypeDef *m0_gpio_port;
	uint16_t m0_pin;
	GPIO_TypeDef *m1_gpio_port;
	uint16_t m1_pin;

	GPIO_TypeDef *aux_gpio_port;
	uint16_t aux_pin;
} EBYTE_Handle_t;

EBYTE_Status_t EBYTE_Init(EBYTE_Handle_t *dev);
EBYTE_Status_t EBYTE_WakeUp(EBYTE_Handle_t *dev);
EBYTE_Status_t EBYTE_Sleep(EBYTE_Handle_t *dev);
EBYTE_Status_t EBYTE_Transmit(EBYTE_Handle_t *dev, const uint16_t target_addr, const uint8_t target_chan, const uint8_t *data, uint16_t size);

#endif /* INC_EBYTE_H_ */
