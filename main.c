#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "decode.h"
#include "encode.h"
void print_usage(const char *program)
{
    printf("Hide a text file inside a 24-bit BMP image, or recover it.\n\n"
           "COMMANDS:\n"
           "  Encode: %s -e <cover image.bmp> <text file.txt> [output image.bmp]\n"
           "  Decode: %s -d <image with hidden text.bmp> [recovered text.txt]\n"
           "  Help:   %s -h\n\n"
           "EXAMPLES:\n"
           "  %s -e beautiful.bmp secret.txt\n"
           "      Hides secret.txt in beautiful.bmp and creates stego.bmp.\n"
           "  %s -e beautiful.bmp secret.txt hidden.bmp\n"
           "      Hides secret.txt in beautiful.bmp and creates hidden.bmp.\n"
           "  %s -d hidden.bmp\n"
           "      Recovers the text from hidden.bmp and creates decoded.txt.\n"
           "  %s -d hidden.bmp recovered.txt\n"
           "      Recovers the text from hidden.bmp and creates recovered.txt.\n\n"
           "Use -e to encode and -d to decode. File names in [brackets] are optional.\n"
           "The cover image must be an uncompressed 24-bit BMP. The hidden file must be .txt.\n",
           program, program, program, program, program, program, program);
}

int main(int argc, char *argv[])
{
    if (argc == 2 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0))
    {
        print_usage(argv[0]);
        return EXIT_SUCCESS;
    }
    if (argc >= 2 && strcmp(argv[1], "-e") == 0)
    {
        const char *output_path;
        if (argc < 4 || argc > 5)
        {
            print_usage(argv[0]);
            return EXIT_FAILURE;
        }
        output_path = (argc == 5) ? argv[4] : "stego.bmp";
        return encode_file(argv[2], argv[3], output_path) == e_success ? EXIT_SUCCESS : EXIT_FAILURE;
    }
    if (argc >= 2 && strcmp(argv[1], "-d") == 0)
    {
        const char *output_path;
        if (argc < 3 || argc > 4)
        {
            print_usage(argv[0]);
            return EXIT_FAILURE;
        }
        output_path = (argc == 4) ? argv[3] : "decoded.txt";
        return decode_file(argv[2], output_path) == e_success ? EXIT_SUCCESS : EXIT_FAILURE;
    }
    print_usage(argv[0]);
    return EXIT_FAILURE;
}
