/* File:	print.c
 * Description:	Routines for printing PDU fields (incomplete)
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

#define PI (3.1415926535)

void printPDU(char *pdu)
{
   PDUHeader		*header;

   if (pdu == NULL)
      return;

   header = (PDUHeader *) pdu;
   switch (header->type) {
      case (EntityStatePDU_Type):
         printTitle("EntityStatePDU", header);
         printEntityStatePDU((EntityStatePDU *) pdu);
         break;
      case (DetonationPDU_Type):
         printTitle("DetonationPDU", header);
         printDetonationPDU((DetonationPDU *) pdu);
         break;
      case (ServiceRequestPDU_Type):
         printTitle("ServiceRequestPDU", header);
         /* printServiceRequestPDU((ServiceRequestPDU *) pdu); */
         break;
      case (ResupplyOfferPDU_Type):     /* same struct */
      case (ResupplyReceivedPDU_Type):
         printTitle("ResupplyPDU", header);
         /* printResupplyPDU((ResupplyPDU *) pdu); */
         break;
#ifdef DIS_1_0
      case (ActivatRequestPDU_Type):
         printTitle("ActivatRequestPDU", header);
         printActivatRequestPDU((ActivatRequestPDU *) pdu);
         break;
      case (EmitterPDU_Type):
         printTitle("EmitterPDU", header);
         printEmitterPDU((EmitterPDU *) pdu);
         break;
      case (RadarPDU_Type):
         printTitle("RadarPDU", header);
         printRadarPDU((RadarPDU *) pdu);
         break;
      case (ActivatResponsePDU_Type):
         printTitle("ActivatResponsePDU", header);
         printActivatResponsePDU((ActivatResponsePDU *) pdu);
         break;
      case (DeactivatRequestPDU_Type):
         printTitle("DeactivatRequestPDU", header);
         printDeactivatRequestPDU((DeactivatRequestPDU *) pdu);
         break;
      case (DeactivatResponsePDU_Type):
         printTitle("DeactivatResponsePDU", header);
         printDeactivatResponsePDU((DeactivatResponsePDU *) pdu);
         break;
#endif	/* DIS_1_0 */
#ifdef DIS_2_0
      case (RemoveEntityPDU_Type):
         printTitle("RemoveEntityPDU", header);
         /* printRemoveEntityPDU((RemoveEntityPDU *) pdu); */
         break;
#endif	/* DIS_2_0 */
      case (FirePDU_Type):
         printTitle("FirePDU", header);
         printFirePDU((FirePDU *) pdu);
         break;
      case (ResupplyCancelPDU_Type):
         printTitle("ResupplyCancelPDU", header);
         /* printResupplyCancelPDU((ResupplyCancelPDU *) pdu); */
         break;
      case (RepairCompletePDU_Type):
         printTitle("RepairCompletePDU", header);
         /* printRepairCompletePDU((RepairCompletePDU *) pdu); */
         break;
      case (RepairResponsePDU_Type):
         printTitle("RepairResponsePDU", header);
         /* printRepairResponsePDU((RepairResponsePDU *) pdu); */
         break;
      case (CollisionPDU_Type):
         printTitle("CollisionPDU", header);
         /* printCollisionPDU((CollisionPDU *) pdu); */
         break;

      case (OtherPDU_Type):
      default:
         printf("\nInvalid PDU type received.\n");
   }

}



void printTitle(char *title, PDUHeader *header)
{
   printf("\n%s (v.%d), ex. %d:\n", title,
                            header->protocol_version, header->exercise_ident);
}


void printEntityStatePDU(EntityStatePDU *pdu)
{
   int			i;

   printf("  Entity ID: ");
   printEntityID(&(pdu->entity_id));

   printf("  Force ID: ");
   switch (pdu->force_id) {
#ifdef DIS_1_0
      case (ForceID_Blue):
         printf("Blue\n");
         break;
      case (ForceID_Red):
         printf("Red\n");
         break;
      case (ForceID_White):
         printf("White\n");
         break;
#endif
#ifdef DIS_2_0
      case (ForceID_Friendly):
         printf("Friendly\n");
         break;
      case (ForceID_Opposing):
         printf("Opposing\n");
         break;
      case (ForceID_Neutral):
         printf("Neutral\n");
         break;
#endif
      case (ForceID_Other):
         printf("Other\n");
         break;
      default:
         printf("Invalid\n");
   }

   printf("  Type:     ");
   printEntityType(&(pdu->entity_type));
   printf("  Alt Type: ");
   printEntityType(&(pdu->alt_entity_type));

#ifdef DIS_1_0
   printf("  Time: %d\n", pdu->time_stamp);
#endif /* DIS_1_0 */

#ifdef DIS_2_0
   printf("  Time: %d\n", pdu->entity_state_header.time_stamp);
#endif /* DIS_2_0 */

   printf("  Location: ");
   printEntityLocation(&(pdu->entity_location));

   printf("  Velocity: ");
   printVelocityVector(&(pdu->entity_velocity));

   printf("  Orientation: ");
   printEntityOrientation(&(pdu->entity_orientation));

   printf("  Dead Reckoning Params:\n");
   printDeadReckonParams(&(pdu->dead_reckon_params));

   printf("  Appearance: 0x%x\n", pdu->entity_appearance);

   printf("  Markings: \"%s\"\n", pdu->entity_marking.markings);

   printf("  Capabilities: 0x%x\n", pdu->capabilities);

   printf("  Number of articulated params: %d\n", pdu->num_articulat_params);
   for (i = 0; i < pdu->num_articulat_params; i++) {
      printf("  Articulation param %d:\n", i);
      printArticulatParams(&(pdu->articulat_params[i]));
   }

}


void printDeadReckonParams(DeadReckonParams *drp)
{
   int			i;

   printf("    Algorithm: %d\n", drp->algorithm);
   printf("    Linear accel: ");
   for (i = 0; i < 3; i++)
      printf("%f ", drp->linear_accel[i]);
   putchar('\n');
   printf("    Angular velocity: ");
   for (i = 0; i < 3; i++)
      printf("%d ", drp->angular_velocity[i]);
   putchar('\n');
}


void printArticulatParams(ArticulatParams *ap)
{
   int			i;

   printf("    change: %d, ID: %d, type: %d\n",
                        ap->change, ap->ID, ap->parameter_type);
   printf("    value: ");
   for (i = 0; i < 8; i++)
      printf("%d.", ap->parameter_value[i]);
   printf("\b \n");
}


void printEntityOrientation(EntityOrientation *orient)
{
   printf("psi: %.0f, theta: %.0f, phi: %.0f\n",
	  orient->psi*180/PI, 
	  orient->theta*180/PI,
	  orient->phi*180/PI);
}


void printVelocityVector(VelocityVector *vec)
{
   printf("x: %f, y: %f, z: %f\n", vec->x, vec->y, vec->z);
}


void printEntityLocation(EntityLocation *loc)
{
   printf("x: %f, y: %f, z: %f\n", loc->x, loc->y, loc->z);
}


void printEntityType(EntityType *t)
{
   printf("kind ");
   switch (t->entity_kind) {
      default:
         printf("%d", t->entity_kind);
   }
   printf(", dom ");
   switch (t->domain) {
      default:
         printf("%d", t->domain);
   }
   printf(", country ");
   switch (t->country) {
      case (USA):
         printf("USA");
         break;
      case (USSR):
         printf("USSR");
         break;
      default:
         printf("%d", t->country);
   }
   printf(", cat ");
   switch (t->category) {
      default:
         printf("%d", t->category);
   }
   printf(", sub ");
   switch (t->subcategory) {
      default:
         printf("%d", t->subcategory);
   }
   printf(", spec ");
   switch (t->specific) {
      default:
         printf("%d", t->specific);
   }
   printf(", ext ");
   switch (t->extra) {
      default:
         printf("%d", t->extra);
   }
   putchar('\n');
}


void printEventID(EventID *id)
{
   printf("site ");
   switch (id->address.site) {
      case (SITE_ID_NPS):
         printf("NPS");
         break;
      case (SITE_ID_AFIT):
         printf("AFIT");
         break;
      default:
         printf("%d", id->address.site);
   }
   printf(", host %d, event %d\n", id->address.host, id->event);
}


void printEntityID(EntityID *id)
{
   printf("site ");
   switch (id->address.site) {
      case (SITE_ID_NPS):
         printf("NPS");
         break;
      case (SITE_ID_AFIT):
         printf("AFIT");
         break;
      default:
         printf("%d", id->address.site);
   }
   printf(", host %d, entity %d\n", id->address.host, id->entity);
}


void printDetonationPDU(DetonationPDU *pdu)
{
   int i;

   printf("  Firing Entity ID: ");
   printEntityID(&(pdu->firing_entity_id));
   printf("  Target Entity ID: ");
   printEntityID(&(pdu->target_entity_id));
   printf("  Munition ID: ");
   printEntityID(&(pdu->munition_id));

   printf("  Event ID: ");
   printEventID(&(pdu->event_id));

   printf("  Velocity: ");
   printVelocityVector(&(pdu->velocity));

   printf("  Location: ");
   printEntityLocation(&(pdu->location_in_world));

   printf("  Burst Descriptor: ");
   printBurstDescriptor(&(pdu->burst_descriptor));
   printf("    BD Entity Type: ");
   printEntityType(&(pdu->burst_descriptor.munition));

   printf("  Location in Entity: ");
   printEntityRelativeLoc(&(pdu->location_in_entity));

   printf("  Detonation Result: %d\n", (int) pdu->detonation_result);

   printf("  Number of articulated params: %d\n", pdu->num_articulat_params);
   for (i = 0; i < pdu->num_articulat_params; i++) {
      printf("  Articulation param %d:\n", i);
      printArticulatParams(&(pdu->articulat_params[i]));
   }

} /* printDetonationPDU() */


void printEntityRelativeLoc(EntityRelativeLoc *erl)
{
   printf("x: %f, y: %f, z: %f\n", erl->x, erl->y, erl->z);
}


void printBurstDescriptor(BurstDescriptor *bd)
{
   printf("warhead %d, fuze %d, qty %d, rate %d\n", (int) bd->warhead,
                      (int) bd->fuze, (int) bd->quantity, (int) bd->rate);
}


#if 0
void printServiceRequestPDU(ServiceRequestPDU *pdu)
{
   printf("ServiceRequestPDU printing not yet implemented.\n", pdu);
}


void printResupplyPDU(ResupplyPDU *pdu)
{
   printf("ResupplyPDU printing not yet implemented.\n", pdu);
}


void printRepairResponsePDU(RepairResponsePDU *pdu)
{
   printf("RepairResponsePDU printing not yet implemented.\n", pdu);
}


void printRepairCompletePDU(RepairCompletePDU *pdu)
{
   printf("RepairCompletePDU printing not yet implemented.\n", pdu);
}


void printCollisionPDU(CollisionPDU *pdu)
{
   printf("CollisionPDU printing not yet implemented.\n", pdu);
}


void printResupplyCancelPDU(ResupplyCancelPDU *pdu)
{
   printf("ResupplyCancelPDU printing not yet implemented.\n", pdu);
}


#ifdef DIS_2_0
void printRemoveEntityPDU(RemoveEntityPDU *pdu)
{
   printf("RemoveEntityPDU printing not yet implemented.\n", pdu);
}
#endif /* DIS_2_0 */

#endif /* 0 */


void printFirePDU(FirePDU *pdu)
{
   printf("  Firing Entity ID: ");
   printEntityID(&(pdu->firing_entity_id));
   printf("  Target Entity ID: ");
   printEntityID(&(pdu->target_entity_id));
   printf("  Munition ID: ");
   printEntityID(&(pdu->munition_id));

   printf("  Event ID: ");
   printEventID(&(pdu->event_id));

   printf("  Location in World: ");
   printEntityLocation(&(pdu->location_in_world));

   printf("  Burst Descriptor: ");
   printBurstDescriptor(&(pdu->burst_descriptor));
   printf("    BD Entity Type: ");
   printEntityType(&(pdu->burst_descriptor.munition));

   printf("  Velocity: ");
   printVelocityVector(&(pdu->velocity));

   printf("  Range: %f\n", pdu->range);
}

/* EOF */
