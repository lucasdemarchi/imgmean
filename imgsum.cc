#include <stack>
#include <iostream>
#include <pthread.h>
#include <getopt.h>
#include <stdlib.h>
//#include <Cimg.h>

#define DEFAULT_NUM_THREADS 4
#define DEFAULT_MEMORY_CONSTRAINT 0
struct options_t {
	int memory_constraint;
	int num_threads;
};

//queue<Cimg *> imgqueue;
//std::queue<int> imgqueue;

options_t options ={
	DEFAULT_MEMORY_CONSTRAINT,
	DEFAULT_NUM_THREADS
};

static inline void print_usage(std::ostream& f)
{


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
			case 'i':
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
}

std::stack<int> img_stack;

int main(int argc, char* argv[])
{
	if(!getoptions(argc, argv))
		return 0;


	return 0;
}
