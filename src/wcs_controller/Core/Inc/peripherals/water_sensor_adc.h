#ifndef WATER_SENSOR_ADC_H
#define WATER_SENSOR_ADC_H

#include <stdbool.h>
#include <stdint.h>

bool WaterSensorAdc_Read(uint16_t *raw);

#endif
