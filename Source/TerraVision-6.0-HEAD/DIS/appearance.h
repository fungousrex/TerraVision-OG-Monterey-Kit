/* File:	appearance.h
 * Description:	DIS Entity State AppearanceLands
 * Revision:	1.0 - 11Jun92
 *
 * Reference:	Military Standard--Protocol Data Units for Entity
 *		Information and Entity Interaction in a Distributed
 *		Interactive Simulation (DIS) (30Oct91)
 *		Appendix D
 *
 * Author:	John Locke
 *		CS Department, Naval Postgraduate School
 * Internet:	jxxl@taurus.cs.nps.navy.mil
 */


/* Land Vehicle Appearances */
#define AppearanceLand_Destroyed		01
#define AppearanceLand_SmokePlume		02
#define AppearanceLand_Flaming			04
#define AppearanceLand_SmallDustCloud		010
#define AppearanceLand_MediumDustCloud		020
#define AppearanceLand_LargeDustCloud		030
#define AppearanceLand_WinterPaint		040
#define AppearanceLand_DesertPaint		0140
#define AppearanceLand_Launcher			0200
#define AppearanceLand_EngineSmoke		0400
#define AppearanceLand_HatchClosed		01000
#define AppearanceLand_HatchOpen		02000
#define AppearanceLand_HatchOpenWithHead	03000

/* Air Vehicle Appearances */
#define AppearanceAir_Destroyed			01
#define AppearanceAir_Flaming			02
#define AppearanceAir_AfterBurner		04
#define AppearanceAir_RunningLights		010
#define AppearanceAir_NavigationLights		020
#define AppearanceAir_FormationLights		040

/* Surface Vehicle Appearances */
#define AppearanceSurf_Destroyed		01
#define AppearanceSurf_Flaming			02
#define AppearanceSurf_SmallWake		04
#define AppearanceSurf_MediumWake		010
#define AppearanceSurf_LargeWake		014
#define AppearanceSurf_RunningLights		040

/* Subsurface Vehicle Appearances */
#define AppearanceSubSurf_Destroyed		01
#define AppearanceSubSurf_SmallWake		02
#define AppearanceSubSurf_MediumWake		04
#define AppearanceSubSurf_LargeWake		06
#define AppearanceSubSurf_RunningLights		020
#define AppearanceSubSurf_HatchClosed		040
#define AppearanceSubSurf_HatchOpen		0100
#define AppearanceSubSurf_HatchOpenWithHead	0140

/* Space Vehicle Appearances */
#define AppearanceSpace_Destroyed		01

/* Guided Munition Appearances */
#define AppearanceMunition_LaunchFlash		01
#define AppearanceMunition_RocketFlame		02

/* Life Form Appearances */
#define AppearanceLifeForm_Upright		01
#define AppearanceLifeForm_Kneeling		02
#define AppearanceLifeForm_Prone		03
#define AppearanceLifeForm_Weapon1		020
#define AppearanceLifeForm_Weapon2		040

/* Environmental Appearances */
#define AppearanceEnviron_DensityHazy		01
#define AppearanceEnviron_DensityDense		02
#define AppearanceEnviron_DensityVeryDense	03
#define AppearanceEnviron_DensityOpaque		04

/* Cultural Feature Appearances */
#define AppearanceCultural_DamageSlight		01
#define AppearanceCultural_DamageModerate	02
#define AppearanceCultural_DamageDestroyed	03
#define AppearanceCultural_SmokePlume		010
#define AppearanceCultural_Flaming		020

/* EOF */
