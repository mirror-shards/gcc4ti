/******************************************************************************
*
* project name:    TI-68k Developer Utilities
* file name:       ttebkgen.c
* initial date:    24/08/2000
* authors:         thomas.nussbaumer@gmx.net
* description:     executes all steps till a complete eBook
*
******************************************************************************/

/*
  This file is part of TI-68k Developer Utilities.

  This file is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  As a special exception, UNMODIFIED copies of ttebkgen may also be
  redistributed or sold without source code, for any purpose. (The Lesser
  General Public License restrictions do apply in other respects; for example,
  they cover modification of the program.) This exception notice must be
  removed on modified copies of this file.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "tt.h"          // generic defines
#include "ttversion.h"   // TI-68k Developer Utilities version info
#include "revtools.h"    // automatic cvs version extraction

#ifdef FILE_REVISION
#undef FILE_REVISION
#endif
#define FILE_REVISION "1.10"

#define EMBEDDED_USE
#include "ttsplit.c"
#include "ttebkmeta.h"
#include "packhead.h"    // compressed header definition
#include "ttdos2ebk.c"
#include "ttarchive.h"
#include "bin2oth.c"
#include "ttarchive.c"
#undef EMBEDDED_USE

//=============================================================================
// outputs usage information of this tool
//=============================================================================
void PrintUsage() {
    PRINT_ID("TTEbkGen");

    fprintf(USAGE_OUT, "Usage: ttebkgen [flags] <infile> <varname> \"description\"\n\n"\
                       "-t titlefile ... own title image - see Note (8)\n"\
                       "-89          ... generate ONLY TI89 ebook\n"\
                       "-92          ... generate ONLY TI92p ebook\n"\
                       "-v           ... verbose (unsets -quiet)\n"\
                       "-quiet       ... don't output standard messages (unsets -v)\n"\
                       "-keepfiles   ... keep all temporary generated files (for debugging only)\n\n"\
                       "the infile must contain the complete text of the final ebook.\n"\
                       "split points must be marked within the inputfile with tag\n"\
                       "<--split--> on a single line.\n"\
                       "NOTES:\n"\
                       " (1) the maximum uncompressed size of a ebook part is 65518 bytes.\n"\
                       " (2) the final size of the ebook cannot extend 65608 bytes\n"\
                       "     (including PC header infos)\n"\
                       " (3) if a part is too long, define more splitpoints in the infile\n"\
                       " (4) if the complete text will not fit into one ebook, split\n"\
                       "     the text into two or more ebooks\n"\
                       " (5) varname is used as outfile name and as varname on the calculator\n"\
                       " (6) don't forget to enclose the description in quotes !!\n"\
                       " (7) just the first 19 character of the description are used\n"\
                       " (8) titlefile must be a 4-grayscale pic with 160x100 pixels in binary form\n"\
                       "     generated with ImageStudio (not compressed binary)\n\n");
}


char* tmptitlename = "tt_tf00.bin";
char* titlename    = 0;

unsigned char default_image[] = {
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1f,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xe0,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1f,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xe0,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1f,0xff,0xff,0xff,0xfe,
0x00,0x00,0x00,0x00,0x60,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1f,0xff,0xff,0xff,0xfe,
0x00,0x00,0x00,0x00,0x60,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1e,0x00,0x00,0x00,0x0e,
0x00,0x7f,0xff,0x80,0x60,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1e,0xff,0xff,0xff,0xef,
0xff,0xf7,0xff,0xff,0xff,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1e,0xcf,0xff,0xfc,0xef,
0xff,0xff,0xfb,0xff,0xff,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1e,0xbf,0xff,0xff,0xef,
0xf0,0x03,0xfc,0x00,0x63,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1e,0xff,0xff,0xff,0xaf,
0xf0,0x03,0xfc,0x00,0x63,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1e,0x7f,0xc0,0x7f,0xce,
0x70,0x03,0xff,0xf0,0x63,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1e,0xff,0xc0,0x7f,0xce,
0x70,0x03,0xff,0xe0,0x63,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1f,0xff,0xfc,0x7f,0xce,
0x70,0x03,0xff,0xc0,0x63,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1f,0xff,0xfc,0x7f,0xfe,
0x70,0x03,0xff,0xc0,0x63,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1f,0xff,0xfc,0x7f,0xfe,
0x70,0x03,0xff,0xc0,0x63,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1f,0xff,0xfc,0x7f,0xfe,
0x70,0x03,0xff,0xc0,0x63,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1f,0xff,0xfc,0x7f,0xfe,
0x70,0x03,0xff,0xc0,0x63,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1f,0xff,0xfc,0x7f,0xfe,
0x70,0x03,0xff,0xc0,0x63,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1f,0xff,0xfc,0x7f,0xfe,
0x70,0x03,0xff,0xc0,0x63,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1f,0xff,0xfc,0x7f,0xff,
0xf0,0x03,0xff,0xc0,0x63,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1f,0xff,0xfc,0x7f,0xff,
0xf0,0x03,0xff,0xc0,0x63,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1f,0xff,0xfc,0x7f,0xff,
0xf0,0x03,0xff,0xc0,0x63,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1f,0xff,0xfc,0x7f,0xff,
0xf0,0x03,0xff,0xc0,0x63,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1f,0xff,0xfc,0x7f,0xff,
0xf0,0x03,0xff,0xc0,0x63,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1f,0xff,0xfc,0x7f,0xff,
0xf0,0x03,0xff,0xc0,0x63,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1f,0xff,0xfc,0x7f,0xff,
0xf0,0x03,0xff,0xc0,0x63,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1f,0xff,0xfc,0x7f,0xff,
0xf0,0x03,0xff,0xc0,0x63,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1f,0xff,0xfc,0x7f,0xff,
0xf0,0x03,0xff,0xc0,0x63,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1f,0xff,0xfc,0x7f,0xff,
0xf0,0x03,0xff,0xc0,0x63,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1f,0xff,0xfc,0x7f,0xff,
0xf0,0x03,0xff,0xc0,0x63,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1f,0xff,0xfc,0x7f,0xff,
0xf0,0x03,0xff,0xc0,0x63,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1f,0xff,0xfc,0x7f,0xff,
0xf0,0x03,0xff,0xc0,0x63,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1f,0xff,0xfc,0x7f,0xff,
0xf0,0x03,0xff,0xc0,0x63,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1f,0xff,0xfc,0x7f,0xff,
0xf0,0x03,0xff,0xc0,0x63,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1f,0xff,0xfc,0x7f,0xff,
0xf0,0x07,0xfb,0xc0,0x63,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1f,0xff,0xfc,0x7f,0xff,
0xf0,0x07,0xff,0xc0,0x63,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1f,0xff,0xfc,0x1f,0xff,
0xf0,0x7f,0xff,0x80,0x63,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1f,0xff,0xff,0xff,0xff,
0xf0,0x00,0x1f,0xc0,0x63,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1f,0xff,0xff,0xff,0xff,
0xf0,0x00,0x1f,0xc0,0x63,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1f,0xff,0xff,0xff,0xff,
0xf0,0x00,0x1f,0xc0,0x63,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1f,0xff,0xff,0xff,0xff,
0xf0,0x00,0x3f,0xe0,0x63,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x18,0xff,0x80,0x00,0x1f,
0xff,0xff,0xff,0xff,0xe3,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x18,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xe3,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x18,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xe3,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x18,0xff,0xd7,0xd7,0xef,
0xff,0xff,0xff,0xff,0xe3,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x18,0xff,0x7f,0xfc,0xdf,
0xf0,0x00,0x00,0x00,0xe3,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x18,0xc1,0xfc,0xef,0xb9,
0xef,0xff,0xff,0xfe,0xff,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x18,0xc7,0xf0,0x07,0xf9,
0xec,0xff,0xff,0xce,0xff,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x18,0xdf,0xe0,0x00,0xf9,
0xeb,0xff,0xff,0xfe,0xff,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x18,0xff,0xc0,0x00,0xf9,
0xef,0xff,0xff,0xfa,0xff,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x18,0xff,0x81,0xff,0xf9,
0xe7,0xfc,0x07,0xfc,0xe7,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x18,0x7f,0x87,0xc0,0x39,
0xef,0xfc,0x07,0xfc,0xe7,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x18,0x7f,0x1f,0x80,0x1f,
0xff,0xff,0xc7,0xfc,0xe7,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x18,0xfe,0x7e,0x00,0x1f,
0xff,0xff,0xc7,0xff,0xe7,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x19,0xfe,0xfc,0x00,0x1b,
0xff,0xff,0xc7,0xff,0xe7,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x19,0xff,0xf8,0x00,0x01,
0xff,0xff,0xc7,0xff,0xe7,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x19,0xff,0xf8,0x00,0x01,
0xff,0xff,0xc7,0xff,0xe7,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1b,0xfd,0xf0,0x00,0x01,
0xff,0xff,0xc7,0xff,0xe7,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1b,0xff,0xf0,0x00,0x01,
0xff,0xff,0xc7,0xff,0xe7,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1b,0xff,0xf0,0x00,0x01,
0xff,0xff,0xc7,0xff,0xff,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1f,0xff,0xe0,0x00,0x01,
0xff,0xff,0xc7,0xff,0xff,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1f,0xff,0xe0,0x00,0x01,
0xff,0xff,0xc7,0xff,0xff,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1f,0xff,0xe0,0x00,0x01,
0xff,0xff,0xc7,0xff,0xff,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1f,0xfd,0xe0,0x00,0x01,
0xff,0xff,0xc7,0xff,0xff,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1b,0xfd,0xe0,0x00,0x01,
0xff,0xff,0xc7,0xff,0xff,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1b,0xff,0xe0,0x00,0x01,
0xff,0xff,0xc7,0xff,0xff,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x19,0xff,0xe0,0x00,0x01,
0xff,0xff,0xc7,0xff,0xff,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x19,0xfe,0xe0,0x00,0x01,
0xff,0xff,0xc7,0xff,0xff,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x19,0xff,0xe0,0x00,0x01,
0xff,0xff,0xc7,0xff,0xff,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x19,0xff,0xe0,0x00,0x01,
0xff,0xff,0xc7,0xff,0xff,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x18,0xff,0x60,0x00,0x21,
0xff,0xff,0xc7,0xff,0xff,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x18,0x7f,0xe0,0x00,0x61,
0xff,0xff,0xc7,0xff,0xff,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x18,0xff,0xf0,0x00,0xe1,
0xff,0xff,0xc7,0xff,0xff,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x18,0xff,0xf0,0x01,0xc1,
0xff,0xff,0xc7,0xff,0xff,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x18,0xcf,0xe0,0x07,0xc1,
0xff,0xff,0xc7,0xff,0xff,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x18,0xc3,0xfd,0xcf,0x01,
0xff,0xff,0xc7,0xff,0xff,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x18,0xc2,0xff,0xfe,0x01,
0xff,0xff,0xc1,0xff,0xff,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x18,0xc1,0xa7,0xd0,0x03,
0xff,0xff,0xff,0xff,0xff,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x18,0xc0,0xfe,0x00,0x07,
0xff,0xff,0xff,0xff,0xff,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1f,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1f,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0xc0,0x03,0xff,0xc0,
0x0f,0xf8,0x00,0x01,0xff,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0xc0,0x00,0x00,0x00,
0x0f,0xff,0xff,0xff,0xff,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0xc0,0x00,0x00,0x00,
0x0f,0xff,0xff,0xff,0xff,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x60,0x03,0x80,0x00,
0x00,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x7c,0x00,0x00,0x70,0x07,0xc0,0x00,
0x1f,0xef,0x1e,0x1e,0x03,0x9e,0x1e,0x00,0x00,0x00,
0x00,0x00,0x00,0x7e,0x00,0x00,0x70,0x07,0x00,0x00,
0x0f,0xf7,0xbf,0x3f,0x07,0xbf,0x3f,0x04,0x00,0x00,
0x00,0x00,0x3c,0x77,0x3c,0x3c,0x76,0x0f,0x9e,0x36,
0x03,0x87,0x3b,0xbb,0x87,0x3b,0x9b,0x86,0x00,0x00,
0x00,0x00,0x7e,0x7f,0x7e,0x7e,0x7f,0x07,0xff,0x3f,
0x03,0x87,0x1f,0xbb,0x8f,0x3b,0x87,0x86,0x00,0x00,
0x00,0x00,0x7f,0x7e,0x77,0x77,0x7e,0x07,0x3b,0xbf,
0x03,0x87,0x3f,0x1f,0x8e,0x1f,0x8f,0x3f,0x80,0x00,
0x00,0x00,0x7f,0x77,0x77,0x77,0x7c,0x07,0x3b,0xb8,
0x03,0x87,0x3b,0x8f,0x8e,0x0f,0x9e,0x1f,0xc0,0x00,
0x00,0x00,0x72,0x77,0x77,0x77,0x7c,0x07,0x3b,0xb8,
0x03,0x87,0x3b,0x87,0x9e,0x07,0xbc,0x06,0x00,0x00,
0x00,0x00,0x3d,0x7f,0x3f,0x3f,0x76,0x07,0x1f,0xb8,
0x03,0x8f,0x1f,0x9f,0x1c,0x1f,0x3f,0x06,0x00,0x00,
0x00,0x00,0x1e,0x3e,0x1e,0x1e,0x33,0x03,0x0f,0x18,
0x01,0x87,0x8f,0x0e,0x3c,0x0e,0x1f,0x82,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x38,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x18,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1f,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xe0,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1f,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xe0,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1f,0xff,0xff,0xff,0xfe,
0x00,0x00,0x00,0x00,0x60,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1f,0xff,0xff,0xff,0xfe,
0x00,0x00,0x00,0x00,0x60,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1e,0x00,0x00,0x00,0x1e,
0x00,0x7f,0xff,0xc0,0x60,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1e,0x00,0x00,0x00,0x1e,
0x00,0x0f,0xfc,0x00,0x60,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1e,0x1f,0x80,0x7e,0x1e,
0x00,0x07,0xfc,0x00,0x60,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1e,0x3f,0x80,0x7f,0x1e,
0x00,0x07,0xf8,0x00,0x60,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1e,0x7f,0x80,0x7f,0x9e,
0x00,0x07,0xf8,0x00,0x60,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1e,0xff,0x80,0x7f,0xde,
0x00,0x07,0xf8,0x00,0x60,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1e,0xff,0x80,0x7f,0xde,
0x00,0x07,0xf8,0x00,0x60,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1e,0xff,0x80,0x7f,0xfe,
0x00,0x07,0xf8,0x00,0x60,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1f,0xff,0x80,0x7f,0xfe,
0x00,0x07,0xf8,0x00,0x60,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1f,0xff,0x80,0x7f,0xfe,
0x00,0x07,0xf8,0x00,0x60,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1f,0xff,0x80,0x7f,0xfe,
0x00,0x07,0xf8,0x00,0x60,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1f,0xff,0x80,0x7f,0xfe,
0x00,0x07,0xf8,0x00,0x60,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1f,0xff,0x80,0x7f,0xfe,
0x00,0x07,0xf8,0x00,0x60,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1f,0xff,0x80,0x7f,0xfe,
0x00,0x07,0xf8,0x00,0x60,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1f,0xff,0x80,0x7f,0xfe,
0x00,0x07,0xf8,0x00,0x60,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1f,0xff,0x80,0x7f,0xfe,
0x00,0x07,0xf8,0x00,0x60,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1f,0xff,0x80,0x7f,0xfe,
0x00,0x07,0xf8,0x00,0x60,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1f,0xff,0x80,0x7f,0xfe,
0x00,0x07,0xf8,0x00,0x60,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1f,0xff,0x80,0x7f,0xfe,
0x00,0x07,0xf8,0x00,0x60,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1f,0xff,0x80,0x7f,0xfe,
0x00,0x07,0xf8,0x00,0x60,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1f,0xff,0x80,0x7f,0xfe,
0x00,0x07,0xf8,0x00,0x60,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1f,0xff,0x80,0x7f,0xfe,
0x00,0x07,0xf8,0x00,0x60,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1f,0xff,0x80,0x7f,0xfe,
0x00,0x07,0xf8,0x00,0x60,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1f,0xff,0x80,0x7f,0xfe,
0x00,0x07,0xf8,0x00,0x60,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1f,0xff,0x80,0x7f,0xfe,
0x00,0x07,0xf8,0x00,0x60,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1f,0xff,0x80,0x7f,0xfe,
0x00,0x07,0xf8,0x00,0x60,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1f,0xff,0x80,0x7f,0xfe,
0x00,0x07,0xf8,0x00,0x60,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1f,0xff,0x80,0x7f,0xfe,
0x00,0x07,0xf8,0x00,0x60,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1f,0xff,0x80,0x7f,0xfe,
0x00,0x07,0xf8,0x00,0x60,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1f,0xff,0x80,0x7f,0xfe,
0x00,0x07,0xfc,0x00,0x60,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1f,0xff,0x00,0x3f,0xfe,
0x00,0x0f,0xfc,0x00,0x60,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1f,0xf8,0x00,0x07,0xfe,
0x00,0x7f,0xff,0xc0,0x60,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1f,0xff,0xff,0xff,0xfe,
0x00,0x00,0x00,0x00,0x60,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1f,0xff,0xff,0xff,0xfe,
0x00,0x00,0x00,0x00,0x60,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1f,0xff,0xff,0xff,0xfe,
0x00,0x00,0x00,0x00,0x60,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1f,0xff,0xff,0xff,0xfe,
0x00,0x00,0x00,0x00,0x60,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x18,0x00,0x00,0x00,0x01,
0xff,0xff,0xff,0xff,0xe0,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x18,0x00,0x00,0x00,0x01,
0xff,0xff,0xff,0xff,0xe0,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x18,0x00,0x00,0x00,0x01,
0xff,0xff,0xff,0xff,0xe0,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x18,0x00,0x0f,0xe0,0x11,
0xff,0xff,0xff,0xff,0xe0,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x18,0x00,0xff,0xfe,0x11,
0xe0,0x00,0x00,0x01,0xe0,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x18,0x03,0xff,0x1f,0xf1,
0xe0,0x00,0x00,0x01,0xe0,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x18,0x07,0xf8,0x03,0xf1,
0xe1,0xf8,0x07,0xe1,0xe0,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x18,0x0f,0xe0,0x01,0xf1,
0xe3,0xf8,0x07,0xf1,0xe0,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x18,0x1f,0xc0,0x00,0x71,
0xe7,0xf8,0x07,0xf9,0xe0,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x18,0x3f,0x80,0x00,0x71,
0xef,0xf8,0x07,0xfd,0xe0,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x18,0x7f,0x00,0x00,0x31,
0xef,0xf8,0x07,0xfd,0xe0,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x18,0xff,0x00,0x00,0x31,
0xef,0xf8,0x07,0xff,0xe0,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x18,0xff,0x00,0x00,0x11,
0xff,0xf8,0x07,0xff,0xe0,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x19,0xfe,0x00,0x00,0x11,
0xff,0xf8,0x07,0xff,0xe0,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x19,0xfe,0x00,0x00,0x01,
0xff,0xf8,0x07,0xff,0xe0,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1b,0xfe,0x00,0x00,0x01,
0xff,0xf8,0x07,0xff,0xe0,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1b,0xfe,0x00,0x00,0x01,
0xff,0xf8,0x07,0xff,0xe0,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1b,0xfc,0x00,0x00,0x01,
0xff,0xf8,0x07,0xff,0xe0,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1b,0xfc,0x00,0x00,0x01,
0xff,0xf8,0x07,0xff,0xe0,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1b,0xfc,0x00,0x00,0x01,
0xff,0xf8,0x07,0xff,0xe0,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1b,0xfc,0x00,0x00,0x01,
0xff,0xf8,0x07,0xff,0xe0,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1b,0xfc,0x00,0x00,0x01,
0xff,0xf8,0x07,0xff,0xe0,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1b,0xfe,0x00,0x00,0x01,
0xff,0xf8,0x07,0xff,0xe0,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1b,0xfe,0x00,0x00,0x01,
0xff,0xf8,0x07,0xff,0xe0,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1b,0xfe,0x00,0x00,0x01,
0xff,0xf8,0x07,0xff,0xe0,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1b,0xfe,0x00,0x00,0x01,
0xff,0xf8,0x07,0xff,0xe0,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x19,0xfe,0x00,0x00,0x01,
0xff,0xf8,0x07,0xff,0xe0,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x19,0xfe,0x00,0x00,0x01,
0xff,0xf8,0x07,0xff,0xe0,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x18,0xff,0x00,0x00,0x01,
0xff,0xf8,0x07,0xff,0xe0,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x18,0xff,0x00,0x00,0x11,
0xff,0xf8,0x07,0xff,0xe0,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x18,0x7f,0x80,0x00,0x31,
0xff,0xf8,0x07,0xff,0xe0,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x18,0x3f,0x80,0x00,0x71,
0xff,0xf8,0x07,0xff,0xe0,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x18,0x1f,0xc0,0x00,0xe1,
0xff,0xf8,0x07,0xff,0xe0,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x18,0x0f,0xf0,0x03,0x81,
0xff,0xf8,0x07,0xff,0xe0,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x18,0x07,0xfe,0x3f,0x01,
0xff,0xf0,0x03,0xff,0xe0,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x18,0x01,0xff,0xfc,0x01,
0xff,0x80,0x00,0x7f,0xe0,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x18,0x00,0x1f,0xe0,0x01,
0xff,0xff,0xff,0xff,0xe0,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x18,0x00,0x00,0x00,0x01,
0xff,0xff,0xff,0xff,0xe0,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1f,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xe0,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1f,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xe0,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x60,0x03,0x80,0x00,
0x00,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x7c,0x00,0x00,0x60,0x06,0x00,0x00,
0x1f,0xef,0x1e,0x1e,0x03,0x1e,0x1e,0x00,0x00,0x00,
0x00,0x00,0x00,0x66,0x00,0x00,0x60,0x06,0x00,0x00,
0x03,0x06,0x33,0x33,0x06,0x33,0x33,0x04,0x00,0x00,
0x00,0x00,0x3c,0x66,0x3c,0x3c,0x66,0x0f,0x9e,0x36,
0x03,0x06,0x33,0x33,0x06,0x33,0x03,0x04,0x00,0x00,
0x00,0x00,0x66,0x7c,0x66,0x66,0x6c,0x06,0x33,0x3e,
0x03,0x06,0x1e,0x33,0x0c,0x33,0x06,0x04,0x00,0x00,
0x00,0x00,0x7e,0x66,0x66,0x66,0x78,0x06,0x33,0x30,
0x03,0x06,0x33,0x1f,0x0c,0x1f,0x0c,0x3f,0x80,0x00,
0x00,0x00,0x60,0x66,0x66,0x66,0x78,0x06,0x33,0x30,
0x03,0x06,0x33,0x03,0x0c,0x03,0x18,0x04,0x00,0x00,
0x00,0x00,0x62,0x66,0x66,0x66,0x6c,0x06,0x33,0x30,
0x03,0x06,0x33,0x06,0x18,0x06,0x30,0x04,0x00,0x00,
0x00,0x00,0x3c,0x7c,0x3c,0x3c,0x66,0x06,0x1e,0x30,
0x03,0x0f,0x1e,0x1c,0x18,0x1c,0x3f,0x04,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x30,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x30,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};


//=============================================================================
// delete temporary files
//=============================================================================
void CleanUp(int keepfiles) {
    int i;
    char tmpname[100];

    if (!keepfiles) {
        for (i=0;i<1000;i++) {
            sprintf(tmpname,"tictp%03d.txt",i); // splitted input files
            if (remove(tmpname)) break;
        }
        for (i=0;i<1000;i++) {
            sprintf(tmpname,"tictp%03d.tmp",i); // converted input files
            if (remove(tmpname)) break;
        }

        remove("tict.txt");
        remove("tictbook.cfg");

        if (tmptitlename == titlename) remove(tmptitlename);
    }
}


//=============================================================================
// copies files
//=============================================================================
int FileCopy(char* source, char* dest) {
    FILE* fp_src = fopen(source,"rb");
    FILE* fp_dest;
    int   c;

    if (!fp_src) return 0;

    if (!(fp_dest = fopen(dest,"wb"))) {
        fclose(fp_src);
        return 0;
    }

    do {
        c = fgetc(fp_src);
        if (c == EOF) break;
        fputc(c,fp_dest);
    }
    while (1);

    fclose(fp_src);
    fclose(fp_dest);

    return 1;
}

#define EBK_NOTSPECIFIED  0
#define EBK_TI89VERSION   1
#define EBK_TI92PVERSION  2

//=============================================================================
// a main is a main is main .....
//=============================================================================
int main(int argc, char* argv[]) {
    char* infilename  = 0;
    char* varname     = 0;
    char* description = 0;
    FILE* fp;
    char  buffer[1024];
    char  tmpname[1024];
    int   i;
    int   count;
    int   keep = 0;
    char* ttdos2ebk_params[3];
    //char* ttpack_params[4];
    char* ttarchive_params[8];
    //char* ttbin2bin_params[2];
    int   ebk_type = EBK_NOTSPECIFIED;
    int   verbose  = 0;
    int   quiet    = 0;

    ttdos2ebk_params[0] = "";
    ttdos2ebk_params[1] = tmpname;
    ttdos2ebk_params[2] = buffer;

    if (argc < 4) {
        PrintUsage();
        return 1;
    }

    // parse arguments
    for (i=1; i<argc; i++) {
        if      (!strcmp(argv[i], "-keepfiles")) keep       = 1;
        else if (!strcmp(argv[i], "-89"))        ebk_type   |= EBK_TI89VERSION;
        else if (!strcmp(argv[i], "-92"))        ebk_type   |= EBK_TI92PVERSION;
        else if (!strcmp(argv[i], "-v"))         quiet=0,verbose=1;
        else if (!strcmp(argv[i], "-quiet"))     quiet=1,verbose=0;
        else if (!strcmp(argv[i], "-t")) {
            if (i == argc-1) {
                PrintUsage();
                return 1;
            }
            i++;
            titlename = argv[i];
        }
        else if (!infilename)  infilename  = argv[i];
        else if (!varname)     varname     = argv[i];
        else if (!description) description = argv[i];
        else {
            PrintUsage();
            return 1;
        }
    }


    if (!infilename || !varname || !description) {
        PrintUsage();
        return 1;
    }

    CleanUp(0); // delete old files to make sure we won't process them

    if (ebk_type == EBK_NOTSPECIFIED) {
        ebk_type = EBK_TI89VERSION | EBK_TI92PVERSION;
    }

    if (!quiet) PRINT_ID("TTEbkGen");

    //--------------------------------------------
    // if no title file is given, generate default
    // title file
    //--------------------------------------------
    if (!titlename) {
        FILE *f;
        titlename = tmptitlename;

        if (!(f = fopen(titlename,"wb"))) {
            fprintf(stderr,"ERROR: cannot generate temporary titlefile %s\n",titlename);
            return 1;
        }
        if (fwrite(default_image,4000,1,f) != 1) {
            fprintf(stderr,"ERROR: cannot generate temporary titlefile %s\n",titlename);
            return 1;
        }
        fclose(f);
    }


    //--------------------------------------------------------
    // check if files exists and if title file is of size 4000
    //--------------------------------------------------------
    if (!(fp=fopen(infilename,"r"))) {
        fprintf(stderr,"ERROR: infile %s doesn't exists\n",infilename);
        CleanUp(keep);
        return 1;
    }
    fclose(fp);

    if (!(fp=fopen(titlename,"rb"))) {
        fprintf(stderr,"ERROR: titlefile %s doesn't exists\n",titlename);
        CleanUp(keep);
        return 1;
    }
    fseek(fp,0,SEEK_END);
    if (ftell(fp) != 4000) {
        fprintf(stderr,"ERROR: titlefile %s is not of size 4000\n",titlename);
        fclose(fp);
        CleanUp(keep);
        return 1;
    }
    fclose(fp);


    //-------------------------------------------------------------------------
    // copy inputfile to "tict.txt" ...
    //-------------------------------------------------------------------------
    if (verbose) fprintf(stdout,"copying inputfile to tict.txt ...\n");
    if (!FileCopy(infilename,"tict.txt")) {
        CleanUp(keep);
        return 1;
    }

    //-------------------------------------------------------------------------
    // split "tict.txt" into "tictp000.txt", "tictp001.txt" ...
    //-------------------------------------------------------------------------
    if (verbose) fprintf(stdout,"splitting tict.txt ...\n");
    {
        char* parameters[] = {"","-s","<--split-->","-t","tict.txt"};
        if (TTSplit(5,parameters)) {
            CleanUp(keep);
            return 1;
        }
    }

    //--------------------------------------------------------
    // converting files "tictp000.txt" into "tictp000.ebk"
    //--------------------------------------------------------
    if (verbose) fprintf(stdout,"converting parts ...\n");

    for (i=0;i<1000;i++) {
        sprintf(tmpname,"tictp%03d.txt",i);
        fp = fopen(tmpname,"r");
        if (!fp) break;
        fclose(fp);

        sprintf(buffer,"tictp%03d.tmp",i);
        if (TTDos2Ebk(3,ttdos2ebk_params)) {
            CleanUp(keep);
            return 1;
        }
    }

    count = i;
    if (count == 0) {
        fprintf(stderr,"ERROR: no data to process further\n");
        CleanUp(keep);
        return 1;
    }

    //--------------------------------------------------------
    // generating ebook
    //--------------------------------------------------------
    if (verbose) fprintf(stdout,"generatic ebook .....");

    fp = fopen("tictbook.cfg","w");

    if (!fp) {
        fprintf(stderr,"ERROR: cannot generate tictbook.cfg\n");
        CleanUp(keep);
        return 1;
    }

    fprintf(fp,"#%s\n",description);
    fprintf(fp,"%s title 0 0 compress\n",titlename);

    for (i=0;i<count;i++) fprintf(fp,"tictp%03d.tmp p%03d 0 0 compress\n",i,i);
    fclose(fp);

    if (ebk_type & EBK_TI89VERSION) {
        ttarchive_params[0] = "";
        ttarchive_params[1] = (verbose) ? "-v" : "-quiet";
        ttarchive_params[2] = "-89";
        ttarchive_params[3] = "-e";
        ttarchive_params[4] = "ebk";
        ttarchive_params[5] = "tictbook.cfg";
        ttarchive_params[6] = varname;
        ttarchive_params[7] = "tictebks";

        if (TTArchive(8,ttarchive_params)) {
            fprintf(stderr,"ERROR: eBook %s.89y generation failed.\n",varname);
            CleanUp(keep);
            return 1;
        }

        if (!quiet) fprintf(stdout,"eBook %s.89y sucessfully generated.\n",varname);
    }
    if (ebk_type & EBK_TI92PVERSION) {
        ttarchive_params[0] = "";
        ttarchive_params[1] = (verbose) ? "-v" : "-quiet";
        ttarchive_params[2] = "-92";
        ttarchive_params[3] = "-e";
        ttarchive_params[4] = "ebk";
        ttarchive_params[5] = "tictbook.cfg";
        ttarchive_params[6] = varname;
        ttarchive_params[7] = "tictebks";

        if (TTArchive(8,ttarchive_params)) {
            fprintf(stderr,"ERROR: eBook %s.9xy generation failed.\n",varname);
            CleanUp(keep);
            return 1;
        }

        if (!quiet) fprintf(stdout,"eBook %s.9xy sucessfully generated.\n",varname);
    }

    CleanUp(keep);
    return 0;
}

//#############################################################################
//###################### NO MORE FAKES BEYOND THIS LINE #######################
//#############################################################################
//
//=============================================================================
// Revision History
//=============================================================================
//
// Revision 1.10 2009/01/25           Lionel Debroux
// Changes by Romain Li�vin and/or me for 64-bit compatibility.
// Adapt to new version display (revtools.h).
//
// Revision 1.9  2002/03/14 11:27:19  tnussb
// (1) new commandline parameter -89 (generate only TI89 ebook)
// (2) new commandline parameter -92 (generate only TI92p ebook)
// (3) new commandline parameter -v (verbose)
// (4) new commandline parameter -quiet (suppress standard messages)
// (5) TTBin2Bin usage replaced by second call to TTArchive
// (6) TTPack usage removed (is now handled by TTArchive itself)
//
// Revision 1.8  2002/03/04 14:35:02  tnussb
// No use of external TIGCC Tools anymore, but using now compiled-in versions.
// This way this tool can be distributed by its own without the need of a
// TIGCC Tools Suite installation.
//
// Revision 1.7  2002/02/07 09:49:37  tnussb
// all local includes changed, because header files are now located in pctools folder
//
// Revision 1.6  2000/11/28 00:08:29  Thomas Nussbaumer
// using now USAGE_OUT stream for usage info
//
// Revision 1.5  2000/10/18 23:21:35  Thomas Nussbaumer
// filecopy is done now by own routine instead of a system call to support
// linux
//
// Revision 1.4  2000/10/01 15:06:13  Thomas Nussbaumer
// using now default title image if none is specified
//
// Revision 1.3  2000/08/26 19:07:09  Thomas Nussbaumer
// using now inline split tag (much easier to handle than an external splitfile)
//
// Revision 1.2  2000/08/25 18:10:33  Thomas Nussbaumer
// some corrections to usage text
//
// Revision 1.1  2000/08/25 18:06:30  Thomas Nussbaumer
// initial version
//
//
//