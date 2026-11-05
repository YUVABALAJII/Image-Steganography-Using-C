#ifndef DECODE_H
#define DECODE_H
#include "types.h"
/* Decode hidden text; CLI form: -d <stego.bmp> [recovered.txt]. */
Status decode_file(const char *bmp_path, const char *output_path);
#endif
