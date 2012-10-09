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
#include <malloc.h>
#include "layout.h"
#include "field.h"

#define LAYOUT_CHECK_BYTE	44
#define RESERVED_FIELDS		0
#define NO_LAYOUT_FIELDS	"Could not detect layout. Dumping raw data\n"

static int set_layout_legacy(struct layout *layout)
{
	int field_num = 5;
	layout->num_of_fields = 0;
	layout->fields = (struct field *) malloc(sizeof(struct field) *
								field_num);

	if (layout->fields == NULL)
		return 0;

	layout->fields[0] = set_field("MAC address", 6, ":",
					print_bin, update_binary);
	layout->fields[1] = set_field("Board Revision", 2, "",
					print_bin, update_binary);
	layout->fields[2] = set_field("Serial Number", 8, "",
					print_bin, update_binary);
	layout->fields[3] = set_field("Board Configuration", 64, "",
					print_ascii, update_ascii);
	layout->fields[4] = set_field(RESERVED_FIELDS, 176, "",
					print_reserved, update_ascii);
	layout->num_of_fields = field_num;

	return 1;
}

static int set_layout_v1(struct layout *layout)
{
	int field_num = 12;
	layout->num_of_fields = 0;
	layout->fields = (struct field *) malloc(sizeof(struct field) *
								field_num);

	if (layout->fields == NULL)
		return 0;

	layout->fields[0] = set_field("Major Revision", 2, ".",
					print_bin_ver, update_binary);
	layout->fields[1] = set_field("Minor Revision", 2, ".",
					print_bin_ver, update_binary);
	layout->fields[2] = set_field("1st MAC addr", 6, ":",
					print_bin, update_binary);
	layout->fields[3] = set_field("2nd MAC addr", 6, ":",
					print_bin, update_binary);
	layout->fields[4] = set_field("Production Date", 4, "/",
					print_date, update_binary);
	layout->fields[5] = set_field("Serial Number", 12, " ",
					print_bin_rev, update_binary);
	layout->fields[6] = set_field(RESERVED_FIELDS, 96, "",
					print_reserved, update_binary);
	layout->fields[7] = set_field("Product Name", 16, "",
					print_ascii, update_ascii);
	layout->fields[8] = set_field("Product Options #1", 16, "",
					print_ascii, update_ascii);
	layout->fields[9] = set_field("Product Options #2", 16, "",
					print_ascii, update_ascii);
	layout->fields[10] = set_field("Product Options #3", 16, "",
					print_ascii, update_ascii);
	layout->fields[11] = set_field(RESERVED_FIELDS, 64, "",
					print_reserved,	update_ascii);
	layout->num_of_fields = field_num;

	return 1;
}

static int set_layout_v2(struct layout *layout)
{
	int field_num = 15;
	layout->num_of_fields = 0;
	layout->fields = (struct field *) malloc(sizeof(struct field) *
								field_num);

	if (layout->fields == NULL)
		return 0;

	layout->fields[0] = set_field("Major Revision", 2, ".",
					print_bin_ver, update_binary);
	layout->fields[1] = set_field("Minor Revision", 2, ".",
					print_bin_ver, update_binary);
	layout->fields[2] = set_field("1st MAC addr", 6, ":",
					print_bin, update_binary);
	layout->fields[3] = set_field("2nd MAC addr", 6, ":",
					print_bin, update_binary);
	layout->fields[4] = set_field("Production Date", 4, "/",
					print_date, update_binary);
	layout->fields[5] = set_field("Serial Number", 12, " ",
					print_bin_rev, update_binary);
	layout->fields[6] = set_field("3rd MAC Address (WIFI)", 6, ":",
					print_bin, update_binary);
	layout->fields[7] = set_field("4th MAC Address (Bluetooth)", 6, ":",
					print_bin, update_binary);
	layout->fields[8] = set_field("Layout Version", 1, " ",
					print_bin, update_binary);
	layout->fields[9] = set_field(RESERVED_FIELDS, 83, "",
					print_reserved, update_binary);
	layout->fields[10] = set_field("Product Name", 16, "",
					print_ascii, update_ascii);
	layout->fields[11] = set_field("Product Options #1", 16, "",
					print_ascii, update_ascii);
	layout->fields[12] = set_field("Product Options #2", 16, "",
					print_ascii, update_ascii);
	layout->fields[13] = set_field("Product Options #3", 16, "",
					print_ascii, update_ascii);
	layout->fields[14] = set_field(RESERVED_FIELDS, 64, "",
					print_reserved,	update_ascii);
	layout->num_of_fields = field_num;

	return 1;
}

static int set_layout_unrecognized(struct layout *layout)
{
	int field_num = 1;
	layout->num_of_fields = 0;
	layout->fields = (struct field *) malloc(sizeof(struct field) *
								field_num);

	if (layout->fields == NULL)
		return 0;

	layout->fields[0] = set_field(NO_LAYOUT_FIELDS, 256, " ",
					print_bin, update_binary);
	layout->num_of_fields = field_num;

	return 1;
}

static enum layout_version detect_layout(unsigned char *data)
{
	int check_byte = LAYOUT_CHECK_BYTE;

	if (data[check_byte] == 0xff || data[check_byte] == 0)
		return LAYOUT_VER1;

	if (data[check_byte] >= 0x20)
		return LAYOUT_LEGACY;

	return LAYOUT_VER2;
}

static void print_layout(const struct layout *layout)
{
	int i;
	struct field *fields = layout->fields;

	for (i = 0; i < layout->num_of_fields; i++)
		fields[i].print(&fields[i]);
}

static enum layout_res update_field(struct layout *layout, char *field_name,
				    char *new_data)
{
	int i;
	struct field *fields = layout->fields;

	if (layout == NULL || field_name == NULL || new_data == NULL)
		return -LAYOUT_NULL_ARGUMENTS;

	/* Advance until the field name is found. */
	for (i = 0; i < layout->num_of_fields; i++) {
		if (fields[i].name != RESERVED_FIELDS &&
		    !strcmp(fields[i].name, field_name))
			break;
	}

	if (i >= layout->num_of_fields)
		return -LAYOUT_NO_SUCH_FIELD;

	fields[i].update(&fields[i], new_data);

	return LAYOUT_SUCCESS;
}

static enum layout_res update_byte(struct layout *layout, unsigned int offset,
				   char new_byte)
{
	if (layout == NULL)
		return -LAYOUT_NULL_ARGUMENTS;

	if (offset >= layout->data_size)
		return -LAYOUT_OFFSET_OUT_OF_BOUNDS;

	layout->data[offset] = new_byte;

	return LAYOUT_SUCCESS;
}

/*
 * Allocates a new layout based on the data given in buf. The layout version
 * is automatically detected. The resulting layout struct contains a copy of
 * the provided data.
 */
struct layout *new_layout(unsigned char *buf, unsigned int buf_size)
{
	int i, success;
	struct layout *l;
	unsigned char *temp;

	l = (struct layout *) malloc(sizeof(struct layout));
	if (l == NULL)
		return NULL;

	l->layout_version = detect_layout(buf);
	switch (l->layout_version) {
	case LAYOUT_LEGACY:
		success = set_layout_legacy(l);
		break;
	case LAYOUT_VER1:
		success = set_layout_v1(l);
		break;
	case LAYOUT_VER2:
		success = set_layout_v2(l);
		break;
	default:
		success = set_layout_unrecognized(l);
	}

	if (!success)
		goto free_layout;

	l->data = (unsigned char *) malloc(sizeof(unsigned char) * buf_size);
	if (l->data == NULL)
		goto free_fields;

	for (i = 0; i < buf_size; i++)
		l->data[i] = buf[i];

	temp = l->data;
	for (i = 0; i < l->num_of_fields; i++) {
		l->fields[i].buf = temp;
		temp += l->fields[i].size;
	}

	l->data_size = buf_size;
	l->print = print_layout;
	l->update_field = update_field;
	l->update_byte = update_byte;

	return l;

free_fields:
	free(l->fields);
free_layout:
	free(l);

	return NULL;
}

void free_layout(struct layout *layout)
{
	free(layout->fields);
	free(layout->data);
	free(layout);
}
