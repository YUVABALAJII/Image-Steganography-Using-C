#include <stdio.h>
#include <string.h>
#include "common.h"
#include "encode.h"
/* Write one hidden byte into the low bits of eight image bytes. */
int hide_byte(FILE *image, FILE *output, unsigned char value)
{
    int bit;

    for (bit = 7; bit >= 0; bit--)
    {
        int image_byte = fgetc(image);
        if (image_byte == EOF || fputc((image_byte & 0xFE) | ((value >> bit) & 1), output) == EOF)
        {
            return 0;
        }
    }
    return 1;
}
/* Write a number as four hidden bytes, most significant byte first. */
int hide_number(FILE *image, FILE *output, unsigned long number)
{
    int shift;

    for (shift = 24; shift >= 0; shift -= 8)
    {
        if (!hide_byte(image, output, (unsigned char)(number >> shift)))
        {
            return 0;
        }
    }
    return 1;
}
/* Encode a text file into a BMP; CLI: -e <cover.bmp> <secret.txt> [stego.bmp]. */
Status encode_file(const char *bmp_path, const char *secret_path, const char *output_path)
{
    FILE *image = NULL;
    FILE *secret = NULL;
    FILE *output = NULL;
    long pixel_start;
    long pixel_size;
    long secret_size;
    long max_secret_size;
    long index;
    int value;
    Status result = e_failure;
    if (strcmp(bmp_path, output_path) == 0 || strcmp(secret_path, output_path) == 0)
    {
        fprintf(stderr, "ERROR: INPUT AND OUTPUT FILE NAMES MUST BE DIFFERENT.\n");
        return e_failure;
    }
    printf("OPENING INPUT FILES...\n");
    image = fopen(bmp_path, "rb");
    secret = fopen(secret_path, "rb");
    if (image == NULL || secret == NULL)
    {
        fprintf(stderr, "ERROR: COULD NOT OPEN AN INPUT FILE.\n");
        goto cleanup;
    }
    if (!get_bmp_info(image, &pixel_start, &pixel_size))
    {
        fprintf(stderr, "ERROR: USE AN UNCOMPRESSED 24-BIT BMP IMAGE.\n");
        goto cleanup;
    }
    printf("BMP IMAGE CHECKED SUCCESSFULLY.\n");
    if (fseek(secret, 0, SEEK_END) != 0 || (secret_size = ftell(secret)) < 0 || fseek(secret, 0, SEEK_SET) != 0)
    {
        fprintf(stderr, "ERROR: COULD NOT READ THE SECRET FILE SIZE.\n");
        goto cleanup;
    }
    printf("SECRET FILE SIZE: %ld BYTES.\n", secret_size);
    max_secret_size = pixel_size / 8 - (long)strlen(MAGIC_STRING) - 4 - (long)strlen(SECRET_EXTENSION) - 4;
    if (max_secret_size < 0 || secret_size > max_secret_size)
    {
        fprintf(stderr, "ERROR: IMAGE IS TOO SMALL FOR THIS TEXT FILE.\n");
        goto cleanup;
    }
    printf("IMAGE CAPACITY CHECKED SUCCESSFULLY.\n");
    output = fopen(output_path, "wb");
    if (output == NULL)
    {
        fprintf(stderr, "ERROR: COULD NOT CREATE THE OUTPUT IMAGE.\n");
        goto cleanup;
    }
    rewind(image);
    for (index = 0; index < pixel_start; index++)
    {
        value = fgetc(image);
        if (value == EOF || fputc(value, output) == EOF)
        {
            fprintf(stderr, "ERROR: COULD NOT COPY THE BMP HEADER.\n");
            goto cleanup;
        }
    }
    printf("BMP HEADER COPIED SUCCESSFULLY.\n");
    for (index = 0; MAGIC_STRING[index] != '\0'; index++)
    {
        if (!hide_byte(image, output, (unsigned char)MAGIC_STRING[index]))
        {
            fprintf(stderr, "ERROR: COULD NOT WRITE THE MAGIC STRING.\n");
            goto cleanup;
        }
    }
    printf("MAGIC STRING ENCODED: %s\n", MAGIC_STRING);
    if (!hide_number(image, output, strlen(SECRET_EXTENSION)))
    {
        fprintf(stderr, "ERROR: COULD NOT WRITE THE EXTENSION SIZE.\n");
        goto cleanup;
    }
    printf("SECRET FILE EXTENSION SIZE ENCODED SUCCESSFULLY.\n");
    for (index = 0; SECRET_EXTENSION[index] != '\0'; index++)
    {
        if (!hide_byte(image, output, (unsigned char)SECRET_EXTENSION[index]))
        {
            fprintf(stderr, "ERROR: COULD NOT WRITE THE SECRET FILE EXTENSION.\n");
            goto cleanup;
        }
    }
    printf("SECRET FILE EXTENSION ENCODED: %s\n", SECRET_EXTENSION);
    if (!hide_number(image, output, (unsigned long)secret_size))
    {
        fprintf(stderr, "ERROR: COULD NOT WRITE THE SECRET FILE SIZE.\n");
        goto cleanup;
    }
    printf("SECRET FILE SIZE ENCODED SUCCESSFULLY.\n");
    for (index = 0; index < secret_size; index++)
    {
        value = fgetc(secret);
        if (value == EOF || !hide_byte(image, output, (unsigned char)value))
        {
            fprintf(stderr, "ERROR: COULD NOT WRITE THE SECRET FILE DATA.\n");
            goto cleanup;
        }
    }
    printf("SECRET FILE DATA ENCODED SUCCESSFULLY.\n");
    while ((value = fgetc(image)) != EOF)
    {
        if (fputc(value, output) == EOF)
        {
            fprintf(stderr, "ERROR: COULD NOT FINISH WRITING THE OUTPUT IMAGE.\n");
            goto cleanup;
        }
    }
    printf("REMAINING BMP DATA COPIED SUCCESSFULLY.\n");
    if (ferror(image))
    {
        fprintf(stderr, "ERROR: COULD NOT READ THE COMPLETE BMP IMAGE.\n");
        goto cleanup;
    }
    if (fclose(output) != 0)
    {
        output = NULL;
        fprintf(stderr, "ERROR: COULD NOT FINISH WRITING THE OUTPUT IMAGE.\n");
        goto cleanup;
    }
    output = NULL;
    printf("ENCODING COMPLETED: %s\n", output_path);
    result = e_success;
cleanup:
    if (output != NULL)
    {
        fclose(output);
    }
    if (secret != NULL)
    {
        fclose(secret);
    }
    if (image != NULL)
    {
        fclose(image);
    }
    return result;
}
