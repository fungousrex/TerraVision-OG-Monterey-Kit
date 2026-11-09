#include "se_conv.h"
#include <stdio.h>

int
main (int argc, char *argv[])
{
  SE_COORD_STATUS_CODE_ENUM status = SE_COORD_SUCCESS;
  char const  *FileName = argv[1];
  FILE        *GeoidHeightFile;
  SE_INT16    NumbHeaderItems = 6;
  SE_INT32    ItemsDiscarded, ItemsRead;
  SE_FLOAT32  geoid_header[6];
  SE_UINT32   size_of_array,rows,cols,ElevationsRead;
  SE_FLOAT32  *SE_separation_table;
#ifdef CONV_DEBUG
  FILE *f_ptr = stderr;
#endif

  if (( GeoidHeightFile = fopen(FileName, "r")) == NULL)
  {
#ifdef CONV_DEBUG
    fprintf(f_ptr,"[SEDRIS CONVERSIONS::SRM_InitializeGeoid]\n"\
	    "    Can't open file: %s!",FileName);
#endif
    return SE_COORD_MISSING_WGS84_GEOID_SEPARATION_FILE;
  }

  /*
   *  Skip the Header Line within the file.
   */
  ItemsDiscarded = 0;
  while ((status == SE_COORD_SUCCESS) &&
	 (ItemsDiscarded < NumbHeaderItems))
  {
    if (feof(GeoidHeightFile))
      status = SE_COORD_INVALID_WGS84_GEOID_SEPARATION_FILE;
    else if (ferror(GeoidHeightFile))
      status = SE_COORD_INVALID_WGS84_GEOID_SEPARATION_FILE;
    else
    {
      ItemsRead = fscanf(GeoidHeightFile, "%f",
			 &geoid_header[ItemsDiscarded]);

      if (ItemsRead > 0)
      {
	ItemsDiscarded = ItemsDiscarded + 1;
      }
    }
  }

  /*
   *  Determine if header read properly, or NOT
   */
  if ((status != SE_COORD_SUCCESS) ||
      geoid_header[0] !=  -90.0 ||
      geoid_header[1] !=   90.0 ||
      geoid_header[2] !=    0.0 ||
      geoid_header[3] !=  360.0 ||
      geoid_header[4] !=  .250000 ||
      geoid_header[5] !=  .250000 ||
      ItemsDiscarded != NumbHeaderItems)
  {
#ifdef CONV_DEBUG
    fprintf(f_ptr,"[SEDRIS CONVERSIONS::SRM_InitializeGeoid]\n"\
	    "    Error reading geoid separation table header!");
#endif
    return SE_COORD_INVALID_WGS84_GEOID_SEPARATION_FILE;
  }

  /*
   * Allocate space for heights array (3 step process)
   *
   * (1) compute rows by dividing total degrees of latitude by the
   *     step size + 1 (should equal 721 rows)
   *
   * (2) compute columns by dividing total degrees of longitude by
   *     the step size + 1 (should equal 1441 columns)
   *
   * (3) allocate memory
   */
  rows = (SE_UINT32) ((geoid_header[1] - geoid_header[0]) /
		      geoid_header[4]) +1;

  cols = (SE_UINT32) ((geoid_header[3] - geoid_header[2]) /
		      geoid_header[5]) +1;
  size_of_array = rows * cols;

  if ((SE_separation_table = (SE_FLOAT32 *) calloc(size_of_array,
						   sizeof(SE_FLOAT32))) == NULL)
  {
#ifdef CONV_DEBUG
    fprintf(f_ptr, "[SEDRIS CONVERSIONS::SRM_InitializeGeoid]\n"\
	    "    Not enough free memory.  %i bytes required",
	    size_of_array * sizeof(SE_FLOAT32));
#endif
    status = SE_COORD_FAILURE;
  }
  else
  {
    /*
     *  Extract elements from the file
     */
    ElevationsRead = 0;
    while ((status == SE_COORD_SUCCESS) &&
	   (ElevationsRead < size_of_array))
    {
      if (feof(GeoidHeightFile))
	status = SE_COORD_INVALID_WGS84_GEOID_SEPARATION_FILE;
      else if (ferror(GeoidHeightFile))
	status = SE_COORD_INVALID_WGS84_GEOID_SEPARATION_FILE;
      else
      {
	ItemsRead = fscanf(GeoidHeightFile, "%f",
			   &SE_separation_table[ElevationsRead]);
	if (ItemsRead > 0)
	{
	  ElevationsRead = ElevationsRead + 1;
	}
      }
    }

    /*
     *  Determine if all elevations of file read properly, or NOT
     */
    if ((status != SE_COORD_SUCCESS) ||
	(ElevationsRead != size_of_array))
    {
#ifdef CONV_DEBUG
      fprintf(f_ptr,"[SEDRIS CONVERSIONS::SRM_InitializeGeoid]\n"\
	      "    Error reading geoid separation table header!");
#endif
      status = SE_COORD_INVALID_WGS84_GEOID_SEPARATION_FILE;
    }
  }

  fclose(GeoidHeightFile);

  {
    int ds = 4;
    int i, j;

    printf ("/* %d rows, %d cols */\n\n", (rows-1)/ds+1, (cols-1)/ds+1);

    /* Write out to source code */
    printf ("static SE_FLOAT32 default_geoid_header[] = {\n");
    printf ("  %g, %g, %g, %g, %g, %g\n",
	    geoid_header[0], geoid_header[1], geoid_header[2],
	    geoid_header[3], geoid_header[4]*ds, geoid_header[5]*ds);
    printf ("};\n\n");

    printf ("static SE_FLOAT32 default_geoid_sep[] = {");
    for (j = 0; j < rows; j += ds) {
      SE_FLOAT32 *r = &SE_separation_table[j*cols];
      for (i = 0; i < cols; i += ds) {
	if (i%(8*ds) == 0) printf ("\n ");
	printf ("%g, ", r[i]);
      }
      printf ("\n");
    }
    printf ("\n 0\n};\n");
  }
  
  return status;
}
