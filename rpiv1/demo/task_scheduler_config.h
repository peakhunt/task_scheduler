#ifndef __TASK_SCHEDULDER_CONFIG_DEF_H__
#define __TASK_SCHEDULDER_CONFIG_DEF_H__

#include <stdint.h>

typedef uint32_t StackType_t;
#define TASK_SCHEDULER_CONFIG_STACK_ALIGN   __attribute__((aligned(8)))

//
// any IRQ whose priority is lower than this number (higher priority )
// shouldn't use any of task scheduler API!
//
#define TASK_SCHEDULER_CONFIG_TIME_QUANTA             5             // 5ms 
#define TASK_SCHEDULER_CONFIG_TICK_HERTZ              1000U         // 1Khz tick rate
#define TASK_SCHEDULER_CONFIG_IDLE_TASK_STACK_SIZE    256

#endif /* !__TASK_SCHEDULDER_CONFIG_DEF_H__ */
