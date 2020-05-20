#ifndef __TASK_SCHEDULER_HW_DEF_H__
#define __TASK_SCHEDULER_HW_DEF_H__

#include "task_scheduler.h"

extern void ts_hw_initialize_task_stack(task_t* task, task_entry_t entry, void* arg);
extern void ts_hw_disable_interrupts(void);
extern void ts_hw_enable_interrupts(void);
extern void ts_hw_start_scheduler(void);
extern void ts_hw_invoke_scheduler(void);
extern void ts_hw_enter_idle_task(void);

#endif /* !__TASK_SCHEDULER_HW_DEF_H__ */
