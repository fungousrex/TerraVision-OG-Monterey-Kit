
/* File:	pdu.h
 * Description:	PDU structures
 * Revision:	1.1 - 02Jun93
 *              2.0 -  9Jul93
 *              3.0 - 13Jul94
 *
 * Refs:    a)  Military Standard--Protocol Data Units for Entity
 *		Information and Entity Interaction in a Distributed
 *		Interactive Simulation (DIS) (1.0) (30Oct91)
 *
 *	    b)	Proposed IEEE Standard Draft, Standard for Information
 *		Technology - Protocols for Distributed Interactive
 *		Simulation Applications, Version 2.0, Second Draft (22Mar93)
 *
 *	    c)	Proposed IEEE Standard Draft, Standard for Information
 *		Technology - Protocols for Distributed Interactive
 *		Simulation Applications, Version 2.0, Third Draft (28May93)
 *
 *	    d)  Review Draft, Interface Requirements Specification for the
 *		War Breaker Distributed Simulation, DIS 2.03 (May93)
 *
 * Author:	John Locke
 *              Mike Macedonia
 *              Steve Zeswitz
 *		CS Department, Naval Postgraduate School
 * Internet:	jxxl@taurus.cs.nps.navy.mil
 
 * 930709 : revised Entity State, Fire, and Detonation PDUs to comply
            with reference c.  All references to DIS 2.0 include changes
	    incorporated in the third draft, reference c.
 * 930710 : ROOT (AKA Dave Pratt added the 2.0.3 xyz and hpr to psi,theta, phi.
            this way we might have some idea on wha they mean
 *
 * 940713 : converted linked list features to fixed length arrays
 */

/* Define the DIS version to be used */

#undef  DIS_1_0
#define DIS_2_0		

/* PDU Header **************************************************************/

#define MAX_ARTICULAT_PARAMS	3
#define MAX_SUPPLY_QTY		3
#define MAX_RADAR_SYS		3
#define MAX_STORES		3
#define MAX_EMITTERS		3
#define MAX_ILLUMINED_ENTITY	3


/* PDU Header **************************************************************/

/* Versions */
#define DISVersionOct91		1
#define DISVersionIEEE1278_93	2
#define DISVersionMay93		3

typedef unsigned char	PDUType;

/* Official PDU Types: Required */
#define OtherPDU_Type			0
#define EntityStatePDU_Type		1
#define FirePDU_Type			2
#define DetonationPDU_Type		3
#define CollisionPDU_Type		4
#define ServiceRequestPDU_Type		5
#define ResupplyOfferPDU_Type		6
#define ResupplyReceivedPDU_Type	7
#define ResupplyCancelPDU_Type		8
#define RepairCompletePDU_Type		9
#define RepairResponsePDU_Type		10

#ifdef DIS_2_0
/* Official PDU Types: Required */
#define CreateEntityPDU_Type		11
#define RemoveEntityPDU_Type		12
#define StartResumePDU_Type		13
#define StopFreezePDU_Type		14
#define AcknowledgePDU_Type		15
#define ActionRequestPDU_Type		16
#define ActionResponsePDU_Type		17
#define DataQueryPDU_Type		18
#define SetDataPDU_Type			19
#define DataPDU_Type			20
#define EventReportPDU_Type		21
#define MessagePDU_Type			22
#define EmissionPDU_Type		23
#define LaserPDU_Type			24
#define TransmitterPDU_Type		25
#define SignalPDU_Type			26
#define ReceiverPDU_Type		27

#define MAX_NUM_PDUS                    32
#endif /* DIS_2_0 */

#ifdef DIS_1_0
/* Experimental PDU Types */
#define RadarPDU_Type			128
#define UpdateThreshRequestPDU_Type	129
#define UpdateThreshResponsePDU_Type	130
#define ActivatRequestPDU_Type		131
#define ActivatResponsePDU_Type		132
#define DeactivatRequestPDU_Type	133
#define DeactivatResponsePDU_Type	134
#define EmitterPDU_Type			135

#define MAX_NUM_PDUS                     10
#endif /* DIS_1_0 */

/* defined header the same as NRAD 10 Jun */

typedef struct {
   unsigned char	protocol_version;
   unsigned char	exercise_ident;		/* non-zero */
   PDUType		type;
   char			padding_8;		/* unused */
#ifdef DIS_2_0
   unsigned int	        time_stamp;	
   unsigned short	length;		        /* length of PDU in 32-bit words */
   short                padding_16; 
#endif
} PDUHeader;

/* Entity State PDU ********************************************************/

typedef struct {
   unsigned short	site;
   unsigned short	host;
} SimulationAddress;

typedef struct {
   SimulationAddress	address;
   unsigned short	entity;
} EntityID;

typedef unsigned char	ForceID;
#define ForceID_Other		0
#define ForceID_Blue		1		/* US, NATA, SEATO, etc. */
#define ForceID_Red		2		/* USSR, Warsaw Pact, etc. */
#define ForceID_White		3		/* Neutral */

#ifdef DIS_2_0
/* New World Order Mix */
#define ForceID_Friendly	1
#define ForceID_Opposing	2
#define ForceID_Neutral		3
#endif /* DIS_2_0 */

typedef unsigned char	EntityKind;
#define EntityKind_Other		0
#define EntityKind_Platform		1
#define EntityKind_Munition		2
#define EntityKind_LifeForm		3
#define EntityKind_Environmental	4
#define EntityKind_Cultural		5
#ifdef DIS_2_0
#define EntityKind_Supply		6
#define EntityKind_Radio		7
#endif

typedef unsigned char	Domain;
#define Domain_Other			0
#define Domain_Land			1
#define Domain_Air			2
#define Domain_Surface			3
#define Domain_Subsurface		4
#define Domain_Space			5

#define TargetDomain_Other		0
#define TargetDomain_AntiAir		1
#define TargetDomain_AntiArmor		2
#define TargetDomain_AntiGuided		3
#define TargetDomain_AntiRadar		4
#define TargetDomain_AntiSatellite	5
#define TargetDomain_AntiShip		6
#define TargetDomain_AntiSubmarine	7
#define TargetDomain_AntiPersonnel      8
#define TargetDomain_BattlefieldSupp	9
#define TargetDomain_Strategic		10
#define TargetDomain_Misc		11

typedef unsigned short	Country;
/* countries #define'd in country.h */

typedef unsigned char	Category;
/* categories #define'd in category.h */

typedef unsigned char	Subcategory;
typedef unsigned char	Specific;
typedef unsigned char	Extra;

typedef struct {
   EntityKind		entity_kind;
   Domain		domain;
   Country		country;
   Category		category;
   Subcategory		subcategory;
   Specific		specific;
   Extra		extra;
} EntityType;

typedef struct {
   double		x;
   double		y;
   double		z;
} EntityLocation;

typedef struct {
   float		x;
   float		y;
   float		z;
} VelocityVector;

#ifdef DIS_1_0
typedef int		BAM;		/* Binary Angle Measurement */
#define BAM_UNIT	8.3819e-8	/* (360 / 2**32) degrees per BAM
					   unit */
#endif

typedef struct {
#  ifdef DIS_1_0
   BAM			psi;
   BAM			theta;
   BAM			phi;
#  endif
#  ifdef DIS_2_0
   float		psi;   /*Z axis Performer hpr[HEADING]*/
   float		theta; /*Y axis Performer hpr[PITCH]*/
   float		phi;   /*X axis Performer hpr[ROLL] */
   /* as a public service: conversion between the two coordinate systems used 
      in the TRG code is as follows:
       psi = -hpr[HEADING];
       theta = hpr[PITCH];
       phi = hpr[ROLL];
   */
#  endif
} EntityOrientation;

typedef unsigned char	DeadReckAlgorithm;
#define DRAlgo_Other	0
#define DRAlgo_Static	1			/* entity does not move */
#define DRAlgo_DRM_FPW	2
#define DRAlgo_DRM_RPW	3
#define DRAlgo_DRM_RVW	4
#define DRAlgo_DRM_FVW	5
#define DRAlgo_DRM_FPB	6
#define DRAlgo_DRM_RPB	7
#define DRAlgo_DRM_RVB	8
#define DRAlgo_DRM_FVB	9

typedef struct {
   DeadReckAlgorithm	algorithm;
   char			TBD[15];		/* To Be Determined */
   float		linear_accel[3];
   int			angular_velocity[3];
} DeadReckonParams;

typedef unsigned char	CharacterSet;
#define CharSet_Unused	0
#define CharSet_ASCII	1

#define MARKINGS_LEN	11
typedef struct {
   CharacterSet		character_set;
   unsigned char	markings[MARKINGS_LEN];
} EntityMarking;

#define Capability_AmmunitionSupply	01
#define Capability_FuelSupply		02
#define Capability_MiscSupply		04
#define Capability_Repair		010

typedef struct {
   unsigned short	change;
   unsigned short	ID;
   unsigned int		parameter_type;
   unsigned char	parameter_value[8];
} ArticulatParams;

typedef struct {
#ifdef DIS_1_0
   PDUHeader		entity_state_header;
   EntityID		entity_id;
   char			padding_1_;		/* unused */
   unsigned char	force_id;
   EntityType		entity_type;
   EntityType		alt_entity_type;	/* guise */
   unsigned int		time_stamp;
   EntityLocation	entity_location;
   VelocityVector	entity_velocity;
   EntityOrientation	entity_orientation;
   DeadReckonParams	dead_reckon_params;
   unsigned int		entity_appearance;
   EntityMarking	entity_marking;
   unsigned int		capabilities;		/* boolean fields */
   char			padding_2_[3];		/* unused */
   unsigned char	num_articulat_params;	/* num of ArticulatParams */
   int			padding_3_;		/* local kludge */
   ArticulatParams	articulat_params[MAX_ARTICULAT_PARAMS];
#endif
#ifdef DIS_2_0
   PDUHeader		entity_state_header;
   EntityID		entity_id;
   ForceID		force_id;	      
   unsigned char	num_articulat_params;   /* number of ArticulatParams */
   EntityType		entity_type;
   EntityType		alt_entity_type;	/* guise */
   VelocityVector	entity_velocity;
   EntityLocation	entity_location;
   EntityOrientation	entity_orientation;
   unsigned int		entity_appearance;
   DeadReckonParams	dead_reckon_params;
   EntityMarking	entity_marking;
   unsigned int		capabilities;		/* boolean fields */
   ArticulatParams	articulat_params[MAX_ARTICULAT_PARAMS];
#endif
} EntityStatePDU;

#ifdef DIS_2_0
#define EntityStatePDUBaseLength  144           /* base length in bytes */
#endif

/* Fire PDU ****************************************************************/

typedef struct {
   SimulationAddress	address;
   unsigned short	event;
} EventID;

typedef unsigned short	Warhead;
#define WarheadMunition_Other			0000
#define WarheadMunition_HighExp			1000
#define WarheadMunition_HighExpPlastic		1100
#define WarheadMunition_HighExpIncendiary	1200
#define WarheadMunition_HighExpFragment		1300
#define WarheadMunition_HighExpAntiTank		1400
#define WarheadMunition_HighExpBomblets		1500
#define WarheadMunition_HighExpShapedCharg	1600
#define WarheadMunition_Smoke			2000
#define WarheadMunition_Illumination		3000
#define WarheadMunition_Practice		4000
#define WarheadMunition_Kinetic			5000
#define WarheadMunition_Unused			6000
#define WarheadMunition_Nuclear			7000
#define WarheadMunition_ChemGeneral		8000
#define WarheadMunition_ChemBlister		8100
#define WarheadMunition_ChemBlood		8200
#define WarheadMunition_ChemNerve		8300
#define WarheadMunition_BiologicalGeneral	9000

typedef unsigned short	Fuze;
#define FuzeMunition_Other		0000
#define FuzeMunition_Contact		1000
#define FuzeMunition_ContactInstant	1100
#define FuzeMunition_ContactDelayed	1200
#define FuzeMunition_Timed		2000
#define FuzeMunition_Proximity		3000
#define FuzeMunition_Command		4000
#define FuzeMunition_Altitude		5000
#define FuzeMunition_Depth		6000
#define FuzeMunition_Acoustic		7000

typedef struct {
   EntityType		munition;
   Warhead		warhead;
   Fuze			fuze;
   unsigned short	quantity;
   unsigned short	rate;
} BurstDescriptor;

typedef struct {
   PDUHeader		fire_header;
   EntityID		firing_entity_id;
   EntityID		target_entity_id;
   EntityID		munition_id;
   EventID		event_id;
#ifdef DIS_1_0
   unsigned int		time_stamp;
   unsigned int         padding_8;
#endif
#ifdef DIS_2_0
   unsigned int         padding_32;
#endif
   EntityLocation	location_in_world;
   BurstDescriptor	burst_descriptor;
   VelocityVector	velocity;
   float		range;
} FirePDU;

/* Detonation PDU **********************************************************/

typedef struct {
   float		x;
   float		y;
   float		z;
} EntityRelativeLoc;

typedef unsigned char	DetonationResult;
#ifdef DIS_1_0
#define DetResult_Detonation	0
#define DetResult_Impact	1
#define DetResult_None		2
#endif
#ifdef DIS_2_0
#define DetResult_Other		0
#define DetResult_EntityImpact	1
#define DetResult_EntityProxDet	2
#define DetResult_GroundImpact	3
#define DetResult_GroundProxDet	4
#define DetResult_Detonation	5
#define DetResult_None		6
#endif

typedef struct {
#ifdef DIS_1_0
   PDUHeader		detonation_header;
   EntityID		firing_entity_id;
   EntityID		target_entity_id;
   EntityID		munition_id;
   EventID		event_id;
   unsigned int		time_stamp;
   EntityLocation	location_in_world;
   BurstDescriptor	burst_descriptor;
   VelocityVector	velocity;
   EntityRelativeLoc	location_in_entity;
   DetonationResult	detonation_result;
   unsigned char	num_articulat_params;
   short		padding_16;
   ArticulatParams	articulat_params[MAX_ARTICULAT_PARAMS];
#endif
#ifdef DIS_2_0
   PDUHeader		detonation_header;
   EntityID		firing_entity_id;
   EntityID		target_entity_id;
   EntityID		munition_id;
   EventID		event_id;
   VelocityVector	velocity;
   EntityLocation	location_in_world;
   BurstDescriptor	burst_descriptor;
   EntityRelativeLoc	location_in_entity;
   DetonationResult	detonation_result;
   unsigned char	num_articulat_params;
   short		padding_16;
   ArticulatParams	articulat_params[MAX_ARTICULAT_PARAMS];
#endif
} DetonationPDU;
#ifdef DIS_2_0
#define DetonationPDUBaseLength  104           /* base length in bytes */
#endif

/* Service Request PDU *****************************************************/

typedef unsigned char	ServiceType;
#define ServType_Other		0
#define ServType_Resupply	1
#define ServType_Repair		2

typedef struct {
   EntityType		supply_type;
   float		quantity;
} SupplyQuantity;

typedef struct {
   PDUHeader		service_request_header;
#ifdef DIS_1_0
   EntityID		requesting_entity;
   EntityID		servicing_entity;
#endif
#ifdef DIS_2_0
   EntityID		receiving_entity;
   EntityID		supplying_entity;
#endif
   ServiceType		service_type;
   unsigned char	num_supply_types;
   short		padding;
   SupplyQuantity	supply_qty[MAX_SUPPLY_QTY];
} ServiceRequestPDU;

/* Resupply Offer/Received PDU *********************************************/

typedef struct {
   PDUHeader		resupply_header;
#ifdef DIS_1_0
   EntityID		requesting_entity;
   EntityID		servicing_entity;
#endif
#ifdef DIS_2_0
   EntityID		receiving_entity;
   EntityID		supplying_entity;
#endif
   unsigned char	num_supply_types;
   char			padding[3];
   SupplyQuantity	supply_qty[MAX_SUPPLY_QTY];
} ResupplyPDU;

/* Resupply Cancel PDU *****************************************************/

typedef struct {
   PDUHeader		resupply_cancel_header;
#ifdef DIS_1_0
   EntityID		requesting_entity;
   EntityID		servicing_entity;
#endif
#ifdef DIS_2_0
   EntityID		receiving_entity;
   EntityID		supplying_entity;
#endif
} ResupplyCancelPDU;

/* Repair Complete PDU *****************************************************/

typedef unsigned short	RepairType;
#define RepairType_NoRepair	0
#define RepairType_AllRepair	1

typedef struct {
   PDUHeader		repair_complete_header;
#ifdef DIS_1_0
   EntityID		requesting_entity;
   EntityID		servicing_entity;
#endif
#ifdef DIS_2_0
   EntityID		receiving_entity;
   EntityID		repairing_entity;
#endif
   RepairType		repair_type;
   short		padding;
} RepairCompletePDU;

/* Repair Response PDU *****************************************************/

typedef unsigned char	RepairResult;
#define RepResult_Other		0
#define RepResult_Ended		1
#define RepResult_Invalid	2
#define RepResult_Interrupted	3
#define RepResult_Cancelled	4

typedef struct {
   PDUHeader		repair_response_header;
#ifdef DIS_1_0
   EntityID		requesting_entity;
   EntityID		servicing_entity;
#endif
#ifdef DIS_2_0
   EntityID		receiving_entity;
   EntityID		repairing_entity;
#endif
   RepairResult		repair_result;
   char			padding[3];
} RepairResponsePDU;

/* Collision PDU ***********************************************************/

typedef struct {
   PDUHeader		collision_header;
   EntityID		issuing_entity;
   EntityID		colliding_entity;
   EventID		event_id;
   short		padding;
   unsigned int		time_stamp;
   VelocityVector	entity_velocity;
   float		mass;
   EntityRelativeLoc	location_to_entity;
} CollisionPDU;

/* *************************************************************************/
/* *************************************************************************/

#ifdef DIS_2_0		/* PDUs 11 - 27 */

	/* TO BE COMPLETED */

typedef struct {
   PDUHeader		header;
} CreateEntityPDU;

typedef struct {
   PDUHeader		header;
} RemoveEntityPDU;

typedef struct {
   PDUHeader		header;
} StartResumePDU;

#define StopReason_Other	0
#define StopReason_Recess	1
#define StopReason_Termination	2
#define StopReason_SysFailure	3
#define StopReason_SecurityViol	4
#define StopReason_EntityRecons	5

typedef struct {
   PDUHeader		header;
} StopFreezePDU;

#define AckFlag_CreateEntity	1
#define AckFlag_RemoveEntity	2
#define AckFlag_StartResume	3
#define AckFlag_StopFreeze	4

typedef struct {
   PDUHeader		header;
} AcknowledgePDU;

#define ActionID_Other		0
/* INCOMPLETE */

typedef struct {
   PDUHeader		header;
} ActionRequestPDU;

#define ActionReqStat_Other	0
#define ActionReqStat_Pending	1
#define ActionReqStat_Executing	2
#define ActionReqStat_Partial	3
#define ActionReqStat_Complete	4

typedef struct {
   PDUHeader		header;
} ActionResponsePDU;

typedef struct {
   PDUHeader		header;
} DataQueryPDU;

typedef struct {
   PDUHeader		header;
} SetDataPDU;

typedef struct {
   PDUHeader		header;
} DataPDU;

typedef struct {
   PDUHeader		header;
} EventReportPDU;

typedef struct {
   PDUHeader		header;
} MessagePDU;

typedef struct {
   PDUHeader		header;
} EmissionPDU;

typedef struct {
   PDUHeader		header;
} LaserPDU;

typedef struct {
   PDUHeader		header;
} TransmitterPDU;

typedef struct {
   PDUHeader		header;
} SignalPDU;

typedef struct {
   PDUHeader		header;
} ReceiverPDU;

#endif	/* DIS_2_0 */	/* PDUs 11 - 27 */

/* *************************************************************************/
/* *************************************************************************/

#ifdef DIS_1_0		/* Experimental PDUs */

/* Activate Request PDU ****************************************************/

typedef struct {
   unsigned short	site;
   unsigned short	host;
} HostID;

typedef unsigned char	ActivatReason;
#define ActivReason_Other	0
#define ActivReason_Start	1
#define ActivReason_Restart	2
#define ActivReason_Entry	3
#define ActivReason_Recons	4

typedef struct {
   char			description[11];
   unsigned char	version;
} TerrainDatabaseID;

typedef unsigned char	ServiceID;
#define ServiceID_Other		0
#define ServiceID_Army		1
#define ServiceID_AirForce	2
#define ServiceID_CoastGuard	3
#define ServiceID_Marines	4
#define ServiceID_Navy		5

typedef struct {
   Country		country;
   ForceID		force_id;
   ServiceID		service_id;
   unsigned char	hierarchy[8];
} Unit;

typedef struct {
   float		x;
   float		y;
   float		z;
   BAM			x1;
   BAM			y1;
   BAM			z1;
} UpdateThresholdVals;

typedef struct {
   unsigned int			radar_system;
} Radar;

typedef struct {
   float		quantity;
   EntityType		stores_entity_type;
} Stores;

typedef struct {
   PDUHeader		activat_request_header;
   HostID		activat_host;
   short		padding_1_;
   EntityID		new_entity;
   unsigned int		date_time;
   ActivatReason	activat_reason;
   char			padding_2_[3];
   TerrainDatabaseID	terrain_id;
   EntityType		entity_type;
   EntityType		alt_entity_type;	/* guise */
   Unit			unit;
   EntityMarking	entity_marking;
   unsigned int		capabilities;		/* boolean fields */
   int			subsystem_params[2];	/* 64-bits, undefined type */
   EntityLocation	entity_location;
   EntityOrientation	entity_orientation;
   UpdateThresholdVals	update_threshold;
   unsigned char	num_radar_systems;
   unsigned char	num_stores;
   unsigned char	num_articulat_params;	/* num of ArticulatParams */
   char			padding_3_;
   int			padding_4_;		/* local kludge */
   Radar		radar[MAX_RADAR_SYS];
   Stores		stores[MAX_STORES];
   ArticulatParams	articulat_params[MAX_ARTICULAT_PARAMS];
} ActivatRequestPDU;

/* Activate Response PDU ***************************************************/

typedef unsigned char	ActivatResult;
#define ActivResult_Other		0
#define ActivResult_ReqAccepted		1
#define ActivResult_InvalidParam	2
#define ActivResult_UnexpectedParam	3

typedef struct {
   PDUHeader		activat_response_header;
   EntityID		req_rec_entity;
   unsigned short	time_limit;
   ActivatResult	activat_result;
   char			padding[3];
} ActivatResponsePDU;

/* Deactivate Request PDU **************************************************/

typedef unsigned char	DeactivatReason;
#define DeactivReason_Other	0
#define DeactivReason_End	1
#define DeactivReason_Withdrawn	2
#define DeactivReason_Destroyed	3

typedef struct {
   PDUHeader		deactivat_request_header;
   EntityID		deactivat_entity;
   DeactivatReason	deactivat_reason;
   char			padding;
} DeactivatRequestPDU;

/* Deactivate Response PDU *************************************************/

typedef unsigned char	DeactivatResult;
#define DeactivResult_Other		0
#define DeactivResult_ReqAccepted	1
#define DeactivResult_InvalidParam	2
#define DeactivResult_UnexpectedReason	3
#define DeactivResult_NotActive		4

typedef struct {
   PDUHeader		deactivat_response_header;
   EntityID		deactivat_entity;
   DeactivatResult	deactivat_result;
   char			padding;
} DeactivatResponsePDU;

/* Emitter PDU *************************************************************/

typedef unsigned char	EmitterClass;
#define EmitterClass_Other	0
#define EmitterClass_Sound	1
#define EmitterClass_Infrasonic	2
#define EmitterClass_VLFreq	3
#define EmitterClass_LFreq	4
#define EmitterClass_MFreq	5
#define EmitterClass_HFreq	6
#define EmitterClass_VHFreq	7
#define EmitterClass_UHFreq	8
#define EmitterClass_SHFreq	9
#define EmitterClass_EHFreq	10
#define EmitterClass_Infrared	11
#define EmitterClass_Visible	12
#define EmitterClass_UV		13
#define EmitterClass_XRay	14
#define EmitterClass_GammaRay	15
#define EmitterClass_CosmicRay	16

typedef struct {
   EmitterClass		emitter_class;
   unsigned char	mode_num;
   unsigned short	dbase_entry_num;
} EmitterType;

typedef struct {
   unsigned short	power;
   short		param_1;		/* 16-bit floats ?! */
   short		param_2;
   short		param_3;
} EmitterParams;

typedef struct {				/* directional emitters only */
   BAM			azimuth_center;
   BAM			azimuth_width;
   BAM			elevation_center;
   BAM			elevation_width;
} SweepParams;

typedef struct {
   EmitterType		emitter_type;
   EmitterParams	emitter_params;
   SweepParams		emitter_sweep;
} Emitter;

#define EmitterDBNum_Other	0
#define EmitterDBNum_UTSS	1

typedef struct {
   PDUHeader            emitter_header;
   EntityID		emitting_entity;
   short		padding;
   unsigned int		entity_type;
   unsigned int		time_stamp;
   EntityLocation	entity_location;
   VelocityVector	entity_velocity;
   EntityOrientation	entity_orientation;
   DeadReckonParams	dead_reckon_params;
   unsigned short	num_emitters;
   unsigned short	database_num;
   Emitter		emitters[MAX_EMITTERS];
} EmitterPDU;

/* Radar PDU ***************************************************************/

typedef unsigned char	RadarMode;
#define RadarMode_Off			0
#define RadarMode_Search		1
#define RadarMode_DopplerHPRF		2
#define RadarMode_DopplerMPRF		3
#define RadarMode_DopplerLPRF		4
#define RadarMode_Monopulse		5
#define RadarMode_Acquisition		6
#define RadarMode_Tracking		7
#define RadarMode_TrackWhileScan	8
#define RadarMode_TerrainFollow		9
#define RadarMode_DataLink		10

typedef struct {
   EntityID		target_id;
   short		padding;
   unsigned int		radar_data;
} IlluminedEntity;

#define RadarSysCat_Reserved		0x0
#define RadarSysCat_AirFireCntrl	0x1
#define RadarSysCat_AirSearch		0x2
#define RadarSysCat_GroundFireCntrl	0x3
#define RadarSysCat_GroundSearch	0x4
#define RadarSysCat_SeaFireCntrl	0x5
#define RadarSysCat_SeaSearch		0x6

typedef struct {
   EntityRelativeLoc	location_to_entity;
   unsigned int		radar_system;
   short		power;
   RadarMode		radar_mode;
   unsigned char	num_illumined;
   char			specific_data[8];	/* unused */
   SweepParams		radar_sweep;
   IlluminedEntity	illumined_entity[MAX_ILLUMINED_ENTITY];
} RadarSystem;

typedef struct {
   PDUHeader            radar_header;
   EntityID		emitting_entity;
   EventID		event_id;
   unsigned int		time_stamp;
   char			padding[3];
   unsigned char	num_radar_systems;
   RadarSystem		radar_systems[MAX_RADAR_SYS];
} RadarPDU;

#endif	/* DIS_1_0 */

/* PDU union ***************************************************************/

#ifdef DIS_2_0
typedef union PDU {
   EntityStatePDU	ESpdu;
   FirePDU		Fpdu;
   DetonationPDU	Dpdu;
   CollisionPDU		Cpdu;
   ServiceRequestPDU	SRpdu;
   ResupplyPDU		Rpdu;
   ResupplyCancelPDU	RCpdu;
   RepairCompletePDU	RC_pdu;
   RepairResponsePDU	RRpdu;
} PDU;
#endif /* DIS_2_0 */

/* EOF */
