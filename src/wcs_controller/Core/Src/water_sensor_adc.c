#include "water_sensor_adc.h"
#include "main.h"

extern ADC_HandleTypeDef hadc3;

bool WaterSensorAdc_Read(uint16_t *raw)
{
  if (raw == 0)
  {
    return false;
  }
  if (HAL_ADC_Start(&hadc3) != HAL_OK)
  {
    return false;
  }
  if (HAL_ADC_PollForConversion(&hadc3, 10U) != HAL_OK)
  {
    (void)HAL_ADC_Stop(&hadc3);
    return false;
  }
  *raw = (uint16_t)HAL_ADC_GetValue(&hadc3);
  (void)HAL_ADC_Stop(&hadc3);
  return true;
}
