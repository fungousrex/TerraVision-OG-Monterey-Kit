#ifndef _rpc_types_h_
#define _rpc_types_h_

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#else
#include <prtypes.h>

typedef PRUint32 u_int;
typedef PRUint16 u_short;
#endif

typedef int bool_t;

#endif /* _rpc_types_h_ */
