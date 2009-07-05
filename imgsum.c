#include <pthread.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gtk/gtk.h>

#include "parser.h"

int main(int argc, char* argv[])
{
	int ret;
	gdk_init(&argc, &argv);
	if((ret = getoptions(argc, argv)) <=0)
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
