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
	EBYTE_AIR_DATA_RATE_2K4,
	EBYTE_AIR_DATA_RATE_4K8,
	EBYTE_AIR_DATA_RATE_9K6,
	EBYTE_AIR_DATA_RATE_19K2,
	EBYTE_AIR_DATA_RATE_38K4,
	EBYTE_AIR_DATA_RATE_62K5
} EBYTE_AirDataRate_t;

typedef enum {
	EBYTE_TX_POWER_MAX,
	EBYTE_TX_POWER_HIGH,
	EBYTE_TX_POWER_MEDIUM,
	EBYTE_TX_POWER_LOW
} EBYTE_TxPower_t;

typedef enum {
	EBYTE_SUB_PACKET_MAX,  // E220-xxxTxxD = 200; E22-xxxTxxD = 240;
	EBYTE_SUB_PACKET_128,
	EBYTE_SUB_PACKET_64,
	EBYTE_SUB_PACKET_32
} EBYTE_SubPacketLen_t;

typedef struct {
	UART_HandleTypeDef *huart;
	uint32_t timeout;
	volatile uint8_t events;

	GPIO_TypeDef *m_gpio_port;
	uint16_t m0_pin;
	uint16_t m1_pin;
	GPIO_TypeDef *aux_gpio_port;
	uint16_t aux_pin;
} EBYTE_Device_t;

typedef struct {
	uint16_t address;
	uint16_t crypt_key;

	EBYTE_AirDataRate_t  air_data_rate;
	EBYTE_TxPower_t      tx_power;
	EBYTE_SubPacketLen_t sub_packet_len;

	uint8_t channel;
} EBYTE_Config_t;

#endif /* INC_EBYTE_H_ */
