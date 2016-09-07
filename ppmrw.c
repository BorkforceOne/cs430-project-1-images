#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define TRUE 1
#define FALSE 0
#define BUFFER_SIZE 1024

typedef struct RGBpixel {
    uint8_t r, g, b;
} RGBpixel;

void show_help() {
    printf("Usage: ppmrw 3|6 input.ppm output.ppm\n");
}

int main (int argc, char *argv[])
{
    if (argc != 4) {
        show_help();
        return 0;
    }

    int ppm_version = 0;
    char* ppm_version_str = argv[1];
    char* fname_input = argv[2];
    char* fname_output = argv[3];
    RGBpixel *pixmap1d;

    // Check for a correct version input
    if (strcmp(ppm_version_str, "3") == 0)
        ppm_version = 3;
    else if (strcmp(ppm_version_str, "6") == 0)
        ppm_version = 6;

    if (ppm_version == 0) {
        show_help();
        return 0;
    }

    // Process the input file

	return 0;
}
