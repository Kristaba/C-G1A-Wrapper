#include "g1a_header.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>


#define UINT32_TO_BIG_ENDIAN(i) ((((i)&0xFF000000)>>24) + (((i)&0x00FF0000)>>8) + (((i)&0x0000FF00)<<8) + (((i)&0x000000FF)<<24))

#define BUFFER_SIZE 1024


// return the opt's position or -1 if there is no opt with this name
int getOptPos(int argc, char **argv, const char *opt);

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
int getMonoBitmap(const struct BMP_File *bmp, unsigned char *bitmap);


// Just for Fun : fill a file (stdout for example) with an ASCII representation of the icon!
int printBitmap(FILE *stream, unsigned char *bitmap, int w, int h);


int main(int argc, char **argv) {
	struct G1A_Header header;

	char *imgFileName;
	char *binFileName;
	char *g1aFileName;

	int tmp;
	int previewIcon = 0;
	int g1aNameProvided, imgNameProvided;
	int error;
	int i;
	int binSize;

	unsigned char buffer[BUFFER_SIZE];

	FILE *img_f = NULL, *g1a_f = NULL, *bin_f = NULL;

	printf("-- c_g1awrapper Copyright (C) 2008 Andreas Bertheussen, 2011 Leo Grange.\n");
	memset(&header, 0, sizeof(struct G1A_Header)); // init header struct

	// default file names :
	imgFileName = "icon.bmp";
	g1aFileName = "output.g1a";

	error = 0;
	i = 2;
	g1aNameProvided = 0;
	imgNameProvided = 0;
	while(!error && i<argc) {
		if(!strcmp(argv[i], "-o") || !strcmp(argv[i], "--output")) {
			if(i+1 < argc) g1aFileName = argv[i+1];
			g1aNameProvided = 1;
			i+=2;
		}
		else if(!strcmp(argv[i], "-i") || !strcmp(argv[i], "--icon")) {
			if(i+1 < argc) imgFileName = argv[i+1];
			imgNameProvided = 1;
			i+=2;
		}
		else if(!strcmp(argv[i], "-p") || !strcmp(argv[i], "--preview-icon")) {
			previewIcon = 1;
			i+=1;
		}
		else error = 1;
		if(i>argc) error = 1;
	}

	if(error || argc<2) {
		printf("Usage: %s <bin_filename> [OPTION]...\n"
				"Options are :\n"
				"\t-o --output <g1a filename>\n"
				"\t-i --icon <icon filename>\n"
				"\t-p --preview-icon\n",
				argv[0]);

		return -1;
	}

	if(!imgNameProvided) printf("[I] No icon file name provided. Name set to default : \'icon.bmp\'\n");
	if(!g1aNameProvided) printf("[I] No output name provided. Name set to default : \'output.g1a\'\n");
	
	binFileName = argv[1]; // <bin filename> argument

	// Set the icon bitmap :
	
	printf("[>] Reading icon file \'%s\'.\n", imgFileName); 
	img_f = fopen(imgFileName, "rb");
	if(img_f == NULL) {
		printf("[W] Could not open the icon file. Icon will be BLANK.\n");
	}
	else {
		struct BMP_File bmp;
		if((tmp = readBMP(&bmp, img_f))) {
			// error when reading the icon file
			if(tmp == -2) printf("[W] Unreconized BMP file. Icon will be BLANK.\n");
			else printf("[W] Error when reading icon file. Icon will be BLANK.\n");
		}
		else if(bmp.width!=30 || bmp.height!=18) {
			printf("[W] Bad icon size (%d*%d). Must be 30*18 pixel. Icon will be BLANK.\n", bmp.width, bmp.height);
		}
		else {
			getMonoBitmap(&bmp, header.menu_icon);
			if(previewIcon) printBitmap(stdout, header.menu_icon, 30, 18);
		}
		fclose(img_f);
		img_f = NULL;
	}

	bin_f = fopen(binFileName, "rb");
	if(bin_f == NULL) {
		printf("[E] Binary file '%s' does not exist or can't be opened. Exiting.\n", binFileName);
		return -1;
	}
	printf("[I] File '%s' selected as binary source.\n", binFileName);
	// Get Binary file size
	fseek(bin_f, 0, SEEK_END);
	binSize = ftell(bin_f);
	//printf("bin size : %d\n", binSize);
	fseek(bin_f, 0, SEEK_SET);

	printf("[I] File '%s' selected for G1A file.\n", g1aFileName);
	g1a_f = fopen(g1aFileName, "rb");
	if(g1a_f != NULL) {
		printf("[W] The destination file '%s' exists, and will be overwritten.\n", g1aFileName);
		fclose(g1a_f);
		g1a_f = NULL;
	}

	// Now fill the header struct :
	memcpy(header.magic,"\xAA\xAC\xBD\xAF\x90\x88\x9A\x8D\x0C\xFF\xEF\xFF\xEF\xFF", sizeof(header.magic));
	header.stuff1 = 0xFE;
	header.inverted_filesize = ~UINT32_TO_BIG_ENDIAN(binSize + 0x200);
	
	// write the checkbytes
	uint8_t lowbyte = (uint8_t)((header.inverted_filesize & 0xFF000000)>>24);	// last (LSB) byte in the little-endian uint32
	header.checkbyte1 = lowbyte - 0x41;
	header.checkbyte2 = lowbyte - 0xB8;
	
	header.name_start = '@';
	// TODO use an other name here?
	tmp = strlen(g1aFileName);
	memcpy(header.filename, g1aFileName, tmp > sizeof(header.filename) ? sizeof(header.filename) : tmp);
	header.estrip_count = 0;
	memcpy(header.version, "fxSDK.1337", 10); // less important field, let's brand it

	// TODO get the real date
	//printf ("[I] Timestamp: %s\n", ?);
	memcpy(header.date, "2011.1012.1200", sizeof(header.date));

	// TODO use an other name here?
	tmp = strlen(g1aFileName);
	memcpy(header.name, g1aFileName, tmp > sizeof(header.filename) ? sizeof(header.filename) : tmp);
	header.filesize = UINT32_TO_BIG_ENDIAN(binSize + 0x200); // include size of header
	printf("[I] G1A size:  %d\n", binSize + 0x200);

	printf("[>] Writing header..");
	g1a_f = fopen(g1aFileName, "wb+");
	if(g1a_f == NULL) {
		printf("\n[E] G1A file could not be opened for writing. Exiting.");
		return -1;
	}
	if(fwrite((char*)&header, sizeof(char), sizeof(header), g1a_f) != sizeof(header)) {
		printf("\n[E] Could not write to G1A. Exiting.");
		return -1;
	}
	printf(" OK.\n[>] Writing data..");

	int read;
	while ((read = fread(buffer, sizeof(char), BUFFER_SIZE, bin_f)) > 0) {
		if(fwrite(buffer, sizeof(char), read, g1a_f) != read) {
			printf("\n[E] Could not write to G1A. Exiting.");
			return -1;
		}
	}
	fflush(g1a_f);

	fclose(bin_f);
	fclose(g1a_f);

	printf(" OK.\n\n");
	return 0;
}



int readBMP(struct BMP_File *bmp, FILE *file) {
	unsigned char buffer[128];
	
	if(fread(buffer, sizeof(unsigned char), 2, file) != 2) return -1;

	if(	   (buffer[0]=='B' && buffer[1]=='M')
		|| (buffer[0]=='B' && buffer[1]=='A')
		|| (buffer[0]=='C' && buffer[1]=='I')
		|| (buffer[0]=='C' && buffer[1]=='P')
		|| (buffer[0]=='I' && buffer[1]=='C')
		|| (buffer[0]=='P' && buffer[1]=='T') ) 
	{
		unsigned int offset;
		unsigned int tmp, rawsize;
		int w, h;
		int bpp;
		
		fseek(file, 8, SEEK_CUR);
		if(fread(buffer, sizeof(unsigned char), 4, file) != 4) return -1;
		offset = (buffer[3]<<24) + (buffer[2]<<16) + (buffer[1]<<8) + (buffer[0]);
		printf("offset : %d\n", offset);

		if(fread(buffer, sizeof(unsigned char), 4, file) != 4) return -1;
		tmp = (buffer[3]<<24) + (buffer[2]<<16) + (buffer[1]<<8) + (buffer[0]);
		printf("header size : %d\n", tmp);
		if(tmp == 40) {
			if(fread(buffer, sizeof(unsigned char), 4, file) != 4) return -1;
			w = (buffer[3]<<24) + (buffer[2]<<16) + (buffer[1]<<8) + (buffer[0]);
			if(fread(buffer, sizeof(unsigned char), 4, file) != 4) return -1;
			h = (buffer[3]<<24) + (buffer[2]<<16) + (buffer[1]<<8) + (buffer[0]);
			printf("image dimensions : (%d,%d)\n", w, h);
			
			fseek(file, 2, SEEK_CUR);
			if(fread(buffer, sizeof(unsigned char), 2, file) != 2) return -1;
			bpp = (buffer[1]<<8) + (buffer[0]);
			printf("bit per pixel : %d\n", bpp);

			fseek(file, 4, SEEK_CUR);
			if(fread(buffer, sizeof(unsigned char), 4, file) != 4) return -1;
			rawsize = (buffer[3]<<24) + (buffer[2]<<16) + (buffer[1]<<8) + (buffer[0]);
			tmp = ((bpp * w - 1)/32 + 1)*4 * h;
			printf("raw size : %d (computed : %d)\n", rawsize, tmp);

			if(rawsize != tmp) return -2;

			fseek(file, offset, SEEK_SET);
			if(!(bmp->bitmap = malloc(sizeof(unsigned char)*rawsize))) return -1;
			if(fread(bmp->bitmap, sizeof(unsigned char), rawsize, file) != rawsize) {
				free(bmp->bitmap);
				return -1;
				printf("Error when reading the raw bitmap.\n");
			}

			bmp->width = w;
			bmp->height = h;
			bmp->bitPerPixel = bpp;
			printf("Bitmap read.\n");

		}
		else {
			// Unknown BMP header format!
			return -2;
		} 

	}
	else return -2;

	return 0;
}


int getMonoBitmap(const struct BMP_File *bmp, unsigned char *bitmap) {
	int i, j;
	int lineSize = ((bmp->bitPerPixel * bmp->width - 1)/32 + 1)*4;
	int bLineSize = (bmp->width -1)/8 + 1;
	int rowOffset;

	for(i=0; i < bLineSize * bmp->height; i++) bitmap[i] = 0x00;
	
	for(i=0; i < bmp->height; i++) {
		rowOffset = lineSize*(bmp->height - i - 1);
		for(j=0; j < bmp->width; j++) {
			// pixel : if is 0 current pixel is black.
			int pixel = 1;

			switch(bmp->bitPerPixel) {
			case 1:
				pixel = !(bmp->bitmap[rowOffset + j/8] & (0x01 << (7-(j%8))));
				break;
			case 2:
				// TODO TODO TODO
				pixel = !(bmp->bitmap[rowOffset + j/4] & (0x03 << (2*(3-(j%4)))));
				break;
			case 4:
				pixel = !(bmp->bitmap[rowOffset + j/2] & (0x07 << (4*(1-(j%2)))));
				break;
			case 8:
				pixel = !(bmp->bitmap[rowOffset + j]);
				break;
			case 24:
				pixel = ((bmp->bitmap[rowOffset + 3*j] & bmp->bitmap[rowOffset + 3*j+1] & bmp->bitmap[rowOffset + 3*j+2]) == 0xFF);
				break;
			case 32:
				pixel = ((bmp->bitmap[rowOffset + 4*j] | bmp->bitmap[rowOffset + 4*j+1] | bmp->bitmap[rowOffset + 4*j+2]) == 0xFF);
				break;
			}
			if(pixel == 0) bitmap[bLineSize*i + j/8] |= 0x1 << (7-(j%8));
		}
	}
}


int printBitmap(FILE *stream, unsigned char *bitmap, int w, int h) {
	int i, j;
	int bLineSize = (w -1)/8 + 1;
	int pixelInByte;

	for(i=0; i<h; i++) {
		for(j=0; j<w; j++) {
			//pixelInByte 
			//if(j+1 >= w) 
			if(bitmap[bLineSize*i + j/8] & (0x1 << (7-(j%8)))) {
				fputc('#', stream);
				fputc('#', stream);
			}
			else {
				fputc(' ', stream);
				fputc(' ', stream);
			}
		}
		fputc('\n', stream);
	}

}
