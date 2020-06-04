#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdio.h>

#define TIMER_TICK_COUNT_FOR_1MS            250
#define configMINIMAL_STACK_SIZE            85

#define PERIOD1       50
#define PERIOD2       50
#define PERIOD3       50
#define PERIOD4       50

static volatile uint8_t     _tick = 0;

#define get_tick()      (_tick)

ISR(TIMER0_COMPA_vect)
{
  _tick++;
}

static void
test_delay(uint16_t delay)
{
  uint8_t loop,
          i,
          tickstart;

  if(delay == 0)
  {
    return;
  }

  loop = (delay - 1) / 255;

  for(i = 0; i <= loop; i++)
  {
    uint8_t wait_in_loop;
    uint8_t remain;

    wait_in_loop = delay >= 255 ? 255 : delay;
    tickstart = get_tick();

    while(remain < wait_in_loop)
    {
      remain = get_tick() - tickstart;
    }
    delay -= wait_in_loop;
  }
}

static void
init_led(void)
{
  DDRB = 0x20;      // PB5
}

static void
init_sys_tick(void)
{
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //
  // set TC0 registers to generate 1ms system tick
  //
  // the core is running with 16 MHz Clock.
  // So to generate 1ms tick with 8 bit TC value,
  //
  // 1ms  = 16000000 Hz / 1000 = 16000 Hz
  // with 64 pre-scaler,
  // 1ms  = 16000 Hz / 64 = 250, which is good enough for us
  //
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  TCCR0A  = (1 << WGM01) | (0 << WGM00) |                /* CTC mode.        */
            (0 << COM0A1) | (0 << COM0A0) |              /* OC0A disabled.   */
            (0 << COM0B1) | (0 << COM0B0);               /* OC0B disabled.   */
  TCCR0B  = (0 << WGM02) | _BV(CS01) | _BV(CS00);         /* CTC mode., 64 prescale  */
  OCR0A   = TIMER_TICK_COUNT_FOR_1MS - 1;
  TCNT0   = 0;                                           /* Reset counter.   */
  TIFR0   = (1 << OCF0A);                                /* Reset pending.   */
  TIMSK0  = (1 << OCIE0A);                               /* IRQ on compare.  */
}

static void
LEDTask3(void* pvParameters)
{
	while(1)
	{
    PORTB = (PORTB ^ _BV(PB5));
    test_delay(PERIOD4);

    PORTB = (PORTB ^ _BV(PB5));
    test_delay(PERIOD3);

    PORTB = (PORTB ^ _BV(PB5));
    test_delay(PERIOD2);

    PORTB = (PORTB ^ _BV(PB5));
    test_delay(PERIOD1);
	}
}

static void
init_tasks(void)
{
}

int
main(void)
{
  init_led();
	init_sys_tick();

  init_tasks();

  sei();

  LEDTask3(NULL);

  while(1)
  {
  }
  return 0;
}
