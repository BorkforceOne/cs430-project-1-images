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

/**
 * Shows a help message to the user
 */
void show_help() {
	printf("Usage: ppmrw 3|6 input.ppm output.ppm\n");
}

/**
 * Load a PPM P3 file into image_ptr
 * @param fp
 * @param image_ptr
 * @return
 */
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
	uint16_t value;

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

/**
 * Save an image in PPM P3 format to the specified file
 * @param image_ptr
 * @param fname
 * @return
 */
int image_save_p3(Image* image_ptr, char* fname) {
	FILE* fp = fopen(fname, "w");
	int i;
	int j;
	if (fp) {
		fprintf(fp, "P3\n");
		fprintf(fp, "%i %i\n", image_ptr->width, image_ptr->height);
		fprintf(fp, "255");
		for (i=0; i<image_ptr->height; i++) {
			for (j=0; j<image_ptr->width; j++) {
				RGBpixel pixel = image_ptr->pixmap[i*image_ptr->width + j];
				fprintf(fp, "\n%i\n%i\n%i", pixel.r, pixel.g, pixel.b);
			}
		}
		fclose(fp);
		return 0;
	}
	else {
		fprintf(stderr, "Error: Could not open destination file for writing '%s'\n", fname);
		return 1;
	}
}

/**
 * Load a PPM P6 file into image_ptr
 * @param fp
 * @param image_ptr
 * @return
 */
int image_load_p6(FILE* fp, Image* image_ptr) {
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
	uint16_t value;

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

	// Read the actual image in
	for (i=0; i<height; i++) {
		for (j=0; j<width; j++) {
			for (k=0; k<3; k++) {
				// If color_max is < 256 the values are 8 bit, otherwise they're 16 bit!
				if (color_max < 256) {
					fread(buffer, sizeof(char), 1, fp);
					value = buffer[0] & 0xFF;
				}
				else {
					fread(buffer, sizeof(char), 2, fp);
					value = ((buffer[0] << 8) & 0xFF00) | (buffer[1] & 0xFF);
				}

				value = (value/color_max)*255;

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

/**
 * Loads an PPM image in P3 or P6 formats into the specified image_ptr
 * @param image_ptr
 * @param fname
 * @return
 */
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
				result = image_load_p6(fp, image_ptr);
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
	int result;

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

	result = load_image(&image, fname_input);
	if (result != 0) {
		return result;
	}

	if (ppm_version == 3) {
		result = image_save_p3(&image, fname_output);
	}
	else {
		//result = image_save_p6(image, fname_output);
	}
	if (result != 0) {
		return result;
	}

	return 0;
}
