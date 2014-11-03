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

#ifndef _COMMAND_
#define _COMMAND_

#include "common.h"
#include "layout.h"

enum action {
	EEPROM_READ,
	EEPROM_WRITE_FIELDS,
	EEPROM_WRITE_BYTES,
	EEPROM_LIST,
	EEPROM_ACTION_INVALID,
};

struct command {
	enum action action;
	int i2c_bus;
	int i2c_addr;
	enum layout_version layout_ver;
	int new_data_size;
	struct strings_pair *new_field_data;

	void (*execute)(struct command *cmd);
};

struct command *new_command(enum action action, int i2c_bus, int i2c_addr,
			    enum layout_version layout_ver, int new_data_size,
			    struct strings_pair *new_field_data);
void free_command(struct command *cmd);

#endif
