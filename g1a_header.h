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


#ifndef _G1A_HEADER_H
#define _G1A_HEADER_H

// for compatibility with all arch
#include <stdint.h>

struct G1A_Header {
	uint8_t magic[14];
	uint8_t checkbyte1;
	uint8_t stuff1;
	uint32_t inverted_filesize;
	uint8_t checkbyte2;
	uint8_t unknown[11];
	uint8_t name_start;
	uint8_t filename[8]; /* name used internally for settings etc? */
	uint8_t unknown_2[2];
	uint8_t estrip_count;
	uint8_t unknown_3[4];
	uint8_t version[10]; /* first 5 bytes shown in system menu */
	uint8_t unknown_4[2];
	uint8_t date[14];
	uint8_t unknown_5[2];
	uint8_t menu_icon[68];
	uint8_t estrip_1_data[80];
	uint8_t estrip_2_data[80];
	uint8_t estrip_3_data[80];
	uint8_t estrip_4_data[80];
	uint8_t unknown_6[4];
	uint8_t name[8];	/* shown in VERSION in the system menu. should be called 'title'? */
	uint8_t unknown_7[20];
	uint32_t filesize;
	uint8_t unknown_8[12];
};


#endif 
// _G1A_HEADER_H

