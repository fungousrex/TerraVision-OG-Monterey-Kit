#ifdef RPC_HDR
%#ifndef _tsmPurlDefns_h_
%#define _tsmPurlDefns_h_
%#include <rpc/rpc.h>
#endif

enum TsmProtocol {
  tsmUnknownProtocol = 0,
  tsmNoProtocol,
  tsmDSM,
  tsmFile,
  tsmHTTP,
  tsmISS,
  tsmLocalFile,
  tsmDPSS,
  tsmWMT,
  tsmNumProtocols
};

/* ================================================================
   The Purl struct.  Note that all strings except the protocolString
   are copies and can be freed.  But use tsmFreePurl to be safe. See
   the notes on parseUrl below for a better understanding of the
   fields. 
   ================================================================ */

struct Purl {
  string originalUrl<>;		/* A COPY of the original URL. */
  string url<>;			/* The reconstructed URL after parsing. */
  TsmProtocol protocol;		/* The protocol. */
  string protocolString<>;	/* A CONSTANT string representation of the protocol. */
  string hostName<>;		/* A COPY of the hostname from the original URL. */
  string port<>;                  /* The port number of the URL; defaults to 80. */
  string uri<>;			/* A COPY of the uri.  Guaranteed to start with "/". */
};

#ifdef RPC_HDR
%#endif /* _tsmPurlDefns_h_ */
#endif
