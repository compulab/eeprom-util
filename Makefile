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

VERSION = 3
MINORVER = 2
PATCHLEVEL = 0
EEPROM_UTIL_VERSION = $(VERSION).$(MINORVER).$(PATCHLEVEL)

CROSS_COMPILE ?=
CC = $(CROSS_COMPILE)gcc

OBJDIR := obj
DEPDIR := dep

TARGET := eeprom-util
GOAL_FILE := $(OBJDIR)/make_goal
AUTO_GENERATED_FILE := auto_generated.h

CORE := common.o field.o layout.o command.o linux_api.o
MAIN := parser.o

OBJECTS := $(addprefix $(OBJDIR)/,$(CORE))
DEPS    := $(addprefix $(DEPDIR)/,$(CORE:.o=.d) $(MAIN:.o=.d))

CFLAGS     = -Wall -std=gnu99
DEPFLAGS   = -MMD -MF $(DEPDIR)/$(*F).d
WRITEFLAGS = -D ENABLE_WRITE
DEBUGFLAGS = -g -D DEBUG

$(TARGET): $(OBJECTS) $(AUTO_GENERATED_FILE) $(OBJDIR)/$(MAIN)
	$(CC) $(LDFLAGS) $(OBJECTS) $(OBJDIR)/$(MAIN) -o $(TARGET)

$(OBJDIR)/%.o : %.c $(GOAL_FILE)
	$(CC) $(CFLAGS) $(DEPFLAGS) -c -o $@ $<

# pull in dependency info for *existing* .o files
-include $(DEPS)

static: LDFLAGS += -static
static: $(TARGET) ;

write: CFLAGS += $(WRITEFLAGS)
write: $(TARGET) ;

write_static: LDFLAGS += -static
write_static: write ;

debug: CFLAGS += $(DEBUGFLAGS)
debug: write ;

$(AUTO_GENERATED_FILE): $(OBJECTS) $(MAIN:.o=.c)
	@( printf '#define VERSION "%s%s"\n' "$(EEPROM_UTIL_VERSION)" \
	'$(shell ./setversion)' ) > $@
	@date +'#define BUILD_DATE "%d %b %C%y"' >> $@
	@date +'#define BUILD_TIME "%T"' >> $@

# make directory only if they don't exists (prevent unnecessary recompiling)
$(OBJECTS): | $(OBJDIR)
$(DEPS):    | $(DEPDIR)
$(OBJDIR):
	@mkdir -p $(OBJDIR)
$(DEPDIR):
	@mkdir -p $(DEPDIR)

# save goal to file so target will recompile if goal changes
$(GOAL_FILE): .FORCE | $(OBJDIR)
ifneq ($(CROSS_COMPILE)$(MAKECMDGOALS),$(shell cat $(GOAL_FILE) 2>&1))
	@echo '$(CROSS_COMPILE)$(MAKECMDGOALS)' > $@
endif

# fix implicit pattern rules to compile without liniking
$(CORE:.o=): % : $(OBJDIR)/%.o ;
$(MAIN:.o=): % : $(AUTO_GENERATED_FILE) $(OBJDIR)/%.o ;

clean:
	rm -rf $(TARGET) $(OBJDIR) $(DEPDIR) $(AUTO_GENERATED_FILE)

.PHONY: clean .FORCE
