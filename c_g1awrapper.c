#include "g1a_header.h"
#include "bmp_utils.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>


#define UINT32_TO_BIG_ENDIAN(i) ((((i)&0xFF000000)>>24) + (((i)&0x00FF0000)>>8) + (((i)&0x0000FF00)<<8) + (((i)&0x000000FF)<<24))

#define BUFFER_SIZE 1024

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
			free(bmp.bitmap);
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
