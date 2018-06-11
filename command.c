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
#include <string.h>
#include "command.h"
#include "layout.h"
#include "api.h"

static struct api api;
static unsigned char buf[EEPROM_SIZE];

static int read_eeprom(unsigned char *buf)
{
	int ret = api.read(buf, 0, EEPROM_SIZE);

	if (ret < 0)
		api.system_error("Read error");

	return ret;
}

static int write_eeprom(unsigned char *data)
{
	int ret = api.write(data, 0, EEPROM_SIZE);

	if (ret < 0)
		api.system_error("Write error");

	return ret;
}

static struct layout *prepare_layout(struct command *cmd)
{
	if (read_eeprom(buf) < 0)
		return NULL;

	struct layout *layout = NULL;
	layout = new_layout(buf, EEPROM_SIZE, cmd->opts->layout_ver,
			    cmd->opts->print_format);

	if (!layout)
		api.system_error("Memory allocation error");

	return layout;
}

static int print_i2c_accessible(struct command *cmd)
{
	return api.probe(cmd->opts->i2c_bus);
}

static int execute_command(struct command *cmd)
{
	ASSERT(cmd && cmd->action != EEPROM_ACTION_INVALID);

	int ret = -1;
	struct layout *layout = NULL;

	ret = setup_interface(&api, cmd->opts->i2c_bus, cmd->opts->i2c_addr);
	if (ret)
		return ret;

	if (cmd->action == EEPROM_LIST)
		return print_i2c_accessible(cmd);

	if (cmd->action == EEPROM_CLEAR) {
		memset(buf, 0xff, EEPROM_SIZE);
		return write_eeprom(buf);
	}

	layout = prepare_layout(cmd);
	if (!layout)
		return -1;

	switch(cmd->action) {
	case EEPROM_READ:
		layout->print(layout);
		ret = 0;
		goto done;
	case EEPROM_WRITE_FIELDS:
		if (!layout->update_fields(layout, cmd->data)) {
			ret = -1;
			goto done;
		}
		break;
	case EEPROM_WRITE_BYTES:
		if (!layout->update_bytes(layout, cmd->data)) {
			ret = -1;
			goto done;
		}
		break;
	case EEPROM_CLEAR_FIELDS:
		if (!layout->clear_fields(layout, cmd->data)) {
			ret = -1;
			goto done;
		}
		break;
	case EEPROM_CLEAR_BYTES:
		if (!layout->clear_bytes(layout, cmd->data)) {
			ret = -1;
			goto done;
		}
		break;
	default:
		goto done;
	}

	ret = write_eeprom(layout->data);

done:
	free_layout(layout);
	return ret;
}

struct command *new_command(enum action action, struct options *options,
		struct data_array *data)
{
	struct command *cmd = malloc(sizeof(struct command));
	if (!cmd)
		return cmd;

	cmd->action = action;
	cmd->opts = options;
	cmd->data = data;
	cmd->execute = execute_command;

	return cmd;
}

void free_command(struct command *cmd)
{
	free(cmd);
}
