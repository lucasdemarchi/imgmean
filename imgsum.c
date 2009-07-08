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


unsigned int get_n_threads(unsigned int n_frames)
{
	unsigned int n_windows = n_frames / (options.window *2);
	return n_windows < options.num_threads ? n_windows : options.num_threads;
}

static struct dirent **namelist;


struct thread_params{
	pthread_t thr;
	unsigned int start_frame;
	unsigned int end_frame;
};


void *worker_thread(void *param)
{
	struct thread_params* my_data = (struct thread_params*) param;

	printf("start_frame: %u",my_data->start_frame); 
	pthread_exit(NULL);
}

static inline int read_filenames()
{
	int n_files = scandir(".", &namelist, filter_images, alphasort);
	if(n_files < 0){
		perror("Couldn't open directory to scan files\n");
		return 1;
	}
	LOG("Images: %d \n", n_files);

	return n_files;
}

int main(int argc, char* argv[])
{
	int ret;

	int n_files;
	unsigned int n_threads;

	struct thread_params* pool_threads;
	pthread_attr_t attr;


	//init graphic library
	gdk_init(&argc, &argv);

	//read options from command line
	if((ret = getoptions(argc, argv, &options)) > 0)
		return ret-1;

	
	n_files = read_filenames();
	n_threads = get_n_threads(n_files);


	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);

	pool_threads = malloc( sizeof(struct thread_params) * n_threads);
	for(int i = 0; i < n_threads; i++){
		pthread_create(&pool_threads[i].thr, NULL, worker_thread, (void*) &pool_threads[i] );
	}

	pthread_exit(NULL);
	

#if 0
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
#endif
}
