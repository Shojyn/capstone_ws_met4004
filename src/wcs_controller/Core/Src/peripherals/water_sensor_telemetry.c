#include "peripherals/water_sensor_telemetry.h"
#include "SEGGER_RTT.h"
#include "peripherals/water_sensor.h"

#define WATER_SENSOR_TELEMETRY_INTERVAL_MS 100U

static uint32_t last_telemetry_at;

void WaterSensorTelemetry_Init(uint32_t now)
{
  last_telemetry_at = now - WATER_SENSOR_TELEMETRY_INTERVAL_MS;
  SEGGER_RTT_WriteString(0U, "water_ms,raw,filtered_raw,millivolts,signal_percent,state\r\n");
}

void WaterSensorTelemetry_Update(uint32_t now)
{
  if ((uint32_t)(now - last_telemetry_at) < WATER_SENSOR_TELEMETRY_INTERVAL_MS)
  {
    return;
  }
  last_telemetry_at = now;
  SEGGER_RTT_printf(0U, "%lu,%u,%u,%u,%u,%s\r\n",
                    (unsigned long)now,
                    (unsigned int)WaterSensor_GetRaw(),
                    (unsigned int)WaterSensor_GetFilteredRaw(),
                    (unsigned int)WaterSensor_GetMillivolts(),
                    (unsigned int)WaterSensor_GetSignalPercent(),
                    WaterSensor_IsDetected() ? "WET" : "DRY");
}
