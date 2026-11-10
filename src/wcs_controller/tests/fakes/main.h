#ifndef TEST_MAIN_H
#define TEST_MAIN_H

#include <stdint.h>

typedef struct
{
  unsigned int id;
} GPIO_TypeDef;

typedef struct
{
  unsigned int id;
} TIM_HandleTypeDef;

typedef enum
{
  HAL_OK = 0,
  HAL_ERROR = 1
} HAL_StatusTypeDef;

typedef enum
{
  GPIO_PIN_RESET = 0,
  GPIO_PIN_SET = 1
} GPIO_PinState;

extern GPIO_TypeDef test_gpioe;
extern GPIO_TypeDef test_gpiof;
extern GPIO_TypeDef test_gpioa;
extern TIM_HandleTypeDef htim3;

#define valve_1_GPIO_Port (&test_gpioe)
#define valve_2_GPIO_Port (&test_gpioe)
#define valve_3_GPIO_Port (&test_gpioe)
#define valve_1_Pin (1U << 2)
#define valve_2_Pin (1U << 3)
#define valve_3_Pin (1U << 4)

#define in1_linear_1_GPIO_Port (&test_gpiof)
#define in2_linear_1_GPIO_Port (&test_gpiof)
#define in1_linear_1_Pin (1U << 2)
#define in2_linear_1_Pin (1U << 3)

#define stepper_dir_GPIO_Port (&test_gpioa)
#define stepper_dir_Pin (1U << 5)
#define TIM3 (3U)
#define TIM_CHANNEL_1 (1U)
#define TIM_FLAG_CC1 (1U)

void HAL_GPIO_WritePin(GPIO_TypeDef *port, uint16_t pin, GPIO_PinState state);
HAL_StatusTypeDef HAL_TIM_PWM_Start_IT(TIM_HandleTypeDef *timer, uint32_t channel);
HAL_StatusTypeDef HAL_TIM_PWM_Stop_IT(TIM_HandleTypeDef *timer, uint32_t channel);
void TestTimClearFlag(TIM_HandleTypeDef *timer, uint32_t flag);

#define __HAL_TIM_CLEAR_FLAG(timer, flag) TestTimClearFlag((timer), (flag))

#endif
