/*
 * Copyright (C) 2009-2017 CompuLab, Ltd.
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

#ifndef _LAYOUT_
#define _LAYOUT_

#include "common.h"

#define EEPROM_SIZE 256

enum layout_version {
	LAYOUT_AUTODETECT = -1,
	LAYOUT_LEGACY,
	LAYOUT_VER1,
	LAYOUT_VER2,
	LAYOUT_VER3,
	LAYOUT_VER4,
	LAYOUT_UNRECOGNIZED, /* marks the end of the layout versions */
	RAW_DATA,
};

struct layout {
	struct field *fields;
	int num_of_fields;
	enum layout_version layout_version;
	unsigned char *data;
	int data_size;
	void (*print)(const struct layout *layout);
	int (*update_fields)(struct layout *layout,
			     struct data_array *data);
	int (*clear_fields)(struct layout *layout,
			    struct data_array *data);
	int (*update_bytes)(struct layout *layout,
			    struct data_array *data);
	int (*clear_bytes)(struct layout *layout,
			   struct data_array *data);
};

struct layout *new_layout(unsigned char *buf, unsigned int buf_size,
			  enum layout_version layout_version);
void free_layout(struct layout *layout);

#endif
