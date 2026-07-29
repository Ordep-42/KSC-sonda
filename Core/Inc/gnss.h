/*
 * gnss.h
 *
 *  Created on: 21 de mai. de 2026
 *      Author: pedro
 */

#ifndef INC_GNSS_H_
#define INC_GNSS_H_

#include "main.h"

#define GNSS_DMA_BUF_SIZE   64U
#define GNSS_LINE_BUF_SIZE  128U
#define GNSS_STALE_MS       2000U

/* Eventos do driver */
typedef enum {
    GNSS_EVT_NONE        = 0x00,
    GNSS_EVT_LINE_READY  = 0x01,
	GNSS_EVT_FIX_VALID   = 0x02,
	GNSS_EVT_STALE_DATA  = 0x03,
	GNSS_EVT_PARSE_ERROR = 0x04,
} GNSS_Event_t;

/* Camada IRQ/DMA */
typedef struct {
	uint8_t dma_buf[GNSS_DMA_BUF_SIZE];      // Buffer circular do DMA
	char line_buf[GNSS_LINE_BUF_SIZE];       // Sentença NMEA completa
	uint16_t rx_idx;
} GNSS_RxBuffer_t;

/* Camada de dados - contexto da main */
typedef struct {
	/* GGA - Global Positioning System Fixed Data */
	int32_t lat_e7; // Latitude em graus * 1e7
	int32_t lon_e7; // Longitude em graus * 1e7
    uint8_t utc_hour;
    uint8_t utc_min;
    uint8_t utc_sec;
    uint8_t fix; // Qualidade de triangulação de posição
    uint8_t satellites; // Quantidade de satélites conectados
    uint8_t hdop_centi; // Diluição posicional em centésimos
	uint16_t alt_m; // Altitude em metros

    /* RMC - Recommended Minimmum Specific GNS Data */
    uint16_t speed_centi_knot; // Velocidade em centésimos de nó
    uint16_t course_centi_deg; // Curso verdadeiro em centésimos de graus
    uint8_t date_day;
    uint8_t date_month;
    uint8_t date_year;

	/* VTG - Course over ground, ground speed */
    uint16_t speed_centi_kph;

    char status; // Status dos dados do GPS
    char pos_mode; // Modo de posicionamento

    /* Estado do driver */
    uint32_t last_fix_tick; // HAL_GetTick() do último fix válido
    uint16_t sentences_err; // Contador de sentenças com erro no checksum
} GNSS_Data_t;

/* Handle para API pública */
typedef struct {
	UART_HandleTypeDef *huart; // Handle para o periférico UART usado pelo módulo GNSS

	GNSS_RxBuffer_t rx; // Struct do buffer
	GNSS_Data_t data; 	// Struct de dados

	volatile uint8_t events;
} GNSS_Handle_t;

/**
 * @brief Inicializa o driver GNSS, zera o handle e arma a primeira recepção por DMA.
 * @param hgnss Ponteiro para o handle do driver.
 * @param huart Ponteiro para o handle HAL do UART conectado ao módulo GNSS.
 */
void GNSS_Init(GNSS_Handle_t *hgnss, UART_HandleTypeDef *huart);

/**
 * @brief Processa uma linha NMEA pendente. Deve ser chamada no loop principal ou em task dedicada, nunca em IRQ.
 * @param hgnss Ponteiro para o handle do driver.
 */
void GNSS_Process(GNSS_Handle_t *hgnss);

void GNSS_RxCallback(GNSS_Handle_t *hgnss, uint16_t offset);

/**
 * @brief Retorna 1 se o último fix válido é mais antigo que GNSS_STALE_MS, 0 caso contrário.
 * @param hgnss Ponteiro constante para o handle do driver.
 */
uint8_t GNSS_IsStale(const GNSS_Handle_t *hgnss);

#endif /* INC_GNSS_H_ */
