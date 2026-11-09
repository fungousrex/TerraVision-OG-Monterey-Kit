/*
 * Name: c_print.c
 *
 * Library:
 *   This source file is part of the vrml2 library, a library for parsing
 *   and generating VRML97 format files. This library is supplied as part
 *   of the tsmApi distribution, available from:
 *
 *     http://www.tsmApi.com/
 *
 * Revision History:
 *   VRML 2.0 version by Chris Connolly, SRI International
 *   Updated interface by Lee Iverson, SRI International
 *
 *   $Id: c_print.c,v 1.13 2000/08/28 23:21:32 reddy Exp $
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

#include <stddef.h>
#include <stdlib.h>

#include "vrml2int.h"
#include "c_nodesI.h"

typedef struct defs_used {
  struct defs_used *next;
  void *val;
} defs_used;

typedef struct {
  vsgWorld *wrl;
  FILE *fp;
  int pos_on_line;
  defs_used *defs;
} print_state;

typedef int (*printFunc)(print_state *s, void *data, plist_t p, int indent);

static void cleanup1 (print_state *s);

static int print_protos (print_state *s, MFProto const *mfproto, 
			 plist_t props, int indent);
static int print_SFNode (print_state *s, SFNode node, plist_t p, int indent);
static int print_MFNode1 (print_state *s, MFNode *mf, plist_t p, int indent);
static int print_Any (print_state *s, vsgDataType type, vsgAnyArg const *val,
		      plist_t props, int indent);
static int print_routes (print_state *s, vsgRouteLink const *routes,
			 plist_t props, int indent);

static void
advance1 (print_state *s)
{
  ++s->pos_on_line;
}

static void
putc1 (print_state *s, char c)
{
  putc (c, s->fp);
  advance1 (s);
}

static void
puts1 (print_state *s, char *str)
{
  if (*str) {
    fputs (str, s->fp);
    advance1 (s);
  }
}

static void
indent1 (print_state *s, int indent)
{
  if (! s->pos_on_line) {
    s->pos_on_line = indent;
    for (; indent >= 8; indent -= 8)
      putc ('\t', s->fp);
    for (; indent; --indent)
      putc (' ', s->fp);
  }
}

static void
newline1 (print_state *s)
{
  if (s->pos_on_line) {
    s->pos_on_line = 0;
    putc ('\n', s->fp);
  }
}

int
vsgdPrint (vsgWorld *wrl, FILE *fp)
{
  char const *ftype = vsgWorldFiletype (wrl);
  MFProto *protos = vsgWorldProtos (wrl);
  MFNode *nodes = vsgWorldNodes (wrl);
  plist_t props = vsgWorldProperties (wrl);
  print_state s_s, *s = &s_s;
  int ret = TRUE;

  s->wrl = wrl;
  s->pos_on_line = 0;
  s->defs = 0;
  s->fp = fp;

  if (ftype)
    fprintf (fp, "%s\n\n", ftype);

  /* Print out any proto definitions. */
  if (protos)
    print_protos (s, protos, props, 0);
  
  /* Now the nodes. */
  ret = print_MFNode1 (s, nodes, props, 0);
  if (ret) newline1 (s);

  /* And finally, any routes. */
  print_routes (s, wrl->frame->routes, props, 0);

  cleanup1 (s);
  return ret;
}


static void
cleanup1 (print_state *s)
{
  defs_used *def = s->defs, *next;
  for ( ; def; def = next) {
    next = def->next;
    free (def);
  }
}

static void
define1 (print_state *s, void *val, symbol_t name)
{
  defs_used *def = (defs_used *) malloc (sizeof (*def));
  def->next = s->defs;
  def->val = val;
  s->defs = def;
}

static int
defined1 (print_state *s, void *val, symbol_t name)
{
  defs_used *def = s->defs;
  for ( ; def; def = def->next) {
    if (def->val == val)
      return 1;
  }
  return 0;
}

static int
print_SFBool (print_state *s, SFBool b, plist_t p, int indent)
{
  puts1 (s, b ? "TRUE" : "FALSE");
  return 1;
}

static int
print_SFColor (print_state *s, SFColor const *c, plist_t p, int indent)
{
  fprintf (s->fp, "%g %g %g", c->r, c->g, c->b);
  return 1;
}

static int
print_SFFloat (print_state *s, SFFloat t, plist_t p, int indent)
{
  fprintf (s->fp, "%g", t);
  return 1;
}

static int
print_SFImage (print_state *s, SFImage const *im, plist_t props, int indent)
{
  int w = im->width, h = im->height, depth = im->depth;
  int i, n = w*h;
  unsigned char const *p = im->data;
  static char *fmts[] = { 0, "0x%02x ", "0x%04x ", "0x%06x ", "0x%08x " };
  fprintf (s->fp, "%d %d %d", w, h, depth);
  for (i = 0; i < n; ++i, p += depth) {
    int j, v = 0;
    for (j = 0; j < depth; ++j)
      v <<= (v << 8) + p[j];
    if ((i%w) == 0) {
      newline1 (s);
      indent1 (s, indent+2);
    }
    fprintf (s->fp, fmts[depth], v);
  }
  return 1;
}

static int
print_SFInt32 (print_state *s, SFInt32 i, plist_t p, int indent)
{
  fprintf (s->fp, "%ld", (long) i);
  return 1;
}

static int
print_SFRotation (print_state *s, SFRotation const *r, plist_t p, int indent)
{
  fprintf (s->fp, "%g %g %g %g", r->a, r->b, r->c, r->d);
  return 1;
}

static int
print_SFString (print_state *s, SFString str, plist_t p, int indent)
{
  fprintf (s->fp, "\"%s\"", str);
  return 1;
}

static int
print_SFTime (print_state *s, SFTime t, plist_t p, int indent)
{
  fprintf (s->fp, "%g", t);
  return 1;
}

static int
print_SFVec2f (print_state *s, SFVec2f const *v2, plist_t p, int indent)
{
  SFFloat const *v = v2->v;
  fprintf (s->fp, "%g %g", v[0], v[1]);
  return 1;
}

static int
print_SFVec3f (print_state *s, SFVec3f const *v3, plist_t p, int indent)
{
  SFFloat const *v = v3->v;
  fprintf (s->fp, "%g %g %g", v[0], v[1], v[2]);
  return 1;
}

static int
print_MFGeneric (print_state *s, 
		 MFGeneric const *mf, vsgDataType type, int size, char *sep,
		 int (*newlinep) (int i, int n, void *v),
		 plist_t props, int indent)
{
  int n;

  if (! mf || (n = mf->mf.len) < 1) {
    puts1 (s, "[]");

  } else if (n == 1) {
    return print_Any (s, type, mf->data, props, indent+2);

  } else {
    char *p = mf->data;
    int i;

    putc1 (s, '[');
    if (n > 1 && (*newlinep) (-1, n, 0)) {
      newline1 (s);
      indent1 (s, indent+2);
    } else {
      putc1 (s, ' ');
    }

    for (i = 0; i < n-1; ++i, p += size) {
      print_Any (s, type, (vsgAnyArg *) p, props, indent+2);
      puts1 (s, sep);
      if ((*newlinep) (i, n, p)) {
	newline1 (s);
	indent1 (s, indent+2);
      }
    }

    print_Any (s, type, (vsgAnyArg *) p, props, indent+2);
    puts1 (s, " ]");
  }
  return 1;
}

static int
newlinep_all (int i, int n, void *val)
{
  return 1;
}

static int
newlinep_neg1 (int i, int n, void *val)
{
  SFInt32 v = (val ? *(SFInt32 *)val : -1);
  return n >= 8 && v == -1;
}

static int
newlinep_every5 (int i, int n, void *val)
{
  return n >= 10 && (i%5) == 0;
}

static int
print_MFColor (print_state *s, MFColor const *mf, plist_t p, int indent)
{
  return print_MFGeneric (s, (MFGeneric const *) mf,
			  vsgSFColor, sizeof(mf->data[0]), 
			  ",", newlinep_all, p, indent);
}

static int
print_MFFloat (print_state *s, MFFloat const *mf, plist_t p, int indent)
{
  return print_MFGeneric (s, (MFGeneric const *) mf,
			  vsgSFFloat, sizeof(mf->data[0]), 
			  " ", newlinep_every5, p, indent);
}

static int
print_MFInt32 (print_state *s,  MFInt32 const *mf, plist_t p, int indent)
{
  return print_MFGeneric (s, (MFGeneric const *) mf,
			  vsgSFInt32, sizeof(mf->data[0]), 
			  " ", newlinep_neg1, p, indent);
}

static int
print_MFNode1 (print_state *s, MFNode *mf, plist_t p, int indent)
{
  SFNode *f = mf->data;
  int i, n = mf->mf.len;
  for (i = 0; i < n; ++i, ++f) {
    newline1 (s);
    indent1 (s, indent);
    print_SFNode (s, *f, p, indent);
  }

  return 1;
}

static int
print_MFNode (print_state *s, MFNode *mf, plist_t p, int indent)
{
  int n;

  if (! mf || (n = mf->mf.len) < 1) {
    puts1 (s, "[]");

  } else if (n == 1) {
    return print_SFNode (s, mf->data[0], p, indent);
    
  } else {
    putc1 (s, '[');
    print_MFNode1 (s, mf, p, indent+2);
    newline1 (s);
    indent1 (s, indent);
    putc1 (s, ']');
  }
  return 1;
}

static int
print_MFRotation (print_state *s, MFRotation const *mf, plist_t p, int indent)
{
  return print_MFGeneric (s, (MFGeneric const *) mf,
			  vsgSFRotation, sizeof(mf->data[0]), 
			  ",", newlinep_all, p, indent);
}

static int
print_MFString (print_state *s, MFString const *mf, plist_t p, int indent)
{
  return print_MFGeneric (s, (MFGeneric const *) mf,
			  vsgSFString, sizeof(mf->data[0]), 
			  "", newlinep_all, p, indent);
}

static int
print_MFVec2f (print_state *s, MFVec2f const *mf, plist_t p, int indent)
{
  return print_MFGeneric (s, (MFGeneric const *) mf,
			  vsgSFVec2f, sizeof(mf->data[0]), 
			  ",", newlinep_all, p, indent);
}

static int
print_MFVec3f (print_state *s, MFVec3f const *mf, plist_t p, int indent)
{
  return print_MFGeneric (s, (MFGeneric const *) mf,
			  vsgSFVec3f, sizeof(mf->data[0]), 
			  ",", newlinep_all, p, indent);
}

static void
print_fieldDecl (print_state *s, 
		 vsgClassField const *field, int print_default,
		 plist_t props, int indent)
{
  vsgFieldType ftype = field->fieldType;
  vsgDataType type = field->type;
  newline1 (s);
  indent1 (s, indent);
  fprintf (s->fp, "%s %s %s",
	   vsgFieldTypeName[ftype], vsgDataTypeName[type], field->name);
  if (print_default && (ftype == vsgTfield || ftype == vsgTexposedField)) {
    putc1 (s, ' ');
    print_Any (s, type, &field->dflt, props, indent);
  }
}

static void
print_PROTOfields (print_state *s, vsgProtoClass const *proto, int indent)
{
  int defaults = ! proto->is_extern;
  int nfields = proto->nfields;
  if (nfields) {
    int i;
    for (i = 0; i < nfields; ++i) {
      vsgClassField *field = (vsgClassField *) proto->fields[i];
      print_fieldDecl (s, field, defaults, proto->props, indent+2);
    }
    newline1 (s);
    indent1 (s, indent);
  } else {
    putc1 (s, ' ');
  }
}

static int
print_EXTERNPROTO (print_state *s, vsgProtoClass const *proto,
		   plist_t props, int indent)
{
  vsgExternProtoClass const *externproto = (vsgExternProtoClass const *) proto;
  MFString *urls = externproto->extern_urls;

  fprintf (s->fp, "EXTERNPROTO %s [", proto->c.name);
  advance1 (s);

  print_PROTOfields (s, proto, indent);

  puts1 (s, "] ");
  print_MFString (s, urls, props, indent);
  
  return 1;
}

static int
print_PROTO (print_state *s, vsgProtoClass const *proto,
	     plist_t props, int indent)
{
  MFNode *nodes = proto->nodes;

  if (proto->is_extern)
    return print_EXTERNPROTO (s, proto, props, indent);

  fprintf (s->fp, "PROTO %s [", proto->c.name);
  advance1 (s);

  print_PROTOfields (s, proto, indent);

  puts1 (s, "] {");

  /* Print out any internally defined protos. */
  if (proto->protos)
    print_protos (s, proto->protos, proto->props, indent+2);

  /* Print the list of nodes. */
  if (nodes->mf.len) {
    print_MFNode1 (s, nodes, proto->props, indent+2);
    newline1 (s);
    indent1 (s, indent);
  } else {
    putc1 (s, ' ');
  }

  /* Print any routes. */
  print_routes (s, proto->routes, proto->props, indent+2);

  indent1 (s, indent);
  putc1 (s, '}');
  
  return 1;
}

static int
print_protos (print_state *s, MFProto const *mfproto, 
	      plist_t props, int indent)
{
  int len, i;
  vsgProtoClass const **protos;
  if (! mfproto || (len=mfproto->mf.len) == 0)
    return 1;
  protos = (vsgProtoClass const **) mfproto->data;
  for (i = 0; i < len; ++i) {
    vsgProtoClass const *proto = protos[i];
    indent1 (s, indent);
    print_PROTO (s, proto, props, indent);
    newline1 (s);
  }
  return 1;
}

static int
print_Any (print_state *s, vsgDataType type, vsgAnyArg const *val,
	   plist_t props, int indent)
{
  switch (type) {
  case vsgMFColor:
    return print_MFColor (s, val->MFColor, props, indent);
  case vsgMFFloat:
    return print_MFFloat (s, val->MFFloat, props, indent);
  case vsgMFInt32:
    return print_MFInt32 (s, val->MFInt32, props, indent);
  case vsgMFNode:
    return print_MFNode (s, val->MFNode, props, indent);
  case vsgMFRotation:
    return print_MFRotation (s, val->MFRotation, props, indent);
  case vsgMFString:
    return print_MFString (s, val->MFString, props, indent);
  case vsgMFVec2f:
    return print_MFVec2f (s, val->MFVec2f, props, indent);
  case vsgMFVec3f:
    return print_MFVec3f (s, val->MFVec3f, props, indent);
  case vsgSFBool:
    return print_SFBool (s, val->SFBool, props, indent);
  case vsgSFColor:
    return print_SFColor (s, &val->SFColor, props, indent);
  case vsgSFFloat:
    return print_SFFloat (s, val->SFFloat, props, indent);
  case vsgSFImage:
    return print_SFImage (s, &val->SFImage, props, indent);
  case vsgSFInt32:
    return print_SFInt32 (s, val->SFInt32, props, indent);
  case vsgSFNode:
    return print_SFNode (s, val->SFNode, props, indent);
  case vsgSFRotation:
    return print_SFRotation (s, &val->SFRotation, props, indent);
  case vsgSFString:
    return print_SFString (s, val->SFString, props, indent);
  case vsgSFTime:
    return print_SFTime (s, val->SFTime, props, indent);
  case vsgSFVec2f:
    return print_SFVec2f (s, &val->SFVec2f, props, indent);
  case vsgSFVec3f:
    return print_SFVec3f (s, &val->SFVec3f, props, indent);
  default:
    return 0;
  }
}

static int
equalMFGeneric (MFGeneric const *mf0, MFGeneric const *mf1, int el_size)
{
  return (mf0
	  ? (mf0->mf.len == mf1->mf.len
	     && ! memcmp (mf0->data, mf1->data, mf0->mf.len * el_size))
	  : (mf1->mf.len == 0));
}

static int
equalAnyArg (vsgDataType type, vsgAnyArg const *v0, vsgAnyArg const *v1)
{
  switch (type) {
  case vsgMFColor:
    return equalMFGeneric (v0->MFGeneric, v1->MFGeneric, sizeof (SFColor));
  case vsgMFFloat:
    return equalMFGeneric (v0->MFGeneric, v1->MFGeneric, sizeof (SFFloat));
  case vsgMFInt32:
    return equalMFGeneric (v0->MFGeneric, v1->MFGeneric, sizeof (SFInt32));
  case vsgMFNode:
    return equalMFGeneric (v0->MFGeneric, v1->MFGeneric, sizeof (SFNode));
  case vsgMFRotation:
    return equalMFGeneric (v0->MFGeneric, v1->MFGeneric, sizeof (SFRotation));
  case vsgMFString:
    return equalMFGeneric (v0->MFGeneric, v1->MFGeneric, sizeof (SFString));
  case vsgMFVec2f:
    return equalMFGeneric (v0->MFGeneric, v1->MFGeneric, sizeof (SFVec2f));
  case vsgMFVec3f:
    return equalMFGeneric (v0->MFGeneric, v1->MFGeneric, sizeof (SFVec3f));
  case vsgSFBool:
    return v0->SFBool == v1->SFBool;
  case vsgSFColor:
    return ! memcmp (&v0->SFColor, &v1->SFColor, sizeof (SFColor));
  case vsgSFFloat:
    return v0->SFFloat == v1->SFFloat;
  case vsgSFImage:
    return ! memcmp (&v0->SFImage, &v1->SFImage, sizeof (SFImage));
  case vsgSFInt32:
    return v0->SFInt32 == v1->SFInt32;
  case vsgSFNode:
    return v0->SFNode == v1->SFNode;
  case vsgSFRotation:
    return ! memcmp (&v0->SFRotation, &v1->SFRotation, sizeof (SFRotation));
  case vsgSFString:
    return !strcmp (v0->SFString, v1->SFString);
  case vsgSFTime:
    return v0->SFTime == v1->SFTime;
  case vsgSFVec2f:
    return ! memcmp (&v0->SFVec2f, &v1->SFVec2f, sizeof (SFVec2f));
  case vsgSFVec3f:
    return ! memcmp (&v0->SFVec3f, &v1->SFVec3f, sizeof (SFVec3f));
  default:
    return 0;
  }
}

static void
print_fieldValue (print_state *s,
		  SFNode node,
		  vsgClassField const *field,
		  plist_t props, int indent)
{
  vsgClassField *is_field = vsgHasIS (s->wrl, props, node, field->name);

  if (is_field) {
    indent1 (s, indent+2);
    fprintf (s->fp, "%s IS %s", field->name, is_field->name);
    newline1 (s);
  }

  if (field->fieldType == vsgTfield
      || field->fieldType == vsgTexposedField) {
    vsgAnyArg val;

    vsgGet (s->wrl, node, field->name, &val);

    if (! equalAnyArg (field->type, &field->dflt, &val)) {
      indent1 (s, indent+2);
      fprintf (s->fp, "%s ", field->name);
      print_Any (s, field->type, &val, props, indent+2);
      newline1 (s);
    }
  }
}

static int
print_SFNode (print_state *s, SFNode node, plist_t props, int indent)
{
  symbol_t defn;
  vsgClass const *cl;
  vsgClassField **fields;

  if (! node) {
    puts1 (s, "NULL");
    return 1;
  }

  /* Check for a DEF associated with this node. */
  indent1 (s, indent);

  if ((defn = vsgHasDEF (props, node))) {
    if (defined1 (s, node, defn)) {
      fprintf (s->fp, "USE %s", defn);
      advance1 (s);
      return 1;
    } else {
      fprintf (s->fp, "DEF %s ", defn);
      define1 (s, node, defn);
    }
  }

  cl = vsgNodeClass (node);
  fields = vsgGetFields (cl);

  fprintf (s->fp, "%s {", cl->name);
  advance1 (s);
  newline1 (s);

  if (cl->name == vsgNScript) {
    for ( ; *fields; ++fields) {
      vsgClassField const *field = *fields;
      if (field->name == vsgNurl
	  || field->name == vsgNdirectOutput
	  || field->name == vsgNmustEvaluate) {
	print_fieldValue (s, node, field, props, indent+2);
	
      } else {
	vsgClassField *is_field = vsgHasIS (s->wrl, props, node, field->name);
	if (is_field) {
	  indent1 (s, indent+2);
	  fprintf (s->fp, "%s %s %s IS %s",
		   vsgFieldTypeName[field->fieldType],
		   vsgDataTypeName[field->type], 
		   field->name, is_field->name);
	  newline1 (s);
	}
	print_fieldDecl (s, field, 1, props, indent+2);
	newline1 (s);
      }
    }
    
  } else {
    for ( ; *fields; ++fields)
      print_fieldValue (s, node, *fields, props, indent+2);
  }

  indent1 (s, indent);
  putc1 (s, '}');
  return 1;
}

static int
print_routes (print_state *s, vsgRouteLink const *routes, 
	      plist_t props, int indent)
{
  int ret;
  symbol_t fromName, toName;
  vsgRoute const *route;

  if (! routes)
    return 0;

  /* We recurse like this in order to print in the reverse order. */
  print_routes (s, routes->next, props, indent);

  route = routes->route;
  fromName = vsgHasDEF (props, route->from);
  toName = vsgHasDEF (props, route->to);

  indent1 (s, indent);
  fprintf (s->fp, "ROUTE %s.%s TO %s.%s", 
	   fromName, route->fromField, toName, route->toField);
  advance1 (s);
  newline1 (s);
  
  return 0;
}
