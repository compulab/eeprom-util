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

void print_command(const struct command *command)
{
	if (command->action == EEPROM_READ)
		printf("Reading ");
	else
		printf("Writing ");

	if (command->mode == EEPROM_DRIVER_MODE)
		printf("via driver at %s\n", command->dev_file);
	else
		printf("via i2c at %s, from address 0x%x\n",
		       command->dev_file, command->i2c_addr);
}

/* This function is meant to be end user friendly, not debugging friendly. */
static void print_eeprom_error(int error)
{
	switch (-error) {
	case EEPROM_NULL_PTR:
		printf("Out of memory!\n");
		break;
	case EEPROM_OPEN_FAILED:
		printf("Could not open the device file.\n");
		break;
	case EEPROM_NO_I2C_ACCESS:
		printf("I2C communication with EEPROM failed.\nThis could be "
		"due to wrong address or wrong dev file (not an i2c file).\n");
		break;
	case EEPROM_IO_FAILED:
		printf("I/O failed. Check your path.\n");
	}
}

static void do_io(struct command *command)
{
	unsigned char buf[EEPROM_SIZE];
	struct layout *layout;
	int res, fd;
	int offset = 0, size = EEPROM_SIZE;

	fd = open_device_file(command->dev_file, command->mode,
					command->i2c_addr, O_RDWR);
	if (fd < 0) {
		print_eeprom_error(fd);
		return;
	}

	print_command(command);
	res = (command->mode == EEPROM_DRIVER_MODE) ?
		eeprom_driver_io(fd, EEPROM_READ, buf, offset, size) :
		eeprom_i2c_io(fd, EEPROM_READ, buf, offset, size);

	if (res < 0) {
		print_eeprom_error(res);
		goto close_fd;
	}

	layout = new_layout(buf, EEPROM_SIZE);
	if (layout == NULL) {
		print_eeprom_error(-EEPROM_NULL_PTR);
		goto close_fd;
	}

	if (command->action == EEPROM_READ) {
		layout->print(layout);
		goto free_layout;
	}

	if (command->new_byte_data != NULL)
		layout->update_bytes(layout, command->new_byte_data,
					command->new_data_size);
	else if (command->new_field_data != NULL)
		layout->update_fields(layout, command->new_field_data,
					command->new_data_size);

	res = (command->mode == EEPROM_DRIVER_MODE) ?
		eeprom_driver_io(fd, EEPROM_WRITE, layout->data, offset, size) :
		eeprom_i2c_io(fd, EEPROM_WRITE, layout->data, offset, size);

	if (res < 0)
		print_eeprom_error(res);

free_layout:
	free_layout(layout);
close_fd:
	close(fd);
}

void print_i2c_accessible(struct command *command)
{
	int i, j, fd;
	char dev_file_name[13];

	/*
	 * Documentation/i2c/dev-interface: "All 256 minor device numbers are
	 * reserved for i2c."
	 */
	for (i = 0; i < 256; i++) {
		sprintf(dev_file_name, "/dev/i2c-%d", i);
		fd = open(dev_file_name, O_RDWR);
		if (fd < 0)
			continue;

		printf("On i2c-%d:\n", i);
		printf("\t");
		for (j = 0; j < 128; j++) { /* Assuming 7 bit addresses here. */
			if (!i2c_probe(fd, j))
				continue;

			printf("0x%x ", j);
		}

		printf("\n");
		close(fd);
	}
}

int setup_command(struct command *cmd, enum action action, enum mode mode,
		int i2c_addr, char *dev_file,
		struct offset_value_pair *new_byte_data,
		struct strings_pair *new_field_data, int new_data_size)
{
	cmd->mode = mode;
	cmd->action = action;
	cmd->i2c_addr = i2c_addr;
	cmd->dev_file = dev_file;
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

	return 0;
}

void reset_command(struct command *command)
{
	command->mode = EEPROM_MODE_INVALID;
	command->i2c_addr = -1;
	command->dev_file = NULL;
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
