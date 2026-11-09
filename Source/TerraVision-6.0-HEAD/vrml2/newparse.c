/*
 * Name: newparse.c
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
 *   $Id: newparse.c,v 1.7.6.1 2002/07/30 22:26:12 reddy Exp $
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

#include <symtab.h>
#include <setjmp.h>
#include <stdarg.h>

#include "vrml2int.h"
#include "parse.h"

#if YYDEBUG
int vsgpDebug = 0;
#endif

#define EMPTY_TOKEN (-1)

#define FAIL   (0)
#define ACCEPT (1)
#define ERROR(p) return FAIL

typedef struct vsgParser {
  int yychar;
  YYSTYPE yylval;

  vsgWorld *wrl;
  vsgpLexer *lexer;

  jmp_buf errjmp;
} vsgParser;

static inline void
pushProto (vsgParser *p)
{
  vsglPushProto (p->wrl);
}

static inline void
pushNormal (vsgParser *p)
{
  vsglPushNormal (p->wrl);
}

static inline void
pop (vsgParser *p)
{
  vsglPop (p->wrl);
}

static inline int 
get_token (vsgParser *p, int yychar)
{
  if (yychar == EMPTY_TOKEN)
    yychar = p->yychar = vsgpLex (&p->yylval, p->wrl);
  return yychar;
}

static inline int
use_token (vsgParser *p)
{
  return p->yychar = EMPTY_TOKEN;
}

static inline int 
next_token (vsgParser *p)
{
  return get_token (p, use_token (p));
}

static inline int
get_id (vsgParser *p, int yychar)
{
  int ret;
  pushProto (p);
  ret = get_token (p, yychar);
  pop (p);
  return ret;
}

static inline int
next_id (vsgParser *p)
{
  return get_id (p, use_token (p));
}

static int vsgpStatements (vsgParser *p);
static int vsgpProtoStatement (vsgParser *p);
static int vsgpRouteStatement (vsgParser *p);
static int vsgpInterfaceDeclaration (vsgParser *p, int externproto,
				     SFNode script);

typedef void (*vsgpValueCB) (vsgWorld *wrl, void *closure, ...);
static int vsgpValue (vsgParser *p, vsgDataType type,
		      vsgpValueCB cb, void *closure);

int
vsgpParse (vsgWorld *wrl)
{
  vsgParser p_s, *p= &p_s;
  int ret;

  p->wrl = wrl;
  p->lexer = wrl->lexer;

  use_token (p);
  
  if (setjmp (p->errjmp) != 0)
    return 1;

  vsgStartWorld (p->wrl);

  ret = vsgpStatements (p);

  vsgEndWorld (p->wrl);

  return ret != ACCEPT;
}

static void
vsgpReportError (vsgWorld *wrl, char const *fmt, va_list ap)
{
  char const *ff = wrl->filename;

  if (ff && wrl->lexer) {
    int linenum = vsglLinenum (wrl);
    fprintf(stderr, "%s:%d: ERROR: ", ff, linenum);
  } else {
    fprintf(stderr, "ERROR: ");
  }

  vfprintf (stderr, fmt, ap);
  fprintf(stderr, "\n");
}

void
vsgError (vsgWorld *wrl, char const *fmt, ...)
{
  va_list ap;

  va_start (ap, fmt);
  vsgpReportError (wrl, fmt, ap);
  va_end (ap);
}

static void
vsgpError (vsgParser *p, char const *fmt, ...)
{
  va_list ap;
  va_start (ap, fmt);
  vsgpReportError (p->wrl, fmt, ap);
  va_end (ap);

  longjmp (p->errjmp, 1);
}

static int vsgpProtoStatements (vsgParser *p);

static inline vsgClass const *
builtinClass (vsgParser *p, symbol_t name, int tag)
{
  static vsgClass const *cls[tkn_END-tkn_START];
  int idx = tag - tkn_START;
  if (! cls[idx]) {
    cls[idx] = vsgGetClass (p->wrl, name);
    if (! cls[idx])
      vsgpError (p, "Class %s not defined.", name);
  }
  return cls[idx];
}

static inline SFNode
builtinNew (vsgParser *p, symbol_t name, int tag)
{
  vsgClass const *cls = builtinClass (p, name, tag);
  return (cls ? vsgCreateNode (p->wrl, cls) : NULL);
}

typedef struct {
  vsgEventInDesc evin;
  SFNode node;
  
} vsgpSetClosure;

static void
vsgpSetCB (vsgWorld *wrl, void *closure, ...)
{
  vsgpSetClosure *cl = closure;
  va_list ap;
  va_start (ap, closure);
  vsgVaDescSet (wrl, cl->node, cl->evin, ap);
  va_end (ap);
}

static int
vsgpNodeBodyElement (vsgParser *p, vsgClass const *cls, SFNode node)
{
  int yychar = get_token (p, p->yychar);
  if (yychar == tkIDENTIFIER
      || (yychar > tkf_START && yychar <= tkf_END)) {
    symbol_t fieldId = p->yylval.symbol;

    yychar = next_token (p);

    /* Check for IS declaration. */
    if (yychar == tkIS) {
      yychar = next_id (p);
      if (yychar != tkIDENTIFIER) {
	vsgpError (p, "Expect field name after %s IS", fieldId);
	ERROR (p);
      }
      use_token (p);

      vsgSetIS (p->wrl, node, fieldId, p->yylval.symbol);
      return ACCEPT;
    }

    /* Otherwise, we read a value and set the field. */
    {
      vsgEventInDesc evin = vsgiGet_setFunc (cls, fieldId);
      vsgpSetClosure cl;
      cl.evin = evin;
      cl.node = node;
      return vsgpValue (p, evin.type, vsgpSetCB, &cl);
    }
  } else if (vsgpProtoStatement (p) == ACCEPT) {
    return ACCEPT;
  } else if (vsgpRouteStatement (p) == ACCEPT) {
    return ACCEPT;
  } else {
    return FAIL;
  }
}

static int
vsgpNodeBody (vsgParser *p, vsgClass const *cls, SFNode node)
{
  while (1) {
    if (vsgpNodeBodyElement (p, cls, node) != ACCEPT)
      return ACCEPT;
  }
}

static int
vsgpScriptBodyElement (vsgParser *p, vsgClass *cls, SFNode script)
{
  if (vsgpNodeBodyElement (p, cls, script) == ACCEPT)
    return ACCEPT;
  else if (vsgpInterfaceDeclaration (p, 0, script) == ACCEPT)
    return ACCEPT;
  else
    return FAIL;
}

static int
vsgpScriptBody (vsgParser *p, vsgClass *cls, SFNode script)
{
  while (1) {
    if (vsgpScriptBodyElement (p, cls, script) != ACCEPT)
      return ACCEPT;
  }
}

static int
vsgpScriptNode (vsgParser *p, symbol_t def)
{
  int yychar = get_token (p, p->yychar);
  vsgClass *cls;
  SFNode script;

  if (yychar != tknSCRIPT)
    return FAIL;

  if ((yychar = next_token (p)) != '{') {
    vsgpError (p, "Script: Expecting '{'");
    ERROR (p);
  }
  use_token (p);

  cls = vsgStartScript (p->wrl);
  script = vsgCreateNode (p->wrl, cls);

  /* Name the node. */
  if (def)
    vsgDEF (p->wrl, def, script);

  if (vsgpScriptBody (p, cls, script) != ACCEPT) {
    vsgpError (p, "Script: Invalid body");
    ERROR (p);
  }

  if ((yychar = get_token (p, p->yychar)) != '}') {
    vsgpError (p, "Script: Expecting '}'");
    ERROR (p);
  }
  use_token (p);

  vsgEndScript (p->wrl);

  p->yylval.node = script;

  return ACCEPT;
}

static int
vsgpNode (vsgParser *p, symbol_t def)
{
  int yychar = get_token (p, p->yychar);
  vsgClass const *cls;
  SFNode node;

  if (vsgpScriptNode (p, def) == ACCEPT) {
    return ACCEPT;

  } else if (yychar > tkn_START  && yychar < tkn_END) {
    cls = builtinClass (p, p->yylval.symbol, yychar);

  } else if (yychar == tkIDENTIFIER) {
    cls = vsgGetClass (p->wrl, p->yylval.symbol);
    
  } else {
    return FAIL;
  }

  /* Create the node. */
  node = vsgCreateNode (p->wrl, cls);
  if ( ! node ) return FAIL;

  /* Name the node. */
  if (def)
    vsgDEF (p->wrl, def, node);

  if ((yychar = next_token (p)) != '{') {
    vsgpError (p, "%s: Expecting '{'", cls->name);
    ERROR (p);
  }
  use_token (p);

  if (vsgpNodeBody (p, cls, node) != ACCEPT) {
    vsgpError (p, "%s: Invalid body", cls->name);
    ERROR (p);
  }

  if ((yychar = get_token (p, p->yychar)) != '}') {
    vsgpError (p, "%s: Expecting '}'", cls->name);
    ERROR (p);
  }
  use_token (p);

  p->yylval.node = node;

  return ACCEPT;
}

static int
vsgpNodeStatement1 (vsgParser *p, int root)
{
  vsgWorld *wrl = p->wrl;
  int yychar = get_token (p, p->yychar);
  symbol_t def = 0;

  if (yychar == tkDEF) {
    yychar = next_id (p);
    if (yychar != tkIDENTIFIER)
      vsgpError (p, "Parsing DEF, expecting name.");
    use_token (p);

    /* Set the def value to attach to the node. */
    def = p->yylval.symbol;

  } else if (! root && yychar == tkUSE) {
    yychar = next_id (p);
    if (yychar != tkIDENTIFIER)
      vsgpError (p, "Parsing USE, expecting name.");

    use_token (p);
    p->yylval.node = vsgUSE (wrl, p->yylval.symbol);
    return ACCEPT;

  } 
    
  return vsgpNode (p, def);
}

static inline int 
vsgpNodeStatement (vsgParser *p)
{
  return vsgpNodeStatement1 (p, 0);
}

static inline int 
vsgpRootNodeStatement (vsgParser *p)
{
  return vsgpNodeStatement1 (p, 1);
}

static vsgDataType
vsgpDataType (int yychar)
{
  switch (yychar) {
  case tkMFCOLOR:
    return vsgMFColor;
  case tkMFFLOAT:
    return vsgMFFloat;
  case tkMFINT32:
    return vsgMFInt32;
  case tkMFNODE:
    return vsgMFNode;
  case tkMFROTATION:
    return vsgMFRotation;
  case tkMFSTRING:
    return vsgMFString;
  case tkMFVEC2F:
    return vsgMFVec2f;
  case tkMFVEC3F:
    return vsgMFVec3f;
  case tkSFBOOL:
    return vsgSFBool;
  case tkSFCOLOR:
    return vsgSFColor;
  case tkSFFLOAT:
    return vsgSFFloat;
  case tkSFIMAGE:
    return vsgSFImage;
  case tkSFINT32:
    return vsgSFInt32;
  case tkSFNODE:
    return vsgSFNode;
  case tkSFROTATION:
    return vsgSFRotation;
  case tkSFSTRING:
    return vsgSFString;
  case tkSFTIME:
    return vsgSFTime;
  case tkSFVEC2F:
    return vsgSFVec2f;
  case tkSFVEC3F:
    return vsgSFVec3f;
  default:
    return vsgErrorType;
  }
}

typedef struct {
  vsgDataType type;
  symbol_t name;
} vsgpDefaultClosure;

static void
vsgpDefaultCB (vsgWorld *wrl, void *closure, ...)
{
  vsgpDefaultClosure *cl = closure;
  va_list ap;
  va_start (ap, closure);
  vsgVaDeclareFieldDefault (wrl, cl->type, cl->name, ap);
  va_end (ap);
}

static int
vsgpInterfaceDeclaration (vsgParser *p, int externproto, SFNode script)
{
  int yychar = get_token (p, p->yychar);
  vsgFieldType ftype;
  vsgDataType type;
  symbol_t name;

  switch (yychar) {
  default:
    return FAIL;

  case tkEVENTIN:
  case tkEVENTOUT:
  case tkEXPOSEDFIELD:
  case tkFIELD:
    ftype = (yychar == tkEVENTIN ? vsgTeventIn 
	     : yychar == tkEVENTOUT ? vsgTeventOut
	     : yychar == tkFIELD ? vsgTfield : vsgTexposedField);
    yychar = next_token (p);
    if (yychar < tkMETA_START)
      vsgpError (p, "Parsing PROTO interface, expecting type name.");
    type = vsgpDataType (yychar);
    
    yychar = next_id (p);
    if (yychar != tkIDENTIFIER)
      vsgpError (p, "Parsing PROTO interface, expecting field name.");
    name = p->yylval.symbol;
    yychar = use_token (p);

    if (script && ftype == vsgTexposedField) 
      vsgpError (p, "Cannot declare exposedField in Script node.");

    vsgDeclareField (p->wrl, ftype, type, name);

    if (script) {
      yychar = get_token (p, yychar);
      if (yychar == tkIS) {
	yychar = next_id (p);
	if (yychar != tkIDENTIFIER)
	  vsgpError (p, "Parsing IS declaration, expecting field name.");
	use_token (p);
	vsgSetIS (p->wrl, script, name, p->yylval.symbol);
	return ACCEPT;
      }
    }

    if (! externproto 
	&& (ftype == vsgTfield || ftype == vsgTexposedField)) {
      vsgpDefaultClosure cl;
      cl.type = type;
      cl.name = name;
      
      if (vsgpValue (p, type, vsgpDefaultCB, &cl) != ACCEPT)
	ERROR (p);
    }

    break;
  }

  return ACCEPT;
}

static int
vsgpInterfaceDeclarations (vsgParser *p, int externproto)
{
  while (1) {
    if (vsgpInterfaceDeclaration (p, externproto, NULL) == FAIL)
      return ACCEPT;
  }
}

static int
vsgpProtoBody (vsgParser *p)
{
  if (vsgpProtoStatements (p) != ACCEPT)
    return FAIL;

  if (vsgpRootNodeStatement (p) != ACCEPT)
    return FAIL;
  vsgAppendSFNode (p->wrl, vsgiCurrNodes (p->wrl), p->yylval.node);

  return vsgpStatements (p);
}

static int
vsgpProto (vsgParser *p)
{
  int yychar = get_token (p, p->yychar);
  vsgClass *proto;

  if (yychar != tkPROTO)
    return FAIL;

  yychar = next_id (p);
  if (yychar != tkIDENTIFIER)
    vsgpError (p, "Parsing PROTO: Expecting name.");

  proto = (vsgClass *) vsgStartProto (p->wrl, p->yylval.symbol);

  yychar = next_token (p);
  if (yychar != '[')
    vsgpError (p, "Parsing PROTO: Expecting [.");
  use_token (p);

  if (vsgpInterfaceDeclarations (p, 0) != ACCEPT)
    return FAIL;

  yychar = get_token (p, p->yychar);
  if (yychar != ']')
    vsgpError (p, "Parsing PROTO: Expecting ].");

  vsgEndProtoFields (p->wrl);

  yychar = next_token (p);
  if (yychar != '{')
    vsgpError (p, "Parsing PROTO: Expecting {.");
  use_token (p);

  if (vsgpProtoBody (p) != ACCEPT)
    return FAIL;
  
  vsgDeclareNodes (p->wrl, vsgWorldNodes (p->wrl));

  yychar = get_token (p, p->yychar);
  if (yychar != '}')
    vsgpError (p, "Parsing PROTO: Expecting }.");
  use_token (p);

  vsgEndProto (p->wrl);

  return ACCEPT;
}

static void
vsgpDeclareExternCB (vsgWorld *wrl, void *closure, ...)
{
  va_list ap;
  va_start (ap, closure);
  vsgDeclareExtern (wrl, va_arg (ap, MFString *));
  va_end (ap);
}

static int
vsgpExternProto (vsgParser *p)
{
  int yychar = get_token (p, p->yychar);
  vsgClass *proto;

  if (yychar != tkEXTERNPROTO)
    return FAIL;

  yychar = next_id (p);
  if (yychar != tkIDENTIFIER) {
    vsgpError (p, "Parsing EXTERNPROTO: Expecting name.");
    ERROR (p);
  }

  proto = (vsgClass *) vsgStartExternProto (p->wrl, p->yylval.symbol);

  yychar = next_token (p);
  if (yychar != '[')
    vsgpError (p, "Parsing EXTERNPROTO: Expecting [.");
  use_token (p);

  if (vsgpInterfaceDeclarations (p, 1) != ACCEPT)
    return FAIL;

  yychar = get_token (p, p->yychar);
  if (yychar != ']')
    vsgpError (p, "Parsing EXTERNPROTO: Expecting ].");
  use_token (p);

  if (vsgpValue (p, vsgMFString, vsgpDeclareExternCB, NULL) != ACCEPT)
    vsgpError (p, "Parsing EXTERNPROTO: Expecting urls."); 

  vsgEndExternProto (p->wrl);

  return ACCEPT;
}

static int
vsgpProtoStatement (vsgParser *p)
{
  if (vsgpProto (p) == ACCEPT)
    return ACCEPT;
  else if (vsgpExternProto (p) == ACCEPT)
    return ACCEPT;
  else
    return FAIL;
}

static int
vsgpProtoStatements (vsgParser *p)
{
  while (1) {
    if (vsgpProtoStatement (p) == FAIL)
      return ACCEPT;
  }
}

static int
vsgpRouteStatement (vsgParser *p)
{
  int yychar = get_token (p, p->yychar);
  symbol_t fromId, fromOutId;
  symbol_t toId, toInId;
  SFNode from, to;

  if (yychar != tkROUTE)
    return FAIL;
  
  yychar = next_id (p);
  if (yychar != tkIDENTIFIER)
    vsgpError (p, "Parsing ROUTE: Expecting from identifier.");
  fromId = p->yylval.symbol;

  from = vsgUSE (p->wrl, fromId);
  if (! from)
    vsgError (p->wrl, "Unable to find named node '%s'", fromId);

  yychar = next_token (p);
  if (yychar != '.')
    vsgpError (p, "Parsing ROUTE: Expecting %s.fieldId.", fromId);

  yychar = next_id (p);
  if (yychar != tkIDENTIFIER)
    vsgpError (p, "Parsing ROUTE: Expecting %s.fieldId.", fromId);
  fromOutId = p->yylval.symbol;

  yychar = next_token (p);
  if (yychar != tkTO)
    vsgpError (p, "Parsing ROUTE: Expecting %s.%s TO ...", fromId, fromOutId);

  yychar = next_id (p);
  if (yychar != tkIDENTIFIER)
    vsgpError (p, "Parsing ROUTE: Expecting to identifier.");
  toId = p->yylval.symbol;

  to = vsgUSE (p->wrl, toId);
  if (! from)
    vsgError (p->wrl, "Unable to find named node '%s'", toId);

  yychar = next_token (p);
  if (yychar != '.')
    vsgpError (p, "Parsing ROUTE: Expecting %s.fieldId.", toId);

  yychar = next_id (p);
  if (yychar != tkIDENTIFIER)
    vsgpError (p, "Parsing ROUTE: Expecting %s.fieldId.", toId);
  toInId = p->yylval.symbol;
  use_token (p);

  if (from && to)
    vsgAddRoute (p->wrl, from, fromOutId, to, toInId);

  return ACCEPT;
}

static int
vsgpStatement (vsgParser *p)
{
  if (vsgpNodeStatement (p) == ACCEPT) {
    vsgAppendSFNode (p->wrl, vsgiCurrNodes (p->wrl), p->yylval.node);
    return ACCEPT;
  } else if (vsgpProtoStatement (p) == ACCEPT) {
    return ACCEPT;
  } else if (vsgpRouteStatement (p) == ACCEPT) {
    return ACCEPT;
  }

  return FAIL;
}

static int
vsgpStatements (vsgParser *p)
{
  while (1) {
    if (vsgpStatement (p) == FAIL)
      return ACCEPT;
  }
}

#define GET_INTEGER(v)				\
  do {						\
    yychar = get_token (p, yychar);		\
    if (yychar == tkNUMBER) {			\
      (v) = p->yylval.intval;			\
      yychar = use_token (p);			\
    } else					\
      return FAIL;				\
  } while (0)

#define GET_FLOAT(v)				\
  do {						\
    yychar = get_token (p, yychar);		\
    if (yychar == tkNUMBER) {			\
      (v) = p->yylval.intval;			\
      yychar = use_token (p);			\
    } else if (yychar == tkFLOAT) {		\
      (v) = p->yylval.floatval;			\
      yychar = use_token (p);			\
    } else					\
      return FAIL;				\
  } while (0)

static int vsgpMFValue (vsgParser *p, vsgDataType type, 
			vsgpValueCB cb, void *closure, int seen_bracket);

static int
vsgpValue (vsgParser *p, vsgDataType type, vsgpValueCB cb, void *closure)
{
  vsgWorld *wrl = p->wrl;
  int yychar = p->yychar;

  switch (type) {
  case vsgSFBool:
    yychar = get_token (p, yychar);
    if (yychar == tkTRUE)
      (*cb) (wrl, closure, TRUE);
    else if (yychar == tkFALSE)
      (*cb) (wrl, closure, FALSE);
    else
      return FAIL;
    yychar = use_token (p);
    return ACCEPT;

  case vsgSFColor: {
    SFColor c;
    GET_FLOAT (c.r);
    GET_FLOAT (c.g);
    GET_FLOAT (c.b);
    (*cb) (wrl, closure, c);
    return ACCEPT;
  }

  case vsgSFFloat: {
    static SFFloat flt;
    GET_FLOAT (flt);
    (*cb) (wrl, closure, flt);
    return ACCEPT;
  }

  case vsgSFImage: {
    SFImage *im;
    int w, h, d, n;
    GET_INTEGER (w);
    GET_INTEGER (h);
    GET_INTEGER (d);
    im = vsgStartSFImage (wrl, w, h, d);
    for (n = w*h; n; --n) {
      GET_INTEGER (d);
      vsgAppendPixel (wrl, im, d);
    }
    (*cb) (wrl, closure, *im);
    return ACCEPT;
  }

  case vsgSFInt32: {
    SFInt32 v;
    GET_INTEGER (v);
    (*cb) (wrl, closure, v);
    return ACCEPT;
  }

  case vsgSFNode: {
    yychar = get_token (p, yychar);
    if (yychar == tkNULL) {
      p->yylval.node = NULL;
      use_token (p);
    } else if (vsgpNodeStatement (p) != ACCEPT)
      return FAIL;
    (*cb) (wrl, closure, p->yylval.node);
    return ACCEPT;
  }

  case vsgSFRotation: {
    SFRotation r;
    GET_FLOAT (r.a);
    GET_FLOAT (r.b);
    GET_FLOAT (r.c);
    GET_FLOAT (r.d);
    (*cb) (wrl, closure, r);
    return ACCEPT;
  }

  case vsgSFString: {
    yychar = get_token (p, yychar);
    if (yychar == tkVRMLSTRING)
      (*cb) (wrl, closure, p->yylval.string);
    else
      return FAIL;
    yychar = use_token (p);
    return ACCEPT;
  }

  case vsgSFTime: {
    static SFTime time;
    GET_FLOAT (time);
    (*cb) (wrl, closure, time);
    return ACCEPT;
  }

  case vsgSFVec2f: {
    SFVec2f v;
    GET_FLOAT (v.v[0]);
    GET_FLOAT (v.v[1]);
    (*cb) (wrl, closure, v);
    return ACCEPT;
  }

  case vsgSFVec3f: {
    SFVec3f v;
    GET_FLOAT (v.v[0]);
    GET_FLOAT (v.v[1]);
    GET_FLOAT (v.v[2]);
    (*cb) (wrl, closure, v);
    return ACCEPT;
  }

  case vsgMFColor:
  case vsgMFFloat:
  case vsgMFInt32:
  case vsgMFNode:
  case vsgMFRotation:
  case vsgMFString:
  case vsgMFVec2f:
  case vsgMFVec3f:
    return vsgpMFValue (p, type, cb, closure, FALSE);

  default:
    return FAIL;
  }
}

static int
vsgpMFValue (vsgParser *p, vsgDataType type, 
	     vsgpValueCB cb, void *closure, int seen_bracket)
{
  typedef void *(*MFStartFunc_t) (vsgWorld *wrl);
  typedef void *(*MFEndFunc_t) (vsgWorld *wrl, void *mf);

  MFStartFunc_t startf;
  MFEndFunc_t endf;
  vsgpValueCB el_cb;
  vsgDataType el_type;
  void *mf;
  int yychar = p->yychar;
  vsgWorld *wrl = p->wrl;

  switch (type) {
  case vsgMFColor:
    startf = (MFStartFunc_t) vsgStartMFColor;
    endf = (MFEndFunc_t) vsgEndMFColor;
    el_type = vsgSFColor;
    el_cb = (vsgpValueCB) vsgAppendSFColor;
    break;
  case vsgMFFloat:
    startf = (MFStartFunc_t) vsgStartMFFloat;
    endf = (MFEndFunc_t) vsgEndMFFloat;
    el_type = vsgSFFloat;
    el_cb = (vsgpValueCB) vsgAppendSFFloat;
    break;
  case vsgMFInt32:
    startf = (MFStartFunc_t) vsgStartMFInt32;
    endf = (MFEndFunc_t) vsgEndMFInt32;
    el_type = vsgSFInt32;
    el_cb = (vsgpValueCB) vsgAppendSFInt32;
    break;
  case vsgMFNode:
    startf = (MFStartFunc_t) vsgStartMFNode;
    endf = (MFEndFunc_t) vsgEndMFNode;
    el_type = vsgSFNode;
    el_cb = (vsgpValueCB) vsgAppendSFNode;
    break;
  case vsgMFRotation:
    startf = (MFStartFunc_t) vsgStartMFRotation;
    endf = (MFEndFunc_t) vsgEndMFRotation;
    el_type = vsgSFRotation;
    el_cb = (vsgpValueCB) vsgAppendSFRotation;
    break;
  case vsgMFString:
    startf = (MFStartFunc_t) vsgStartMFString;
    endf = (MFEndFunc_t) vsgEndMFString;
    el_type = vsgSFString;
    el_cb = (vsgpValueCB) vsgAppendSFString;
    break;
  case vsgMFVec2f:
    startf = (MFStartFunc_t) vsgStartMFVec2f;
    endf = (MFEndFunc_t) vsgEndMFVec2f;
    el_type = vsgSFVec2f;
    el_cb = (vsgpValueCB) vsgAppendSFVec2f;
    break;
  case vsgMFVec3f:
    startf = (MFStartFunc_t) vsgStartMFVec3f;
    endf = (MFEndFunc_t) vsgEndMFVec3f;
    el_type = vsgSFVec3f;
    el_cb = (vsgpValueCB) vsgAppendSFVec3f;
    break;

  default:
    return FAIL;
  }

  mf = (*startf) (wrl);
  yychar = get_token (p, yychar);
  if (seen_bracket || yychar == '[') {
    if (! seen_bracket)
      yychar = next_token (p);

    while (yychar != ']') {
      if (vsgpValue (p, el_type, el_cb, mf) == FAIL) {
	(*endf) (wrl, mf);
	return FAIL;
      }
      yychar = get_token (p, p->yychar);
     }
    yychar = use_token (p);
  } else {
    if (vsgpValue (p, el_type, el_cb, mf) == FAIL) {
      (*endf) (wrl, mf);
      return FAIL;
    }
  }

  (*endf) (wrl, mf);
  (*cb) (wrl, closure, mf);
  return ACCEPT;
}
