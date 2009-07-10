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
#define _GNU_SOURCE
#include <errno.h>
#include <dirent.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <FreeImage.h>
#include <string.h>
#include "parse_options.h"

#define LOG             \
	if(options.verbose) \
		printf

#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)     __builtin_expect((x),0)

#define FILENAME_IN_MAXLEN 15
#define FILENAME_OUT_NAME_FORMAT "%05u"

struct thread_params{
	pthread_t thr;
	unsigned int start_frame;
	unsigned int end_frame;
	int affinity;
};

struct images_params_t {
	unsigned int height;
	unsigned int width;
	unsigned int channels;
};



static struct options_t options ={
	.window       = 2,
	.output_dir   = NULL,
	.input_dir    = NULL,
	.format       = "jpg",
	.format_mask  = FIF_JPEG,

	.num_threads  = 1,
	.affinity     = 0,

	.verbose      = 0

};

static struct images_params_t image_params;
static struct dirent **namelist;



/* Used by scandir function to select only image files 
 * */
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

/* Returns the maximum possible number of threads
 * based on window size and number of frames 
 * */
unsigned int get_n_threads(unsigned int n_frames)
{
	unsigned int max_threads = n_frames / options.window;
	
	if (options.num_threads > max_threads){
		options.num_threads = max_threads;
		printf("Maximum number of threads reached: %u", max_threads);
	}

	return options.num_threads;
}


#if !defined(STRICT_C)
/* We know it works in Linux, Windows and MacOS
 * Use it as it is a faster operation 
 * */
static inline void zerov(float* v, unsigned int size)
{
	memset((void*) v, 0, size * sizeof(float));
}
#else 
/* otherwise, use this other one 
 * */
static inline void zerov(float* v)
{
	for(int i =0; i < size; i++)
		v[i] = 0.0;
}
#endif

/* @result: where the result will be stored. Must be already allocated
 * @image:  the pixbuf image. Each pixel is converted to float, divided by
 *          window size and stored in corresponding result pixel 
 * */
static inline void sum_image(float* result, FIBITMAP* image)
{
	FREE_IMAGE_TYPE image_type = FreeImage_GetImageType(image);
	unsigned pitch = FreeImage_GetPitch(image);

	if(likely((image_type == FIT_BITMAP) && (FreeImage_GetBPP(image) == 24))) {
		BYTE *bits = (BYTE*)FreeImage_GetBits(image);
		for(int j = 0; j < image_params.height; j++) {
			BYTE *pixel = (BYTE*)bits;
			for(int i = 0; i < image_params.width; i++) {
				*result += pixel[FI_RGBA_RED];  ++result;
				*result += pixel[FI_RGBA_GREEN];++result;
				*result += pixel[FI_RGBA_BLUE]; ++result;
				pixel += 3;
			}
			// next line
			bits += pitch;
		}
	}
	else if(image_type == FIT_RGBF) {
		BYTE *bits = (BYTE*)FreeImage_GetBits(image);
		for(int j = 0; j < image_params.height; j++) {
			FIRGBF *pixel = (FIRGBF*)bits;
			for(int i = 0; i < image_params.width; i++) {
				*result += pixel[i].red;   ++result;
				*result += pixel[i].green; ++result;
				*result += pixel[i].blue;  ++result;
			}
			// next line
			bits += pitch;
		}
	}
	else {
		fprintf(stderr, "Unknown file format:\n"
				"image_type: %d\n"
				"BPP: %d", image_type, FreeImage_GetBPP(image));
		exit(1);
	}
}

/* @buffer:   the image in float format.
 * @filename: the filename where of the file. The path passed on command line
 *            will be prepended to this name
 * */
static inline void save_image(float* buffer, char* filename)
{
	FIBITMAP* image = FreeImage_Allocate(image_params.width,
			image_params.height, image_params.channels * 8, 0, 0, 0);
		
	BYTE *bits = (BYTE*)FreeImage_GetBits(image);
	unsigned pitch = FreeImage_GetPitch(image);

	for(int j = 0; j < image_params.height; j++) {
		BYTE *pixel = (BYTE*)bits;
		for(int i = 0; i < image_params.width; i++) {
			pixel[FI_RGBA_RED] = (BYTE) (*buffer / options.window);
			buffer++;
			pixel[FI_RGBA_GREEN] = (BYTE) (*buffer / options.window);
			buffer++;
			pixel[FI_RGBA_BLUE] = (BYTE) (*buffer / options.window);
			buffer++;
			pixel += 3;
		}
		// next line
		bits += pitch;
	}

	int len = strlen(options.output_dir) + strlen(filename) +2;
	char* complete_path = malloc(sizeof(char) * len);
	sprintf(complete_path, "%s/%s",options.output_dir, filename);

	int r;
	if(!strcmp(options.format,"jpg"))
		r = FreeImage_Save(options.format_mask, image,
				complete_path,JPEG_QUALITYSUPERB);
	else //PNG
		r = FreeImage_Save(options.format_mask, image, 
				complete_path,PNG_DEFAULT);
	if(!r)
		LOG("Not possible to save image %s",complete_path );

	free(complete_path);
	FreeImage_Unload(image);


}

static inline void set_affinity(int cpu)
{
	cpu_set_t cpuset;
	
	CPU_ZERO(&cpuset);
	CPU_SET(cpu, &cpuset);
	int r;
	if((r = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t),
				&cpuset)) != 0){
		errno=r;
		perror("pthread_setaffinity_np");
		exit(r);
	}

}

/* The worker thread. Each thread will execute this function and each of them
 * have a struct thread_params as parameter (allocated in main function) 
 * */
void *worker_thread(void *param)
{
	struct thread_params* my_data = (struct thread_params*) param;
	float* res;
	unsigned int size;
	char filename_out[20];
	char* filename_in;


	if(my_data->affinity >=0)
		set_affinity(my_data->affinity);
	
	size = image_params.width * image_params.height * image_params.channels;

	res = malloc(sizeof(float) * size);
	
	filename_in = malloc (sizeof(char) * (strlen(options.input_dir)
				+ FILENAME_IN_MAXLEN));

	for(; my_data->start_frame <= my_data->end_frame;
	                   my_data->start_frame++){
		
		sprintf(filename_out, FILENAME_OUT_NAME_FORMAT, my_data->start_frame);
		strcat(filename_out, options.format);

		zerov(res, size);
	
		for(int i = my_data->start_frame;
				i < my_data->start_frame + options.window; i++) {

			sprintf(filename_in, "%s/%s", options.input_dir,
					namelist[i]->d_name);
			if(options.verbose)
				printf("Opening %s", filename_in);

			FIBITMAP *image_i = FreeImage_Load(options.format_mask, filename_in, 0);
			if(unlikely(!image_i))
				goto error;

			sum_image(res, image_i);

			FreeImage_Unload(image_i);
		}
		save_image(res, filename_out);
	}
	free(filename_in);
	free(res);	
	pthread_exit(NULL);

error:
	fprintf(stderr, "[LOADING FILE] Error.\n"
			"All threads will die now");
	exit(1);

}

/* Helper function to open input_dir and get all image files.
 * It saves in the global namelist variable and returns the number of
 * filenames present in this var. 
 * */
static inline int read_filenames()
{
	int n_files = scandir(options.input_dir, &namelist, filter_images, alphasort);
	if(n_files < 0){
		perror("Couldn't open directory to scan files\n");
		return 1;
	}
	LOG("Images: %d \n", n_files);

	//read the first image to get parameters
	//
	//
	//
	char* filename_in = malloc (sizeof(char) * (strlen(options.input_dir) +15));
	
	sprintf(filename_in, "%s/%s", options.input_dir,
					namelist[0]->d_name);


	FIBITMAP *image = FreeImage_Load(options.format_mask, filename_in, 0);
	image_params.width     = FreeImage_GetWidth(image);
	image_params.height    = FreeImage_GetHeight(image);
	image_params.channels  = FreeImage_GetBPP(image) / 8; //each channel has 8 bits

	FreeImage_Unload(image);
	free(filename_in);

	return n_files;
}

int main(int argc, char* argv[])
{
	int ret;

	int n_files, n_files_old;

	struct thread_params* pool_threads;
	pthread_attr_t attr;
	
	FreeImage_Initialise(0);

	//read options from command line
	if((ret = getoptions(argc, argv, &options)) > 0)
		return ret-1;

	
	n_files = read_filenames();
	n_files_old = n_files;

	if(n_files < options.window){
		printf("%u files detected. Number of files has to be at least the same"
			   " dimension of window",n_files);
		exit(1);
	}

//	if(n_files % options.window != 0 && options.verbose){
//		printf("Number of files (%u) is not a multiple of window size.\n",n_files);
//		printf("Ignoring the last files");
//	}
//	n_files -= n_files % options.window;

	//update options.num_threads to min(options.num_thread, MAX_threads)
	get_n_threads(n_files);


	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);

	pool_threads = malloc( sizeof(struct thread_params) * options.num_threads);

	int n_files_out = n_files - options.window + 1;
	int size_threaded =(int)(n_files_out / options.num_threads);
	for(int i = 0; i < options.num_threads; i++){
		pool_threads[i].start_frame = i * size_threaded;

		if(unlikely(i == options.num_threads-1))
			pool_threads[i].end_frame = n_files_out -1; // the last thread can
		                                                // have some more work
		else
			pool_threads[i].end_frame = (i+1)* size_threaded - 1;

		if(options.affinity && 
				(options.num_threads <= sysconf(_SC_NPROCESSORS_ONLN)))
			pool_threads[i].affinity = i;
		else
			pool_threads[i].affinity = -1;
 
		pthread_create(&pool_threads[i].thr, NULL, worker_thread,
				(void*) &pool_threads[i] );

	}

	for(int i = 0; i < options.num_threads; i++){
		pthread_join(pool_threads[i].thr, NULL);
	}

	//finish free things
	
	free(pool_threads);
	for(int i = 0; i < n_files; i++)
	   free(namelist[i]);

	free(namelist);


	FreeImage_DeInitialise();
	pthread_exit(NULL);

}
