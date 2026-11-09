/*
 * Checksum routine for Internet Protocol family headers (C Version).
 *
 * Refer to "Computing the Internet Checksum" by R. Braden, D. Borman and
 * C. Partridge, Computer Communication Review, Vol. 19, No. 2, April 1989,
 * pp. 86-101, for additional details on computing this checksum.
 */
#include <prtypes.h>

int checksum(register PRUint16 *ptr, register int  nbytes);

int                             /* return checksum in low-order 16 bits */
checksum(register PRUint16 *ptr,
	 register int  nbytes)
{
  const char *funcName = "checksum";
  PRInt32            sum;            /* assumes int == 32 bits */
  PRUint16                 oddbyte;
  PRUint16        answer;

  /*
   * Our algorithm is simple, using a 32-bit accumulator (sum),
   * we add sequential 16-bit words to it, and at the end, fold back
   * all the carry bits from the top 16 bits into the lower 16 bits.
   */
  /*
    TSMTRACE ((TSMMASK|4, funcName,
    "computing checksum on %d bytes, data = %d %d %d %d",
    nbytes, ptr[0], ptr[1], ptr[2], ptr[3]));
  */

  sum = 0;
  while (nbytes > 1)  
  {
    sum += ntohs(*ptr);
    ptr++;
    nbytes -= 2;
  }

                                /* mop up an odd byte, if necessary */
  if (nbytes == 1)
  {
    oddbyte = 0;            /* make sure top half is zero */
    *((PRUint8 *) &oddbyte) = *(PRUint8 *)ptr;   /* one byte only */
    sum += oddbyte;
  }

  /*
   * Add back carry outs from top 16 bits to low 16 bits.
   */
  sum  = (sum >> 16) + (sum & 0x0000ffff); /* add high-16 to low-16 */
  sum += (sum >> 16);                     /* add carry */
  answer = ~sum;          /* ones-complement, then truncate to 16 bits */

  /*
    TSMTRACE ((TSMMASK|4, funcName, "computed checksum to be: %u", (unsigned int)answer));
  */
  return((int)answer);
}

