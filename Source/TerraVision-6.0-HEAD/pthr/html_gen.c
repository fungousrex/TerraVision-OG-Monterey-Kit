/* 

BASIC DESCRIPTION:

   html_gen.c is a collection of functions for creating html output.

   For example, a title in html is defined as <TITLE>My Title</TITLE>.
   The easiest way to produce this is the html_title function:

   html_title ("This is the title");

   But there are many situations where the enclosing string (the title
   in this case) is computed on the fly.  So, as with most functions
   below, there is a "_start" and "_end" variant:

   html_title_start ();
   ... print the title yourself ...
   html_title_end ();

   There are 3 types of functions in this package:

   o Functions to print html tags, like html_title.  They all begin
     with html_tagname.  Many of these functions take an optional
     string.  You can either use NULL or the empty string for this.

   o Meta-functions to print multiple tags.

   o Utitility functions.  They all begin with html__ as well.

     These are used by the tag and meta-functions.

   o Basic printing functions.

     All the functions above use the basic printing functions:
   
     html__as:  print a string with a leading blank if its not empty
     html__d:   print an integer
     html__s:   print a single string if its not empty
     html__s1s: print a string with one argument
     html__s2s: print a string with two arguments

     Their main advantage is that you can define their output stream
     once at the beginning of your program (the default is stdout),
     and you're guaranteed that all your html output will go to that
     stream.

     For example:

     html_title_start ();
     html__s1s ("This is the %s title", title_modifier);
     html_title_end ();

   The functions within each group are listed in alphabetical order
   below.

NOTES ON PACKAGE DESIGN:

   The basic philosophy of the package is to make html generation as
   easy as possible while maintaining as much flexibility as possible,
   both with regards to the user's needs and to the fluid state of
   html.  These two ideals are somewhat opposed, so some rather
   arbitrary decisions had to be made.  For example, the <TITLE> tag
   can actually take 3 optional attributes: ID, LANG, INDEX (at least,
   today, who knows how many tomorrow). Rather than requiring these
   three optional arguments for every title, there's a variant
   function called html_title__1as (title, attribute_string).  This
   maintains the simplicity of the html_title function, but allows
   enough flexibility for future changes (I hope!).

   If they're defined for a given tagname, the variants will be called:

   html_tagname__1as -- optional attributes string.  If the string is
     not NULL or empty, it will be printed withing the tag with a
     leading blank.  For example: 
     html_title_1as ("My Title", "ID=\"myID\"") --> 
     <TITLE ID="myID">My Title</TITLE>\n

   html_tagname__1ks -- optional keyword-string pair.  For example:
     html_title_1ks ("My Title:, "ID", "myID") -->
     <TITLE ID="myID">My Title</TITLE>\n

   html_tagname__2ks, html_tagname_3ks, etc. -- 2, 3, etc optional
     keyword-string pairs.

   html_tagname__1kd -- optional keyword-integer pair.  The keywork-integer
     pair will only be printed if the integer is >= 0.

   The definitions of html used here derive primarily from:
   http://www.w3.org/hypertext/WWW/MarkUp/HTMLPlus/htmlplus_1.html

   This is a slightly out of date reference, but nicely written and
   understandable.  Appendix IV contains a complete list of tags
   and their attributes, included at the end of this file.

*/

#include "html_gen.h"

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef HAVE_STATIC_INIT
static FILE *html__stream = stdout;
#else
static FILE *html__stream = NULL;
#endif

/* ================================================================
   ================================================================

   TAG FUNCTIONS:

   These are the user-level functions for creating tags and their
   arguments.

   ================================================================
   ================================================================ */


/* ================================================================
 * A_HREF --
 *   Create a link to a url.
 *
 * <A HREF="url">link</A>
 * ================================================================ */

void html_a_href_start (char *url)
{
  html__tag_1ks ("", "A", "", "HREF", url);
}

void html_a_href_end (char *link)
{
  html__tag (link, "/A", "");
}

void html_a_href (char *url, char *link)
{
  html_a_href_start (url);
  html_a_href_end (link);
}

/* ================================================================
 * A_NAME --
 *   Create a named anchor.
 *
 * <A NAME="name"></A>
 * ================================================================ */

void html_a_name (char *name)
{
  html__tag_1ks ("", "A", "", "NAME", name);
}

/* ================================================================
 * ADDRESS --
 *   Print a string using the address font.
 *
 * <ADDRESS>string</ADDRESS>
 * ================================================================ */

void html_address_start (void)
{
  html__tag ("", "ADDRESS", "");
}

void html_address_end (void)
{
  html__tag ("", "/ADDRESS", "");
}

void html_address (char *string)
{
  html_address_start ();
  html__s (string);
  html_address_end ();
}

/* ================================================================
 * B --
 *   Print a string with the bold font.
 * 
 * <B>string</B>
 * ================================================================ */

void html_b_start (void)
{
  html__tag ("", "B", "");
}

void html_b_end (void)
{
  html__tag ("", "/B", "");
}

void html_b (char *string)
{
  html_b_start ();
  html__s (string);
  html_b_end ();
}

/* ================================================================
 * BODY --
 *   BODY enclosing tag.
 *
 * <BODY>stuff</BODY>\n
 * ================================================================ */

void html_body_start (void)
{
  html__tag ("", "BODY BGCOLOR=\"#FFFFFF\" TEXT=\"#000000\"", "\n");
}

void html_body_end (void)
{
  html__tag ("\n", "/BODY", "\n");
}

/* ================================================================
 * BR --
 *   Line break.
 *
 * <BR>\n
 * ================================================================ */

void html_br (void)
{
  html__tag ("", "BR", "\n");
}

/* ================================================================
 * CENTER --
 *   Center a string.
 *
 * <CENTER> string </CENTER>\n
 * ================================================================ */

void html_center_start (void)
{
  html__tag ("", "CENTER", "");
}

void html_center_end (void)
{
  html__tag ("", "/CENTER", "\n");
}

void html_center (char *string)
{
  html_center_start ();
  html__s (string);
  html_center_end ();
}

/* ================================================================
 * dl --
 *    Create a definition list.
 *      - attributes: either COMPACT or nothing
 *      - term: the term for each list item
 *      - definition: the definition for each list item
 *
 * <DL COMPACT|nothing>\nl
 * <DT>term\nl
 * <DD>definition\nl
 * ...
 * </DL>\nl
 * ================================================================ */

void html_dl_start (char *attributes_string)
{
  html__tag_1as ("", "DL", "\n", attributes_string);
}

void html_dt (char *term)
{
  html__tag ("", "DT", term);
  html__nl ();
}

void html_dd (char *definition)
{
  html__tag ("", "DD", definition);
  html__nl ();
}

void html_dl_item (char *term, char *definition)
{
  html_dt (term);
  html_dd (definition);
}

void html_dl_end (void)
{
  html__tag ("", "/DL", "\n");
}

/* ================================================================
 * form --
 *   Create a form, sending the CGI data to URL action by method.
 *
 * <FORM METHOD="method" ACTION="action">\n
 * ================================================================ */

void html_form_start (char *method, char *action)
{
  html__tag_2ks ("", "FORM", "\n", "METHOD", method, "ACTION", action);
}

void html_form_end (void)
{
  html__tag ("", "/FORM", "\n");
}

/* ================================================================
 * H --
 *   Header.
 *
 * <Hsize>string</Hsize>\n
 * ================================================================ */

void html_h_start (int size)
{
  html__tag_1d ("", "H", "", size);
}

void html_h_end (int size)
{
  html__tag_1d ("", "/H", "\n", size);
}

void html_h (int size, char *string)
{
  html_h_start (size);
  html__s (string);
  html_h_end (size);
}

/* ================================================================
 * HEAD --
 *   HEAD enclosing tag.
 *
 * <HEAD>stuff</HEAD>\n
 * ================================================================ */

void html_head_start (void)
{
  html__tag ("", "HEAD", "\n");
}

void html_head_end (void)
{
  html__tag ("\n", "/HEAD", "\n");
}

/* ================================================================
 * HTML_HEADER --
 *   HTML, TITLE, and BODY enclosing tags.
 *
 * <HTML> <HEAD>
 * <TITLE>stuff</TITLE>
 * </HEAD>\n
 *
 * <BODY>
 * ...
 * </BODY> </HTML>
 * ================================================================ */

void html__html_header_start (char *title)
{
  html_html_start ();
  html_head_start ();
  html_title (title);
  html_head_end ();
  html_body_start ();
}

void html__html_header_end (void)
{
  html_body_end ();
  html_html_end ();
}

/* ================================================================
 * HTML_HEADERN --
 *   HTML, TITLE, BODY, and H enclosing tags.
 *
 * <HTML> <HEAD>
 * <TITLE>title</TITLE>
 * </HEAD>\n
 * <Hn>header_string></Hn>
 *
 * <BODY>
 * ...
 * </BODY> </HTML>
 * ================================================================ */

void html__html_headern_start (char *title, int n, char *header)
{
  html__html_header_start (title);
  html_h (n, header);
}

void html__html_headern_end (void)
{
  html__html_header_end ();
}

/* ================================================================
 * HTML --
 *   HTML enclosing tag.
 *
 * <HTML>stuff</HTML>\n
 * ================================================================ */

void html_html_start (void)
{
#ifndef HAVE_STATIC_INIT
  if (! html__stream)
    html__stream = stdout;
#endif
  html__tag ("", "HTML", "\n");
}

void html_html_end (void)
{
  html__tag ("\n", "/HTML", "\n");
}

/* ================================================================
 * HR --
 *   Horizontal rule.
 *
 * <HR>
 * ================================================================ */

void html_hr (void)
{
  html__tag ("\n", "HR NOSHADE", "\n");
}

/* ================================================================
 * IMG --
 *   Insert an image specified by URL src.
 *     alt is an optional string to print if the viewer cannot do images
 *
 * <IMG SRC="src" ALT="alt">
 * ================================================================ */

void html_img (char *src, char *alt)
{
  html__tag_2ks ("", "IMG", "", "SRC", src, "ALT", alt);
}

/* ================================================================
 * input_blank -- 
 *   Create a blank text entry field.
 *     - name is an optional CGI variable name
 *     - size is the size of the field; use -1 for the default size
 *     - max_length is the maximum length of the input string
 *     - value is the optional string entered into the field,
 *       which is the default value of the CGI variable
 *
 * <INPUT NAME="name" SIZE=size MAXLENGTH=maxlength VALUE="value">\n
 * ================================================================ */

void html_input_blank (char *name, int size, int maxlength, char *value)
{
  html__s ("<INPUT");
  html__ks ("NAME", name);
  html__kd ("SIZE", size);
  html__kd ("MAXLENGTH", maxlength);
  html__ks ("VALUE", value);
  html__s (">");
}

/* ================================================================
 * input_hidden -- 
 *   Create a hidden CGI variable
 *     - name is an optional CGI variable name
 *     - value is the value of the CGI variable
 *
 * <INPUT TYPE="hidden" NAME="name" VALUE="value">
 * ================================================================ */

void html_input_hidden (char *name, char *value)
{
  html__s ("<INPUT");
  html__ks ("TYPE", "hidden");
  html__ks ("NAME", name);
  html__ks ("VALUE", value);
  html__s (">\n");
}

/* ================================================================
 * input_radio --
 *   Create a radio button.
 *     - name: the name of the CGI variable
 *     - value: the value of the CGI variable if checked
 *     - attribute: one of CHECKED or nothing if null.
 *
 * <INPUT TYPE=RADIO NAME="name" VALUE="value" CHECKED|nothing>
 * ================================================================ */

void html_input_radio (char *name, char *value, char *attributes_string)
{
  html__s ("<INPUT TYPE=RADIO");
  html__ks ("NAME", name);
  html__ks ("VALUE", value);
  html__as (attributes_string);
  html__s (">"); 
}

void html_input_radio__2s (char *name, char *value, char *attributes_string, char *string)
{
  html__s ("<INPUT TYPE=RADIO");
  html__ks ("NAME", name);
  html__ks ("VALUE", value);
  html__as (attributes_string);
  html__s (">"); 
  html__snl (string);
}

void html_input_radio__2d (char *name, int int_value, char *attributes_string, int i)
{
  html__s ("<INPUT TYPE=RADIO");
  html__ks ("NAME", name);
  html__kd ("VALUE", int_value);
  html__as (attributes_string);
  html__s (">"); 
  html__d (i);
  html__nl ();
}

/* ================================================================
 * input_checkbox --
 *   Create a checkbox.
 *     - name: the name of the CGI variable
 *     - value: the value of the CGI variable if checked
 *     - attribute: one of CHECKED or nothing if null.
 *
 * <INPUT TYPE=CHECKBOX NAME="name" VALUE="value" CHECKED|nothing>
 * ================================================================ */

void html_input_checkbox (char *name, char *value, char *attributes_string)
{
  html__s ("<INPUT TYPE=CHECKBOX");
  html__ks ("NAME", name);
  html__ks ("VALUE", value);
  html__as (attributes_string);
  html__s (">"); 
}


/* ================================================================
 * input_reset -- 
 *   Create a "RESET" button.
 *     label is an optional string written on the button
 *     src is a URL of an image for the button
 *
 * <INPUT TYPE=RESET VALUE="label" SRC="src">
 * ================================================================ */

void html_input_reset (char *label, char *src)
{
  html__tag_2ks ("", "INPUT TYPE=RESET", "", "VALUE", label, "SRC", src);
}

/* ================================================================
 * input_submit --
 *    Create a "SUBMIT" button.
 *      label is an optional string written on the button
 *      src is a URL of an image for the button
 *
 * <INPUT TYPE=SUBMIT VALUE="label" SRC="src">
 * ================================================================ */

void html_input_submit (char *label, char *src)
{
  html__tag_2ks ("", "INPUT TYPE=SUBMIT", "", "VALUE", label, "SRC", src);
}

/* ================================================================
 * li --
 *    List item.
 *      - src (optional): the URL of an icon
 *
 * <LI SRC="src">
 * ================================================================ */

void html_li (char *src)
{
  html__tag_1ks ("", "LI", "", "SRC", src);
}

/* ================================================================
 * ol --
 *    Create an ordered list.
 *      - attributes: either COMPACT or nothing
 *      - item: the list item
 *
 * <OL COMPACT|nothing>
 * <LI>item
 * ...
 * </OL>
 * ================================================================ */

void html_ol_start (char *attributes_string)
{
  html__tag_1as ("", "OL", "\n", attributes_string);
}

void html_ol_item (char *item)
{
  html__tag ("", "LI", item);
  html__nl ();
}

void html_ol_end ()
{
  html__tag ("", "/OL", "\n");
}

/* ================================================================
 * OPTION --
 *   Create a selection option.
 *     - value: optional.  If this item is selected, the CGI SELECT
 *       variable will have value value (if its not NULL), or value name.
 *     - attributes: either SELECTED or DISABLED or nothing if its NULL
 *     - name: the name of the option as seen in the viewer.
 *     
 * <OPTION VALUE="value" SELECTED|DISABLED|LANG|nothing>name
 * ================================================================ */

void html_option (char *value, char *attributes_string, char *name)
{
  html__s ("<OPTION");
  html__ks (" VALUE", value);
  html__as (attributes_string);
  if (name && (strlen (name) > 0))
    html__s1s (">%s\n", name);
  else
    html__s (">\n");
}

/* ================================================================
 * P --
 *   Paragraph break.
 *
 * <P>
 * ================================================================ */

void html_p (void)
{
  html__tag ("", "P", "\n");
}

/* ================================================================
 * PRE --
 *   Print pre-formated output.
 *
 * \n<PRE>
 * ...
 * </PRE>\n
 * ================================================================ */

void html_pre_start (void)
{
  html__tag ("\n", "PRE", "");
}

void html_pre_end (void)
{
  html__tag ("", "/PRE", "\n");
}

void html_pre (char *string)
{
  html_pre_start ();
  html__s (string);
  html_pre_end ();
}

/* ================================================================
 * SELECT --
 *   Create a list of select options.
 *     - name: the name of the CGI variable whose value will be the
 *       selected the option(s).
 *     - size: the length of the list on the screen; use -1 for the default
 *     - attributes: one of SEVERAL, ERROR, or nothing if its NULL
 *
 * <SELECT NAME="name" SIZE=size SEVERAL|ERROR|nothing>\n
 * ================================================================ */

void html_select_start (char *name, int size, char *attributes_string)
{
  html__s ("<SELECT ");
  html__ks ("NAME", name);
  html__kd ("SIZE", size);
  html__as (attributes_string);
  html__s (">\n");
}

void html_select_end (void)
{
  html__tag ("", "/SELECT", "\n");
}

void html__select_integer_range (char *name, int size, char *attributes_string,
				 int min_integer, int max_integer, int selected_integer)
{
  int i;
  char buffer[10];

  html_select_start (name, size, attributes_string);
  for (i=min_integer; i<=max_integer; i++) {
    sprintf (buffer, "%d", i);
    if (i == selected_integer)
      html_option (buffer, "SELECTED", buffer);
    else
      html_option (buffer, "", buffer);
  }
  html_select_end ();
}

/* ================================================================
 * TITLE --
 *   Print a title.
 *
 * <TITLE>string</TITLE>\n
 * ================================================================ */

void html_title_start (void)
{
  html__tag ("", "TITLE", "");
}

void html_title_end (void)
{
  html__tag ("", "/TITLE", "\n");
}

void html_title (char *string)
{
  html_title_start ();
  html__s (string);
  html_title_end ();
}

/* ================================================================
 * ul --
 *    Create an unordered list.
 *      - attributes (optional): 
 *        - COMPACT reduces the inter-item spacing
 *        - PLAIN creates a plain list, otherwise its bulleted
 *        - WRAP=VERT or WRAP=HORIZ makes vertical or horizontal tables
 *      - item: the list item
 *      - src (optional): the URL of an icon
 *
 * <UL PLAIN|nothing WRAP=VERT|HORIZ|nothing>
 * <LI SRC="src">item
 * ...
 * </UL>
 * ================================================================ */

void html_ul_start (char *attributes_string)
{
  html__tag_1as ("", "UL", "\n", attributes_string);
}

void html_ul_item (char *item, char *src)
{
  html__tag_1ks ("", "LI", item, "SRC", src);
  html__nl ();
}

void html_ul_end ()
{
  html__tag ("", "/UL", "\n");
}


/* ================================================================
   ================================================================

   Meta-functions:

   ================================================================
   ================================================================ */


/* ================================================================
 * centered item --
 *   Use for centering strings, titles, bold strings, etc.
 *   For example, to print a centered header:
 *     html_centered_item ("H1", "hello")
 *
 * <CENTER> <item_type>"item"</item_type> </CENTER>\n
 * ================================================================ */

void html__centered_item_start (char *item_type)
{
  html__s1s ("<CENTER> <%s>", item_type);
}

void html__centered_item_end (char *item_type)
{
  html__s1s ("</%s> </CENTER>\n", item_type);
}

void html__centered_item (char *item_type, char *item)
{
  html__centered_item_start (item_type);
  html__s (item);
  html__centered_item_end (item_type);
}


/* ================================================================
   ================================================================

   TAG CREATION UTILITY FUNCTIONS

   Most of the functions above use the utility functions to create
   their output.  Occasionally, they will use the base printing
   functions in the next section.

   ================================================================
   ================================================================ */


/* ================================================================
 * __kd --
 *   Print a keyword-integer pair if int_value >= 0
 *
 * keyword=int_value
 * ================================================================ */

void html__kd (char *keyword, int int_value)
{
  if (keyword && (strlen (keyword) > 0) && (int_value >= 0)) {
    html__s1s (" %s=", keyword);
    html__d (int_value);
  }
}

/* ================================================================
 * __ks --
 *   Print a keyword-string pair if the string is not empty.
 *
 * keyword="string_value"
 * ================================================================ */

void html__ks (char *keyword, char *string_value)
{
  if (keyword && (strlen (keyword) > 0) &&
      string_value && (strlen (string_value) > 0))
    html__s2s (" %s=\"%s\"", keyword, string_value);
}

/* ================================================================
 * __nl --
 *   Print a newline.
 *
 * \n
 * ================================================================ */

void html__nl (void)
{
  html__s ("\n");
}

/* ================================================================
 * __tag --
 *   Print a string with two format arguments.
 *
 * string
 * ================================================================ */

void html__tag (char *pre_string, char *tagname, char *post_string)
{
  html__s (pre_string);
  html__s1s ("<%s>", tagname);
  html__s (post_string);
}

void html__tag_1as (char *pre_string, char *tagname, char *post_string, char *attributes_string)
{
  html__s (pre_string);
  html__s1s ("<%s", tagname);
  html__as (attributes_string);
  html__s (">");
  html__s (post_string);
}

void html__tag_1d (char *pre_string, char *tagname, char *post_string, int int_value1)
{
  html__s (pre_string);
  html__s1s ("<%s", tagname);
  if (int_value1 > 0)
    html__d (int_value1);
  html__s (">");
  html__s (post_string);
}

void html__tag_1kd (char *pre_string, char *tagname, char *post_string, 
		    char *keyword1, int int_value1)
{
  html__s (pre_string);
  html__s1s ("<%s", tagname);
  html__kd (keyword1, int_value1);
  html__s (">");
  html__s (post_string);
}

void html__tag_1ks (char *pre_string, char *tagname, char *post_string, 
		    char *keyword1, char *string_value1)
{
  html__s (pre_string);
  html__s1s ("<%s", tagname);
  html__ks (keyword1, string_value1);
  html__s (">");
  html__s (post_string);
}

void html__tag_2ks (char *pre_string, char *tagname, char *post_string, 
		    char *keyword1, char *string_value1,
		    char *keyword2, char *string_value2)
{
  html__s (pre_string);
  html__s1s ("<%s", tagname);
  html__ks (keyword1, string_value1);
  html__ks (keyword2, string_value2);
  html__s (">");
  html__s (post_string);
}

void html__tag_1s (char *pre_string, char *tagname, char *post_string, char *optional_string)
{
  html__s (pre_string);
  html__s1s ("<%s", tagname);
  html__s (optional_string);
  html__s (">");
  html__s (post_string);
}

/* ================================================================
 * snl --
 *   Print a string, followed by newline.
 *
 * string\n
 * ================================================================ */

void html__snl (char *string)
{
  html__s1s ("%s\n", string);
}



/* ================================================================
   ================================================================

   Base Printing functions.  All printing is done through the following
   functions.

   ================================================================
   ================================================================ */


/* ================================================================
 * __set_html_stream --
 *   Set the html__stream static variable used by all print functions.
 *
 * ================================================================ */

void html__set_html_stream (FILE *new_stream)
{
  html__stream = new_stream;
}

/* ================================================================
 * __get_html_stream --
 *   Get the current html__stream static variable used by all print functions.
 *
 * ================================================================ */

FILE *html__get_html_stream (void)
{
  return (html__stream);
}

/* ================================================================
 * __as --
 *   Print an attributes string with a leading blank if its not empty.
 *
 *  attributes_string (note the leading blank)
 * ================================================================ */

void html__as (char *attributes_string)
{
  if (attributes_string && (strlen (attributes_string) > 0))
    fprintf (html__stream, " %s", attributes_string);
}

/* ================================================================
 * __d --
 *   Print an integer.
 *
 * string
 * ================================================================ */

void html__d (int i)
{
  fprintf (html__stream, "%d", i);
}

/* ================================================================
 * __s --
 *   Print a string.
 *
 * string
 * ================================================================ */

void html__s (char *string)
{
  if (string && (strlen (string) > 0))
    fprintf (html__stream, string);
}

/* ================================================================
 * __s1d --
 *   Print a string with one embedded integer format argument.
 *
 * string
 * ================================================================ */

void html__s1d (char *string, int i)
{
  fprintf (html__stream, string, i);
}

/* ================================================================
 * __s1s --
 *   Print a string with one embedded string format argument.
 *
 * string
 * ================================================================ */

void html__s1s (char *string, char *arg1)
{
  fprintf (html__stream, string, arg1);
}

/* ================================================================
 * __s2s --
 *   Print a string with two embedded string format arguments.
 *
 * string
 * ================================================================ */

void html__s2s (char *string, char *arg1, char * arg2)
{
  fprintf (html__stream, string, arg1, arg2);
}


/*

A 
      id, name, shape, href, rel, rev, methods, effect, print, title, type, size, lang 
ABBREV 
      id, lang, index 
ABSTRACT 
      id, lang, index 
ACRONYM 
      id, lang, index 
ADDED 
      id, lang, index 
ADDRESS 
ARG 
      id, lang, index 
B 
      id, lang, index 
BASE 
      href 
BLOCKQUOTE 
BODY 
BOX 
BR 
BYLINE 
      id, lang, index 
CAPTION 
      id, lang, index 
CHANGED 
      id, idref 
CITE 
      id, lang, index 
CMD 
      id, lang, index 
CODE 
      id, lang, index 
DD 
      id, lang, index 
DFN 
      id, lang, index 
DIR 
      id, lang, index 
DL 
      id, compact, lang, index 
DT 
      id, lang, index 
EM 
      id, lang, index 
FIG 
      id, align, ismap, src, index, lang 
FOOTNOTE 
      id, lang, index 
FORM 
      id, action, method, lang, index 
H1 
      id, lang, index 
H2 
      id, lang, index 
H3 
      id, lang, index 
H4 
      id, lang, index 
H5 
      id, lang, index 
H6 
      id, lang, index 
HEAD 
HTMLPLUS 
      version, forms 
I 
      id, lang, index 
IMAGE 
      src, align, seethru, ismap, lang 
IMG 
      src, align, seethru, alt, ismap 
INPUT 
      name, type, size, min, max, value, checked, disabled, error, src, align, lang 
ISINDEX 
      href 
KBD 
      id, lang, index 
L 
      id, align, lang, index 
LI 
      id, src, lang, index 
LINK 
      idref, href, rel, rev, methods 
LIT 
      id, lang, index 
MARGIN 
      id, lang, index 
MATH 
      id 
MENU 
      id, lang, index 
NEXTID 
      n 
NOTE 
      id, role, lang, index 
OL 
      id, compact, lang, index 
OPTION 
      selected, disabled, lang 
OVER 
P 
      id, align, lang, index 
PERSON 
      id, lang, index 
PRE 
      id, lang, index 
Q 
      id, lang, index 
QUOTE 
      id, lang, index 
RENDER 
      tag, style 
REMOVED 
      id, lang, index 
S 
      id, lang, index 
SAMP 
      id, lang, index 
SELECT 
      name, several, error, lang 
STRONG 
      id, lang, index 
SUB 
      id, lang, index 
SUP 
      id, lang, index 
TAB 
      at, align 
TABLE 
      id, border, lang, index 
TD 
      colspan, rowspan, align, lang 
TEXTAREA 
      name, cols, rows, disabled, error, lang 
TH 
      colspan, rowspan, align, lang 
TITLE 
      id, lang, index 
TR 
      id 
TT 
      id, lang, index 
U 
      id, lang, index 
UL 
      id, compact, plain, wrap, lang, index 
VAR 
      id, lang, index 

*/
