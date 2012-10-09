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
#include "command.h"

void print_command(const struct command command)
{
	if (command.action == EEPROM_READ)
		printf("Reading ");
	else
		printf("Writing ");

	if (command.mode == EEPROM_DRIVER_MODE)
		printf("via driver at %s\n", command.dev_file);
	else
		printf("via i2c at %s, from address 0x%x\n",
		       command.dev_file, command.i2c_addr);
}
