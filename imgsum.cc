#include <stack>
#include <iostream>
#include <pthread.h>
#include <getopt.h>
#include <stdlib.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gtk/gtk.h>


#define DEFAULT_NUM_THREADS 4
#define DEFAULT_MEMORY_CONSTRAINT 0

struct options_t {
	int memory_constraint;
	int num_threads;
};

options_t options ={
	DEFAULT_MEMORY_CONSTRAINT,
	DEFAULT_NUM_THREADS
};

static inline void print_usage(std::ostream& f)
{


	return;
}

static inline bool getoptions(int argc, char* argv[])
{
	int next_option;
	const char* const short_options = "hi:o:";

	const struct option long_options[] = {
		{"help",    0, NULL, 'h' },
		{"input",   1, NULL, 'i' },
		{"output",  1, NULL, 'o' },
		{ NULL,     0, NULL,  0  }
	};

	do {
		next_option = getopt_long(argc, argv, short_options, long_options, NULL);
		switch(next_option){
			case 'h':
				print_usage(std::cout);
				return false;
				break;
			case 'o':
				break;
			case '?':
				print_usage(std::cerr);
				return false;
				break;
			case -1:
				break;

			default:
				exit(-1);
		}
	} while(next_option != -1);

	return true;
}

//std::stack<int> img_stack;


int main(int argc, char* argv[])
{
	gdk_init(&argc, &argv);
	if(!getoptions(argc, argv))
		return 0;
	// iterate through images

	for(; optind < argc; optind++){

		GdkPixbuf* buf = NULL;
		buf = gdk_pixbuf_new_from_file (argv[optind], NULL);
		
		if(!buf){
			std::cerr << "Error loading "<< argv[optind] << "\n";

		}
		//std::cout << "Image info: \n";
		//std::cout << "Width: " << gdk_pixbuf_get_width(buf) << "\n";
		//std::cout << "Height: " << gdk_pixbuf_get_height(buf) << "\n\n";

		g_object_unref (G_OBJECT (buf));

	}


	return 0;

}
