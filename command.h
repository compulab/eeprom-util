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

#include "eeprom.h"
#include "pairs.h"

struct command {
	enum action action;
	enum mode mode;
	int i2c_addr;
	char *dev_file;
	struct offset_value_pair *new_byte_data;
	struct strings_pair *new_field_data;
	int new_data_size; /* Used for both new_*_data arrays */
};

void print_command(const struct command command);

#endif
