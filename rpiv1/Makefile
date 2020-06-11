######################################
# target
######################################
TARGET = rpiv1_demo


######################################
# building variables
######################################
# debug build?
DEBUG = 1
# optimization
OPT = -Og


#######################################
# paths
#######################################
# Build path
BUILD_DIR = build

######################################
# source
######################################
# C sources
C_SOURCES =                                             \
../task_scheduler/task_scheduler.c                      \
../task_scheduler/port/armv6z/task_scheduler_armv6z.c   \
drivers/gpio.c                                          \
drivers/irq.c                                           \
demo/main.c

# ASM sources
ASM_SOURCES =                                           \
demo/startup.s


#######################################
# binaries
#######################################
PREFIX = arm-none-eabi-
# The gcc compiler bin path can be either defined in make command via GCC_PATH variable (> make GCC_PATH=xxx)
# either it can be added to the PATH environment variable.
ifdef GCC_PATH
CC = $(GCC_PATH)/$(PREFIX)gcc
LD = $(GCC_PATH)/$(PREFIX)ld
AS = $(GCC_PATH)/$(PREFIX)gcc -x assembler-with-cpp
CP = $(GCC_PATH)/$(PREFIX)objcopy
SZ = $(GCC_PATH)/$(PREFIX)size
OD = $(GCC_PATH)/$(PREFIX)objdump
else
CC = $(PREFIX)gcc
LD = $(PREFIX)ld
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size
OD = $(PREFIX)objdump
endif
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S
 
#######################################
# CFLAGS
#######################################
# cpu
CPU = -march=armv6z

# mcu
MCU = $(CPU)

# macros for gcc
# AS defines
AS_DEFS = 

# C defines
C_DEFS =


# AS includes
AS_INCLUDES = 

# C includes
C_INCLUDES =                        \
-I../task_scheduler                 \
-I../task_scheduler/port/armv6z     \
-Idrivers                           \
-Idemo


# compile gcc flags
ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall -Werror

CFLAGS = $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -Werror

ifeq ($(DEBUG), 1)
CFLAGS += -g
endif


# Generate dependency information
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"


#######################################
# LDFLAGS
#######################################
# link script
LDSCRIPT = raspberrypi.ld

# libraries
LIBS = -lgcc -lc
LIBDIR = -L ~/toolchains/gcc-arm-none-eabi-9-2019-q4-major/lib/gcc/arm-none-eabi/9.2.1\
         -L ~/toolchains/gcc-arm-none-eabi-9-2019-q4-major/arm-none-eabi/lib
LDFLAGS = -Map $(BUILD_DIR)/$(TARGET).map -T $(LDSCRIPT) $(LIBDIR)

# default action: build all
all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).bin $(BUILD_DIR)/$(TARGET).list $(BUILD_DIR)/$(TARGET).syms


#######################################
# build the application
#######################################
# list of objects
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
# list of ASM program objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR) 
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	$(LD) $(LDFLAGS) $(OBJECTS) -o $@ $(LIBS)

$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(BIN) $< $@	

$(BUILD_DIR)/%.list: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(OD) -D -S $< > $@

$(BUILD_DIR)/%.syms: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(OD) -t $< > $@
	$(SZ) $<

$(BUILD_DIR):
	mkdir $@		

#######################################
# clean up
#######################################
clean:
	-rm -fR $(BUILD_DIR)
  
#######################################
# dependencies
#######################################
-include $(wildcard $(BUILD_DIR)/*.d)
