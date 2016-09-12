# CS430 Project 1 - Images

This project allows loading and saving images in PPM P3 and P6 formats. The images are first completely loaded into a pixmap in memory before being written back out into the specified format.

### Building

```sh
$ git clone https://github.com/bjg96/cs430-project-1-images.git
$ cd cs430-project-1-images
$ make
```

### Usage

```sh
$ ./ppmrw <output PPM version 3|6> <input_image.ppm> <output_image.ppm>
```