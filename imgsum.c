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

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gtk/gtk.h>

#include <dirent.h>
#include "parse_options.h"

struct options_t options ={
	.output_dir   = NULL,
	.input_dir    = NULL,
	.format       = "jpg",

	.num_threads  = 1,
	.affinity     = 0,

	.verbose      = 0
};

static inline int isdir(char* path)
{
	DIR* dir = opendir(path);
	if(!dir)
		return 0;
	closedir(dir);
	return 1;
}



int main(int argc, char* argv[])
{
	int ret;
	gdk_init(&argc, &argv);



	if((ret = getoptions(argc, argv, &options)) <=0)
		return ret;
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
