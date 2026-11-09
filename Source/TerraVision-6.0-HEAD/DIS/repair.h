/* File:	repair.h
 * Description:	DIS Repair Type Subsystems
 * Revision:	1.0 - 10Jun92
 *
 * Reference:	Military Standard--Protocol Data Units for Entity
 *		Information and Entity Interaction in a Distributed
 *		Interactive Simulation (DIS) (30Oct91)
 *		Appendix C
 *
 * Author:	John Locke
 *		CS Department, Naval Postgraduate School
 * Internet:	jxxl@taurus.cs.nps.navy.mil
 */


#define RepairType_Propulsion_Motor		10
#define RepairType_Propulsion_Starter		20
#define RepairType_Propulsion_Alternator	30
#define RepairType_Propulsion_Generator		40
#define RepairType_Propulsion_Battery		50
#define RepairType_Propulsion_EngCoolantLeak	60
#define RepairType_Propulsion_FuelFilter	70
#define RepairType_Propulsion_TransOilLeak	80
#define RepairType_Propulsion_EngineOilLeak	90
#define RepairType_Propulsion_Pumps		100
#define RepairType_Propulsion_Filters		110

#define RepairType_Hull				1000
#define RepairType_Airframe			1010

#define RepairType_InterfEnviron_Propeller	1500
#define RepairType_InterfEnviron_Transmission	1510
#define RepairType_InterfEnviron_Filters	1520
#define RepairType_InterfEnviron_Brakes		1530

#define RepairType_WeaponSys_GunElevation	2000
#define RepairType_WeaponSys_GunStabilization	2010
#define RepairType_WeaponSys_GPS		2020
#define RepairType_WeaponSys_GPS_Extension	2030
#define RepairType_WeaponSys_LoadingMechanism	2040

#define RepairType_FuelSys_FuelTransferPump	4000
#define RepairType_FuelSys_FuelLines		4010
#define RepairType_FuelSys_Guages		4020

#define RepairType_ElecSys_ElectronicCtrMeas	4500
#define RepairType_ElecSys_ElectronicWarfareSys	4600
#define RepairType_ElecSys_LaserRangeFinder	4700
#define RepairType_ElecSys_Radios		4800
#define RepairType_ElecSys_Intercoms		4900
#define RepairType_ElecSys_Coders		5000
#define RepairType_ElecSys_Decoders		5100
#define RepairType_ElecSys_Lasers		5200
#define RepairType_ElecSys_Computers		5300
#define RepairType_ElecSys_Emitters		5400
#define RepairType_ElecSys_DetectionSys		5600

#define RepairType_LifeSuppSys_AirSupply	8000
#define RepairType_LifeSuppSys_Filters		8010
#define RepairType_LifeSuppSys_WaterSupply	8020
#define RepairType_LifeSuppSys_RefrigSys	8030
#define RepairType_LifeSuppSys_ContamProtect	8040
#define RepairType_LifeSuppSys_WashDownSys	8050
#define RepairType_LifeSuppSys_DecontamSys	8060

#define RepairType_HydraulSys_WaterSupply	9000
#define RepairType_HydraulSys_CoolingSys	9010
#define RepairType_HydraulSys_Winches		9020
#define RepairType_HydraulSys_Catapults		9030
#define RepairType_HydraulSys_Cranes		9040
#define RepairType_HydraulSys_Launchers		9050

#define RepairType_AuxCraft_LifeBoats		10000
#define RepairType_AuxCraft_LandingCraft	10010
#define RepairType_AuxCraft_EjectionSeats	10020

/* EOF */
