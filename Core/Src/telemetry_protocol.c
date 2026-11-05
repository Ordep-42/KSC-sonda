/*
 * telemetry_protocol.c
 *
 *  Created on: 2 de nov. de 2026
 *      Author: pedro
 */
#include "telemetry_protocol.h"
#include "telemetry_packet.h"

#include <stddef.h>
#include <stdint.h>

static void write_u16_le(uint8_t *buffer, uint16_t value) {
    buffer[0] = (uint8_t)(value & 0xFF);
    buffer[1] = (uint8_t)((value >> 8) & 0xFF);
}

static void write_u32_le(uint8_t *buffer, uint32_t value) {
    buffer[0] = (uint8_t)(value & 0xFF);
    buffer[1] = (uint8_t)((value >> 8) & 0xFF);
    buffer[2] = (uint8_t)((value >> 16) & 0xFF);
    buffer[3] = (uint8_t)((value >> 24) & 0xFF);
}

uint8_t telemetry_serialize(const TelemetryData_t *data,
							uint16_t sequence,
							uint8_t *buffer,
							uint16_t buffer_size) {

	uint16_t offset = 0;

	if (data == NULL) return TELEMETRY_ERROR;
	if (buffer_size < TELEMETRY_FRAME_SIZE) return TELEMETRY_ERROR;

	// Header
	write_u16_le(&buffer[offset], TELEMETRY_MAGIC);
	offset += 2;
	buffer[offset++] = TELEMETRY_VER;
	buffer[offset++] = TELEMETRY_PKT_TYPE;
	write_u16_le(&buffer[offset], sequence);
	offset += 2;

	// Sistema
	write_u16_le(&buffer[offset], data->system_data.bat_volt);
	offset += 2;
	write_u16_le(&buffer[offset], data->system_data.sys_flag);
	offset += 2;

	// Nav
	write_u32_le(&buffer[offset], (uint32_t)data->nav_data.lat_e7);
    offset += 4;
    write_u32_le(&buffer[offset], (uint32_t)data->nav_data.lon_e7);
    offset += 4;
    write_u16_le(&buffer[offset], (uint16_t)data->nav_data.alt_m);
    offset += 2;
    write_u16_le(&buffer[offset], data->nav_data.speed_ckt);
    offset += 2;
    write_u16_le(&buffer[offset], data->nav_data.course_cdeg);
    offset += 2;
    buffer[offset++] = data->nav_data.sat_num;
    buffer[offset++] = data->nav_data.fix;

    // Dados Ambientais
    write_u16_le(&buffer[offset], (uint16_t)data->amb_data.temp_aht);
    offset += 2;
    write_u16_le(&buffer[offset], (uint16_t)data->amb_data.temp_bmp);
    offset += 2;
    write_u16_le(&buffer[offset], data->amb_data.humi);
    offset += 2;
    write_u32_le(&buffer[offset], data->amb_data.pres);
    offset += 4;

    // Dados Magnéticos
    write_u16_le(&buffer[offset], (uint16_t)data->mag_data.mag_x);
    offset += 2;
    write_u16_le(&buffer[offset], (uint16_t)data->mag_data.mag_y);
    offset += 2;
    write_u16_le(&buffer[offset], (uint16_t)data->mag_data.mag_z);
    offset += 2;

    return (offset == TELEMETRY_FRAME_SIZE) ?
                TELEMETRY_OK : TELEMETRY_ERROR;
}

