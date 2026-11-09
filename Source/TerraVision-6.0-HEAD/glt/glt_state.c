/*
 * Name: GLT_STATE.C
 *
 * Description: OpenGL state analysis and comparison functions.
 *
 * Function List:
 *
 * Dependencies:
 *
 * Revision History:
 *     $Date: 2002/10/31 05:26:57 $
 *     $Author: reddy $
 *
 *  ----- BEGIN LICENSE BLOCK -----
 *  Version: MPL 1.1
 * 
 *  The contents of this file are subject to the Mozilla Public License Version
 *  1.1 (the "License"); you may not use this file except in compliance with
 *  the License. You may obtain a copy of the License at
 *  http://www.mozilla.org/MPL/
 * 
 *  Software distributed under the License is distributed on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 *  for the specific language governing rights and limitations under the
 *  License.
 * 
 *  The Original Code is the TerraVision 3D Terrain Visualization System.
 * 
 *  The Initial Developer of the Original Code is SRI International.
 *
 *  Portions created by the Initial Developer are Copyright (C) 1992-2002
 *  the Initial Developer. All Rights Reserved.
 * 
 *  Contributor(s): Yvan Leclerc, Lee Iverson, Martin Reddy, Nat Bletter. 
 *  ----- END LICENSE BLOCK -----
 *
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include <string.h>

#include <pthr/trace.h>

#include "glt.h"

#undef BOOL_STATE
#define BOOL_STATE(x,y)
#undef INT_STATE
#define INT_STATE(x,y,z)
#undef FLOAT_STATE
#define FLOAT_STATE(x,y,z)

#ifdef GL_MULTISAMPLE_SGIS
#define GL_MS_BOOL_STATES					\
  BOOL_STATE(GL_MULTISAMPLE_SGIS, ms_sgis)			\
  INT_STATE(GL_SAMPLE_BUFFERS_SGIS, sample_buffers_sgis, 1)	\
  INT_STATE(GL_SAMPLE_PATTERN_SGIS, sample_pattern_sgis, 1)	\
  INT_STATE(GL_SAMPLES_SGIS, samples_sgis, 1)
#else
#define GL_MS_BOOL_STATES
#endif

#ifdef GL_VERSION_1_2
#define GL_PO_BOOL_STATES						\
  BOOL_STATE(GL_POLYGON_OFFSET_EXT, po_ext)				\
  BOOL_STATE(GL_POLYGON_OFFSET_FILL, po_fill)				\
  BOOL_STATE(GL_POLYGON_OFFSET_LINE, po_line)				\
  BOOL_STATE(GL_POLYGON_OFFSET_POINT, po_point)
#define GL_PO_FLOAT_STATES						\
  FLOAT_STATE(GL_POLYGON_OFFSET_FACTOR, polygon_offset_factor, 1)	\
  FLOAT_STATE(GL_POLYGON_OFFSET_UNITS, polygon_offset_units, 1)
#else
#ifdef GL_POLYGON_OFFSET_EXT
#define GL_PO_BOOL_STATES			\
  BOOL_STATE(GL_POLYGON_OFFSET_EXT, po_ext)
#define GL_PO_FLOAT_STATES				\
  BOOL_STATE(GL_POLYGON_OFFSET_FACTOR_EXT, po_factor)	\
  BOOL_STATE(GL_POLYGON_OFFSET_BIAS_EXT, po_bias)
#else
#define GL_PO_BOOL_STATES 
#define GL_PO_FLOAT_STATES 
#endif
#endif

#ifdef GL_VERSION_1_1
#define GL_VA_BOOL_STATES					\
  BOOL_STATE(GL_VERTEX_ARRAY, vertex_array)			\
  BOOL_STATE(GL_COLOR_ARRAY, color_array)			\
  BOOL_STATE(GL_EDGE_FLAG_ARRAY, edge_flag_array)		\
  BOOL_STATE(GL_INDEX_ARRAY, index_array)			\
  BOOL_STATE(GL_NORMAL_ARRAY, normal_array)			\
  BOOL_STATE(GL_TEXTURE_COORD_ARRAY, texture_coord_array)
#define GL_VA_INT_STATES						  \
  INT_STATE(GL_COLOR_ARRAY_SIZE, color_array_size, 1)			  \
  INT_STATE(GL_COLOR_ARRAY_STRIDE, color_array_stride, 1)		  \
  INT_STATE(GL_COLOR_ARRAY_TYPE, color_array_type, 1)			  \
  INT_STATE(GL_EDGE_FLAG_ARRAY_STRIDE, edge_flag_array_stride, 1)	  \
  INT_STATE(GL_INDEX_ARRAY_STRIDE, index_array_stride, 1)		  \
  INT_STATE(GL_INDEX_ARRAY_TYPE, index_array_type, 1)			  \
  INT_STATE(GL_NORMAL_ARRAY_STRIDE, normal_array_stride, 1)		  \
  INT_STATE(GL_TEXTURE_COORD_ARRAY_SIZE, texture_coord_array_size, 1)	  \
  INT_STATE(GL_TEXTURE_COORD_ARRAY_STRIDE, texture_coord_array_stride, 1) \
  INT_STATE(GL_TEXTURE_COORD_ARRAY_TYPE, texture_coord_array_type, 1)	  \
  INT_STATE(GL_VERTEX_ARRAY_SIZE, vertex_array_size, 1)			  \
  INT_STATE(GL_VERTEX_ARRAY_STRIDE, vertex_array_stride, 1)		  \
  INT_STATE(GL_VERTEX_ARRAY_TYPE, vertex_array_type, 1)
#else
#define GL_VA_BOOL_STATES
#define GL_VA_INT_STATES
#endif

#ifdef GL_VERSION_1_1
#define GL_LOGIC_BOOL_STATES			\
  BOOL_STATE(GL_COLOR_LOGIC_OP, color_logic_op)	\
  BOOL_STATE(GL_INDEX_LOGIC_OP, index_logic_op)
#else
#define GL_LOGIC_BOOL_STATES
#endif

#define STATES								    \
  GL_MS_BOOL_STATES							    \
  GL_PO_BOOL_STATES							    \
  GL_VA_BOOL_STATES							    \
  GL_LOGIC_BOOL_STATES							    \
  BOOL_STATE(GL_ALPHA_TEST, alpha_test)					    \
  BOOL_STATE(GL_AUTO_NORMAL, auto_normal)				    \
  BOOL_STATE(GL_BLEND, blend)						    \
  BOOL_STATE(GL_CLIP_PLANE0, clip_plane0)				    \
  BOOL_STATE(GL_CLIP_PLANE1, clip_plane1)				    \
  BOOL_STATE(GL_CLIP_PLANE2, clip_plane2)				    \
  BOOL_STATE(GL_COLOR_MATERIAL, color_material)				    \
  BOOL_STATE(GL_CULL_FACE, cull_face)					    \
  BOOL_STATE(GL_CURRENT_RASTER_POSITION_VALID, current_rp_valid)	    \
  BOOL_STATE(GL_DEPTH_TEST, depth_test)					    \
  BOOL_STATE(GL_DEPTH_WRITEMASK, depth_writemask)			    \
  BOOL_STATE(GL_DITHER, dither)						    \
  BOOL_STATE(GL_DOUBLEBUFFER, doublebuffer)				    \
  BOOL_STATE(GL_EDGE_FLAG, edge_flag)					    \
  BOOL_STATE(GL_FOG, fog)						    \
  BOOL_STATE(GL_INDEX_MODE, index_mode)					    \
  BOOL_STATE(GL_LIGHT0, light0)						    \
  BOOL_STATE(GL_LIGHT1, light1)						    \
  BOOL_STATE(GL_LIGHT2, light2)						    \
  BOOL_STATE(GL_LIGHTING, lighting)					    \
  BOOL_STATE(GL_LIGHT_MODEL_LOCAL_VIEWER, light_model_local_viewer)	    \
  BOOL_STATE(GL_LIGHT_MODEL_TWO_SIDE, light_model_two_side)		    \
  BOOL_STATE(GL_LINE_SMOOTH, line_smooth)				    \
  BOOL_STATE(GL_LINE_STIPPLE, line_stipple)				    \
  BOOL_STATE(GL_MAP_COLOR, map_color)					    \
  BOOL_STATE(GL_MAP_STENCIL, map_stencil)				    \
  BOOL_STATE(GL_NORMALIZE, normalize)					    \
  BOOL_STATE(GL_PACK_LSB_FIRST, pack_lsb_first)				    \
  BOOL_STATE(GL_PACK_SWAP_BYTES, pack_swap_bytes)			    \
  BOOL_STATE(GL_POINT_SMOOTH, point_smooth)				    \
  BOOL_STATE(GL_POLYGON_SMOOTH, polygon_smooth)				    \
  BOOL_STATE(GL_POLYGON_STIPPLE, polygon_stipple)			    \
  BOOL_STATE(GL_RGBA_MODE, rgba_mode)					    \
  BOOL_STATE(GL_SCISSOR_TEST, scissor_test)				    \
  BOOL_STATE(GL_STENCIL_TEST, stencil_test)				    \
  BOOL_STATE(GL_STEREO, stereo)						    \
  BOOL_STATE(GL_TEXTURE_1D, texture_1d)					    \
  BOOL_STATE(GL_TEXTURE_2D, texture_2d)					    \
  BOOL_STATE(GL_TEXTURE_GEN_Q, texture_gen_q)				    \
  BOOL_STATE(GL_TEXTURE_GEN_R, texture_gen_r)				    \
  BOOL_STATE(GL_TEXTURE_GEN_S, texture_gen_s)				    \
  BOOL_STATE(GL_TEXTURE_GEN_T, texture_gen_t)				    \
  BOOL_STATE(GL_UNPACK_LSB_FIRST, unpack_lsb_first)			    \
  BOOL_STATE(GL_UNPACK_SWAP_BYTES, unpack_swap_bytes)			    \
  INT_STATE(GL_ALPHA_BITS, alpha_bits, 1)				    \
  INT_STATE(GL_ATTRIB_STACK_DEPTH, attrib_stack_depth, 1)		    \
  INT_STATE(GL_AUX_BUFFERS, aux_buffers, 1)				    \
  INT_STATE(GL_BLEND_DST, blend_dst, 1)					    \
  INT_STATE(GL_BLEND_SRC, blend_src, 1)					    \
  INT_STATE(GL_COLOR_MATERIAL_FACE, color_material_face, 1)		    \
  INT_STATE(GL_COLOR_MATERIAL_PARAMETER, color_material_parameter, 1)	    \
  INT_STATE(GL_CULL_FACE_MODE, cull_face_mode, 1)			    \
  INT_STATE(GL_DEPTH_BITS, depth_bits, 1)				    \
  INT_STATE(GL_DEPTH_FUNC, depth_func, 1)				    \
  INT_STATE(GL_DRAW_BUFFER, draw_buffer, 1)				    \
  INT_STATE(GL_FOG_HINT, fog_hint, 1)					    \
  INT_STATE(GL_FOG_MODE, fog_mode, 1)					    \
  INT_STATE(GL_FRONT_FACE, front_face, 1)				    \
  INT_STATE(GL_INDEX_BITS, index_bits, 1)				    \
  INT_STATE(GL_INDEX_WRITEMASK, index_writemask, 1)			    \
  INT_STATE(GL_LINE_SMOOTH_HINT, line_smooth_hint, 1)			    \
  INT_STATE(GL_LINE_STIPPLE_PATTERN, line_stipple_pattern, 1)		    \
  INT_STATE(GL_LINE_STIPPLE_REPEAT, line_stipple_repeat, 1)		    \
  INT_STATE(GL_LIST_BASE, list_base, 1)					    \
  INT_STATE(GL_LIST_INDEX, list_index, 1)				    \
  INT_STATE(GL_LIST_MODE, list_mode, 1)					    \
  INT_STATE(GL_LOGIC_OP_MODE, logic_op_mode, 1)				    \
  INT_STATE(GL_MATRIX_MODE, matrix_mode, 1)				    \
  INT_STATE(GL_MODELVIEW_STACK_DEPTH, modelview_stack_depth, 1)		    \
  INT_STATE(GL_NAME_STACK_DEPTH, name_stack_depth, 1)			    \
  INT_STATE(GL_PACK_ALIGNMENT, pack_alignment, 1)			    \
  INT_STATE(GL_PACK_ROW_LENGTH, pack_row_length, 1)			    \
  INT_STATE(GL_PACK_SKIP_PIXELS, pack_skip_pixels, 1)			    \
  INT_STATE(GL_PACK_SKIP_ROWS, pack_skip_rows, 1)			    \
  INT_STATE(GL_PERSPECTIVE_CORRECTION_HINT, perspective_correction_hint, 1) \
  INT_STATE(GL_POINT_SMOOTH_HINT, point_smooth_hint, 1)			    \
  INT_STATE(GL_POLYGON_MODE, polygon_mode, 1)				    \
  INT_STATE(GL_POLYGON_SMOOTH_HINT, polygon_smooth_hint, 1)		    \
  INT_STATE(GL_PROJECTION_STACK_DEPTH, projection_stack_depth, 1)	    \
  INT_STATE(GL_READ_BUFFER, read_buffer, 1)				    \
  INT_STATE(GL_RENDER_MODE, render_mode, 1)				    \
  INT_STATE(GL_SHADE_MODEL, shade_model, 1)				    \
  INT_STATE(GL_STENCIL_BITS, stencil_bits, 1)				    \
  INT_STATE(GL_STENCIL_FAIL, stencil_fail, 1)				    \
  INT_STATE(GL_STENCIL_FUNC, stencil_func, 1)				    \
  INT_STATE(GL_STENCIL_PASS_DEPTH_FAIL, stencil_pass_depth_fail, 1)	    \
  INT_STATE(GL_STENCIL_PASS_DEPTH_PASS, stencil_pass_depth_pass, 1)	    \
  INT_STATE(GL_STENCIL_VALUE_MASK, stencil_value_mask, 1)		    \
  INT_STATE(GL_STENCIL_WRITEMASK, stencil_writemask, 1)			    \
  INT_STATE(GL_SUBPIXEL_BITS, subpixel_bits, 1)				    \
  /*INT_STATE(GL_TEXTURE_1D_BINDING_EXT, texture_1d_binding, 1)*/	    \
  /*INT_STATE(GL_TEXTURE_2D_BINDING_EXT, texture_2d_binding, 1)*/	    \
  INT_STATE(GL_TEXTURE_STACK_DEPTH, texture_stack_depth, 1)		    \
  INT_STATE(GL_UNPACK_ALIGNMENT, unpack_alignment, 1)			    \
  INT_STATE(GL_UNPACK_ROW_LENGTH, unpack_row_length, 1)			    \
  INT_STATE(GL_UNPACK_SKIP_PIXELS, unpack_skip_pixels, 1)		    \
  INT_STATE(GL_UNPACK_SKIP_ROWS, unpack_skip_rows, 1)			    \
  GL_VA_INT_STATES							    \
  FLOAT_STATE(GL_ALPHA_TEST_REF, alpha_test_ref, 1)			    \
  FLOAT_STATE(GL_COLOR_CLEAR_VALUE, color_clear_value, 4)		    \
  FLOAT_STATE(GL_CURRENT_COLOR, current_color, 4)			    \
  FLOAT_STATE(GL_CURRENT_NORMAL, current_normal, 3)			    \
  FLOAT_STATE(GL_CURRENT_TEXTURE_COORDS, current_texture_coords, 4)	    \
  FLOAT_STATE(GL_DEPTH_CLEAR_VALUE, depth_clear_value, 1)		    \
  FLOAT_STATE(GL_DEPTH_RANGE, depth_range, 2)				    \
  FLOAT_STATE(GL_FOG_COLOR, fog_color, 4)				    \
  FLOAT_STATE(GL_FOG_DENSITY, fog_density, 1)				    \
  FLOAT_STATE(GL_FOG_END, fog_end, 1)					    \
  FLOAT_STATE(GL_FOG_START, fog_start, 1)				    \
  FLOAT_STATE(GL_LIGHT_MODEL_AMBIENT, light_model_ambient, 4)		    \
  FLOAT_STATE(GL_LINE_WIDTH, line_width, 1)				    \
  FLOAT_STATE(GL_MODELVIEW_MATRIX, modelview_matrix, 16)		    \
  FLOAT_STATE(GL_POINT_SIZE, point_size, 1)				    \
  GL_PO_FLOAT_STATES							    \
  FLOAT_STATE(GL_PROJECTION_MATRIX, projection_matrix, 16)		    \
  FLOAT_STATE(GL_TEXTURE_MATRIX, texture_matrix, 16)			    \
  FLOAT_STATE(GL_VIEWPORT, viewport, 4)					    \
  FLOAT_STATE(GL_ZOOM_X, ZOOM_X, 1)					    \
  FLOAT_STATE(GL_ZOOM_Y, ZOOM_Y, 1)

#undef BOOL_STATE
#define BOOL_STATE(name, field) unsigned int field:1;
#undef INT_STATE
#define INT_STATE(name, field, len) int field[len];
#undef FLOAT_STATE
#define FLOAT_STATE(name, field, len) float field[len];

struct gltGLState {
  STATES
};
#undef BOOL_STATE
#define BOOL_STATE(x,y)
#undef INT_STATE
#define INT_STATE(x,y,z)
#undef FLOAT_STATE
#define FLOAT_STATE(x,y,z)

#undef BOOL_STATE
#define BOOL_STATE(name, field) {#name, name},
static struct {
  char const *name;
  GLenum sel;
} bools[] = {
  STATES
};
#undef BOOL_STATE
#define BOOL_STATE(x,y)

#undef INT_STATE
#define INT_STATE(name, field, len) {#name, name, len},
static struct {
  char const *name;
  GLenum sel;
  int len;
} ints[] = {
  STATES
};
#undef INT_STATE
#define INT_STATE(x,y,z)

#undef FLOAT_STATE
#define FLOAT_STATE(name, field, len) {#name, name, len},
static struct {
  char const *name;
  GLenum sel;
  int len;
} floats[] = {
  STATES
};
#undef FLOAT_STATE
#define FLOAT_STATE(x,y,z)

#define bool_value(v) ((v) ? "True" : "False")

static char *
int_value (char *buf, int const *v, int len)
{
  int i; char *p;
  for (i = 0, p = buf; i < len; ++i, p = strchr(p, '\0')) {
    if (i > 0) *p++ = ' ';
    sprintf (p, "%d", v[i]);
  }
  return buf;
}

static char *
float_value (char *buf, float const *v, int len)
{
  int i; char *p;
  for (i = 0, p = buf; i < len; ++i, p = strchr(p, '\0')) {
    if (i > 0) *p++ = ' ';
    sprintf (p, "%g", v[i]);
  }
  return buf;
}

void
gltTraceState (unsigned int cond, char const *descr)
{
  if (PTRACEP (cond)) {
    int i;
    char buf[128];
    for (i = 0; i < sizeof(bools)/sizeof(bools[0]); ++i) {
      GLboolean val;
      glGetBooleanv (bools[i].sel, &val);
      PTRACE ((cond, descr, "%20s = %s", bools[i].name, bool_value (val)));
    }
    for (i = 0; i < sizeof(ints)/sizeof(ints[0]); ++i) {
      GLint val[16];
      glGetIntegerv (ints[i].sel, val);
      PTRACE ((cond, descr, "%20s = %s", ints[i].name,
	       int_value (buf, val, ints[i].len)));
    }
    for (i = 0; i < sizeof(floats)/sizeof(floats[0]); ++i) {
      GLfloat val[16];
      glGetFloatv (floats[i].sel, val);
      PTRACE ((cond, descr, "%20s = %s", floats[i].name,
	       float_value (buf, val, floats[i].len)));
    }
  }
}

gltGLState *
gltGetState (void)
{
  GLboolean b;
  gltGLState *state = malloc (sizeof (*state));

#undef BOOL_STATE
#define BOOL_STATE(name, field) glGetBooleanv(name, &b), state->field = b;
#undef INT_STATE
#define INT_STATE(name, field, len) glGetIntegerv(name, state->field);
#undef FLOAT_STATE
#define FLOAT_STATE(name, field, len) glGetFloatv(name, state->field);

  STATES;

#undef BOOL_STATE
#define BOOL_STATE(x,y)
#undef INT_STATE
#define INT_STATE(x,y,z)
#undef FLOAT_STATE
#define FLOAT_STATE(x,y,z)

  return state;
}

void
gltFreeState (gltGLState *state)
{
  free (state);
}

static int
int_cmp (int const *v1, int const *v2, int len)
{
  int i;
  for (i = 0; i < len; ++i) {
    if (v1[i] != v2[i])
      return 0;
  }
  return 1;
}

static int
float_cmp (float const *v1, float const *v2, int len)
{
  int i;
  for (i = 0; i < len; ++i) {
    if (v1[i] != v2[i])
      return 0;
  }
  return 1;
}

void
gltTraceStateDiff (unsigned int cond, char const *descr,
		   gltGLState const *s1, gltGLState const *s2)
{
  char vbuf1[128];
  char vbuf2[128];

#undef BOOL_STATE
#define BOOL_STATE(name, field)					\
  if (s1->field != s2->field)					\
    PTRACE ((cond, descr, "%s %s != %s", #name,			\
	     bool_value (s1->field), bool_value (s2->field)));

#undef INT_STATE
#define INT_STATE(name, field, len)				\
  if (! int_cmp (s1->field, s2->field, len)) {			\
    int_value (vbuf1, s1->field, len);				\
    int_value (vbuf2, s2->field, len);				\
    PTRACE ((cond, descr, "%s %s != %s", #name, vbuf1, vbuf2));	\
  }

#undef FLOAT_STATE
#define FLOAT_STATE(name, field, len)				\
  if (! float_cmp (s1->field, s2->field, len)) {		\
    float_value (vbuf1, s1->field, len);			\
    float_value (vbuf2, s2->field, len);			\
    PTRACE ((cond, descr, "%s %s != %s", #name, vbuf1, vbuf2));	\
  }

  if (PTRACEP (cond)) {
    STATES;
  }
}
