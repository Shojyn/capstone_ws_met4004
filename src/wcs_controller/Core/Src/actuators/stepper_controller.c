#include "actuators/stepper_controller.h"
#include "main.h"

extern TIM_HandleTypeDef htim3;

#define STEPPER_PULSES_PER_REVOLUTION 6400U
#define STEPPER_MOVES_PER_REVOLUTION 7U
#define STEPPER_BASE_MOVE_PULSES \
  (STEPPER_PULSES_PER_REVOLUTION / STEPPER_MOVES_PER_REVOLUTION)
#define STEPPER_MOVE_REMAINDER \
  (STEPPER_PULSES_PER_REVOLUTION % STEPPER_MOVES_PER_REVOLUTION)

static volatile bool busy;
static volatile uint32_t pulse_count;
static volatile uint32_t target_pulses;
static uint32_t remainder_accumulator;

static uint32_t Stepper_NextMovePulses(uint32_t *next_remainder)
{
  uint32_t pulses = STEPPER_BASE_MOVE_PULSES;
  uint32_t accumulated_remainder = remainder_accumulator + STEPPER_MOVE_REMAINDER;

  if (accumulated_remainder >= STEPPER_MOVES_PER_REVOLUTION)
  {
    ++pulses;
    accumulated_remainder -= STEPPER_MOVES_PER_REVOLUTION;
  }

  *next_remainder = accumulated_remainder;
  return pulses;
}

void Stepper_Init(void)
{
  busy = false;
  pulse_count = 0U;
  target_pulses = 0U;
  remainder_accumulator = 0U;
  HAL_GPIO_WritePin(stepper_dir_GPIO_Port, stepper_dir_Pin, GPIO_PIN_RESET);
}

bool Stepper_Start(StepperDirection direction)
{
  uint32_t next_remainder;
  uint32_t move_pulses;

  if (busy)
  {
    return false;
  }

  move_pulses = Stepper_NextMovePulses(&next_remainder);
  HAL_GPIO_WritePin(stepper_dir_GPIO_Port, stepper_dir_Pin,
                   direction == STEPPER_CLOCKWISE ? GPIO_PIN_RESET : GPIO_PIN_SET);
  pulse_count = 0U;
  target_pulses = move_pulses;
  busy = true;
  __HAL_TIM_CLEAR_FLAG(&htim3, TIM_FLAG_CC1);
  if (HAL_TIM_PWM_Start_IT(&htim3, TIM_CHANNEL_1) != HAL_OK)
  {
    busy = false;
    return false;
  }
  remainder_accumulator = next_remainder;
  return true;
}

void Stepper_OnPwmPulse(void)
{
  if (!busy)
  {
    return;
  }

  ++pulse_count;
  if (pulse_count >= target_pulses)
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
