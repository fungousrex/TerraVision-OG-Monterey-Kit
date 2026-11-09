/*
 * Name: text2geovrml.c
 *
 * Description: 
 *   This is the main program for the text to GeoVRML converter.
 *   It's function is to take a simple plain text format and
 *   output GeoVRML code. See the usage() function for program
 *   usage information. The input file format is described as
 *   follows:
 *   
 *   Values will be in column separated format. One line per entry.
 *   
 *   Latitude (Decimal Degrees)
 *   Longitude (Decimal Degrees)
 *   Color (RED, GREEN, YELLOW, BLUE, PURPLE, CYAN, ORANGE, WHITE, BLACK, GRAY)
 *   Symbol Shape (BOX, CONE, INVERTED_CONE SPHERE, CROSS)
 *   Symbol Size (KM: side length for BOX, *CONE, diameter for SPHERE, CROSS)
 *   Comment (Double quote delimited comment field as Text node)
 *   Anchor Description (Double quote delimited description field)
 *   Hyperlink URL (Double quote delimited URL)
 *   
 *   Example:
 *    -22.9062, -43.1748, RED, INVERTED_CONE, 20, "", "Rio De Janeiro"
 *
 *   The parser has the following characteristics:
 *   
 *      - blank lines are ignored (actually lines < 20 chars )
 *      - commas and/or whitespace can be used to separate values
 *      - supports quoted strings with whitespaces inside them
 *      - if any of the strings are not provided, "" is assumed
 *      - lines beginning '#' are ignored
 *
 *   This is based upon code written under contact to AeroJet in order
 *   to support their Fire 2000 demonstration using TerraVision.
 *
 *   Copyright (c) SRI International, 2000. All rights reserved.
 *
 * Revision History:
 *   Created 29 Aug 2000 by Martin Reddy <reddy@ai.sri.com>
 *
 *   $Id: text2geovrml.c,v 1.3.2.1 2002/03/14 05:16:07 reddy Exp $
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#define VERSION  "1.0"

#define FALSE   0
#define TRUE    1

/* specify the defines for all of the supported symbol shapes */

#define  SYMBOL_NONE      0
#define  SYMBOL_BOX       1
#define  SYMBOL_CONE      2
#define  SYMBOL_SPHERE    3
#define  SYMBOL_CROSS     4

/* define the various option settings that we can change by geovrmlMode */

#define OPTION_NONE          0

#define OPTION_METADATA      1
#define OPTION_NOMETADATA    2

#define OPTION_VIEWPOINTS    3
#define OPTION_NOVIEWPOINTS  4

#define OPTION_LABELS        5
#define OPTION_NOLABELS      6

/* macros used for parsing the text file */

#define BUFFER_SIZE   1024    /* size of string buffer for reading from file */

#define IS_WHITESPACE(x) \
  ( (x) == ' ' || (x) == '\t' || (x) == ',' || (x) == '\n' || (x) == '\"' )

/* declare the structure that describes a single model instance */

typedef struct {
  double latitude;             /* latitude of model, decimal degrees */
  double longitude;            /* longitude of model, decimal degrees */
  float  r, g, b;              /* the RGB colour for the object [0..1] */
  int    symbol;               /* one of SYMBOL_BOX etc. */
  float  size;                 /* size of symbol in meters (not km) */
  char   *comment;             /* comment field for this model instance */
  char   *anchor;              /* the name for an Anchor node */
  char   *url;                 /* the url for an Anchor node */
} ModelInstance;

/* read a single model instance from a file handle */

int  readModelInstance( FILE *handle, ModelInstance *inst );

/* free a model instance */

void freeModelInstance( ModelInstance *inst );

/* output the header part of a GeoVRML 1.0 file */

void geovrmlHeader( FILE *handle );

/* output the footer part of a GeoVRML 1.0 file */

void geovrmlFooter( FILE *handle );

/* output a single model instance to the GeoVRML 1.0 file */

void geovrmlAdd( FILE *handle, ModelInstance *inst );

/* change the behaviour of geovrmlAdd by setting an output mode */

void geovrmlSetMode( int option );

/* define our static variables */

static double elevation  = 100.0;         /* symbol elevation above terrain */
static int    metadata   = TRUE;          /* output a GeoMetadata node? */
static int    viewpoints = TRUE;          /* output a GeoViewpoint node? */
static int    labels     = TRUE;          /* output Text nodes for labels? */
static int    count      = 0;             /* number of instances added */
static char   *title     = NULL;          /* title for GeoMetadata node */
static char   *desc      = NULL;          /* description for GeoMetadata */
static char   *target    = NULL;          /* URL target parameter */
static int    inverted   = FALSE;         /* rotate the object 180 deg? */
static float  viewoffset = 0.0;           /* height above obj for viewpoint */

/* return the next string from the buffer and move the buffer point on */

static char *
readModelColumn( char **buffer )
{
  char *start, *end;
  int  quotes = FALSE;

  if ( *buffer == NULL || *buffer == NULL || **buffer == '\0' )
    return NULL;

  start = *buffer;

  while ( IS_WHITESPACE( *start ) ) {
    if ( *(start++) == '\"' ) {
      quotes = !quotes;
      break;
    }
  }

  end = start;

  while ( quotes || ! IS_WHITESPACE( *end ) ) {
    if ( *end == '\0' ) break;
    if ( *end == '\"' ) break;
    end++;
  }

  if ( *end == '\0' ) {
    *buffer = NULL;
    return start;
  }

  *(end++) = '\0';
  *buffer = end;

  return start;
}

/* convert a string into a number value to represent colour */

static void
getColour( ModelInstance *inst, char const *string )
{
  if ( ! strcmp( string, "RED" ) ) {
    inst->r = 1.0;
    inst->g = inst->b = 0.0;
  } else if ( ! strcmp( string, "GREEN" ) ) {
    inst->r = inst->b = 0.0;
    inst->g = 1.0;
  } else if ( ! strcmp( string, "YELLOW" ) ) {
    inst->r = inst->g = 1.0;
    inst->b = 0.0;
  } else if ( ! strcmp( string, "BLUE" ) ) {
    inst->r = inst->g = 0.0;
    inst->b = 1.0;
  } else if ( ! strcmp( string, "PURPLE" ) ) {
    inst->r = inst->b = 1.0;
    inst->g = 0.0;
  } else if ( ! strcmp( string, "CYAN" ) ) {
    inst->g = inst->b = 1.0;
    inst->r = 0.0;
  } else if ( ! strcmp( string, "ORANGE" ) ) {
    inst->r = 1.0;
    inst->g = 0.5;
    inst->b = 0.0;
  } else if ( ! strcmp( string, "WHITE" ) ) {
    inst->r = inst->g = inst->b = 1.0f;
  } else if ( ! strcmp( string, "BLACK" ) ) {
    inst->r = inst->g = inst->b = 0.0f;
  } else if ( ! strcmp( string, "GRAY" ) ) {
    inst->r = inst->g = inst->b = 0.5f;
  } else {
    inst->r = inst->g = inst->b = 0.7f;
  }
}

/* convert a string into a number value to represent a symbol type */

static int
getSymbol( char const *string )
{
  inverted = FALSE;

  if ( ! strcmp( string, "BOX" ) )
    return SYMBOL_BOX;
  else if ( ! strcmp( string, "CONE" ) )
    return SYMBOL_CONE;
  else if ( ! strcmp( string, "INVERTED_CONE" ) ) {
    inverted = TRUE;
    return SYMBOL_CONE;
  } else if ( ! strcmp( string, "SPHERE" ) )
    return SYMBOL_SPHERE;
  else if ( ! strcmp( string, "CROSS" ) )
    return SYMBOL_CROSS;
  else {
    fprintf( stderr, "Unrecognised symbol: %s\n", string );
    return SYMBOL_NONE;
  }
}

/* read a line from the file and fill in the ModelInstance fields */

int
readModelInstance( FILE *handle, ModelInstance *inst )
{
  char buffer[BUFFER_SIZE], *scan, *value;

  /* check we have valid input values */

  if ( handle == NULL || inst == NULL || feof( handle ) )
    return FALSE;

  /* initialise the model structure to zeros/NULLs */

  memset( inst, 0, sizeof(ModelInstance) );

  /* read the next valid line from the file (>= 20 chars in a line) */

  do {
    fgets( buffer, BUFFER_SIZE, handle );
    if ( feof( handle ) ) return FALSE;
  } while ( strlen( buffer ) < 20 || buffer[0] == '#' );

  scan = buffer;

  /* the first column is latitude (decimal degrees) */

  if ( ( value = readModelColumn( &scan ) ) )
    inst->latitude = atof( value );

  /* the second column is longitude (decimal degrees) */

  if ( ( value = readModelColumn( &scan ) ) )
    inst->longitude = atof( value );

  /* the third column is colour */

  if ( ( value = readModelColumn( &scan ) ) )
    getColour( inst, value );

  /* the fourth column is symbol type */

  if ( ( value = readModelColumn( &scan ) ) )
    inst->symbol = getSymbol( value );

  /* the fifth column is size */

  if ( ( value = readModelColumn( &scan ) ) )
    inst->size = 1000.0f * (float) atof( value );

  /* the six column is a comment string */

  if ( ( value = readModelColumn( &scan ) ) )
    inst->comment = strdup( value );

  /* the seventh column is an Anchor node description */

  if ( ( value = readModelColumn( &scan ) ) )
    inst->anchor = strdup( value );

  /* the eighth column is an Anchor node URL */

  if ( ( value = readModelColumn( &scan ) ) )
    inst->url = strdup( value );

  return TRUE;
}

/* free the contents of a model instance (i.e. the comment string) */

void
freeModelInstance( ModelInstance *inst )
{
  if ( inst != NULL ) {
    if ( inst->comment != NULL ) {
      free( inst->comment );
      inst->comment = NULL;
    }
    if ( inst->anchor != NULL ) {
      free( inst->anchor );
      inst->anchor = NULL;
    }
    if ( inst->url != NULL ) {
      free( inst->url );
      inst->url = NULL;
    }
  }
}

/* output a line of VRML97 text to a file and automatically indent it */

int
out( FILE *handle, char const *fmt, ... )
{
  static int indent = 0;
  static int tabsize = 2;
  int        i, offset = 0;
  va_list    args;
  char       *testchar;

  if ( ! ( testchar = strchr( fmt, '#' ) ) )
    testchar = (char *) fmt + strlen(fmt) - 1;

  while ( testchar >= fmt ) {
    if ( *testchar == '{' || *testchar == '[' )
      offset += tabsize;
    else if ( *testchar == '}' || *testchar == ']' )
      offset -= tabsize;
    --testchar;
  }

  if ( offset < 0 )
    indent += offset;

  for ( i = 0; i < indent; i++ )
    fprintf( handle, " " );

  va_start(args, fmt);
  vfprintf( handle, fmt, args );
  fprintf( handle, "\n" );
  va_end(args);

  if ( offset > 0 )
    indent += offset;

  return indent;
}

/* output the header part of a GeoVRML 1.0 file */

void
geovrmlHeader( FILE *handle )
{
  if ( handle == NULL ) return;

  count = 0;

  out( handle, "#VRML V2.0 utf8\n" );

  /* output an EXTERNPROTO for the GeoLocation node */

  out( handle, "EXTERNPROTO GeoLocation [" );
  out( handle, "field    SFNode    geoOrigin          # NULL" );
  out( handle, "field    MFString  geoSystem          # [ \"GDC\" ]" );
  out( handle, "field    SFString  geoCoords          # \"\"" );
  out( handle, "eventIn  SFString  set_geoCoords" );
  out( handle, "eventOut SFString  geoCoords_changed" );
  out( handle, "field    MFNode    children           # []" );
  out( handle, "] [ \"urn:web3d:geovrml:1.0/protos/GeoLocation.wrl\"" );
  out( handle, "    \"file:///C|/Program%%20Files/GeoVRML/1.0/protos/GeoLocation.wrl\"" );
  out( handle, "    \"http://www.geovrml.org/1.0/protos/GeoLocation.wrl\" ]\n" );

  /* output an EXTERNPROTO for the GeoViewpoint node */

  if ( viewpoints ) {
    out( handle, "EXTERNPROTO GeoViewpoint [" );
    out( handle, "field         SFNode      geoOrigin" );
    out( handle, "field         MFString    geoSystem" );
    out( handle, "field         SFString    position" );
    out( handle, "field         SFRotation  orientation" );
    out( handle, "exposedField  SFFloat     fieldOfView" );
    out( handle, "exposedField  SFBool      jump" );
    out( handle, "exposedField  MFString    navType" );
    out( handle, "exposedField  SFBool      headlight" );
    out( handle, "field         SFString    description" );
    out( handle, "field         SFFloat     speed" );
    out( handle, "eventIn       SFString    set_position" );
    out( handle, "eventIn       SFString    set_orientation" );
    out( handle, "eventIn       SFBool      set_bind" );
    out( handle, "eventOut      SFTime      bindTime" );
    out( handle, "eventOut      SFBool      isBound" );
    out( handle, "] [ \"urn:web3d:geovrml:1.0/protos/GeoViewpoint.wrl\"");
    out( handle, "    \"file:///C|/Program%%20Files/GeoVRML/1.0/protos/GeoViewpoint.wrl\"" );
    out( handle, "    \"http://www.geovrml.org/1.0/protos/GeoViewpoint.wrl\" ]\n" );
  }

  /* output an EXTERNPROTO for the GeoMetadata node */

  if ( metadata ) {
    out( handle, "EXTERNPROTO GeoMetadata [" );
    out( handle, "exposedField MFString url        # []" );
    out( handle, "exposedField MFString summary    # []" );
    out( handle, "exposedField MFNode   data       # []" );
    out( handle, "] [ \"urn:web3d:geovrml:1.0/protos/GeoMetadata.wrl\"" );
    out( handle, "    \"file:///C|/Program%%20Files/GeoVRML/1.0/protos/GeoMetadata.wrl\"" );
    out( handle, "    \"http://www.geovrml.org/1.0/protos/GeoMetadata.wrl\" ]\n" );
  }    

  out( handle, "Group {" );
  out( handle, "children [" );
}

/* output the footer part of a GeoVRML 1.0 file */

void
geovrmlFooter( FILE *handle )
{
  if ( handle == NULL ) return;

  if ( metadata ) {
    out( handle, "GeoMetadata {" );
    out( handle, "summary [" );
    out( handle, "\"title\"       \"%s\"", title );
    if ( desc )
      out( handle, "\"description\" \"%s\"", desc );
    out( handle, "\"software\"    \"text2geovrml, "
	     "(C) 2000 SRI International\"" );
    out( handle, "\"models\"      \"%d\"", count );
    out( handle, "]" );
    out( handle, "}" );
  }

  out( handle, "]" );
  out( handle, "}" );
}

/* change the behaviour of geovrmlAdd through some setting */

void
geovrmlSetMode( int option )
{
  switch ( option ) {
  case OPTION_METADATA:
  case OPTION_NOMETADATA:
    metadata = ( option == OPTION_METADATA );
    break;

  case OPTION_VIEWPOINTS:
  case OPTION_NOVIEWPOINTS:
    viewpoints = ( option == OPTION_VIEWPOINTS );
    break;
    
  case OPTION_LABELS:
  case OPTION_NOLABELS:
    labels = ( option == OPTION_LABELS );
    break;
  }
}

/* output an Appearance node for the desired surface proporties */

void
geovrmlAppearance( FILE *handle, ModelInstance *inst )
{
  out( handle, "appearance Appearance {" );
  out( handle, "material Material {" );
  out( handle, "diffuseColor %1.3g %1.3g %1.3g", inst->r, inst->g, inst->b );
  out( handle, "}" );
  out( handle, "}" );
}

/* output geometry for the BOX symbol type */

void
geovrmlBox( FILE *handle, ModelInstance *inst )
{
  out( handle, "Shape {" );
  geovrmlAppearance( handle, inst );
  out( handle, "geometry Box { size %1.10g %1.10g %1.10g }",
       inst->size, inst->size, inst->size );
  out( handle, "}" );
}

/* output geometry for the CONE symbol type */

void
geovrmlCone( FILE *handle, ModelInstance *inst )
{
  out( handle, "Shape {" );
  geovrmlAppearance( handle, inst );
  out( handle, "geometry Cone {" );
  out( handle, "bottomRadius %1.10g", inst->size / 2.0 );
  out( handle, "height %1.10g", inst->size );
  out( handle, "}" );
  out( handle, "}" );
}

/* output geometry for the SPHERE symbol type */

void
geovrmlSphere( FILE *handle, ModelInstance *inst )
{
  out( handle, "Shape {" );
  geovrmlAppearance( handle, inst );
  out( handle, "geometry Sphere { radius %1.10g }",
	   inst->size / 2.0 );
  out( handle, "}" );
}

/* output geometry for the CROSS symbol type */

void
geovrmlCross( FILE *handle, ModelInstance *inst, int shape )
{
  float w, h, d;

  switch ( shape ) {
  case 0:
    w = inst->size;
    h = d = inst->size / 4.0f;
    break;
  case 1:
    h = inst->size;
    w = d = inst->size / 4.0f;
    break;
  case 2:
    d = inst->size;
    w = h = inst->size / 4.0f;
    break;
  default:
    w = h = d = 0.0f;
  }

  out( handle, "Shape {" );
  geovrmlAppearance( handle, inst );
  out( handle, "geometry Box { size %1.10g %1.10g %1.10g }", w, h, d );
  out( handle, "}" );
}

/* output geometry for symbol's label */

void
geovrmlLabel( FILE *handle, ModelInstance *inst )
{
  out( handle, "Transform {" );
  out( handle, "translation 0 %1.10g 0", inst->size * 1.5 );
  out( handle, "children [" );
  out( handle, "Shape {" );
  out( handle, "appearance Appearance {" );
  out( handle, "material Material { diffuseColor 1 0 0 }" );
  out( handle, "}" );
  out( handle, "geometry Text {" );
  out( handle, "string \"%s\"", inst->comment );
  out( handle, "fontStyle FontStyle {" );
  out( handle, "justify \"MIDDLE\"" );
  out( handle, "size %1.10g", inst->size * 0.75 );
  out( handle, "spacing 1.2" );
  out( handle, "}" );
  out( handle, "}" );
  out( handle, "}" );
  out( handle, "]" );
  out( handle, "}" );
}

/* output a single model instance to the GeoVRML 1.0 file */

void
geovrmlAdd( FILE *handle, ModelInstance *inst )
{
  int shape;

  if ( handle == NULL || inst == NULL ) return;

  count++;

  if ( viewpoints ) {
    out( handle, "GeoViewpoint {" );
    out( handle, "geoSystem [ \"GDC\", \"WE\" ]" );
    out( handle, "position \"%1.10g %1.10g +%1.10g\"",
	 inst->latitude,
	 inst->longitude,
	 ( viewoffset ) ?
	   elevation + viewoffset : elevation + ( inst->size * 10 ) );
    out( handle, "orientation 1 0 0 -1.57" );
    out( handle, "description \"Model %d\"", count );
    out( handle, "}" );
  }

  out( handle, "GeoLocation {" );
  out( handle, "geoSystem [ \"GDC\", \"WE\" ]" );
  out( handle, "geoCoords \"%1.10lg %1.10lg +%1.10g\"", inst->latitude,
       inst->longitude, elevation + inst->size / 2.0 );
  out( handle, "children [" );

  /* output the Anchor node if needed */

  if ( inst->anchor && inst->anchor[0] != '\0' ) {
    out( handle, "Anchor {" );
    out( handle, "description \"%s\"", inst->anchor );
    if ( target )
      out( handle, "parameter \"target=%s\"", target );
    if ( inst->url && inst->url[0] != '\0' )
      out( handle, "url \"%s\"", inst->url );
    out( handle, "children [" );
  }

  /* are we inverting this geometry? */

  if ( inverted ) {
    out( handle, "Transform {" );
    out( handle, "rotation 1 0 0 3.14159" );
    out( handle, "children [" );
  }

  /* define the symbol's geometry */
    
  switch ( inst->symbol ) {
  case SYMBOL_BOX:
    geovrmlBox( handle, inst );    
    break;
    
  case SYMBOL_CONE:
    geovrmlCone( handle, inst );    
    break;
    
  case SYMBOL_SPHERE:
    geovrmlSphere( handle, inst );    
    break;
    
  case SYMBOL_CROSS:
    for ( shape = 0; shape < 3; shape++ )
      geovrmlCross( handle, inst, shape );
    break;
    
  default:
    fprintf( handle, "# no symbol defined\n" );
  }
  
  /* close the inverting Transform */

  if ( inverted ) {
    out( handle, "]" );
    out( handle, "}" );
  }

  /* add a Text node for the comment label */

  if ( labels && inst->comment && strlen( inst->comment ) > 0 )
    geovrmlLabel( handle, inst );    

  /* close the Anchor node */

  if ( inst->anchor && inst->anchor[0] != '\0' ) {
    out( handle, "]" );
    out( handle, "}" );
  }

  out( handle, "]" );
  out( handle, "}" );
}

void
usage( void )
{
  printf( "Text2GeoVRML - Version %s\n\n", VERSION );
  puts( "This utility will take a simple input text file, where each line" );
  puts( "describes an object, its location, and visual attributes. It then" );
  puts( "outputs a valid GeoVRML 1.0 description of thoese objects.\n" );
  puts( "usage: text2geovrml [options] <infile> [<outfile>]\n" );
  puts( "  <infile>       : simple text file, see -info for details" );
  puts( "  <outfile>      : output GeoVRML file, or stdout if not specified" );
  puts( "  -metadata      : output a GeoMetadata node (def)" );
  puts( "  -nometadata    : suppress the output of the GeoMetadata node" );
  puts( "  -viewpoints    : output a GeoViewpoint above each model (def)" );
  puts( "  -noviewpoints  : suppress the output of the GeoViewpoint nodes" );
  puts( "  -labels        : output Text node labels for all comments (def)" );
  puts( "  -nolabels      : suppress the output of the Text node labels" );
  puts( "  -viewoffset <h>: height above objects to place GeoViewpoints" );
  puts( "  -title <name>  : the title string to put in the GeoMetadata node" );
  puts( "  -desc <text>   : description to put in the GeoMetadata node" );
  puts( "  -target <frm>  : target frame for all Anchor urls" );
  puts( "  -help          : this command line usage information" );
  puts( "  -info          : display information about the simple file format" );
  puts( "" );
  exit( 1 );
}

void
info( void )
{
  printf( "Text2GeoVRML - Version %s\n\n", VERSION );
  puts( "The simple text file is an ASCII file with each line containing an" );
  puts( "object description. The entry format is described below. All string");
  puts( "fields are optional and will be treated as \"\" if not specified." );
  puts( "\nText file entry format:" );
  puts( "  lat, long, colour, shape, size, textnode, anchor, url" );
  puts( "\nwhere," );
  puts( "  - lat is WGS84 latitude in decimal degrees" );
  puts( "  - long is WGS84 longitude in decimal degrees" );
  puts( "  - color is one of RED, GREEN, YELLOW, BLUE, PURPLE, CYAN, ORANGE" );
  puts( "    WHITE, BLACK, GRAY" );
  puts( "  - shape is one of BOX, CONE, INVERTED_CONE SPHERE, CROSS" );
  puts( "  - size is in km. Side length for BOX, *CONE, or diameter for" );
  puts( "    SPHERE, CROSS" );
  puts( "  - comment is a double quote delimited string for a Text node" );
  puts( "  - anchor is a double quote delimited Anchor description field" );
  puts( "  - url is a double quote delimited Anchor node URL field" );
  puts( "\nExample text file entry:" );
  puts( "  -22.9062, -43.1748, RED, INVERTED_CONE, 20, \"\", "
	"\"Rio De Janeiro\"" );
  puts( "" );
  exit( 1 );
}

int
main( int argc, char **argv )
{
  int          i;
  FILE         *infile = NULL, *outfile = NULL;
  ModelInstance model;

  /* parse the command line arguments from the user */

  for ( i = 1; i < argc; i++ ) {
    if ( argv[i][0] == '-' ) {

      if ( strcmp( argv[i], "-metadata" ) == 0 )
	geovrmlSetMode( OPTION_METADATA );
      else if ( strcmp( argv[i], "-nometadata" ) == 0 )
	geovrmlSetMode( OPTION_NOMETADATA );
      else if ( strcmp( argv[i], "-viewpoints" ) == 0 )
	geovrmlSetMode( OPTION_VIEWPOINTS );
      else if ( strcmp( argv[i], "-noviewpoints" ) == 0 )
	geovrmlSetMode( OPTION_NOVIEWPOINTS );
      else if ( strcmp( argv[i], "-labels" ) == 0 )
	geovrmlSetMode( OPTION_LABELS );
      else if ( strcmp( argv[i], "-nolabels" ) == 0 )
	geovrmlSetMode( OPTION_NOLABELS );
      else if ( strcmp( argv[i], "-help" ) == 0 )
	usage();
      else if ( strcmp( argv[i], "-info" ) == 0 )
	info();
      else if ( strcmp( argv[i], "-title" ) == 0 )
	title = ( argc > i+1 ) ? argv[++i] : NULL;
      else if ( strcmp( argv[i], "-desc" ) == 0 )
	desc = ( argc > i+1 ) ? argv[++i] : NULL;
      else if ( strcmp( argv[i], "-target" ) == 0 )
	target = ( argc > i+1 ) ? argv[++i] : NULL;
      else if ( strcmp( argv[i], "-viewoffset" ) == 0 )
	viewoffset = ( argc > i+1 ) ? (float) atof(argv[++i]) : 0.0f;
      else {
	fprintf( stderr, "Unsupported option: %s\n", argv[i] );
	exit( 1 );
      }

    } else if ( infile == NULL ) {
      if ( ( infile = fopen( argv[i], "r" ) ) == NULL ) {
	fprintf( stderr, "Cannot open input file: %s\n", argv[i] );
	exit( 1 );
      }
    } else if ( outfile == NULL ) { 
      if ( ( outfile = fopen( argv[i], "w" ) ) == NULL ) {
	fprintf( stderr, "Cannot write geovrml file: %s\n", argv[i] );
	exit( 1 );
      }
    } else {
      fprintf( stderr, "Too many command line arguments, try -h for help." );
      exit( 1 );
    }
      
  }

  if ( ! title ) title = "GeoVRML Models";

  /* use stdout if no filenames specified, but require an input file */

  if ( ! infile ) usage();
  if ( ! outfile ) outfile = stdout;

  /* go through the input file, read its contents and convert to GeoVRML */

  geovrmlHeader( outfile );

  while ( readModelInstance( infile, &model ) ) {
    geovrmlAdd( outfile, &model );
  }

  geovrmlFooter( outfile );
  
  /* close down */

  if ( infile != stdin ) fclose( infile );
  if ( outfile != stdout ) fclose( outfile );

  return 0;
}

/*** EOF: text2geovrml.c ***/
