#include "task_scheduler_hw.h"
#include "irq.h"
#include "gpio.h"

//
// in this port,
// - tasks run in ARM/System Mode
// - IRQs run in IRQ mode of course
//   so preemption occurs in IRQ mode directly
// - voluntary context switching occurs
//   in IRQ mode using SWI
//
#define portINSTRUCTION_SIZE        ((StackType_t)4)
#define portINITIAL_SPSR            ((StackType_t)0x1f) /* System mode, ARM mode, interrupts enabled. */

static volatile uint8_t   _in_irq = 0;

void __ts_hw_systick_handler_callback(void) __attribute__ (( weak ));
void __start_first_task(void) __attribute__ (( naked ));

void
__start_first_task(void)
{
  //
  // just return from Supervisor mode to system mode
  //
  __asm volatile (
    "LDR R0, =_current_task       \n"
    "LDR R0, [R0]                 \n"
    "LDR LR, [R0]                 \n"     // LR = pxCurrentTCB->sp_top

    //"LDR R0, =ulCriticalNesting   \n"     // R0 = &ulCriticalNesting
    //"LDMFD  LR!, {R1}             \n"     // pop SP_usr. R1 = ulCriticalNesting from SP_usr
    //"STR    R1, [R0]              \n"     // ulCriticalNesting = R1
    "LDMFD  LR!, {R0}             \n"     // pop SP_usr. R0 = SPSR from SP_usr
    "MSR    SPSR, R0              \n"     // SPSR = R0

    "LDMFD  LR, {R0-R14}^         \n"     // pop SP_Usr. R0-R14_usr from SP_usr
    "NOP                          \n"
    "LDR    LR, [LR, #+60]        \n"     // LR = LR from SP_Usr, that is PC saved in user mode stack
    "MOVS PC, LR                  \n"
    //"SUBS PC, LR, #4              \n"     //
                                          // PC = LR - 4
                                          // SUBS pc, lr, #imm subtracts a value from the link register and loads the PC
                                          // with the result, then copies the SPSR to the CPSR.
                                          //
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

  // test
}

extern void irqHandler(void);

void __ts_hw_swi_handler(void) __attribute__((naked));
void __ts_hw_swi_handler(void)
{
  //
  // runs in supervisor mode
  // IRQs are blocked upon entering SVC mode
  //

  /* Perform the context switch.  First save the context of the current task. */
  __asm volatile (
    "STMDB  SP!, {R0}             \n"     // save R0 in stack. we will tamper it
    "STMDB  SP,{SP}^              \n"     // save SP_usr in stack
    "NOP                          \n"     // why this way??? why not '!' suffix?
    "SUB  SP, SP, #4              \n"
    "LDMIA  SP!,{R0}              \n"     // R0 = SP_usr
    "STMDB  R0!, {LR}             \n"     // save LR in SP_usr
    "MOV  LR, R0                  \n"     // LR = R0 = SP_usr
    "LDMIA  SP!, {R0}             \n"     // restore original R0
    "STMDB  LR,{R0-LR}^           \n"     // push user mode R0-R14 to LR(user mode SP). Even R13(SP) ???
    "NOP                          \n"
    "SUB  LR, LR, #60             \n"     // LR(user mode SP) -= 60 ( 15 registers * 4 bytes = 60)
    "MRS  R0, SPSR                \n"     // R0 = SPSR
    "STMDB  LR!, {R0}             \n"     // Push R0 (SPSR) into LR (user mode SP)
    //"LDR  R0, =ulCriticalNesting  \n"
    //"LDR  R0, [R0]                \n"     // R0 = ulCriticalNesting    
    //"STMDB  LR!, {R0}             \n"     // push ulCriticalNesting to user mode stack
    "LDR  R0, =_current_task      \n"     //
    "LDR  R0, [R0]                \n"     // R0 = _current_task
    "STR  LR, [R0]                \n"     // _current_task->sp_top = LR (user mode SP)
  );

  /* Find the highest priority task that is ready to run. */
  ts_pick_new_task();

  /* Restore the context of the new task. */
  __asm volatile (
    "LDR R0, =_current_task       \n"
    "LDR R0, [R0]                 \n"
    "LDR LR, [R0]                 \n"     // LR = pxCurrentTCB->sp_top

    //"LDR R0, =ulCriticalNesting   \n"     // R0 = &ulCriticalNesting
    //"LDMFD  LR!, {R1}             \n"     // pop SP_usr. R1 = ulCriticalNesting from SP_usr
    //"STR    R1, [R0]              \n"     // *ulCriticalNesting = R1
    "LDMFD  LR!, {R0}             \n"     // pop SP_usr. R0 = SPSR from SP_usr
    "MSR    SPSR, R0              \n"     // SPSR = R0

    "LDMFD  LR, {R0-R14}^         \n"     // pop SP_Usr. R0-R14_usr from SP_usr
    "NOP                          \n"
    "LDR    LR, [LR, #+60]        \n"     // LR = LR from SP_Usr, that is PC saved in user mode stack
    "MOVS PC, LR                  \n"
    //"SUBS PC, LR, #4              \n"     //
                                          // PC = LR - 4
                                          // SUBS pc, lr, #imm subtracts a value from the link register and loads the PC
                                          // with the result, then copies the SPSR to the CPSR.
                                          //
  );
}

void __ts_hw_irq_handler(void) __attribute__((naked));
void __ts_hw_irq_handler(void)
{
  /*
      saved context

      ----------------------
      LR (R14. interrupted PC(R15) in user mode)
      R0
      R1
      ...
      R14
      SPSR                                      <--- top of the stack
      ----------------------
  */

  //
  // save context
  //
  __asm volatile (
    "STMDB  SP!, {R0}             \n"     // save R0 in stack. we will tamper it
    "STMDB  SP,{SP}^              \n"     // save SP_usr in stack
    "NOP                          \n"     // why this way??? why not '!' suffix?
    "SUB  SP, SP, #4              \n"
    "LDMIA  SP!,{R0}              \n"     // R0 = SP_usr
    "STMDB  R0!, {LR}             \n"     // save LR in SP_usr
    "MOV  LR, R0                  \n"     // LR = R0 = SP_usr
    "LDMIA  SP!, {R0}             \n"     // restore original R0
    "STMDB  LR,{R0-LR}^           \n"     // push user mode R0-R14 to LR(user mode SP). Even R13(SP) ???
    "NOP                          \n"
    "SUB  LR, LR, #60             \n"     // LR(user mode SP) -= 60 ( 15 registers * 4 bytes = 60)
    "MRS  R0, SPSR                \n"     // R0 = SPSR
    "STMDB  LR!, {R0}             \n"     // Push R0 (SPSR) into LR (user mode SP)
    //"LDR  R0, =ulCriticalNesting  \n"
    //"LDR  R0, [R0]                \n"     // R0 = ulCriticalNesting    
    //"STMDB  LR!, {R0}             \n"     // push ulCriticalNesting to user mode stack
    "LDR  R0, =_current_task      \n"     //
    "LDR  R0, [R0]                \n"     // R0 = _current_task
    "STR  LR, [R0]                \n"     // _current_task->sp_top = LR (user mode SP)
  );

  irqHandler();

  //
  // restore context
  //
  __asm volatile (
    "LDR R0, =_current_task       \n"
    "LDR R0, [R0]                 \n"
    "LDR LR, [R0]                 \n"     // LR = pxCurrentTCB->sp_top

    //"LDR R0, =ulCriticalNesting   \n"     // R0 = &ulCriticalNesting
    //"LDMFD  LR!, {R1}             \n"     // pop SP_usr. R1 = ulCriticalNesting from SP_usr
    //"STR    R1, [R0]              \n"     // *ulCriticalNesting = R1
    "LDMFD  LR!, {R0}             \n"     // pop SP_usr. R0 = SPSR from SP_usr
    "MSR    SPSR, R0              \n"     // SPSR = R0

    "LDMFD  LR, {R0-R14}^         \n"     // pop SP_Usr. R0-R14_usr from SP_usr
    "NOP                          \n"
    "LDR    LR, [LR, #+60]        \n"     // LR = LR from SP_Usr, that is PC saved in user mode stack
    "SUBS PC, LR, #4              \n"     //
                                          // PC = LR - 4
                                          // SUBS pc, lr, #imm subtracts a value from the link register and loads the PC
                                          // with the result, then copies the SPSR to the CPSR.
                                          //
  );
}

void
ts_hw_context_switch(void)
{
  __asm volatile ( "SWI 0" );
}

void
ts_hw_context_switch_from_isr(void)
{
  //
  // nothing to do here
  // irq handler entry will
  // automatically do this
  //
}

void
ts_hw_initialize_task_stack(task_t* task, task_entry_t entry, void* arg)
{
  StackType_t* original_tos = (StackType_t*)task->sp_top;

  /*
     First on the stack is the return address - which in this case is the
     start of the task.  The offset is added to make the return address appear
     as it would within an IRQ ISR.
   */
  *(task->sp_top) = ( StackType_t ) entry + portINSTRUCTION_SIZE;
  task->sp_top--;

  *(task->sp_top) = ( StackType_t ) 0xaaaaaaaa;         /* R14 */
  task->sp_top--;

  *(task->sp_top) = ( StackType_t ) original_tos;       /* R13 : task stack */
  task->sp_top--;

  *(task->sp_top) = ( StackType_t ) 0x12121212;         /* R12 */
  task->sp_top--;

  *(task->sp_top) = ( StackType_t ) 0x11111111;         /* R11 */
  task->sp_top--;

  *(task->sp_top) = ( StackType_t ) 0x10101010;         /* R10 */
  task->sp_top--;

  *(task->sp_top) = ( StackType_t ) 0x09090909;         /* R9 */
  task->sp_top--;

  *(task->sp_top) = ( StackType_t ) 0x08080808;         /* R8 */
  task->sp_top--;

  *(task->sp_top) = ( StackType_t ) 0x07070707;         /* R7 */
  task->sp_top--;

  *(task->sp_top) = ( StackType_t ) 0x06060606;         /* R6 */
  task->sp_top--;

  *(task->sp_top) = ( StackType_t ) 0x05050505;         /* R5 */
  task->sp_top--;

  *(task->sp_top) = ( StackType_t ) 0x04040404;         /* R4 */
  task->sp_top--;

  *(task->sp_top) = ( StackType_t ) 0x03030303;         /* R3 */
  task->sp_top--;

  *(task->sp_top) = ( StackType_t ) 0x02020202;         /* R2 */
  task->sp_top--;
  
  *(task->sp_top) = ( StackType_t ) 0x01010101;         /* R1 */
  task->sp_top--;

  *(task->sp_top) = ( StackType_t ) arg;                /* R0 */
  task->sp_top--;

  /* 
     The last thing onto the stack is the status register, which is set for
     system mode, with interrupts enabled.
   */
  *(task->sp_top) = ( StackType_t ) portINITIAL_SPSR;
  //task->sp_top--;
}

void
ts_hw_disable_interrupts(void)
{
  if(_in_irq) return;

  irqBlock();
}

void
ts_hw_enable_interrupts(void)
{
  if(_in_irq) return;

  // FIXME memory barrier

  irqUnblock();
}

#define portTIMER_PRESCALE                    ((unsigned long)0xF9)
#define portTIMER_BASE                        ((unsigned long)0x2000B400)

typedef struct _BCM2835_TIMER_REGS {
  unsigned long LOD;
  unsigned long VAL;
  unsigned long CTL;
  unsigned long CLI;
  unsigned long RIS;
  unsigned long MIS;
  unsigned long RLD;
  unsigned long DIV;
  unsigned long CNT;
} BCM2835_TIMER_REGS;

static volatile BCM2835_TIMER_REGS * const pRegs = (BCM2835_TIMER_REGS *) (portTIMER_BASE);

static void
__ts_tick_isr(unsigned int nIRQ, void *pParam)
{
  (void)nIRQ;
  (void)pParam;

  _in_irq = 1;
  __ts_hw_systick_handler_callback();
   ts_handle_tick();
  _in_irq = 0;

  pRegs->CLI = 0;     // Acknowledge the timer interrupt.
}

void
__ts_hw_setup_timer(void)
{
  unsigned long ulCompareMatch;


  /* Calculate the match value required for our wanted tick rate. */
  // ulCompareMatch = 1000000 / configTICK_RATE_HZ;
  ulCompareMatch = 1000000 / TASK_SCHEDULER_CONFIG_TICK_HERTZ;

  /* Protect against divide by zero.  Using an if() statement still results
     in a warning - hence the #if. */
#if portPRESCALE_VALUE != 0
  {
    ulCompareMatch /= ( portPRESCALE_VALUE + 1 );
  }
#else
  (void)ulCompareMatch;
#endif

  pRegs->CTL = 0x003E0000;
  pRegs->LOD = 1000 - 1;
  pRegs->RLD = 1000 - 1;
  pRegs->DIV = portTIMER_PRESCALE;
  pRegs->CLI = 0;
  pRegs->CTL = 0x003E00A2;

  irqRegister(64, __ts_tick_isr, NULL);

  irqEnable(64);
}

void
ts_hw_start_scheduler(void)
{
  // XXX IRQs are already disabled.

  __ts_hw_setup_timer();
  __start_first_task();

  while(1)
  {
  }
}

void
ts_hw_enter_idle_task(void)
{
  __asm volatile(
      " wfi   \n"
  );
}
