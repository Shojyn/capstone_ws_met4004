#include "valve_controller.h"
#include "main.h"

typedef struct
{
  GPIO_TypeDef *port;
  uint16_t pin;
  ValveState command;
} ValveOutput;

static ValveOutput valves[VALVE_COUNT] =
{
  {valve_1_GPIO_Port, valve_1_Pin, VALVE_CLOSED},
  {valve_2_GPIO_Port, valve_2_Pin, VALVE_CLOSED},
  {valve_3_GPIO_Port, valve_3_Pin, VALVE_CLOSED}
};

void Valve_Init(void)
{
  for (unsigned int i = 0U; i < VALVE_COUNT; ++i)
  {
    Valve_Set((ValveId)i, VALVE_CLOSED);
  }
}

void Valve_Set(ValveId valve, ValveState state)
{
  if ((unsigned int)valve >= VALVE_COUNT ||
      (state != VALVE_CLOSED && state != VALVE_OPEN))
  {
    return;
  }
  HAL_GPIO_WritePin(valves[valve].port, valves[valve].pin,
                   state == VALVE_OPEN ? GPIO_PIN_SET : GPIO_PIN_RESET);
  valves[valve].command = state;
}

void Valve_Toggle(ValveId valve)
{
  if ((unsigned int)valve < VALVE_COUNT)
  {
    Valve_Set(valve, valves[valve].command == VALVE_OPEN ? VALVE_CLOSED : VALVE_OPEN);
  }
}

ValveState Valve_GetCommand(ValveId valve)
{
  return (unsigned int)valve < VALVE_COUNT ? valves[valve].command : VALVE_CLOSED;
}
