#include "valve_controller.h"
#include "main.h"

/* Physical travel times, measured/specified maximums. */
#define VALVE_OPENING_TIME_MS 5000U
#define VALVE_CLOSING_TIME_MS 5000U

/* Fully-open process dwell per valve. Temporary placeholders. */
#define WASTE_FILL_TIME_MS    5000U
#define WASTE_DRAIN_TIME_MS   5000U
#define COLLECTION_TIME_MS    5000U

#define VALVE_1_OPERATION_TIME_MS \
  (VALVE_OPENING_TIME_MS + WASTE_FILL_TIME_MS)
#define VALVE_2_OPERATION_TIME_MS \
  (VALVE_OPENING_TIME_MS + WASTE_DRAIN_TIME_MS)
#define VALVE_3_OPERATION_TIME_MS \
  (VALVE_OPENING_TIME_MS + COLLECTION_TIME_MS)

typedef enum
{
  VALVE_PHASE_IDLE,
  VALVE_PHASE_OPERATING,
  VALVE_PHASE_CLOSING
} ValvePhase;

typedef struct
{
  GPIO_TypeDef *port;
  uint16_t pin;
  ValveState command;
  uint32_t operation_ms;
} ValveOutput;

static ValveOutput valves[VALVE_COUNT] =
{
  {valve_1_GPIO_Port, valve_1_Pin, VALVE_CLOSED, VALVE_1_OPERATION_TIME_MS},
  {valve_2_GPIO_Port, valve_2_Pin, VALVE_CLOSED, VALVE_2_OPERATION_TIME_MS},
  {valve_3_GPIO_Port, valve_3_Pin, VALVE_CLOSED, VALVE_3_OPERATION_TIME_MS}
};

static ValvePhase phase = VALVE_PHASE_IDLE;
static ValveId active_valve = VALVE_1;
static uint32_t phase_started_at;

void Valve_Init(void)
{
  phase = VALVE_PHASE_IDLE;
  phase_started_at = 0U;
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

ValveState Valve_GetCommand(ValveId valve)
{
  return (unsigned int)valve < VALVE_COUNT ? valves[valve].command : VALVE_CLOSED;
}

bool Valve_StartOperation(ValveId valve, uint32_t now)
{
  if ((unsigned int)valve >= VALVE_COUNT || phase != VALVE_PHASE_IDLE)
  {
    return false;
  }
  active_valve = valve;
  phase = VALVE_PHASE_OPERATING;
  phase_started_at = now;
  Valve_Set(active_valve, VALVE_OPEN);
  return true;
}

bool Valve_RequestClose(ValveId valve, uint32_t now)
{
  if ((unsigned int)valve >= VALVE_COUNT ||
      phase != VALVE_PHASE_OPERATING ||
      active_valve != valve ||
      Valve_GetCommand(valve) != VALVE_OPEN)
  {
    return false;
  }
  Valve_Set(active_valve, VALVE_CLOSED);
  phase = VALVE_PHASE_CLOSING;
  phase_started_at = now;
  return true;
}

void Valve_Update(uint32_t now)
{
  if (phase == VALVE_PHASE_OPERATING &&
      active_valve != VALVE_1 &&
      ((uint32_t)(now - phase_started_at) >= valves[active_valve].operation_ms))
  {
    Valve_Set(active_valve, VALVE_CLOSED);
    phase = VALVE_PHASE_CLOSING;
    phase_started_at = now;
  }
  else if (phase == VALVE_PHASE_CLOSING &&
           ((uint32_t)(now - phase_started_at) >= VALVE_CLOSING_TIME_MS))
  {
    phase = VALVE_PHASE_IDLE;
  }
}

bool Valve_IsBusy(void)
{
  return phase != VALVE_PHASE_IDLE;
}

bool Valve_IsDwelling(uint32_t now)
{
  if (phase != VALVE_PHASE_OPERATING)
  {
    return false;
  }
  uint32_t elapsed = now - phase_started_at;
  return (elapsed >= VALVE_OPENING_TIME_MS) &&
         (active_valve == VALVE_1 || elapsed < valves[active_valve].operation_ms);
}

bool Valve_IsClosing(void)
{
  return phase == VALVE_PHASE_CLOSING;
}

ValveId Valve_GetActiveValve(void)
{
  return active_valve;
}
