/**************************************************************************
    This file is part of c_g1awrapper.

    c_g1awrapper is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    c_g1awrapper is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with c_g1awrapper.  If not, see <http://www.gnu.org/licenses/>.
 *************************************************************************/


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
