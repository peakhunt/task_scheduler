#include "irq.h"
#include "gpio.h"
#include "task_scheduler.h"

#define TASK_STACK_SIZE     512

task_t       _task1;
task_t       _task2;
task_t       _task3;
task_t       _task4;

StackType_t  _task1_stack[TASK_STACK_SIZE] TASK_SCHEDULER_CONFIG_STACK_ALIGN;
StackType_t  _task2_stack[TASK_STACK_SIZE] TASK_SCHEDULER_CONFIG_STACK_ALIGN;
StackType_t  _task3_stack[TASK_STACK_SIZE] TASK_SCHEDULER_CONFIG_STACK_ALIGN;
StackType_t  _task4_stack[TASK_STACK_SIZE] TASK_SCHEDULER_CONFIG_STACK_ALIGN;

static void
set_gpio(int pin, int v)
{
  //irqBlock();
  SetGpio(pin, v);
  //irqUnblock();
}

void
task1(void *pParam)
{
  (void)pParam;
  while(1)
  {
    set_gpio(16, 1);

    ts_delay_ms(1000);

    set_gpio(16, 0);

    ts_delay_ms(1000);
  }
}

void
task2(void *pParam)   // first led
{
  (void)pParam;

  while(1)
  {
#if 1
    ts_delay_ms(500);   
    set_gpio(9, 0);
    ts_delay_ms(500);
    set_gpio(9, 1);
#endif
  }
}

void
task3(void *pParam)   // 2nd led
{
  (void)pParam;

  while(1)
  {
    ts_delay_ms(1500);
    set_gpio(10, 0);
    ts_delay_ms(1500);
    set_gpio(10, 1);
  }
}

void
task4(void *pParam)
{
  (void)pParam;

  while(1)
  {
#if 0
    ts_delay_ms(500);
    set_gpio(11, 0);
    ts_delay_ms(500);
    set_gpio(11, 1);
#else
    for(int i = 0; i < 300000; i++)
    {
    }
    set_gpio(11, 0);
    for(int i = 0; i < 300000; i++)
    {
    }
    set_gpio(11, 1);
#endif
  }
}


/**
 *	This is the systems main entry, some call it a boot thread.
 *
 *	-- Absolutely nothing wrong with this being called main(), just it doesn't have
 *	-- the same prototype as you'd see in a linux program.
 **/
int
main (void)
{
  SetGpioFunction(16, 1);			// RDY led

  SetGpioFunction(9, 1);
  SetGpioFunction(10, 1);
  SetGpioFunction(11, 1);

  ts_init();

  ts_create_task(&_task1, task1, _task1_stack, TASK_STACK_SIZE, NULL);
  ts_create_task(&_task2, task2, _task2_stack, TASK_STACK_SIZE, NULL);
  ts_create_task(&_task3, task3, _task3_stack, TASK_STACK_SIZE, NULL);
  ts_create_task(&_task4, task4, _task4_stack, TASK_STACK_SIZE, NULL);

  ts_start();

  while(1)
  {
  }
  return 0;
}
