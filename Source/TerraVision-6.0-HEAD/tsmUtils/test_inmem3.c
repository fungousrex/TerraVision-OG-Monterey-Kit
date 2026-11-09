/*
 * Filename: test_inmem3.c
 *
 * Description:
 *     This program was written to test the new tiling and down
 *     sampling routines in tsmApi 2.0.
 *
 *     This program was written for use with the tsmApi library from
 *     SRI International. For further information about this library,
 *     including downloads, documentation, and other examples, see:
 *
 *         http://www.tsmApi.com/
 *
 * Author:
 *     Yong Tze Chi, <chi@ai.sri.com> - August 2000.
 *
 * Revision Information:
 *
 *     $Id: test_inmem3.c,v 1.4 2000/09/26 17:56:17 reddy Exp $
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
 */

#if 0

#include <tsm/tsm.h>
#include <tsm/tsmInternal.h>
#include <tsm/tsmIntProts.h>
#include <tsm/tsmReadRowsPBM.h>
#include <tsm/tsmReadRowsGIF.h>
#include <tsm/tsmReadRowsJPEG.h>
#include <string.h>

#define TYPE_ERROR -1
#define TYPE_PBM  1
#define TYPE_GIF  2
#define TYPE_JPEG 3


static int
nlevels (int size, int tile_size)
{
  int n = 1;
  while (size > tile_size) {
    ++n;
    size = (size+1)/2;
  }
  return n;
}

typedef struct {
  FILE **fout;
  int **imageSizes;
  int levels;
  int imageWidth, imageHeight;
} RowClosure;

RowClosure *RowClosure_new(int width, int height, int levels);
void RowClosure_destroy(RowClosure **this);


static int
read_rows (void *vbuf, int row, int n, int nbytes, void *closure)
{
  RowClosure *rowClosure = closure;
  char *buf = vbuf;

  printf ("L0 Read [%d] %d rows %dB/row\n", row, n, nbytes);
  memset (buf, 0x55, n*nbytes);

  return 1;
}

static int
write_rows (int level,
	    void const *vbuf, int row, int n, int nbytes,
	    void *closure)
{
  RowClosure *rowClosure = closure;
  int i, j;
  char *buffer = (char*) vbuf;
  float r, g, b;
  long offset;
  int zero;

  /* printf("Level %d, Row: %d, Rows: %d, Bytes: %d\n", level, row, n, nbytes); */

  printf ("L%d write(%p, %d, %d, %d) [%d]\n", 
	  level, vbuf, row, n, nbytes, n*nbytes);
#if 1
  printf ("fwrite(%p, %d, %d, %p)\n",
	  buffer, sizeof(unsigned char) * 3,
	  rowClosure->imageSizes[level][0] * n,
	  rowClosure->fout[level]);
  fwrite(buffer, sizeof(unsigned char) * 3, rowClosure->imageSizes[level][0] * n, rowClosure->fout[level]);
#endif

  /*
  zero = 1;
  for (i=row, offset=0; i<row+n; i++) {
    for (j=0; j<rowClosure->imageSizes[level][0]; j++, offset+=3) {
      r = g = (i==row) ? 255 : (((i+j)%20>=10) ? 255 : 0);
      b = (i==row) ? 255 : 0;
      fprintf(rowClosure->fout[level], "%c%c%c",
	      (unsigned char) (((float)buffer[offset]*0.8) + r*0.2),
              (unsigned char) (((float)buffer[offset+1]*0.8) + g*0.2),
              (unsigned char) (((float)buffer[offset+2]*0.8) + b*0.2));
      if (buffer[offset]!=0 || buffer[offset+1]!=0 || buffer[offset+2]!=0) zero = 0;
    }
  }

  if (zero) {
    printf("All zero.\n");
  }
  */
  /*
  for (i=0; i<n * rowClosure->imageSizes[level][0] * 3; i++)
    if (fprintf(rowClosure->fout[level], "%c", buffer[i])<=0) return 0;
  */

  return 1;
}

RowClosure *RowClosure_new(int width, int height, int levels) {
  RowClosure *this;
  int i;
  char buffer[256];

  if ((this = malloc(sizeof(RowClosure))) == NULL) {
    printf("Couldn't allocate memory for RowClosure instance.\n");
    return NULL;
  }

  this->levels = levels;

  if ((this->fout = malloc(levels * sizeof(FILE *))) == NULL) {
    printf("Couldn't allocate memory for file array.\n");
    RowClosure_destroy(&this);
    return NULL;
  }

  if ((this->imageSizes = malloc(levels * sizeof(int *))) == NULL) {
    printf("Couldn't allocate memory for size array.\n");
    RowClosure_destroy(&this);
    return NULL;
  }

  for (i=0; i<levels; i++) {
    sprintf(buffer, "output%d.ppm", i);
    if ((this->fout[i] = fopen(buffer, "w")) == NULL) {
      RowClosure_destroy(&this);
      printf("Couldn't open file %d for output.\n", buffer);
      RowClosure_destroy(&this);
      return NULL;
    }
    fprintf(this->fout[i], "P6\n%d %d\n255\n", width, height);

    if ((this->imageSizes[i] = malloc(2 * sizeof(int))) == NULL) {
      printf("Couldn't allocate memory for size array.\n");
      RowClosure_destroy(&this);
      return NULL;
    }
    this->imageSizes[i][0] = width;
    this->imageSizes[i][1] = height;

    width  = (width+1)/2;
    height = (height+1)/2;
  }

  return this;
}

void RowClosure_destroy(RowClosure **this) {
  int i;
  if (this!=NULL && *this!=NULL) {
    if ((*this)->fout!=NULL) {
      for (i=0; i<(*this)->levels; i++) {
        if ((*this)->fout[i] != NULL) {
          fclose((*this)->fout[i]);
        }
	free ((*this)->imageSizes[i]);
      }
      free((*this)->fout);
      free ((*this)->imageSizes);
    }
    free(*this);
    *this = NULL;
  }
}

int getFileType(char *filename) {
  int len = strlen(filename);
  char *ext;

  for (ext = filename+len; ext!=filename && *ext!='.'; ext--);

  if (ext==filename && *ext!='.') return TYPE_ERROR;

  if (strcasecmp(ext, ".gif")==0) return TYPE_GIF;
  else if (strcasecmp(ext, ".pgm")==0) return TYPE_PBM;
  else if (strcasecmp(ext, ".ppm")==0) return TYPE_PBM;
  else if (strcasecmp(ext, ".jpg")==0) return TYPE_JPEG;
  else if (strcasecmp(ext, ".jpeg")==0) return TYPE_JPEG;
  else return TYPE_ERROR;
}

int importGIF(char *filename) {
  TsmReadRowsGIF *reader;
  RowClosure *rowClosure;

  if ((reader = tsmReadRowsGIF_new(filename)) == NULL) {
    return 0;
  }

  if ((rowClosure = RowClosure_new(reader->imageWidth, reader->imageHeight, 3)) == NULL) {  
    printf("Couldn't create RowClosure instance.\n");
    return 0;
  }

  tsmDownSample_inmemory (reader->imageWidth, reader->imageHeight, 3, 32,
			  1, 3, 0,
			  tsmReadRowsGIF_read, reader, write_rows, rowClosure);

  RowClosure_destroy(&rowClosure);
  tsmReadRowsGIF_destroy(&reader);

  return 1;
}

int importPBM(char *filename) {
  TsmReadRowsPBM *reader;
  RowClosure *rowClosure;

  if ((reader = tsmReadRowsPBM_new(filename)) == NULL) {
    return 0;
  }

  if ((rowClosure = RowClosure_new(reader->imageWidth, reader->imageHeight, 3)) == NULL) {  
    printf("Couldn't create RowClosure instance.\n");
    return 0;
  }

  tsmDownSample_inmemory (reader->imageWidth, reader->imageHeight, 3, 32,
			  1, 3, 0,
			  tsmReadRowsPBM_read, reader, write_rows, rowClosure);

  RowClosure_destroy(&rowClosure);
  tsmReadRowsPBM_destroy(&reader);

  return 1;
}

int importJPEG(char *filename) {
  TsmReadRowsJPEG *reader;
  RowClosure *rowClosure;

  if ((reader = tsmReadRowsJPEG_new(filename)) == NULL) {
    return 0;
  }

  if ((rowClosure = RowClosure_new(reader->imageWidth, reader->imageHeight, 3)) == NULL) {  
    printf("Couldn't create RowClosure instance.\n");
    return 0;
  }

  tsmDownSample_inmemory (reader->imageWidth, reader->imageHeight, 3, 32,
			  1, 3, 0,
			  tsmReadRowsJPEG_read, reader, write_rows, rowClosure);

  RowClosure_destroy(&rowClosure);
  tsmReadRowsJPEG_destroy(&reader);

  return 1;
}

int main(int argc, char **argv) {
  int fileType;
  TsmConnection connection;

  if (argc<2) {
    printf("Usage: test_inmem3 <image filename>\n");
    return -1;
  }

  fileType = getFileType(argv[1]);
  switch (fileType) {
  case TYPE_GIF:
    return importGIF(argv[1]);
  case TYPE_PBM:
    return importPBM(argv[1]);
  case TYPE_JPEG:
    return importJPEG(argv[1]);
  }

  return 0;
}

#endif
