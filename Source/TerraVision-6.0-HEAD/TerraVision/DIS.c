/* -*- tab-width: 2; -*-

  Name:	DIS.c

  Description: Manage DIS PDUs.

  Revision History:
      $Id: DIS.c,v 2.2.10.2 2002/10/31 05:26:28 reddy Exp $

  ----- BEGIN LICENSE BLOCK -----
  Version: MPL 1.1
 
  The contents of this file are subject to the Mozilla Public License Version
  1.1 (the "License"); you may not use this file except in compliance with
  the License. You may obtain a copy of the License at
  http://www.mozilla.org/MPL/
 
  Software distributed under the License is distributed on an "AS IS" basis,
  WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
  for the specific language governing rights and limitations under the
  License.
 
  The Original Code is the TerraVision 3D Terrain Visualization System.
 
  The Initial Developer of the Original Code is SRI International.

  Portions created by the Initial Developer are Copyright (C) 1992-2002
  the Initial Developer. All Rights Reserved.
 
  Contributor(s): Yvan Leclerc, Lee Iverson, Martin Reddy, Nat Bletter. 
  ----- END LICENSE BLOCK -----

*/

#include "config.h"

#include "DIS.h"
#include "Trace.h"

#include <DIS/disdefs.h>

static int DIS_site_id;

static int DIS_open = FALSE;

extern unsigned short host_id;
extern PDUType	pdulist[];

int
tvBaseStartDIS (int site_id)
{
  static char const funcName[] = "tvBaseStartDIS";

  if (DIS_open)
    return TRUE;

  /* This filters down to only ES PDUs.  Must be done before net_open() */
  pdulist[0] = EntityStatePDU_Type;
  pdulist[1] = (PDUType) 0;	/* NULL terminate list */

  if (net_open(128) == FALSE) {
    tsmError (funcName, "DIS net_open() failed");
    return FALSE;
  }

  TSMTRACE ((THREED_MASK|1, funcName, "site %d", site_id));

  DIS_site_id = site_id;
  DIS_open = TRUE;

  return TRUE;
}

void
tvBaseStopDIS (void)
{
  static char const funcName[] = "tvBaseStopDIS";

  if (DIS_open) {
    net_close ();
    DIS_open = FALSE;
    TSMTRACE ((THREED_MASK|1, funcName, ""));
  }
}

int
tvBaseSendESPDU (int force_id, int entity_id, 
		 double const location[3],
		 float const velocity[3],
		 float const orientation[3])
{
  static char const funcName[] = "tvBaseSendESPDU";

  TSMTRACE ((THREED_MASK|1, funcName, "open %d", DIS_open));
  if (DIS_open) {
    union PDU pdu;
    EntityStatePDU *ESpdu = (EntityStatePDU *) &pdu;

    memset (ESpdu, 0, sizeof (pdu));

    ESpdu->entity_id.address.site = DIS_site_id;
    ESpdu->entity_id.address.host = host_id;
    ESpdu->entity_id.entity = entity_id;
    ESpdu->force_id = force_id;

    ESpdu->entity_type.entity_kind = EntityKind_Platform;
    ESpdu->entity_type.domain = Domain_Air;
    ESpdu->entity_type.country = USA;
    ESpdu->entity_type.category = Category_UtilityHelicopter;
    ESpdu->entity_type.subcategory = UH_1;

    ESpdu->entity_location.x = location[0];
    ESpdu->entity_location.y = location[1];
    ESpdu->entity_location.z = location[2];

    ESpdu->entity_velocity.x = velocity[0];
    ESpdu->entity_velocity.y = velocity[1];
    ESpdu->entity_velocity.z = velocity[2];

    ESpdu->entity_orientation.psi   = orientation[0];
    ESpdu->entity_orientation.theta = orientation[1];
    ESpdu->entity_orientation.phi   = orientation[2];

    ESpdu->dead_reckon_params.algorithm = DRAlgo_DRM_FVW;

    TSMTRACE ((THREED_MASK|1, funcName, "Send PDU %d:%d %g,%g,%g",
	       force_id, entity_id,
	       location[0], location[1], location[2]));

    return net_write((char *) &pdu, EntityStatePDU_Type);
  }

  return FALSE;
}
