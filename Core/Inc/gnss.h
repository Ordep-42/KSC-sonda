/*
 * gnss.h
 *
 *  Created on: 21 de mai. de 2026
 *      Author: pedro
 */

#ifndef INC_GNSS_H_
#define INC_GNSS_H_

#include "main.h"

#define GNSS_LINE_BUF_SIZE 128U
#define GNSS_STALE_MS 2000U


/* Camada IRQ - contexto da interrupção de recepção */
typedef struct {
	uint8_t rx_byte; // Byte recebido pela UART
	char rx_buf[GNSS_LINE_BUF_SIZE]; // Acumulador de bytes
	uint8_t rx_idx;
	volatile uint8_t line_ready; // Flag IRQ para main
	char line_buf[GNSS_LINE_BUF_SIZE]; // Copia segura para parse
	uint32_t overflow_count;
} GNSS_RxBuffer_t;

/* Máquina de estado do driver do GNSS */
typedef enum {
	GNSS_UNINIT = 0,
    GNSS_IDLE,
    GNSS_RECEIVING,
    GNSS_NO_FIX,
    GNSS_FIX_VALID,
    GNSS_ERROR
} GNSS_State_t;

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
    char rmc_status; // Status dos dados do RMC
    uint16_t speed_centi_knot; // Velocidade em centésimos de nó
    uint16_t course_centi_deg; // Curso verdadeiro em centésimos de graus
    uint8_t date_day;
    uint8_t date_month;
    uint8_t date_year;
    char rmc_mode; // Modo do RMC

	/* VTG - Course over ground, ground speed */
    uint16_t speed_centi_kph;
    uint8_t  vtg_mode;

    /* Estado do driver */
    GNSS_State_t state;
    uint32_t last_fix_tick; // HAL_GetTick() do último fix válido
    uint16_t sentences_err; // Contador de sentenças com erro no checksum
} GNSS_Data_t;

/* Handle para API pública */
typedef struct {
	UART_HandleTypeDef *huart; // Handle para o periférico UART usado pelo módulo GNSS
	GNSS_RxBuffer_t rx; // Struct do buffer
	GNSS_Data_t data; 	// Struct de dados
} GNSS_Handle_t;

/**
 * @brief Inicializa o driver GNSS, zera o handle e arma a primeira recepção por IRQ.
 * @param hgnss Ponteiro para o handle do driver.
 * @param huart Ponteiro para o handle HAL do UART conectado ao módulo GNSS.
 */
void GNSS_Init(GNSS_Handle_t *hgnss, UART_HandleTypeDef *huart);

/**
 * @brief Processa uma linha NMEA pendente. Deve ser chamada no loop principal ou em task dedicada, nunca em IRQ.
 * @param hgnss Ponteiro para o handle do driver.
 */
void GNSS_Process(GNSS_Handle_t *hgnss);

/**
 * @brief Acumula o byte recebido, detecta fim de linha e sinaliza GNSS_Process(). Deve ser chamada dentro do HAL_UART_RxCpltCallback.
 * @param hgnss Ponteiro para o handle do driver.
 */
void GNSS_RxCallback(GNSS_Handle_t *hgnss);

/**
 * @brief Retorna 1 se o fix atual é válido (fix_quality > 0 e satellites >= 4), 0 caso contrário.
 * @param hgnss Ponteiro constante para o handle do driver.
 */
uint8_t GNSS_IsFixValid(const GNSS_Handle_t *hgnss);

/**
 * @brief Retorna 1 se o último fix válido é mais antigo que GNSS_STALE_MS, 0 caso contrário.
 * @param hgnss Ponteiro constante para o handle do driver.
 */
uint8_t GNSS_IsStale(const GNSS_Handle_t *hgnss);

/**
 * @brief Retorna o estado atual do driver.
 * @param hgnss Ponteiro constante para o handle do driver.
 * @return      Valor do enum GNSS_State_t correspondente ao estado corrente.
 */
GNSS_State_t GNSS_GetState(const GNSS_Handle_t *hgnss);

#endif /* INC_GNSS_H_ */
