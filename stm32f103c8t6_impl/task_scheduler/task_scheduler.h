#ifndef __TASK_SCHEDULER_DEF_H__
#define __TASK_SCHEDULER_DEF_H__

#include <stdint.h>
#include "generic_list.h"
#include "task_scheduler_config.h"

typedef enum
{
  task_state_running,
  task_state_ready,
  task_state_sleeping,
} task_state_t;

typedef struct
{
  volatile uint32_t*  sp_top;       // stack pointer top
  uint32_t            status;       // task status flags
  struct list_head    rqe;          // run queue entry
  struct list_head    tqe;          // tasks queue entry

  volatile uint32_t   tick;         // number of ticks that this task has been running
  task_state_t        state;
  uint32_t            tick_left;
} task_t;

typedef void (*task_entry_t)(void*);

extern void ts_init(void);
extern void ts_start(void);
extern void ts_create_task(task_t* task, task_entry_t entry, void* stack, uint32_t stack_size, void* arg);
extern void ts_enter_critical(void);
extern void ts_leave_critical(void);
extern void ts_delay_ms(uint32_t ms);
extern void ts_yield(void);

//
// these are called with interrupts disabled
//
extern void ts_handle_tick(void);
extern void ts_schedule(void);


extern task_t* volatile _current_task;

#endif /* !__TASK_SCHEDULER_DEF_H__ */
