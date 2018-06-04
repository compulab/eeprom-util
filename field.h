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

#ifndef _FIELD_
#define _FIELD_

#include <stdbool.h>

enum field_type {
	FIELD_BINARY,
	FIELD_REVERSED,
	FIELD_VERSION,
	FIELD_ASCII,
	FIELD_MAC,
	FIELD_DATE,
	FIELD_RESERVED,
	FIELD_RAW,
};

struct field {
	char *name;
	char *short_name;
	int data_size;
	enum field_type type;
	unsigned char *data;
	struct field_ops *ops;
};

struct field_ops {
	int (*get_data_size)(const struct field *field);
	bool (*is_named)(const struct field *field, const char *str);
	void (*print)(const struct field *field);
	int (*update)(struct field *field, char *value);
	void (*clear)(struct field *field);
};

void init_field(struct field *field, unsigned char *data);

#endif
