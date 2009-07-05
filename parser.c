/*
 * parser.c
 * This file is part of imgsum
 *
 * Copyright (C) 2009 - Lucas De Marchi
 *
 * imgsum is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 * imgsum is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Trissa; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */
#include "parser.h"
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>

#include <dirent.h>

static struct options_t options ={
//	.memory_constraint=DEFAULT_MEMORY_CONSTRAINT,
	.num_threads=DEFAULT_NUM_THREADS,
	.output_dir = NULL,
	.input_dir = NULL
};

static inline void print_usage(FILE* f)
{
	fprintf(f, "imgsum\nAuthor: Lucas De Marchi @ www.politreco.com\nUsage:\n");
	fprintf(f, "\timgsum [-t] -i|--input input_dir -o|--output output_dir\n" );
	fprintf (f,"\t-h,--help\t\tPrint this usage message\n"
	         "\t-i,--input\t\tSet the directory with input images\n"
	         "\t-o,--output\t\tSet output directory\n"
	         "\t-t,--threads\t\tSet number of threads (Defaults to 1)\n");
	fprintf (f, "\n\n");

	return;
}

static inline int isdir(char* path)
{
	DIR* dir = opendir(path);
	if(!dir)
		return 0;
	closedir(dir);
	return 1;
}

/* Returns the index of next non option element
 * 0 if no error and it's a "help" execution
 * < 0 on error
 */
int getoptions(int argc, char* argv[])
{
	int next_option;
	const char* const short_options = "hi:o:t:";

	const struct option long_options[] = {
		{"help",    0, NULL, 'h' },
		{"input",   1, NULL, 'i' },
		{"output",  1, NULL, 'o' },
		{"threads", 1, NULL, 't' },
		{ NULL,     0, NULL,  0  }
	};

	do {
		next_option = getopt_long(argc, argv, short_options, long_options, NULL);
		switch(next_option){
			case 'h':
				print_usage(stdout);
				return 0;
				break;
			case 'i':
				options.input_dir = optarg;
				printf("Input dir: %s\n", optarg);
				break;
			case 'o':
				options.output_dir = optarg;

				break;
			case 't':
				options.num_threads = atoi(optarg);
				break;
			case -1:
				break;
			default:
				print_usage(stderr);
				return -1;
				break;
		}
	} while(next_option != -1);

	if(options.output_dir == NULL || options.input_dir == NULL){
		fprintf(stderr, "You need to pass input_dir and output_dir parameters\n\n");
		print_usage(stderr);
		return -2;
	}
	if(optind > argc){
		fprintf(stderr, "Option not recognized. See Usage.\n\n");
		print_usage(stderr);
		return -3;
	}

	return optind;
}
void print_options()
{
	printf("Using folowing options: \n");
	printf("\tInput dir: %s\n", options.input_dir);
	printf("\tOutput dir: %s\n", options.output_dir);
	printf("\tNumber of threads: %d\n", options.num_threads);
}
