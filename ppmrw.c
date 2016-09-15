#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define TRUE 1
#define FALSE 0
#define IMAGE_READ_BUFFER_SIZE 1024


#define ERR_INVALID_FILE "Error: The source file is not a valid PPM3 or PPM6 file\n"
#define ERR_UNEXPECTED_EOF "Error: Unexpected EOF\n"
#define ERR_OPEN_FILE_WRITING "Error: Could not open destination file for writing '%s'\n"
#define ERR_OPEN_FILE_READING "Error: Could not open source file for reading '%s'\n"

/**
 * RGB Pixel
 */
typedef struct RGBpixel {
	uint8_t r, g, b;
} RGBpixel;

/**
 * Image
 */
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
 * Increments the pointer past past comments in a PPM file
 * @param fp
 * @return
 */
int skip_comments(FILE* fp) {
	int c;
	char in_comment = FALSE;
	while (TRUE) {
		c = getc(fp);
		if (c == EOF) {
			fprintf(stderr, ERR_INVALID_FILE);
			return 1;
		}
		if (in_comment) {
			if (c == '\n' || c == '\r')
				in_comment = FALSE;
		}
		else if (c == '#')
			in_comment = TRUE;
		else {
			// We read one to far, move back one
			fseek(fp, -1, SEEK_CUR);
			return 0;
		}
	}
};

/**
 * Increments the pointer past whitespace AND comments in a PPM file
 * This includes: spaces, TABs, CRs, LFs
 */
int skip_whitespace(FILE* fp) {
	int c;
	do {
		c = getc(fp);
		// make sure we didn't read to the EOF
		if (c == EOF) {
			fprintf(stderr, ERR_UNEXPECTED_EOF);
			return 1;
		}
		if (c == '\n' || c == '\r') {
			// read past any comments
			if (skip_comments(fp) != 0)
				return 1;
		}
	}
	while(c == '\r' || // carriage return
  	      c == '\n' || // newline
	      c == ' '  || // space
	      c == '\t'); // tab
	// We read one to far, move back one
	fseek(fp, -1, SEEK_CUR);
	return 0;
}

/**
 * Reads the contents of the current position in the file to the next whitespace character into the specified buffer
 * This function null terminates the buffer at the end of the read string
 * @param fp
 * @param buffer
 * @param buffer_size
 * @return
 */
int read_to_whitespace(FILE* fp, char buffer[], int buffer_size) {
	int c;
	int pos = 0;
	while (TRUE) {
		if (pos > buffer_size - 1) {
			fprintf(stderr, "Error: Buffer not large enough to finish reading to whitespace\n");
			return -1;
		}
		c = getc(fp);
		// make sure we didn't read to the EOF
		if (c == EOF) {
			fprintf(stderr, ERR_UNEXPECTED_EOF);
			return -1;
		}
		if (c == '\r' || // carriage return
		    c == '\n' || // newline
		    c == ' '  || // space
		    c == '\t') { // tab
			fseek(fp, -1, SEEK_CUR);
			// add ASCIIZ, null terminate the string
			buffer[pos] = '\0';
			return pos;
		}
		buffer[pos++] = (char)c;
	}
};

/**
 * Load a PPM P3 file into an image
 * @param fp
 * @param image_ptr
 * @return
 */
int image_load_p3(FILE* fp, Image* image_ptr, int color_max, char buffer[]) {
	int height = image_ptr->height;
	int width = image_ptr->width;
	// Allocate space for the image in memory
	image_ptr->pixmap = malloc(sizeof(RGBpixel) * width * height);

	// Read the actual image in
	int i;
	int j;
	int k;
	int value;
	int bytes_read;
	for (i=0; i<height; i++) {
		for (j=0; j<width; j++) {
			for (k=0; k<3; k++) {
				// Read past the comments and whitespace
				if (skip_whitespace(fp) != 0) {
					fprintf(stderr, "Error: An error occurred skipping file whitespace\n");
					return 1;
				}

				bytes_read = read_to_whitespace(fp, buffer, IMAGE_READ_BUFFER_SIZE);
				if (bytes_read <= 0) {
					fprintf(stderr, "Error: Expected a color value but read nothing\n");
					return 1;
				}
				else if (atoi(buffer) > color_max) {
					fprintf(stderr, "Error: A color sample is greater than the maximum color (%i) value \n", color_max);
					return 1;
				}
				else if (atoi(buffer) < 0) {
					fprintf(stderr, "Error: A negative color sample is not a valid value \n");
					return 1;
				}
				value = (atoi(buffer)/(float)color_max)*255;
				
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
		// write the magic number
		fprintf(fp, "P3\n");
		// write the width and height
		fprintf(fp, "%i %i\n", image_ptr->width, image_ptr->height);
		// write the color max
		fprintf(fp, "255\n");
		// write all individual pixels
		for (i=0; i<image_ptr->height; i++) {
			for (j=0; j<image_ptr->width; j++) {
				RGBpixel pixel = image_ptr->pixmap[i*image_ptr->width + j];
				fprintf(fp, "%i\n%i\n%i\n", pixel.r, pixel.g, pixel.b);
			}
		}
		// close the file
		fclose(fp);
		return 0;
	}
	else {
		fprintf(stderr, ERR_OPEN_FILE_WRITING, fname);
		return 1;
	}
}

/**
 * Load a PPM P6 file into image_ptr
 * @param fp
 * @param image_ptr
 * @return
 */
int image_load_p6(FILE* fp, Image* image_ptr, int color_max, char buffer[]) {
	int height = image_ptr->height;
	int width = image_ptr->width;
	image_ptr->pixmap = malloc(sizeof(RGBpixel) * width * height);

	// Read past the comments and whitespace
	if (skip_whitespace(fp) != 0) {
		fprintf(stderr, "Error: An error occurred skipping file whitespace\n");
		return 1;
	}

	// Read the actual image in
	int i;
	int j;
	int k;
	size_t bytes_read;
	int value;
	for (i=0; i<height; i++) {
		for (j=0; j<width; j++) {
			for (k=0; k<3; k++) {
				// If color_max is < 256 the values are 8 bit, otherwise they're 16 bit!
				if (color_max < 256) {
					bytes_read = fread(buffer, sizeof(char), 1, fp);
					if (bytes_read < 1) {
						fprintf(stderr, "Error: Expected a color value but read nothing\n");
						return 1;
					}
					value = buffer[0] & 0xFF;
				}
				else {
					bytes_read = fread(buffer, sizeof(char), 2, fp);
					if (bytes_read < 2) {
						fprintf(stderr, "Error: Expected a color value but read nothing\n");
						return 1;
					}
					value = ((buffer[0] << 8) & 0xFF00) | (buffer[1] & 0xFF);
				}

				value = (value/(float)color_max)*255;

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
 * Save an image in PPM P6 format to the specified file
 * @param image_ptr
 * @param fname
 * @return
 */
int image_save_p6(Image* image_ptr, char* fname) {
	FILE* fp = fopen(fname, "w");
	int i;
	int j;
	// a small buffer to hold our values for the pixel to be written
	char buffer[3];
	if (fp) {
		// write the magic number
		fprintf(fp, "P6\n");
		// write the width and height
		fprintf(fp, "%i %i\n", image_ptr->width, image_ptr->height);
		// write the max color
		fprintf(fp, "255\n");
		for (i=0; i<image_ptr->height; i++) {
			for (j=0; j<image_ptr->width; j++) {
				// grab the correct pixel
				RGBpixel pixel = image_ptr->pixmap[i*image_ptr->width + j];
				// copy the rgb values to our buffer
				buffer[0] = pixel.r;
				buffer[1] = pixel.g;
				buffer[2] = pixel.b;
				// write the pixel to the file
				fwrite(buffer, sizeof(uint8_t), 3, fp);
			}
		}
		// close the file
		fclose(fp);
		return 0;
	}
	else {
		fprintf(stderr, ERR_OPEN_FILE_WRITING, fname);
		return 1;
	}
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
		int ppm_version = 0;
		char buffer[IMAGE_READ_BUFFER_SIZE];
		int bytes_read;
		int width;
		int height;
		int color_max;

		bytes_read = read_to_whitespace(fp, buffer, IMAGE_READ_BUFFER_SIZE);

		// Check for the magic number
		if (bytes_read != 2) {
			fprintf(stderr, ERR_INVALID_FILE);
			fclose(fp);
			return 1;
		}
		if (strncmp("P3", buffer, 2) == 0) {
			ppm_version = 3;
		}
		else if (strncmp("P6", buffer, 2) == 0) {
			ppm_version = 6;
		}
		else {
			fprintf(stderr, ERR_INVALID_FILE);
			fclose(fp);
			return 1;
		}

		// Read past the comments and whitespace
		if (skip_whitespace(fp) != 0) {
			fprintf(stderr, "Error: An error occurred skipping file whitespace\n");
			fclose(fp);
			return 1;
		}

		// We are at the first line of image header information
		// Read in the dimensions of the image

		// Read the width of the image
		bytes_read = read_to_whitespace(fp, buffer, IMAGE_READ_BUFFER_SIZE);

		if (bytes_read <= 0) {
			fprintf(stderr, "Error: Expected a width value but read nothing\n");
			return 1;
		}

		width = atoi(buffer);

		if (width < 0)
		{
			fprintf(stderr, ERR_INVALID_FILE);
			fclose(fp);
			return 1;
		}

		// Read past the comments and whitespace
		if (skip_whitespace(fp) != 0) {
			fprintf(stderr, "Error: An error occurred skipping file whitespace\n");
			fclose(fp);
			return 1;
		}

		// Read the height of the image
		bytes_read = read_to_whitespace(fp, buffer, IMAGE_READ_BUFFER_SIZE);

		if (bytes_read <= 0) {
			fprintf(stderr, "Error: Expected a width value but read nothing\n");
			return 1;
		}

		height = atoi(buffer);

		if (height < 0)
		{
			fprintf(stderr, ERR_INVALID_FILE);
			fclose(fp);
			return 1;
		}

		// Read past the comments and whitespace
		if (skip_whitespace(fp) != 0) {
			fprintf(stderr, "Error: An error occurred skipping file whitespace\n");
			fclose(fp);
			return 1;
		}

		// Read in the max color value
		bytes_read = read_to_whitespace(fp, buffer, IMAGE_READ_BUFFER_SIZE);

		if (bytes_read <= 0) {
			fprintf(stderr, "Error: Expected a maximum color value but read nothing\n");
			return 1;
		}

		color_max = atoi(buffer);

		if (color_max < 0 || color_max > 65535)
		{
			fprintf(stderr, ERR_INVALID_FILE);
			fclose(fp);
			return 1;
		}

		image_ptr->width = (uint32_t) width;
		image_ptr->height = (uint32_t) height;

		int result;
		if (ppm_version == 6)
			result = image_load_p6(fp, image_ptr, color_max, buffer);

		if (ppm_version == 3)
			result = image_load_p3(fp, image_ptr, color_max, buffer);

		fclose(fp);
		return result;
	}
	else {
		fprintf(stderr, ERR_OPEN_FILE_READING, fname);
		return 1;
	}
}

int main (int argc, char *argv[])
{
	if (argc != 4) {
		show_help();
		return 0;
	}

	char ppm_version_to = 0;
	char* ppm_version_str = argv[1];
	char* fname_input = argv[2];
	char* fname_output = argv[3];
	Image image;
	int result;

	// Check for a correct version input
	if (strcmp(ppm_version_str, "3") == 0)
		ppm_version_to = 3;
	else if (strcmp(ppm_version_str, "6") == 0)
		ppm_version_to = 6;

	if (ppm_version_to == 0) {
		show_help();
		return 0;
	}

	// Process the input file

	result = load_image(&image, fname_input);
	if (result != 0) {
		return result;
	}

	if (ppm_version_to == 3) {
		result = image_save_p3(&image, fname_output);
	}
	else {
		result = image_save_p6(&image, fname_output);
	}
	if (result != 0) {
		return result;
	}

	return 0;
}
