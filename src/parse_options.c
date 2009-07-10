/*
 * parse_options.c
 * This file is part of imgmean
 *
 * Copyright (C) 2009 - Lucas De Marchi
 *
 * imgmean is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 * imgmean is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with imgmean; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */
#include "parse_options.h"
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <dirent.h>
#include <FreeImage.h>

static inline int is_dir(char* path)
{
	DIR* dir = opendir(path);
	if(!dir)
		return 0;
	closedir(dir);
	return 1;
}

static inline void print_usage(FILE* f)
{
	fprintf(f, "imgmean\n"
	"Author: Lucas De Marchi @ www.politreco.com\n\n"
	"USAGE:\n"
	"\timgsum <options> input_folder output_folder\n\n"
	"OPTIONS:\n"
	"\t-w,--window NUM            Window size, i.e., how many images are\n"
	"\t                           are used to make one image. Default: 2\n"
	"\t-h,--help                  Print this usage message\n"
	"\t-f,--format FORMAT         Format to load pictures.They will be saved\n"
	"\t                           with the same format.\n"
	"\t                           Available options: jpg, png. Default: jpg\n"
	"\t-a,--affinity              Each created thread runs in a separated\n"
	"\t                           processor (if possible)\n"
	"\t-j,--threads [NUM]         Create NUM threads.\n"
	"\t                           Without NUM, threads = number_of_cpus\n"
	"\t                           Without this option, it uses only 1 thread\n"
	"\t-v,--verbose               Print info messages while working\n"
	"\n");
	return;
}


/* Returns the index of next non option element
 * 0 if no error
 * 1 if it's a "help" execution
 * >1 on error
 */
int getoptions(int argc, char* argv[], struct options_t *options)
{
	int next_option;
	const char* const short_options = "w:hf:aj::v";

	static const struct option long_options[] = {
		{"window",    required_argument, NULL, 'w' },
		{"help",      no_argument,       NULL, 'h' },
		{"format",    required_argument, NULL, 'f' },
		{"affinity",  no_argument,       NULL, 'a' },
		{"threads",   optional_argument, NULL, 'j' },
		{"verbose",   no_argument,       NULL, 'v' },
		{ NULL, 0, NULL,0 }
	};

	do {
		next_option = getopt_long(argc, argv, short_options, long_options, NULL);
		switch(next_option){
			case -1:
				break;
			case 'h':
				print_usage(stdout);
				return 1;
				break;
			case 'f':
				options->format = optarg;
				if(!strcmp(options->format, "jpg")){
					options->format_mask = FIF_JPEG;
				}
				else if(!strcmp(options->format, "png")){
					options->format_mask = FIF_PNG;
				}
				else{
					fprintf(stderr, "ERROR: file format not recognized. See USAGE");
					print_usage(stderr);
				}
				break;
			case 'a':
				options->affinity = 1;
				break;
			case 'j':
				if(optarg != NULL)
					options->num_threads = atoi(optarg);
				else
					options->num_threads = sysconf(_SC_NPROCESSORS_ONLN);
				break;
			case 'v':
				options->verbose = 1;
				break;
			case 'w':
				options->window = atoi(optarg);
				if(options->window < 2){
					fprintf(stderr, "Error setting window size\n");
					return 2;
				}
				break;
			default:
				print_usage(stderr);
				return 3;
				break;
		}
	} while(next_option != -1);

	if(optind != argc-2) {
		fprintf(stderr, "ERROR: you must provide input and output directories\n");
		print_usage(stderr);
		return 2;
	}

	options->input_dir = argv[optind++];
	//test input directory
	if(!is_dir(options->input_dir)){
		fprintf(stderr, "Couldn't access input directory\n");
		return 1;
	}
	
	options->output_dir = argv[optind++];
	//test output directory
	if(!is_dir(options->output_dir)){
		fprintf(stderr, "Couldn't access output directory\n");
		return 1;
	}
	
	return 0;	
}
void print_options(struct options_t *options)
{
	printf("Using folowing options: \n");
	printf("\tInput dir: %s\n", options->input_dir);
	printf("\tOutput dir: %s\n", options->output_dir);
	printf("\tFormat to save: %s\n", options->format);
	printf("\tNumber of threads: %d\n", options->num_threads);
	if(options->num_threads > 1)
		printf("\tSet affinity: %s\n", options->affinity ? "true":"false");
}
