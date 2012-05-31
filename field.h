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
#ifndef _FIELD_
#define _FIELD_

struct field {
	char *name;
	int size;
	/* Tells printing functions what to print between data members. */
	char *delim;
	unsigned char *buf;

	void (*print)(struct field self);
	void (*update)(struct field *self, char *value);
};

struct field set_field(char *name, int size, char *delim,
			void (*print)(struct field self),
			void (*update)(struct field *self, char *value));

void print_bin_ver(struct field self);
void print_bin_rev(struct field self);
void print_bin(struct field self);
void print_date(struct field self);
void print_ascii(struct field self);
void print_reserved(struct field self);

void update_binary(struct field *self, char *value);
void update_ascii(struct field *self, char *value);

#endif
