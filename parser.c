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
#include <stdlib.h>
#include <string.h>
#include "pairs.h"
#include "parser.h"

#define DEFAULT_DRIVER_PATH	"/sys/bus/i2c/devices/3-0050/eeprom"
#define DEFAULT_I2C_PATH	"/dev/i2c-3"
#define DEFAULT_I2C_ADDR	0x50

#define NEXT_OR_STOP(i) do {				\
				(i)++;			\
				if ((i) == argc)	\
					goto done;	\
			} while (0);

#ifdef ENABLE_WRITE
static inline int write_enabled(void) { return 1; }
#else
static inline int write_enabled(void) { return 0; }
#endif

static void examples_exit(void)
{
	printf("Reading EEPROM via driver (default address: 0x%x):\n"
	       "\teeprom-util read -d\n", DEFAULT_I2C_ADDR);

	if (write_enabled()) {
		printf("Writing '4c' to bytes 0, 5, and 23 via I2C:\n"
		       "\teeprom-util write -i "
		       "--change-bytes=0,4c:5,4c:23,4c\n");

		printf("Changing fields \"Bytes Field\" and \"String Field\" "
		       "to bytes 2,3,4 and string \"Hello World!\", "
		       "via driver:\n"
		       "\teeprom-util write -d -- \"Bytes Field=2 3 4\" "
		       "\"String Field=Hello World\"\n");

		printf("Doing the above via I2C with "
		       "custom device file and address:\n"
		       "\teeprom-util write -i --addr=0x%x "
		       "--path=/path/to/devfile -- \"Bytes Field=2 3 4\" "
		       "\"String Field=Hello World\"\n", DEFAULT_I2C_ADDR);
	}

	printf("\n");
	exit(0);
}

/*
 * Prints usage guide and exits. General format is:
 * <function> <mode> [--addr=num] [--path=file_path] -- [changes]
 */
static void usage_exit(const char *message)
{
	printf("%s", message);
	printf("Usage: eeprom-util <function> "
		"(-d|-i [--addr=<address>]) [-p <devfile>]");

	if (write_enabled())
		printf(" [<data>]");

	printf("\n\n"
		"function:\n"
		"help\t\t- Print this help and exit ([-h|--help|help])\n"
		"examples\t- Print usage examples and exit\n"
		"list\t\t- List device addresses accessible via "
				"the i2c dev files\n"
		"read\t\t- Read from EEPROM\n");

	if (write_enabled())
		printf("write\t\t- Write to EEPROM\n");

	printf("\n"
		"Flags:\n"
		"-d, --driver\t\t- Use kernel eeprom driver for I/O\n"
		"-i, --i2c\t\t- Use direct I2C access for I/O "
			"(custom address can be supplied)\n"
		"    --addr=<address>\t- Eeprom chip I2C address "
					"(default: 0x50)\n"
		"-p, --path=<devfile>\t- Path to the driver "
					"device file (/sys/.../eeprom) "
					"or the I2C device file (/dev/i2c-x)"
		"\n\n");

	if (write_enabled()) {
		printf("data:\n"
			"[-- [\"<name>=<vals>\"]* | "
			"--change-bytes=<offset>,<val>[:<offset>,<val>]*]\n"
			"\n");
	}

	printf("\n");
	exit(0);
}

/*
 * All of our special value passing arguments are in the form of
 * "--type=value". This method extracts the value.
 */
static char *extract_value(char *argv[], int arg_index)
{
	strtok(argv[arg_index], "=");

	return strtok(NULL, "=");
}

static enum action parse_action(char *argv[], int arg_index)
{
	if (!strcmp(argv[arg_index], "list"))
		return EEPROM_LIST;
	else if (!strcmp(argv[arg_index], "read"))
		return EEPROM_READ;
	else if (write_enabled() && !strcmp(argv[arg_index], "write"))
		return EEPROM_WRITE;
	else if (!strcmp(argv[arg_index], "examples"))
		examples_exit();
	else if (!strcmp(argv[arg_index], "help") ||
		!strcmp(argv[arg_index], "-h") ||
		!strcmp(argv[arg_index], "--help"))
		usage_exit("");
	else
		usage_exit("Unknown function specified!\n");

	return EEPROM_ACTION_INVALID; /* Not reached */
}

static enum mode parse_mode(char *argv[], int arg_index)
{
	if (!strcmp(argv[arg_index], "-d") ||
	    !strcmp(argv[arg_index], "--driver"))
		return EEPROM_DRIVER_MODE;
	else if (!strcmp(argv[arg_index], "-i") ||
		 !strcmp(argv[arg_index], "--i2c"))
		return EEPROM_I2C_MODE;
	else
		usage_exit("Unknown I/O mode specified!\n");

	return -1; /* Not reached */
}

static char *parse_path(char *argv[], int *arg_index)
{
	char *custom_path = NULL;

	if (!strcmp(argv[*arg_index], "-p")) {
		(*arg_index)++;
		custom_path = argv[*arg_index];
	} else if (!strncmp(argv[*arg_index], "--path=", 7)) {
		custom_path = extract_value(argv, *arg_index);
	}

	return custom_path;
}

#ifdef ENABLE_WRITE
/*
 * validate_byte_changes - Check how many pairs are in the --change-bytes
 * string.
 * @str:	The string to check.
 *
 * This function validates the following syntax:
 *	<offset>,<val>[:<offset>,<val>]*
 * offset and val are non-negative numbers.
 *
 * NOTE: this function's role is to help the parser, so it should not do things
 * that may interfere with parsing, like invoking strtok.
 *
 * Returns: how many pairs the string has, or -1 if it is malformed.
 */
static int validate_byte_changes(const char *str)
{
	unsigned int counter = 0;
	char *end_ptr;

	if (str == NULL)
		return -1;

	/*
	 * We take advantage of strtoul's behavior to check that a string starts
	 * with a number, and ends in a desired character (value of *end_ptr).
	 */
	while (*str) {
		/* First part of pair is a >=0 number that ends with a comma */
		if (*str == '-')
			return -1;

		strtoul(str, &end_ptr, 0);
		if (*end_ptr != ',')
			return -1;

		str = ++end_ptr;
		/*
		 * Second part of pair is a >=0 number that either ends
		 * the string, or ends with a pair delimiter ":"
		 */
		if (*str == '-' || *str == '\0')
			return -1;

		strtoul(str, &end_ptr, 0);
		if (*end_ptr == '\0') {
			counter++;
			break;
		}

		if (*end_ptr != ':')
			return -1;

		str = ++end_ptr;
		counter++;
	}

	/* Consider no changes as a syntax error */
	if (counter == 0)
		return -1;

	return counter;
}

static struct offset_value_pair *parse_change_bytes(char *changes, int size)
{
	char *tok, *end_ptr;
	int i = 0;
	struct offset_value_pair *res;

	if (size == 0)
		return NULL;

	res = (struct offset_value_pair *)malloc(size *
					sizeof(struct offset_value_pair));
	if (res == NULL)
		return NULL;

	tok = strtok(changes, ",");
	while (tok) {
		res[i].offset = strtoul(tok, &end_ptr, 0);
		if (*end_ptr != '\0')
			goto error;

		tok = strtok(NULL, ":");
		if (tok == NULL)
			goto error;

		res[i].value = strtoul(tok, &end_ptr, 0);
		if (*end_ptr != '\0')
			goto error;

		i++;
		tok = strtok(NULL, ",");
	}

	return res;

error:
	free(res);
	return NULL;
}

static struct strings_pair *parse_new_field_data(char *field_changes[],
							int field_changes_size)
{
	int i;
	struct strings_pair *res = (struct strings_pair *)malloc(
			sizeof(struct strings_pair) * field_changes_size);

	if (res == NULL)
		return res;

	for (i = 0; i < field_changes_size; i++) {
		res[i].key = strtok(field_changes[i], "=");
		res[i].value = strtok(NULL, "=");
	}

	return res;
}
#else
static struct offset_value_pair *parse_change_bytes(char *changes, int size)
{
	return NULL;
}
static struct strings_pair *parse_new_field_data(char *field_changes[],
							int field_changes_size)
{
	return NULL;
}
static int validate_byte_changes(char *str)
{
	return -1;
}
#endif

/*
 * This function operates in stages of user input, whose general format can
 * be seen in usage_exit.
 */
void parse(int argc, char *argv[], struct command *command)
{
	int cli_arg = 1;
	int i2c_addr = -1;
	int new_data_size = -1;
	char *dev_file = NULL, *custom_path = NULL;
	struct offset_value_pair *new_byte_data = NULL;
	struct strings_pair *new_field_data = NULL;
	enum action action = EEPROM_ACTION_INVALID;
	enum mode mode = EEPROM_MODE_INVALID;
	char *tok;

	reset_command(command);

	if (argc <= 1)
		usage_exit("");

	action = parse_action(argv, cli_arg);
	if (action == EEPROM_LIST)
		goto done;

	cli_arg++;
	/* Reads and writes require additional parameters. */
	if (cli_arg == argc)
		usage_exit("Specified function implies "
			   "I/O mode to be specified!\n");

	mode = parse_mode(argv, cli_arg);
	if (mode == EEPROM_DRIVER_MODE) {
		dev_file = DEFAULT_DRIVER_PATH;
	} else if (mode == EEPROM_I2C_MODE) {
		dev_file = DEFAULT_I2C_PATH;
		i2c_addr = DEFAULT_I2C_ADDR;
	}

	NEXT_OR_STOP(cli_arg);
	/* Next argument might be --addr= */
	if (mode == EEPROM_I2C_MODE && !strncmp(argv[cli_arg], "--addr=", 7)) {
		tok = extract_value(argv, cli_arg);
		i2c_addr = strtol(tok, 0, 0);
		NEXT_OR_STOP(cli_arg);
	}

	/* Next argument might be file path */
	custom_path = parse_path(argv, &cli_arg);
	if (custom_path != NULL) {
		dev_file = custom_path;
		NEXT_OR_STOP(cli_arg);
	}

	if (!strncmp(argv[cli_arg], "--change-bytes=", 15)) {
		strtok(argv[cli_arg], "=");
		tok = strtok(NULL, "=");
		new_data_size = validate_byte_changes(tok);
		if (new_data_size < 0)
			usage_exit("Malformed change-bytes input!\n");

		new_byte_data = parse_change_bytes(tok, new_data_size);
		goto done;
	}

	if (strcmp(argv[cli_arg], "--"))
		usage_exit("Too many arguments! (Have you forgot the '--'?)\n");

	NEXT_OR_STOP(cli_arg);

	if (cli_arg == argc)
		return;

	new_data_size = argc - cli_arg;
	new_field_data = parse_new_field_data(argv + cli_arg, new_data_size);

done:
	if (setup_command(command, action, mode, i2c_addr,
			dev_file, new_byte_data, new_field_data, new_data_size))
		exit(1);
}
