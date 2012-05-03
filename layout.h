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
#ifndef _LAYOUT_
#define _LAYOUT_

#include "field.h"

struct layout {
	struct field *fields;
	int layout_number;
	char *data;
	int data_size;
};

struct layout *new_layout(char *buf, int buf_size);
void free_layout(struct layout *layout);
void print_layout(struct layout *layout);
enum layout_res update_field(struct layout *layout, char *field_name,
								char *new_data);
enum layout_res update_byte(struct layout *layout, int offset, char new_byte);

enum layout_names {
	LAYOUT_LEGACY,
	LAYOUT_VER1,
	LAYOUT_VER2,
	LAYOUT_INCORRECT,
};

enum layout_res {
	LAYOUT_SUCCESS = 0,
	LAYOUT_NULL_ARGUMENTS,
	LAYOUT_OFFSET_OUT_OF_BOUNDS,
	LAYOUT_NO_SUCH_FIELD,
};

#endif
