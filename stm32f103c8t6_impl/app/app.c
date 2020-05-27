#include "stm32f1xx_hal.h"
#include "gpio.h"
#include "app.h"
#include "task_scheduler.h"

#define STACK_SIZE_IN_4           128

static task_t   _demo_task1,
                _demo_task2,
                _demo_task3,
                _demo_task4;

static uint32_t _demo_task_stack1[STACK_SIZE_IN_4] __attribute__((aligned(8)));
static uint32_t _demo_task_stack2[STACK_SIZE_IN_4] __attribute__((aligned(8)));
static uint32_t _demo_task_stack3[STACK_SIZE_IN_4] __attribute__((aligned(8)));
static uint32_t _demo_task_stack4[STACK_SIZE_IN_4] __attribute__((aligned(8)));

static void
demo_task1(void* arg)
{
  while(1)
  {
    HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
    ts_delay_ms(50);
  }
}

static void
demo_task2(void* arg)
{
  while(1)
  {
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_14);
    ts_delay_ms(100);
  }
}

static void
demo_task3(void* arg)
{
  while(1)
  {
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_15);
    ts_delay_ms(150);
  }
}

static void
demo_task4(void* arg)
{
  while(1)
  {
    HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);
    // ts_delay_ms(1000);
    HAL_Delay(1000);
  }
}

void
__ts_hw_systick_handler_callback(void)
{
  HAL_IncTick();
}

void
app_init_f(void)
{
}

void
app_init(void)
{
  ts_init();

  ts_create_task(&_demo_task1, demo_task1, _demo_task_stack1, STACK_SIZE_IN_4, NULL);
  ts_create_task(&_demo_task2, demo_task2, _demo_task_stack2, STACK_SIZE_IN_4, NULL);
  ts_create_task(&_demo_task3, demo_task3, _demo_task_stack3, STACK_SIZE_IN_4, NULL);
  ts_create_task(&_demo_task4, demo_task4, _demo_task_stack4, STACK_SIZE_IN_4, NULL);
}

void
app_loop(void)
{
  ts_start();

  //
  // we should never reach here
  //
  while(1)
  {
    //event_dispatcher_dispatch();
  }
}
