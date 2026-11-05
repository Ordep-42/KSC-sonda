/*
 * telemtry_packet.h
 *
 *  Created on: 2 de nov. de 2026
 *      Author: pedro
 */

#ifndef INC_TELEMETRY_PACKET_H_
#define INC_TELEMETRY_PACKET_H_

#include <stdint.h>

#define TELEMETRY_VER 1
#define TELEMETRY_PKT_TYPE 0x01
#define TELEMETRY_MAGIC 0x5594

#define TELEMETRY_HEADER_SIZE 10
#define TELEMETRY_PAYLOAD_SIZE 36
#define TELEMETRY_FRAME_SIZE 46

typedef struct {
	uint16_t bat_volt;
	uint16_t sys_flag;
} TelemetrySystem_t;

typedef struct {
	int32_t lat_e7;
	int32_t lon_e7;
	uint16_t alt_m;
	uint16_t speed_ckt;
	uint16_t course_cdeg;
	uint8_t sat_num;
	uint8_t fix;
} TelemetryNav_t;

typedef struct {
	int16_t temp_aht;
	int16_t temp_bmp;
	uint16_t humi;
	int32_t pres;
} TelemetryAmb_t;

typedef struct {
	int16_t mag_x;
	int16_t mag_y;
	int16_t mag_z;
} TelemetryMag_t;

typedef struct {
	TelemetrySystem_t system_data;
	TelemetryNav_t nav_data;
	TelemetryAmb_t amb_data;
	TelemetryMag_t mag_data;
} TelemetryData_t;

#endif /* INC_TELEMETRY_PACKET_H_ */
