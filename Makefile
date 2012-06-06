CROSS_COMPILE ?=

AUTO_GENERATED_FILE = auto_generated.h

OBJS = eeprom_utility.o eeprom.o field.o parser.o layout.o
SOURCES	:= $(OBJS:.o=.c)
CFLAGS = -Wall
COMPILE_CMD = $(CROSS_COMPILE)gcc $(CFLAGS) $(SOURCES) -o eeprom-util

eeprom: $(AUTO_GENERATED_FILE) $(OBJS)
	$(COMPILE_CMD)

static: CFLAGS += -static
static: eeprom

write: CFLAGS += -D ENABLE_WRITE
write: eeprom

write_static: CFLAGS += -D ENABLE_WRITE -static
write_static: eeprom

$(AUTO_GENERATED_FILE):
	@( printf '#define VERSION "%s"\n' \
	'$(shell ./setversion)' ) > $@.tmp
	@cmp -s $@ $@.tmp && rm -f $@.tmp || mv -f $@.tmp $@

.PHONY: clean
clean:
	rm -f eeprom-util $(OBJS) $(AUTO_GENERATED_FILE)
