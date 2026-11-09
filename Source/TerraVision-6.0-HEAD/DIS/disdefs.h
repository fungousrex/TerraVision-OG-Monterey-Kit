/* File:	disdefs.h
 * Description:	Common info
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

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <prthread.h>
#include <prlock.h>
#include <prio.h>

extern int errno;

#ifndef FALSE
#   define FALSE		0
#endif
#ifndef TRUE
#   define TRUE			!FALSE
#endif

#undef SYSLOG
#ifdef SYSLOG
#   include <syslog.h>
#   define SYSLOG_FACILITY	LOG_LOCAL7
#   define DIS_APP_NAME		"DIS Lib"
#endif

#if 1
#define UDP_SEND_PORT	2099
#define UDP_RECV_PORT	3000
#else
#define UDP_SEND_PORT	3099
#define UDP_RECV_PORT	(UDP_SEND_PORT+1)
#endif

/* DIS */
#include "pdu.h"	/* list this first */
#include "appearance.h"
#include "articulat.h"
#include "category.h"
#include "country.h"
#include "datum.h"
#include "radar.h"
#include "repair.h"
#include "sites.h"

/* net_read() status return struct */
struct readstat {
   PDUType	rs_type;
   int		rs_wasted;	/* wasted (overwritten) PDUs */
   int		rs_swap_buffers_flag;
};

/* Prototypes */
void	assign_host_id();
int	init_arena();
void	net_close();
int	net_open(int buf_len);
union PDU *net_read(struct readstat *rstat);
int	net_write(char *pdu, PDUType type);
void	printArticulatParams(ArticulatParams *ap);
void	printBurstDescriptor(BurstDescriptor *bd);
/* void	printCollisionPDU(CollisionPDU *pdu); */
void	printDeadReckonParams(DeadReckonParams *drp);
void	printDetonationPDU(DetonationPDU *pdu);
void	printEntityID(EntityID *id);
void	printEntityLocation(EntityLocation *loc);
void	printEntityOrientation(EntityOrientation *orient);
void	printEntityRelativeLoc(EntityRelativeLoc *erl);
void	printEntityStatePDU(EntityStatePDU *pdu);
void	printEntityType(EntityType *t);
void	printEventID(EventID *id);
void	printFirePDU(FirePDU *pdu);
void	printPDU(char *pdu);
/* void	printRemoveEntityPDU(RemoveEntityPDU *pdu); */
/* void	printRepairCompletePDU(RepairCompletePDU *pdu); */
/* void	printRepairResponsePDU(RepairResponsePDU *pdu); */
/* void	printResupplyCancelPDU(ResupplyCancelPDU *pdu); */
/* void	printResupplyPDU(ResupplyPDU *pdu); */
/* void	printServiceRequestPDU(ServiceRequestPDU *pdu); */
void	printTitle(char *title, PDUHeader *header);
void	printVelocityVector(VelocityVector *vec);
void	receiver_process(void *);
void	swap_net_buffers();


#include <string.h>
#if !( defined(WIN32) && defined(MSVC) )
#include <unistd.h>
#endif
/* EOF */
