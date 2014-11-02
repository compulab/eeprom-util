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

struct field {
	char *name;
	int size;
	unsigned char *buf;

	void (*print)(const struct field *field);
	int (*update)(struct field *field, char *value);
};

struct field set_field(char *name, int size,
			void (*print)(const struct field *field),
			int (*update)(struct field *field, char *value));

void print_bin(const struct field *field);
int update_bin(struct field *field, char *value);

void print_bin_ver(const struct field *field);
int update_bin_ver(struct field *field, char *value);

void print_bin_rev(const struct field *field);
int update_bin_rev(struct field *field, char *value);

void print_mac(const struct field *field);
int update_mac(struct field *field, char *value);

void print_date(const struct field *field);
int update_date(struct field *field, char *value);

void print_ascii(const struct field *field);
int update_ascii(struct field *field, char *value);

void print_reserved(const struct field *field);

#endif
