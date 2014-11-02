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
			int (*update)(struct field *field, char *value))
{
	struct field f;

	f.name = name;
	f.size = size;
	f.delim = delim;
	f.print = print;
	f.update = update;

	return f;
}

static void __print_bin(const struct field *field, char *delimiter, int reverse)
{
	printf(PRINT_FIELD_SEGMENT, field->name);
	int i;
	int from = reverse ? field->size - 1 : 0;
	int to = reverse ? 0 : field->size - 1;
	for (i = from; i != to; reverse ? i-- : i++)
		printf("%02x%s", field->buf[i], delimiter);

	printf("%02x\n", field->buf[i]);
}

static int __update_bin(struct field *field, char *value,
			char *delimiter, int reverse)
{
	int count = 0;
	const char *tmp = value;
	tmp = strstr(tmp, delimiter);
	while (tmp != NULL) {
	   count++;
	   tmp++;
	   tmp = strstr(tmp, delimiter);
	}

	if (count > field->size)
		return -1;

	char *tok = strtok(value, delimiter);
	int from = reverse ? field->size - 1 : 0;
	int to = reverse ? -1 : field->size;
	for (int i = from; tok && i != to; reverse ? i-- : i++) {
		field->buf[i] = (unsigned char)strtol(tok, 0, 0);
		tok = strtok(NULL, delimiter);
	}

	return 0;
}

/**
 * print_bin() - print a field which contains binary data
 *
 * Treat the field data as simple binary data, and print it as two digit
 * hexadecimal values separated by spaces.
 * Sample output:
 * 	Field Name	01 02 03 04 05 06 07 08 09 0a
 *
 * @field:	an initialized field to print
 */
void print_bin(const struct field *field)
{
	__print_bin(field, " ", 0);
}

/**
 * update_bin() - Update field with new data in binary form
 *
 * @field:	an initialized field
 * @value:	a space delimited string of byte values (i.e. "1 02 3 0x4")
 */
int update_bin(struct field *field, char *value)
{
	return __update_bin(field, value, " ", 0);
}

/**
 * print_bin_rev() - print a field which contains binary data in reverse order
 *
 * Treat the field data as simple binary data, and print it in reverse order
 * as two digit hexadecimal values separated by spaces.
 *
 * Data in field:
 *  			01 02 03 04 05 06 07 08 09 0a
 * Sample output:
 * 	Field Name	0a 09 08 07 06 05 04 03 02 01
 *
 * @field:	an initialized field to print
 */
void print_bin_rev(const struct field *field)
{
	__print_bin(field, " ", 1);
}

/**
 * print_bin_ver() - print a "version field" which contains binary data
 *
 * Treat the field data as simple binary data, and print it formatted as a
 * version number (2 digits after decimal point).
 * The field size must be exactly 2 bytes.
 *
 * Sample output:
 * 	Field Name	123.45
 *
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

char *months[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
		    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

/**
 * print_date() - print a field which contains date data
 *
 * Treat the field data as simple binary data, and print it formatted as a date.
 * Sample output:
 * 	Field Name	07/Feb/2014
 * 	Field Name	56/BAD/9999
 *
 * @field:	an initialized field to print
 */
void print_date(const struct field *field)
{
	printf(PRINT_FIELD_SEGMENT, field->name);
	printf("%d/", field->buf[0]);
	if (field->buf[1] >= 1 && field->buf[1] <= 12)
		printf("%s", months[field->buf[1] - 1]);
	else
		printf("BAD");

	printf("/%d\n", field->buf[3] << 8 | field->buf[2]);
}

/**
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

/**
 * update_ascii() - Update field with new data in ASCII form
 * @field:	an initialized field
 * @value:	the new string data
 *
 * Returns 0 on success, -1 of failure (new string too long).
 */
int update_ascii(struct field *field, char *value)
{
	if (strlen(value) >= field->size) {
		printf("%s: new data too long\n", field->name);
		return -1;
	}

	strncpy((char *)field->buf, value, field->size - 1);
	field->buf[field->size - 1] = '\0';

	return 0;
}

/**
 * print_reserved() - print the "Reserved fields" field
 *
 * Print a notice that the following field_size bytes are reserved.
 *
 * Sample output:
 * 	Reserved fields	              (64 bytes)
 *
 * @field:	an initialized field to print
 */
void print_reserved(const struct field *field)
{
	printf(PRINT_FIELD_SEGMENT, "Reserved fields\t");
	printf("(%d bytes)\n", field->size);
}
