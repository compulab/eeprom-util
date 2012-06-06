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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "field.h"

#define PRINT_FIELD_SEGMENT	"%-30s"

struct field set_field(char *name, int size, char *delim,
			void (*print)(struct field self),
			void (*update)(struct field *self, char *value))
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
void print_bin_ver(struct field self)
{
	if ((self.buf[0] == 0xff) && (self.buf[1] == 0xff)) {
		self.buf[0] = 0;
		self.buf[1] = 0;
	}

	printf(PRINT_FIELD_SEGMENT, self.name);
	printf("%#.2f\n", (self.buf[1] << 8 | self.buf[0]) / 100.0);
}

/* For printing binary data in reverse. */
void print_bin_rev(struct field self)
{
	int i;

	printf(PRINT_FIELD_SEGMENT, self.name);
	for (i = self.size - 1; i > 0; i--)
		printf("%02x%s", self.buf[i], self.delim);

	printf("%02x\n", self.buf[0]);
}

/* For printing binary data. */
void print_bin(struct field self)
{
	int i;

	printf(PRINT_FIELD_SEGMENT, self.name);
	for (i = 0; i < self.size - 1; i++)
		printf("%02x%s", self.buf[i], self.delim);

	printf("%02x\n", self.buf[self.size - 1]);
}

void print_date(struct field self)
{
	printf(PRINT_FIELD_SEGMENT, self.name);
	printf("%d%s%d%s%d\n", self.buf[0], self.delim, self.buf[1],
	       self.delim, self.buf[3] << 8 | self.buf[2]);
}

/* For printing data meant to be interpreted as an ASCII string. */
void print_ascii(struct field self)
{
	char format[8];

	sprintf(format, "%%.%ds\n", self.size);
	printf(PRINT_FIELD_SEGMENT, self.name);
	printf(format, self.buf);
}

/* For printing the "Reserved field" section. */
void print_reserved(struct field self)
{
	printf(PRINT_FIELD_SEGMENT, "Reserved fields\t");
	printf("(%d bytes)\n", self.size);
}

/*====================Update field functions==================*/
void update_binary(struct field *self, char *value)
{
	int i;
	char *tok = strtok(value, " ");

	for (i = 0; tok && i < self->size; i++) {
		if (strcmp(tok, ""))
			self->buf[i] = (unsigned char)strtol(tok, 0, 0);

		tok = strtok(NULL, " ");
	}
}

void update_ascii(struct field *self, char *value)
{
	int i;

	for (i = 0; i < self->size; i++)
		self->buf[i] = value[i];
}
