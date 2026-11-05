#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>

#define MAGIC_STRING "#*"
#define SECRET_EXTENSION ".txt"

unsigned long read_bmp_number(const unsigned char bytes[]);
int get_bmp_info(FILE *image, long *pixel_start, long *pixel_size);

#endif
