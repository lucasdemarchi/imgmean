#include <stack>
#include <iostream>
#include <pthread.h>
#include <getopt.h>
#include <stdlib.h>
#include <CImg.h>

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

using namespace cimg_library;

int main(int argc, char* argv[])
{
	if(!getoptions(argc, argv))
		return 0;
	// iterate through images
	CImg<double> out(800, 600, 1, 3, 0);

	for(int n = argc - optind; optind < argc; optind++){
		try{
			CImg<double> image(argv[optind]);
//			std::cout << "File info: " << image.size() << " pixel values (r + g + b).\n";
//			std::cout << "dimx, dimy, dimz, dimv: " << image.dimx() << " "
//			          << image.dimy() << " " << image.dimz() << " " << image.dimv() << "\n";

//			std::cout << "Memory used: " << ((image.size() * sizeof(unsigned int))
//			                             /(float)(1024*1024)) << " MB.\n";
			image /=n;
			out += image;

		} catch(CImgIOException ex) {
			std::cerr << "File " << argv[optind] << " not found\n";
		}
	}

	out.save("out.jpg");

	return 0;
}
