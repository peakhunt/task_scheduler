#include <stdlib.h>
#include <avr/interrupt.h>
#include "task_scheduler_hw.h"

/* Start tasks with interrupts enabled. */
#define portFLAGS_INT_ENABLED           ( (StackType_t) 0x80 )
#define TIMER_TICK_COUNT_FOR_1MS        250

#define portSAVE_CONTEXT()                                                        \
  __asm__ __volatile__ (  "push   r0                                      \n\t"   \
                          "in     r0, __SREG__                            \n\t"   \
                          "cli                                            \n\t"   \
                          "push   r0                                      \n\t"   \
                          "push   r1                                      \n\t"   \
                          "clr    r1                                      \n\t"   \
                          "push   r2                                      \n\t"   \
                          "push   r3                                      \n\t"   \
                          "push   r4                                      \n\t"   \
                          "push   r5                                      \n\t"   \
                          "push   r6                                      \n\t"   \
                          "push   r7                                      \n\t"   \
                          "push   r8                                      \n\t"   \
                          "push   r9                                      \n\t"   \
                          "push   r10                                     \n\t"   \
                          "push   r11                                     \n\t"   \
                          "push   r12                                     \n\t"   \
                          "push   r13                                     \n\t"   \
                          "push   r14                                     \n\t"   \
                          "push   r15                                     \n\t"   \
                          "push   r16                                     \n\t"   \
                          "push   r17                                     \n\t"   \
                          "push   r18                                     \n\t"   \
                          "push   r19                                     \n\t"   \
                          "push   r20                                     \n\t"   \
                          "push   r21                                     \n\t"   \
                          "push   r22                                     \n\t"   \
                          "push   r23                                     \n\t"   \
                          "push   r24                                     \n\t"   \
                          "push   r25                                     \n\t"   \
                          "push   r26                                     \n\t"   \
                          "push   r27                                     \n\t"   \
                          "push   r28                                     \n\t"   \
                          "push   r29                                     \n\t"   \
                          "push   r30                                     \n\t"   \
                          "push   r31                                     \n\t"   \
                          "lds    r26, _current_task                      \n\t"   \
                          "lds    r27, _current_task + 1                  \n\t"   \
                          "in     r0, 0x3d                                \n\t"   \
                          "st     x+, r0                                  \n\t"   \
                          "in     r0, 0x3e                                \n\t"   \
                          "st     x+, r0                                  \n\t"   \
  );

#define portRESTORE_CONTEXT()                                                     \
  __asm__ __volatile__ (  "lds    r26, _current_task                      \n\t"   \
                          "lds    r27, _current_task + 1                  \n\t"   \
                          "ld     r28, x+                                 \n\t"   \
                          "out    __SP_L__, r28                           \n\t"   \
                          "ld     r29, x+                                 \n\t"   \
                          "out    __SP_H__, r29                           \n\t"   \
                          "pop    r31                                     \n\t"   \
                          "pop    r30                                     \n\t"   \
                          "pop    r29                                     \n\t"   \
                          "pop    r28                                     \n\t"   \
                          "pop    r27                                     \n\t"   \
                          "pop    r26                                     \n\t"   \
                          "pop    r25                                     \n\t"   \
                          "pop    r24                                     \n\t"   \
                          "pop    r23                                     \n\t"   \
                          "pop    r22                                     \n\t"   \
                          "pop    r21                                     \n\t"   \
                          "pop    r20                                     \n\t"   \
                          "pop    r19                                     \n\t"   \
                          "pop    r18                                     \n\t"   \
                          "pop    r17                                     \n\t"   \
                          "pop    r16                                     \n\t"   \
                          "pop    r15                                     \n\t"   \
                          "pop    r14                                     \n\t"   \
                          "pop    r13                                     \n\t"   \
                          "pop    r12                                     \n\t"   \
                          "pop    r11                                     \n\t"   \
                          "pop    r10                                     \n\t"   \
                          "pop    r9                                      \n\t"   \
                          "pop    r8                                      \n\t"   \
                          "pop    r7                                      \n\t"   \
                          "pop    r6                                      \n\t"   \
                          "pop    r5                                      \n\t"   \
                          "pop    r4                                      \n\t"   \
                          "pop    r3                                      \n\t"   \
                          "pop    r2                                      \n\t"   \
                          "pop    r1                                      \n\t"   \
                          "pop    r0                                      \n\t"   \
                          "out    __SREG__, r0                            \n\t"   \
                          "pop    r0                                      \n\t"   \
  );

void __ts_hw_systick_handler_callback(void) __attribute__ (( weak ));

ISR(TIMER0_COMPA_vect)
{
  __ts_hw_systick_handler_callback();
  ts_handle_tick();
}

static void
__setup_tick_timer(void)
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

void
__ts_hw_systick_handler_callback(void)
{
  //
  // just special weak function for a special case
  // for convenience
  //
  // do nothing
}

//void ts_hw_context_switch(void) __attribute__ ((naked));
void
ts_hw_context_switch(void)
{
  portSAVE_CONTEXT();

  ts_pick_new_task();

  portRESTORE_CONTEXT();
}

void
ts_hw_initialize_task_stack(task_t* task, task_entry_t entry, void* arg)
{
  //
  // XXX
  // shamelessly copied from FreeRTOS source
  //
	uint16_t usAddress;

  /* Simulate how the stack would look after a call to vPortYield() generated by
  the compiler. */

  /* The start of the task code will be popped off the stack last, so place
  it on first. */

  usAddress = ( uint16_t ) entry;
  *(task->sp_top) = ( usAddress & ( uint16_t ) 0x00ff );
  task->sp_top--;

  usAddress >>= 8;
  *(task->sp_top) = ( StackType_t ) ( usAddress & ( uint16_t ) 0x00ff );
  task->sp_top--;

  /* Next simulate the stack as if after a call to portSAVE_CONTEXT().
  portSAVE_CONTEXT places the flags on the stack immediately after r0
  to ensure the interrupts get disabled as soon as possible, and so ensuring
  the stack use is minimal should a context switch interrupt occur. */
  *(task->sp_top) =  0x00; /* R0 */
  task->sp_top--;
  *(task->sp_top) = portFLAGS_INT_ENABLED;
  task->sp_top--;

  /* Now the remaining registers.   The compiler expects R1 to be 0. */
  *(task->sp_top) = ( StackType_t ) 0x00; /* R1 */
  task->sp_top--;
  *(task->sp_top) = ( StackType_t ) 0x02; /* R2 */
  task->sp_top--;
  *(task->sp_top) = ( StackType_t ) 0x03; /* R3 */
  task->sp_top--;
  *(task->sp_top) = ( StackType_t ) 0x04; /* R4 */
  task->sp_top--;
  *(task->sp_top) = ( StackType_t ) 0x05; /* R5 */
  task->sp_top--;
  *(task->sp_top) = ( StackType_t ) 0x06; /* R6 */
  task->sp_top--;
  *(task->sp_top) = ( StackType_t ) 0x07; /* R7 */
  task->sp_top--;
  *(task->sp_top) = ( StackType_t ) 0x08; /* R8 */
  task->sp_top--;
  *(task->sp_top) = ( StackType_t ) 0x09; /* R9 */
  task->sp_top--;
  *(task->sp_top) = ( StackType_t ) 0x10; /* R10 */
  task->sp_top--;
  *(task->sp_top) = ( StackType_t ) 0x11; /* R11 */
  task->sp_top--;
  *(task->sp_top) = ( StackType_t ) 0x12; /* R12 */
  task->sp_top--;
  *(task->sp_top) = ( StackType_t ) 0x13; /* R13 */
  task->sp_top--;
  *(task->sp_top) = ( StackType_t ) 0x14; /* R14 */
  task->sp_top--;
  *(task->sp_top) = ( StackType_t ) 0x15; /* R15 */
  task->sp_top--;
  *(task->sp_top) = ( StackType_t ) 0x16; /* R16 */
  task->sp_top--;
  *(task->sp_top) = ( StackType_t ) 0x17; /* R17 */
  task->sp_top--;
  *(task->sp_top) = ( StackType_t ) 0x18; /* R18 */
  task->sp_top--;
  *(task->sp_top) = ( StackType_t ) 0x19; /* R19 */
  task->sp_top--;
  *(task->sp_top) = ( StackType_t ) 0x20; /* R20 */
  task->sp_top--;
  *(task->sp_top) = ( StackType_t ) 0x21; /* R21 */
  task->sp_top--;
  *(task->sp_top) = ( StackType_t ) 0x22; /* R22 */
  task->sp_top--;
  *(task->sp_top) = ( StackType_t ) 0x23; /* R23 */
  task->sp_top--;

  /* Place the parameter on the stack in the expected location. */
  usAddress = ( uint16_t ) arg;
  *(task->sp_top) = ( StackType_t ) ( usAddress & ( uint16_t ) 0x00ff );
  task->sp_top--;

  usAddress >>= 8;
  *(task->sp_top) = ( StackType_t ) ( usAddress & ( uint16_t ) 0x00ff );
  task->sp_top--;

  *(task->sp_top) = ( StackType_t ) 0x26; /* R26 X */
  task->sp_top--;
  *(task->sp_top) = ( StackType_t ) 0x27; /* R27 */
  task->sp_top--;
  *(task->sp_top) = ( StackType_t ) 0x28; /* R28 Y */
  task->sp_top--;
  *(task->sp_top) = ( StackType_t ) 0x29; /* R29 */
  task->sp_top--;
  *(task->sp_top) = ( StackType_t ) 0x30; /* R30 Z */
  task->sp_top--;
  *(task->sp_top) = ( StackType_t ) 0x031;  /* R31 */
  task->sp_top--;
}

void
ts_hw_disable_interrupts(void)
{
  cli();
}

void
ts_hw_enable_interrupts(void)
{
  sei();
}

void
ts_hw_start_scheduler(void)
{
  ts_hw_disable_interrupts();

  //
  // setup tick timer
  //
  __setup_tick_timer();


  //
  // start first task
  //
  portRESTORE_CONTEXT();

  //
  // Simulate a function call end as generated by the compiler.  We will now
  // jump to the start of the task the context of which we have just restored.
  //
  __asm__ __volatile__ ( "ret" );
}

void
ts_hw_enter_idle_task(void)
{
  // FIXME
}
