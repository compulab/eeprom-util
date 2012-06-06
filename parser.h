/*
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
 * along with this program; if not, write to the Free Software
 * Foundation.
 */
/* ------------------------------------------------------------------------- */
#ifndef _PARSER_
#define _PARSER_

enum cli_action {
	READ,
	WRITE,
	LIST,
	ACTION_INVALID
};

enum cli_mode {
	DRIVER_MODE,
	I2C_MODE,
	MODE_INVALID
};

struct cli_command {
	enum cli_action action;
	enum cli_mode mode;
	int i2c_addr;
	char *dev_file;
	char *new_byte_data;
	char **new_field_data;
};

void parse(int argc, char *argv[], struct cli_command *cli_cmd);

#endif
