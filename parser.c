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
 * along with this program; if not, write to the Free Software
 * Foundation.
 */
/* ------------------------------------------------------------------------- */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"

#define NEXT_OR_STOP(i) do {\
				(i)++;\
				if ((i) == argc)\
					return cli_cmd;\
			} while (0);

#ifdef ENABLE_WRITE
static inline int write_enabled(void) { return 1; }
#else
static inline int write_enabled(void) { return 0; }
#endif

/*
 * Prints usage guide and exits. General format is:
 * <function> <mode> [--addr=num] [--path=file_path] -- [changes]
 */
static void usage_exit(void)
{
	printf("Usage: eeprom-util <function> "
		"(-d|-i [--addr=<address>]) [-p <devfile>]"
		);

	if (write_enabled())
		printf(" [<data>]");

	printf("\n\n"
		"function:\n"
		"help\t- Print this help and exit ([-h|--help|help])\n"
		"list\t- List device addresses accessible via "
				"the i2c dev files\n"
		"read\t- Read from EEPROM\n"
		);

	if (write_enabled())
		printf("write\t- Write to EEPROM");

	printf("\n\n"
		"Flags:\n"
		"-d, --driver\t\t- Use kernel eeprom driver for I/O\n"
		"-i, --i2c\t\t- Use direct I2C access for I/O "
			"(custom address can be supplied)\n"
		"    --addr=<address>\t- Eeprom chip I2C address "
					"(default: 0x50)\n"
		"-p, --path=<devfile>\t- Path to the driver "
					"device file (/sys/.../eeprom) "
					"or the I2C device file (/dev/i2c-x)"
		"\n\n"
		);

	if (write_enabled()) {
		printf("data:\n"
			"[-- [\"<name>=<vals>\"]* | "
			"--change-bytes=<offset>,<val>[,<offset>,<val>]*]\n"
			"\n"
			);
		printf("Write EEPROM format:\n"
			"\tWriting fields:\n"
			"\t\"field1=this is ascii\" or "
				"\"Field 1=<byte1> <byte2> <byte3>\")\n"
			"\tWriting bytes:\n"
			"\tsupply a list of tuples (offset,val).\n"
			"\tExample: 0,1,2,4,3,10,4,9,5,10\n"
			);
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

static void parse_function(char *argv[], int arg_index,
					struct cli_command *cli_command)
{
	if (!strcmp(argv[arg_index], "list"))
		cli_command->action = LIST;
	else if (!strcmp(argv[arg_index], "read"))
		cli_command->action = READ;
	else if (write_enabled() && !strcmp(argv[arg_index], "write"))
		cli_command->action = WRITE;
	else
		usage_exit();
}

static void parse_mode(char *argv[], int arg_index,
					struct cli_command *cli_command)
{
	if (!strcmp(argv[arg_index], "-d") ||
	    !strcmp(argv[arg_index], "--driver"))
		cli_command->mode = DRIVER_MODE;
	else if (!strcmp(argv[arg_index], "-i") ||
		 !strcmp(argv[arg_index], "--i2c"))
		cli_command->mode = I2C_MODE;
	else
		usage_exit();
}

static int parse_path(char *argv[], int *arg_index,
					struct cli_command *cli_command)
{
	if (!strcmp(argv[*arg_index], "-p")) {
		(*arg_index)++;
		cli_command->dev_file = argv[*arg_index];
	} else if (!strncmp(argv[*arg_index], "--path=", 7)) {
		cli_command->dev_file = extract_value(argv, *arg_index);
	}

	return cli_command->dev_file ? 1 : 0;
}

#ifdef ENABLE_WRITE
/*
 * This method records the location of the new data to write.
 * Currently data can come in two forms: "--change-bytes=..." and
 * "-- field1=value field2=value...".
 */
static void parse_new_data(int argc, char *argv[], int arg_index,
				struct cli_command *cli_command)
{
	int i = 0;

	if (!strncmp(argv[arg_index], "--change-bytes=", 15)) {
		strtok(argv[arg_index], "=");
		cli_command->new_byte_data = strtok(NULL, "=");
		return;
	}

	if (strcmp(argv[arg_index], "--"))
		usage_exit();

	arg_index++;
	if (arg_index == argc)
		return;

	/*
	 * new_field_data will point to the remainder of argv,
	 * where the new field values reside. We move all the
	 * pointers one step back to place a null pointer in
	 * the end of argv.
	 */
	cli_command->new_field_data = argv + arg_index - 1;
	for (; arg_index < argc; arg_index++, i++)
		cli_command->new_field_data[i] = argv[arg_index];

	cli_command->new_field_data[i] = NULL;
}
#else
static inline void parse_new_data(int argc, char *argv[], int arg_index,
				struct cli_command *cli_command) {}
#endif

/*
 * This method returns an "uninitialized" command; that is- a command
 * initialized with the appropriate "uninitialized" values.
 */
struct cli_command set_command(void)
{
	struct cli_command command;

	command.new_field_data = NULL;
	command.new_byte_data = NULL;
	command.dev_file = NULL;
	command.i2c_addr = -1;
	command.mode = MODE_INVALID;
	command.action = ACTION_INVALID;

	return command;
}

/*
 * This function operates in stages of user input, whose general format can
 * be seen in usage_exit.
 */
struct cli_command parse(int argc, char *argv[])
{
	int cli_arg = 1;
	char *tok;
	struct cli_command cli_cmd = set_command();

	if (argc <= 1)
		usage_exit();

	parse_function(argv, cli_arg, &cli_cmd);
	if (cli_cmd.action == LIST)
		return cli_cmd;

	cli_arg++;
	/* Reads and writes require additional parameters. */
	if (cli_arg == argc)
		usage_exit();

	parse_mode(argv, cli_arg, &cli_cmd);
	NEXT_OR_STOP(cli_arg);
	/* Next argument might be --addr= */
	if (cli_cmd.mode == I2C_MODE && !strncmp(argv[cli_arg], "--addr=", 7)) {
		tok = extract_value(argv, cli_arg);
		cli_cmd.i2c_addr = strtol(tok, 0, 0);
		NEXT_OR_STOP(cli_arg);
	}

	/* Next argument might be file path */
	if (parse_path(argv, &cli_arg, &cli_cmd))
		NEXT_OR_STOP(cli_arg);

	parse_new_data(argc, argv, cli_arg, &cli_cmd);

	return cli_cmd;
}
