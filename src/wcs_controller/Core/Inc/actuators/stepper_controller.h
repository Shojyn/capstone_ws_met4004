#ifndef STEPPER_CONTROLLER_H
#define STEPPER_CONTROLLER_H

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
  STEPPER_CLOCKWISE,
  STEPPER_COUNTER_CLOCKWISE
} StepperDirection;

void Stepper_Init(void);
bool Stepper_Start(StepperDirection direction);
void Stepper_OnPwmPulse(void);
bool Stepper_IsBusy(void);
uint32_t Stepper_GetPulseCount(void);

#endif
