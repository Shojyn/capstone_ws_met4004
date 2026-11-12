#ifndef STEPPER_BUTTON_CONTROL_H
#define STEPPER_BUTTON_CONTROL_H

#include <stdbool.h>
#include <stdint.h>

void StepperButtonControl_Init(bool clockwise_high, bool counter_clockwise_high, uint32_t now);
void StepperButtonControl_Update(bool clockwise_high, bool counter_clockwise_high, uint32_t now);

#endif
