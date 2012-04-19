CROSS_COMPILE ?=

eeprom: eeprom_utility.o eeprom.o field.o parser.o layout.o
	$(CROSS_COMPILE)gcc -lm -Wall eeprom.c field.c parser.c layout.c eeprom_utility.c -o eeprom-util
