/*
 * Copyright (C) 2009-2017 CompuLab, Ltd.
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
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include "layout.h"
#include "common.h"
#include "field.h"

#define LAYOUT_CHECK_BYTE	44
#define RESERVED_FIELDS		NULL
#define NO_LAYOUT_FIELDS	"Unknown layout. Dumping raw data\n"

struct field layout_legacy[5] = {
	{ "MAC address",          6, NULL, print_mac,      update_mac },
	{ "Board Revision",       2, NULL, print_bin,      update_bin },
	{ "Serial Number",        8, NULL, print_bin,      update_bin },
	{ "Board Configuration", 64, NULL, print_ascii,    update_ascii },
	{ RESERVED_FIELDS,      176, NULL, print_reserved, update_ascii },
};

struct field layout_v1[12] = {
	{ "Major Revision",      2, NULL, print_bin_ver,  update_bin_ver },
	{ "Minor Revision",      2, NULL, print_bin_ver,  update_bin_ver },
	{ "1st MAC Address",     6, NULL, print_mac,      update_mac },
	{ "2nd MAC Address",     6, NULL, print_mac,      update_mac },
	{ "Production Date",     4, NULL, print_date,     update_date },
	{ "Serial Number",      12, NULL, print_bin_rev,  update_bin_rev },
	{ RESERVED_FIELDS,      96, NULL, print_reserved, update_reserved },
	{ "Product Name",       16, NULL, print_ascii,    update_ascii },
	{ "Product Options #1", 16, NULL, print_ascii,    update_ascii },
	{ "Product Options #2", 16, NULL, print_ascii,    update_ascii },
	{ "Product Options #3", 16, NULL, print_ascii,    update_ascii },
	{ RESERVED_FIELDS,      64, NULL, print_reserved, update_ascii },
};

struct field layout_v2[15] = {
	{ "Major Revision",           2, NULL, print_bin_ver,  update_bin_ver },
	{ "Minor Revision",           2, NULL, print_bin_ver,  update_bin_ver },
	{ "1st MAC Address",          6, NULL, print_mac,      update_mac },
	{ "2nd MAC Address",          6, NULL, print_mac,      update_mac },
	{ "Production Date",          4, NULL, print_date,     update_date },
	{ "Serial Number",           12, NULL, print_bin_rev,  update_bin_rev },
	{ "3rd MAC Address (WIFI)",   6, NULL, print_mac,      update_mac },
	{ "4th MAC Address (Bluetooth)", 6, NULL, print_mac,   update_mac },
	{ "Layout Version",           1, NULL, print_bin,      update_bin },
	{ RESERVED_FIELDS,          83, NULL, print_reserved, update_reserved },
	{ "Product Name",            16, NULL, print_ascii,    update_ascii },
	{ "Product Options #1",      16, NULL, print_ascii,    update_ascii },
	{ "Product Options #2",      16, NULL, print_ascii,    update_ascii },
	{ "Product Options #3",      16, NULL, print_ascii,    update_ascii },
	{ RESERVED_FIELDS,           64, NULL, print_reserved, update_ascii },
};

struct field layout_v3[16] = {
	{ "Major Revision",           2, NULL, print_bin_ver,  update_bin_ver },
	{ "Minor Revision",           2, NULL, print_bin_ver,  update_bin_ver },
	{ "1st MAC Address",          6, NULL, print_mac,      update_mac },
	{ "2nd MAC Address",          6, NULL, print_mac,      update_mac },
	{ "Production Date",          4, NULL, print_date,     update_date },
	{ "Serial Number",           12, NULL, print_bin_rev,  update_bin_rev },
	{ "3rd MAC Address (WIFI)",   6, NULL, print_mac,      update_mac },
	{ "4th MAC Address (Bluetooth)", 6, NULL, print_mac,   update_mac },
	{ "Layout Version",           1, NULL, print_bin,      update_bin },
	{ "CompuLab EEPROM ID",       3, NULL, print_bin,      update_bin },
	{ RESERVED_FIELDS,          80, NULL, print_reserved, update_reserved },
	{ "Product Name",            16, NULL, print_ascii,    update_ascii },
	{ "Product Options #1",      16, NULL, print_ascii,    update_ascii },
	{ "Product Options #2",      16, NULL, print_ascii,    update_ascii },
	{ "Product Options #3",      16, NULL, print_ascii,    update_ascii },
	{ RESERVED_FIELDS,           64, NULL, print_reserved, update_ascii },
};

struct field layout_v4[21] = {
	{ "Major Revision",           2, NULL, print_bin_ver,  update_bin_ver },
	{ "Minor Revision",           2, NULL, print_bin_ver,  update_bin_ver },
	{ "1st MAC Address",          6, NULL, print_mac,      update_mac },
	{ "2nd MAC Address",          6, NULL, print_mac,      update_mac },
	{ "Production Date",          4, NULL, print_date,     update_date },
	{ "Serial Number",           12, NULL, print_bin_rev,  update_bin_rev },
	{ "3rd MAC Address (WIFI)",   6, NULL, print_mac,      update_mac },
	{ "4th MAC Address (Bluetooth)", 6, NULL, print_mac,   update_mac },
	{ "Layout Version",           1, NULL, print_bin,      update_bin },
	{ "CompuLab EEPROM ID",       3, NULL, print_bin,      update_bin },
	{ "5th MAC Address",          6, NULL, print_mac,      update_mac },
	{ "6th MAC Address",          6, NULL, print_mac,      update_mac },
	{ RESERVED_FIELDS,          4, NULL, print_reserved, update_reserved },
	{ RESERVED_FIELDS,          64, NULL, print_reserved, update_reserved },
	{ "Product Name",            16, NULL, print_ascii,    update_ascii },
	{ "Product Options #1",      16, NULL, print_ascii,    update_ascii },
	{ "Product Options #2",      16, NULL, print_ascii,    update_ascii },
	{ "Product Options #3",      16, NULL, print_ascii,    update_ascii },
	{ "Product Options #4",      16, NULL, print_ascii,    update_ascii },
	{ "Product Options #5",      16, NULL, print_ascii,    update_ascii },
	{ RESERVED_FIELDS,           32, NULL, print_reserved, update_ascii },
};

struct field layout_unknown[1] = {
	{ NO_LAYOUT_FIELDS, 256, NULL, print_bin_raw, update_bin },
};

/*
 * detect_layout() - detect layout based on the contents of the data.
 * @data: Pointer to the data to be analyzed.
 *
 * Returns: the detected layout version.
 */
static enum layout_version detect_layout(unsigned char *data)
{
	switch (data[LAYOUT_CHECK_BYTE]) {
	case 0xff:
	case 0:
		return LAYOUT_VER1;
	case 2:
		return LAYOUT_VER2;
	case 3:
		return LAYOUT_VER3;
	case 4:
		return LAYOUT_VER4;
	}

	if (data[LAYOUT_CHECK_BYTE] >= 0x20)
		return LAYOUT_LEGACY;

	return LAYOUT_UNRECOGNIZED;
}

/*
 * print_layout() - print the layout and the data which is assigned to it.
 * @layout: A pointer to an existing struct layout.
 */
static void print_layout(const struct layout *layout)
{
	struct field *fields = layout->fields;

	for (int i = 0; i < layout->num_of_fields; i++)
		fields[i].print(&fields[i]);
}

/*
 * Selectively update EEPROM data by bytes.
 * @layout:		An initialized layout
 * @new_byte_data:	An array of (offset,value) strings matching the regexp
 * 			([[:digit:]]+,[[:digit:]]+,)*([[:digit:]]+,[[:digit:]]+)
 * @new_data_size: Size of the new_field_data array
 *
 * Returns: number of updated bytes.
 */
static int update_bytes(struct layout *layout,
			struct strings_pair *new_byte_data,
			int new_data_size)
{
	int updated_bytes = 0;
	for (int i = 0; i < new_data_size; i++) {
		char *str = strtok(new_byte_data[i].key, "-");
		if (str == NULL)
			return 0;

		int offset = safe_strtoui(str, 0);
		if (!(offset >= 0 && offset < EEPROM_SIZE)) {
			fprintf(stderr, "Invalid offset '%s'; will not update!\n",
				new_byte_data[i].key);
			return 0;
		}

		int value = safe_strtoui(new_byte_data[i].value, 0);
		if (!(value >= 0 && value <= 255)) {
			fprintf(stderr, "Invalid value '%s' at offset '%s'; "
				"will not update!\n", new_byte_data[i].value,
				new_byte_data[i].key);
			return 0;
		}

		/* in case of a range of bytes to write */
		str = strtok(NULL, "-");
		if (str != NULL) {
			int offset_end = safe_strtoui(str, 0);
			if (!(offset_end >= 0 && offset_end < EEPROM_SIZE)) {
				fprintf(stderr, "Invalid offset '%s'; "
					"will not update!\n", str);
				return 0;
			}

			size_t range = offset_end - offset + 1;
			memset(layout->data + offset, value, range);
			updated_bytes += range;
		} else {
			layout->data[offset] = value;
			updated_bytes++;
		}
	}

	return updated_bytes;
}

/*
 * update_field() - update a single field in the layout data.
 * @layout:	A pointer to an existing struct layout.
 * @field_name:	The name of the field to update
 * @new_data:	The new field data (a string. Format depends on the field)
 *
 * Returns: 0 on success, -1 on failure.
 */
static int update_field(struct layout *layout, char *field_name, char *new_data)
{
	struct field *fields = layout->fields;

	if (new_data == NULL)
		return 0;

	if (field_name == NULL)
		return -1;

	for (int i = 0; i < layout->num_of_fields; i++) {
		if (fields[i].name == RESERVED_FIELDS ||
		    strcmp(fields[i].name, field_name))
			continue;

		int err = fields[i].update(&fields[i], new_data);
		if (err)
			fprintf(stderr, "Invalid data for field %s\n", field_name);

		return err;
	}

	fprintf(stderr, "No such field '%s'\n", field_name);

	return -1;
}

/*
 * Selectively update EEPROM data by fields.
 * @layout:			An initialized layout
 * @new_field_data:		An array of string pairs (fieldname,data)
 * @new_field_array_size:	Size of the new_field_data array
 *
 * Returns: number of updated fields.
 */
static int update_fields(struct layout *layout,
			 struct strings_pair *new_field_data,
			 int new_field_array_size)
{
	int updated_fields_cnt = 0;

	for (int i = 0; i < new_field_array_size; i++) {
		if (update_field(layout, new_field_data[i].key,
				 new_field_data[i].value)) {

			return 0;
		}

		updated_fields_cnt++;
	}

	return updated_fields_cnt;
}

#define ARRAY_LEN(x)	(sizeof(x) / sizeof(x[0]))

/*
 * new_layout() - Allocate a new layout based on the data given in buf.
 * @buf:	Data seed for layout
 * @buf_size:	Size of buf
 *
 * Allocates a new layout based on data in buf. The layout version is
 * automatically detected. The resulting layout struct contains a copy of the
 * provided data.
 *
 * Returns: pointer to a new layout on success, NULL on failure
 */
struct layout *new_layout(unsigned char *buf, unsigned int buf_size,
			  enum layout_version layout_version)
{
	struct layout *layout = malloc(sizeof(struct layout));
	if (layout == NULL)
		return NULL;

	if (layout_version == LAYOUT_AUTODETECT)
		layout->layout_version = detect_layout(buf);
	else
		layout->layout_version = layout_version;

	switch (layout->layout_version) {
	case LAYOUT_LEGACY:
		layout->fields = layout_legacy;
		layout->num_of_fields = ARRAY_LEN(layout_legacy);
		break;
	case LAYOUT_VER1:
		layout->fields = layout_v1;
		layout->num_of_fields = ARRAY_LEN(layout_v1);
		break;
	case LAYOUT_VER2:
		layout->fields = layout_v2;
		layout->num_of_fields = ARRAY_LEN(layout_v2);
		break;
	case LAYOUT_VER3:
		layout->fields = layout_v3;
		layout->num_of_fields = ARRAY_LEN(layout_v3);
		break;
	case LAYOUT_VER4:
		layout->fields = layout_v4;
		layout->num_of_fields = ARRAY_LEN(layout_v4);
		break;
	default:
		layout->fields = layout_unknown;
		layout->num_of_fields = ARRAY_LEN(layout_unknown);
	}

	layout->data = buf;
	for (int i = 0; i < layout->num_of_fields; i++) {
		layout->fields[i].buf = buf;
		buf += layout->fields[i].size;
	}

	layout->data_size = buf_size;
	layout->print = print_layout;
	layout->update_fields = update_fields;
	layout->update_bytes = update_bytes;

	return layout;
}

/*
 * free_layout() - a destructor for layout
 * @layout:	the layout to deallocate
 */
void free_layout(struct layout *layout)
{
	free(layout);
}
