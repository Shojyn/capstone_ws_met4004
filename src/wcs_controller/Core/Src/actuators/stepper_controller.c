#include "actuators/stepper_controller.h"
#include "main.h"

extern TIM_HandleTypeDef htim3;

#define STEPPER_MOVE_PULSES 114U

static volatile bool busy;
static volatile uint32_t pulse_count;

void Stepper_Init(void)
{
  busy = false;
  pulse_count = 0U;
  HAL_GPIO_WritePin(stepper_dir_GPIO_Port, stepper_dir_Pin, GPIO_PIN_RESET);
}

bool Stepper_Start(StepperDirection direction)
{
  if (busy)
  {
    return false;
  }

  HAL_GPIO_WritePin(stepper_dir_GPIO_Port, stepper_dir_Pin,
                   direction == STEPPER_CLOCKWISE ? GPIO_PIN_RESET : GPIO_PIN_SET);
  pulse_count = 0U;
  busy = true;
  __HAL_TIM_CLEAR_FLAG(&htim3, TIM_FLAG_CC1);
  if (HAL_TIM_PWM_Start_IT(&htim3, TIM_CHANNEL_1) != HAL_OK)
  {
    busy = false;
    return false;
  }
  return true;
}

void Stepper_OnPwmPulse(void)
{
  if (!busy)
  {
    return;
  }

  ++pulse_count;
  if (pulse_count >= STEPPER_MOVE_PULSES)
  {
    (void)HAL_TIM_PWM_Stop_IT(&htim3, TIM_CHANNEL_1);
    busy = false;
  }
}

bool Stepper_IsBusy(void)
{
  return busy;
}

uint32_t Stepper_GetPulseCount(void)
{
  return pulse_count;
}
