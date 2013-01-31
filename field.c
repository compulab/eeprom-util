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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "field.h"

#define PRINT_FIELD_SEGMENT	"%-30s"

/*
 * set_field() - initialize a field object
 * @name:	field name
 * @size:	field size in bytes
 * @delim:	a string to print in between values
 * @print:	a function for printing this field
 * @update:	a function for updating this field
 *
 * Returns: an initialized field
 */
struct field set_field(char *name, int size, char *delim,
			void (*print)(const struct field *field),
			void (*update)(struct field *field, char *value))
{
	struct field f;

	f.name = name;
	f.size = size;
	f.delim = delim;
	f.print = print;
	f.update = update;

	return f;
}

/*
 * print_bin_ver() - print a "version field" which contains binary data
 * @field:	an initialized field to print
 */
void print_bin_ver(const struct field *field)
{
	if ((field->buf[0] == 0xff) && (field->buf[1] == 0xff)) {
		field->buf[0] = 0;
		field->buf[1] = 0;
	}

	printf(PRINT_FIELD_SEGMENT, field->name);
	printf("%#.2f\n", (field->buf[1] << 8 | field->buf[0]) / 100.0);
}

/*
 * print_bin_rev() - print in reverse a field which contains binary data
 * @field:	an initialized field to print
 */
void print_bin_rev(const struct field *field)
{
	int i;

	printf(PRINT_FIELD_SEGMENT, field->name);
	for (i = field->size - 1; i > 0; i--)
		printf("%02x%s", field->buf[i], field->delim);

	printf("%02x\n", field->buf[0]);
}

/*
 * print_bin() - print a field which contains binary data
 * @field:	an initialized field to print
 */
void print_bin(const struct field *field)
{
	int i;

	printf(PRINT_FIELD_SEGMENT, field->name);
	for (i = 0; i < field->size - 1; i++)
		printf("%02x%s", field->buf[i], field->delim);

	printf("%02x\n", field->buf[field->size - 1]);
}

/*
 * print_date() - print a field which contains date data
 * @field:	an initialized field to print
 */
void print_date(const struct field *field)
{
	char *months[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
			    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

	printf(PRINT_FIELD_SEGMENT, field->name);
	printf("%d%s", field->buf[0], field->delim);
	if (field->buf[1] >= 1 && field->buf[1] <= 12)
		printf("%s", months[field->buf[1] - 1]);
	else
		printf("BAD");

	printf("%s%d\n", field->delim, field->buf[3] << 8 | field->buf[2]);
}

/*
 * print_ascii() - print a field which contains ASCII data
 * @field:	an initialized field to print
 */
void print_ascii(const struct field *field)
{
	char format[8];

	sprintf(format, "%%.%ds\n", field->size);
	printf(PRINT_FIELD_SEGMENT, field->name);
	printf(format, field->buf);
}

/*
 * print_reserved() - print the "Reserved fields" field
 * @field:	an initialized field to print
 */
void print_reserved(const struct field *field)
{
	printf(PRINT_FIELD_SEGMENT, "Reserved fields\t");
	printf("(%d bytes)\n", field->size);
}

/*
 * update_binary() - Update field with new data in binary form
 * @field:	an initialized field
 * @value:	a space delimited string of byte values (i.e. "1 2 3 4")
 */
void update_binary(struct field *field, char *value)
{
	int i;
	char *tok = strtok(value, " ");

	for (i = 0; tok && i < field->size; i++) {
		if (strcmp(tok, ""))
			field->buf[i] = (unsigned char)strtol(tok, 0, 0);

		tok = strtok(NULL, " ");
	}
}

/*
 * update_ascii() - Update field with new data in ASCII form
 * @field:	an initialized field
 * @value:	the new string data
 */
void update_ascii(struct field *field, char *value)
{
	strncpy((char *)field->buf, value, field->size - 1);
	field->buf[field->size - 1] = '\0';
}
