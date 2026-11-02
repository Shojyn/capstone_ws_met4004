#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdint.h>

#include "fakes/main.h"
#include "../Core/Inc/valve_controller.h"

GPIO_TypeDef test_gpioe = {0U};
static GPIO_PinState outputs[VALVE_COUNT];
static unsigned int writes;

void HAL_GPIO_WritePin(GPIO_TypeDef *port, uint16_t pin, GPIO_PinState state)
{
  assert(port == &test_gpioe);
  unsigned int index;
  switch (pin)
  {
    case valve_1_Pin: index = 0U; break;
    case valve_2_Pin: index = 1U; break;
    case valve_3_Pin: index = 2U; break;
    default: assert(0); return;
  }
  outputs[index] = state;
  ++writes;
}

static void TestTimingAndLockout(void)
{
  Valve_Init();
  for (unsigned int i = 0U; i < VALVE_COUNT; ++i)
  {
    assert(outputs[i] == GPIO_PIN_RESET);
    assert(Valve_GetCommand((ValveId)i) == VALVE_CLOSED);
  }

  assert(Valve_StartOperation(VALVE_1, 100U));
  assert(outputs[0] == GPIO_PIN_SET);
  assert(outputs[1] == GPIO_PIN_RESET);
  assert(outputs[2] == GPIO_PIN_RESET);
  assert(!Valve_StartOperation(VALVE_2, 101U));
  assert(!Valve_StartOperation(VALVE_1, 101U));
  assert(!Valve_IsDwelling(5099U));

  Valve_Update(5100U);
  assert(Valve_IsDwelling(5100U));
  assert(outputs[0] == GPIO_PIN_SET);

  Valve_Update(10099U);
  assert(Valve_IsDwelling(10099U));
  assert(outputs[0] == GPIO_PIN_SET);

  Valve_Update(10100U);
  assert(!Valve_IsDwelling(10100U));
  assert(Valve_IsClosing());
  assert(Valve_IsBusy());
  assert(outputs[0] == GPIO_PIN_RESET);
  assert(Valve_GetCommand(VALVE_1) == VALVE_CLOSED);
  assert(!Valve_StartOperation(VALVE_3, 10101U));

  Valve_Update(15099U);
  assert(Valve_IsClosing());
  assert(Valve_IsBusy());
  Valve_Update(15100U);
  assert(!Valve_IsClosing());
  assert(!Valve_IsBusy());
  assert(Valve_StartOperation(VALVE_3, 15100U));
  assert(outputs[2] == GPIO_PIN_SET);
  assert(outputs[0] == GPIO_PIN_RESET);
  assert(outputs[1] == GPIO_PIN_RESET);
}

static void TestTickWraparoundAndInvalidId(void)
{
  Valve_Init();
  unsigned int initial_writes = writes;
  assert(!Valve_StartOperation(VALVE_COUNT, 0U));
  assert(writes == initial_writes);

  uint32_t start = UINT32_MAX - 1000U;
  assert(Valve_StartOperation(VALVE_2, start));
  Valve_Update(start + 4999U);
  assert(!Valve_IsDwelling(start + 4999U));
  Valve_Update(start + 5000U);
  assert(Valve_IsDwelling(start + 5000U));
  Valve_Update(start + 9999U);
  assert(outputs[1] == GPIO_PIN_SET);
  Valve_Update(start + 10000U);
  assert(outputs[1] == GPIO_PIN_RESET);
  assert(Valve_IsClosing());
  assert(Valve_IsBusy());
  Valve_Update(start + 15000U);
  assert(!Valve_IsClosing());
  assert(!Valve_IsBusy());
}

int main(void)
{
  TestTimingAndLockout();
  TestTickWraparoundAndInvalidId();
  return 0;
}
