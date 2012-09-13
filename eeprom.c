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

#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <time.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include "eeprom.h"

static int check_io_params(unsigned char *buf, enum action function,
			enum mode mode, int offset, int size)
{
	if (buf == NULL)
		return -EEPROM_NULL_PTR;

	if (function != EEPROM_READ && function != EEPROM_WRITE)
		return -EEPROM_NO_SUCH_FUNCTION;

	if (mode != EEPROM_I2C_MODE && mode != EEPROM_DRIVER_MODE)
		return -EEPROM_INVAL_MODE;

	if (offset < 0)
		return -EEPROM_INVAL_OFFSET;

	if (offset + size > EEPROM_SIZE)
		return -EEPROM_INVAL_SIZE;

	return 0;
}

/*
 * Prepares a device file fd for read or write.
 * flags are the standard file open flags.
 * On success:	returns the fd.
 * On failure: negative values of:
 *	EEPROM_OPEN_FAILED:	catchall retval for file open errors.
 *	EEPROM_NO_I2C_ACCESS:	couldn't point i2c to the given address.
 *	EEPROM_INVALID_MODE:	mode is illegal.
 */
static int open_device_file(struct command command, int flags)
{
	int fd;

	if (command.mode == EEPROM_I2C_MODE ||
	    command.mode == EEPROM_DRIVER_MODE)
		fd = open(command.dev_file, flags);
	else
		return -EEPROM_INVAL_MODE;

	if (fd < 0)
		return -EEPROM_OPEN_FAILED;

	if (command.mode == EEPROM_I2C_MODE) {
		if (ioctl(fd, I2C_SLAVE_FORCE, command.i2c_addr) < 0) {
			close(fd);
			return -EEPROM_NO_I2C_ACCESS;
		}
	}

	return fd;
}

/*
 * This function supplies the appropriate delay needed for consecutive writes
 * via i2c to succeed
 */
static void msleep(unsigned int msecs)
{
	struct timespec time = {0, 1000000 * msecs};
	nanosleep(&time, NULL);
}

static inline __s32 i2c_smbus_access(int file, char read_write, __u8 command,
				     int size, union i2c_smbus_data *data)
{
	struct i2c_smbus_ioctl_data args;

	args.read_write = read_write;
	args.command = command;
	args.size = size;
	args.data = data;

	return ioctl(file, I2C_SMBUS, &args);
}

/*
 * I2C mode IO function.
 * On success: returns number of bytes transferred.
 * On failure: enum eeprom_errors.
 */
static int eeprom_i2c_io(struct command command, enum action function,
			 unsigned char *buf, int offset, int size)
{
	int res, fd, i, bytes_transferred = 0;
	union i2c_smbus_data data;

	res = check_io_params(buf, function, EEPROM_I2C_MODE, offset, size);
	if (res < 0)
		return res;

	fd = open_device_file(command, O_RDWR);
	if (fd < 0)
		return fd;

	/* Reset the reading pointer of the EEPROM to offset 0 */
	i2c_smbus_access(fd, I2C_SMBUS_WRITE, 0, I2C_SMBUS_BYTE, NULL);
	if (function == EEPROM_READ) {
		for (i = offset; i < size; i++) {
			if (i2c_smbus_access(fd, I2C_SMBUS_READ, i,
					     I2C_SMBUS_BYTE, &data) < 0)
				return -EEPROM_IO_FAILED;

			buf[i] = (unsigned char)(data.byte & 0xFF);
			bytes_transferred++;
		}

		return bytes_transferred;
	}

	/* function == EEPROM_WRITE */
	for (i = offset; i < size; i++) {
		data.byte = buf[i];
		if (i2c_smbus_access(fd, I2C_SMBUS_WRITE, i,
				     I2C_SMBUS_BYTE_DATA, &data) < 0)
			return -EEPROM_IO_FAILED;

		msleep(5);
		bytes_transferred++;
	}

	return bytes_transferred;
}

/*
 * Driver mode IO function.
 * On success: returns number of bytes transferred.
 * On failure: returns negative values of eeprom_errors.
 */
static int eeprom_driver_io(struct command command, enum action function,
			    unsigned char *buf, int offset, int size)
{
	int res, fd;

	res = check_io_params(buf, function, EEPROM_DRIVER_MODE, offset, size);
	if (res < 0)
		return res;

	fd = open_device_file(command, O_RDWR);
	if (fd < 0)
		return fd;

	lseek(fd, offset, SEEK_SET);
	if (function == EEPROM_READ)
		res = read(fd, buf + offset, size);
	else if (function == EEPROM_WRITE)
		res = write(fd, buf + offset, size);

	close(fd);
	if (res <= 0)
		return -EEPROM_IO_FAILED;

	return res;
}

/*
 * Tries to read from i2c dev file at the given address.
 * Returns 1 if read succeeded, 0 otherwise.
 */
int i2c_probe(int fd, int address)
{
	struct i2c_smbus_ioctl_data args;
	union i2c_smbus_data data;

	if (ioctl(fd, I2C_SLAVE_FORCE, address) < 0)
		return 0;

	args.read_write = I2C_SMBUS_READ;
	args.command = 0;
	args.size = I2C_SMBUS_BYTE;
	args.data = &data;
	if (ioctl(fd, I2C_SMBUS, &args) < 0)
		return 0;

	return 1;
}

/*
 * eeprom_read and eeprom_write:
 * On success: returns number of bytes written.
 * On failure: negative values of enum eeprom_errors, sans EEPROM_IO_FAILED.
 */
int eeprom_read(struct command command, unsigned char *buf, int offset, int size)
{
	if (command.mode == EEPROM_DRIVER_MODE)
		return eeprom_driver_io(command, EEPROM_READ, buf, offset, size);
	else /* mode == EEPROM_I2C_MODE) */
		return eeprom_i2c_io(command, EEPROM_READ, buf, offset, size);
}

int eeprom_write(struct command command, unsigned char *buf, int offset, int size)
{
	if (command.mode == EEPROM_DRIVER_MODE)
		return eeprom_driver_io(command, EEPROM_WRITE, buf, offset, size);
	else /* mode == EEPROM_I2C_MODE) */
		return eeprom_i2c_io(command, EEPROM_WRITE, buf, offset, size);
}
