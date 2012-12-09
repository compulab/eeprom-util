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

/*====================Print field functions==================*/
/* For printing the binary "version" field. */
void print_bin_ver(const struct field *field)
{
	if ((field->buf[0] == 0xff) && (field->buf[1] == 0xff)) {
		field->buf[0] = 0;
		field->buf[1] = 0;
	}

	printf(PRINT_FIELD_SEGMENT, field->name);
	printf("%#.2f\n", (field->buf[1] << 8 | field->buf[0]) / 100.0);
}

/* For printing binary data in reverse. */
void print_bin_rev(const struct field *field)
{
	int i;

	printf(PRINT_FIELD_SEGMENT, field->name);
	for (i = field->size - 1; i > 0; i--)
		printf("%02x%s", field->buf[i], field->delim);

	printf("%02x\n", field->buf[0]);
}

/* For printing binary data. */
void print_bin(const struct field *field)
{
	int i;

	printf(PRINT_FIELD_SEGMENT, field->name);
	for (i = 0; i < field->size - 1; i++)
		printf("%02x%s", field->buf[i], field->delim);

	printf("%02x\n", field->buf[field->size - 1]);
}

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

/* For printing data meant to be interpreted as an ASCII string. */
void print_ascii(const struct field *field)
{
	char format[8];

	sprintf(format, "%%.%ds\n", field->size);
	printf(PRINT_FIELD_SEGMENT, field->name);
	printf(format, field->buf);
}

/* For printing the "Reserved field" section. */
void print_reserved(const struct field *field)
{
	printf(PRINT_FIELD_SEGMENT, "Reserved fields\t");
	printf("(%d bytes)\n", field->size);
}

/*====================Update field functions==================*/
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

void update_ascii(struct field *field, char *value)
{
	strncpy((char *)field->buf, value, field->size - 1);
	field->buf[field->size - 1] = '\0';
}
