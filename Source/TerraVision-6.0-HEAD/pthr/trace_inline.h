/*
 * Name: trace_inline.h
 *
 * Description: 
 *   Part of the pthr library: a collection of thread-safe functions
 *   and datatypes.
 *
 * Revision History:
 *   $Id: trace_inline.h,v 1.3 2000/08/29 00:31:38 reddy Exp $
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

#ifdef __cplusplus
extern "C" {
#endif

extern int ptrace_mask;
extern int ptrace_level;

#ifdef __cplusplus
}
#endif

exinline int
pthr_trace_level (int level)
{
  int old_level = ptrace_level;
  if (level >= 0)
    ptrace_level = (level >= PTRACE_MASK ? PTRACE_MASK-1 : level);
  return old_level;
}

exinline int
pthr_trace_mask (int mask)
{
  int old_mask = ptrace_mask;
  ptrace_mask = mask;
  return old_mask;
}

exinline int
pthr_tracep (int level)
{
  int mask = level & ~(PTRACE_MASK-1);
  level &= (PTRACE_MASK-1);
  return ((! mask || (mask & ptrace_mask)) 
	  && level <= ptrace_level);
}

exinline int
pthr_trace_indent (int level, int inc)
{
  return (pthr_tracep (level) ? pthr_trace_indent1 (inc) : 0);
}
