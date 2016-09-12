#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define TRUE 1
#define FALSE 0
#define MAGIC_NUMBER_BUFFER_SIZE 3
#define IMAGE_READ_BUFFER_SIZE 1024

typedef struct RGBpixel {
	uint8_t r, g, b;
} RGBpixel;

typedef struct Image {
	uint32_t width, height;
	RGBpixel* pixmap;
} Image;

void show_help() {
	printf("Usage: ppmrw 3|6 input.ppm output.ppm\n");
}

int image_load_p3(FILE* fp, Image* image_ptr) {
	// Read until we get past the comments
	fseek(fp, MAGIC_NUMBER_BUFFER_SIZE, SEEK_SET);
	size_t bytes_read;
	int i;
	int j;
	int k;
	int c;
	int width;
	int height;
	float color_max;
	char buffer[IMAGE_READ_BUFFER_SIZE];
	char* tokens;

	// Read past the comments
	c = fgetc(fp);
	if (c == EOF)
		return 1;
	while (c == '#') {
		while (c != '\n') {
			c = fgetc(fp);
			if (c == EOF)
				return 1;
		}
		c = fgetc(fp);
		if (c == EOF)
			return 1;
	};
	// We read one to far, move back one
	fseek(fp, -1, SEEK_CUR);

	// We are at the first line of image header information
	// Read in the dimensions of the image
	fgets(buffer, IMAGE_READ_BUFFER_SIZE, fp);

	tokens = strtok(buffer, " ");
	if (tokens == NULL)
		return 1;
	width = atoi(tokens);

	tokens = strtok(NULL, " ");
	if (tokens == NULL)
		return 1;
	height = atoi(tokens);

	// Read in the max color value
	fgets(buffer, IMAGE_READ_BUFFER_SIZE, fp);
	color_max = atoi(buffer);

	image_ptr->pixmap = malloc(sizeof(RGBpixel) * width * height);
	image_ptr->width = width;
	image_ptr->height = height;

	uint8_t value;
	float scale;

	// Read the actual image in
	for (i=0; i<height; i++) {
		for (j=0; j<width; j++) {
			for (k=0; k<3; k++) {
				fgets(buffer, IMAGE_READ_BUFFER_SIZE, fp);
				if (strlen(buffer) <= 0 || atoi(buffer) > color_max)
					return 1;
				value = (atoi(buffer)/color_max)*255;
				
				if (k == 0)
					image_ptr->pixmap[i*width + j].r = value;
				if (k == 1)
					image_ptr->pixmap[i*width + j].g = value;
				if (k == 2)
					image_ptr->pixmap[i*width + j].b = value;
			}
		}
	}

	return 0;
}

int load_image(Image* image_ptr, char* fname) {
	FILE* fp = fopen(fname, "r");
	if (fp) {
		int result = 1;
		char f_buffer[MAGIC_NUMBER_BUFFER_SIZE];
		// Check for the magic number
		size_t bytes_read = fread(f_buffer, sizeof(char), MAGIC_NUMBER_BUFFER_SIZE, fp);
		if (bytes_read != 3) {
			fprintf(stderr, "Error: The source file is not a valid PPM3 or PPM6 file\n");
			result = 1;
		}
		else {
			if (strncmp("P3\n", f_buffer, MAGIC_NUMBER_BUFFER_SIZE) == 0) {
				result = image_load_p3(fp, image_ptr);
			}
			else if (strncmp("P6\n", f_buffer, MAGIC_NUMBER_BUFFER_SIZE) == 0) {
				//result = load_p6_image(fp, image_ptr);
			}
			else {
				result = 1;
			}
		}

		if (result != 0)
			fprintf(stderr, "Error: The source file is not a valid PPM3 or PPM6 file\n");

		fclose(fp);
		return result;
	}
	else {
		fprintf(stderr, "Error: Could not open source file '%s'\n", fname);
		return 1;
	}
}

int main (int argc, char *argv[])
{
	if (argc != 4) {
		show_help();
		return 0;
	}

	uint8_t ppm_version = 0;
	char* ppm_version_str = argv[1];
	char* fname_input = argv[2];
	char* fname_output = argv[3];
	Image image;

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

	int result = load_image(&image, fname_input);
	if (result != 0) {
		return result;
	}

	return 0;
}
