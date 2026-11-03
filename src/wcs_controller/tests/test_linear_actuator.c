#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdint.h>

#include "fakes/main.h"
#include "../Core/Inc/linear_actuator_controller.h"
#include "../Core/Inc/linear_actuator_manual_control.h"

GPIO_TypeDef test_gpiof = {0U};
static GPIO_PinState in1;
static GPIO_PinState in2;

void HAL_GPIO_WritePin(GPIO_TypeDef *port, uint16_t pin, GPIO_PinState state)
{
  assert(port == &test_gpiof);
  if (pin == in1_linear_1_Pin)
  {
    in1 = state;
  }
  else
  {
    assert(pin == in2_linear_1_Pin);
    in2 = state;
  }
  assert(!(in1 == GPIO_PIN_SET && in2 == GPIO_PIN_SET));
}

static void TestManualMovement(void)
{
  LinearActuator_Init();
  LinearActuatorManual_Init(false, 0U);
  assert(in1 == GPIO_PIN_RESET && in2 == GPIO_PIN_RESET);
  LinearActuatorManual_Update(false, 30U, false);

  LinearActuatorManual_OnEdge(true, 100U);
  LinearActuatorManual_Update(true, 129U, false);
  assert(LinearActuator_GetState() == LINEAR_ACTUATOR_STOPPED);
  LinearActuatorManual_Update(true, 130U, false);
  assert(LinearActuator_GetState() == LINEAR_ACTUATOR_EXTENDING);
  assert(in1 == GPIO_PIN_SET && in2 == GPIO_PIN_RESET);
  LinearActuator_Update(1129U);
  assert(in1 == GPIO_PIN_SET);
  LinearActuator_Update(1130U);
  assert(LinearActuator_GetState() == LINEAR_ACTUATOR_STOPPED);
  assert(in1 == GPIO_PIN_RESET && in2 == GPIO_PIN_RESET);

  LinearActuatorManual_OnEdge(false, 1200U);
  LinearActuatorManual_Update(false, 1230U, false);
  assert(LinearActuator_GetState() == LINEAR_ACTUATOR_STOPPED);
  LinearActuatorManual_OnEdge(true, 1250U);
  LinearActuatorManual_Update(true, 1280U, false);
  assert(LinearActuator_GetState() == LINEAR_ACTUATOR_RETRACTING);
  assert(in1 == GPIO_PIN_RESET && in2 == GPIO_PIN_SET);

  LinearActuatorManual_OnEdge(false, 1300U);
  LinearActuatorManual_Update(false, 1330U, false);
  LinearActuatorManual_OnEdge(true, 1350U);
  LinearActuatorManual_Update(true, 1380U, false);
  assert(LinearActuator_GetState() == LINEAR_ACTUATOR_STOPPED);
  assert(in1 == GPIO_PIN_RESET && in2 == GPIO_PIN_RESET);
}

static void TestStartupBounceAndReversePause(void)
{
  LinearActuator_Init();
  LinearActuatorManual_Init(true, 0U);
  LinearActuatorManual_Update(true, 30U, false);
  assert(LinearActuator_GetState() == LINEAR_ACTUATOR_STOPPED);
  LinearActuatorManual_OnEdge(false, 40U);
  LinearActuatorManual_Update(false, 70U, false);
  LinearActuatorManual_OnEdge(true, 100U);
  LinearActuatorManual_OnEdge(false, 110U);
  LinearActuatorManual_OnEdge(true, 120U);
  LinearActuatorManual_Update(true, 149U, false);
  assert(LinearActuator_GetState() == LINEAR_ACTUATOR_STOPPED);
  LinearActuatorManual_Update(true, 150U, false);
  assert(LinearActuator_GetState() == LINEAR_ACTUATOR_EXTENDING);

  LinearActuator_Stop(200U);
  assert(LinearActuator_Start(LINEAR_ACTUATOR_RETRACTING, 210U));
  assert(LinearActuator_GetState() == LINEAR_ACTUATOR_RETRACTING);
  assert(in1 == GPIO_PIN_RESET && in2 == GPIO_PIN_RESET);
  LinearActuator_Update(249U);
  assert(in2 == GPIO_PIN_RESET);
  LinearActuator_Update(250U);
  assert(in1 == GPIO_PIN_RESET && in2 == GPIO_PIN_SET);
  LinearActuator_Update(1250U);
  assert(in1 == GPIO_PIN_RESET && in2 == GPIO_PIN_RESET);
}

static void TestWraparoundAndPendingEdge(void)
{
  LinearActuator_Init();
  uint32_t start = UINT32_MAX - 100U;
  assert(!LinearActuator_Start(LINEAR_ACTUATOR_STOPPED, start));
  assert(LinearActuator_Start(LINEAR_ACTUATOR_EXTENDING, start));
  LinearActuator_Update(start + 999U);
  assert(in1 == GPIO_PIN_SET);
  LinearActuator_Update(start + 1000U);
  assert(LinearActuator_GetState() == LINEAR_ACTUATOR_STOPPED);

  uint32_t button_start = start + 1100U;
  LinearActuatorManual_Init(false, button_start);
  LinearActuatorManual_Update(false, button_start + 30U, false);
  LinearActuatorManual_OnEdge(true, button_start + 100U);
  LinearActuatorManual_Update(true, button_start + 130U, true);
  assert(LinearActuator_GetState() == LINEAR_ACTUATOR_STOPPED);
  LinearActuatorManual_Update(true, button_start + 160U, false);
  assert(LinearActuator_GetState() == LINEAR_ACTUATOR_EXTENDING);
}

int main(void)
{
  TestManualMovement();
  TestStartupBounceAndReversePause();
  TestWraparoundAndPendingEdge();
  return 0;
}
