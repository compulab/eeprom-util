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
#include <stdbool.h>
#include "common.h"
#include "field.h"

// Macro for printing field's input value error messages
#define iveprintf(str, value, name) \
	ieprintf("Invalid value \"%s\" for field \"%s\" - " str, value, name);

static void __print_bin(const struct field *field,
			char *delimiter, bool reverse)
{
	ASSERT(field && field->data && delimiter);

	int i;
	int from = reverse ? field->data_size - 1 : 0;
	int to = reverse ? 0 : field->data_size - 1;
	for (i = from; i != to; reverse ? i-- : i++)
		printf("%02x%s", field->data[i], delimiter);

	printf("%02x\n", field->data[i]);
}

static int __update_bin(struct field *field, const char *value, bool reverse)
{
	ASSERT(field && field->data && field->name && value);

	int len = strlen(value);
	int i = reverse ? len - 1 : 0;

	/* each two characters in the string are fit in one byte */
	if (len > field->data_size * 2) {
		iveprintf("Value is too long", value, field->name);
		return -1;
	}

	/* pad with zeros */
	memset(field->data, 0, field->data_size);

	/* i - string iterator, j - data iterator */
	for (int j = 0; j < field->data_size; j++) {
		int byte = 0;
		char tmp[3] = { 0, 0, 0 };

		if ((reverse && i < 0) || (!reverse && i >= len))
			break;

		for (int k = 0; k < 2; k++) {
			if (reverse && i == 0) {
				tmp[k] = value[i];
				break;
			}

			tmp[k] = value[reverse ? i - 1 + k : i + k];
		}

		char *str = tmp;
		if (strtoi_base(&str, &byte, 16) < 0 || byte < 0 || byte >> 8) {
			iveprintf("Syntax error", value, field->name);
			return -1;
		}

		field->data[j] = (unsigned char)byte;
		i = reverse ? i - 2 : i + 2;
	}

	return 0;
}

static int __update_bin_delim(struct field *field, char *value, char delimiter)
{
	ASSERT(field && field->data && field->name && value);

	int i, val;
	char *bin = value;

	for (i = 0; i < (field->data_size - 1); i++) {
		if (strtoi_base(&bin, &val, 16) != STRTOI_STR_CON ||
		    *bin != delimiter || val < 0 || val >> 8) {
			iveprintf("Syntax error", value, field->name);
			return -1;
		}

		field->data[i] = (unsigned char)val;
		bin++;
	}

	if (strtoi_base(&bin, &val, 16) != STRTOI_STR_END ||
	    val < 0 || val >> 8) {
		iveprintf("Syntax error", value, field->name);
		return -1;
	}

	field->data[i] = (unsigned char)val;

	return 0;
}

/**
 * print_bin() - print the value of a field from type "binary"
 *
 * Treat the field data as simple binary data, and print it as two digit
 * hexadecimal values.
 * Sample output: 0102030405060708090a
 *
 * @field:	an initialized field to print
 */
static void print_bin(const struct field *field)
{
	__print_bin(field, "", false);
}

/**
 * print_bin_raw() - print raw data both in hexadecimal and in ascii format
 *
 * @field:	an initialized field to print
 */
static void print_bin_raw(const struct field *field)
{
	ASSERT(field && field->data);

	printf("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f"
	       "     0123456789abcdef\n");
	int i, j;

	for (i = 0; i < 256; i += 16) {
		printf("%02x: ", i);
		for (j = 0; j < 16; j++) {
			printf("%02x", field->data[i+j]);
			printf(" ");
		}
		printf("    ");

		for (j = 0; j < 16; j++) {
			if (field->data[i+j] == 0x00 || field->data[i+j] == 0xff)
				printf(".");
			else if (field->data[i+j] < 32 || field->data[i+j] >= 127)
				printf("?");
			else
				printf("%c", field->data[i+j]);
		}
		printf("\n");
	}
}

/**
 * update_bin() - Update field with new data in binary form
 *
 * @field:	an initialized field
 * @value:	a string of values (i.e. "10b234a")
 */
static int update_bin(struct field *field, char *value)
{
	return __update_bin(field, value, false);
}

/**
 * print_bin_rev() - print the value of a field from type "reversed"
 *
 * Treat the field data as simple binary data, and print it in reverse order
 * as two digit hexadecimal values.
 *
 * Data in field: 0102030405060708090a
 * Sample output: 0a090807060504030201
 *
 * @field:	an initialized field to print
 */
static void print_bin_rev(const struct field *field)
{
	__print_bin(field, "", true);
}

/**
 * update_bin_rev() - Update field with new data in binary form, storing it in
 * 		      reverse
 *
 * This function takes a string of byte values, and stores them
 * in the field in the reverse order. i.e. if the input string was "1234",
 * "3412" will be written to the field.
 *
 * @field:	an initialized field
 * @value:	a string of byte values
 */
static int update_bin_rev(struct field *field, char *value)
{
	return __update_bin(field, value, true);
}

/**
 * print_bin_ver() - print the value of a field from type "version"
 *
 * Treat the field data as simple binary data, and print it formatted as a
 * version number (2 digits after decimal point).
 * The field size must be exactly 2 bytes.
 *
 * Sample output: 123.45
 *
 * @field:	an initialized field to print
 */
static void print_bin_ver(const struct field *field)
{
	ASSERT(field && field->data);

	if ((field->data[0] == 0xff) && (field->data[1] == 0xff)) {
		field->data[0] = 0;
		field->data[1] = 0;
	}

	printf("%#.2f\n", (field->data[1] << 8 | field->data[0]) / 100.0);
}

/**
 * update_bin_ver() - update a "version field" which contains binary data
 *
 * This function takes a version string in the form of x.y (x and y are both
 * decimal values, y is limited to two digits), translates it to the binary
 * form, then writes it to the field. The field size must be exactly 2 bytes.
 *
 * This function strictly enforces the data syntax, and will not update the
 * field if there's any deviation from it. It also protects from overflow.
 *
 * @field:	an initialized field
 * @value:	a version string
 *
 * Returns 0 on success, -1 on failure.
 */
static int update_bin_ver(struct field *field, char *value)
{
	ASSERT(field && field->data && field->name && value);

	char *version = value;
	int num, remainder;

	if (strtoi(&version, &num) != STRTOI_STR_CON && *version != '.') {
		iveprintf("Syntax error", value, field->name);
		return -1;
	}

	version++;
	if (strtoi(&version, &remainder) != STRTOI_STR_END) {
		iveprintf("Syntax error", value, field->name);
		return -1;
	}

	if (num < 0 || remainder < 0) {
		iveprintf("Version must be positive", value, field->name);
		return -1;
	}

	if (remainder > 99) {
		iveprintf("Minor version is 1-2 digits", value, field->name);
		return -1;
	}

	num = num * 100 + remainder;
	if (num >> 16) {
		iveprintf("Version is too big", value, field->name);
		return -1;
	}

	field->data[0] = (unsigned char)num;
	field->data[1] = num >> 8;

	return 0;
}

/**
 * print_mac_addr() - print the value of a field from type "mac"
 *
 * Treat the field data as simple binary data, and print it formatted as a MAC
 * address.
 * Sample output: 01:02:03:04:05:06
 *
 * @field:	an initialized field to print
 */
static void print_mac(const struct field *field)
{
	__print_bin(field, ":", false);
}

/**
 * update_mac() - Update a mac address field which contains binary data
 *
 * @field:	an initialized field
 * @value:	a colon delimited string of byte values (i.e. "1:02:3:ff")
 */
static int update_mac(struct field *field, char *value)
{
	return __update_bin_delim(field, value, ':');
}

static char *months[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
		    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

/**
 * print_date() - print the value of a field from type "date"
 *
 * Treat the field data as simple binary data, and print it formatted as a date.
 * Sample output: 07/Feb/2014
 * 		  56/BAD/9999
 *
 * @field:	an initialized field to print
 */
static void print_date(const struct field *field)
{
	ASSERT(field && field->data);

	printf("%02d/", field->data[0]);
	if (field->data[1] >= 1 && field->data[1] <= 12)
		printf("%s", months[field->data[1] - 1]);
	else
		printf("BAD");

	printf("/%d\n", field->data[3] << 8 | field->data[2]);
}

static int validate_date(unsigned char day, unsigned char month,
			unsigned int year)
{
	int days_in_february;

	switch (month) {
	case 0:
	case 2:
	case 4:
	case 6:
	case 7:
	case 9:
	case 11:
		if (day > 31)
			return -1;
		break;
	case 3:
	case 5:
	case 8:
	case 10:
		if (day > 30)
			return -1;
		break;
	case 1:
		days_in_february = 28;
		if (year % 4 == 0) {
			if (year % 100 != 0) {
				days_in_february = 29;
			} else if (year % 400 == 0) {
				days_in_february = 29;
			}
		}

		if (day > days_in_february)
			return -1;

		break;
	default:
		return -1;
	}

	return 0;
}

/**
 * update_date() - update a date field which contains binary data
 *
 * This function takes a date string in the form of x/Mon/y (x and y are both
 * decimal values), translates it to the binary representation, then writes it
 * to the field.
 *
 * This function strictly enforces the data syntax, and will not update the
 * field if there's any deviation from it. It also protects from overflow in the
 * year value, and checks the validity of the date.
 *
 * @field:	an initialized field
 * @value:	a date string
 *
 * Returns 0 on success, -1 on failure.
 */
static int update_date(struct field *field, char *value)
{
	ASSERT(field && field->data && field->name && value);

	char *date = value;
	int day, month, year;

	if (strtoi(&date, &day) != STRTOI_STR_CON || *date != '/') {
		iveprintf("Syntax error", value, field->name);
		return -1;
	}

	if (day == 0) {
		iveprintf("Invalid day", value, field->name);
		return -1;
	}

	date++;
	if (strlen(date) < 4 || *(date + 3) != '/') {
		iveprintf("Syntax error", value, field->name);
		return -1;
	}

	for (month = 1; month <= 12; month++)
		if (!strncmp(date, months[month - 1], 3))
			break;

	if (strncmp(date, months[month - 1], 3)) {
		iveprintf("Invalid month", value, field->name);
		return -1;
	}

	date += 4;
	if (strtoi(&date, &year) != STRTOI_STR_END) {
		iveprintf("Syntax error", value, field->name);
		return -1;
	}

	if (validate_date(day, month - 1, year)) {
		iveprintf("Invalid date", value, field->name);
		return -1;
	}

	if (year >> 16) {
		iveprintf("Year overflow", value, field->name);
		return -1;
	}

	field->data[0] = (unsigned char)day;
	field->data[1] = (unsigned char)month;
	field->data[2] = (unsigned char)year;
	field->data[3] = (unsigned char)(year >> 8);

	return 0;
}

/**
 * print_ascii() - print the value of a field from type "ascii"
 * @field:	an initialized field to print
 */
static void print_ascii(const struct field *field)
{
	ASSERT(field && field->data);

	char format[8];
	int *str = (int*)field->data;
	int pattern = *str;
	/* assuming field->data_size is a multiple of 32bit! */
	int block_count = field->data_size / sizeof(int);
	char *print_buf = "";

	/* check if str is trivial (contains only 0's or only 0xff's), if so print nothing */
	for (int i = 0; i < block_count - 1; i++) {
		str++;
		if (*str != pattern || (pattern != 0 && pattern != -1)) {
			print_buf = (char*)field->data;
			break;
		}
	}

	sprintf(format, "%%.%ds\n", field->data_size);
	printf(format, print_buf);
}

/**
 * update_ascii() - Update field with new data in ASCII form
 * @field:	an initialized field
 * @value:	the new string data
 *
 * Returns 0 on success, -1 of failure (new string too long).
 */
static int update_ascii(struct field *field, char *value)
{
	ASSERT(field && field->data && field->name && value);

	if (strlen(value) >= field->data_size) {
		iveprintf("Value is too long", value, field->name);
		return -1;
	}

	strncpy((char *)field->data, value, field->data_size - 1);
	field->data[field->data_size - 1] = '\0';

	return 0;
}

/**
 * print_reserved() - print the size of a field from type "reserved"
 *
 * Print a notice that the following field_size bytes are reserved.
 *
 * Sample output: (64 bytes)
 *
 * @field:	an initialized field to print
 */
static void print_reserved(const struct field *field)
{
	ASSERT(field);
	printf("(%d bytes)\n", field->data_size);
}

/**
 * clear_field() - clear a field
 *
 * A cleared field is defined by having all bytes set to 0xff.
 *
 * @field:	an initialized field to clear
 */
static void clear_field(struct field *field)
{
	ASSERT(field && field->data);
	memset(field->data, 0xff, field->data_size);
}

/**
 * get_data_size() - get the size of field's data
 *
 * @field:	an initialized field
 *
 * return: the size of field's data
 */
static int get_data_size(const struct field *field)
{
	ASSERT(field);
	return field->data_size;
}

/**
 * is_named() - check if any of the field's names match the given string
 *
 * @field:	an initialized field to check
 * @str:	the string to check
 *
 * Returns:	true if field's names matches, false otherwise.
 */
static bool is_named(const struct field *field, const char *str)
{
	ASSERT(field && field->name && field->short_name && str);

	if (field->type != FIELD_RESERVED && field->type != FIELD_RAW &&
	    (!strcmp(field->name, str) || !strcmp(field->short_name, str)))
		return true;

	return false;
}

/**
 * print_field() - print the given field
 *
 * @field:	an initialized field to to print
 */
static void print_field(const struct field *field)
{
	ASSERT(field && field->name && field->ops);

	printf("%-30s", field->name);
	field->ops->print_value(field);
}

#define OPS_UPDATABLE(type) { \
	.get_data_size	= get_data_size, \
	.is_named	= is_named, \
	.print_value	= print_##type, \
	.print		= print_field, \
	.update		= update_##type, \
	.clear		= clear_field, \
}

#define OPS_PRINTABLE(type) { \
	.get_data_size	= get_data_size, \
	.is_named	= is_named, \
	.print_value	= print_##type, \
	.print		= print_field, \
	.update		= NULL, \
	.clear		= NULL, \
}

static struct field_ops field_ops[] = {
	[FIELD_BINARY]		= OPS_UPDATABLE(bin),
	[FIELD_REVERSED]	= OPS_UPDATABLE(bin_rev),
	[FIELD_VERSION]		= OPS_UPDATABLE(bin_ver),
	[FIELD_ASCII]		= OPS_UPDATABLE(ascii),
	[FIELD_MAC]		= OPS_UPDATABLE(mac),
	[FIELD_DATE]		= OPS_UPDATABLE(date),
	[FIELD_RESERVED]	= OPS_PRINTABLE(reserved),
	[FIELD_RAW]		= OPS_PRINTABLE(bin_raw)
};

/**
 * init_field() - init field according to field.type
 *
 * @field:	an initialized field with a known field.type to init
 * @data:	the binary data of the field
 */
void init_field(struct field *field, unsigned char *data)
{
	ASSERT(field && data);

	field->ops = &field_ops[field->type];
	field->data = data;
}
