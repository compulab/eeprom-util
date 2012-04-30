CROSS_COMPILE ?=

OBJS = eeprom_utility.o eeprom.o field.o parser.o layout.o
SOURCES	:= $(OBJS:.o=.c)
CFLAGS = -lm -Wall
COMPILE_CMD = $(CROSS_COMPILE)gcc $(CFLAGS) $(SOURCES) -o eeprom-util

eeprom: $(OBJS)
	$(COMPILE_CMD)

write: CFLAGS += -D ENABLE_WRITE
write: eeprom
