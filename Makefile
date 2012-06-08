#
# Copyright (C) 2012 CompuLab, Ltd.
# Authors: Nikita Kiryanov <nikita@compulab.co.il>
#	   Igor Grinberg <grinberg@compulab.co.il>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

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
	'$(shell ./setversion)' ) > $@
	@date +'#define BUILD_DATE "%d %b %C%y"' >> $@
	@date +'#define BUILD_TIME "%T"' >> $@

.PHONY: clean
clean:
	rm -f eeprom-util $(OBJS) $(AUTO_GENERATED_FILE)
