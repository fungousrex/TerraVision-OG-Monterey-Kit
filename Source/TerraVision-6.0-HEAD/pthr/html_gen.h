/*
 * Name: html_gen.h
 *
 * Description: 
 *   Part of the pthr library: a collection of thread-safe functions
 *   and datatypes.
 *
 * Revision History:
 *   $Id: html_gen.h,v 1.2 2000/08/29 00:31:28 reddy Exp $
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

#ifndef HTML_H
#define HTML_H

#include <stdio.h>
#include <string.h>

/* Prototypes */

/* ================================================================
   ================================================================

   TAG FUNCTIONS:

   These are the user-level functions for creating tags and their
   arguments.

   ================================================================
   ================================================================ */

void html_a_href_start (char *);
void html_a_href_end (char *);
void html_a_href (char *, char *);

void html_a_name (char *);

void html_address_start (void);
void html_address_end (void);
void html_address (char *);

void html_b_start (void);
void html_b_end (void);
void html_b (char *);

void html_body_start (void);
void html_body_end (void);

void html_br (void);

void html_center_start (void);
void html_center_end (void);
void html_center (char *);

void html_dl_start (char *);
void html_dt (char *);
void html_dd (char *);
void html_dl_item (char *, char *);
void html_dl_end (void);

void html_form_start (char *, char *);
void html_form_end (void);

void html_h_start (int);
void html_h_end (int);
void html_h (int, char *);

void html_head_start (void);
void html_head_end (void);

void html_html_start (void);
void html_html_end (void);

void html_hr (void);

void html_img (char *, char *);

void html_input_blank (char *, int, int, char *);
void html_input_checkbox (char *, char *, char *);
void html_input_hidden (char *, char *);
void html_input_radio (char *, char *, char *);
void html_input_radio__2d (char *, int, char *, int);
void html_input_radio__2s (char *, char *, char *, char *);
void html_input_reset (char *, char *);
void html_input_submit (char *, char *);

void html_li (char *);

void html_ol_start (char *);
void html_ol_item (char *);
void html_ol_end (void);

void html_option (char *, char *, char *);

void html_p (void);

void html_pre_start (void);
void html_pre_end (void);
void html_pre (char *);

void html_select_start (char *, int, char *);
void html_select_end (void);

void html_title_start (void);
void html_title_end (void);
void html_title (char *);

void html_ul_start (char *);
void html_ul_item (char *, char *);
void html_ul_end (void);

/* ================================================================
   ================================================================

   Meta-functions:

   ================================================================
   ================================================================ */

void html__centered_item_start (char *);
void html__centered_item_end (char *);
void html__centered_item (char *, char *);

void html__html_header_start (char *);
void html__html_header_end (void);

void html__html_headern_start (char *, int, char *);
void html__html_headern_end (void);

void html__select_integer_range (char *, int, char *, int, int, int);

/* ================================================================
   ================================================================

   TAG CREATION UTILITY FUNCTIONS

   Most of the functions above use the utility functions to create
   their output.  Occasionally, they will use the base printing
   functions in the next section.

   ================================================================
   ================================================================ */

void html__kd (char *, int);
void html__ks (char *, char *);
void html__nl (void);
void html__tag (char *, char *, char *);
void html__tag_1as (char *, char *, char *, char *);
void html__tag_1d  (char *, char *, char *, int);
void html__tag_1kd (char *, char *, char *, char *, int);
void html__tag_1ks (char *, char *, char *, char *, char *);
void html__tag_2ks (char *, char *, char *, char *, char *, char *, char *);
void html__tag_1s (char *, char *, char *, char *);
void html__snl (char *);

/* ================================================================
   ================================================================

   Base Printing functions.  All printing is done through the following
   functions.

   ================================================================
   ================================================================ */

void html__set_html_stream (FILE *);
FILE *html__get_html_stream (void);
void html__as (char *);
void html__d (int);
void html__s (char *);
void html__s1d (char *, int);
void html__s1s (char *, char *);
void html__s2s (char *, char *, char *);


#endif
