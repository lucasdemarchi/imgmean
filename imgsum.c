/*
 * imgsum.c
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
#include <errno.h>
#include <dirent.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gtk/gtk.h>
#include <string.h>
#include "parse_options.h"

#ifdef DEBUG
#define LOG(a, b)          \
	if(options.verbose) \
		printf(a, b)
#else
#define LOG(a,b) do { } while(0)
#endif

struct options_t options ={
	.window       = 2,
	.output_dir   = NULL,
	.input_dir    = NULL,
	.format       = "jpg",

	.num_threads  = 1,
	.affinity     = 0,

	.verbose      = 0
};

static inline int is_dir(char* path)
{
	DIR* dir = opendir(path);
	if(!dir)
		return 0;
	closedir(dir);
	return 1;
}

/* Used by scandir function to select only image files */
static int filter_images(const struct dirent *d)
{
	size_t len = strlen(d->d_name);
	size_t ext_len = strlen(options.format);
	char* ext;

	if(ext_len >= len)
		return 0;

	ext = (char*)(d->d_name + len - ext_len);
	if(!strcmp(ext, options.format))
		return 1;
	
	return 0;
}



int main(int argc, char* argv[])
{
	int ret;
	struct dirent **namelist;
	int n_files;
	unsigned int n_threads;

	pthread_t* pool_threads;

//init graphic library
	gdk_init(&argc, &argv);

//read options from command line
	if((ret = getoptions(argc, argv, &options)) > 0)
		return ret-1;

//read list of filenames
	n_files = scandir(".", &namelist, filter_images, alphasort);
	if(n_files < 0){
		perror("Couldn't open directory to scan files\n");
		return 1;
	}
	LOG("Images: %d \n", n_files);

//test output directory
	if(!is_dir(options.output_dir)){
		fprintf(stderr, "Couldn't access output directory\n");
		return 1;
	}

//	n_threads = get_n_threads(n_files);
//	pool_threads = malloc( sizeof(pthread_t) * n_threads);
	for(int i = 0; i < n_files; i++){
		printf("%s\n", namelist[i]->d_name);
		free(namelist[i]);
	}

	free(namelist);

	return 0;
	// iterate through images
	// for_each_file_in_input_dir{
		GdkPixbuf* buf = NULL;
		buf = gdk_pixbuf_new_from_file (argv[0], NULL);
		
		if(!buf){
			fprintf(stderr,"Error loading %s\n", argv[0]);

		}
		//std::cout << "Image info: \n";
		//std::cout << "Width: " << gdk_pixbuf_get_width(buf) << "\n";
		//std::cout << "Height: " << gdk_pixbuf_get_height(buf) << "\n\n";

		g_object_unref (G_OBJECT (buf));

//	}


	return 0;
}
