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

#ifndef _COMMON_
#define _COMMON_

#define COLOR_RED  "\x1B[31m"
#define COLOR_GREEN  "\x1B[32m"
#define COLOR_RESET  "\033[0m"

#define MIN_I2C_BUS 0
#define MAX_I2C_BUS 255

#define MIN_I2C_ADDR 0x03
#define MAX_I2C_ADDR 0x77

#define STR_ENO_MEM "Out of memory"

// Macro for printing error messages
#define eprintf(args...) fprintf (stderr, args)
// Macro for printing input error messages
#define ie_fmt(fmt) "Input error: " fmt " - Operation Aborted!\n"
#define ieprintf(fmt, ...) eprintf(ie_fmt(fmt), ##__VA_ARGS__)
// Macro for printing layout error messages
#define le_fmt(fmt) "Layout error: " fmt " - Operation Aborted!\n"
#define leprintf(fmt, ...) eprintf(le_fmt(fmt), ##__VA_ARGS__)

// Macro for handling debug checks
#ifndef DEBUG
#define ASSERT(args) ((void)0)
#else // ifndef DEBUG
void failed_assert(const char* func, char *file, int line);
#define ASSERT(arg) ((arg) ? ((void)0) : \
	failed_assert(__func__, __FILE__, __LINE__))
#endif // ifndef DEBUG

struct field_change {
	char *field;
	char *value;
};

struct bytes_change {
	int start;
	int end;
	int value;
};

struct bytes_range {
	int start;
	int end;
};

struct data_array {
	int size;
	union {
		struct field_change *fields_changes;
		struct bytes_change *bytes_changes;
		char **fields_list;
		struct bytes_range *bytes_list;
	};
};

#define STRTOI_STR_CON 1
#define STRTOI_STR_END 2
int strtoi_base(char **str, int *dest, int base);
int strtoi(char **str, int *dest);

#endif
