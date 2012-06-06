CROSS_COMPILE ?=

OBJS = eeprom_utility.o eeprom.o field.o parser.o layout.o
SOURCES	:= $(OBJS:.o=.c)
CFLAGS = -Wall
COMPILE_CMD = $(CROSS_COMPILE)gcc $(CFLAGS) $(SOURCES) -o eeprom-util

eeprom: $(OBJS)
	$(COMPILE_CMD)

static: CFLAGS += -static
static: eeprom

write: CFLAGS += -D ENABLE_WRITE
write: eeprom

write_static: CFLAGS += -D ENABLE_WRITE -static
write_static: eeprom

.PHONY: clean
clean:
	rm -f eeprom-util $(OBJS)
