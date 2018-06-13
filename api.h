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

#ifndef API_H_
#define API_H_

struct api {
	int fd;

	int (*read)(struct api *api, unsigned char *buf, int offset, int size);
	int (*write)(struct api *api, unsigned char *buf, int offset, int size);
	int (*probe)(int bus);
	void (*system_error)(const char *message);
};

int setup_interface(struct api *api, int i2c_bus, int i2c_addr);

#endif
