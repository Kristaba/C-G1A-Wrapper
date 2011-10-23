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


#include "g1a_header.h"
#include "bmp_utils.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>


#define UINT32_TO_BIG_ENDIAN(i) ((((i)&0xFF000000)>>24) + (((i)&0x00FF0000)>>8) + (((i)&0x0000FF00)<<8) + (((i)&0x000000FF)<<24))

#define BUFFER_SIZE 1024

// Print c_g1awrapper usage into stdout
// programName is the name of this binary program (argv[0])
void printUsage(const char *programName);

int main(int argc, char **argv) {
	struct G1A_Header header;

	char *imgFileName;
	char *binFileName;
	char *g1aFileName;
	char *name;
	char *version;
	char *date;
	char *internalName;

	int tmp;
	int previewIcon = 0;
	int g1aNameProvided, imgNameProvided, nameProvided, versionProvided, dateProvided, internalNameProvided;
	int error;
	int i;
	int binSize;

	unsigned char buffer[BUFFER_SIZE];

	FILE *img_f = NULL, *g1a_f = NULL, *bin_f = NULL;

	printf("-- g1awrapper : 2008 Andreas Bertheussen, 2011 Leo Grange.\n");
	memset(&header, 0, sizeof(struct G1A_Header)); // init header struct

	// default file names :
	imgFileName = "icon.bmp";
	g1aFileName = "output.g1a";

	if(argc >= 2 && (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help"))) {
		printUsage(argv[0]);
		return 0;
	}
	
	error = 0;
	i = 2;
	g1aNameProvided = 0;
	imgNameProvided = 0;
	versionProvided = 0;
	nameProvided = 0;
	dateProvided = 0;
	internalNameProvided = 0;
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
		else if(!strcmp(argv[i], "-n") || !strcmp(argv[i], "--name")) {
			nameProvided = 1;
			name = argv[i+1];
			i+=2;
		}
		else if(!strcmp(argv[i], "-N") || !strcmp(argv[i], "--internal-name")) {
			internalNameProvided = 1;
			internalName = argv[i+1];
			i+=2;
		}
		else if(!strcmp(argv[i], "-v") || !strcmp(argv[i], "--version-string")) {
			versionProvided = 1;
			version = argv[i+1];
			i+=2;
		}
		else if(!strcmp(argv[i], "-d") || !strcmp(argv[i], "--date-string")) {
			dateProvided = 1;
			date = argv[i+1];
			i+=2;
		}
		else if(!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
			printUsage(argv[0]);
			return 0;
		}
		else error = 1;
		if(i>argc) error = 1;
	}

	if(error || argc<2) {
		printUsage(argv[0]);
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
		else if(bmp.width!=30 || (bmp.height!=18 && bmp.height != 19)) {
			printf("[W] Bad icon size (%d*%d). Must be 30*18 pixel. Icon will be BLANK.\n", bmp.width, bmp.height);
		}
		else {
			if(bmp.height == 19) bmp.height = 18; // to support 30*19 icons from the official SDK 
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
	if(!internalNameProvided) {
		if(!nameProvided) internalName = g1aFileName;
		else internalName = name;
	}	
	tmp = strlen(internalName);
	memcpy(header.filename, internalName, tmp > (int)sizeof(header.filename) ? (int)sizeof(header.filename) : tmp);
	header.estrip_count = 0;
	if(!versionProvided) memcpy(header.version, "fxSDK.1337", 10);
	else {
		memcpy(header.version, "     .1337", 10);
		tmp = strlen(version);
		memcpy(header.version, version, tmp > (int)sizeof(header.version) ? (int)sizeof(header.version) : tmp);
	}

	// TODO get the real date
	//printf ("[I] Timestamp: %s\n", ?);
	if(!dateProvided) date = "2011.1012.1200";
	tmp = strlen(date);
	memcpy(header.date, date, tmp > (int)sizeof(header.date) ? (int)sizeof(header.date) : tmp);

	if(!nameProvided) name = g1aFileName;
	tmp = strlen(name);
	memcpy(header.name, name, tmp > (int)sizeof(header.name) ? (int)sizeof(header.name) : tmp);
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
		if((int)fwrite(buffer, sizeof(char), read, g1a_f) != read) {
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



void printUsage(const char *programName) {
	printf("Usage: %s <bin_filename> [OPTION]...\n"
		"Options are :\n"
		"  -o, --output <g1a filename>      Name of the output file. Default is\n"
		"                                    'output.g1a'.\n"
		"  -i, --icon <icon filename>       Name of the icon file. Default is 'icon.bmp'.\n"
		"  -v, --version-string <version>   String visible in 'VERSION' menu.\n"
		"                                   5 characters are displayed.\n"
		"  -d, --date-string <date>         Date of the build, not important.\n"
		"                                   Format is 'yyyy.MMdd.hhmm'\n"
		"  -n, --name <title name>          Title name, shown in 'VERSION' menu.\n"
		"                                   8 characters used.\n"
		"                                   Default is the g1a filename.\n"
		"  -N, --name <internal name>       Internal name used by the OS.\n"
		"                                   8 characters used, upper case is advisable.\n"
		"                                   Default is the title name.\n"
		"  -p, --preview-icon               Print an ASCII preview of the icon in stdout.\n"
		"  -h, --help                       Display this help.\n"
		, programName);
}
