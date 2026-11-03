#ifndef TEST_MAIN_H
#define TEST_MAIN_H

#include <stdint.h>

typedef struct
{
  unsigned int id;
} GPIO_TypeDef;

typedef enum
{
  GPIO_PIN_RESET = 0,
  GPIO_PIN_SET = 1
} GPIO_PinState;

extern GPIO_TypeDef test_gpioe;
extern GPIO_TypeDef test_gpiof;

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

void HAL_GPIO_WritePin(GPIO_TypeDef *port, uint16_t pin, GPIO_PinState state);

#endif
