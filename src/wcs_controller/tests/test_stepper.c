#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#include "fakes/main.h"
#include "../Core/Inc/stepper_controller.h"
#include "../Core/Inc/stepper_button_control.h"

GPIO_TypeDef test_gpioa = {0U};
GPIO_TypeDef test_gpioe = {0U};
GPIO_TypeDef test_gpiof = {0U};
TIM_HandleTypeDef htim3 = {TIM3};

static GPIO_PinState direction;
static unsigned int pwm_starts;
static unsigned int pwm_stops;

void HAL_GPIO_WritePin(GPIO_TypeDef *port, uint16_t pin, GPIO_PinState state)
{
  assert(port == &test_gpioa);
  assert(pin == stepper_dir_Pin);
  direction = state;
}

HAL_StatusTypeDef HAL_TIM_PWM_Start_IT(TIM_HandleTypeDef *timer, uint32_t channel)
{
  assert(timer == &htim3);
  assert(channel == TIM_CHANNEL_1);
  ++pwm_starts;
  return HAL_OK;
}

HAL_StatusTypeDef HAL_TIM_PWM_Stop_IT(TIM_HandleTypeDef *timer, uint32_t channel)
{
  assert(timer == &htim3);
  assert(channel == TIM_CHANNEL_1);
  ++pwm_stops;
  return HAL_OK;
}

void TestTimClearFlag(TIM_HandleTypeDef *timer, uint32_t flag)
{
  assert(timer == &htim3);
  assert(flag == TIM_FLAG_CC1);
}

static void ResetFakes(void)
{
  direction = GPIO_PIN_RESET;
  pwm_starts = 0U;
  pwm_stops = 0U;
}

static void TestExactMove(void)
{
  ResetFakes();
  Stepper_Init();
  assert(!Stepper_IsBusy());
  assert(Stepper_GetPulseCount() == 0U);
  assert(Stepper_Start(STEPPER_CLOCKWISE));
  assert(Stepper_IsBusy());
  assert(direction == GPIO_PIN_RESET);
  assert(pwm_starts == 1U);
  assert(!Stepper_Start(STEPPER_COUNTER_CLOCKWISE));

  for (uint32_t i = 0U; i < 113U; ++i)
  {
    Stepper_OnPwmPulse();
  }
  assert(Stepper_IsBusy());
  assert(Stepper_GetPulseCount() == 113U);
  assert(pwm_stops == 0U);

  Stepper_OnPwmPulse();
  assert(!Stepper_IsBusy());
  assert(Stepper_GetPulseCount() == 114U);
  assert(pwm_stops == 1U);
  Stepper_OnPwmPulse();
  assert(Stepper_GetPulseCount() == 114U);
  assert(pwm_stops == 1U);
}

static void TestDirection(void)
{
  ResetFakes();
  Stepper_Init();
  assert(Stepper_Start(STEPPER_COUNTER_CLOCKWISE));
  assert(direction == GPIO_PIN_SET);
  Stepper_OnPwmPulse();
}

static void TestButtons(void)
{
  ResetFakes();
  Stepper_Init();
  StepperButtonControl_Init(false, false, 0U);
  StepperButtonControl_Update(false, false, 30U);
  StepperButtonControl_Update(true, false, 40U);
  StepperButtonControl_Update(true, false, 69U);
  assert(pwm_starts == 0U);
  StepperButtonControl_Update(true, false, 70U);
  assert(pwm_starts == 1U);
  assert(direction == GPIO_PIN_RESET);

  for (uint32_t i = 0U; i < 114U; ++i)
  {
    Stepper_OnPwmPulse();
  }
  StepperButtonControl_Update(false, false, 100U);
  StepperButtonControl_Update(false, false, 130U);
  StepperButtonControl_Update(true, false, 140U);
  StepperButtonControl_Update(true, false, 170U);
  assert(pwm_starts == 2U);

  Stepper_Init();
  ResetFakes();
  StepperButtonControl_Init(true, false, 0U);
  StepperButtonControl_Update(true, false, 30U);
  assert(pwm_starts == 0U);
  StepperButtonControl_Update(false, false, 40U);
  StepperButtonControl_Update(false, false, 70U);
  StepperButtonControl_Update(true, true, 80U);
  StepperButtonControl_Update(true, true, 110U);
  assert(pwm_starts == 0U);
}

int main(void)
{
  TestExactMove();
  TestDirection();
  TestButtons();
  return 0;
}
