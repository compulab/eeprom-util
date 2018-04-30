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
		printf("       eeprom-util write (fields|bytes) [-l <layout_version>] <bus_num> <device_addr> DATA\n");
		printf("       eeprom-util clear [fields|bytes|all] <bus_num> <device_addr> [DATA]\n");
	}

	printf("       eeprom-util version|-v|--version\n");
	printf("       eeprom-util [help|-h|--help]\n");

	printf("\n"
		"COMMANDS\n"
		"   list 	List device addresses accessible via i2c\n"
		"   read 	Read from EEPROM\n");

	if (write_enabled()) {
		printf("   write	Write to EEPROM. Must specify if writing to 'fields' or 'bytes'\n");
		printf("   clear	Clear EEPROM. Default is 'all'. Other options are clearing 'fields' or 'bytes'.\n");
	}

	printf("   version	Print the version banner and exit\n"
	       "   help		Print this help and exit\n");
	printf("\n"
	       "LAYOUT VERSIONS\n"
	       "   The -l option can be used to force the utility to interpret the EEPROM data using the chosen layout.\n"
	       "   If the -l option is omitted, the utility will auto detect the layout based on the data in the EEPROM.\n"
	       "   The following values can be provided with the -l option:\n"
	       "      auto			use auto-detection to print layout\n"
	       "      legacy, 1, 2, 3, 4	print according to layout version\n"
	       "      raw			print raw data\n");

	if (write_enabled()) {
		printf("\n"
			"DATA FORMAT\n"
			"   Some commands require additional data input. The data can be passed inline or from standard input.\n"
			"   The patterns of the data input for each command are listed as follows:\n"
			"      write fields:	[<field_name>=<value> ]*\n"
			"      write bytes: 	[<offset>[-<offset-end>],<value> ]*\n"
			"      clear fields:	[<field_name> ]*\n"
			"      clear bytes: 	[<offset>[-<offset-end>] ]*\n\n"

			"   When using inline input:\n"
			"      * Each entry should be separated by a white space.\n"
			"      * Quote marks are needed if spaces exist in an entry.\n\n"

			"   When using standard input:\n"
			"      * Each entry should be on its own line.\n"
			"      * Quote marks are not needed if spaces exist in an entry.\n\n"

			"   Notes for bytes:\n"
			"      * Offset range is inclusive. Range inputs can be mixed with non-range inputs.\n"
			"      * Writing a byte value to an offset:		<offset>,<value>\n"
			"      * Writing a byte value to all offsets in range:	<offset>-<offset-end>,<value>\n\n"

			"   Notes for fields:\n"
			"      * The value input should be in the same pattern like in the output of the read command.\n"
			);

		printf("\n"
			"USAGE EXAMPLE\n"
			"   The input for the following command can be passed inline:\n"
			"      eeprom-util write fields 6 0x20 \"Major Revision=1.20\" \"Production Date=01/Feb/2018\" \\\n"
			"         \"1st MAC Address=01:23:45:67:89:ab\"\n"
			"   or via the standard input:\n"
			"      eeprom-util write fields 6 0x20 < fields_data\n"
			"   Where fields_data is the name of a file containing 3 non empty lines:\n"
			"      Major Revision=1.20\n"
			"      Production Date=01/Feb/2018\n"
			"      1st MAC Address=01:23:45:67:89:ab\n"
			);
	}

	printf("\n");
}

static void message_exit(const char *message)
{
	ASSERT(message);

	eprintf(COLOR_RED "%s" COLOR_RESET, message);
	print_help();
	exit(1);
}

static void cond_usage_exit(bool cond, const char *message)
{
	if (!cond)
		return;

	message_exit(message);
}

static void usage_exit(void)
{
	print_help();
	exit(0);
}

static enum action parse_action(int argc, char *argv[])
{
	ASSERT(argv && argc > 0);

	if (!strncmp(argv[0], "list", 4)) {
		return EEPROM_LIST;
	} else if (!strncmp(argv[0], "read", 4)) {
		return EEPROM_READ;
	} else if (write_enabled() && !strncmp(argv[0], "clear", 5)) {
		if (argc > 1 && (!strncmp(argv[1], "fields", 6)))
			return EEPROM_CLEAR_FIELDS;
		if (argc > 1 && (!strncmp(argv[1], "bytes", 5)))
			return EEPROM_CLEAR_BYTES;

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

	message_exit("Unknown function!\n");
	return EEPROM_ACTION_INVALID; //To appease the compiler
}

static enum layout_version parse_layout_version(char *str)
{
	ASSERT(str);

	if (!strncmp(str, "legacy", 6))
		return LAYOUT_LEGACY;
	else if (!strncmp(str, "raw", 3))
		return RAW_DATA;
	else if (!strncmp(str, "auto", 4))
		return LAYOUT_AUTODETECT;
	else if(!strncmp(str, "v", 1))
		str++;

	int layout = LAYOUT_UNRECOGNIZED;
	if (strtoi_base(&str, &layout, 10) != STRTOI_STR_END)
		message_exit("Invalid layout version!\n");

	if (layout < LAYOUT_AUTODETECT || layout >= LAYOUT_UNRECOGNIZED)
		message_exit("Unknown layout version!\n");

	return (enum layout_version)layout;
}

int parse_numeric_param(char *str, char *error_message)
{
	ASSERT(str);

	char *endptr;
	int value = strtol(str, &endptr, 0);
	cond_usage_exit(*endptr != '\0', error_message);

	return value;
}

#ifdef ENABLE_WRITE
// The max size of a conventional line from stdin. defined as:
// MAX[ (field name) + (1 for '=') + (field value) + (1 for '/0') ]
#define STDIN_LINE_SIZE 	47

// The max line count from stdin. defines as num of fields in layout v4
#define STDIN_LINES_COUNT	18

// The size of each reallocation of stdin line size or line count
#define STDIN_REALLOC_SIZE 	10

// Macro for printing input syntax error messages
#define iseprintf(str) ieprintf("Syntax error in \"%s\"", str)

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
 * Returns:	0 on success. -ENOMEM on failure.
 */
static int mem_realloc(void **ptr, unsigned int mem_needed,
			unsigned int *mem_size, size_t bytes)
{
	ASSERT(ptr && mem_size);
	ASSERT(mem_needed > 0 && *mem_size > 0 && bytes > 0);

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
 * Returns:	0 on success. -ENOMEM on failure.
 */
static int read_line_stdin(char **line)
{
	ASSERT(line);

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
	ASSERT(input);
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
 * Returns:	0 on success. -ENOMEM on failure.
 */
static int read_lines_stdin(char ***input, int *size)
{
	ASSERT(input && size);

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
	free_stdin(*input, i);
	*size = 0;
	return ret;
}

/*
 * parse_bytes_list - parse the strings representing bytes offsets
 *
 * Allocate a bytes_range array, verify the syntax of the input strings and
 * for each array element set the bytes 'start' offset and 'end' offset as
 * extracted from the input strings.
 *
 * @size:	The size of input[]
 * @input:	A string array containing strings in the expected format:
 * 		"<offset>[-<offset-end>]"
 *
 * Returns:	Allocated and populated bytes_range array on success
 * 		NULL on failure.
 */
static struct bytes_range *parse_bytes_list(int size, char *input[])
{
	ASSERT(input && *input);
	ASSERT(size > 0);

	struct bytes_range *bytes_list;
	bytes_list = malloc(sizeof(struct bytes_range) * size);
	if (!bytes_list) {
		perror("Out of memory!");
		return NULL;
	}

	int i;
	for (i = 0; i < size ; i++) {
		char *str = input[i];
		int ret = strtoi(&str, &bytes_list[i].start);

		if ((ret == STRTOI_STR_CON) && (*str == '-')) {
			str++;
			if (strtoi(&str, &bytes_list[i].end) != STRTOI_STR_END)
				goto syntax_error;
		} else if (ret == STRTOI_STR_END) {
			bytes_list[i].end = bytes_list[i].start;
		} else {
			goto syntax_error;
		}
	}

	return bytes_list;

syntax_error:
	iseprintf(input[i]);
	free(bytes_list);
	return NULL;
}

/*
 * parse_bytes_changes - parse the strings representing new bytes values
 *
 * Allocate a bytes_change array, verify the syntax of the input strings and
 * for each array element set the bytes 'start' offset, 'end' offset and the
 * 'value' to be written as extracted from the input strings.
 *
 * @size:	The size of input[]
 * @input:	A string array containing strings in the expected format:
 * 		"<offset>[-<offset-end>],<value>"
 *
 * Returns:	Allocated and populated bytes_change array on success
 * 		NULL on failure.
 */
static struct bytes_change *parse_bytes_changes(int size, char *input[])
{
	ASSERT(input && *input);
	ASSERT(size > 0);

	struct bytes_change *changes;
	changes = malloc(sizeof(struct bytes_change) * size);
	if (!changes) {
		perror("Out of memory!");
		return NULL;
	}

	int i;
	for (i = 0; i < size ; i++) {
		char *change = input[i];

		if (strtoi(&change, &changes[i].start) != STRTOI_STR_CON)
			goto syntax_error;

		if (*change == '-') {
			change++;
			if (strtoi(&change, &changes[i].end) != STRTOI_STR_CON)
				goto syntax_error;
		} else {
			changes[i].end = changes[i].start;
		}

		if (*change != ',')
			goto syntax_error;

		change++;
		if (strtoi(&change, &changes[i].value) != STRTOI_STR_END)
			goto syntax_error;
	}

	return changes;

syntax_error:
	iseprintf(input[i]);
	free(changes);
	return NULL;
}

/*
 * parse_field_changes - parse the strings representing new fields values
 *
 * Allocate a field_change array, verify the syntax of the input strings and
 * for each array element set the 'field' and 'value' strings as extracted
 * from the input strings.
 *
 * @size:	The size of input[]
 * @input:	A string array containing strings in the expected format:
 * 		"<field_name>=<value>"
 *
 * Returns:	Allocated and populated field_change array on success
 * 		NULL on failue.
 */
static struct field_change *parse_field_changes(int size, char *input[])
{
	ASSERT(input && *input);
	ASSERT(size > 0);

	struct field_change *changes;
	changes = malloc(sizeof(struct field_change) * size);
	if (!changes) {
		perror("Out of memory!");
		return NULL;
	}

	int i;
	for (i = 0; i < size; i++) {
		char *delim = strchr(input[i], '=');
		if (!delim || input[i] == delim || *(delim + 1) == '\0')
			goto syntax_error;

		*delim = '\0';
		changes[i].field = input[i];
		changes[i].value = delim + 1;
	}

	return changes;

syntax_error:
	iseprintf(input[i]);
	free(changes);
	return NULL;
}

#else
static inline void free_stdin(char **input, int size) {}

static inline int read_lines_stdin(char ***input, int *size)
{
	return -ENOSYS;
}

static inline struct bytes_range *parse_bytes_list(int size, char *input[])
{
	return NULL;
}

static inline struct bytes_change *parse_bytes_changes(int size, char *input[])
{
	return NULL;
}

static inline struct field_change *parse_field_changes(int size, char *input[])
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
	if (action == EEPROM_LIST && argc == 0)
		goto done;

	// parse_action already took care of parsing the bytes/fields qualifier
	if (action == EEPROM_WRITE_BYTES || action == EEPROM_WRITE_FIELDS ||
	    action == EEPROM_CLEAR_FIELDS || action == EEPROM_CLEAR_BYTES)
		NEXT_PARAM(argc, argv);

	// The "all" qualifier is optional for clear command
	if (action == EEPROM_CLEAR && argc > 0 && !strncmp(argv[0], "all", 3))
		NEXT_PARAM(argc, argv);

	if (argc > 1 && !strcmp(argv[0], "-l")) {
		NEXT_PARAM(argc, argv);
		layout_ver = parse_layout_version(argv[0]);
		NEXT_PARAM(argc, argv);
	}

	cond_usage_exit(argc < 1, STR_ENO_PARAMS);
	i2c_bus = parse_numeric_param(argv[0], STR_EINVAL_BUS);
	cond_usage_exit(i2c_bus > MAX_I2C_BUS || i2c_bus < 0, STR_EINVAL_BUS);
	NEXT_PARAM(argc, argv);

	if (action == EEPROM_LIST)
		goto done;

	cond_usage_exit(argc < 1, STR_ENO_PARAMS);
	i2c_addr = parse_numeric_param(argv[0], STR_EINVAL_ADDR);
	cond_usage_exit(i2c_addr > 0x77 || i2c_addr < 0x03, STR_EINVAL_ADDR);
	NEXT_PARAM(argc, argv);

	if (action == EEPROM_READ || action == EEPROM_CLEAR)
		goto done;

	input = argv;
	if (is_stdin && read_lines_stdin(&input, &argc))
		return 1;

	cond_usage_exit(argc == 0, STR_ENO_PARAMS);

	data.size = argc;
	if (action == EEPROM_WRITE_FIELDS)
		data.fields_changes = parse_field_changes(argc, input);
	else if (action == EEPROM_WRITE_BYTES)
		data.bytes_changes = parse_bytes_changes(argc, input);
	else if (action == EEPROM_CLEAR_FIELDS)
		data.fields_list = input;
	else if (action == EEPROM_CLEAR_BYTES)
		data.bytes_list = parse_bytes_list(argc,input);

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

	if (action == EEPROM_WRITE_FIELDS)
		free(data.fields_changes);
	else if (action == EEPROM_WRITE_BYTES)
		free(data.bytes_changes);
	else if (action == EEPROM_CLEAR_BYTES)
		free(data.bytes_list);

clean_input:
	if (input && is_stdin)
		free_stdin(input, argc);

	return ret ? 1 : 0;
}
