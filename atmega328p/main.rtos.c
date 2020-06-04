#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdio.h>
#include "task_scheduler.h"

#define TIMER_TICK_COUNT_FOR_1MS            250

#define PERIOD1       200
#define PERIOD2       400
#define PERIOD3       800
#define PERIOD4       1600

#define STACK_SIZE    (32 * 8)

//#define STACK_ATTRIBUTE __attribute__((aligned(8)));
#define STACK_ATTRIBUTE 

static task_t         _led_task1;
static StackType_t    _led_task_stack1[STACK_SIZE] STACK_ATTRIBUTE;

static task_t         _led_task2;
static StackType_t    _led_task_stack2[STACK_SIZE] STACK_ATTRIBUTE;

static task_t         _led_task3;
static StackType_t    _led_task_stack3[STACK_SIZE] STACK_ATTRIBUTE;

static void
init_led(void)
{
  DDRB = 0x20;      // PB5
  DDRD = 0x20;      // PD5
  DDRC = 0x01;      // PC0
}

static void
LEDTask1(void* pvParameters)
{
	while(1)
	{
    PORTB = (PORTB ^ _BV(PB5));
    ts_delay_ms(PERIOD4);

    PORTB = (PORTB ^ _BV(PB5));
    ts_delay_ms(PERIOD3);

    PORTB = (PORTB ^ _BV(PB5));
    ts_delay_ms(PERIOD2);

    PORTB = (PORTB ^ _BV(PB5));
    ts_delay_ms(PERIOD1);
	}
}

static void
LEDTask2(void* pvParameters)
{
	while(1)
	{
    PORTD = (PORTD ^ _BV(PD5));
    ts_delay_ms(100);
	}
}

static void
LEDTask3(void* pvParameters)
{
	while(1)
	{
    PORTC = (PORTC ^ _BV(PC0));

    //
    // 16000000U Clock
    //
    // 1600U is 1ms
    for(uint16_t i = 0; i < 1000; i++)
    {
      for(uint16_t j = 0; j < 1600; j++)
      {
        __asm__ __volatile__ ("nop");
      }
    }
    // ts_delay_ms(300);
	}
}

static void
init_tasks(void)
{
  ts_create_task(&_led_task1, LEDTask1, _led_task_stack1, sizeof(_led_task_stack1), NULL);
  ts_create_task(&_led_task2, LEDTask2, _led_task_stack2, sizeof(_led_task_stack2), NULL);
  ts_create_task(&_led_task3, LEDTask3, _led_task_stack3, sizeof(_led_task_stack3), NULL);
}

int
main(void)
{
  init_led();

  ts_init();

  init_tasks();

  ts_start();

  while(1)
  {
  }
  return 0;
}
