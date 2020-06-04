#ifndef __TASK_SCHEDULDER_CONFIG_DEF_H__
#define __TASK_SCHEDULDER_CONFIG_DEF_H__

#include "stm32f1xx.h"

typedef uint32_t StackType_t;

//
// any IRQ whose priority is lower than this number (higher priority )
// shouldn't use any of task scheduler API!
//
#define TASK_SCHEDULER_CONFIG_TIME_QUANTA             5             // 5ms 
#define TASK_SCHEDULER_CONFIG_SYS_CLOCK               72000000U     // 72 Mhz
#define TASK_SCHEDULER_CONFIG_TICK_HERTZ              1000U         // 1Khz tick rate
#define TASK_SCHEDULER_CONFIG_IDLE_TASK_STACK_SIZE    128

#define TASK_SCHEDULER_CONFIG_
#define TASK_SCHEDULER_CONFIG_IRQ_DISABLE_PRIORITY    5
#define TASK_SCHEDULER_CONFIG_PENDSV_SYSTICK_PRIORITY 15

//
// to hookup interrupt handlers
// this is quite platform specific and hw dependent!
//
#define __ts_svc_handler      SVC_Handler
#define __ts_pendsv_handler   PendSV_Handler
#define __ts_systick_handler  SysTick_Handler

#endif /* !__TASK_SCHEDULDER_CONFIG_DEF_H__ */
