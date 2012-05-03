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
 */
/* ------------------------------------------------------------------------- */
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <unistd.h>
#include <malloc.h>
#include <linux/i2c-dev.h>
#include "eeprom.h"
#include "layout.h"
#include "parser.h"

#define EEPROM_MODE(mode) ((mode) == DRIVER_MODE) ? EEPROM_DRIVER_MODE : \
								EEPROM_I2C_MODE

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
	case EEPROM_READ_FAILED:
	case EEPROM_WRITE_FAILED:
		printf("I/O failed. Check your path.\n");
	}
}

/*
 * This function takes a command that points to a string in the form of
 * <int>,<int>[,<int>,<int>]*, which is a list of tuples that stand for
 * (offset, new_byte). It does the actual updating within the layout struct.
 */
static void update_bytes(struct layout *layout, struct cli_command *command)
{
	int offset, value, res;
	char *tok = strtok(command->new_byte_data, ",");

	if (tok == NULL)
		return;

	do {
		offset = strtol(tok, 0, 0);
		tok = strtok(NULL, ",");
		if (tok == NULL)
			return;

		value = strtol(tok, 0, 0);
		res = update_byte(layout, offset, value);
		if (res == -LAYOUT_OFFSET_OUT_OF_BOUNDS)
			printf("Offset %d out of bounds. "
				"Did not update.\n", offset);

		tok = strtok(NULL, ",");
	} while (tok != NULL);
}

/*
 * This function takes a command that points to an array of strings in the
 * form of: field name=new value(s), and does the actual updating within the
 * layout struct.
 */
static void update_fields(struct layout *layout, struct cli_command *command)
{
	int i, res;
	char *field_name, *value;

	for (i = 0; command->new_field_data[i] != NULL; i++) {
		field_name = strtok(command->new_field_data[i], "=");
		value = strtok(NULL, "=");
		res = update_field(layout, field_name, value);
		if (res == -LAYOUT_NO_SUCH_FIELD)
			printf("'%s' is not a valid field. "
				"Skipping update", field_name);
	}
}

static void do_io(struct cli_command command)
{
	int res;
	char buf[EEPROM_SIZE];
	struct eeprom eeprom;
	struct layout *layout;

	if (command.mode == DRIVER_MODE)
		eeprom_set_params(&eeprom, command.dev_file, NULL, -1);
	else
		eeprom_set_params(&eeprom, NULL, command.dev_file,
							command.i2c_addr);

	res = eeprom_read(eeprom, buf, 0, EEPROM_SIZE,
						EEPROM_MODE(command.mode));
	if (res < 0) {
		print_eeprom_error(res);
		return;
	}

	layout = new_layout(buf, EEPROM_SIZE);
	if (layout == NULL)
		goto out_of_memory;

	if (command.action == READ) {
		print_layout(layout);
		goto free_layout;
	}

	if (command.new_byte_data != NULL)
		update_bytes(layout, &command);
	else if (command.new_field_data != NULL)
		update_fields(layout, &command);

	res = eeprom_write(eeprom, layout->data, 0, EEPROM_SIZE,
						EEPROM_MODE(command.mode));
	if (res < 0)
		print_eeprom_error(res);

free_layout:
	free_layout(layout);

	return;
out_of_memory:
	printf("Out of memory!\n");
}

void print_i2c_accessible()
{
	int i, j, fd;
	char dev_file_name[13];
	char buf;

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
			if (ioctl(fd, I2C_SLAVE_FORCE, j) < 0)
				continue;

			if (read(fd, &buf, 1) < 0)
				continue;

			printf("0x%x ", j);
		}

		printf("\n");
		close(fd);
	}
}

/*=================================================================*/
int main(int argc, char *argv[])
{
	struct cli_command command = parse(argc, argv);

	if (command.action == LIST)
		print_i2c_accessible();
	else /* READ/WRITE */
		do_io(command);

	return 0;
}
