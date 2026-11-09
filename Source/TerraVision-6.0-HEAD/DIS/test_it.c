/* File:	test_it.c
 * Description:	Minimal front end to DIS client library
 * Revision:	3.1 - 04May98
 *
 * Reference:	Military Standard--Protocol Data Units for Entity
 *		Information and Entity Interaction in a Distributed
 *		Interactive Simulation (DIS) (30Oct91)
 *
 * Authors:	John Locke, Paul Barham
 *		CS Department, Naval Postgraduate School
 * Internet:	jxxl@cs.nps.navy.mil,barham@cs.nps.navy.mil
 */

#include "disdefs.h"

int main(int argc, char **argv)
{
   int			i = 0, print_help = FALSE,
			read_flag,
			write_flag,
			test_open_flag = FALSE,
			eth_flag = FALSE,
                        verbose = FALSE,
			buf_len = 0,
			recvd = 0, sent = 0;
   extern unsigned short host_id;
   extern PDUType	pdulist[];	/* PDUs to receive */
   struct readstat	rstat;	/* net_read() status return */
   PDUType		type;
   union PDU		pdu, *pdu_ptr;

   DetonationPDU	*Dpdu;
   EntityStatePDU	*ESpdu;
   FirePDU		*Fpdu;


   /* Parse command line */
   for (i = 1; i < argc; i++) {
      if ((argv[i][0] != '-') || print_help) {
         print_help = TRUE;
         break;
      }
      switch (argv[i][1]) {
         case 'h':      /* help */
            print_help = TRUE;
            break;
         case 'b':      /* set length (in PDUs) of network input buffers */
            buf_len = atoi(argv[i+1]);
            i++;
            break;
         case 'r':      /* test reading PDUs */
            read_flag = TRUE;
            write_flag = FALSE;
            rstat.rs_swap_buffers_flag = FALSE;
            break;
         case 'w':      /* test writing PDUs */
            read_flag = FALSE;
            write_flag = TRUE;
            break;
         case 't':      /* test program initialization */
            test_open_flag = TRUE;
            break;
         case 'v':
	    verbose = TRUE;
	    break;
         default:
            print_help = TRUE;
      }
   }

   if (argc == 1 || print_help) {
      printf("Usage: %s [-r | -w] [-b buffer_size] [-h]\n",
                                                                   argv[0]);
      exit(0);
   }

   /* PDUs we want to receive; all others will be filtered out; must be */
   /* done before calling net_open() */
   pdulist[0] = EntityStatePDU_Type;
   pdulist[1] = FirePDU_Type;
   pdulist[2] = DetonationPDU_Type;
   pdulist[3] = (PDUType) 0;	/* NULL terminate list */

   /* Set input buffer length (in PDUs) */
   if (buf_len == 0)	/* not set in command line */
      buf_len = 32;
   printf("Setting input buffers to %d PDUs\n", buf_len);

   if (net_open(buf_len) == FALSE) {
      printf("main(): net_open() failed\n");
      exit(1);
   }

   if (test_open_flag) {
      /* test net_open only */
      sleep(10);	/* kill time */
      exit(0);
   }

   if (read_flag) {
      /*sleep(10); */
      while (TRUE) {
         if ((pdu_ptr = net_read(&rstat)) == NULL) {
            /* printf("No PDUs available.\n"); */
         } else {
            recvd++;
            printf("Recvd %d\n", recvd);
            if (rstat.rs_wasted > 0)
               printf("WASTED = %d\n", rstat.rs_wasted);
	    if (verbose)
	       printPDU((char *) pdu_ptr);
            fflush(stdout);
         }
         if (!(recvd % 50))
            sleep(1);
      }
   }

   if (write_flag) {
      memset((void *) &pdu, 0, sizeof(union PDU));
      printf("Writing PDUs:\n");
      type = EntityStatePDU_Type;
      while (TRUE) {
         switch (type) {

            case (EntityStatePDU_Type):
               ESpdu = (EntityStatePDU *) &pdu;
               ESpdu->entity_id.address.site = SITE_ID_NPS;
               ESpdu->entity_id.address.host = host_id;
               ESpdu->force_id = ForceID_Friendly;
               ESpdu->entity_type.entity_kind = EntityKind_Platform;
               ESpdu->entity_type.domain = Domain_Land;
               ESpdu->entity_type.country = USA;
               ESpdu->entity_type.category = Category_Tank;
               ESpdu->entity_type.subcategory = M1_Abrams;
               ESpdu->entity_velocity.x = 3.0;
               ESpdu->entity_velocity.y = 1.0;
               ESpdu->entity_location.x = 18975.0;
               ESpdu->entity_location.y = 12300.0;
               ESpdu->dead_reckon_params.algorithm = DRAlgo_DRM_FVW;
               sprintf((char *) ESpdu->entity_marking.markings,
                                                         "PKT %d", sent+1);
               ESpdu->num_articulat_params = 0;
               for (i = 0; i < 2; i++, ESpdu->num_articulat_params++) {
                  ESpdu->articulat_params[i].ID = i+1;
               }
               break;

            case (FirePDU_Type):
               Fpdu = (FirePDU *) &pdu;
               Fpdu->burst_descriptor.fuze = (Fuze) FuzeMunition_Timed;
               Fpdu->range = 13.2;
               break;

            case (DetonationPDU_Type):
               Dpdu = (DetonationPDU *) &pdu;
               Dpdu->location_in_entity.x = 1.0;
               Dpdu->location_in_entity.y = 2.0;
               Dpdu->location_in_entity.z = 3.0;
               Dpdu->detonation_result =
                                (DetonationResult) DetResult_EntityImpact;
               break;

            default:
               printf("default case reached!\n");

         } /* end switch(type) */

         if (net_write((char *) &pdu, type) == TRUE) {
            sent++;
            printf("Sent %d\n", sent);
            fflush(stdout);
            /*sleep(1); */
         }
         if (!(sent % 40))
            sleep(1);
         /*
         if (sent >= 20) {
            putchar('\n');
            break;
         }
         */

      }
   }

   net_close();

   return 0;
}

/* EOF */
