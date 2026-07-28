/*
 * ebyte_config.h
 *
 *  Created on: 22 de jul. de 2026
 *      Author: pedro
 */

#ifndef INC_EBYTE_CONF_H_
#define INC_EBYTE_CONF_H_

// Habilite APENAS UM dos modelos definindo-o como 1 e o outro como 0.
#define EBYTE_MODEL_E22   0  // Define 1 se estiver usando a família E22 (SX1262)
#define EBYTE_MODEL_E220  1  // Define 1 se estiver usando a família E220 (LLCC68)

#define EBYTE_AUX_WAIT_MS 3  // 3ms de espera após o pino AUX subir
#define EBYTE_BUFFER_SIZE 96

#if (!EBYTE_MODEL_E22 && !EBYTE_MODEL_E220) || (EBYTE_MODEL_E22 && EBYTE_MODEL_E220)
	#error "You must define exactly one EBYTE module target (E22 or E220)!"
#endif

#if EBYTE_MODEL_E220 // Configurações da família E220
	#define EBYTE_CHANNEL_MAX 80
	#define EBYTE_PACKET_MAX_LEN 200
	#define EBYTE_RF_BUFFER_SIZE 400
	#define EBYTE_CONFIG_REG_OFFSET 0 // Registrador NETID não existe, REG0 continua no 0x03

#elif EBYTE_MODEL_E22 // Configurações da família E22
	#define EBYTE_CHANNEL_MAX 83
	#define EBYTE_PACKET_MAX_LEN 240
	#define EBYTE_RF_BUFFER_SIZE 1000
	#define EBYTE_CONFIG_REG_OFFSET 1  // Registrador NETID existe, a partir do REG0 o endereço soma 1

#endif

#define IS_EBYTE_CHANNEL_VALID(ch)  ((ch) <= EBYTE_CHANNEL_MAX)

#endif /* INC_EBYTE_CONF_H_ */
