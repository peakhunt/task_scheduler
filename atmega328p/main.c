#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdio.h>
#include "task_scheduler.h"

#define TIMER_TICK_COUNT_FOR_1MS            250
#define configMINIMAL_STACK_SIZE            85

#define PERIOD1       50
#define PERIOD2       100
#define PERIOD3       150
#define PERIOD4       200

static task_t         _led_task1;
static StackType_t    _led_task_stack1[128];

static task_t         _led_task2;
static StackType_t    _led_task_stack2[128];

static void
init_led(void)
{
  DDRB = 0x20;      // PB5
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
    ts_delay_ms(PERIOD1);

    PORTD = (PORTD ^ _BV(PD5));
    ts_delay_ms(PERIOD1);

    PORTD = (PORTD ^ _BV(PD5));
    ts_delay_ms(PERIOD1);

    PORTD = (PORTD ^ _BV(PD5));
    ts_delay_ms(PERIOD1);
	}
}

static void
init_tasks(void)
{
  ts_create_task(&_led_task1, LEDTask1, _led_task_stack1, sizeof(_led_task_stack1), NULL);
  ts_create_task(&_led_task2, LEDTask2, _led_task_stack2, sizeof(_led_task_stack2), NULL);
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
