#include "task_scheduler_hw.h"

void __ts_svc_handler(void) __attribute__ (( naked ));
void __ts_pendsv_handler(void) __attribute__ (( naked ));
void __ts_systick_handler(void);

void __ts_hw_systick_handler_callback(void) __attribute__ (( weak ));

static void __start_first_task(void) __attribute__ (( naked ));

static volatile uint8_t    _scheduler_started = 0;

static void
__start_first_task(void)
{
  //
  // XXX
  // shamelessly copied from FreeRTOS source
  //
  __asm volatile(
      " ldr r0, =0xE000ED08   \n"       // setup msp
      " ldr r0, [r0]          \n"
      " ldr r0, [r0]          \n"
      " msr msp, r0           \n"       
      " cpsie i               \n"       // enable interrupt
      " cpsie f               \n"
      " dsb                   \n"
      " isb                   \n"
      " svc 0                 \n"       // invoke SVC
      " nop                   \n"
  );

  // BUG if we reach here
  while(1)
    ;
}

void
__ts_svc_handler(void)
{
  //
  // XXX
  // shamelessly copied from FreeRTOS source
  //
  __asm volatile
  (
      " ldr r3, current_task2                 \n"       // r3 = &_current_task
      " ldr r1, [r3]                          \n"       // r1 = _current_task
      " ldr r0, [r1]                          \n"       // r0 = _current_task->sp_top
      " ldmia r0!, {r4-r11}                   \n"       // restore r4-r11
      " msr psp, r0                           \n"       // setup PSP
      " isb                                   \n"
      " mov r0, #0                            \n"
      " msr basepri, r0                       \n"       // enable interrupts by reducing base pri
      " orr r14, #0xd                         \n"       
      " bx r14                                \n"       // return to thread mode
      "                                       \n"
      " .align 4                              \n"
      "current_task2:.word _current_task      \n"       // current_task2 = &_current_task
  );
}

void
__ts_pendsv_handler(void)
{
  //
  // XXX
  // shamelessly copied from FreeRTOS source
  //
  __asm volatile
  (
     " mrs r0, psp                      \n"             // r0 = psp
     " isb                              \n"
     "                                  \n"             
     " ldr r3, current_task             \n"             // r3 = &_current_task
     " ldr r2, [r3]                     \n"             // r2 = _current_task
     "                                  \n"
     " stmdb r0!, {r4-r11}              \n"             // push r4-r11 into psp
     " str r0, [r2]                     \n"             // _current_task->sp_top = r0
     "                                  \n"
     " stmdb sp!, {r3, r14}             \n"
     " mov r0, %0                       \n"             // disable interrupts
     " msr basepri, r0                  \n"
     " bl ts_pick_new_task              \n"             // ts_pick_new_task()
     " mov r0, #0                       \n"
     " msr basepri, r0                  \n"             // enable interrupts
     " ldmia sp!, {r3, r14}             \n"
     "                                  \n"
     " ldr r1, [r3]                     \n"             // r1 = new _current_task
     " ldr r0, [r1]                     \n"             // r0 = new _current_task->sp_top
     " ldmia r0!, {r4-r11}              \n"             // restore context
     " msr psp, r0                      \n"             // set psp to new task's sp
     " isb                              \n"
     " bx r14                           \n"             // return to thread mode
     "                                  \n"
     " .align 4                         \n"
     "current_task: .word _current_task \n"             // current_task = &_current_task
     ::"i"(TASK_SCHEDULER_CONFIG_IRQ_DISABLE_PRIORITY)
  );
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

void
__ts_systick_handler(void)
{
  __ts_hw_systick_handler_callback();

  if(_scheduler_started == 0)
  {
    return;
  }

  ts_handle_tick();
}

void
ts_hw_context_switch(void)
{
  //
  // here in this implementation
  // instead of performing context switch,
  // we invoke pendsv IRQ to do the context switching
  //
  ts_enter_critical();

  SCB->ICSR |= (1 << SCB_ICSR_PENDSVSET_Pos);

  ts_leave_critical();
}

void
ts_hw_initialize_task_stack(task_t* task, task_entry_t entry, void* arg)
{
  //
  // XXX
  // shamelessly copied from FreeRTOS source
  //
  task->sp_top--;
  *(task->sp_top) = 0x01000000UL;                       // PSR
  task->sp_top--;
  *(task->sp_top) = ((uint32_t)entry & 0xfffffffeUL);   // PC
  task->sp_top--;
  *(task->sp_top) = ((uint32_t)entry & 0xfffffffeUL);   // LR: FIXME error handler
  task->sp_top -= 5;                                    // dummy R12, R3, R2, R1
  *(task->sp_top) = (uint32_t)arg;                      // R0
  task->sp_top -= 8;                                    // dummy R11, R10, R9, R8, R7, R6, R5 and R4
}

void
ts_hw_disable_interrupts(void)
{
  __set_BASEPRI(TASK_SCHEDULER_CONFIG_IRQ_DISABLE_PRIORITY);

  //
  // with this memory barrier
  // core kernel structureѕ will be always synchronized
  //
  __ISB();
  __DSB();
}

void
ts_hw_enable_interrupts(void)
{
  __set_BASEPRI(0);
  //
  // with this memory barrier
  // core kernel structureѕ will be always synchronized
  //
  __ISB();
  __DSB();
}

void
ts_hw_start_scheduler(void)
{
  // startup sequence
  // 1. adjust priority for pendsvc and systick
  // 2. setup systick timer
  // 3. start first task
  //    --> SVC is invoked. SVC priority is set to 0
  //    --> In SVC handler
  //          a) first task's context is restored
  //          b) basepri is lowered to 0, which enables interrupts
  //          c) return to thread mode and first task starts running
  //

  //
  // make pend svc and tick lowest priority
  //
  NVIC_SetPriority(PendSV_IRQn, TASK_SCHEDULER_CONFIG_PENDSV_SYSTICK_PRIORITY);
  NVIC_SetPriority(SysTick_IRQn, TASK_SCHEDULER_CONFIG_PENDSV_SYSTICK_PRIORITY);

  //
  // setup tick timer
  //
  SysTick_Config(TASK_SCHEDULER_CONFIG_SYS_CLOCK/TASK_SCHEDULER_CONFIG_TICK_HERTZ);


  _scheduler_started = 1;

  //
  // start first task
  //
  __start_first_task();

  //
  // XXX should never reach here
  //
  while(1)
    ;
}

void
ts_hw_enter_idle_task(void)
{
  __asm volatile(
      " wfi   \n"
  );
}
