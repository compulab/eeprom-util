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

#ifndef _LAYOUT_
#define _LAYOUT_

#include "field.h"

enum layout_res {
	LAYOUT_SUCCESS = 0,
	LAYOUT_NULL_ARGUMENTS,
	LAYOUT_OFFSET_OUT_OF_BOUNDS,
	LAYOUT_NO_SUCH_FIELD,
};

enum layout_version {
	LAYOUT_UNRECOGNIZED,
	LAYOUT_LEGACY,
	LAYOUT_VER1,
	LAYOUT_VER2,
};

struct layout {
	struct field *fields;
	int num_of_fields;
	enum layout_version layout_version;
	unsigned char *data;
	int data_size;
	void (*print)(const struct layout *layout);
	enum layout_res (*update_field)(struct layout *layout, char *field_name,
					char *new_data);
	enum layout_res (*update_byte)(struct layout *layout,
				       unsigned int offset, char new_byte);
};

struct layout *new_layout(unsigned char *buf, unsigned int buf_size);
void free_layout(struct layout *layout);

#endif
