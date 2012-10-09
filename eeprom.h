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

#ifndef _EEPROM_
#define _EEPROM_

#define EEPROM_SIZE		256
#define EEPROM_PAGE_SIZE	16

#define DEFAULT_DRIVER_PATH	"/sys/bus/i2c/devices/3-0050/eeprom"
#define DEFAULT_I2C_PATH	"/dev/i2c-3"
#define DEFAULT_I2C_ADDR	0x50

enum eeprom_errors {
	EEPROM_SUCCESS = 0,
	EEPROM_NULL_PTR,
	EEPROM_INVAL_MODE,
	EEPROM_INVAL_OFFSET,
	EEPROM_INVAL_SIZE,
	EEPROM_OPEN_FAILED,
	EEPROM_NO_SUCH_FUNCTION,
	EEPROM_NO_I2C_ACCESS,
	EEPROM_IO_FAILED,
};

enum action {
	EEPROM_READ,
	EEPROM_WRITE,
	EEPROM_LIST,
	EEPROM_ACTION_INVALID,
};

enum mode {
	EEPROM_DRIVER_MODE,
	EEPROM_I2C_MODE,
	EEPROM_MODE_INVALID,
};

int i2c_probe(int fd, int address);
int eeprom_driver_io(int fd, enum action function, unsigned char *buf,
		     int offset, int size);
int eeprom_i2c_io(int fd, enum action function, unsigned char *buf, int offset,
		  int size);
int open_device_file(char *dev_file, enum mode mode, int i2c_addr, int flags);

#endif
