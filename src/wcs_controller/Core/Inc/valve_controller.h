#ifndef VALVE_CONTROLLER_H
#define VALVE_CONTROLLER_H

typedef enum
{
  VALVE_1,
  VALVE_2,
  VALVE_3,
  VALVE_COUNT
} ValveId;

typedef enum
{
  VALVE_CLOSED,
  VALVE_OPEN
} ValveState;

/* Main-loop API. Init follows MX_GPIO_Init; states are commands, not feedback. */
void Valve_Init(void);
void Valve_Set(ValveId valve, ValveState state);
void Valve_Toggle(ValveId valve);
ValveState Valve_GetCommand(ValveId valve);

#endif
