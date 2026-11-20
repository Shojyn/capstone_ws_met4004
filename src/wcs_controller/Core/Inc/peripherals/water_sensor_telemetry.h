#ifndef WATER_SENSOR_TELEMETRY_H
#define WATER_SENSOR_TELEMETRY_H

#include <stdint.h>

void WaterSensorTelemetry_Init(uint32_t now);
void WaterSensorTelemetry_Update(uint32_t now);

#endif
