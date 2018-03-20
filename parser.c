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
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "common.h"
#include "command.h"
#include "auto_generated.h"

#ifdef ENABLE_WRITE
static inline bool write_enabled(void) { return true; }
#else
static inline bool write_enabled(void) { return false; }
#endif

void print_banner(void)
{
	char *version = strnlen(VERSION, 20) ? " version " VERSION : "";
	char *date = " (" BUILD_DATE " - " BUILD_TIME ")";

	printf("CompuLab EEPROM utility%s%s\n\n", version, date);
}

static void print_help(void)
{
	print_banner();
	printf("Usage: eeprom-util list [<bus_num>]\n");
	printf("       eeprom-util read [-l <layout_version>] <bus_num> <device_addr>\n");


	if (write_enabled()) {
		printf("       eeprom-util write (fields|bytes) [-l <layout_version>] <bus_num> <device_addr> CHANGES\n");
		printf("       eeprom-util clear <bus_num> <device_addr>\n");
	}

	printf("       eeprom-util version|-v|--version\n");
	printf("       eeprom-util [help|-h|--help]\n");

	printf("\n"
		"COMMANDS\n"
		"       list\tList device addresses accessible via i2c\n"
		"       read\tRead from EEPROM\n");

	if (write_enabled()) {
		printf("       write\tWrite to EEPROM\n");
		printf("       clear\tClear EEPROM\n");
        }

	printf("       version\tPrint the version banner and exit\n"
	       "       help\tPrint this help and exit\n");
	printf("\n"
	       "LAYOUT VERSIONS\n"
	       "The -l option can be used to force the utility to interpret the EEPROM data using the chosen layout.\n"
	       "If the -l option is omitted, the utility will auto detect the layout based on the data in the EEPROM.\n"
	       "The following values can be provided with the -l option:\n"
	       "       auto			use auto-detection to print layout\n"
	       "       legacy, 1, 2, 3, 4	print according to layout version\n"
	       "       raw			print raw data\n");

	if (write_enabled())
		printf("\n"
			"CHANGES FORMAT\n"
			"The list of changes to the write command can be passed inline:\n"
			"       eeprom-util write fields [-l <layout_version>] <bus_num> <device_addr> [<field_name>=<value> ]*\n"
			"       eeprom-util write bytes [-l <layout_version>] <bus_num> <device_addr> [<offset>[-<offset-end>],<value> ]*\n"
			"or via file input:\n"
			"       eeprom-util write (fields | bytes) [-l <layout_version>] <bus_num> <device_addr> < file\n"
			"\nWhen file input is used, each <field_name>=<value> or <offset>,<value> pair should be on its own line,\n"
			"and no quote marks are necessary if there are spaces in either <field_name> or <value>\n"
			"\nWhen writing a range of bytes use the syntax:	[<offset>[-<offset-end>],<value> ]* \n"
			"Range is inclusive. Range changes can be mixed with non-range changes.\n");

	printf("\n");
}

static void cond_usage_exit(bool cond, const char *message)
{
	if (!cond)
		return;

	fprintf(stderr, COLOR_RED "%s" COLOR_RESET, message);
	print_help();
	exit(1);
}

static void usage_exit(void)
{
	print_help();
	exit(0);
}

static enum action parse_action(int argc, char *argv[])
{
	if (!strncmp(argv[0], "list", 4)) {
		return EEPROM_LIST;
	} else if (!strncmp(argv[0], "read", 4)) {
		return EEPROM_READ;
	} else if (write_enabled() && !strncmp(argv[0], "clear", 5)) {
		return EEPROM_CLEAR;
	} else if (write_enabled() && !strncmp(argv[0], "write", 5)) {
		if (argc > 1) {
			if (!strncmp(argv[1], "fields", 6)) {
				return EEPROM_WRITE_FIELDS;
			} else if (!strncmp(argv[1], "bytes", 5)) {
				return EEPROM_WRITE_BYTES;
			}
		}
	} else if (!strncmp(argv[0], "help", 4) ||
		!strncmp(argv[0], "-h", 2) ||
		!strncmp(argv[0], "--help", 6)) {
		usage_exit();
	} else if (!strncmp(argv[0], "version", 7) ||
		!strncmp(argv[0], "-v", 2) ||
		!strncmp(argv[0], "--version", 9)) {
		print_banner();
		exit(0);
	}

	cond_usage_exit(true, "Unknown function!\n");
	return EEPROM_ACTION_INVALID; //To appease the compiler
}

static enum layout_version parse_layout_version(char *str)
{
	char *endptr;

	if (!strncmp(str, "legacy", 6))
		return LAYOUT_LEGACY;
	else if (!strncmp(str, "raw", 3))
		return RAW_DATA;
	else if (!strncmp(str, "auto", 4))
		return LAYOUT_AUTODETECT;
	else if(!strncmp(str, "v", 1))
		str++;

	int layout = strtol(str, &endptr, 10);
	if (*endptr != '\0' || endptr == str ||
	    layout < LAYOUT_AUTODETECT || layout >= LAYOUT_UNRECOGNIZED)
		return LAYOUT_UNRECOGNIZED;
	else
		return (enum layout_version)layout;
}

int parse_numeric_param(char *str, char *error_message)
{
	char *endptr;
	int value = strtol(str, &endptr, 0);
	cond_usage_exit(*endptr != '\0', error_message);

	return value;
}

#ifdef ENABLE_WRITE
static int alloc_cpy_str(char **dest, char *source)
{
	*dest = malloc(strlen(source) + 1);
	if (!*dest)
		return -ENOMEM;

	strcpy(*dest, source);

	return 0;
}

// The max size of a conventional line from stdin. defined as:
// MAX[ (field name) + (1 for '=') + (field value) + (1 for '/0') ]
#define STDIN_LINE_SIZE 	47

// The max line count from stdin. defines as num of fields in layout v4
#define STDIN_LINES_COUNT	18

// The size of each reallocation of stdin line size or line count
#define STDIN_REALLOC_SIZE 	10

/*
 * mem_realloc - Realloc memory if needed
 *
 * If memory needed is not smaller than memory available, allocate more memory
 * for the same allocation and update the pointer.
 *
 * @ptr 	A pointer to the allocated memory
 * @mem_needed	The current memory size (in blocks) needed for the allocation
 * @mem_size	A pointer to where the current memory size (in blocks) is saved
 * @bytes	The size of every block of memory
 *
 * Returns:	0 on success. EINVAL or ENOMEM on failure.
 */
static int mem_realloc(void **ptr, unsigned int mem_needed,
			unsigned int *mem_size, size_t bytes)
{
	if (!ptr || !mem_size)
		return -EINVAL;

	if (mem_needed < *mem_size)
		return 0;

	unsigned int new_size = *mem_size + STDIN_REALLOC_SIZE;
	void *new_alloc = realloc(*ptr, new_size * bytes);
	if (!new_alloc)
		return -ENOMEM;

	*mem_size = new_size;
	*ptr = new_alloc;
	return 0;
}

/*
 * read_line_stdin - Read one line from stdin. Ignore empty lines.
 *
 * @line	A pointer to where a string will be allocated and populated
 *		with the next non empty line from stdin.
 *
 * Returns:	0 on success. EINVAL or ENOMEM on failure.
 */
static int read_line_stdin(char **line)
{
	if (!line)
		return -EINVAL;

	int value = fgetc(stdin);
	while (value == '\n')
		value = fgetc(stdin);

	if (value == EOF) {
		*line = NULL;
		return 0;
	}

	unsigned int msize = STDIN_LINE_SIZE, pos = 0;
	*line = malloc(msize * sizeof(char));
	if (!*line)
		return -ENOMEM;

	while (value != EOF && value != '\n') {
		(*line)[pos++] = value;
		int ret = mem_realloc((void**)line, pos, &msize, sizeof(char));
		if (ret) {
			free(*line);
			return ret;
		}

		value = fgetc(stdin);
	}

	(*line)[pos] = '\0';
	return 0;
}

/*
 * free_stdin - Free allocations made by read_lines_stdin()
 *
 * @input	An allocated array of strings
 * @size:	The size of the allocated string array
 */
static void free_stdin(char **input, int size)
{
	for (int i = 0; i < size; i++)
		free(input[i]);
	free(input);
}

/*
 * read_lines_stdin - Read non empty lines from stdin.
 *
 * Allocate an array of strings. populate it with non empty lines from stdin.
 * Handle errors by printing an error message to the user, clear the allocated
 * memory and propagate the error.
 *
 * @input	A pointer to where an allocated array of strings will be
 *		written. Each string is a non empty line from stdin.
 * @size:	A pointer to where the size of the allocated string array
 *		will be written.
 *
 * Returns:	0 on success. EINVAL or ENOMEM on failure.
 */
static int read_lines_stdin(char ***input, int *size)
{
	if (!input || !size) {
		fprintf(stderr, "%s: Internal error! (%d - %s)\n",
			__func__, EINVAL, strerror(EINVAL));
		return -EINVAL;
	}

	unsigned int msize = STDIN_LINES_COUNT, i = 0;
	*input = malloc(msize * sizeof(char *));
	if (!*input) {
		perror("Out of memory");
		return -ENOMEM;
	}

	char *line;
	int ret = read_line_stdin(&line);
	if (ret || !line)
		goto cleanup;

	while (line) {
		(*input)[i++] = line;
		ret = mem_realloc((void**)input, i, &msize, sizeof(char *));
		if (ret)
			goto cleanup;

		ret = read_line_stdin(&line);
		if (ret)
			goto cleanup;
	}

	*size = i;
	return 0;

cleanup:
	if (ret == -ENOMEM)
		perror("Out of memory");
	else if (ret == -EINVAL)
		fprintf(stderr, "%s: Internal error! (%d - %s)\n",
			__func__, -ret, strerror(-ret));

	free_stdin(*input, i);

	*size = 0;
	return ret;
}

/*
 * valid_key - check if byte offset key is valid.
 *
 * Valid byte offset is a positive number.
 * If a range is passed in key, the first offset must be
 * smaller then the second.
 *
 * @key - one byte offset or byte range to be checked.
 *
 * Returns 1 if key is valid, 0 if not.
 */
static bool valid_key(char *key)
{
	char *endptr1;
	int key_num1, key_num2 = 0;

	key_num1 = strtol(key, &endptr1, 0);
	if (endptr1 == key)
		return false;

	if (*endptr1 != '\0') {
		endptr1++;
		key_num2 = strtol(endptr1, NULL, 0);
		if (key_num1 >= key_num2)
			return false;
	}

	return key_num1 >= 0 && key_num2 >=0;
}

/*
 * parse_new_data - parse the strings representing new per-key data
 *
 * Allocate a strings_pair array and populate it with (key, new_data) values
 * as extracted from the input strings.
 *
 * @changes_size:	The size of changes[]
 * @changes:		A string array containing "key<delim>new_data" strings
 * @delim:		The delimiter string between key and new_data
 *
 * Returns: Allocated strings_pair populated with field_changes_size pairs of
 * 			(key, new_data) values.
 */
static struct strings_pair *parse_new_data(int changes_size, char *changes[],
					   char *delim, bool is_bytes)
{
	struct strings_pair *pairs;
	pairs = malloc(sizeof(struct strings_pair) * changes_size);
	if (!pairs) {
		perror("Out of memory!");
		return NULL;
	}

	int i, sizeof_delim = strlen(delim);
	for (i = 0; i < changes_size; i++) {
		char *key, *value;
		if (!strncmp(changes[i], delim, sizeof_delim)) {
			goto cleanup;
		} else {
			key = strtok(changes[i], delim);
			// if writing bytes, check if offset is valid
			if (is_bytes && !valid_key(key)) {
				fprintf(stderr, "Invalid offset '%s';"
					"will not update!\n", key);
				goto cleanup;
			}

			value = strtok(NULL, delim);
			if (!value) {
				fprintf(stderr, "Invalid value, will not update!\n");
				goto cleanup;
			}
		}

		if (alloc_cpy_str(&pairs[i].key, key)) {
			perror("Out of memory!");
			goto cleanup;
		}

		if (alloc_cpy_str(&pairs[i].value, value)) {
			free(pairs[i].key);
			perror("Out of memory!");
			goto cleanup;
		}
	}

	return pairs;
cleanup:
	for (--i; i >= 0; i--) {
		free(pairs[i].key);
		free(pairs[i].value);
	}

	free(pairs);

	return NULL;
}
#else
static inline void free_stdin(char **input, int size) {}

static inline int read_lines_stdin(char ***input, int *size)
{
	return -ENOSYS;
}

static inline struct strings_pair *parse_new_data(int field_changes_size,
						  char *field_changes[],
						  char *delim,
						  bool is_bytes)
{
	return NULL;
}
#endif

#define NEXT_PARAM(argc, argv)	{(argc)--; (argv)++;}
#define STR_EINVAL_BUS		"Invalid bus number!\n"
#define STR_EINVAL_ADDR		"Invalid device address!\n"
#define STR_EINVAL_PARAM	"Invalid parameter for action!\n"
#define STR_ENO_PARAMS		"Missing parameters!\n"
#define STR_ENO_MEM		"Out of memory!\n"
#define MAX_I2C_BUS		255
int main(int argc, char *argv[])
{
	struct command *cmd;
	enum layout_version layout_ver = LAYOUT_AUTODETECT;
	enum action action = EEPROM_ACTION_INVALID;
	struct data_array data;
	int i2c_bus = -1, i2c_addr = -1, ret = -1;
	char **input = NULL;
	bool is_stdin = !isatty(STDIN_FILENO);
	errno = 0;

	if (argc <= 1)
		usage_exit();

	NEXT_PARAM(argc, argv); // Skip program name
	action = parse_action(argc, argv);
	NEXT_PARAM(argc, argv);
	if (action == EEPROM_LIST) {
		if (argc >= 1) {
			i2c_bus = parse_numeric_param(argv[0], STR_EINVAL_BUS);
			cond_usage_exit(i2c_bus > MAX_I2C_BUS ||
					i2c_bus < 0, STR_EINVAL_BUS);
		}
		goto done;
	}

	// parse_action already took care of parsing the bytes/fields qualifier
	if (action == EEPROM_WRITE_BYTES || action == EEPROM_WRITE_FIELDS)
		NEXT_PARAM(argc, argv);

	cond_usage_exit(argc <= 1, STR_ENO_PARAMS);
	if (!strcmp(argv[0], "-l")) {
		NEXT_PARAM(argc, argv);
		layout_ver = parse_layout_version(argv[0]);
		cond_usage_exit(layout_ver == LAYOUT_UNRECOGNIZED, STR_EINVAL_PARAM);
		NEXT_PARAM(argc, argv);
	}

	cond_usage_exit(argc <= 1, STR_ENO_PARAMS);
	i2c_bus = parse_numeric_param(argv[0], STR_EINVAL_BUS);
	cond_usage_exit(i2c_bus > MAX_I2C_BUS || i2c_bus < 0, STR_EINVAL_BUS);
	NEXT_PARAM(argc, argv);

	i2c_addr = parse_numeric_param(argv[0], STR_EINVAL_ADDR);
	cond_usage_exit(i2c_addr > MAX_I2C_BUS || i2c_addr < 0, STR_EINVAL_BUS);
	NEXT_PARAM(argc, argv);

	if (action == EEPROM_READ || action == EEPROM_CLEAR)
		goto done;

	input = argv;
	if (is_stdin && read_lines_stdin(&input, &argc))
		return 1;

	cond_usage_exit(argc == 0, STR_ENO_PARAMS);

	data.size = argc;
	if (action == EEPROM_WRITE_FIELDS)
		data.fields_changes = parse_new_data(argc, input, "=", false);
	else if (action == EEPROM_WRITE_BYTES)
		data.bytes_changes = parse_new_data(argc, input, ",", true);

	// it is enough to test only one field in the union
	if (!data.fields_changes)
		goto clean_input;

done:
	cmd = new_command(action, i2c_bus, i2c_addr, layout_ver, &data);
	if (!cmd)
		perror(STR_ENO_MEM);
	else
		ret = cmd->execute(cmd);

	free_command(cmd);

	if (action == EEPROM_WRITE_FIELDS) {
		for (int i = 0; i < data.size; i++) {
			free(data.fields_changes[i].key);
			free(data.fields_changes[i].value);
		}
		free(data.fields_changes);
	} else if (action == EEPROM_WRITE_BYTES) {
		for (int i = 0; i < data.size; i++) {
			free(data.bytes_changes[i].key);
			free(data.bytes_changes[i].value);
		}
		free(data.bytes_changes);
	}

clean_input:
	if (input && is_stdin)
		free_stdin(input, argc);

	return ret ? 1 : 0;
}
