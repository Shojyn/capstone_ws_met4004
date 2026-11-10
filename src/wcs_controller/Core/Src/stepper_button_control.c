#include "stepper_button_control.h"
#include "stepper_controller.h"

#define STEPPER_BUTTON_DEBOUNCE_MS 30U

typedef struct
{
  bool candidate;
  bool stable;
  uint32_t changed_at;
  bool pending;
} StepperButton;

static StepperButton clockwise_button;
static StepperButton counter_clockwise_button;

static void StepperButton_Init(StepperButton *button, bool input_high, uint32_t now)
{
  /* A button held at boot must be released before it can start a move. */
  button->candidate = input_high;
  button->stable = true;
  button->changed_at = now;
  button->pending = true;
}

static bool StepperButton_Update(StepperButton *button, bool input_high, uint32_t now)
{
  if (input_high != button->candidate)
  {
    button->candidate = input_high;
    button->changed_at = now;
    button->pending = true;
    return false;
  }
  if (!button->pending || (uint32_t)(now - button->changed_at) < STEPPER_BUTTON_DEBOUNCE_MS)
  {
    return false;
  }

  button->pending = false;
  if (input_high == button->stable)
  {
    return false;
  }

  button->stable = input_high;
  return input_high;
}

void StepperButtonControl_Init(bool clockwise_high, bool counter_clockwise_high, uint32_t now)
{
  StepperButton_Init(&clockwise_button, clockwise_high, now);
  StepperButton_Init(&counter_clockwise_button, counter_clockwise_high, now);
}

void StepperButtonControl_Update(bool clockwise_high, bool counter_clockwise_high, uint32_t now)
{
  bool clockwise_pressed = StepperButton_Update(&clockwise_button, clockwise_high, now);
  bool counter_clockwise_pressed = StepperButton_Update(&counter_clockwise_button,
                                                        counter_clockwise_high, now);
  if (clockwise_high && counter_clockwise_high)
  {
    return;
  }
  if (clockwise_pressed)
  {
    (void)Stepper_Start(STEPPER_CLOCKWISE);
  }
  else if (counter_clockwise_pressed)
  {
    (void)Stepper_Start(STEPPER_COUNTER_CLOCKWISE);
  }
}
