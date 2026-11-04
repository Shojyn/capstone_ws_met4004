#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdint.h>

#include "water_sensor.h"

static uint16_t fake_raw;
static bool fake_read_success;
static unsigned int read_count;

static bool FakeRead(uint16_t *raw)
{
  ++read_count;
  if (!fake_read_success)
  {
    return false;
  }
  *raw = fake_raw;
  return true;
}

static void TestConversionAndDetection(void)
{
  WaterSensor_Init(FakeRead, 0U);
  WaterSensor_ProcessSample(0U, 0U);
  assert(WaterSensor_GetState() == WATER_SENSOR_DRY);
  assert(WaterSensor_GetMillivolts() == 0U);
  assert(WaterSensor_GetSignalPercent() == 0U);

  WaterSensor_ProcessSample(620U, 100U);
  assert(WaterSensor_GetState() == WATER_SENSOR_DRY);
  WaterSensor_ProcessSample(620U, 200U);
  assert(WaterSensor_GetState() == WATER_SENSOR_DRY);
  WaterSensor_ProcessSample(620U, 300U);
  WaterSensor_ProcessSample(620U, 400U);
  WaterSensor_ProcessSample(620U, 500U);
  assert(WaterSensor_GetState() == WATER_SENSOR_WET);
  assert(WaterSensor_GetMillivolts() == 499U);
  assert(WaterSensor_GetSignalPercent() == 29U);

  WaterSensor_ProcessSample(372U, 600U);
  assert(WaterSensor_GetState() == WATER_SENSOR_WET);
  WaterSensor_ProcessSample(0U, 700U);
  WaterSensor_ProcessSample(0U, 800U);
  WaterSensor_ProcessSample(0U, 900U);
  WaterSensor_ProcessSample(0U, 1000U);
  assert(WaterSensor_GetState() == WATER_SENSOR_DRY);
}

static void TestFullScaleAndHysteresis(void)
{
  WaterSensor_Init(FakeRead, 0U);
  for (uint32_t i = 0U; i < 4U; ++i)
  {
    WaterSensor_ProcessSample(2111U, i * 100U);
  }
  assert(WaterSensor_GetMillivolts() == 1701U);
  assert(WaterSensor_GetSignalPercent() == 100U);
  assert(WaterSensor_GetState() == WATER_SENSOR_WET);

  for (uint32_t i = 4U; i < 8U; ++i)
  {
    WaterSensor_ProcessSample(372U, i * 100U);
  }
  assert(WaterSensor_GetMillivolts() == 299U);
  assert(WaterSensor_GetState() == WATER_SENSOR_WET);
}

static void TestSamplingIntervalAndTickWraparound(void)
{
  fake_raw = 620U;
  fake_read_success = true;
  read_count = 0U;
  WaterSensor_Init(FakeRead, 1000U);
  WaterSensor_Update(1000U);
  assert(read_count == 1U);
  WaterSensor_Update(1050U);
  assert(read_count == 1U);
  WaterSensor_Update(1100U);
  assert(read_count == 2U);

  read_count = 0U;
  uint32_t start = UINT32_MAX - 50U;
  WaterSensor_Init(FakeRead, start);
  WaterSensor_Update(start);
  assert(read_count == 1U);
  WaterSensor_Update(start + 50U);
  assert(read_count == 1U);
  WaterSensor_Update(start + 100U);
  assert(read_count == 2U);

  fake_read_success = false;
  WaterSensor_Init(FakeRead, 2000U);
  WaterSensor_ProcessSample(620U, 2000U);
  WaterSensor_Update(2000U);
  assert(WaterSensor_GetRaw() == 620U);
}

int main(void)
{
  TestConversionAndDetection();
  TestFullScaleAndHysteresis();
  TestSamplingIntervalAndTickWraparound();
  return 0;
}
