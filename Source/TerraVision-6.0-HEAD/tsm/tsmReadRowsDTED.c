/*
 * Name: tsmReadRowsDTED.c
 *
 * Description: 
 *   Import functions for supporting DTED Level 0 elevation files.
 *
 * Function List:
 *
 * Revision History:
 *   Created Jul 2000 by Yong Tze Chi <chi@ai.sri.com>
 *
 *   $Id: tsmReadRowsDTED.c,v 1.4 2000/08/28 22:16:04 reddy Exp $
 *
 * License:
 *   The contents of this file are subject to the Open Source Apache
 *   Software License Version 1.1 (the "License"); you may not use
 *   this file except in compliance with the License. You may obtain a
 *   copy of the License at http://www.tsmapi/license.shtml.
 *
 *   Software distributed under the License is distributed on an "AS IS"
 *   basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
 *   License for the specific language governing rights and limitations
 *   under the License.
 *
 *   Portions are Copyright (c) SRI International, 1998-2000.
 *
 */

#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "tsmInternal.h"
#include "tsmReadRowsDTED.h"

#ifndef DBL_MAX
#define DBL_MAX 1.7976931348623158e+308
#endif

#ifndef CHECK_NOT_NULL0
#define CHECK_NOT_NULL0(this) if ((this)==NULL) { tsmError(funcName, "NULL instance"); return; }
#endif

#ifndef CHECK_NOT_NULL
#define CHECK_NOT_NULL(this, rv) if ((this)==NULL) { tsmError(funcName, "NULL instance"); return (rv); }
#endif

static int tsmReadRowsDTED_readInt(TsmReadRowsDTED *this, unsigned int length);
static double tsmReadRowsDTED_readDouble(TsmReadRowsDTED *this, unsigned int length);
static long tsmReadRowsDTED_readFixedBinary(TsmReadRowsDTED *this, unsigned int length);
static int tsmReadRowsDTED_readHeader(TsmReadRowsDTED *this);
static int tsmReadRowsDTED_readRecord(TsmReadRowsDTED *this, void *buffer, int blockIndex, int startRow, int numberOfRows);

/* +--------------------------------------------------------------------+ *
 * | Name: tsmReadRowsDTED_new                                          | *
 * | Description:                                                       | *
 * |   Creates a new instance of TsmReadRowsDTED structure. It will     | *
 * |   read the DTED file header and fill up the fields in the          | *
 * |   structure.                                                       | *
 * | Parameters:                                                        | *
 * |   char *filename    Filename of the DTED file.                     | *
 * | Return value:                                                      | *
 * |   Pointer to the newly created TsmReadRowsDTEd structure. Returns  | *
 * |   NULL if anything goes wrong.                                     | *
 * +--------------------------------------------------------------------+ */
TsmReadRowsDTED *tsmReadRowsDTED_new(char const *filename) {
  static const char *funcName = "tsmReadRowsDTED_new";
  TsmReadRowsDTED *this;

  if ((this = tsmMemMalloc(sizeof(TsmReadRowsDTED))) == NULL) {
    tsmError(funcName, "Failed to create tsmReadRowsDTED instance.");
    return NULL;
  }

  if ((this->fin = fopen(filename, "rb")) == NULL) {
    tsmError(funcName, "Failed to open file named %s.\n", filename);
    tsmReadRowsDTED_destroy(&this);
    return NULL;
  }

  if (!tsmReadRowsDTED_readHeader(this)) {
    tsmError(funcName, "Failed to read DTED header.\n");
    tsmReadRowsDTED_destroy(&this);
    return NULL;
  }

  return this;
}

/* +--------------------------------------------------------------------+ *
 * | Name: tsmReadRowsDTED_destroy                                      | *
 * | Description:                                                       | *
 * |   Destroys or deinitializes the TsmReadRowsDTED structure,         | *
 * |   essentially meaning closing the DTED file.                       | *
 * | Parameters:                                                        | *
 * |   TsmReadRowsDTED **this    Reference to the instance pointer.     | *
 * | Return value:                                                      | *
 * |   None.                                                            | *
 * +--------------------------------------------------------------------+ */
void tsmReadRowsDTED_destroy(TsmReadRowsDTED **this) {
  if (this!=NULL && *this!=NULL) {
    if ((*this)->fin != NULL) fclose((*this)->fin);
    tsmMemFree(*this);
    *this = NULL;
  }
}

/* +--------------------------------------------------------------------+ *
 * | Name: tsmReadRowsDTED_readInt                                      | *
 * | Description:                                                       | *
 * |   Reads an ASCII coded integer value. Caller has to specify how    | *
 * |   long the ASCII integer spans.                                    | *
 * | Parameters:                                                        | *
 * |   TsmReadRowsDTED *this    TsmReadRowsDTED instance.               | *
 * |   unsigned int length      Length of the ASCII integer value.      | *
 * | Return value:                                                      | *
 * |   Value read converted into int type. Returns INT_MIN if there's   | *
 * |   an error.                                                        | *
 * +--------------------------------------------------------------------+ */
static int tsmReadRowsDTED_readInt(TsmReadRowsDTED *this, unsigned int length) {
  static const char *funcName = "tsmReadRowsDTED_readInt";
  static char buffer[128];

  CHECK_NOT_NULL(this, INT_MIN);

  if (fread(buffer, sizeof(char), length, this->fin)!=length) {
    tsmError(funcName, "Error encountered while reading file.\n");
    return INT_MIN;
  }

  buffer[length] = 0;
  return atoi(buffer);
}

/* +--------------------------------------------------------------------+ *
 * | Name: tsmReadRowsDTED_readDouble                                   | *
 * | Description:                                                       | *
 * |   Reads an ASCII coded floating-point value from the DTED file,    | *
 * |   and converts to type double. Caller has to specify how long the  | *
 * |   ASCII floating-point value spans.                                | *
 * | Parameters:                                                        | *
 * |   TsmReadRowsDTED *this    TsmReadRowsDTED instance.               | *
 * |   unsigned int length      Length of the ASCII floating-point      | *
 * |                            value.                                  | *
 * | Return value:                                                      | *
 * |   Value read converted into double type. Returns -DBL_MAX if       | *
 * |   there's an error.                                                | *
 * +--------------------------------------------------------------------+ */
static double tsmReadRowsDTED_readDouble(TsmReadRowsDTED *this, unsigned int length) {
  static const char *funcName = "tsmReadRowsDTED_readDouble";
  static char buffer[128];

  CHECK_NOT_NULL(this, -DBL_MAX);

  if (fread(buffer, sizeof(char), length, this->fin)!=length) {
    tsmError(funcName, "Error encountered while reading file.\n");
    return -DBL_MAX;
  }

  buffer[length] = 0;
  return atof(buffer);
}

/* +--------------------------------------------------------------------+ *
 * | Name: tsmReadRowsDTED_readFixedBinary                              | *
 * | Description:                                                       | *
 * |   Reads an "Fixed Binary" format value from the DTED file. A fixed | *
 * |   binary format is essentially a sign-magnitude binary number.     | *
 * |   The most significant bit is used as the sign bit, and the rest   | *
 * |   represents the magnitude of the value. Magnitude bits are not    | *
 * |   complemented when a negative value is represented.               | *
 * |                                                                    | *
 * |   The function lets the caller specify the length of the fixed     | *
 * |   binary value. In order to maintain compability of code across    | *
 * |   little-endian and big-endian machines, the function bit-shifts   | *
 * |   the bytes read from file in order to convert to a long value,    | *
 * |   instead of simply returning the bytes read from the file.        | *
 * | Parameters:                                                        | *
 * |   TsmReadRowsDTED *this   TsmReadRowsDTED instance.                | *
 * |   unsigned int length     Length of the fixed binary value.        | *
 * | Return value:                                                      | *
 * |   A long value of the fixed binary value read from file. Returns   | *
 * |   LONG_MIN in case of any error.                                   | *
 * +--------------------------------------------------------------------+ */
static long tsmReadRowsDTED_readFixedBinary(TsmReadRowsDTED *this, unsigned int length) {
  static const char *funcName = "tsmReadRowsDTED_readFixedBinary";
  static unsigned char buffer[64];
  static long temp;
  static long sign;
  static int i;

  CHECK_NOT_NULL(this, LONG_MIN);

  if (fread(buffer, sizeof(unsigned char), length, this->fin) != length) {
    tsmError(funcName, "Error encountered while reading file.\n");
    return LONG_MIN;
  }

  sign = (buffer[0] & 0x80) ? -1 : 1;
  buffer[0] = buffer[0] & 0x70;

  for (temp=0, i=0; i<length; temp = (temp << 8) | buffer[i++]);

  return temp * sign;
}

/* +--------------------------------------------------------------------+ *
 * | Name: tsmReadRowsDTED_readHeader                                   | *
 * | Description:                                                       | *
 * |   Reads the header information from the DTED file, and fills in    | *
 * |   all the fields in the TsmReadRowsDTED structure.                 | *
 * | Parameters:                                                        | *
 * |   TsmReadRowsDTED *this    TsmReadRowsDTED instance.               | *
 * | Return value:                                                      | *
 * |   TRUE if everything's ok, FALSE otherwise.                        | *
 * +--------------------------------------------------------------------+ */
static int tsmReadRowsDTED_readHeader(TsmReadRowsDTED *this) {
  static const char *funcName = "tsmReadRowsDTED_readHeader";
  char buffer[512];

  CHECK_NOT_NULL(this, FALSE);

  /* Assuming DTED file to start with the User Header Label(UHL) section */
  FREAD(buffer, sizeof(char), 3, this->fin)
  if (buffer[0]!='U' || buffer[1]!='H' || buffer[2]!='L') {
    tsmError(funcName, "Unrecognized format.\n");
    return FALSE;
  }

  /* Dummy */
  FREAD(buffer, 1, 1, this->fin);

  /* Longitude of origin */
  this->blockOrigin[0] = (tsmReadRowsDTED_readInt(this, 3) +
    tsmReadRowsDTED_readInt(this, 2) * MINTODEG +
    tsmReadRowsDTED_readInt(this, 2) * SECTODEG) *
    (fgetc(this->fin)=='E' ? 1 : -1);

  /* Latitude of origin */
  this->blockOrigin[1] = (tsmReadRowsDTED_readInt(this, 3) +
    tsmReadRowsDTED_readInt(this, 2) * MINTODEG +
    tsmReadRowsDTED_readInt(this, 2) * SECTODEG) *
    (fgetc(this->fin)=='N' ? 1 : -1);

  /* Longitude and latitude data interval in seconds, converted
     into degree before storing in the DTED structure */
  this->blockSpacing[0] = tsmReadRowsDTED_readInt(this, 4) * SECTODEG;
  this->blockSpacing[1] = tsmReadRowsDTED_readInt(this, 4) * SECTODEG;

  /* Absolute Vertival Accuracy in meters */
  this->absVertAccuracy = tsmReadRowsDTED_readInt(this, 4);
  
  /* Security code */
  FREAD(&this->security, sizeof(char), 3, this->fin);

  /* Skipping "unique reference number", irrelevant in THIS program. */
  fseek(this->fin, 12, SEEK_CUR);

  /* Count of the number of longitude and latitude points */
  this->dimension[0] = tsmReadRowsDTED_readInt(this, 4);
  this->dimension[1] = tsmReadRowsDTED_readInt(this, 4);

  /* Skipping 25 bytes of unused data */
  fseek(this->fin, 25, SEEK_CUR);

  /* Data Set Identification (DSI) section */
  FREAD(buffer, 1, 3, this->fin);
  if (buffer[0]!='D' || buffer[1]!='S' || buffer[2]!='I') {
    printf("DTED file appearead to be corrupted.\n");
    return FALSE;
  }

  /* Skipping 56 bytes of irrelevant information */
  fseek(this->fin, 56, SEEK_CUR);

  /* DTED0, DTED1, or DTED2 */
  FREAD(buffer, 1, 5, this->fin);
  if (strncmp(buffer, "DTED", 4)!=0 || buffer[4]<'0' || buffer[4]>'2') {
    tsmError(funcName, "DTED file appearead to be corrupted.\n");
    return FALSE;
  }
  this->level = (DTEDLevel) (buffer[4] - '0');

  /* Skipping 23 bytes of irrelevant information */
  fseek(this->fin, 23, SEEK_CUR);

  
  this->editionNumber = tsmReadRowsDTED_readInt(this, 2);          /* Data Edition Number */
  this->matchVersion = fgetc(this->fin);                /* Match/Merge Version, A-Z */
  FREAD(this->maintenanceDate, 1, 4, this->fin);        /* Maintenance Date, YYMM */
  FREAD(this->matchDate, 1, 4, this->fin);              /* Match/Merge Date, YYMM */
  FREAD(this->maintenenceDesc, 1, 4, this->fin);        /* Maintenance Description */
  FREAD(this->producerCode, 1, 8, this->fin);           /* Producer Code, CCAAABBB */
  fseek(this->fin, 16, SEEK_CUR);                       /* Reserved bytes */
  FREAD(this->productSpec, 1, 9, this->fin);            /* Product Specification */
  this->productSpecAmendment = tsmReadRowsDTED_readInt(this, 2);   /* Product Specification
                                                           Amendment Number */
  FREAD(this->productSpecDate, 1, 4, this->fin);        /* Date of Product Specification, YYMM */
  FREAD(this->verticalDatum, 1, 3, this->fin);          /* Vertical Datum, MSL(Mean Sea Level) */
  FREAD(this->horizontalDatum, 1, 5, this->fin);        /* Horizontal Datum Code,
                                                           WGS84 */
  fseek(this->fin, 10, SEEK_CUR);                       /* Skipping */
  FREAD(this->compileDate, 1, 4, this->fin);            /* Compilation Date, YYMM */
  fseek(this->fin, 22, SEEK_CUR);                       /* Skipping */

  /* Latitude and longitude of origin of data */
  this->coordOrg[1] = (tsmReadRowsDTED_readInt(this, 2) +
    tsmReadRowsDTED_readInt(this, 2) * MINTODEG +
    tsmReadRowsDTED_readDouble(this, 4) * SECTODEG) *
    (fgetc(this->fin)=='E' ? 1 : -1);
  
  this->coordOrg[0] = (tsmReadRowsDTED_readInt(this, 3) +
    tsmReadRowsDTED_readInt(this, 2) * MINTODEG +
    tsmReadRowsDTED_readDouble(this, 4) * SECTODEG) *
    (fgetc(this->fin)=='N' ? 1 : -1);

  /* Latitude and longitude of SW corner of data */
  this->coordSW[1] = (tsmReadRowsDTED_readInt(this, 2) +
    tsmReadRowsDTED_readInt(this, 2) * MINTODEG +
    tsmReadRowsDTED_readInt(this, 2) * SECTODEG) *
    (fgetc(this->fin)=='E' ? 1 : -1);
  
  this->coordSW[0] = (tsmReadRowsDTED_readInt(this, 3) +
    tsmReadRowsDTED_readInt(this, 2) * MINTODEG +
    tsmReadRowsDTED_readInt(this, 2) * SECTODEG) *
    (fgetc(this->fin)=='N' ? 1 : -1);

  /* Latitude and logitude of NW corner of data */
  this->coordNW[1] = (tsmReadRowsDTED_readInt(this, 2) +
    tsmReadRowsDTED_readInt(this, 2) * MINTODEG +
    tsmReadRowsDTED_readInt(this, 2) * SECTODEG) *
    (fgetc(this->fin)=='E' ? 1 : -1);
  
  this->coordNW[0] = (tsmReadRowsDTED_readInt(this, 3) +
    tsmReadRowsDTED_readInt(this, 2) * MINTODEG +
    tsmReadRowsDTED_readInt(this, 2) * SECTODEG) *
    (fgetc(this->fin)=='N' ? 1 : -1);

  /* Latitude and logitude of NE corner of data */
  this->coordNE[1] = (tsmReadRowsDTED_readInt(this, 2) +
    tsmReadRowsDTED_readInt(this, 2) * MINTODEG +
    tsmReadRowsDTED_readInt(this, 2) * SECTODEG) *
    (fgetc(this->fin)=='E' ? 1 : -1);
  
  this->coordNE[0] = (tsmReadRowsDTED_readInt(this, 3) +
    tsmReadRowsDTED_readInt(this, 2) * MINTODEG +
    tsmReadRowsDTED_readInt(this, 2) * SECTODEG) *
    (fgetc(this->fin)=='N' ? 1 : -1);

  /* Latitude and logitude of SE corner of data */
  this->coordSE[1] = (tsmReadRowsDTED_readInt(this, 2) +
    tsmReadRowsDTED_readInt(this, 2) * MINTODEG +
    tsmReadRowsDTED_readInt(this, 2) * SECTODEG) *
    (fgetc(this->fin)=='E' ? 1 : -1);
  
  this->coordSE[0] = (tsmReadRowsDTED_readInt(this, 3) +
    tsmReadRowsDTED_readInt(this, 2) * MINTODEG +
    tsmReadRowsDTED_readInt(this, 2) * SECTODEG) *
    (fgetc(this->fin)=='N' ? 1 : -1);

  /* Clockwise orientation angle of data with respect to true
     North.  Will usually be all zeros for DTED. */
  this->orientationAngle = tsmReadRowsDTED_readInt(this, 3) +
    tsmReadRowsDTED_readInt(this, 2) * MINTODEG +
    tsmReadRowsDTED_readDouble(this, 4) * SECTODEG;

  /* Latitude and logitude interval in tenths of seconds between
     rows of elevation values. Converted to degree before storing
     into DTED structure. */
  this->pointSpacing[1] = (tsmReadRowsDTED_readInt(this, 3) +
    tsmReadRowsDTED_readInt(this, 1) * 0.1) * SECTODEG;

  this->pointSpacing[0] = (tsmReadRowsDTED_readInt(this, 3) +
    tsmReadRowsDTED_readInt(this, 1) * 0.1) * SECTODEG;

  /* Number of valid latitude and logitude points */
  this->numValidPoint[1] = tsmReadRowsDTED_readInt(this, 4);
  this->numValidPoint[0] = tsmReadRowsDTED_readInt(this, 4);

  /* Skipping irrelevant information */
  fseek(this->fin, 359, SEEK_CUR);

  /* Accuracy Desription (ACC) Record */
  /* Just skip through */
  fseek(this->fin, 2700, SEEK_CUR);

  this->dataOffset = ftell(this->fin);

  /* Done reading header */
  return TRUE;
}

/* +--------------------------------------------------------------------+ *
 * | Name: tsmReadRowsDTED_read                                         | *
 * | Description:                                                       | *
 * |   Reads a specified number of rows of data from the DTED file.     | *
 * |   This function is to be hooked as a callback to the               | *
 * |   tsmDownSample_inmemory function for TerraVision dataset creation.| *
 * |                                                                    | *
 * |   Each DTED is assumed to contain data withing a 1-degree cell,    | *
 * |   and assumed to fill up the entire region defined in the header.  | *
 * |   Elevation data are divided into columns, or records according to | *
 * |   the spec. Each record has a starting sentinel value of 170, a    | *
 * |   record index, a longitude index, and a latitude index for the    | *
 * |   first elevation data. Normally the latitude index will equal to  | *
 * |   zero for full cells. Following these fields are the elevation    | *
 * |   data in "fixed binary" format, followed by a 4-byte checksum.    | *
 * |   The checksum is constructed by summing all the fields in each    | *
 * |   record starting from the sentinel to the last elevation data.    | *
 * |   The checksum is ignored in this module.                          | *
 * | Parameters:                                                        | *
 * |   void *buffer         Pointer to the data buffer.                 | *
 * |   int startRow         Starting row number.                        | *
 * |   int numberOfRows     Number of rows to read.                     | *
 * |   int numberOfBytes    Number of bytes in a row.                   | *
 * |   void *closure        Pointer to the TsmReadRowsDTED instance.    | *
 * | Return value:                                                      | *
 * |   TRUE if everything's ok, and FALSE otherwise.                    | *
 * +--------------------------------------------------------------------+ */
int tsmReadRowsDTED_read(void *buffer, int startRow, int numberOfRows, int numberOfBytes, void *closure) {
  static const char *funcName = "tsmReadRowsDTED_read";
  int i;
  TsmReadRowsDTED *this = (TsmReadRowsDTED *) closure;

  CHECK_NOT_NULL(this, FALSE);
  CHECK_NOT_NULL(buffer, FALSE);

  for (i=0; i<this->numValidPoint[0]; i++) {
    if (!tsmReadRowsDTED_readRecord(this, buffer, i, startRow, numberOfRows)) {
      return FALSE;
    }
  }

  return TRUE;
}

/* +--------------------------------------------------------------------+ *
 * | Name: tsmReadRowsDTED_readRecord                                   | *
 * | Description:                                                       | *
 * |   Reads a record (column) and fills the buffer with the elevations | *
 * |   read. The buffer is assumed to be large enough for numberOfRows  | *
 * |   x this->dimension[0] pixels. This function will read in a column | *
 * |   of elevations and then fill the data in the correct places in    | *
 * |   the buffer.                                                      | *
 * | Parameters:                                                        | *
 * |   TsmReadRowsDTED *this    TsmReadRowsDTED instance.               | *
 * |   void *buffer             Elevation data buffer.                  | *
 * |   int blockIndex           Which block (or record) are we reading. | *
 * |   int startRow             Starting row number.                    | *
 * |   int numberOfRows         Number of rows to read.                 | *
 * | Return value:                                                      | *
 * |   TRUE if successful, FALSE otherwise.                             | *
 * +--------------------------------------------------------------------+ */
static int tsmReadRowsDTED_readRecord(TsmReadRowsDTED *this, void *buffer, int blockIndex, int startRow, int numberOfRows) {
  static const char *funcName = "tsmReadRowsDTED_readRecord";
  long offset;
  int longIndex;
  int latIndex;
  int i;
  INT2 *p;

  CHECK_NOT_NULL(this, FALSE);

  offset = this->dataOffset + blockIndex * (this->numValidPoint[1] * sizeof(INT2) + 12);
  if (ftell(this->fin) != offset) fseek(this->fin, offset, SEEK_SET);

  if (fgetc(this->fin) != 170) {
    tsmError(funcName, "DTED record appeared to be corrupted.\n");
    return FALSE;
  }

  if ((i = tsmReadRowsDTED_readFixedBinary(this, 3)) == LONG_MIN) return FALSE;
  if ((longIndex = tsmReadRowsDTED_readFixedBinary(this, 2)) == LONG_MIN) return FALSE;
  if ((latIndex = tsmReadRowsDTED_readFixedBinary(this, 2)) == LONG_MIN) return FALSE;

  offset = (startRow-latIndex) * sizeof(INT2);
  if (offset>0) fseek(this->fin, offset, SEEK_CUR);

  for (i=0, p=((INT2 *)buffer) + blockIndex;
       i<numberOfRows;
       i++, p+=this->dimension[0]) {
    *p = tsmReadRowsDTED_readFixedBinary(this, 2);
  }

  return TRUE;
}

/* +--------------------------------------------------------------------+ *
 * | Name: tsmParseCreateParams_DTED                                    | *
 * | Description:                                                       | *
 * |   Fills in as much information from the DTED file as possible into | *
 * |   the TsmTileParams and TsmTspecParams structures passed in.       | *
 * | Parameters:                                                        | *
 * |   char *filename           The DTED fiename.                       | *
 * |   TsmTileParams *tile      Pointer to TsmTileParams structure.     | *
 * |                            On return, this structure will have its | *
 * |                            width, height, comps, and bytespercomp  | *
 * |                            field set.                              | *
 * |   TsmTspecParams *tspec    Pointer to TsmTspecParams structure.    | *
 * |                            On return, this structure will have its | *
 * |                            width, height, comps, coord_sys, ll_lon | *
 * |                            ll_lat, and start_res fields set.       | *
 * | Return value:                                                      | *
 * |   TRUE if the fields are successfully set, FALSE if the DTED file  | *
 * |   couldn't be opened.                                              | *
 * +--------------------------------------------------------------------+ */
int tsmParseCreateParams_DTED(char *filename, TsmTileParams *tile, TsmTspecParams *tspec) {
  static const char *funcName = "tsmParseCreateParams_DTED";
  TsmReadRowsDTED *dted;

  if ((dted = tsmReadRowsDTED_new(filename)) == NULL) {
    tsmError(funcName, "Failed to initialize TsmTileParams and TsmTspecParams structures.");
    return FALSE;
  }

  tile->width = tspec->width = dted->dimension[0];
  tile->height = tspec->height = dted->dimension[1];
  tile->comps = tspec->comps = 1;
  tile->bytespercomp = sizeof(INT2);

  tspec->coord_sys = TSM_LATLONG_COORDSYS;
  tspec->ll_lon = dted->coordSW[0];
  tspec->ll_lat = dted->coordSW[1];
  tspec->start_res = dted->blockSpacing[0];

  tsmReadRowsDTED_destroy(&dted);

  return TRUE;
}
