/* File:	client_lib.c
 * Description:	Client communications library - DIS package
 * Revision:	3.1 - 04May98
 *
 * Reference:	Military Standard--Protocol Data Units for Entity
 *		Information and Entity Interaction in a Distributed
 *		Interactive Simulation (DIS) (30Oct91)
 *
 * Authors:	John Locke, Paul Barham
 *		CS Department, Naval Postgraduate School
 * Internet:	jxxl@cs.nps.navy.mil, barham@cs.nps.navy.mil
 */

#include "disdefs.h"

#include <prnetdb.h>
#include <prsystem.h>

/* Networking globals */
PRFileDesc *		sock_send;
static PRFileDesc *	sock_recv;
static PRNetAddr	dest;
unsigned short		host_id;
PDUType			pdulist[MAX_NUM_PDUS]; /* PDU types to receive */

#define RECV_TIMEOUT (100U)
#define SEND_TIMEOUT (1000U)

/* Arena globals */

#define BUF1		0
#define BUF2		1
typedef struct {
   PRThread *		receiver_thread;

   /* */
   int			current_net_buf;
			  /* PDUs from net go into current buffer; the */
			  /*   application reads PDUs from the other buf */

   PRLock		*swap_buf_lock;	/* control access to swap_buf_flag */
   int			swap_buf_flag;
   /* */
   int			buf1_oldest;
   int			buf1_newest;
   int			buf1_wasted;	/* overwritten PDUs */
   int			nodes_in_buf1;
   union PDU		*pdu_buf1;	/* PDU array; size selected through */
					/*   arg to net_open() */
   /* */
   int			buf2_oldest;
   int			buf2_newest;
   int			buf2_wasted;	/* overwritten PDUs */
   int			nodes_in_buf2;
   union PDU		*pdu_buf2;
} PDUarena;
PDUarena		*PDUarena_ptr;
int			pdus_in_buffer;


typedef struct {
  char const *name;
  int n_addrs;
  PRNetAddr *addrs;
} host_descr;

static host_descr my_host;

int get_my_host_descr (host_descr *host)
{
  char hostname[64];
  char buf[PR_NETDB_BUF_SIZE];
  PRHostEnt hosts;
  PRIntn n = 0;
  PRNetAddr addr;
  int i, len;

  if (PR_GetSystemInfo (PR_SI_HOSTNAME, hostname, 64) == PR_FAILURE)
    return PR_FAILURE;
  host->name = strdup (hostname);

  if (PR_GetHostByName (hostname, buf, PR_NETDB_BUF_SIZE, &hosts)
      == PR_FAILURE)
    return PR_FAILURE;

  len = 4;
  host->n_addrs = 0;
  host->addrs = malloc (len * sizeof (host->addrs[0]));

  while ((n = PR_EnumerateHostEnt (n, &hosts, 0, &addr)) != 0) {
    if ((i = ++host->n_addrs) >= len) {
      len *= 2;
      host->addrs = realloc (host->addrs, len * sizeof (host->addrs[0]));
    }
    host->addrs[i] = addr;
  }
  
  host->addrs = realloc (host->addrs, host->n_addrs * sizeof (host->addrs[0]));
  
  return PR_SUCCESS;
}

static int
addr_from_host (host_descr const *host, PRNetAddr const *addr)
{
  int i;
  for (i = 0; i < host->n_addrs; ++i) {
    if (addr->inet.ip == host->addrs[i].inet.ip)
      return 1;
  }
  return 0;
}

void assign_host_id(host_descr const *host)
{
  PRInt32 ip = PR_ntohl (host->addrs[0].inet.ip);
  /* Assumes a netmask of 255.255.255.0 */
  host_id = (ip & 0xff);
}


int net_open(int buf_len)
{
   int			i, on = 1;
   int			valid_interface = FALSE;
   char			buf[BUFSIZ];
   PRNetAddr            sin_send, sin_recv;
   struct sockaddr_in	*in_addr;


   /* Set up arena */
   pdus_in_buffer = buf_len;
   if (init_arena() == FALSE)
      return (FALSE);

   get_my_host_descr (&my_host);
   assign_host_id (&my_host);

   /* Create sockets */
   if ((sock_send = PR_NewUDPSocket ()) == NULL) {
      perror("PR_NewUDPSocket()");
      return (FALSE);
   }
   if ((sock_recv = PR_NewUDPSocket ()) == NULL) {
      perror("PR_NewUDPSocket()");
      return (FALSE);
   }

   
   /* Mark send socket for broadcasting */
   {
     PRSocketOptionData opt;
     opt.option = PR_SockOpt_Broadcast;
     opt.value.broadcast = TRUE;

     if (PR_SetSocketOption (sock_send, &opt) == PR_FAILURE) {
       perror("PR_SetSocketOption (PR_SockOpt_Broadcast)");
       return (FALSE);
     }
   }

   /* Bind port numbers to sockets */
   sin_send.inet.family = PR_AF_INET;
   sin_send.inet.port = PR_htons (UDP_SEND_PORT);
   sin_send.inet.ip = PR_INADDR_ANY;
   if (PR_Bind (sock_send, &sin_send) == PR_FAILURE) {
      perror("PR_Bind (sock_send)");
      return (FALSE);
   }

   sin_recv.inet.family = PR_AF_INET;
   sin_recv.inet.port = PR_htons (UDP_RECV_PORT);
   sin_recv.inet.ip = PR_INADDR_ANY;
   if (PR_Bind (sock_recv, &sin_recv) == PR_FAILURE) {
      perror("PR_Bind (sock_recv)");
      return (FALSE);
   }

   dest.inet.family = PR_AF_INET;
   dest.inet.port = PR_htons (UDP_RECV_PORT);
   dest.inet.ip = PR_INADDR_BROADCAST;

   /* Spawn the receiving process */
   PDUarena_ptr->receiver_thread
     = PR_CreateThread (PR_SYSTEM_THREAD,
			receiver_process, NULL,
			PR_PRIORITY_NORMAL, PR_GLOBAL_THREAD, 
			PR_JOINABLE_THREAD, 0);
   if (PDUarena_ptr->receiver_thread == NULL)  {
     perror("PR_CreateThread()");
     return (FALSE);
   }

   return (TRUE);
}


int init_arena()
{
#ifdef DEBUG2
   printf("Enter init_arena()\n");
   fflush(stdout);
#endif

   /* malloc root data structure */
   if ((PDUarena_ptr = (PDUarena *) malloc(sizeof(PDUarena))) == NULL) {
      perror("malloc(PDUarena)");
      return (FALSE);
   }

   /* malloc PDU buffers */
   if ((PDUarena_ptr->pdu_buf1 = (union PDU *)
           malloc((sizeof(union PDU)*pdus_in_buffer))) == NULL) {
      perror("malloc(buf1)");
      return (FALSE);
   }
   if ((PDUarena_ptr->pdu_buf2 = (union PDU *)
           malloc((sizeof(union PDU)*pdus_in_buffer))) == NULL) {
      perror("malloc(buf2)");
      return (FALSE);
   }

   /* Init semaphore */
   if ((PDUarena_ptr->swap_buf_lock = PR_NewLock ()) == NULL) {
      perror("PR_NewLock()");
      return (FALSE);
   }

   /* Data initializations */
   PDUarena_ptr->current_net_buf = BUF1;
   PDUarena_ptr->swap_buf_flag = FALSE;
   PDUarena_ptr->buf1_oldest = 0;
   PDUarena_ptr->buf1_newest = 0;
   PDUarena_ptr->nodes_in_buf1 = 0;
   PDUarena_ptr->buf2_oldest = 0;
   PDUarena_ptr->buf2_newest = 0;
   PDUarena_ptr->nodes_in_buf2 = 0;

   return (TRUE);

} /* init_arena() */


void swap_net_buffers()
{
  /* Simply switch current buffer var */
  PR_Lock (PDUarena_ptr->swap_buf_lock); {
#ifdef DEBUG2
    printf("Swapping buffers\n");
    fflush(stdout);
#endif
    if (PDUarena_ptr->current_net_buf == BUF1) {
      /* switch to buf2 */
      PDUarena_ptr->current_net_buf = BUF2;
    } else {
      /* switch to buf1 */
      PDUarena_ptr->current_net_buf = BUF1;
    }
    PDUarena_ptr->swap_buf_flag = TRUE;
  }; PR_Unlock (PDUarena_ptr->swap_buf_lock);
}


union PDU *net_read(struct readstat *rstat)
   /* Return pointer to oldest currently available PDU; fill rstat with */
   /*   status on individual PDU and buffer state; request buffer swap */
   /*   when buffer is emptied or for other reasons */
{
   union PDU	*tmp_ptr;
   PDUHeader	*header;

   /* Force buffer swap */
   if (rstat->rs_swap_buffers_flag == TRUE) {
      swap_net_buffers();
      rstat->rs_swap_buffers_flag = FALSE;
   }

   /* Use the buffer not in use by receiver_process() */
   if (PDUarena_ptr->current_net_buf == BUF1) {
      /* Any PDUs available? */
      if (PDUarena_ptr->nodes_in_buf2 == 0) {
         /* BUF2 empty */
         swap_net_buffers();
         if (PDUarena_ptr->nodes_in_buf1 == 0) {
            /* both buffers empty */
            return (NULL);
         } else {
            /* PDUs available in BUF1 */
            tmp_ptr = &(PDUarena_ptr->pdu_buf1[PDUarena_ptr->buf1_oldest]);
         }
      } else {
         /* PDUs available in BUF2 */
         tmp_ptr = &(PDUarena_ptr->pdu_buf2[PDUarena_ptr->buf2_oldest]);
      }
   } else { /* (PDUarena_ptr->current_net_buf == BUF2) */
      /* Any PDUs available? */
      if (PDUarena_ptr->nodes_in_buf1 == 0) {
         /* BUF1 empty */
         swap_net_buffers();
         if (PDUarena_ptr->nodes_in_buf2 == 0) {
            /* both buffers empty */
            return (NULL);
         } else {
            /* PDUs available in BUF2 */
            tmp_ptr = &(PDUarena_ptr->pdu_buf2[PDUarena_ptr->buf2_oldest]);
         }
      } else {
         /* PDUs available in BUF1 */
         tmp_ptr = &(PDUarena_ptr->pdu_buf1[PDUarena_ptr->buf1_oldest]);
      }
   }

   /* Now we have a ptr to buffer with nodes; update buffer state */
   if (PDUarena_ptr->current_net_buf == BUF1) {
      rstat->rs_wasted = PDUarena_ptr->buf2_wasted;
      PDUarena_ptr->nodes_in_buf2--;
      if (PDUarena_ptr->nodes_in_buf2 == 0) {
         /* plan ahead for next net_read() */
         swap_net_buffers();
      } else {
         /* update buffer state */
         PDUarena_ptr->buf2_oldest++;
         if (PDUarena_ptr->buf2_oldest > (pdus_in_buffer-1))
            /* circle the buffer */
            PDUarena_ptr->buf2_oldest = 0;
      }
   } else { /* (PDUarena_ptr->current_net_buf == BUF2) */
      rstat->rs_wasted = PDUarena_ptr->buf1_wasted;
      PDUarena_ptr->nodes_in_buf1--;
      if (PDUarena_ptr->nodes_in_buf1 == 0) {
         /* plan ahead for next net_read() */
         swap_net_buffers();
      } else {
         /* update buffer state */
         PDUarena_ptr->buf1_oldest++;
         if (PDUarena_ptr->buf1_oldest > (pdus_in_buffer-1))
            /* circle the buffer */
            PDUarena_ptr->buf1_oldest = 0;
      }
   }
   
   /* Fill read status structure */
   header = (PDUHeader *) tmp_ptr;
   rstat->rs_type = header->type;

   return (tmp_ptr);
}


void receiver_process(void *ignore)
{
   int			i;
   register int		buf_dimension;
   PRNetAddr		from;
   int			oldest = 0, newest = 0;	/* array indexes */
   int			circled = FALSE;
   union PDU		input_buf, *curr_buf;
   PDUHeader		*header = (PDUHeader *) &input_buf;
#ifdef DEBUG2
   int			j = 0;
#endif


   /* Initialize local vars */
   buf_dimension = (pdus_in_buffer-1);
   PR_Lock (PDUarena_ptr->swap_buf_lock); { /* acquire semaphore */
      switch (PDUarena_ptr->current_net_buf) {
         /* get pointer to the correct buffer */
         case (BUF1):
            /* pointer to current buffer; must be handled */
            /*   as local var (i.e. not in arena) in case application */
            /*   swaps buffers between reading the packet */
            curr_buf = (union PDU *) PDUarena_ptr->pdu_buf1;
            break;
         case (BUF2):
            curr_buf = (union PDU *) PDUarena_ptr->pdu_buf2;
            break;
         default:
            /* shouldn't get here */
            printf("Invalid net buffer.\n");
            fflush(stdout);
      }
   }; PR_Unlock (PDUarena_ptr->swap_buf_lock); /* release semaphore */


   /* NETWORK READ LOOP */
   while (TRUE) {
top_of_loop:

      /* Read packet off network into temp buffer */
      /*   Net read must be done while semaphore is released to prevent */
      /*   hanging application while no packets are incoming */
      if (PR_RecvFrom (sock_recv, (void *) &input_buf, sizeof(union PDU),
		       0, &from, RECV_TIMEOUT) == PR_FAILURE) {
         perror("PR_RecvFrom()");
      }

      /* Reject our own broadcasts */
      if (addr_from_host (&my_host, &from)) {
	continue;	/* don't process packet further */
      }

      /* Is it a PDU on the PDUs-to-receive list? */
      for (i = 0; i < MAX_NUM_PDUS; i++) {
         if (pdulist[i] == header->type)   /* A match -- proceed */
            break;
         if (pdulist[i] == (PDUType) 0) {  /* End of receive list -- no match */
            goto top_of_loop;
         }
      }

      /* Save packet, update buffer indexes */
      /*   (this must be done after acquiring semaphore to block */
      /*    buffer swap by application) */
      PR_Lock (PDUarena_ptr->swap_buf_lock); { /* acquire semaphore */
         /* Swap buffers? */
         if (PDUarena_ptr->swap_buf_flag == TRUE) {
            switch (PDUarena_ptr->current_net_buf) {	/* BUF1 or BUF2 */
               /* get pointer to the correct buffer */
               case (BUF1):
                  curr_buf = (union PDU *) PDUarena_ptr->pdu_buf1;
                  PDUarena_ptr->nodes_in_buf1 = 0;
                  PDUarena_ptr->buf1_wasted = 0;
                  break;
               case (BUF2):
                  curr_buf = (union PDU *) PDUarena_ptr->pdu_buf2;
                  PDUarena_ptr->nodes_in_buf2 = 0;
                  PDUarena_ptr->buf2_wasted = 0;
                  break;
               default:
                  /* booboo time */
                  printf("Invalid net buffer.\n");
                  fflush(stdout);
            }
            /* Buffers swapped so start at beginning of buf */
            oldest = newest = 0;
            circled = FALSE;
            /* */
            PDUarena_ptr->swap_buf_flag = FALSE;
         }

         /* Copy new packet to open slot in current buffer */
         memcpy((void *) &(curr_buf[newest]), (void *) &input_buf,
		header->length);

#ifdef DEBUG2
         printPDU((char *) &(curr_buf[newest]));
         fflush(stdout);
#endif
#ifdef DEBUG2
         j++;
         printf("%d:oldest,newest = %d,%d\n", j, oldest, newest);
         fflush(stdout);
#endif

         /* Save buffer state in case swap occurs */
         if (PDUarena_ptr->current_net_buf == BUF1) {
            PDUarena_ptr->buf1_oldest = oldest;
            PDUarena_ptr->buf1_newest = newest;
         } else {
            /* BUF2 */
            PDUarena_ptr->buf2_oldest = oldest;
            PDUarena_ptr->buf2_newest = newest;
         }

         /* Update oldest and newest node values of circular buffer for next */
         /*   packet */
         newest++;
         if (!circled) {	/* first pass through buf is the special case */
            if (PDUarena_ptr->current_net_buf == BUF1) {
               PDUarena_ptr->nodes_in_buf1++;
            } else {
               /* BUF2 */
               PDUarena_ptr->nodes_in_buf2++;
            }
            if (newest > buf_dimension) { /* back to beginning */
               newest = 0, oldest = 1;
               circled = TRUE;
            }
         } else {	/* buf is full; all but first pass */
            oldest = newest + 1;	/* chasing our tail now */
            if (oldest > buf_dimension)
               oldest = 0;
            if (newest > buf_dimension)
               newest = 0, oldest = 1;
            /* track overwritten PDUs */
            if (PDUarena_ptr->current_net_buf == BUF1) {
               PDUarena_ptr->buf1_wasted++;
            } else {
               PDUarena_ptr->buf2_wasted++;
            }
         }

#ifdef DEBUG2
         if (PDUarena_ptr->current_net_buf == BUF1)
            printf("nodes_in_buf1 = %d\n", PDUarena_ptr->nodes_in_buf1);
         else
            printf("nodes_in_buf2 = %d\n", PDUarena_ptr->nodes_in_buf2);
         fflush(stdout);
#endif
      } /* end processing of single packet */
      PR_Unlock (PDUarena_ptr->swap_buf_lock); /* release semaphore */

   } /* end while(TRUE) */

} /* receiver_process() */


int net_write(char *pdu, PDUType type)
{
   int				length;
   PDUHeader			*header;
   EntityStatePDU		*ESpdu;
   DetonationPDU		*Dpdu;
   ServiceRequestPDU		*SRpdu;
   ResupplyPDU			*Rpdu;

#ifdef DEBUG2
   printf("Enter net_write()\n");
   fflush(stdout);
#endif

   /* Fill in header data */
   header = (PDUHeader *) pdu;
#ifdef DIS_1_0
   header->protocol_version = DISVersionMay92;
#endif
#ifdef DIS_2_0
   header->protocol_version = DISVersionMay93;
#endif
   header->exercise_ident = 1;                  /* hardwired, for now */
   header->type = type;

   /* calculate length */
   switch (type) {
      case (EntityStatePDU_Type):
         ESpdu = (EntityStatePDU *) pdu;
         length = sizeof(EntityStatePDU);	/* max */
         length -= (sizeof(ArticulatParams) * MAX_ARTICULAT_PARAMS); /* base */
         length += (sizeof(ArticulatParams) * ESpdu->num_articulat_params);
         break;
      case (FirePDU_Type):
         length = sizeof(FirePDU);
         break;
      case (DetonationPDU_Type):
         Dpdu = (DetonationPDU *) pdu;
         length = sizeof(DetonationPDU);	/* max */
         length -= (sizeof(ArticulatParams) * MAX_ARTICULAT_PARAMS); /* base */
         length += (sizeof(ArticulatParams) * Dpdu->num_articulat_params);
         break;
      case (CollisionPDU_Type):
         length = sizeof(CollisionPDU);
         break;
      case (ServiceRequestPDU_Type):
         SRpdu = (ServiceRequestPDU *) pdu;
         length = sizeof(ServiceRequestPDU);	/* max */
         length -= (sizeof(SupplyQuantity) * MAX_SUPPLY_QTY); /* base */
         length += (sizeof(SupplyQuantity) * SRpdu->num_supply_types);
         break;
      case (ResupplyOfferPDU_Type):
      case (ResupplyReceivedPDU_Type):
         Rpdu = (ResupplyPDU *) pdu;
         length = sizeof(ResupplyPDU);	/* max */
         length -= (sizeof(SupplyQuantity) * MAX_SUPPLY_QTY); /* base */
         length += (sizeof(SupplyQuantity) * Rpdu->num_supply_types);
         break;
      case (ResupplyCancelPDU_Type):
         length = sizeof(ResupplyCancelPDU);
         break;
      case (RepairCompletePDU_Type):
         length = sizeof(RepairCompletePDU);
         break;
      case (RepairResponsePDU_Type):
         length = sizeof(RepairResponsePDU);
         break;
      default:
         length = sizeof(PDU);
         printf("net_write(): Invalid PDU Type %d", type);
   }
#ifdef DIS_2_0
   header->time_stamp = (unsigned int) time(0);	/* not DIS compliant! */
   if (length > sizeof(PDU))
      length = sizeof(PDU);
   header->length = length;
#endif

   /* Actually send it */
   if (PR_SendTo (sock_send, pdu, length, 0, &dest, SEND_TIMEOUT)
       == PR_FAILURE) {
      perror("sendto()");
      return (FALSE);
   }

   return (TRUE);
}


void net_close()
{
  PR_Close(sock_send);
  PR_Close(sock_recv);
  
  PR_Interrupt (PDUarena_ptr->receiver_thread);
  PR_JoinThread (PDUarena_ptr->receiver_thread);
}


/* EOF */
