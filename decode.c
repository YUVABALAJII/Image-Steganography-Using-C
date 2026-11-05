#include <stdio.h>
#include <string.h>
#include "common.h"
#include "decode.h"
/* Read one hidden byte from the low bits of eight image bytes. */
int recover_byte(FILE *image, unsigned char *value)
{
    int bit;
    int image_byte;
    unsigned char result = 0;

    for (bit = 0; bit < 8; bit++)
    {
        image_byte = fgetc(image);
        if (image_byte == EOF)
        {
            return 0;
        }
        result = (unsigned char)((result << 1) | (image_byte & 1));
    }

    *value = result;
    return 1;
}

/* Read four hidden bytes as a number, most significant byte first. */
int recover_number(FILE *image, unsigned long *number)
{
    int index;
    unsigned char value;

    *number = 0;
    for (index = 0; index < 4; index++)
    {
        if (!recover_byte(image, &value))
        {
            return 0;
        }
        *number = (*number << 8) | value;
    }

    return 1;
}

/* Recover hidden text from a BMP; CLI: -d <stego.bmp> [recovered.txt]. */
Status decode_file(const char *bmp_path, const char *output_path)
{
    FILE *image = NULL;
    FILE *output = NULL;
    long pixel_start;
    long pixel_size;
    long max_secret_size;
    long secret_size;
    long index;
    Status result = e_failure;
    unsigned char value;
    unsigned long hidden_size;

    if (strcmp(bmp_path, output_path) == 0)
    {
        fprintf(stderr, "ERROR: INPUT AND OUTPUT FILE NAMES MUST BE DIFFERENT.\n");
        return e_failure;
    }
    image = fopen(bmp_path, "rb");
    if (image == NULL)
    {
        fprintf(stderr, "ERROR: COULD NOT OPEN THE BMP IMAGE.\n");
        goto cleanup;
    }
    printf("BMP IMAGE OPENED SUCCESSFULLY.\n");
    if (!get_bmp_info(image, &pixel_start, &pixel_size) ||
        fseek(image, pixel_start, SEEK_SET) != 0)
    {
        fprintf(stderr, "ERROR: USE AN UNCOMPRESSED 24-BIT BMP IMAGE.\n");
        goto cleanup;
    }
    printf("BMP IMAGE CHECKED SUCCESSFULLY.\n");
    max_secret_size = pixel_size / 8 - (long)strlen(MAGIC_STRING) - 4
        - (long)strlen(SECRET_EXTENSION) - 4;
    if (max_secret_size < 0)
    {
        fprintf(stderr, "ERROR: IMAGE IS TOO SMALL TO CONTAIN HIDDEN TEXT.\n");
        goto cleanup;
    }
    for (index = 0; MAGIC_STRING[index] != '\0'; index++)
    {
        if (!recover_byte(image, &value)
            || value != (unsigned char)MAGIC_STRING[index])
        {
            fprintf(stderr, "ERROR: NO HIDDEN TEXT WAS FOUND IN THIS IMAGE.\n");
            goto cleanup;
        }
    }
    printf("MAGIC STRING FOUND: %s\n", MAGIC_STRING);
    if (!recover_number(image, &hidden_size)
        || hidden_size != strlen(SECRET_EXTENSION))
    {
        fprintf(stderr, "ERROR: HIDDEN FILE EXTENSION IS NOT SUPPORTED.\n");
        goto cleanup;
    }
    printf("SECRET FILE EXTENSION SIZE DECODED SUCCESSFULLY.\n");
    for (index = 0; SECRET_EXTENSION[index] != '\0'; index++)
    {
        if (!recover_byte(image, &value)
            || value != (unsigned char)SECRET_EXTENSION[index])
        {
            fprintf(stderr, "ERROR: THIS PROGRAM ONLY RECOVERS .TXT FILES.\n");
            goto cleanup;
        }
    }
    printf("SECRET FILE EXTENSION DECODED: %s\n", SECRET_EXTENSION);
    if (!recover_number(image, &hidden_size)
        || hidden_size > (unsigned long)max_secret_size)
    {
        fprintf(stderr, "ERROR: HIDDEN FILE SIZE IS INVALID.\n");
        goto cleanup;
    }
    secret_size = (long)hidden_size;
    printf("SECRET FILE SIZE DECODED: %ld BYTES.\n", secret_size);
    output = fopen(output_path, "wb");
    if (output == NULL)
    {
        fprintf(stderr, "ERROR: COULD NOT CREATE THE RECOVERED TEXT FILE.\n");
        goto cleanup;
    }
    for (index = 0; index < secret_size; index++)
    {
        if (!recover_byte(image, &value) || fputc(value, output) == EOF)
        {
            fprintf(stderr, "ERROR: COULD NOT RECOVER THE HIDDEN TEXT.\n");
            goto cleanup;
        }
    }
    printf("SECRET FILE DATA DECODED SUCCESSFULLY.\n");
    if (fclose(output) != 0)
    {
        output = NULL;
        fprintf(stderr, "ERROR: COULD NOT FINISH WRITING THE RECOVERED FILE.\n");
        goto cleanup;
    }
    output = NULL;
    printf("DECODING COMPLETED: %s\n", output_path);
    result = e_success;
cleanup:
    if (output != NULL)
    {
        fclose(output);
    }
    if (image != NULL)
    {
        fclose(image);
    }
    return result;
}
