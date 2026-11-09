#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>

#ifdef HAVE_GLUT

#include <GL/gl.h>
#include <GL/glut.h>

#include <prtime.h>

#include "TsCache.h"

static TsCache *cache;
static TsTile *top;
static int ntiles;
static GLuint *texid;

static void
clear (void)
{
  glClearColor(0.0, 0.0, 0.0, 1.0);
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

static void
swap_rgb (GLubyte *data, int len)
{
  int i;
  for (i = 0; i < len; ++i, data += 3) {
    GLubyte v = data[0];
    data[0] = data[2];
    data[2] = v;
  }
}

typedef union {
  struct {
    GLubyte b0, b1, b2, b3;
  } as_byte;
  struct {
    GLushort s0, s1;
  } as_short;
  GLuint as_word;
} word_union;

typedef union {
  struct {
    GLubyte b0, b1, b2, b3, b4, b5, b6, b7;
  } as_byte;
  struct {
    GLushort s0, s1, s2, s3;
  } as_short;
  struct {
    GLuint w0, w1;
  } as_word;
} dword_union;

static void
rgb_to_rgba4 (GLubyte const *rgb, int len, GLushort *rgba4)
{
#define p0r(x) (((x) & 0x0f) << 28)
#define p0g(x) (((x) & 0x0f) << 24)
#define p0b(x) (((x) & 0x0f) << 20)
#define p0a(x) (((x) & 0x0f) << 16)
#define p1r(x) (((x) & 0x0f) << 12)
#define p1g(x) (((x) & 0x0f) <<  8)
#define p1b(x) (((x) & 0x0f) <<  4)
#define p1a(x) (((x) & 0x0f) <<  0)

  GLuint *pxls = (GLuint *) rgba4;

  word_union const *rgb_w     = (word_union const *) rgb;
  word_union const *rgb_w_end = (word_union const *) (rgb + 3*len);
  word_union rgb0 = rgb_w[0], rgb1 = rgb_w[1], rgb2 = rgb_w[2];

  /* Iterate up to the last with readahead. */
  rgb_w_end -= 3;
  for ( ; rgb_w < rgb_w_end; rgb_w += 3) {
    GLuint p0, p1;

    p0  = (p0r (rgb0.as_byte.b0 >> 4) |
	   p0g (rgb0.as_byte.b1 >> 4) |
	   p0b (rgb0.as_byte.b2 >> 4) |
	   p0a (0x0f) |
	   p1r (rgb0.as_byte.b3 >> 4));
    rgb0 = rgb_w[3];
    p0 |= (p1g (rgb1.as_byte.b0 >> 4) |
	   p1b (rgb1.as_byte.b1 >> 4) | 
	   p1a (0x0f));
    *pxls++ = p0;

    p1  = (p0r (rgb1.as_byte.b2 >> 4) |
	   p0g (rgb1.as_byte.b3 >> 4));
    rgb1 = rgb_w[4];
    p1 |= (p0b (rgb2.as_byte.b0 >> 4) |
	   p0a (0x0f) |
	   p1r (rgb2.as_byte.b1 >> 4) |
	   p1g (rgb2.as_byte.b2 >> 4) |
	   p1b (rgb2.as_byte.b3 >> 4) | 
	   p1a (0x0f));
    rgb2 = rgb_w[5];
    *pxls++ = p1;
  }

  /* Don't need to read ahead with last iteration. */
  {
    GLuint p0, p1;

    p0  = (p0r (rgb0.as_byte.b0 >> 4) |
	   p0g (rgb0.as_byte.b1 >> 4) |
	   p0b (rgb0.as_byte.b2 >> 4) |
	   p0a (0x0f) |
	   p1r (rgb0.as_byte.b3 >> 4) |
	   p1g (rgb1.as_byte.b0 >> 4) |
	   p1b (rgb1.as_byte.b1 >> 4) | 
	   p1a (0x0f));
    *pxls++ = p0;

    p1  = (p0r (rgb1.as_byte.b2 >> 4) |
	   p0g (rgb1.as_byte.b3 >> 4) |
	   p0b (rgb2.as_byte.b0 >> 4) |
	   p0a (0x0f) |
	   p1r (rgb2.as_byte.b1 >> 4) |
	   p1g (rgb2.as_byte.b2 >> 4) |
	   p1b (rgb2.as_byte.b3 >> 4) | 
	   p1a (0x0f));
    *pxls++ = p1;
  }

#undef p0r
#undef p0g
#undef p0b
#undef p0a
#undef p1r
#undef p1g
#undef p1b
#undef p1a
}

static void
rgb_to_rgb5a1 (GLubyte const *rgb, int len, GLushort *rgb5a1)
{
#define p0r(x) (((x) & 0x1f) << 27)
#define p0g(x) (((x) & 0x1f) << 22)
#define p0b(x) (((x) & 0x1f) << 17)
#define p0a(x) (((x) & 0x01) << 16)
#define p1r(x) (((x) & 0x1f) << 11)
#define p1g(x) (((x) & 0x1f) <<  6)
#define p1b(x) (((x) & 0x1f) <<  1)
#define p1a(x) (((x) & 0x0f) <<  0)

  GLuint *pxls = (GLuint *) rgb5a1;

  word_union const *rgb_w     = (word_union const *) rgb;
  word_union const *rgb_w_end = (word_union const *) (rgb + 3*len);
  word_union rgb0 = rgb_w[0], rgb1 = rgb_w[1], rgb2 = rgb_w[2];

  /* Iterate up to the last with readahead. */
  rgb_w_end -= 3;
  for ( ; rgb_w < rgb_w_end; rgb_w += 3) {
    GLuint p0, p1;

    p0  = (p0r (rgb0.as_byte.b0 >> 3) |
	   p0g (rgb0.as_byte.b1 >> 3) |
	   p0b (rgb0.as_byte.b2 >> 3) |
	   p0a (1) |
	   p1r (rgb0.as_byte.b3 >> 3));
    rgb0 = rgb_w[3];
    p0 |= (p1g (rgb1.as_byte.b0 >> 3) |
	   p1b (rgb1.as_byte.b1 >> 3) | 
	   p1a (1));
    *pxls++ = p0;

    p1  = (p0r (rgb1.as_byte.b2 >> 3) |
	   p0g (rgb1.as_byte.b3 >> 3));
    rgb1 = rgb_w[4];
    p1 |= (p0b (rgb2.as_byte.b0 >> 3) |
	   p0a (1) |
	   p1r (rgb2.as_byte.b1 >> 3) |
	   p1g (rgb2.as_byte.b2 >> 3) |
	   p1b (rgb2.as_byte.b3 >> 3) | 
	   p1a (1));
    rgb2 = rgb_w[5];
    *pxls++ = p1;
  }

  /* Don't need to read ahead with last iteration. */
  {
    GLuint p0, p1;

    p0  = (p0r (rgb0.as_byte.b0 >> 3) |
	   p0g (rgb0.as_byte.b1 >> 3) |
	   p0b (rgb0.as_byte.b2 >> 3) |
	   p0a (1) |
	   p1r (rgb0.as_byte.b3 >> 3) |
	   p1g (rgb1.as_byte.b0 >> 3) |
	   p1b (rgb1.as_byte.b1 >> 3) | 
	   p1a (1));
    *pxls++ = p0;

    p1  = (p0r (rgb1.as_byte.b2 >> 3) |
	   p0g (rgb1.as_byte.b3 >> 3) |
	   p0b (rgb2.as_byte.b0 >> 3) |
	   p0a (1) |
	   p1r (rgb2.as_byte.b1 >> 3) |
	   p1g (rgb2.as_byte.b2 >> 3) |
	   p1b (rgb2.as_byte.b3 >> 3) | 
	   p1a (1));
    *pxls++ = p1;
  }

#undef p0r
#undef p0g
#undef p0b
#undef p0a
#undef p1r
#undef p1g
#undef p1b
#undef p1a
}

static GLenum tex_fmt = GL_RGB;
static GLenum tex_type = GL_UNSIGNED_BYTE;

static void
remap_texture (TsTile const *tile, GLenum type)
{
  int w = tile->pixels[0], h = tile->pixels[1];
  switch (type) {
  case GL_UNSIGNED_SHORT_4_4_4_4_EXT:
    rgb_to_rgba4 (tile->data, w*h, (GLushort *) tile->data);
    break;

  case GL_UNSIGNED_SHORT_5_5_5_1_EXT:
    rgb_to_rgb5a1 (tile->data, w*h, (GLushort *) tile->data);
    break;
 
  default:
    abort ();
    break;
  }
}

static int
remap_textures (TsTile const *tile, int idx, GLenum type)
{
  TsTile *children[4];
  int n = TsCacheTileChildren_const (cache, tile, children);
  int i;

  if (idx == 0) {
    switch (type) {
    case GL_UNSIGNED_SHORT_4_4_4_4_EXT:
      tex_fmt = GL_RGBA;
      tex_type = type;
      break;
    case GL_UNSIGNED_SHORT_5_5_5_1_EXT:
      tex_fmt = GL_RGBA;
      tex_type = type;
      break;
    default:
      return 0;
    }
  }

  remap_texture (tile, type);
  idx++;
  for (i = 0; i < n; ++i)
    idx = remap_textures (children[i], idx, type);

  return idx;
}

static void
define_texture (TsTile const *tile, GLuint id, GLenum fmt)
{
  gltBeginTexture (GL_TEXTURE_2D, id); {
    int w = tile->pixels[0], h = tile->pixels[1];

    glTexImage2D (GL_TEXTURE_2D, 0, fmt, w, h,
		  0, tex_fmt, tex_type, tile->data);

    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  }; gltEndTexture ();
}

static int
define_textures (TsTile const *tile, GLuint const *texid, int idx, GLenum fmt)
{
  TsTile *children[4];
  int n = TsCacheTileChildren_const (cache, tile, children);
  int i;

  define_texture (tile, texid[idx++], fmt);
  for (i = 0; i < n; ++i)
    idx = define_textures (children[i], texid, idx, fmt);

  return idx;
}

static float offset_x = 0.0;
static float offset_y = 0.0;

static void
draw_tile (TsTile const *tile, GLuint id, int level)
{
  GLfloat spc = 1.0/(1<<level);
  GLfloat x = tile->id.x*spc;
  GLfloat y = tile->id.y*spc;

  GLfloat tx = offset_x/tile->pixels[0], ty = offset_y/tile->pixels[1];

  gltBindTexture (GL_TEXTURE_2D, id);
  glBegin (GL_QUADS); {
    glTexCoord2f (tx, ty);
    glVertex2f (x, y);

    glTexCoord2f (tx, 1.0+ty);
    glVertex2f (x, y+spc);

    glTexCoord2f (1.0+ty, 1.0+ty);
    glVertex2f (x+spc, y+spc);

    glTexCoord2f (1.0+ty, ty);
    glVertex2f (x+spc, y);
  }; glEnd ();
}

static int
draw_level_r (TsTile const *tile, GLuint const *texid, int idx,
	      int level, int at_level)
{
  if (level == at_level)
    draw_tile (tile, texid[idx], level);
  
  {
    TsTile *children[4];
    int n = TsCacheTileChildren_const (cache, tile, children);
    int i;
    ++idx;
    ++at_level;
    for (i = 0; i < n; ++i)
      idx = draw_level_r (children[i], texid, idx, level, at_level);
  }
  return idx;
}

static void
draw_level (TsTile const *top, GLuint const *texid, int level)
{
  draw_level_r (top, texid, 0, level, 0);
}

static void
time_rendering (int ntiles, GLuint const *texid)
{
  PRTime start, end;
  PRInt32 accum = 0;
  int i;

  for (i = 0; i < 10; ++i) {

    start = PR_Now ();
  
    draw_level (top, texid, 0);
    draw_level (top, texid, 1);
    draw_level (top, texid, 2);
    draw_level (top, texid, 3);

    glFlush ();
    end = PR_Now ();
    accum += end-start;
  }

  printf ("\t%dus/tile", accum/ntiles/10);
}

static void
test_timing (int ntiles, GLuint const *texid, GLenum fmt, char const *fmts)
{
  PRTime start, end;
  PRInt32 accum = 0;
  int i;

  glFlush ();

  printf ("%.8s:", fmts);
  for (i = 0; i < 10; ++i) {
    start = PR_Now ();
    define_textures (top, texid, 0, fmt); glFlush();
    end = PR_Now ();
    accum += (end-start);
    gltDeleteTextures (ntiles, texid); glFlush();
  }

  printf ("\t%dus/tile", accum/ntiles/10);

  define_textures (top, texid, 0, fmt);
  glColor4f (1.0, 1.0, 1.0, 1.0);

  glDisable (GL_DITHER);
  glFlush();

  glEnable (GL_DEPTH_TEST);
  time_rendering (ntiles, texid);

  glEnable (GL_DEPTH_TEST);
  glDepthFunc (GL_ALWAYS);
  time_rendering (ntiles, texid);

  glDisable (GL_DEPTH_TEST);
  time_rendering (ntiles, texid);

  printf ("\n");
}

static void
test_timings (int ntiles, GLuint const *texid,
	      GLenum const *fmts, char const *fmt_names[])
{
  GLenum const *fmtp = fmts;
  char const **fmt_name = fmt_names;
  while (*fmtp) 
    test_timing (ntiles, texid, *fmtp++, *fmt_name++);
}

static GLuint *
gl_setup (void)
{
  static int setup = 0;
  if (! setup) {
    /*
     * Disable stuff that's likely to slow down 
     * glDrawPixels.(Omit as much of this as possible, 
     * when you know in advance that the OpenGL state is
     * already set correctly.)
     */
    glDisable(GL_ALPHA_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_DITHER);
    glDisable(GL_FOG);
    glDisable(GL_LIGHTING);
    glDisable(GL_LOGIC_OP);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_TEXTURE_1D);
    glDisable(GL_TEXTURE_2D);
    glPixelTransferi(GL_MAP_COLOR, GL_FALSE);
    glPixelTransferi(GL_RED_SCALE, 1);
    glPixelTransferi(GL_RED_BIAS, 0);
    glPixelTransferi(GL_GREEN_SCALE, 1);
    glPixelTransferi(GL_GREEN_BIAS, 0);
    glPixelTransferi(GL_BLUE_SCALE, 1);
    glPixelTransferi(GL_BLUE_BIAS, 0);
    glPixelTransferi(GL_ALPHA_SCALE, 1);
    glPixelTransferi(GL_ALPHA_BIAS, 0);

    /*
     * Disable extensions that could slow down 
     * glDrawPixels.(Actually, you should check for the 
     * presence of the proper extension before making 
     * these calls.I omitted that code for simplicity.)
     */

#ifdef GL_EXT_histogram
    glDisable(GL_HISTOGRAM_EXT);
    glDisable(GL_MINMAX_EXT);
#endif

#ifdef GL_EXT_texture3D
    glDisable(GL_TEXTURE_3D_EXT);
#endif

    /*
     * The following is needed only when using a  
     * multisample-capable visual. 
     */

#ifdef GL_SGIS_multisample
    glDisable(GL_MULTISAMPLE_SGIS);
#endif

    glEnable (GL_TEXTURE_2D);
    glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    texid = calloc (ntiles, sizeof (GLuint));
    gltGenTextures (ntiles, texid);
    define_textures (top, texid, 0, GL_RGB5_A1_EXT);

    setup = 1;
  }

  return texid;
}

static void setup_offsets (void);

static void
display_test (void)
{
  GLuint *texid = gl_setup ();

  GLenum fmts[] = {
    GL_RGB, GL_RGBA,
    GL_RGB4_EXT, GL_RGB8_EXT,
    GL_RGBA4_EXT, GL_RGB5_A1_EXT, GL_RGBA8_EXT,
    (GLenum) 0};
  char const *fmt_names[] = {
    "RGB", "RGBA",
    "RGB4", "RGB8",
    "RGBA4", "RGB5A1", "RGBA8",
  };

  clear ();
  glLoadIdentity ();
  glOrtho (-0.25, 1.25, -0.25, 1.25, -1.0, 1.0);
  test_timings (ntiles, texid, fmts, fmt_names);

  exit (0);
}

static int offset_level = 0;

static void
display_offsets (void)
{
  GLuint *texid = gl_setup ();

  clear ();
  glLoadIdentity ();
#define SZ (1.0/16)
  glOrtho (0.5-SZ, 0.5+SZ, 0.5-SZ, 0.5+SZ, -1.0, 1.0);
  glColor4f (1.0, 1.0, 1.0, 1.0);

  draw_level (top, texid, offset_level); 

  glLineWidth (1.0);
  glBegin (GL_LINES); {
    glVertex2f (0.0, 0.5); glVertex2f (1.0, 0.5);
    glVertex2f (0.5, 0.0); glVertex2f (0.5, 1.0);
  }; glEnd ();
  glFlush();
}

static void
key_offsets (unsigned char key, int x, int y)
{
  switch (key) {
  case '\033':
  case 'q':
    exit (0);
    break;
  }
}

static void
special_offsets (int key, int x, int y)
{
  switch (key) {
  case GLUT_KEY_PAGE_UP:
    if (offset_level > 0)
      --offset_level;
    break;
  case GLUT_KEY_PAGE_DOWN:
    if (offset_level < 3)
      ++offset_level;
    break;
  case GLUT_KEY_UP:
    offset_y += 0.1;
    break;
  case GLUT_KEY_DOWN:
    offset_y -= 0.1;
    break;
  case GLUT_KEY_LEFT:
    offset_x -= 0.1;
    break;
  case GLUT_KEY_RIGHT:
    offset_x += 0.1;
    break;
  default:
    return;
  }

  printf ("Level %d: %g,%g\n", offset_level, offset_x, offset_y);
  glutPostRedisplay ();
}

static void
setup_offsets (void)
{
  glutDisplayFunc (display_offsets);
  glutKeyboardFunc (key_offsets);
  glutSpecialFunc (special_offsets);
}

static int
request_pyramid (TsTile *tile)
{
  static char const funcName[] = "request_pyramid";
  int n, i;
  TsTile *children[4];
  int count = 1;
  TsCacheRequestTile (cache, tile, TSM_NOW);
  TsCacheTileRef (tile);
  n = TsCacheTileChildren (cache, tile, children);
  for (i = 0; i < n; ++i)
    count += request_pyramid (children[i]);
  return count;
}

typedef struct {
  PRLock *lock;
  PRCondVar *cond;
  int ntiles;
} TileMonitor;

static void
count_tiles (TsCache const *cache, void *arg, void *closure)
{
  static char const funcName[] = "count_tiles";
  TsCacheTileCB const *cb = arg;
  TsTile const *tile = cb->tile;
  TileMonitor *tileMon = closure;
  /*printf ("Got [%d %d %d]\n", tile->id.x, tile->id.y, tile->id.res);*/
  swap_rgb ((GLubyte *) tile->data, tile->pixels[0]*tile->pixels[1]);
  PR_Lock (tileMon->lock); {
    ++tileMon->ntiles;
    PR_NotifyCondVar (tileMon->cond);
  }; PR_Unlock (tileMon->lock);
}

static int
get_tiles (TsCache *cache, int levels)
{
  static char const funcName[] = "get_tiles";
  TileSetTspec const *tspec = TsCacheTspec (cache);
  int minLevel = tspec->minLevel;
  TileMonitor tileMon;
  
  top = TsCacheGetTile (cache, 0, 0, minLevel+levels-1);
  ntiles = request_pyramid (top);

  tileMon.lock = PR_NewLock();
  tileMon.cond = PR_NewCondVar (tileMon.lock);
  tileMon.ntiles = 0;
  TsCacheAddCallback (cache, TILE_CACHE, count_tiles, &tileMon, 0);

  TsCacheSendRequests (cache);
  printf ("Requested %d tiles\n", ntiles);

  PR_Lock (tileMon.lock); {
    while (tileMon.ntiles < ntiles)
      PR_WaitCondVar (tileMon.cond, PR_INTERVAL_NO_TIMEOUT);
  }; PR_Unlock (tileMon.lock);

  TsCacheRemoveCallback (cache, TILE_CACHE, count_tiles, &tileMon);
  PR_DestroyCondVar (tileMon.cond);
  PR_DestroyLock (tileMon.lock);

  /*printf ("Done\n");*/
  return ntiles;
}

#define MODE_OFFSETS (0)
#define MODE_TIMING (1)

int
main (int argc, char *argv[])
{
  char const *argv0 = argv[0];
  static char const *url = 0;
  int remap = 0;
  char const *glExts;
  int mode = MODE_OFFSETS;

  glutInit (&argc, argv);

  for (++argv, --argc; *argv; ++argv, --argc) {
    char const *arg = argv[0];
    if (arg[0] == '-') {
      switch (*++arg) {
      case 'r':
	if (arg[1]) {
	  remap = (arg[1] - '0');
	} else {
	  remap = 4;
	}
	break;
      case 't':
	mode = MODE_TIMING;
	break;
      default:
	fprintf (stderr, "Error: Unrecognized option: -%c\n", *arg);
	fprintf (stderr, "Usage: %s [-r[<n>]] [-t]\n", argv0);
	break;
      }
    } else if (! url) {
      url = argv[0];
    }
  }

  if (! url)
    url = "/home/crazy5/magic/TileSets/Pyramids/ntc-1-utm-2scene.128.oi";

  fprintf (stderr, "Dataset: %s\n", url);

  cache = TsCacheCreate (url, NULL);
  TsCacheConnect (cache);
  ntiles = get_tiles (cache, 4);

  glutInitDisplayMode (GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowSize (768, 768);

  glutCreateWindow ("test_render");

  glExts = (char const *) glGetString (GL_EXTENSIONS);
  if (remap) {
    if (! strstr (glExts, "GL_EXT_packed_pixels")) {
      fprintf (stderr, "No point in remapping w/o EXT_packed_pixels\n");
    } else {
      switch (remap) {
      case 5:
	fprintf (stderr, "Remapping to UNSIGNED_SHORT_5_5_5_1\n");
	remap_textures (top, 0, GL_UNSIGNED_SHORT_5_5_5_1_EXT);
	break;
      default:
	fprintf (stderr, "Remapping to UNSIGNED_SHORT_4_4_4_4\n");
	remap_textures (top, 0, GL_UNSIGNED_SHORT_4_4_4_4_EXT);
	break;
      }
    }
  }

  switch (mode) {
  case MODE_TIMING:
    glutDisplayFunc (display_test);
    break;
  case MODE_OFFSETS:
    setup_offsets ();
    break;
  }
  
  glutMainLoop ();
  return 0;
}

#else
int
main ()
{
  printf ("Can't run without GLUT.\n");
  return 1;
}
#endif

