#include "peripherals/water_sensor.h"

#define WATER_SENSOR_SAMPLE_INTERVAL_MS 100U
#define WATER_SENSOR_AVERAGE_SAMPLES 4U
#define WATER_SENSOR_CONFIRM_SAMPLES 2U
#define WATER_SENSOR_DETECT_MV 400U
#define WATER_SENSOR_CLEAR_MV 200U
#define WATER_SENSOR_FULL_SCALE_MV 1700U
#define WATER_SENSOR_REFERENCE_MV 3300U
#define WATER_SENSOR_ADC_MAX 4095U

static WaterSensorReadFunction read_sample;
static uint32_t last_sample_at;
static uint16_t samples[WATER_SENSOR_AVERAGE_SAMPLES];
static uint32_t sample_sum;
static uint8_t sample_count;
static uint8_t sample_index;
static uint16_t raw_value;
static uint16_t filtered_raw;
static uint16_t millivolts;
static uint8_t signal_percent;
static WaterSensorState state;
static WaterSensorState candidate_state;
static uint8_t candidate_count;

static uint16_t WaterSensor_RawToMillivolts(uint16_t raw)
{
  return (uint16_t)(((uint32_t)raw * WATER_SENSOR_REFERENCE_MV) /
                    WATER_SENSOR_ADC_MAX);
}

static uint8_t WaterSensor_CalculateSignalPercent(uint16_t mv)
{
  if (mv >= WATER_SENSOR_FULL_SCALE_MV)
  {
    return 100U;
  }
  return (uint8_t)(((uint32_t)mv * 100U) / WATER_SENSOR_FULL_SCALE_MV);
}

static void WaterSensor_UpdateState(void)
{
  WaterSensorState requested = state;
  if (state == WATER_SENSOR_DRY && millivolts >= WATER_SENSOR_DETECT_MV)
  {
    requested = WATER_SENSOR_WET;
  }
  else if (state == WATER_SENSOR_WET && millivolts <= WATER_SENSOR_CLEAR_MV)
  {
    requested = WATER_SENSOR_DRY;
  }

  if (requested == state)
  {
    candidate_count = 0U;
    return;
  }
  if (requested != candidate_state)
  {
    candidate_state = requested;
    candidate_count = 0U;
  }
  if (++candidate_count >= WATER_SENSOR_CONFIRM_SAMPLES)
  {
    state = candidate_state;
    candidate_count = 0U;
  }
}

void WaterSensor_Init(WaterSensorReadFunction read, uint32_t now)
{
  read_sample = read;
  last_sample_at = now - WATER_SENSOR_SAMPLE_INTERVAL_MS;
  sample_sum = 0U;
  sample_count = 0U;
  sample_index = 0U;
  raw_value = 0U;
  filtered_raw = 0U;
  millivolts = 0U;
  signal_percent = 0U;
  state = WATER_SENSOR_DRY;
  candidate_state = WATER_SENSOR_DRY;
  candidate_count = 0U;
  for (uint32_t i = 0U; i < WATER_SENSOR_AVERAGE_SAMPLES; ++i)
  {
    samples[i] = 0U;
  }
}

void WaterSensor_Update(uint32_t now)
{
  if (read_sample == 0 || (uint32_t)(now - last_sample_at) < WATER_SENSOR_SAMPLE_INTERVAL_MS)
  {
    return;
  }
  last_sample_at = now;
  uint16_t raw;
  if (read_sample(&raw))
  {
    WaterSensor_ProcessSample(raw, now);
  }
}

void WaterSensor_ProcessSample(uint16_t raw, uint32_t now)
{
  (void)now;
  raw_value = raw;
  if (sample_count < WATER_SENSOR_AVERAGE_SAMPLES)
  {
    samples[sample_index] = raw;
    sample_sum += raw;
    ++sample_count;
  }
  else
  {
    sample_sum -= samples[sample_index];
    samples[sample_index] = raw;
    sample_sum += raw;
  }
  sample_index = (uint8_t)((sample_index + 1U) % WATER_SENSOR_AVERAGE_SAMPLES);
  filtered_raw = (uint16_t)(sample_sum / sample_count);
  millivolts = WaterSensor_RawToMillivolts(filtered_raw);
  signal_percent = WaterSensor_CalculateSignalPercent(millivolts);
  WaterSensor_UpdateState();
}

WaterSensorState WaterSensor_GetState(void)
{
  return state;
}

bool WaterSensor_IsDetected(void)
{
  return state == WATER_SENSOR_WET;
}

uint16_t WaterSensor_GetRaw(void)
{
  return raw_value;
}

uint16_t WaterSensor_GetFilteredRaw(void)
{
  return filtered_raw;
}

uint16_t WaterSensor_GetMillivolts(void)
{
  return millivolts;
}

uint8_t WaterSensor_GetSignalPercent(void)
{
  return signal_percent;
}
