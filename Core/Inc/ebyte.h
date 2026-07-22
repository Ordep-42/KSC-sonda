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

typedef enum {
    EBYTE_TX_MODE_TRANSPARENT = 0,
    EBYTE_TX_MODE_FIXED       = 1
} EBYTE_TxMode_t;

typedef struct {
	UART_HandleTypeDef *huart;
	uint32_t timeout;
	volatile uint8_t events;

	GPIO_TypeDef *m_gpio_port;
	uint16_t m0_pin;
	uint16_t m1_pin;
	GPIO_TypeDef *aux_gpio_port;
	uint16_t aux_pin;

	EBYTE_TxMode_t tx_mode;
} EBYTE_Handle_t;

typedef struct {
	uint16_t address;
	uint16_t crypt_key;

	EBYTE_AirDataRate_t  air_data_rate;
	EBYTE_TxPower_t      tx_power;
	EBYTE_SubPacketLen_t sub_packet_len;
	EBYTE_TxMode_t tx_mode;

	uint8_t channel;
} EBYTE_Config_t;

EBYTE_Status_t EBYTE_Init(EBYTE_Handle_t *dev, const EBYTE_Config_t *cfg);
EBYTE_Status_t EBYTE_Configure(EBYTE_Handle_t *dev, const EBYTE_Config_t *cfg);
EBYTE_Status_t EBYTE_ReadCurrentConfig(EBYTE_Handle_t *dev, EBYTE_Config_t *out_cfg);
EBYTE_Status_t EBYTE_ReadAmbientNoise(EBYTE_Handle_t *dev, int16_t *out_dbm);
EBYTE_Status_t EBYTE_Transmit(EBYTE_Handle_t *dev, const uint8_t *data, uint16_t size);
EBYTE_Status_t EBYTE_TransmitFixed(EBYTE_Handle_t *dev, const uint16_t target_addr, const uint8_t target_chan, const uint8_t *data, uint16_t size);
EBYTE_Status_t EBYTE_Reset(EBYTE_Handle_t *dev);
EBYTE_Status_t EBYTE_DeInit(EBYTE_Handle_t *dev);

#endif /* INC_EBYTE_H_ */
