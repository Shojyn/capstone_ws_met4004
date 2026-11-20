#include "actuators/linear_actuator_controller.h"
#include "main.h"

#define LINEAR_ACTUATOR_MAX_RUN_MS 1000U
#define LINEAR_ACTUATOR_REVERSE_PAUSE_MS 50U

static LinearActuatorState state = LINEAR_ACTUATOR_STOPPED;
static uint32_t started_at;
static uint32_t stopped_at;
static bool previously_stopped;
static bool start_pending;

static void LinearActuator_OutputsOff(void)
{
  HAL_GPIO_WritePin(in1_linear_1_GPIO_Port, in1_linear_1_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(in2_linear_1_GPIO_Port, in2_linear_1_Pin, GPIO_PIN_RESET);
}

static void LinearActuator_OutputsOn(uint32_t now)
{
  /* Both outputs were LOW before selecting either direction. */
  HAL_GPIO_WritePin(state == LINEAR_ACTUATOR_EXTENDING ? in1_linear_1_GPIO_Port : in2_linear_1_GPIO_Port,
                    state == LINEAR_ACTUATOR_EXTENDING ? in1_linear_1_Pin : in2_linear_1_Pin,
                    GPIO_PIN_SET);
  started_at = now;
  start_pending = false;
}

void LinearActuator_Init(void)
{
  LinearActuator_OutputsOff();
  state = LINEAR_ACTUATOR_STOPPED;
  previously_stopped = false;
  start_pending = false;
  started_at = 0U;
  stopped_at = 0U;
}

bool LinearActuator_Start(LinearActuatorState direction, uint32_t now)
{
  if (state != LINEAR_ACTUATOR_STOPPED ||
      (direction != LINEAR_ACTUATOR_EXTENDING && direction != LINEAR_ACTUATOR_RETRACTING))
  {
    return false;
  }

  LinearActuator_OutputsOff();
  state = direction;
  if (previously_stopped && (uint32_t)(now - stopped_at) < LINEAR_ACTUATOR_REVERSE_PAUSE_MS)
  {
    start_pending = true;
  }
  else
  {
    LinearActuator_OutputsOn(now);
  }
  return true;
}

void LinearActuator_Stop(uint32_t now)
{
  LinearActuator_OutputsOff();
  if (state != LINEAR_ACTUATOR_STOPPED)
  {
    stopped_at = now;
    previously_stopped = true;
  }
  state = LINEAR_ACTUATOR_STOPPED;
  start_pending = false;
}

void LinearActuator_Update(uint32_t now)
{
  if (state == LINEAR_ACTUATOR_STOPPED)
  {
    return;
  }
  if (start_pending)
  {
    if ((uint32_t)(now - stopped_at) >= LINEAR_ACTUATOR_REVERSE_PAUSE_MS)
    {
      LinearActuator_OutputsOn(now);
    }
  }
  else if ((uint32_t)(now - started_at) >= LINEAR_ACTUATOR_MAX_RUN_MS)
  {
    LinearActuator_Stop(now);
  }
}

LinearActuatorState LinearActuator_GetState(void)
{
  return state;
}
