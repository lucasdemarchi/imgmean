/*
 * parse_options.h
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
#ifndef PARSER_H
#define PARSER_H

#include <FreeImage.h>

struct options_t {
	unsigned int window;
	char* output_dir;
	char* input_dir;
	char* format;
	enum FREE_IMAGE_FORMAT format_mask;

	unsigned int num_threads;	
	int affinity;

	int verbose;

};

int getoptions(int argc, char* argv[], struct options_t *options);
void print_options();

#endif
