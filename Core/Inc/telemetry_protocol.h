/*
 * telemetry_protocol.h
 *
 *  Created on: 2 de nov. de 2026
 *      Author: pedro
 */

#ifndef INC_TELEMETRY_PROTOCOL_H_
#define INC_TELEMETRY_PROTOCOL_H_

#include <stdint.h>
#include "telemetry_packet.h"

#define TELEMETRY_OK       0
#define TELEMETRY_ERROR    1

uint8_t telemetry_serialize(
    const TelemetryData_t *data,
    uint16_t sequence,
    uint32_t uptime,
    uint8_t *buffer,
    uint16_t buffer_size
);


#endif /* INC_TELEMETRY_PROTOCOL_H_ */
