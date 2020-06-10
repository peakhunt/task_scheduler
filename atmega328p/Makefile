CC=avr-gcc
MCU=atmega328p
F_CPU=16000000UL

TARGET=mega328p.hex

TASK_SCHEDULER_SRC :=                                             \
                  ../task_scheduler/task_scheduler.c              \
                  ../task_scheduler/port/avr/task_scheduler_avr.c

SOURCES :=        $(TASK_SCHEDULER_SRC)         \
                  main.c

V = 0
ifeq ($V, 0)
	Q = @
	P = > /dev/null
else
  Q =
  P = 
endif

AVRDUDE = avrdude

AVRDUDE_PROGRAMMER = usbasp

AVRDUDE_WRITE_FLASH = -U flash:w:$(TARGET)

AVRDUDE_FLAGS = -p m328p
AVRDUDE_FLAGS += -c $(AVRDUDE_PROGRAMMER)

INC_PATH= -I./ -I../task_scheduler -I../task_scheduler/port/avr

CFLAGS = -mmcu=$(MCU)
CFLAGS += -funsigned-char -funsigned-bitfields -DDEBUG
CFLAGS += -DF_CPU=$(F_CPU) 
CFLAGS += $(INC_PATH)
CFLAGS += -O3 -ffunction-sections -fdata-sections -fpack-struct -fshort-enums
CFLAGS += -mrelax -Wall -Wstrict-prototypes
CFLAGS += -std=gnu11 -Wundef
CFLAGS += -MMD -MP -MF .dep/$(@F).d
CFLAGS += -DBMP180_API

LFLAGS = -mmcu=$(MCU)
LFLAGS += -Wl,-Map=$(MAPFILE),--cref,--gc-sections
LFLAGS += -lm

#OBJECTS := $(addprefix obj/,$(SOURCES:.c=.o))
OBJECTS := $(addprefix obj/,$(notdir $(SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(SOURCES)))

LINKED := $(addprefix obj/, $(TARGET:.hex=.elf))
BINARY := $(addprefix obj/, $(TARGET:.hex=.bin))
MAPFILE := $(addprefix obj/, $(TARGET:.hex=.map))
SYMFILE := $(addprefix obj/, $(TARGET:.hex=.sym))
LSSFILE := $(addprefix obj/, $(TARGET:.hex=.lss))

all: $(TARGET)


$(TARGET): $(OBJECTS)
	@echo "[LD]        $(TARGET)"
	$Qavr-gcc $(LFLAGS) -o $(LINKED) $^
	$Qavr-objcopy -O ihex -R .eeprom $(LINKED) $@
	$Qavr-objcopy -I ihex $(TARGET) -O binary $(BINARY)
	$Qavr-size --format=berkeley $(LINKED)
	$Qavr-nm -n $(LINKED) > $(SYMFILE)
	$Qavr-objdump -h -S $(LINKED) > $(LSSFILE)

obj/%.o: %.c
	@echo "[CC]        $(notdir $<)"
	$Qmkdir -p $(dir $@)
	$Q$(CC) $(CFLAGS) -c -o $@ $<

program: $(TARGET)
	$(AVRDUDE) $(AVRDUDE_FLAGS) $(AVRDUDE_WRITE_FLASH) $(AVRDUDE_WRITE_EEPROM)

clean:
	@echo "Cleaning $(TARGET)"
	$Qrm -f $(TARGET)
	$Qrm -f $(BINARY)
	$Qrm -f $(MAPFILE)
	$Qrm -f $(SYMFILE)
	$Qrm -f $(LSSFILE)
	@echo "Cleaning Objects"
	$Qrm -fR obj
	$Qrm -fR .dep

-include $(shell mkdir .dep 2>/dev/null) $(wildcard .dep/*)
