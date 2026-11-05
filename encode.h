#ifndef ENCODE_H
#define ENCODE_H
#include "types.h"
/* Encode a text file; CLI form: -e <cover.bmp> <secret.txt> [stego.bmp]. */
Status encode_file(const char *bmp_path, const char *secret_path, const char *output_path);
#endif
