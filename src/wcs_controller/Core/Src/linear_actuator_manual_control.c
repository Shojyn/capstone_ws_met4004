#include "linear_actuator_manual_control.h"
#include "linear_actuator_controller.h"

#define LINEAR_ACTUATOR_DEBOUNCE_MS 30U

static volatile bool candidate;
static bool stable;
static volatile uint32_t changed_at;
static volatile bool pending;
static LinearActuatorState next_direction;

void LinearActuatorManual_Init(bool input_high, uint32_t now)
{
  /* A button already pressed at boot must be released before it can start movement. */
  stable = true;
  candidate = input_high;
  changed_at = now;
  pending = true;
  next_direction = LINEAR_ACTUATOR_EXTENDING;
}

void LinearActuatorManual_OnEdge(bool input_high, uint32_t now)
{
  candidate = input_high;
  changed_at = now;
  pending = true;
}

void LinearActuatorManual_Update(bool input_high, uint32_t now, bool exti_pending)
{
  if (!pending || (uint32_t)(now - changed_at) < LINEAR_ACTUATOR_DEBOUNCE_MS)
  {
    return;
  }
  if (exti_pending || input_high != candidate)
  {
    candidate = input_high;
    changed_at = now;
    return;
  }
  pending = false;
  if (input_high == stable)
  {
    return;
  }
  stable = input_high;
  if (!input_high)
  {
    return;
  }
  if (LinearActuator_GetState() != LINEAR_ACTUATOR_STOPPED)
  {
    LinearActuator_Stop(now);
  }
  else if (LinearActuator_Start(next_direction, now))
  {
    next_direction = next_direction == LINEAR_ACTUATOR_EXTENDING ?
                     LINEAR_ACTUATOR_RETRACTING : LINEAR_ACTUATOR_EXTENDING;
  }
}
