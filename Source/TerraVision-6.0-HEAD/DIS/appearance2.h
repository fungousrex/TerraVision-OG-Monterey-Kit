/* File:	appearance2.h
 * Description:	DIS Entity State AppearanceLands
 * Revision:	2.0 - 12Jul93
 *
 * Refs:     a)	Proposed IEEE Standard Draft
 *		Standard for Information Technology -
 *		Protocols for Distributed Interactive Simulation 
 *              Applications (DIS Version 2.0, Second Draft) (22 Mar 93)
 *		(IST-CR-93-01)
 *
 *	     b) Enumeration and Bit Encoded Values for Use with Protocols
 *		for Distributed Interactive Simulation Applications
 *		(22 Mar 93)
 *		(IST-CR-93-02)
 *              
 *
 * Author:	Steve Zeswitz
 *		CS Department, Naval Postgraduate School
 * Internet:	zeswitz@taurus.cs.nps.navy.mil
 */


/* General Appearance */
#define Appearance_CamoPaint			01
#define Appearance_MobilityKill			02
#define Appearance_FirePowerKill		04
#define Appearance_DamageSlight			010
#define Appearance_DamageModerate		020
#define Appearance_DamageDestroyed		030
#define Appearance_SmokePlume			040
#define Appearance_SmokeEngine			0100
#define Appearance_SmokePlume_Engine		0140
#define Appearance_TrailingEffectsSmall		0200
#define Appearance_TrailingEffectsMedium	0400
#define Appearance_TrailingEffectsLarge		0600
#define Appearance_HatchClosed			01000
#define Appearance_HatchPopped			02000
#define Appearance_HatchPoppedWithHead		03000
#define Appearance_HatchOpen			04000
#define Appearance_HatchOpenWithHead		05000
#define Appearance_LightsRunning		010000
#define Appearance_LightsNavigation		020000
#define Appearance_LightsFormation		030000
#define Appearance_Flaming			0100000

/* Air Vehicle Appearances */
#define AppearanceAir_AfterBurner		0200000

/* Surface Vehicle Appearances    no specific bits defined */

/* Subsurface Vehicle Appearances no specific bits defined */

/* Space Vehicle Appearances      no specific bits defined */

/* Guided Munition Appearances */
#define AppearanceMunition_LaunchFlash		0200000

/* Life Form Appearances */
#define AppearanceLifeForm_Upright		0200000
#define AppearanceLifeForm_Kneeling		0400000
#define AppearanceLifeForm_Prone		0600000
#define AppearanceLifeForm_Weapon1Deployed	01000000
#define AppearanceLifeForm_Weapon2Deployed	02000000

/* Environmental Appearances */
#define AppearanceEnviron_DensityHazy		0200000
#define AppearanceEnviron_DensityDense		0400000
#define AppearanceEnviron_DensityVeryDense	0600000
#define AppearanceEnviron_DensityOpaque		01000000

/* Cultural Feature Appearances   no specific bits defined */

/* EOF */
