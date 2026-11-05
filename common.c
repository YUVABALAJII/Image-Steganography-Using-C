#include <stdio.h>
#include "common.h"

unsigned long read_bmp_number(const unsigned char bytes[])
{
    return (unsigned long)bytes[0]
        | ((unsigned long)bytes[1] << 8)
        | ((unsigned long)bytes[2] << 16)
        | ((unsigned long)bytes[3] << 24);
}

int get_bmp_info(FILE *image, long *pixel_start, long *pixel_size)
{
    unsigned char header[54];
    unsigned long pixel_offset;
    long file_size;

    if (fseek(image, 0, SEEK_END) != 0)
    {
        return 0;
    }

    file_size = ftell(image);
    if (file_size < 54
        || fseek(image, 0, SEEK_SET) != 0
        || fread(header, 1, sizeof(header), image) != sizeof(header))
    {
        return 0;
    }

    pixel_offset = read_bmp_number(&header[10]);
    if (header[0] != 'B' || header[1] != 'M'
        || read_bmp_number(&header[14]) < 40
        || header[28] != 24 || header[29] != 0
        || read_bmp_number(&header[30]) != 0
        || pixel_offset < 54 || pixel_offset > (unsigned long)file_size)
    {
        return 0;
    }

    *pixel_start = (long)pixel_offset;
    *pixel_size = file_size - *pixel_start;
    return 1;
}
