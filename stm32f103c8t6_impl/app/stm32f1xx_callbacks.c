#include <stdint.h>
#include "stm32f1xx_hal.h"

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  switch(GPIO_Pin)
  {
  case GPIO_PIN_2:
    break;
  }
}
