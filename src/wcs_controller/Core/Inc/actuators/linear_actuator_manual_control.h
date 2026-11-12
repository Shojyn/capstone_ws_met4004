#ifndef LINEAR_ACTUATOR_MANUAL_CONTROL_H
#define LINEAR_ACTUATOR_MANUAL_CONTROL_H

#include <stdbool.h>
#include <stdint.h>

/* Call OnEdge from EXTI; call Update from the main loop with IRQs masked. */
void LinearActuatorManual_Init(bool input_high, uint32_t now);
void LinearActuatorManual_OnEdge(bool input_high, uint32_t now);
void LinearActuatorManual_Update(bool input_high, uint32_t now, bool exti_pending);

#endif
