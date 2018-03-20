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
	{ "MAC address",		"mac",	6,	NULL, print_mac,	update_mac },
	{ "Board Revision",		"rev",	2,	NULL, print_bin,	update_bin },
	{ "Serial Number",		"sn",	8,	NULL, print_bin,	update_bin },
	{ "Board Configuration",	"conf",	64,	 NULL, print_ascii,	update_ascii },
	{ RESERVED_FIELDS,		NULL,	176,	NULL, print_reserved,	update_ascii },
};

struct field layout_v1[12] = {
	{ "Major Revision",	"major",	2,	NULL,	print_bin_ver,	update_bin_ver },
	{ "Minor Revision",	"minor",	2,	NULL,	print_bin_ver,	update_bin_ver },
	{ "1st MAC Address",	"mac1",		6,	NULL,	print_mac,	update_mac },
	{ "2nd MAC Address",	"mac2",		6,	NULL,	print_mac,	update_mac },
	{ "Production Date",	"date",		4,	NULL,	print_date,	update_date },
	{ "Serial Number",	"sn",		12,	NULL,	print_bin_rev,	update_bin_rev },
	{ RESERVED_FIELDS,	NULL,		96,	NULL,	print_reserved,	update_reserved },
	{ "Product Name",	"name",		16,	NULL,	print_ascii,	update_ascii },
	{ "Product Options #1",	"opt1",		16,	NULL,	print_ascii,	update_ascii },
	{ "Product Options #2",	"opt2",		16,	NULL,	print_ascii,	update_ascii },
	{ "Product Options #3",	"opt3",		16,	NULL,	print_ascii,	update_ascii },
	{ RESERVED_FIELDS,	NULL,		64,	NULL,	print_reserved,	update_ascii },
};

struct field layout_v2[15] = {
	{ "Major Revision",			"major",	2,	NULL,	print_bin_ver,	update_bin_ver },
	{ "Minor Revision",			"minor",	2,	NULL,	print_bin_ver,	update_bin_ver },
	{ "1st MAC Address",			"mac1",		6,	NULL,	print_mac,	update_mac },
	{ "2nd MAC Address",			"mac2",		6,	NULL,	print_mac,	update_mac },
	{ "Production Date",			"date",		4,	NULL,	print_date,	update_date },
	{ "Serial Number",			"sn",		12,	NULL,	print_bin_rev,	update_bin_rev },
	{ "3rd MAC Address (WIFI)",		"mac3",		6,	NULL,	print_mac,	update_mac },
	{ "4th MAC Address (Bluetooth)",	"mac4",		6,	NULL,	print_mac,	update_mac },
	{ "Layout Version",			"layout",	1,	NULL,	print_bin,	update_bin },
	{ RESERVED_FIELDS,			NULL,		83,	NULL,	print_reserved,	update_reserved },
	{ "Product Name",			"name", 	16,	NULL,	print_ascii,	update_ascii },
	{ "Product Options #1",			"opt1", 	16,	NULL,	print_ascii,	update_ascii },
	{ "Product Options #2",			"opt2", 	16,	NULL,	print_ascii,	update_ascii },
	{ "Product Options #3",			"opt3", 	16,	NULL,	print_ascii,	update_ascii },
	{ RESERVED_FIELDS,			NULL,		64,	NULL,	print_reserved,	update_ascii },
};

struct field layout_v3[16] = {
	{ "Major Revision",			"major",	2,	NULL,	print_bin_ver,	update_bin_ver },
	{ "Minor Revision",			"minor",	2,	NULL,	print_bin_ver,	update_bin_ver },
	{ "1st MAC Address",			"mac1",		6,	NULL,	print_mac,	update_mac },
	{ "2nd MAC Address",			"mac2",		6,	NULL,	print_mac,	update_mac },
	{ "Production Date",			"date",		4,	NULL,	print_date,	update_date },
	{ "Serial Number",			"sn",		12,	NULL,	print_bin_rev,	update_bin_rev },
	{ "3rd MAC Address (WIFI)",		"mac3",		6,	NULL,	print_mac,	update_mac },
	{ "4th MAC Address (Bluetooth)",	"mac4",		6,	NULL,	print_mac,	update_mac },
	{ "Layout Version",			"layout",	1,	NULL,	print_bin,	update_bin },
	{ "CompuLab EEPROM ID",			"id",		3,	NULL,	print_bin,	update_bin },
	{ RESERVED_FIELDS,			NULL,		80,	NULL,	print_reserved,	update_reserved },
	{ "Product Name",			"name",		16,	NULL,	print_ascii,	update_ascii },
	{ "Product Options #1",			"opt1",		16,	NULL,	print_ascii,	update_ascii },
	{ "Product Options #2",			"opt2",		16,	NULL,	print_ascii,	update_ascii },
	{ "Product Options #3",			"opt3",		16,	NULL,	print_ascii,	update_ascii },
	{ RESERVED_FIELDS,			NULL,		64,	NULL,	print_reserved,	update_ascii },
};

struct field layout_v4[21] = {
	{ "Major Revision",			"major",	2,	NULL,	print_bin_ver,	update_bin_ver },
	{ "Minor Revision",			"minor",	2,	NULL,	print_bin_ver,	update_bin_ver },
	{ "1st MAC Address",			"mac1",		6,	NULL,	print_mac,	update_mac },
	{ "2nd MAC Address",			"mac2",		6,	NULL,	print_mac,	update_mac },
	{ "Production Date",			"date",		4,	NULL,	print_date,	update_date },
	{ "Serial Number",			"sn",		12,	NULL,	print_bin_rev,	update_bin_rev },
	{ "3rd MAC Address (WIFI)",		"mac3",		6,	NULL,	print_mac,	update_mac },
	{ "4th MAC Address (Bluetooth)",	"mac4",		6,	NULL,	print_mac,	update_mac },
	{ "Layout Version",			"layout",	1,	NULL,	print_bin,	update_bin },
	{ "CompuLab EEPROM ID",			"id",		3,	NULL,	print_bin,	update_bin },
	{ "5th MAC Address",			"mac5",		6,	NULL,	print_mac,	update_mac },
	{ "6th MAC Address",			"mac6",		6,	NULL,	print_mac,	update_mac },
	{ RESERVED_FIELDS,			NULL,		4,	NULL,	print_reserved,	update_reserved },
	{ RESERVED_FIELDS,			NULL,		64,	NULL,	print_reserved,	update_reserved },
	{ "Product Name",			"name",		16,	NULL,	print_ascii,	update_ascii },
	{ "Product Options #1",			"opt1",		16,	NULL,	print_ascii,	update_ascii },
	{ "Product Options #2",			"opt2",		16,	NULL,	print_ascii,	update_ascii },
	{ "Product Options #3",			"opt3",		16,	NULL,	print_ascii,	update_ascii },
	{ "Product Options #4",			"opt4",		16,	NULL,	print_ascii,	update_ascii },
	{ "Product Options #5",			"opt5",		16,	NULL,	print_ascii,	update_ascii },
	{ RESERVED_FIELDS,			NULL,		32,	NULL,	print_reserved,	update_ascii },
};

struct field layout_unknown[1] = {
	{ NO_LAYOUT_FIELDS, NULL, 256, NULL, print_bin_raw, update_bin },
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
 * @layout:	An initialized layout.
 * @data:	A data array. Each element contains the following:
 * 		start: The first byte in EEPROM to be written.
 * 		end: The last byte in EEPROM to be written.
 * 		value: The value to be written to EEPROM.
 *
 * Returns: number of updated bytes.
 */
static int update_bytes(struct layout *layout, struct data_array *data)
{
	int updated_bytes = 0;

	for (int i = 0; i < data->size; i++) {
		int offset_start = data->bytes_changes[i].start;
		int offset_end = data->bytes_changes[i].end;

		if (offset_start < 0 || offset_start >= EEPROM_SIZE ||
		    offset_end < offset_start || offset_end >= EEPROM_SIZE) {
			fprintf(stderr, "Invalid offset '0x%02x", offset_start);
			if (offset_end != offset_start)
				fprintf(stderr, "-0x%02x", offset_end);

			fprintf(stderr, "'; will not update!\n");
			return 0;
		}

		int value = data->bytes_changes[i].value;
		if (value < 0 || value > 255){
			fprintf(stderr, "Invalid value '0x%02x' at '0x%02x'; "
				"will not update!\n", value, offset_start);
			return 0;
		}

		size_t range = offset_end - offset_start + 1;
		memset(layout->data + offset_start, value, range);
		updated_bytes += range;
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

	if (!new_data)
		return 0;

	if (!field_name)
		return -1;

	for (int i = 0; i < layout->num_of_fields; i++) {
		if (fields[i].name == RESERVED_FIELDS ||
		    (strcmp(fields[i].name, field_name) &&
		     strcmp(fields[i].short_name, field_name)))
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
 * @layout:	An initialized layout.
 * @data:	A data array. Each element contains field and value strings
 *
 * Returns: number of updated fields.
 */
static int update_fields(struct layout *layout, struct data_array *data)
{
	int updated_fields_cnt = 0;

	for (int i = 0; i < data->size; i++) {
		if (update_field(layout, data->fields_changes[i].field,
				 data->fields_changes[i].value)) {
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
	if (!layout)
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
