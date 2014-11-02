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
 * update_bin_rev() - Update field with new data in binary form, storing it in
 * 		      reverse
 *
 * This function takes a space delimited string of byte values, and stores them
 * in the field in the reverse order. i.e. if the input string was "1 2 3 4",
 * "4 3 2 1" will be written to the field.
 *
 * @field:	an initialized field
 * @value:	a space delimited string of byte values
 */
int update_bin_rev(struct field *field, char *value)
{
	return __update_bin(field, value, " ", 1);
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
int update_bin_ver(struct field *field, char *value)
{
	char *endptr;
	char *tok = strtok(value, ".");
	if (tok == NULL)
		return -1;

	int num = strtol(tok, &endptr, 0);
	if (*endptr != '\0')
		return -1;

	tok = strtok(NULL, "");
	if (tok == NULL)
		return -1;

	int remainder = strtol(tok, &endptr, 0);
	if (*endptr != '\0')
		return -1;

	num = num * 100 + remainder;
	if (num >> 16)
		return -1;

	field->buf[0] = (unsigned char)num;
	field->buf[1] = num >> 8;

	return 0;
}

/**
 * print_mac_addr() - print a field which contains a mac address
 *
 * Treat the field data as simple binary data, and print it formatted as a MAC
 * address.
 * Sample output:
 * 	Field Name	01:02:03:04:05:06
 *
 * @field:	an initialized field to print
 */
void print_mac(const struct field *field)
{
	__print_bin(field, ":", 0);
}

/**
 * update_mac() - Update a mac address field which contains binary data
 *
 * @field:	an initialized field
 * @value:	a colon delimited string of byte values (i.e. "1:02:3:ff")
 */
int update_mac(struct field *field, char *value)
{
	return __update_bin(field, value, ":", 0);
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
int update_date(struct field *field, char *value)
{
	char *endptr;
	char *tok1 = strtok(value, "/");
	char *tok2 = strtok(NULL, "/");
	char *tok3 = strtok(NULL, "/");

	if (tok1 == NULL || tok2 == NULL || tok3 == NULL) {
		printf("%s: syntax error\n", field->name);
		return -1;
	}

	unsigned char day = (unsigned char)strtol(tok1, &endptr, 0);
	if (*endptr != '\0' || day == 0) {
		printf("%s: invalid day\n", field->name);
		return -1;
	}

	unsigned char month;
	for (month = 1; month <= 12; month++)
		if (!strcmp(tok2, months[month - 1]))
			break;

	unsigned int year = strtol(tok3, &endptr, 0);
	if (*endptr != '\0') {
		printf("%s: invalid year\n", field->name);
		return -1;
	}

	if (validate_date(day, month - 1, year)) {
		printf("%s: invalid date\n", field->name);
		return -1;
	}

	if (year >> 16) {
		printf("%s: year overflow\n", field->name);
		return -1;
	}

	field->buf[0] = day;
	field->buf[1] = month;
	field->buf[2] = (unsigned char)year;
	field->buf[3] = (unsigned char)(year >> 8);

	return 0;
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
