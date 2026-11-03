#ifndef LINEAR_ACTUATOR_CONTROLLER_H
#define LINEAR_ACTUATOR_CONTROLLER_H

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
  LINEAR_ACTUATOR_STOPPED,
  LINEAR_ACTUATOR_EXTENDING,
  LINEAR_ACTUATOR_RETRACTING
} LinearActuatorState;

void LinearActuator_Init(void);
bool LinearActuator_Start(LinearActuatorState direction, uint32_t now);
void LinearActuator_Stop(uint32_t now);
void LinearActuator_Update(uint32_t now);
LinearActuatorState LinearActuator_GetState(void);

#endif
