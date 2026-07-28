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


typedef enum {
	EBYTE_IDLE,
	EBYTE_SLEEP,
	EBYTE_WAIT_WAKE,
	EBYTE_WAIT_SLEEP,
	EBYTE_WAIT_UART,
	EBYTE_WAIT_AUX
} EBYTE_State_t;

typedef struct {
	UART_HandleTypeDef *huart;
	uint32_t radio_timeout;
	volatile EBYTE_State_t state;

	uint8_t tx_buffer[EBYTE_BUFFER_SIZE];
	uint16_t tx_len;

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
void EBYTE_TxCpltCallback(EBYTE_Handle_t *dev, UART_HandleTypeDef *huart);
void EBYTE_AuxCallback(EBYTE_Handle_t *dev);

#endif /* INC_EBYTE_H_ */
