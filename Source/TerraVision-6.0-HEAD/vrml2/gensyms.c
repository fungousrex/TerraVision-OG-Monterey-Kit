/*
 * Name: gensyms.c
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
 *   $Id: gensyms.c,v 1.7 2000/08/28 23:21:35 reddy Exp $
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

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <values.h>

int trace = 0;

typedef struct {
  unsigned char *str;
  int len;
  char *def;
  char *const_name;
  unsigned char *index;
} triple;

#define HEADER_TOP(s)
#define SOURCE_TOP(s)
#define STRING_PREFIX(s)
#define FUNC_NAME(s)
#define STRUCT_NAME(s)
#define SYM(s,d)

const char *header_top =
#undef HEADER_TOP
#define HEADER_TOP(s) s;
#include "symlist.def"
#undef HEADER_TOP
#define HEADER_TOP(s)

const char *source_top =
#undef SOURCE_TOP
#define SOURCE_TOP(s) s;
#include "symlist.def"
#undef SOURCE_TOP
#define SOURCE_TOP(s)

const char str_prefix[] =
#undef STRING_PREFIX
#define STRING_PREFIX(s) #s;
#include "symlist.def"
#undef STRING_PREFIX
#define STRING_PREFIX(s)

const char struct_name[] =
#undef STRUCT_NAME
#define STRUCT_NAME(s) #s;
#include "symlist.def"
#undef STRUCT_NAME
#define STRUCT_NAME(s)

const char func_name[] =
#undef FUNC_NAME
#define FUNC_NAME(s) #s;
#include "symlist.def"
#undef FUNC_NAME
#define FUNC_NAME(s)

triple all_syms[] = {
#undef SYM
#define SYM(s,d) { #s, sizeof(#s)-1, #d, "vsgN" #s },
#include "symlist.def"
#undef SYM
#define SYM(s,d)
  { 0, 0, 0 }
};

/* Assume that we always use [0, 1, -2, -1] */
int
next_subset (int *subset, int min_len, int max_len)
{
  int n_chars = subset[0];
  int *subs = subset+1;
  int i;

  for (i = n_chars-1; i >= 2*min_len; --i)
    if (subs[i] < max_len - (min_len+1)) {
      ++subs[i];
      return 1;
    }

  if (n_chars < max_len-1) {
    n_chars = ++subset[0];
    for (i = 2*min_len; i < n_chars; ++i)
      subs[i] = i-min_len;
    return 1;
  }
      
  return 0;
}

#define CHAR(str,len,idx) (idx < 0 ? str[len+idx] : idx < len ? str[idx] : 0)

int
unique_subset_p (int n_syms, triple const *syms, int n_chars, int const *chars)
{
  unsigned char *sub = alloca (n_syms * n_chars);
  int i, j;

  for (i = 0; i < n_syms; ++i) {
    int len = syms[i].len;
    unsigned char const *str = syms[i].str;
    unsigned char *p = &sub[i*n_chars];
    for (j = 0; j < n_chars; ++j)
      p[j] = CHAR (str, len, chars[j]);
    for (j = 0; j < i; ++j)
      if (memcmp (p, &sub[j*n_chars], n_chars) == 0)
	return 0;
  }
  return 1;
}

int *
find_unique_subset (int n_syms, triple const *syms, int min_len, int max_len)
{
  int *subset = malloc ((max_len+1) * sizeof (subset[0]));
  int i;

  subset[0] = 2*min_len;
  for (i = 0; i < min_len; ++i)
    subset[i+1] = i;
  for (i = 0; i < min_len; ++i)
    subset[min_len+i+1] = -1-i;

  do {
    if (unique_subset_p (n_syms, syms, subset[0], &subset[1]))
      return subset;
  } while (next_subset (subset, min_len, max_len));

  fprintf (stderr, "Unable to find unique character subset.\n");
  free (subset);
  return 0;
}

int
hash (unsigned char const *str, int n_chars, short *vals)
{
  int i;
  int h = 0;
  for (i = 0; i < n_chars; ++i) {
    int c = str[i];
    if (c) h += vals[c];
  }
  return h;
}

typedef struct {
  int n_chars;
  short *vals;
  short *uses;
  int max_hash;
  int hash_len;
  triple const **hashes;
} value_state;

int
add_hash (value_state *state, triple const *sym, int h, int verbose)
{
  if (h > state->max_hash) {
    state->max_hash = h;
    if (h >= state->hash_len) {
      int old_len = state->hash_len;
      while (state->hash_len <= state->max_hash)
	state->hash_len *= 2;
      state->hashes = realloc (state->hashes,
			       state->hash_len * sizeof (state->hashes[0]));
      memset (&state->hashes[old_len], 0,
	      (state->hash_len - old_len) * sizeof (state->hashes[0]));
    }
    if (verbose) printf ("  hash %d = %s\n", h, sym->str);
    state->hashes[h] = sym;
    return 1;
  } else if (! state->hashes[h]) {
    if (verbose) printf ("  hash %d = %s\n", h, sym->str);
    state->hashes[h] = sym;
    return 1;
  }

  return 0;
}

int
rehash_all (value_state *state, triple const *sym)
{
  value_state old_state = *state;
  int i, h;

  state->hashes = calloc (state->hash_len, sizeof (state->hashes[0]));
  if (sym) {
    h = hash (sym->index, state->n_chars, state->vals);
    add_hash (state, sym, h, trace);
  }
  for (i = 0; i <= old_state.max_hash; ++i) {
    triple const *sym = old_state.hashes[i];
    if (sym) {
      h = hash (sym->index, state->n_chars, state->vals);
      if (! add_hash (state, sym, h, 0)) {
	if (trace)
	  printf ("  rehashing conflict hash %d %s/%s\n",
		  h, sym->str, state->hashes[h]->str);
	free (state->hashes);
	*state = old_state;
	return 0;
      }
    }
  }

  if (0 && trace) {
    printf ("rehash_all:\n");
    for (i = 0; i <= state->max_hash; ++i) {
      triple const *sym = state->hashes[i];
      if (sym)
	printf ("  hash %d = %s\n", i, sym->str);
    }
  }

  free (old_state.hashes);
  return 1;
}

int
modify_single_use (value_state *state, triple const *sym,
		   int from, int to)
{
  int i;
  for (i = 0; i < state->n_chars; ++i) {
    int c = sym->index[i];
    if (c && state->uses[c] == 1) {
      int old_val = state->vals[c];
      for (i = from; i <= to; ++i) {
	int h;
	state->vals[c] = old_val + i;
	h = hash (sym->index, state->n_chars, state->vals);
	if (add_hash (state, sym, h, trace)) {
	  if (trace)
	    printf ("  1: resolved by modifying value['%c'] = %d\n", 
		    c, old_val+i);
	  return 1;
	}
      }
      state->vals[c] = old_val;
    }
  }

  return 0;
}

int
modify_multiple_use (value_state *state, triple const *sym, int n,
		     int from, int to)
{
  int i;
  if (trace) {
    printf (" modify for %d uses (%.*s):",
	    n, state->n_chars, sym->index);
    for (i = 0; i < state->n_chars; ++i) {
      int c = sym->index[i];
      if (c)
	printf (" %c/%d", c, state->uses[c]);
    }
    printf ("\n");
  }
  for (i = 0; i < state->n_chars; ++i) {
    int c = sym->index[i];
    if (c && state->uses[c] == n) {
      int old_val = state->vals[c];
      int j;
      for (j = from; j <= to; ++j) {
	state->vals[c] = old_val + j;
	if (trace) 
	  printf ("  try value['%c'] = %d\n", c, old_val+j);
	if (rehash_all (state, sym)) {
	  if (trace) 
	    printf (" %d: resolved by modifying value['%c'] = %d\n",
		    n, c, old_val+j);
	  return 1;
	}
      }
      state->vals[c] = old_val;
    }
  }

  return 0;
}

int
resolve_conflict (value_state *state, triple const *sym, int h)
{
  triple const *osym = state->hashes[h];
  int i;
  int min_uses1, max_uses1;
  int min_uses2, max_uses2;

  if (trace)
    printf ("Conflict between %s/%s hash %d\n", sym->str, osym->str, h);

  min_uses1 = state->max_hash; max_uses1 = 0;
  for (i = 0; i < state->n_chars; ++i) {
    int c = sym->index[i];
    if (c) {
      int uses = state->uses[c];
      if (uses > max_uses1)
	max_uses1 = uses;
      if (uses < min_uses1)
	min_uses1 = uses;
    }
  }
  if (trace)
    printf ("  %s: uses [%d,%d]\n", sym->str, min_uses1, max_uses1);

  if (min_uses1 == 1
      && modify_single_use (state, sym, 1, 7))
    return 1;

  min_uses2 = state->max_hash; max_uses2 = 0;
  for (i = 0; i < state->n_chars; ++i) {
    int c = osym->index[i];
    if (c) {
      int uses = state->uses[c];
      if (uses > max_uses2)
	max_uses2 = uses;
      if (uses < min_uses2)
	min_uses2 = uses;
    }
  }
  
  if (trace)
    printf ("  %s: uses [%d,%d]\n", osym->str, min_uses2, max_uses2);

  if (min_uses2 == 1
      && modify_single_use (state, osym, 1, 7)) {
    if (trace) printf ("  hash %d = %s\n", h, sym->str);
    state->hashes[h] = sym;
    return 1;
  }

  if (min_uses1 == 1
      && modify_single_use (state, sym, 8, state->max_hash))
    return 1;

  if (min_uses2 == 1
      && modify_single_use (state, osym, 8, state->max_hash)) {
    if (trace) printf ("  hash %d = %s\n", h, sym->str);
    state->hashes[h] = sym;
    return 1;
  }

  for (i = min_uses1; i <= max_uses1; ++i) {
    if (modify_multiple_use (state, sym, i, 1, 16))
      return 1;
  }

  for (i = min_uses2; i <= max_uses2; ++i) {
    if (modify_multiple_use (state, osym, i, 1, 16))
      return 1;
  }

  for (i = min_uses1; i <= max_uses1; ++i) {
    if (modify_multiple_use (state, sym, i, 17, state->hash_len))
      return 1;
  }

  for (i = min_uses2; i <= max_uses2; ++i) {
    if (modify_multiple_use (state, osym, i, 17, state->hash_len))
      return 1;
  }

  return 0;
}

value_state *
add_sym (value_state *state, triple const *sym, int n_chars)
{
  int i, j, h;
  unsigned char *unique;
  int n_unique;

  if (! state) {
    state = malloc (sizeof (*state));
    state->n_chars = n_chars;
    state->vals = calloc (256, sizeof (state->vals[0]));
    state->uses = calloc (256, sizeof (state->uses[0]));
    state->max_hash = 0;
    state->hash_len = 256;
    state->hashes = calloc (state->hash_len, sizeof (state->hashes[0]));
  }

  unique = alloca (n_chars);
  n_unique = 0;
  for (i = 0; i < n_chars; ++i) {
    int c = sym->index[i];
    for (j = 0; j < n_unique; ++j)
      if (unique[j] == c)
	break;
    if (j == n_unique) {
      unique[n_unique++] = c;
      if (c && ! state->vals[c])
	state->vals[c] = 1;
      ++state->uses[c];
      /*printf ("  char['%c'] used %d times\n", c, state->uses[c]);*/
    }
  }

  h = hash (sym->index, state->n_chars, state->vals);
  if (! add_hash (state, sym, h, trace)
      && ! resolve_conflict (state, sym, h)) {
    if (trace)
      printf ("Unable to resolve conflict %s/%s hash %d\n",
	      sym->str, state->hashes[h]->str, h);
    return 0;
  }

  return state;
}

typedef struct {
  int idx;
  int uses;
} cum_uses;

int
compare_uses (void const *v1, void const *v2)
{
  cum_uses const *u1 = v1, *u2 = v2;
  return u2->uses - u1->uses;
}

triple const **
presort (int n_syms, triple const *syms, int n_chars)
{
  triple const **new_list = malloc (n_syms * sizeof (new_list[0]));
  cum_uses *all_uses = alloca (n_syms * sizeof(all_uses[0]));
  int *uses = alloca (256 * sizeof (uses[0]));
  int i;
  memset (uses, 0, 256 * sizeof (uses[0]));
  
  for (i = 0; i < n_syms; ++i) {
    unsigned char *p = syms[i].index, *end = p + n_chars;
    for ( ; p < end; ++p)
      if (*p) ++uses[*p];
  }

  for (i = 0; i < n_syms; ++i) {
    int accum;
    unsigned char *p = syms[i].index, *end = p + n_chars;
    for (accum = 0; p < end; ++p)
      accum += uses[*p];
    all_uses[i].idx = i;
    all_uses[i].uses = accum;
  }

  qsort (all_uses, n_syms, sizeof (all_uses[0]), compare_uses);

  for (i = 0; i < n_syms; ++i) {
    new_list[i] = &syms[all_uses[i].idx];
    if (trace)
      printf (" %3d [%.*s]: %s\n", all_uses[i].uses,
	      n_chars, new_list[i]->index, new_list[i]->str);
  }

  return new_list;
}

value_state *
find_values (int n_syms, triple const *syms, int n_chars)
{
  int i;
  short *vals = alloca (256 * sizeof (vals[0]));
  value_state *state = 0;
  triple const **sorted_syms = presort (n_syms, syms, n_chars);
  for (i = 0; i < n_syms; ++i) {
    state = add_sym (state, sorted_syms[i], n_chars);
    if (! state) {
      free (sorted_syms);
      return 0;
    }
  }

  free (sorted_syms);
  return state;
}

void
compress_values (value_state *state)
{
  
}

int
int_decrease (const void *v1, const void *v2)
{
  int i1 = *(int const *)v1;
  int i2 = *(int const *)v2;
  return i2 - i1;
}

void
output_state (value_state const *state, char const *fname,
	      int *chars, int min_len, int max_len)
{
  int i;
  char *name = malloc (strlen (fname) + 3);
  FILE *header = 0, *source = 0;
  int min_hash;
  char const *short_name;

  strcpy (name, fname); strcat (name, ".h");
  if (! (header = fopen (name, "w"))) {
    fprintf (stderr, "Unable to create %s\n", name);
    exit (1);
  }

  strcpy (name, fname); strcat (name, ".c");
  if (! (source = fopen (name, "w"))) {
    fprintf (stderr, "Unable to create %s\n", name);
    exit (1);
  }
  free (name);

  if (!! (short_name = strrchr (fname, '/')))
    ++short_name;
  else
    short_name = fname;

  fprintf (header, "#ifndef _gs_%s_h_\n#define _gs_%s_h_\n\n", short_name, short_name);
  if (header_top)
    fprintf (header, "%s\n\n", header_top);
  fprintf (header, "struct %s { char const *name; int id; };\n\n",
	   struct_name);

  fprintf (source, "#include \"%s.h\"\n\n", short_name);
  if (source_top)
    fprintf (source, "%s\n\n", source_top);

  min_hash = state->max_hash;
  for (i = 0; i <= state->max_hash; ++i) {
    triple const *sym = state->hashes[i];
    if (sym) {
      if (i < min_hash) min_hash = i;
      fprintf (header, "extern char %s%s[];\n", str_prefix, sym->str);
      fprintf (source, "char %s%s[] = \"%s\";\n", 
	       str_prefix, sym->str, sym->str);
    }
  }
  
  {
    int newline = 1;
    fprintf (source, "\nstatic char const aNIL[2];\n");
    fprintf (source, "\nstatic struct %s assoc[] = {", struct_name);
    for (i = 0; i <= state->max_hash; ++i) {
      triple const *sym = state->hashes[i];
      if (sym) {
	fprintf (source, "\n {%s%s, %s} /*%d*/,",
		 str_prefix, sym->str, sym->def, i);
	newline = 1;
      } else {
	if (newline>0 || (-newline)%6==0) fprintf (source, "\n");
	fprintf (source, " {aNIL,0},");
	newline = (newline>0 ? -1 : newline-1);
      }
    }
    if (newline>0 || (-newline)%6==0) fprintf (source, "\n");
    fprintf (source, " {aNIL,0}\n};\n\n");
  }

  fprintf (header, "struct %s * %s (char const *str, int len);\n\n",
	   struct_name, func_name);
  fprintf (header, "#endif /* _gs_%s_h_ */\n", short_name);
  fclose (header);

  /* Constants. */
  fprintf (source, "#define MIN_LEN %d\n", min_len);
  fprintf (source, "#define MAX_LEN %d\n", max_len);
  fprintf (source, "#define MIN_HASH %d\n", min_hash);
  fprintf (source, "#define MAX_HASH %d\n", state->max_hash);

  /* Hash function. */
  fprintf (source, "static unsigned int\nhash (unsigned char const *str, int len) {\n");
  fprintf (source, "  unsigned short values[] = {");
  for (i = 0; i < 256; ++i) {
    if (i%12 == 0) fprintf (source, "\n   ");
    fprintf (source, " %d,",
	     state->vals[i] ? state->vals[i] : state->max_hash+1);
  }
  fprintf (source, "  %d };\n\n", state->max_hash+1);
  fprintf (source, "  unsigned int hval = 0;\n");
  fprintf (source, "  switch (len) {\n  default:\n");
  {
    int *chars_end = chars + state->n_chars;
    qsort (chars, state->n_chars, sizeof (chars[0]), int_decrease);
    i = chars[0];
    for (; i >= 0; --i) {
      fprintf (source, "  case %d:\n", i+1);
      if (i == chars[0]) {
	fprintf (source, "    hval += values[str[%d]];\n", i);
	++chars;
      }
    }
    fprintf (source, "  }\n");
    for ( ; chars < chars_end; ++chars)
      fprintf (source, "  hval += values[str[len%d]];\n", *chars);
    fprintf (source, "  return hval;\n");
  }
  
  fprintf (source, "}\n\n");

  /* Lookup. */
  fprintf (source, "struct %s *\n", struct_name);
  fprintf (source, "%s (char const *str, int len) {\n", func_name);
  fprintf (source, "  if (len >= MIN_LEN && len <= MAX_LEN) {\n");
  fprintf (source, "    unsigned int hval = hash ((const unsigned char *) str, len);\n");
  fprintf (source, "    if (hval <= MAX_HASH) {\n");
  fprintf (source, "      struct %s *a = &assoc[hval];\n", struct_name);
  fprintf (source, "      if (! strcmp (a->name, str))\n");
  fprintf (source, "        return a;\n");
  fprintf (source, "    }\n");
  fprintf (source, "  }\n");
  fprintf (source, "  return 0;\n");
  fprintf (source, "}\n");

  fclose (source);
}

int
main (int argc , char *argv[])
{
  int n_syms = sizeof (all_syms)/sizeof(all_syms[0]) - 1;
  int i;
  int min_len, max_len;
  int *subset;
  char *fname = "genout";

  int c;
  extern char *optarg;
  char *cmd = argv[0];
  int usage = 0;

  while ((c = getopt (argc, argv, "o:t")) != -1) {
    switch (c) {
    case 't':
      ++trace;
      break;
    case 'o':
      fname = optarg;
      break;
    case ':':
    case '?':
      ++usage;
      break;
    }
  }
  if (usage) {
    fprintf (stderr, "Usage: %s [-t] [-o file]\n", cmd);
    exit (1);
  }

  min_len = max_len = all_syms[0].len;
  for (i = 1; i < n_syms; ++i) {
    int len = all_syms[i].len;
    if (len < min_len) min_len = len;
    else if (len > max_len) max_len = len;
  }
  if (trace)
    printf ("strlen: min %d, max %d\n", min_len, max_len);

  subset = find_unique_subset (n_syms, all_syms, min_len, max_len);
  if (subset) {
    int n_chars = subset[0];
    unsigned char *p;
    value_state *state;

    ++subset;
    if (trace) {
      printf ("Subset:");
      for (i = 0; i < n_chars; ++i) {
	printf (" %d", subset[i]);
      }
      printf ("\n");
    }

    p = malloc (n_syms * n_chars * sizeof (p[0]));
    for (i = 0; i < n_syms; ++i, p += n_chars) {
      unsigned char const *str = all_syms[i].str;
      int len = all_syms[i].len;
      int j;
      all_syms[i].index = p;
      for (j = 0; j < n_chars; ++j)
	p[j] = CHAR (str, len, subset[j]);
    }

    state = find_values (n_syms, all_syms, n_chars);
    if (state) {
      compress_values (state);
      printf ("Max hash = %d\n", state->max_hash);
      output_state (state, fname, subset, min_len, max_len);
      return 0;
    }
  }

  return 1;
}
