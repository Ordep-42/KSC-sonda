/*
 * gnss.c
 *
 *  Created on: 21 de mai. de 2026
 *      Author: pedro
 */

#include "gnss.h"
#include <string.h>

static inline void gnss_rx_byte(GNSS_Handle_t *hgnss, uint8_t byte);

static uint8_t gnss_validate(const char *strnmea); // Validador do checksum da sentença NMEA
static void gnss_parse(GNSS_Handle_t *hgnss, const char *strnmea); // Parser do tipo de sentença NMEA
static void gnss_parse_gga(GNSS_Handle_t *hgnss, const char *strnmea); // Parser para senteças NMEA GGA
static void gnss_parse_rmc(GNSS_Handle_t *hgnss, const char *strnmea); // Parser para senteças NMEA RMC
static void gnss_parse_vtg(GNSS_Handle_t *hgnss, const char *strnmea); // Parser para senteças NMEA VTG
//static void gnss_parse_gll(GNSS_Handle_t *hgnss, const char *strnmea); // Parser para senteças NMEA GLL


static const char *next_field(const char *strnmea); // Avança o ponteiro para o próximo campo (prox. vírgula)

/* API Pública */
void GNSS_Init(GNSS_Handle_t *hgnss, UART_HandleTypeDef *huart) {
	memset(hgnss, 0, sizeof(GNSS_Handle_t));
	hgnss->huart = huart;
	HAL_UARTEx_ReceiveToIdle_DMA(hgnss->huart, hgnss->rx.dma_buf, GNSS_DMA_BUF_SIZE);
}

void GNSS_Process(GNSS_Handle_t *hgnss) {
	if (gnss_validate(hgnss->rx.line_buf))
		gnss_parse(hgnss, hgnss->rx.line_buf);
	else hgnss->data.sentences_err++;
}

void GNSS_RxCallback(GNSS_Handle_t *hgnss, uint16_t offset) {
	static uint16_t last_offset = 0;

	if (offset < last_offset) {
		while (last_offset < GNSS_DMA_BUF_SIZE)
			gnss_rx_byte(hgnss, hgnss->rx.dma_buf[last_offset++]);

		last_offset = 0;
	}

	while (last_offset < offset)
		gnss_rx_byte(hgnss, hgnss->rx.dma_buf[last_offset++]);
}

uint8_t GNSS_IsStale(const GNSS_Handle_t *hgnss) {
	return (HAL_GetTick() - hgnss->data.last_fix_tick > GNSS_STALE_MS) ? 1 : 0;
}

/* Funções privadas */

// Valida checksum da sentença NMEA com XOR
static uint8_t gnss_validate(const char *strnmea) {
	if (*strnmea != '$') return 0;

	uint8_t calculated = 0;
	const char *prox = strnmea + 1;

	while (*prox && *prox != '*' && (prox - strnmea) < 82) calculated ^= (uint8_t)*prox++;

	if (*prox != '*') return 0;

	uint8_t hi = prox[1];
	uint8_t lo = prox[2];
	hi = (hi >= 'A') ? (hi - 'A' + 10) : (hi - '0');
	hi = (lo >= 'A') ? (lo - 'A' + 10) : (lo - '0');
	uint8_t expected = (hi << 4) | lo;

	return (calculated == expected) ? 1 : 0;
}

static inline void gnss_rx_byte(GNSS_Handle_t *hgnss, uint8_t byte) {
	if (byte == '\r')
		return;

	if (byte == '\n') {
		hgnss->rx.line_buf[hgnss->rx.rx_idx] = '\0';
		hgnss->rx.rx_idx = 0;
		hgnss->events |= GNSS_EVT_LINE_READY;

		return;
	}

	if (hgnss->rx.rx_idx < GNSS_LINE_BUF_SIZE - 1)
		hgnss->rx.line_buf[hgnss->rx.rx_idx++] = (char)byte;
}

static void gnss_parse(GNSS_Handle_t *hgnss, const char *strnmea) {
	if (!strncmp(strnmea, "$GPGGA", 6) || !strncmp(strnmea, "$GNGGA", 6)) gnss_parse_gga(hgnss, strnmea);
	else if (!strncmp(strnmea, "$GPRMC", 6) || !strncmp(strnmea, "$GNRMC", 6)) gnss_parse_rmc(hgnss, strnmea);
	else if (!strncmp(strnmea, "$GPVTG", 6) || !strncmp(strnmea, "$GNVTG", 6)) gnss_parse_vtg(hgnss, strnmea);
	//else if (!strncmp(strnmea, "$GPGLL", 6) || !strncmp(strnmea, "$GNGLL", 6)) gnss_parse_gll(hgnss, strnmea);
}

// Avança para o próximo campo na string (coloca o ponteiro para proximo caractere após a vírgula)
static const char *next_field(const char *strnmea) {
	while (*strnmea && *strnmea != ',') strnmea++;
	if (*strnmea == ',') strnmea++;
	return strnmea;
}

/* Converte string decimal para inteiro escalado sem usar float */
static int32_t parse_decimal(const char *str, uint8_t decimal_places) {
	int32_t int_part = 0;
	int32_t frac_part = 0;
	uint8_t frac_digits = 0;
	uint8_t in_frac = 0;

	/* Soma a parte inteira e parte fracionária do número em variáveis separadas */
	while (*str && *str != ',' && *str != '*') {
		if (*str == '.') {
			in_frac = 1;
		} else if (*str >= '0' && *str <= '9') {
			if (!in_frac) {
				int_part = int_part * 10 + (*str - '0');
			} else if (frac_digits < decimal_places) {
				frac_part = frac_part * 10 + (*str - '0');
				frac_digits++;
			} else if (frac_digits == decimal_places) { // Arredonda a parte fracionária caso tenha mais dígitos além do decimal_places
				if (*str >= '5' && *str <= '9') {
					frac_part++;
				}
			}
		}
		str++;
	}

	/* Completa com 0 se a string tinha menos casas do que o esperado */
	while (frac_digits < decimal_places) {
		frac_part *= 10;
		frac_digits++;
	}

	/* Monta a escala: int_par * 10^decimal_places + frac_part */
	int32_t scale = 1;
	for (uint8_t i = 0; i < decimal_places; i++) scale *= 10;

	return int_part * scale + frac_part;
}

/* Converte DDDMM.MMMM + hemisfério (formato nmea) para graus decimais * 1e7 */
static int32_t degree_e7(const char *coord, char hemisphere) {
	int32_t int_part = 0; // parte DDDMM
	int32_t frac_part = 0; // parte .MMMM
	uint8_t frac_digits = 0;
	uint8_t in_frac = 0;

	const char *str = coord;
	/* Soma a parte inteira e parte fracionária do número em variáveis separadas */
	while (*str && *str != ',' && *str != '*') {
		if (*str == '.') {
			in_frac = 1;
		} else if (*str >= '0' && *str <= '9') {
			if (!in_frac) {
				int_part = int_part * 10 + (*str - '0');
			} else if (frac_digits < 4){
				frac_part = frac_part * 10 + (*str - '0');
				frac_digits++;
			}
		}
		str++;
	}

	while (frac_digits < 4) {
		frac_part *= 10;
		frac_digits++;
	}

	int32_t degrees = int_part / 100; // DDD
	int32_t minutes_int = int_part % 100; // MM

	// Minutos completos * 10000 + fração dos minutos
	int32_t minutes_e4 = minutes_int * 10000 + frac_part;
	int32_t frac_e7 = minutes_e4 * 1000 / 60;

	int32_t result = degrees * 10000000	+ frac_e7;

	result *= (hemisphere == 'S' || hemisphere == 'W') ? -1 : 1;

	return result;
}

static void gnss_parse_gga(GNSS_Handle_t *hgnss, const char *strnmea) {
	/* Parser específico para strings GGA.
	 * Padrão: $xxGGA,hhmmss.sss,ddmm.mmmm,N,dddmm.mmmm,W,i,ii,f.f,MMM.MMM,M,MM.MMM,M,f.f,xxxx*HH
	 *
	 * Exemplo: $GPGGA,181908.00,3404.7041778,N,07044.3966270,W,4,13,1.00,495.144,M,29.200,M,0.10,0000*40
	 */

	GNSS_Data_t *data = &hgnss->data;

	strnmea = next_field(strnmea); // Pula o identificador da sentença ($xxGGA)

	// Horário UTC
	int32_t t = parse_decimal(strnmea, 0); // Pega só hhmmss como inteiro
	data->utc_hour = (uint8_t)(t / 10000);
	data->utc_min = (uint8_t)(t % 10000 / 100);
	data->utc_sec = (uint8_t)(t % 100);
	strnmea = next_field(strnmea);

	// Latitude
	const char *lat_str = strnmea; // Coordenadas
	strnmea = next_field(strnmea);
	const char ns = *strnmea; // Hemisfério Norte ou Sul
	strnmea = next_field(strnmea);
	data->lat_e7 = degree_e7(lat_str, ns);

	// Longitude
	const char *lon_str = strnmea; // Coordenadas
	strnmea = next_field(strnmea);
	const char we = *strnmea; // Hemisfério Oeste ou Leste
	strnmea = next_field(strnmea);
	data->lon_e7 = degree_e7(lon_str, we);

	// Qualidade do Fix
	data->fix = (uint8_t)(*strnmea - '0');
	strnmea = next_field(strnmea);

	// Satélites
	data->satellites = (uint8_t)parse_decimal(strnmea, 0);
	strnmea = next_field(strnmea);

	// Diluição da precisão horizontal
	data->hdop_centi = (uint8_t)parse_decimal(strnmea, 2);
	strnmea = next_field(strnmea);

	// Altitude em metros
	data->alt_m = (uint16_t)parse_decimal(strnmea, 0); // Pega só a parte inteira, não precisamos da precisão de frações de metros
	if (data->fix > 0) {
		data->last_fix_tick = HAL_GetTick();
		hgnss->events |= GNSS_EVT_FIX_VALID;
	} else {
		hgnss->events &= ~GNSS_EVT_FIX_VALID;
	}
}

static void gnss_parse_rmc(GNSS_Handle_t *hgnss, const char *strnmea) {
	/* Parser específico para strings GGA.
	 * Padrão: $xxRMC,hhmmss.sss,A,ddmm.mmmm,N,dddmm.mmmm,W,ff.ffff,dd.dd,ddMMyy,--,--,C,*HH
	 *
	 * Exemplo: $GPRMC,083559.00,A,4717.11437,N,00833.91522,E,0.004,77.52,091202,,,A*57
	 */

	GNSS_Data_t *data = &hgnss->data;
	strnmea = next_field(strnmea); // Pula o identificador da sentença ($xxRMC)

	// Horário UTC
	int32_t t = parse_decimal(strnmea, 0); // Pega só a parte inteira hhmmss
	data->utc_hour = (uint8_t)(t / 10000);
	data->utc_min = (uint8_t)(t / 10000 % 100);
	data->utc_sec = (uint8_t)(t % 100);
	strnmea = next_field(strnmea);

	// RMC status
	data->rmc_status = *strnmea;
	strnmea = next_field(strnmea);

	// Latitude e Longitude (Incluso caso leve muito tempo entre uma senteça RMC e GGA)
	const char *lat_str = strnmea; // Coordenadas
	strnmea = next_field(strnmea);
	const char ns = *strnmea; // Hemisfério Norte ou Sul
	strnmea = next_field(strnmea);
	data->lat_e7 = degree_e7(lat_str, ns);

	const char *lon_str = strnmea; // Coordenadas
	strnmea = next_field(strnmea);
	const char we = *strnmea; // Hemisfério Oeste ou Leste
	strnmea = next_field(strnmea);
	data->lon_e7 = degree_e7(lon_str, we);

	// Velocidade em nós * 100
	data->speed_centi_knot = (uint16_t)parse_decimal(strnmea, 2); // Pega até os centésimos de nós
	strnmea = next_field(strnmea);

	// Curso verdadeiro em graus * 100
	data->course_centi_deg = (uint16_t)parse_decimal(strnmea, 2);
	strnmea = next_field(strnmea);

	// Data
	int32_t date = parse_decimal(strnmea, 0);
	data->date_day = (uint8_t)(date / 10000);
	data->date_month = (uint8_t)(date / 10000 % 100);
	data->date_year = (uint8_t)(date % 100);
	strnmea = next_field(strnmea);

	// Modo RMC (NMEA 2.3+)
	if (*strnmea && *strnmea != '*') {
		data->rmc_mode = *strnmea;

		if (data->rmc_mode == 'A' || data->rmc_mode == 'D') {
			hgnss->events |= GNSS_EVT_FIX_VALID;
			data->last_fix_tick = HAL_GetTick();
		} else {
			hgnss->events &= ~GNSS_EVT_FIX_VALID;
		}
	}

	if (data->rmc_status == 'A') {
		hgnss->events |= GNSS_EVT_FIX_VALID;
		data->last_fix_tick = HAL_GetTick();
	} else {
		hgnss->events &= ~GNSS_EVT_FIX_VALID;
	}
}

static void gnss_parse_vtg(GNSS_Handle_t *hgnss, const char *strnmea) {
	/* Parser específico para strings GGA.
	 * Padrão: $xxVTG,dd.dd,T,--,M,ff.fff,N,ff.fff,K,C*HH
	 *
	 * Exemplo: $GPVTG,77.52,T,,M,0.004,N,0.008,K,A*06
	 */

	GNSS_Data_t *data = &hgnss->data;
	strnmea = next_field(strnmea); // Pula o identificador da sentença ($xxRMC)

	// Curso verdadeiro em graus * 100
	data->course_centi_deg = (uint16_t)parse_decimal(strnmea, 2);
	strnmea = next_field(strnmea);
	strnmea = next_field(strnmea); // Pula o 'T'
	strnmea = next_field(strnmea); // Pula o 'M'

	// Velocidade em nós * 100
	data->speed_centi_knot = (uint16_t)parse_decimal(strnmea, 2); // Pega até os centésimos de nós
	strnmea = next_field(strnmea);
	strnmea = next_field(strnmea); // Pula o 'N'

	// Velocidade em Km/h * 100
	data->speed_centi_kph = (uint16_t)parse_decimal(strnmea, 2); // Pega até os centésimos de nós
	strnmea = next_field(strnmea);
	strnmea = next_field(strnmea); // Pula o 'K'

	// Modo VTG (NMEA 2.3+)
	if (*strnmea && *strnmea != '*') {
		data->rmc_mode = *strnmea;

		if (data->rmc_mode == 'A' || data->rmc_mode == 'D') {
			hgnss->events |= GNSS_EVT_FIX_VALID;
			data->last_fix_tick = HAL_GetTick();
		} else {
			hgnss->events &= ~GNSS_EVT_FIX_VALID;
		}
	}
}

