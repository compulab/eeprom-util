/*
 * Copyright (C) 2009-2011 CompuLab, Ltd.
 * Authors: Nikita Kiryanov <nikita@compulab.co.il>
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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "common.h"

#ifdef DEBUG
void failed_assert(const char* func, char *file, int line)
{
	eprintf("Assertion Failed in %s() (%s:%d)\n", func, file, line);
	exit(1);
}
#endif /* ifdef DEBUG */

/*
 * strtoi_base - convert to int using the given numerical base and point
 *		 to the first character after the number
 *
 * @str:	A pointer to a string containing an integer number at the
 *		beginning. On success the pointer will point to the first
 *		character after the number.
 * @dest:	A pointer where to save the int result
 * @base:	The numerical base of the characters in the input string.
 * 		If 0 the base is determined by the format.
 *
 * Returns:	STRTOI_STR_END on success and all characters read.
 *		STRTOI_STR_CON on success and additional characters remain.
 *		-ERANGE or -EINVAL on failure
 */
int strtoi_base(char **str, int *dest, int base)
{
	ASSERT(str && *str && dest);

	if (**str == '\0')
		return -EINVAL;

	char *endptr;
	errno = 0;
	int num = strtol(*str, &endptr, base);

	if (errno != 0)
		return -errno;

	if (*str == endptr)
		return -EINVAL;

	*dest = num;
	*str = endptr;

	if (*endptr == 0)
		return STRTOI_STR_END;

	return STRTOI_STR_CON;
}

/*
 * strtoi - convert to int and point to the first character after the number
 *
 * @str:	A pointer to a string containing an integer number at the
 *		beginning. On success the pointer will point to the first
 *		character after the number.
 * @dest:	A pointer where to save the int result
 *
 * Returns:	STRTOI_STR_END on success and all characters read.
 *		STRTOI_STR_CON on success and additional characters remain.
 *		-ERANGE or -EINVAL on failure
 */
int strtoi(char **str, int *dest)
{
	return strtoi_base(str, dest, 0);
}
