#ifndef _BMP_UTILS_H
#define _BMP_UTILS_H

#include <stdio.h>

// BMP file informations and bitmap
struct BMP_File {
	int width;
	int height;
	int bitPerPixel; // 1 is B&W, 24 is full color...
	unsigned char *bitmap; // BMP raw data
};


// Return 0 if success, -1 if a file error occured, and -2 if the file isn't
// a supported BMP format.
// bmp->bitmap is malloc'ed if success
int readBMP(struct BMP_File *bmp, FILE *file);

// put the monochrome representation of a BMP file into the bitmap array
// bitmap's size must be greater than or equal to the size needed (Casio representation)
void getMonoBitmap(const struct BMP_File *bmp, unsigned char *bitmap);


// Just for Fun : fill a file (stdout for example) with an ASCII representation of the icon!
void printBitmap(FILE *stream, unsigned char *bitmap, int w, int h);


#endif //_BMP_UTILS_H
