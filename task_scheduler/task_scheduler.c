#include <stdio.h>
#include "task_scheduler.h"
#include "task_scheduler_hw.h"

void __ts_idle_task_callback(void) __attribute__ (( weak ));

//
// generic part
//
static void __add_new_task(task_t* task);

// volatile pointer to non volatile pointer
task_t* volatile _current_task = NULL;

static LIST_HEAD_DECL(_tasks_q);
static LIST_HEAD_DECL(_run_q);
static LIST_HEAD_DECL(_tmr_q);

static volatile int32_t     _critical_nesting = 0;

static task_t       _idle_task;
static StackType_t  _idle_task_stack[TASK_SCHEDULER_CONFIG_IDLE_TASK_STACK_SIZE] TASK_SCHEDULER_CONFIG_STACK_ALIGN;

////////////////////////////////////////////////////////////////////////////////
//
// task scheduler utilities
//
////////////////////////////////////////////////////////////////////////////////
static void
__add_new_task(task_t* task)
{
  ts_enter_critical();
  list_add_tail(&task->tqe, &_tasks_q);
  list_add_tail(&task->rqe, &_run_q);
  ts_leave_critical();
}

////////////////////////////////////////////////////////////////////////////////
//
// task scheduler core : generic part
//
////////////////////////////////////////////////////////////////////////////////
/*
 * select the next best task to run
 *
 * XXX this runs with interrupts disabled!!!
 */
void
ts_pick_new_task(void)
{
  _current_task = list_first_entry(&_run_q, task_t, rqe);

  _current_task->state = task_state_running;
}

/*
 * perform round robind scheduling
 *
 */
void
ts_handle_tick(void)
{
  task_t  *t, *n;
  uint8_t reschedule_needed = 0;

  ts_enter_critical();
  //
  // mission #1 timing measurement for current task for RR scheduling
  //
  t = _current_task;
  t->tick++;
  if(t->tick >= TASK_SCHEDULER_CONFIG_TIME_QUANTA)
  {
    list_del_init(&t->rqe);

    //
    // idle task is always in run_q
    //
    if(!list_is_singular(&_run_q))
    {
      t->state = task_state_ready;
      t->tick = 0;
      list_add_tail(&t->rqe, &_run_q);

      reschedule_needed = 1;
    }
    else
    {
      list_add(&t->rqe, &_run_q);
      t->tick = 0;
    }
  }

  //
  // mission #2 wake up any sleeping tasks on timers
  //
  list_for_each_entry_safe(t, n, &_tmr_q, rqe)
  {
    t->tick_left--;
    if(t->tick_left == 0)
    {
      list_move_tail(&t->rqe, &_run_q);
      t->state = task_state_ready;

      reschedule_needed = 1;
    }
  }
  ts_leave_critical();

  //
  // XXX might cause unnecessary context switching
  // due to race conditions here
  // how to prevent that???
  //

  if(reschedule_needed)
  {
    ts_hw_context_switch_from_isr();
  }
}

////////////////////////////////////////////////////////////////////////////////
//
// idile task
//
////////////////////////////////////////////////////////////////////////////////
void
__ts_idle_task_callback(void)
{
}

static void
__ts_idle_task(void* arg)
{
  while(1)
  {
    ts_hw_enter_idle_task();
    __ts_idle_task_callback();
    ts_yield();
  }
}

////////////////////////////////////////////////////////////////////////////////
//
// public interfaces for task scheduler
//
////////////////////////////////////////////////////////////////////////////////
void
ts_init(void)
{
  // nothing to do
}

void
ts_start(void)
{
  // create idle task
  ts_create_task(&_idle_task,
      __ts_idle_task,
      _idle_task_stack,
      TASK_SCHEDULER_CONFIG_IDLE_TASK_STACK_SIZE,
      NULL);

  // disable interrupts
  ts_hw_disable_interrupts();

  _critical_nesting = 0;

  _current_task = list_first_entry(&_run_q, task_t, rqe);

  ts_hw_start_scheduler();
}

void
ts_create_task(task_t* task, task_entry_t entry, StackType_t* stack, uint32_t stack_size, void* arg)
{
  task->sp_top  = &stack[stack_size -1];
  task->status  = 0;
  task->tick    = 0;
  task->state   = task_state_ready;

  INIT_LIST_HEAD(&task->rqe);
  INIT_LIST_HEAD(&task->tqe);

  //
  // initialize stack frame
  //
  ts_hw_initialize_task_stack(task, entry, arg);

  //
  // add to task queue
  //
  __add_new_task(task);
}

void
ts_enter_critical(void)
{
  ts_hw_disable_interrupts();
  _critical_nesting++;
}

void
ts_leave_critical(void)
{
  if(_critical_nesting <= 0)
  {
    while(1)
      ;
  }

  _critical_nesting--;

  if(_critical_nesting == 0)
  {
    ts_hw_enable_interrupts();
  }
}

void
ts_delay_ms(uint32_t ms)
{
  uint32_t nticks = (ms * TASK_SCHEDULER_CONFIG_TICK_HERTZ) / 1000U;
  task_t* t;

  if (nticks == 0)
  {
    return;
  }

  ts_enter_critical();
  t = _current_task;

  t->tick_left = nticks;
  t->state = task_state_sleeping;

  list_move_tail(&t->rqe, &_tmr_q);
  ts_leave_critical();

  //
  // XXX might cause unnecessary context switching
  // due to race conditions here
  // how to prevent that???
  //

  ts_hw_context_switch();
}

void
ts_yield(void)
{
  task_t* t;
  uint8_t reschedule_needed = 0;

  ts_enter_critical();
  if(!list_is_singular(&_run_q))
  {
    t = _current_task;

    t->state = task_state_ready;
    list_move_tail(&t->rqe, &_run_q);

    reschedule_needed = 1;
  }
  ts_leave_critical();

  //
  // XXX might cause unnecessary context switching
  // due to race conditions here
  // how to prevent that???
  //

  if(reschedule_needed)
  {
    ts_hw_context_switch();
  }
}
