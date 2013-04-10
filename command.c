/*
 * Copyright (C) 2009-2011 CompuLab, Ltd.
 * Authors: Nikita Kiryanov <nikita@compulab.co.il>
 *	    Igor Grinberg <grinberg@compulab.co.il>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <malloc.h>
#include "command.h"
#include "layout.h"
#include "api.h"

static struct api api;

void print_command(const struct command *command)
{
	if (command->action == EEPROM_READ)
		printf("Reading ");
	else
		printf("Writing ");

	printf("using %s ", command->mode);
	printf("at %s, address 0x%x\n", (char *)command->platform_specific_data,
					command->i2c_addr);
}

#define EEPROM_SIZE 256
static void do_io(struct command *command)
{
	unsigned char buf[EEPROM_SIZE];
	struct layout *layout;
	int offset = 0, size = EEPROM_SIZE;

	print_command(command);
	if (api.read(buf, offset, size) < 0) {
		api.system_error("Read error");
		return;
	}

	layout = new_layout(buf, EEPROM_SIZE);
	if (layout == NULL) {
		api.system_error("Memory allocation error");
		goto done;
	}

	if (command->action == EEPROM_READ) {
		layout->print(layout);
		goto done;
	}

	if (command->new_byte_data != NULL)
		layout->update_bytes(layout, command->new_byte_data,
					command->new_data_size);
	else if (command->new_field_data != NULL)
		layout->update_fields(layout, command->new_field_data,
					command->new_data_size);

	if (api.write(layout->data, offset, size) < 0)
		api.system_error("Write error");

done:
	free_layout(layout);
}

void print_i2c_accessible(struct command *command)
{
	api.probe();
}

int setup_command(struct command *cmd, enum action action, const char *mode,
		int i2c_addr, void *platform_specific_data,
		struct offset_value_pair *new_byte_data,
		struct strings_pair *new_field_data, int new_data_size)
{
	cmd->mode = mode;
	cmd->action = action;
	cmd->i2c_addr = i2c_addr;
	cmd->platform_specific_data = platform_specific_data;
	cmd->new_byte_data = new_byte_data;
	cmd->new_field_data = new_field_data;
	cmd->new_data_size = new_data_size;
	cmd->print = print_command;
	if (action == EEPROM_LIST)
		cmd->execute = print_i2c_accessible;
	else if (action == EEPROM_READ || action == EEPROM_WRITE)
		cmd->execute = do_io;
	else
		return -1;

	return setup_interface(&api, cmd);
}

void reset_command(struct command *command)
{
	command->mode = "";
	command->i2c_addr = -1;
	command->platform_specific_data = NULL;
	command->new_byte_data = NULL;
	command->new_field_data = NULL;
	command->new_data_size = -1;
	command->print = NULL;
	command->execute = NULL;
}

void free_command(struct command *command)
{
	free(command->new_byte_data);
	free(command->new_field_data);
}
