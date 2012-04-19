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
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <time.h>
#include <linux/i2c-dev.h>
#include <unistd.h>
#include <malloc.h>
#include <math.h>
#include "eeprom.h"

static int check_io_params(char *buf, enum eeprom_cmd function,
			enum access_mode mode, int offset, int size);
static int eeprom_do_io(struct eeprom e, enum eeprom_cmd function,
			enum access_mode mode, char *buf, int offset, int size);
static int open_device_file(struct eeprom e, enum access_mode mode, int flags);
static int do_safe_io(int fd, char *buf, enum eeprom_cmd function, int size);
static int do_i2c_io(int fd, char *buf, enum eeprom_cmd function, int size,
			int offset);
static void msleep(unsigned int msecs);

/*
 * Allocates and initializes a new eeprom struct with default values, or user
 * specified ones.
 * Input:
 *	driver_path: path to driver device file. Input NULL for default value.
 *	i2c_path: path to i2c device file. Input NULL for default value.
 *	i2c_addr: address for i2c. Input <0 for default value.
 */
struct eeprom *new_eeprom(char *driver_path, char *i2c_path, int i2c_addr)
{
	struct eeprom *e = (struct eeprom *) malloc(sizeof(struct eeprom));
	if (e == NULL)
		return e;

	e->driver_devfile = (driver_path != NULL) ? driver_path :
							DEFAULT_DRIVER_PATH;
	e->i2c_devfile = (i2c_path != NULL) ? i2c_path : DEFAULT_I2C_PATH;
	e->i2c_addr = (i2c_addr >= 0) ? i2c_addr : DEFAULT_I2C_ADDR;

	return e;
}

/*
 * The following return values apply to both eeprom_read and eeprom_write.
 * On success: returns number of bytes written.
 * On failure: a value from enum eeprom_errors, sans EEPROM_IO_FAILED.
 */
int eeprom_read(struct eeprom e, char *buf, int offset, int size,
		enum access_mode mode)
{
	int res = eeprom_do_io(e, EEPROM_READ, mode, buf, offset, size);
	return res == EEPROM_IO_FAILED ? EEPROM_READ_FAILED : res;
}

int eeprom_write(struct eeprom e, char *buf, int offset, int size,
		enum access_mode mode)
{
	int res = eeprom_do_io(e, EEPROM_WRITE, mode, buf, offset, size);
	return res == EEPROM_IO_FAILED ? EEPROM_WRITE_FAILED : res;
}

static int check_io_params(char *buf, enum eeprom_cmd function,
			enum access_mode mode, int offset, int size)
{
	if (buf == NULL)
		return EEPROM_NULL_PTR;

	if (function != EEPROM_READ && function != EEPROM_WRITE)
		return EEPROM_NO_SUCH_FUNCTION;

	if (mode != EEPROM_I2C_MODE && mode != EEPROM_DRIVER_MODE)
		return EEPROM_INVAL_MODE;

	if (offset < 0)
		return EEPROM_INVAL_OFFSET;

	if (offset + size > EEPROM_SIZE)
		return EEPROM_INVAL_SIZE;

	return 0;
}

/*
 * The actual I/O function (not a wrapper).
 * On success: returns number of bytes transferred.
 * On failure: returns values from eeprom_errors.
 */
static int eeprom_do_io(struct eeprom e, enum eeprom_cmd function,
			enum access_mode mode, char *buf, int offset, int size)
{
	int res, fd;
	res = check_io_params(buf, function, mode, offset, size);
	if (res < 0)
		return res;

	fd = open_device_file(e, mode, O_RDWR);
	if (fd < 0)
		return fd;

	if (mode == EEPROM_DRIVER_MODE) {
		lseek(fd, offset, SEEK_SET);
		res = do_safe_io(fd, buf + offset, function, size);
	} else {
		res = do_i2c_io(fd, buf + offset, function, size, offset);
	}

	close(fd);
	if (res <= 0)
		return EEPROM_IO_FAILED;

	return res;
}

/*
 * Prepares a device file fd for read or write.
 * flags are the standard file open flags.
 * On success:	returns the fd.
 * On failure:
 *	EEPROM_OPEN_FAILED:	catchall retval for file open errors.
 *	EEPROM_NO_I2C_ACCESS:	couldn't point i2c to the given address.
 *	EEPROM_INVALID_MODE:	mode is illegal.
 */
static int open_device_file(struct eeprom e, enum access_mode mode, int flags)
{
	int fd;
	if (mode == EEPROM_DRIVER_MODE)
		fd = open(e.driver_devfile, flags);
	else if (mode == EEPROM_I2C_MODE)
		fd = open(e.i2c_devfile, flags);
	else
		return EEPROM_INVAL_MODE;

	if (fd < 0)
		return EEPROM_OPEN_FAILED;

	if (mode == EEPROM_I2C_MODE) {
		if (ioctl(fd, I2C_SLAVE_FORCE, e.i2c_addr) < 0) {
			close(fd);
			return EEPROM_NO_I2C_ACCESS;
		}
	}

	return fd;
}

/*
 * Makes sure I/O is done despite possible interruptions.
 * Returns the output of read/write.
 * As a special case, if io_type is illegal returns 0 and
 * does nothing. It is safe to use 0 for an error since the target is an EEPROM
 * which always has data to be read and room for writes, so 0
 * will never be the result of a succesful read/write operation.
 */
static int do_safe_io(int fd, char *buf, enum eeprom_cmd function, int size)
{
	int bytes, offset = 0;

	do {
		if (function == EEPROM_READ)
			bytes = read(fd, buf + offset, size - offset);
		else if (function == EEPROM_WRITE)
			bytes = write(fd, buf + offset, size - offset);
		else
			return 0;

		offset += bytes;
	} while (bytes >= 0 && offset < size);

	if (bytes < 0)
		return bytes;

	return offset;
}

/*
 * do_safe_io works fine for most cases, but i2c writes to EEPROM are a special
 * case because writes wrap around EEPROM page size boundries, not around EEPROM
 * size. The result is that you have to change the write address manually.
 * Returns the output of the write operation.
 */
static int do_i2c_io(int fd, char *buf, enum eeprom_cmd function, int size,
		int offset)
{
	int res, page_cnt, end_page, i, bytes_written = 0;
	char i2c_buf[EEPROM_PAGE_SIZE + 1];

	/* Set the read/write location */
	while (write(fd, &offset, 1) < 0)
		msleep(5);

	if (function == EEPROM_READ)
		return do_safe_io(fd, buf, function, size);

	page_cnt = offset / EEPROM_PAGE_SIZE;
	end_page = ceil(page_cnt + size / EEPROM_PAGE_SIZE);
	for (; page_cnt < end_page; page_cnt++) {
		i2c_buf[0] = page_cnt * EEPROM_PAGE_SIZE; /* Write address */
		for (i = 0; i < EEPROM_PAGE_SIZE && i < size; i++)
			i2c_buf[i+1] = buf[page_cnt * EEPROM_PAGE_SIZE + i];

		size -= EEPROM_PAGE_SIZE;
		do {
			res = write(fd, i2c_buf, i + 1);
			/* A delay is necessary, otherwise next writes fail. */
			msleep(5);
		} while (res != i + 1);

		bytes_written += res - 1;
		if (res < 0)
			return res;
	}

	return bytes_written;
}

/*
 * This function supplies the appropriate delay needed for consecutive writes
 * via i2c to succeed (5mS write timeout)
 */
static void msleep(unsigned int msecs)
{
	struct timespec time = {0, 1000000 * msecs};
	nanosleep(&time, NULL);
}
