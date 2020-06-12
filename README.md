# task_scheduler
This is a toy level task scheduler for various micro controllers.  
Please use it at your own risk.  
This is just my hobby project and going through dangerous experiments!

## goals
- simplicity comes first. the whole purpose of this project is to understand how RTOS  
  works so the code should be quite easy to follow. Performance is not the high priority here 
 
- modular so that it should be easy to add a new feature.

## ToDo
- IRQ handling, context switching strategy review for highly generic implementation  
  since there are already 3 ports (ARM-CM3, AVR, ARM11) available.
