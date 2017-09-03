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
static unsigned char buf[EEPROM_SIZE];

static void read_eeprom(unsigned char *buf)
{
	if (api.read(buf, 0, EEPROM_SIZE) < 0)
		api.system_error("Read error");
}

static void write_eeprom(unsigned char *data)
{
	if (api.write(data, 0, EEPROM_SIZE) < 0)
		api.system_error("Write error");
}

static struct layout *prepare_layout(struct command *cmd)
{
	read_eeprom(buf);
	struct layout *layout = new_layout(buf, EEPROM_SIZE, cmd->layout_ver);
	if (layout == NULL)
		api.system_error("Memory allocation error");

	return layout;
}

static void print_i2c_accessible(struct command *cmd)
{
	api.probe(cmd->i2c_bus);
}

static void execute_command(struct command *cmd)
{
	struct layout *layout = NULL;
	if (setup_interface(&api, cmd->i2c_bus, cmd->i2c_addr))
		return;

	if (cmd->action == EEPROM_ACTION_INVALID) {
		api.system_error("Invalid command");
		return;
	}

	if (cmd->action == EEPROM_LIST) {
		print_i2c_accessible(cmd);
		return;
	}

	layout = prepare_layout(cmd);
	if (!layout)
		return;

	switch(cmd->action) {
	case EEPROM_READ:
		layout->print(layout);
		goto done;
	case EEPROM_WRITE_FIELDS:
		layout->update_fields(layout, cmd->new_field_data,
				      cmd->new_data_size);
		break;
	case EEPROM_WRITE_BYTES:
		layout->update_bytes(layout, cmd->new_field_data,
				     cmd->new_data_size);
		break;
	default:
		goto done;
	}

	write_eeprom(layout->data);

done:
	free_layout(layout);
}

struct command *new_command(enum action action, int i2c_bus, int i2c_addr,
		    	    enum layout_version layout_ver, int new_data_size,
		    	    struct strings_pair *new_field_data)
{
	struct command *cmd = malloc(sizeof(struct command));
	if (cmd == NULL)
		return cmd;

	cmd->action = action;
	cmd->i2c_bus = i2c_bus;
	cmd->i2c_addr = i2c_addr;
	cmd->layout_ver = layout_ver;
	cmd->new_field_data = new_field_data;
	cmd->new_data_size = new_data_size;
	cmd->execute = execute_command;

	return cmd;
}

void free_command(struct command *cmd)
{
	free(cmd);
}
