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

#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)     __builtin_expect((x),0)


struct options_t options ={
	.window       = 2,
	.output_dir   = NULL,
	.input_dir    = NULL,
	.format       = "jpg",

	.num_threads  = 1,
	.affinity     = 0,

	.verbose      = 0

};


struct images_params_t {
	unsigned int height;
	unsigned int width;
	unsigned int channels;
	unsigned int rowstride;
};

struct images_params_t image_params;



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


/* Here starts the algorithm to calculate new images
 * off the sequence availabel at namelist[]
 **********************************************************/

#if !defined(STRICT_C)
/* We know it works in Linux, Windows and MacOS
   Use it as it is a faster option
*/
static inline void zerov(float* v, unsigned int size)
{
	memset((void*) v, 0, size * sizeof(float));
}
#else //use this if above doesn't work
static inline void zerov(float* v)
{
	for(int i =0; i < size; i++)
		v[i] = 0.0;
}
#endif

static inline void sum_image(float* result, GdkPixbuf* image)
{
	guchar* buf = gdk_pixbuf_get_pixels(image);

	for (int i = 0; i < image_params.width * image_params.channels; i++){
		for(int j=0; j < image_params.height; j++){
			*result = (*buf) /(float) options.window;
			buf++;
			result++;
		}
		buf += gdk_pixbuf_get_rowstride(image);
	}

}

static inline void save_image(float* buffer, char* filename)
{
	GdkPixbuf* image = gdk_pixbuf_new(GDK_COLORSPACE_RGB,
	        (image_params.channels==4), image_params.channels*8,
	        image_params.width, image_params.height);
		
	guchar* buffer_new = gdk_pixbuf_get_pixels(image);

	for (int i = 0; i < image_params.width * image_params.channels; i++){
		for(int j=0; j < image_params.height; j++){
			*buffer_new = (guchar) *buffer;
			buffer_new++;
			buffer++;
		}
		buffer += gdk_pixbuf_get_rowstride(image);
	}
	GError* err;
	int len = strlen(options.output_dir) + strlen(filename) +2;
	char* complete_path = malloc(sizeof(char) * len);

	sprintf(complete_path, "%s/%s",options.output_dir, filename);

	if(likely(!strcmp(options.format, "jpg")))
		gdk_pixbuf_save(image, complete_path,"jpeg", &err, "quality", 100, NULL);
	else	
		gdk_pixbuf_save(image, complete_path,"jpeg", &err, "quality", 100, NULL);

	free(complete_path);
	g_object_unref(image);

}

void *worker_thread(void *param)
{
	struct thread_params* my_data = (struct thread_params*) param;
	float* res;
	unsigned int size;
	char filename_out[20];
	GError *err;

	size = image_params.width * image_params.height * image_params.channels;

	res = malloc(sizeof(float) * size);

	sprintf(filename_out, "%05d", my_data->start_frame);
	strcat(filename_out, options.format);

	for(; my_data->start_frame <= my_data->end_frame;
	                   my_data->start_frame++){

		zerov(res, size);
	
		for(int i = my_data->start_frame;
				i < my_data->start_frame + options.window; i++) {
			
			GdkPixbuf* image_i = gdk_pixbuf_new_from_file(namelist[i]->d_name,
					&err);
			if(unlikely(image_i == NULL))
				goto error;

			sum_image(res, image_i);	
			g_object_unref(image_i);
		}
		free(namelist[my_data->start_frame]);

		save_image(res, filename_out);
	}

	free(res);	
	pthread_exit(NULL);

error:
	fprintf(stderr, "[LOADING FILE] Error %d: %s.\n"
			"All threads will die now", err->code, err->message );
	exit(1);

}
/*********/


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
		pthread_create(&pool_threads[i].thr, NULL, worker_thread,
				(void*) &pool_threads[i] );

	}

	for(int i = 0; i < n_threads; i++){
		pthread_join(pool_threads[i].thr, NULL);
	}

	//finish free things
	
	free(pool_threads);
	for(int i = n_files - options.window; i < n_files; i++)
	   free(namelist[i]);

	free(namelist);



	pthread_exit(NULL);

}
