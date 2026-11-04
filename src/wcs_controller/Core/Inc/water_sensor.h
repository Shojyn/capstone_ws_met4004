#ifndef WATER_SENSOR_H
#define WATER_SENSOR_H

#include <stdbool.h>
#include <stdint.h>

typedef bool (*WaterSensorReadFunction)(uint16_t *raw);

typedef enum
{
  WATER_SENSOR_DRY,
  WATER_SENSOR_WET
} WaterSensorState;

void WaterSensor_Init(WaterSensorReadFunction read, uint32_t now);
void WaterSensor_Update(uint32_t now);
void WaterSensor_ProcessSample(uint16_t raw, uint32_t now);
WaterSensorState WaterSensor_GetState(void);
bool WaterSensor_IsDetected(void);
uint16_t WaterSensor_GetRaw(void);
uint16_t WaterSensor_GetFilteredRaw(void);
uint16_t WaterSensor_GetMillivolts(void);
uint8_t WaterSensor_GetSignalPercent(void);

#endif
