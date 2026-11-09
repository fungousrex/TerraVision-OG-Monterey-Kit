#include "TerraVisionInc.h"

#include <assert.h>

#include <pthr/list.h>

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xresource.h>

Widget applicationShell = 0;

XrmDatabase tvDatabase;
XrmDatabase userDatabase;
XrmName tvNameQ;
XrmClass tvClassQ;

static XrmRepresentation StringQ = 0;
static XrmRepresentation IntQ = 0;
static XrmRepresentation FloatQ = 0;

/*
 * Synopsis: Boolean tvGetResource (char const *name,
 *                                  XtPointer *val,
 *                                  XrmRepresentation as_type)
 *
 * Description:  Reads resource from tvDatabase and returns it in `*val' as
 *               type `as_type'
 *
 * Returns: True if found and convertable.
 *
 * Author: Lee Iverson
 *
 * Date: November 1997.
 */

Boolean
tvGetResource (char const *name, XtPointer *val, XrmRepresentation as_type)
{
  static const char *funcName = "tvGetResource()";
  XrmRepresentation type;
  XrmValue value;
  XrmName fq_name[3];
  XrmClass fq_cls[3];

  fq_name[0] = tvNameQ; fq_name[1] = XrmStringToName (name); fq_name[2] = 0;
  fq_cls[0] = tvClassQ; fq_cls[1] = XrmStringToClass (name); fq_cls[2] = 0;

  if (! StringQ) StringQ = XrmStringToRepresentation (XtRString);

  if (XrmQGetResource (tvDatabase, fq_name, fq_cls, &type, &value)) {
    if (type == StringQ)
      TSMTRACE ((INIT_MASK|2, funcName, "%s: (%s) \"%s\"", 
		 name, XrmQuarkToString (type), value.addr));
    else
      TSMTRACE ((INIT_MASK|2, funcName, "%s: (%s) \"%s\"", 
		 name, XrmQuarkToString (type), value.addr));
      
    if (type == as_type) {
      *val = value.addr;
      return True;
    } else {
      XrmValue cvt;
      XtConvert (applicationShell,
		 XrmRepresentationToString (type), &value,
		 XrmRepresentationToString (as_type), &cvt);
      if (cvt.addr)
	*val = cvt.addr;
      return !! cvt.addr;
    }
  }

  TSMTRACE ((INIT_MASK|1, funcName, "Unable to get the resource: %s", name));
  return False;
}

/*
 * Synopsis: void tvExpandStr (char const **val)
 *
 * Description:  Expands any special characters or environment variables in
 *               *val.  If needed allocates a new string and assigns it to
 *               *val.
 *
 * Returns: Nothing.
 *
 * Author: Lee Iverson
 *
 * Date: November 1997.
 */

static void
tvExpandStr (char const **val)
{
  static char const funcName[] = "tvExpandStr()";
  char const *in, *p;
  char *out, *op;
  int changed, len;
  struct env {
    struct env *next;
    char const *p, *q, *name, *val;
  } *envs = 0, *e;

  if (! val || ! (in = *val)) return;

  /* Walk through string and compute length of result. */
  TSMTRACE ((INIT_MASK|2, funcName, "'%s'", in));
  changed = False;
  for (e = envs, len = 0, p = in; *p; ++p, ++len) {
    if (*p == '\\') {
      if (p[1]) ++p;
      changed = True;
    } else if (*p == '$') {
      struct env *new_e = alloca (sizeof (*new_e));
      char const *q = ++p;
      char *name;
      while (*q && (isalnum (*q) || *q == '_')) ++q;
      new_e->p = p; new_e->q = q;
      name = alloca (q-p);
      strncpy (name, p, q-p);
      name[q-p] = '\0';
      new_e->name = name;
      if (! (new_e->val = getenv (name))) {
	TSMTRACE ((INIT_MASK|2, funcName, "No environment variable '%s'",
		   name));
	return;
      }
      TSMTRACE ((INIT_MASK|2, funcName, "Env: %p(%c) %p(%c) %s -> %s",
		 p, p[0], q, q[0], new_e->name, new_e->val));
      p = q-1;
      len += strlen (new_e->val) - 1;
      if (e) e->next = new_e; else envs = new_e;
      e = new_e;
    }
  }

  /* If we don't need to create a new string, exit. */
  if (! changed && ! envs)
    return;

  /* Allocate new string and fill it up. */
  out = malloc (len+1);
  TSMTRACE ((INIT_MASK|2, funcName, "Allocate %d", len+1));
  for (e = envs, p = in, op = out; *p; ) {
    if (*p == '\\') {
      char sel = *++p;
      switch (sel) {
      case 'n': *op = '\n'; break;
      case 't': *op = '\t'; break;
      case 'r': *op = '\r'; break;
      case 'b': *op = '\b'; break;
      case 'f': *op = '\f'; break;
      default:
	if (sel >= '0' && sel <= '7') {
	  /* Read octal character. */
	  char v = sel - '0';
	  if ((sel = p[1]) >= '0' && sel <= '7') {
	    v = (v*8) + (sel-'0');
	    ++p;
	  }
	  if ((sel = p[1]) >= '0' && sel <= '7') {
	    v = (v*8) + (sel-'0');
	    ++p;
	  }
	} else {
	  *op = sel;
	}
	break;
      }
      /* Back up in case we've run off end. */
      if (sel) ++p;
      ++op;
    } else if (*p == '$') {
      assert (e->p == p+1);
      strcpy (op, e->val);
      op += strlen (e->val);
      p = e->q;
      ++e;
    } else {
      *op = *p;
      ++p; ++op;
    }
  }
  assert (op < &out[len+1]);
  *op = '\0';
  TSMTRACE ((INIT_MASK|2, funcName, "Out[%d]: '%s'", len, out));
  
  *val = out;
}

/*
 * Synopsis: Boolean tvGetResourceAsString (char const *name,
 *                                          char const **val)
 *
 * Description:  Reads resource from tvDatabase as a string and returns it in
 *               `*val'.
 *
 * Returns: True if found and convertable.
 *
 * Author: Lee Iverson
 *
 * Date: November 1997.
 */

Boolean
tvGetResourceAsString (char const *name, char const **val)
{
  static const char *funcName = "tvGetResourceAsString()";
  if (! StringQ) StringQ = XrmStringToRepresentation (XtRString);
  if (! tvGetResource (name, (XtPointer *) val, StringQ)) return False;
  tvExpandStr (val);
  TSMTRACE ((INIT_MASK|1, funcName, "%s: \"%s\"", name, *val));
  return True;
}

/*
 * Synopsis: Boolean tvGetResourceAsInt (char const *name, int *val)
 *
 * Description:  Reads resource from tvDatabase as an integer and returns it
 *               in `*val'.
 *
 * Returns: True if found and convertable.
 *
 * Author: Lee Iverson
 *
 * Date: November 1997.
 */

Boolean
tvGetResourceAsInt (char const *name, int *val)
{
  static const char *funcName = "tvGetResourceAsInt()";
  int *v;
  if (! IntQ) IntQ = XrmStringToRepresentation (XtRInt);
  if (! tvGetResource (name, (XtPointer *) &v, IntQ)) return False;
  *val = *v;
  TSMTRACE ((INIT_MASK|1, funcName, "%s: %d", name, *val));
  return True;
}

/*
 * Synopsis: Boolean tvGetResourceAsFloat (char const *name, float *val)
 *
 * Description:  Reads resource from tvDatabase as a float and returns it
 *               in `*val'.
 *
 * Returns: True if found and convertable.
 *
 * Author: Lee Iverson
 *
 * Date: November 1997.
 */

Boolean
tvGetResourceAsFloat (char const *name, float *val)
{
  static const char *funcName = "tvGetResourceAsFloat()";
  float *v;
  if (! FloatQ) FloatQ = XrmStringToRepresentation (XtRFloat);
  if (! tvGetResource (name, (XtPointer *) &v, FloatQ)) return False;
  *val = *v;
  TSMTRACE ((INIT_MASK|1, funcName, "%s: %g", name, *val));
  return True;
}

/*
 * Synopsis: Boolean tvGetResourceAsDouble (char const *name, double *val)
 *
 * Description:  Reads resource from tvDatabase as a double and returns it
 *               in `*val'.
 *
 * Returns: True if found and convertable.
 *
 * Author: Lee Iverson
 *
 * Date: November 1997.
 */

Boolean
tvGetResourceAsDouble (char const *name, double *val)
{
  static const char *funcName = "tvGetResourceAsDouble()";
  char const *str;
  if (! StringQ) StringQ = XrmStringToRepresentation (XtRString);
  if (tvGetResource (name, (XtPointer *) &str, StringQ)) {
    *val = atof (str);
    TSMTRACE ((INIT_MASK|1, funcName, "%s: %g", name, *val));
    return True;
  }
  return False;
}

/*
 * Synopsis: list_t * ParseStringList (const char *str)
 *
 * Description: Converts comma-separated string `str' to list_t.
 *
 * Returns: List extracted or NULL if none.
 *
 * Author: Lee Iverson
 *
 * Date: November 1997.
 */

list_t *
ParseStringList (const char *str)
{
  static char const funcName[] = "ParseStringList()";
  list_t *lst = list_create (FALSE);
  TSMTRACE ((INIT_MASK|1, funcName, "%s", str));
  while (*str) {
    const char *p;
    while (isspace (*str)) ++str;
    if (! *str) break;

    /* Make p point to the end of this string element. */
    if (! (p = strchr (str, ',')))
      p = strchr (str, '\0');

    /* Now initialize a new list entry and attach it to the end of list. */
    {
      char *s = malloc (p-str+1);
      strncpy (s, str, p-str);
      s[p-str] = '\0';
      TSMTRACE ((INIT_MASK|1, funcName, "-- %s", s));
      list_push (lst, s);
    }
    str = *p ? p+1 : p;
  }

  return lst;
}

/*
 * Synopsis: void PrintStringList (FILE *fp, list_t * lst)
 *
 * Description: Prints list `lst' as comma-separated list on stream `fp'
 *
 * Returns: Nothing.
 *
 * Author: Lee Iverson
 *
 * Date: November 1997.
 */

void
PrintStringList (FILE *fp, list_t *lst)
{
  list_iter_t i;
  int count = 0;

  for (i = list_first (lst); ! list_done (i); i = list_next (i)) {
    char *str = list_value (i);
    if (count++) fputs (", ", fp);
    fputs (str, fp);
  }
}

/*
 * Synopsis: Boolean tvGetResourceAsStringList (char const *name,
 *                                              list_t **val)
 *
 * Description:  Reads resource from tvDatabase as a comma-separated string
 *               list, parses and returns it in `*val'.
 *
 * Returns: True if found and convertable.
 *
 * Author: Lee Iverson
 *
 * Date: November 1997.
 */

Boolean
tvGetResourceAsStringList (char const *name, list_t **val)
{
  /*static const char funcName[] = "tvGetResourceAsStringList()";*/
  const char *str, *s;
  if (! StringQ) StringQ = XrmStringToRepresentation (XtRString);
  if (! tvGetResource (name, (XtPointer *) &str, StringQ)) return False;
  s = str;
  tvExpandStr (&str);
  *val = ParseStringList (str);
  if (s != str) free ((void *) str);
  return True;
}

/*
 * Synopsis: Bool tvPrintResource (XrmDatabase *db, 
 *                                 XrmBindingList bindings,
 *                                 XrmQuarkList quarks,
 *                                 XrmRepresentation *type, XrmValue *value,
 *                                 XPointer closure)
 *
 * Description:  Callback for XrmEnumerateDatabase.  Prints elements.
 *
 * Returns: False.
 *
 * Author: Lee Iverson
 *
 * Date: November 1997.
 */

static Bool
tvPrintResource (XrmDatabase *db, 
		 XrmBindingList bindings, XrmQuarkList quarks,
		 XrmRepresentation *type, XrmValue *value,
		 XPointer closure)
{
  if (! StringQ) StringQ = XrmStringToRepresentation (XtRString);

  fprintf (stderr, "Resource: ");
  for ( ; *quarks; ++quarks, ++bindings)
    fprintf (stderr, "%s%s", *bindings==XrmBindTightly ? "." : "*",
	     XrmQuarkToString (*quarks));
  if (*type == StringQ) {
    fprintf (stderr, ": (String) \"%s\"\n", value->addr);
  } else {
    fprintf (stderr, ": (%s len %d) %p\n",
	     XrmRepresentationToString (*type), value->size, value->addr);
  }

  return False;
}

void
tvShowDatabase (char const *appName, char const *appClass)
{
  XrmName name[2];
  XrmClass class[2];
  XrmStringToNameList (appName, name);
  XrmStringToClassList (appClass, class);
  tvNameQ = name[0];
  tvClassQ = class[0];

  XrmEnumerateDatabase (tvDatabase, name, class, XrmEnumOneLevel,
			tvPrintResource, NULL);
}

/*
 * Synopsis: void tvInitDatabase (char const *appName, char const *appClass,
 *                                int *argcp, char * argv[])
 *
 * Description:  Initializes tvDatabase based on screen resources,
 *               command-line arguments and userinfo file.
 *
 * Returns: Nothing.
 *
 * Author: Lee Iverson
 *
 * Date: November 1997.
 */

void
tvInitDatabase (Display *tvDisplay,
		char const *appName, char const *appClass,
		int *argcp, char * argv[])
{
  XrmName name[2];
  XrmClass class[2];
  XrmStringToNameList (appName, name);
  XrmStringToClassList (appClass, class);
  tvNameQ = name[0];
  tvClassQ = class[0];

  XrmInitialize ();
  tvDatabase = XrmGetDatabase (tvDisplay);

  XrmParseCommand (&tvDatabase, NULL, 0, appName, argcp, argv);
#if 0
  tvShowDatabase (appName, appClass);
#endif
}
