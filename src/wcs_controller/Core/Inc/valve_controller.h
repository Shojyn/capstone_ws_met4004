#ifndef VALVE_CONTROLLER_H
#define VALVE_CONTROLLER_H

#include <stdbool.h>
#include <stdint.h>

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
ValveState Valve_GetCommand(ValveId valve);

/* Nonblocking timed cycle: open travel, purpose dwell, then close lockout. */
bool Valve_StartOperation(ValveId valve, uint32_t now);
void Valve_Update(uint32_t now);
bool Valve_IsBusy(void);
bool Valve_IsDwelling(uint32_t now);
bool Valve_IsClosing(void);

#endif
