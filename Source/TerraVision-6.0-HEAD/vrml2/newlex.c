/*
 * Name: newlex.c
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
 *   $Id: newlex.c,v 1.13 2000/12/13 16:37:47 leei Exp $
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
#include <stdio.h>
#include <ctype.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "vrml2int.h"
#include "parse.h"
#include "syms.c"

#ifdef HAVE_MMAP
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#endif

typedef enum { INITIAL_s, PROTO_s, META_s } lex_state_t;
static char const *lex_state_name[] = {"initial", "proto", "meta"};

struct vsgpLexer {
  void (*cleanup) (vsgpLexer *clean, void *closure);

  int (*input) (void *closure);
  void *closure;

  int c;
  int linenum;

  lex_state_t state;
  lex_state_t *stack;
  int stack_size;
  int stack_top;

  int debug;
  istack *tmp;
};

#define CHAR_USED (EOF-1)

#define ID_FIRST (1)
#define ID_REST (2)

#define isIdFirst(c) (!!(idChar[c]&ID_FIRST))
#define isIdRest(c) (!!(idChar[c]&ID_REST))

#if 1
static char idChar[256] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 3, 0, 0, 3, 3, 3, 0, 3, 3, 3, 3, 0, 3, 0, 3, 
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 0, 0, 0, 3, 3, 
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 0, 3, 0, 3, 0, 
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3
};

#define INIT()

#else
static char idChar[256];

static void
init_idChar (void)
{
  static int inited = 0;
  if (! inited) {
    int i;
    for (i = 0; i < 256; ++i) {
      idChar[i] = ID_FIRST | ID_REST;
      if (i <= 0x20
	  || i == 0x22 || i == 0x23 || i == 0x27 || i == 0x2c || i == 0x2e
	  || i == 0x5b || i == 0x5c || i == 0x5d
	  || i == 0x7b || i == 0x7d || i == 0x7f)
	idChar[i] &= ~(ID_FIRST|ID_REST);
      else if (i >= 0x30 && i <= 0x39)
	idChar[i] &= ~ID_FIRST;
    }

    printf ("static char idChar[256] = {");
    for (i = 0; i < 255; ++i) {
      if (i%16 == 0) printf ("\n  ");
      printf ("%d, ", idChar[i]);
    }
    printf ("%d\n};\n", idChar[i]);
    inited = 1;
  }
}

#define INIT() (init_idChar ())

#endif

static void
init_vsgpLexer (vsgpLexer *lexer,
		 int (*input) (void *closure), void *closure,
		 void (*cleanup) (vsgpLexer *l, void *closure))
{
#if YYDEBUG
  lexer->debug = vsgpDebug;
#else
  lexer->debug = 0;
#endif
  lexer->linenum = 1; 
  lexer->stack_size = 16;
  lexer->stack = (lex_state_t *)
    malloc (sizeof(lex_state_t) * lexer->stack_size);
  lexer->stack_top = 0;
  lexer->state = INITIAL_s;
  lexer->c = CHAR_USED;
  
  lexer->input = input;
  lexer->closure = closure;
  lexer->cleanup = cleanup;

  lexer->tmp = istack_create (4096);

  INIT ();
}

void
vsglDestroy (vsgpLexer *l)
{
  if (l->cleanup)
    (*l->cleanup) (l, l->closure);
  istack_destroy (l->tmp);

  free (l->stack);
}

static void
pushStack (vsgpLexer *lexer, lex_state_t state)
{
  if (lexer->stack_top == lexer->stack_size) {
    lexer->stack_size *= 2;
    lexer->stack = (lex_state_t *)
      realloc (lexer->stack, sizeof(lex_state_t) * lexer->stack_size);
  }
  lexer->stack[lexer->stack_top++] = lexer->state;
  lexer->state = state;
  if (lexer->debug)
    fprintf (stderr, "-- %d: push -> {%s}\n",
	     lexer->linenum, lex_state_name[lexer->state]);

}

static void
popStack (vsgpLexer *lexer)
{
  if (lexer->stack_top)
    lexer->state = lexer->stack[--lexer->stack_top];
  else
    lexer->state = INITIAL_s;
  if (lexer->debug)
    fprintf (stderr, "-- %d: pop -> {%s}\n",
	     lexer->linenum, lex_state_name[lexer->state]);

}

void
vsglPop (vsgWorld* wrl)
{
  popStack (wrl->lexer);
}

void
vsglPushNormal (vsgWorld* wrl)
{
  pushStack (wrl->lexer, INITIAL_s);
}

void
vsglPushMeta (vsgWorld* wrl)
{
  pushStack (wrl->lexer, META_s);
}

void
vsglPushProto (vsgWorld* wrl)
{
  pushStack (wrl->lexer, PROTO_s);
}

int
vsglLinenum (vsgWorld* wrl)
{
  return wrl->lexer->linenum;
}

static int
next_char (vsgpLexer *lexer)
{
  int c = lexer->c;
  if (c == CHAR_USED)
    lexer->c = c = (*lexer->input) (lexer->closure);
  return c;
}

static void
use_char (vsgpLexer *lexer)
{
  lexer->c = CHAR_USED;
}

static void
pushback (vsgpLexer *lexer, int c)
{
  lexer->c = c;
}

static void
whitespace (vsgpLexer *lexer)
{
  int c;
  if (lexer->debug > 1)
    fprintf (stderr, "-- %d: WHITESPACE\n", lexer->linenum);
  while ((c = next_char (lexer)) != EOF) {
    if (c == '\n')
      ++lexer->linenum;
    else if (! isspace (c))
      break;
    use_char (lexer);
  }
}

static int
seen_exponent (vsgWorld* wrl, YYSTYPE *lvalp, int mult)
{
  vsgpLexer *lexer = wrl->lexer;
  istack *ist = lexer->tmp;
  int c;
  char *num;

  if ((c = next_char (lexer)) == '-' || c == '+') {
    istack_1grow (ist, c);
    use_char (lexer);
  }
  while ((c = next_char (lexer)) != EOF && isdigit (c)) {
    istack_1grow (ist, c);
    use_char (lexer);
  }

  istack_1grow (ist, '\0');  num = istack_finish (ist);
  lvalp->floatval = mult * atof (num);
  if (lexer->debug)
    fprintf (stderr, "-- %d: FLOAT %g: %s%s\n",
	     lexer->linenum, lvalp->floatval, mult<0 ? "-" : "", num);
  istack_unwind (ist, num);
  return tkFLOAT;
}

static int
seen_decimal_point (vsgWorld* wrl, YYSTYPE *lvalp, int mult)
{
  vsgpLexer *lexer = wrl->lexer;
  istack *ist = lexer->tmp;
  int c;
  char *num;

  while ((c = next_char (lexer)) != EOF && isdigit (c)) {
    istack_1grow (ist, c);
    use_char (lexer);
  }

  /* Possible exponent. */
  if (c == 'e' || c == 'E') {
    istack_1grow (ist, c);
    use_char (lexer);
    return seen_exponent (wrl, lvalp, mult);
  }

  istack_1grow (ist, '\0');
  num = istack_finish (ist);
  lvalp->floatval = mult * atof (num);
  if (lexer->debug)
    fprintf (stderr, "-- %d: FLOAT %g: %s%s\n",
	     lexer->linenum, lvalp->floatval, mult<0 ? "-" : "", num);
  istack_unwind (ist, num);
  return tkFLOAT;
}

static int
octal_or_float (vsgWorld* wrl, YYSTYPE *lvalp, int mult)
{
  vsgpLexer *lexer = wrl->lexer;
  istack *ist = lexer->tmp;
  int c;
  char *num;

  while ((c = next_char (lexer)) != EOF && isdigit (c)) {
    istack_1grow (ist, c);
    use_char (lexer);
  }
  
  if (c == '.') {
    istack_1grow (ist, c);
    use_char (lexer);
    return seen_decimal_point (wrl, lvalp, mult);
  }

  istack_1grow (ist, '\0');
  num = istack_finish (ist);
  lvalp->intval = (num[0] == '\0' ? 0 : mult * strtol (num, NULL, 8));

  if (lexer->debug)
    fprintf (stderr, "-- %d: NUMBER %#lo: %s0%s\n",
	     lexer->linenum, lvalp->intval, mult<0 ? "-" : "", num);
  istack_unwind (ist, num);
  return tkNUMBER;
}

static int
hex (vsgWorld* wrl, YYSTYPE *lvalp, int mult)
{
  vsgpLexer *lexer = wrl->lexer;
  istack *ist = lexer->tmp;
  int c;
  char *num;

  while ((c = next_char (lexer)) != EOF && isxdigit (c)) {
    istack_1grow (ist, c);
    use_char (lexer);
  }

  istack_1grow (ist, '\0');
  num = istack_finish (ist);
  lvalp->intval = mult * strtol (num, NULL, 16);
  if (lexer->debug)
    fprintf (stderr, "-- %d: NUMBER %#lx: %s0x%s\n",
	     lexer->linenum, lvalp->intval, mult<0 ? "-" : "", num);
  istack_unwind (ist, num);
  return tkNUMBER;
}

static int
number (vsgWorld* wrl, YYSTYPE *lvalp, int mult)
{
  vsgpLexer *lexer = wrl->lexer;
  istack *ist = lexer->tmp;
  int c;
  char *num;
  
  /* Check for alternate base (octal or hexadecimal). */
  if ((c = next_char (lexer)) == '0') {
    use_char (lexer);
    if ((c = next_char (lexer)) == 'x' || c == 'X') {
      use_char (lexer);
      return hex (wrl, lvalp, mult);
    } else {
      return octal_or_float (wrl, lvalp, mult);
    }
  }

  /* Read integer part. */
  while ((c = next_char (lexer)) != EOF && isdigit (c)) {
    istack_1grow (ist, c);
    use_char (lexer);
  }

  /* Check for decimal point or start of exponent. */
  if ((c = next_char (lexer)) == '.') {
    istack_1grow (ist, c);
    use_char (lexer);
    return seen_decimal_point (wrl, lvalp, mult);

  } else if (c == 'e' || c == 'E') {
    istack_1grow (ist, c);
    use_char (lexer);
    return seen_exponent (wrl, lvalp, mult);

  } else {
    istack_1grow (ist, '\0');
    num = istack_finish (ist);
    lvalp->intval = mult * atoi (num);
    if (lexer->debug)
      fprintf (stderr, "-- %d: NUMBER %ld: %s%s\n",
	       lexer->linenum, lvalp->intval, mult<0 ? "-" : "", num);
    istack_unwind (ist, num);
    return tkNUMBER;
  }
}

int
vsgpLex (YYSTYPE *lvalp, vsgWorld* wrl)
{
  vsgpLexer *lexer = wrl->lexer;
  istack *temp_ist = lexer->tmp;
  int c;
  
 retry:
  c = next_char (lexer);

  switch (c) {
  case EOF:
    return 0;

  case ' ':
  case '\t':
  case '\r':
  case '\f':
  case ',': /* This is the strange VRML one. */
    use_char (lexer);
    whitespace (lexer);
    goto retry; /* Tail call. */

  case '#': /* Comment start. */
    if (lexer->linenum == 1) {
      istack *ist = wrl->data;
      char *filetype;
      while ((c = next_char (lexer)) != EOF && c != '\n') {
	istack_1grow (ist, c);
	use_char (lexer);
      }
      istack_1grow (ist, '\0');
      filetype = istack_finish (ist);
      if (lexer->debug)
	fprintf (stderr, "-- %d: FILETYPE %s\n", lexer->linenum, filetype);
      vsgSetFiletype (wrl, filetype);
      if (c == '\n') {
	++lexer->linenum;
	use_char (lexer);
      }
    } else {
      if (lexer->debug)
	fprintf (stderr, "-- %d: COMMENT\n", lexer->linenum);
      while ((c = next_char (lexer)) != EOF && c != '\n')
	use_char (lexer);
      if (c == '\n') {
	++lexer->linenum;
	use_char (lexer);
      }
    }
    goto retry; /* Tail call. */
    
  case '\n':
    ++lexer->linenum;
    use_char (lexer);
    whitespace (lexer);
    goto retry; /* Tail call. */

  case '"': {
    istack *ist = wrl->data;
    int linenum = lexer->linenum;
    char *curr_ist = 0;
    size_t curr_len;

    /* This preserves a partially constructed object in data istack. */
    if ((curr_len = istack_object_size (ist)) > 0) {
      char *partial = istack_finish (ist);
      curr_ist = alloca (curr_len);
      memcpy (curr_ist, partial, curr_len);
    }

    use_char (lexer);
    while ((c = next_char (lexer)) != '"' && c != EOF) {
      if (c == '\n')
	++lexer->linenum;
      else if (c == '\\') {
	use_char (lexer);
	if ((c = next_char (lexer)) == EOF) {
	  lexer->linenum = linenum;
	  vsgError (wrl, "unterminated string");
	  istack_unwind (ist, istack_finish (ist));
	  if (curr_ist)
	    istack_grow (ist, curr_ist, curr_len);
	  return 0;
	}
      }
      istack_1grow (ist, c);
      use_char (lexer);
    }

    if (c == EOF) {
      lexer->linenum = linenum;
      vsgError (wrl, "unterminated string");
      istack_unwind (ist, istack_finish (ist));
      if (curr_ist)
	istack_grow (ist, curr_ist, curr_len);
      return 0;
    }

    use_char (lexer);
    istack_1grow (ist, '\0');
    lvalp->string = istack_finish (ist);
    if (lexer->debug)
      fprintf (stderr, "-- %d: STRING \"%s\"\n",
	       lexer->linenum, lvalp->string);
    if (curr_ist)
      istack_grow (ist, curr_ist, curr_len);
     return tkVRMLSTRING;
  }

  case '+':
  case '-': {
    char sign = c;
    use_char (lexer);
    if ((c = next_char (lexer)) == EOF) {
      vsgError (wrl, "trailing '%c' at EOF", sign);
      return 0;
    } else if (! isdigit (c) && c != '.') {
      vsgError (wrl, "dangling '%c'", sign);
      use_char (lexer);
      goto retry;
    }
    return number (wrl, lvalp, (sign == '-' ? -1 : 1));
  }

  case '0': case '1': case '2': case '3': case '4':
  case '5': case '6': case '7': case '8': case '9':
    return number (wrl, lvalp, 1);

  case '.': {
    use_char (lexer);
    c = next_char (lexer);
    if (c != EOF && isdigit (c)) {
      istack_1grow (temp_ist, '.');
      istack_1grow (temp_ist, c);
      use_char (lexer);
      return seen_decimal_point (wrl, lvalp, 1);
    } else {
      if (lexer->debug)
	fprintf (stderr, "-- %d: '%c'\n", lexer->linenum, '.');
      return '.';
    }
  }

  case '{':
  case '}':
  case '[':
  case ']':
    use_char (lexer);
    if (lexer->debug)
      fprintf (stderr, "-- %d: '%c'\n", lexer->linenum, c);
    return c;

  default: {
      if (isIdFirst (c)) {
	/* We gather these symbols in the istack managed by the symbol
	   table.  That way, their persistence is tied to that of the symbol
	   table (as we wish) and no copying is needed! */
	istack *ist = symtab_istack (wrl->symtab);
	struct vsgpId *p;
	char *id; int len = 1;
	istack_1grow (ist, c);
	use_char (lexer);
	while ((c = next_char (lexer)) != EOF && isIdRest (c)) {
	  istack_1grow (ist, c);
	  ++len;
	  use_char (lexer);
	}
	istack_1grow (ist, '\0');
	id = istack_finish (ist);

	if ((p = is_reserved_word (id, len))) {
	  switch (lexer->state) {
	  case INITIAL_s:
	    if (lexer->debug)
	      fprintf (stderr, "-- %d {%s}: TOKEN %d %s\n",
		       lexer->linenum, lex_state_name[lexer->state],
		       p->id, id);
	    istack_unwind (ist, id);
	    lvalp->symbol = p->name;
	    return p->id;

	  case META_s:
	    if (p->id < tkRESERVED_BARRIER 
		|| (p->id > tkMETA_START && p->id < tkMETA_END)) {
	      if (lexer->debug)
		fprintf (stderr, "-- %d {%s}: TOKEN %d %s\n",
			 lexer->linenum, lex_state_name[lexer->state],
			 p->id, id);
	      istack_unwind (ist, id);
	      lvalp->symbol = p->name;
	      return p->id;
	    }
	    break;

	  case PROTO_s:
	    if (p->id < tkRESERVED_BARRIER) {
	      if (lexer->debug)
		fprintf (stderr, "-- %d {%s}: TOKEN %d %s\n",
			 lexer->linenum, lex_state_name[lexer->state], 
			 p->id, id);
	      istack_unwind (ist, id);
	      lvalp->symbol = p->name;
	      return p->id;
	    }
	    break;

	  }
	}

	if (p) {
	  lvalp->symbol = p->name;
	} else {
	  /* We can do this, since we know that finding an old entry in the
	     symtab won't every extend the istack.  So unrolling the istack
	     when we find an old entry is perfectly safe. */
	  lvalp->symbol = sym_intern_nocopy (wrl->symtab, id);
	  if (lvalp->symbol != id)
	    istack_unwind (ist, id);
	}

	if (lexer->debug)
	  fprintf (stderr, "-- %d {%s}: IDENTIFIER '%s'\n",
		   lexer->linenum, lex_state_name[lexer->state], 
		   lvalp->symbol);
	return tkIDENTIFIER;
      }

      vsgError (wrl, "unrecognized char '%c'", c);
      use_char (lexer);
      goto retry;
    }
  }
}

symbol_t
vsgSymIntern (vsgWorld *wrl, char *str)
{
  struct vsgpId *id = is_reserved_word (str, strlen (str));
  if (id)
    return id->name;
  else
    return sym_intern (wrl->symtab, str);
}

static int
stream_input (void *v)
{
  FILE *f = v;
  return getc (f);
}

static void
stream_cleanup (vsgpLexer *l, void *closure)
{
  FILE *f = closure;
  fclose (f);
}

static int
valid_vrml97_file( FILE *f )
{
  char buffer[21];

  fgets( buffer, 20, f );
  rewind( f );

  return ( strncmp( buffer, "#VRML V2.0 utf8", 15 ) == 0 );
}

int
vsgParseStream (vsgWorld *wrl, FILE *f, char const *source)
{
  vsgpLexer lexer;

  if ( ! valid_vrml97_file(f) )
    return 1;

  init_vsgpLexer (wrl->lexer = &lexer, stream_input, f, 0);
  wrl->filename = source;

  return vsgpParse (wrl);
}

typedef struct {
  char const *p;
  char const *start, *end;
} string_closure;

static int
string_input (void *v)
{
  string_closure *cl = v;
  if (cl->end && cl->p >= cl->end)
    return EOF;
  else {
    int c = *cl->p++;
    return c ? c : (cl->end = cl->p, EOF);
  }
}

#ifdef HAVE_MMAP
static void
mmap_cleanup (vsgpLexer *l, void *closure)
{
  string_closure *cl = closure;
  munmap ((void *) cl->start, cl->end - cl->start);
}
#endif

int
vsgParseFile (vsgWorld *wrl, char const *filename)
{
  vsgpLexer lexer;
  FILE *f;

  if ((f = fopen(filename, "r")) == NULL) 
    return 1;

  if ( ! valid_vrml97_file(f) ) {
    fclose( f );
    return 1;
  }

#ifdef HAVE_MMAP
  {
    struct stat st_buf;
    char *p;
    if (fstat (fileno (f), &st_buf) != 0) {
      fprintf(stderr, "Cannot stat open data file %s\n", filename);
      fclose (f);
      return 1;
    }
    p = mmap (0, st_buf.st_size, PROT_READ, MAP_PRIVATE, fileno (f), 0);
    if ((void *) p != MAP_FAILED) {
      string_closure cl;

      cl.p = cl.start = p;
      cl.end = p+st_buf.st_size;

      fclose (f);

      init_vsgpLexer (wrl->lexer = &lexer, string_input, &cl, mmap_cleanup);
      wrl->filename = filename;

      return vsgpParse (wrl);
    }
  }
#endif

  init_vsgpLexer (wrl->lexer = &lexer, stream_input, f, stream_cleanup);
		   
  wrl->filename = filename;

  return vsgpParse (wrl);
}

int
vsgParseString (vsgWorld *wrl, char const *str)
{
  vsgpLexer lexer;
  string_closure cl;

  cl.p = cl.start = str;
  cl.end = 0;

  init_vsgpLexer (wrl->lexer = &lexer, string_input, &cl, 0);
  wrl->filename = "<STRING>";

  return vsgpParse (wrl);
}

int
vsgParseStringN (vsgWorld *wrl, char const *str, size_t len)
{
  vsgpLexer lexer;
  string_closure cl;

  cl.p = cl.start = str;
  cl.end = str+len;

  init_vsgpLexer (wrl->lexer = &lexer, string_input, &cl, 0);
  wrl->filename = "<STRING>";

  return vsgpParse (wrl);
}
