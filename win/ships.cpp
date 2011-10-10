#include <math.h>
#include "misc.h"
#include <string.h>

// number of 250-kg damages that one tonne of metal alloys will replace
#define ALLOY_MULT 16
#define HULL_REGEN_RATE 0.00000075
#define SHIELD_REGEN_RATE 0.002

long double HullDiff[2] = {0.0,0.0}, ShieldDiff[2] = {0.0,0.0};	// for storage
long double hullAccum = 0.0, shieldRechargeAccum[116] = {0.0};

// absorption goes from 85% (full shields) to 40% (no shields)
float c = -0.45, b = -0.40;

// hull absorption goes from 95% to 80%
float hc = -0.15, hb = -0.80;

// lasers take up 25%-50% of total space
float baseLaserPct = 0.25, extLaserPct = 0.25;

// Determines how quickly a ship can turn!  OH BOY!
float TurningMultipliers[] =
{
	1.0, 1.0, 1.0, 1.0,
	// Missiles 0x4-0xd
	1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
	// Escape Capsule and other worthless ships
	0.33, 0.33, 0.33, 0.33, 0.33,
	1.5,	// Osprey interceptor
	1.4,	// Falcon interceptor
	1.325,	// Hawk Airfighter
	1.25,	// Kestrel Airfighter
	1.4,	// Eagle long range fighter
	1.3,	// Eagle MKII
	1.25,	// Eagle MKIII
	1.0,	// Sidewinder
	0.92,	// Krait
	0.88,	// Gecko
	0.68,	// Adder
	1.3,	// Viper Defence Craft
	1.2,	// Saker MKIII
	1.1,	// Osprey 'X' Interceptor
	1.15,	// Merlin Interceptor
	1.2, 	// Viper MKII
	1.1,	// Gyr attack fighter
	0.70,	// Cobra MKI
	0.485,	// Moray Starboat
	0.665,	// Cobra MKIII
	0.60,	// Constrictor
	1.0,	// Asp Explorer
	0.81,	// Lanner
	1.05,	// Harris Fighter
	1.0,	// Spar Interceptor
	0.775,	// Wyvern Explorer
	0.65,	// Skeet Cruiser
	0.70,	// Turner Class
	0.75,	// Lanner II
	0.70,	// Harrier
	1.0, 1.0, 1.0, 1.0,		// Unknowns
	0.5,	// Transporter
	0.6,	// Lion Transport
	0.5,	// Tiger Trader
	0.58,	// Imp. Courier
	0.44,	// Python Trader
	0.46,	// Imp. Trader
	0.41,	// Anaconda Destroyer
	0.25,	// Puma Freighter
	0.31,	// Boa Trader
	0.35,	// Panther Clipper
	1.0,	// Unknown
	0.31,	// Tiercel Freighter
	0.328,	// Imp. Explorer
	0.25,	// INRA Command Ship
	0.36,	// Mantis Freighter
	0.21,	// Griffin Hauler
	1.0,	// Thargoid Transport
	0.5,	// Lynx Bulk Carrier
	0.5		// Long-range Cruiser
};

INT32 maxBounty = 12500;

// New shield value calculate function
// Handles integral of ds/dd = b + cs:
// d = [ln|cs+b| - ln|ci+b|]/c
// s = [e^(cd)*(ci+b) - b]/c (infinitely small interval)
extern "C" float CALC_NewShieldValue (float initial, float damage)
{
	
	// restrict integration to shield values above 0
	// (negative shield values do not exist and could skew)
	float MaxDamage = ((log(fabs(b)) - log(fabs((c*initial) + b))) / c);
	
	if (initial <= 0 || !_finite(MaxDamage))
		return 0; 

	if (damage > MaxDamage)
		damage = MaxDamage;

	return (((exp(c * damage) * ((c*initial) + b)) - b) / c);
}
 
extern "C" float CALC_NewHullValue (float initial, float damage)
{
	float MaxDamage = ((log(fabs(hb)) - log(fabs((hc*initial) + hb))) / hc);
	
	if (initial <= 0 || !_finite(MaxDamage))
		return 0; 

	if (damage > MaxDamage)
		damage = MaxDamage;

	return (((exp(hc * damage) * ((hc*initial) + hb)) - hb) / hc);
}

extern "C" INT32 GetVol(INT32 logVol)
{
	if (logVol < 4)
		return 0x7e00;

	logVol >>= 2;
	
	return (0x7e00 >> logVol);
}

// do damage.  
extern "C" INT32 DoShipDamage(ModelInstance_t *ship, INT32 damage, INT8 bContinuous)
{
	float temp;
	float oldShields;
	float newShields;
	float newHull, oldHull;
	float shieldAbsorb, hullAbsorb;
	float realDamage, equipAbsorb;
	long double damageProbability, baseProbability;
	INT16 shields, hull, totalShields, totalHull;
	INT8  idx, numDamages;
	ShipDef_t *ship_def;
	INT32 rand, iProbability, iResult = 0, vol;

	idx = ((ship == DATA_PlayerObject) ? 1 : 0);

	//if (idx) return 0; // god mode

	shields = ship->globalvars.shields;
	totalShields = ship->globalvars.max_shields;
	hull = ship->mass_x4;
	
	ship_def = FUNC_001538_GetModelPtr(ship->model_num)->Shipdef_ptr;
	totalHull = ship_def->Mass * 4;
	
	oldHull = hull + HullDiff[idx];
	oldShields = shields + ShieldDiff[idx];

	if ((totalShields > 0) && (oldShields > 0))
	{
		temp = c/b;
		realDamage = damage/(float)totalShields;

		newShields = CALC_NewShieldValue(oldShields / (float)totalShields, realDamage);

		// get the new shield strength!
		newShields *= totalShields;

		shieldAbsorb = oldShields - newShields;
	}
	else
	{
		shieldAbsorb = 0.0;
		newShields = 0.0;
	}

	if (newShields < 0)
	{
		hullAbsorb = newShields;
		newShields = 0;
	}
	else
		hullAbsorb = 0.0;

	hullAbsorb += (float)damage - shieldAbsorb;

	newHull = (float)totalHull * CALC_NewHullValue(oldHull/(float)totalHull, hullAbsorb/(float)totalHull);
	equipAbsorb = hullAbsorb + (newHull - oldHull);

	if (newHull < 0.0)
		newHull = 0.0;
	
	hull = (INT16)newHull;
	shields = (INT16)newShields;
	
	// store small differences to avoid rounding-error magnification.
	HullDiff[idx] = newHull - hull;
	ShieldDiff[idx] = newShields - shields;
	
	if (hull == 0)
		return 0;	// death

	realDamage = hullAbsorb + shieldAbsorb;
	hullAbsorb /= realDamage;
	shieldAbsorb /= realDamage;

	if (hullAbsorb > 0)
	{
		ship->smoke_timeout = 10;
		
		vol = idx ? 0x7e00 : GetVol(ship->dist_cam);
		FUNC_001908_SoundPlaySampleLinVol(0x1d, (INT32)(vol * hullAbsorb) | 0x80000000);
	}

	if (shieldAbsorb > 0)
	{
		ship->laser_flags |= 0x1;
		
		vol = idx ? 0x7e00 : GetVol(ship->dist_cam);
		FUNC_001908_SoundPlaySampleLinVol(0x1c, (INT32)(vol * shieldAbsorb) | 0x80000000);
	}

	// calculate probability of equipment damage here (1-FFFF).

	baseProbability = damageProbability = 1 - pow(0.6, 20.0*equipAbsorb / (float)totalHull);

	rand = DATA_RandomizerFunc();

	iProbability = (INT32)(damageProbability * 65536.0);

	numDamages = 0;
	while (rand < iProbability)
	{
		iResult = FUNC_000952_DestroyEquip(ship);
		
		// another chance...
		damageProbability *= baseProbability;
		iProbability = (INT32)(damageProbability * 65536.0);
		numDamages++;
	}
	
	// I don't know what this does, but it would be wise
	// to maintain the status quo :)
	if ((numDamages == 0) && (idx == 1))
	{
		FUNC_000148_Unknown(0x17, 0x0); // ???
		return 1;
	}
	else
		return iResult;
}

extern "C" INT32 GetInitialFuel(ModelInstance_t *ship, INT8 driveType)
{
	float fuelSpace;
	SINT32 rand, randSeed, randSeed2;

	// police have no fuel...
	if (ship->object_type== OBJTYPE_POLICE || driveType == 0x1)
		return 0;

	driveType = ship->globalvars.drive;
	randSeed = ship->globalvars.unique_Id;
	
	randSeed2 = randSeed = (randSeed << 16) | (randSeed >> 16);

	// pseudorandom effect, same all the time
	rand = DATA_FixedRandomFunc(0x10000, &randSeed, &randSeed2) & 0x7fff; 
	
	fuelSpace = DATA_DriveFuelConsumption[driveType];
	fuelSpace += sqrt(DATA_DriveFuelConsumption[driveType]*1.5*(rand/32768.0)) * 4;

	return fuelSpace;
}

extern "C" ModelInstance_t *AIChooseEquipment(ModelInstance_t *ship, INT32 shipType)
{
	INT16 spaceUsed, spaceAvail, hullMass, i;
	float usedWeight, allowedWeight;
	INT16 maxLasers, fuelSpace;
	INT8 driveType, numPylons;
	ShipDef_t *ship_def;
	float laserPct;
	INT32 laserID, oldSystem;
	
	spaceUsed = 0;
	if (shipType >= 0xa && DATA_RandomizerFunc() < 48000)
		oldSystem = DATA_CurrentSystem;
	else
	{
		oldSystem = FUNC_000857_GetNeighborSystem(DATA_CurrentSystem, BoundRandom(3));
		if (oldSystem == 0)
			oldSystem = DATA_CurrentSystem;
		else
		{
			INT8 *c1;
			INT32 population, danger, c4;
			
			FUNC_000870_GetSystemDataExt(oldSystem, &c1, &population, &danger, &c4);
			if (population < 3)
				oldSystem = DATA_CurrentSystem;
		}
	}

	ship->bounty = oldSystem;

	ship->globalvars.laser_front = 0;
	ship->globalvars.shields = ship->globalvars.max_shields = 0;

	ship_def = FUNC_001538_GetModelPtr(ship->model_num)->Shipdef_ptr;
	hullMass = ship_def->Mass * 4;
	spaceAvail = ship_def->Capacity;

	driveType = ship_def->IntegralDrive;
	if (driveType == 0x80)
		driveType = ship_def->Drive;
	
	// police use interplanetary drive for more space, so do station helpers
	if (shipType == 0xe)
	{
		driveType = 0x1;
		ship->object_type = OBJTYPE_POLICE;	// police marking
	}
	else if (shipType == 0xf)
		driveType = 0x1;

	ship->globalvars.drive = driveType;

	usedWeight = DATA_DriveMasses[driveType];
	spaceAvail -= usedWeight;
	spaceUsed = usedWeight;

	// allow space for fuel, but don't overdo it.
	fuelSpace = GetInitialFuel(ship, driveType);
	
	if (fuelSpace > spaceAvail)
	{
		spaceUsed += spaceAvail;
		spaceAvail = 0;
	}
	else
	{
		spaceAvail -= fuelSpace;
		spaceUsed += fuelSpace;
	}

	// Bulk Carriers and other helpers are all about the cargo
	if (shipType == 0xf)
	{
		ship->cargo_space = ship_def->Capacity - spaceUsed;
		return ship;
	}

	// for pirates, 10%-20% goes to cargo space... unoccupied
	// for traders, 20%-40% is occupied
	// for assassination targets, 10%-20% is occupied
	// for military ships, assassins, and police, no extra cargo
	if (shipType < 0x13 && shipType != 0xe)
	{
		usedWeight = 0.1 + 0.1*FloatRandom();
		usedWeight *= spaceAvail;

		if (shipType < 0xa)
			usedWeight *= 2;
		else if (shipType != 0x10)
			spaceUsed += usedWeight;

		spaceAvail -= usedWeight;
	}

	if (driveType == 0x1)
		fuelSpace = 0.0;

	ship->cargo_fuel = fuelSpace;
	spaceAvail -= fuelSpace;
	spaceUsed += fuelSpace;

	
	laserPct = baseLaserPct + extLaserPct * (DATA_RandomizerFunc() / 65536.0);
	
	allowedWeight = laserPct * spaceAvail;

	laserID = 0;
	usedWeight = 0;

	if (DATA_PlasmaMount[ship->model_num] == 0x0)
		maxLasers = 0x7;
	else
		maxLasers = 0x9;

	// no gun mounts?
	if (ship->globalvars.num_gunmountings != 0)
		for (i = 0; i < maxLasers; i++)
		{
			if (DATA_AILasers[i].weight > allowedWeight)
				break;

			laserID = DATA_AILasers[i].id;
			usedWeight = DATA_AILasers[i].weight;
		}

	ship->globalvars.laser_front = laserID;

	spaceUsed += usedWeight;
	spaceAvail -= usedWeight;

	ship->globalvars.equip &= ~(EQUIP_ECM | EQUIP_NAVAL_ECM);

	// install misc. equipment
	// allow up to 50% of the remaining space.
	allowedWeight = 0.707107 * (DATA_RandomizerFunc() / 65536.0);
	allowedWeight *= allowedWeight * spaceAvail;
	
	if (allowedWeight >= DATA_NECM_Weight)
	{
		ship->globalvars.equip |= EQUIP_NAVAL_ECM;
		usedWeight = DATA_NECM_Weight;
		allowedWeight -= DATA_NECM_Weight;
	}
	else if (allowedWeight >= DATA_ECM_Weight)
	{
		ship->globalvars.equip |= EQUIP_ECM;
		usedWeight = DATA_ECM_Weight;
		allowedWeight -= DATA_ECM_Weight;
	}
	else
		usedWeight = 0;

	// laser cooling booster
	if (allowedWeight >= 5)
	{
		ship->globalvars.equip |= EQUIP_LASER_COOLING_BOOSTER;
		usedWeight += 5;
		allowedWeight -= 5;
	}

	spaceAvail -= usedWeight;
	spaceUsed += usedWeight;

	// install shields using whatever's left...
	
	usedWeight = spaceAvail;

	usedWeight = ((INT32)usedWeight / 4) * 4; // 4x increments
	
	ship->globalvars.shields = ship->globalvars.max_shields = usedWeight * 16;
	spaceAvail -= usedWeight;
	spaceUsed += usedWeight;

	if (ship_def->Capacity < spaceUsed || shipType == 0xe)
		ship->cargo_space = 0;
	else
		ship->cargo_space = ship_def->Capacity - spaceUsed;

	// equip missiles.

	numPylons = ship_def->Missiles;
	for (i = 0; i < numPylons; i++)
	{
		ship->globalvars.missiles[i] = DATA_AIMissiles[DATA_RandomizerFunc() & 0xf];
	}

	return ship;
}

// returns the exchange rate of a ship (unit: $1000cr).
extern "C" INT32 GetShipWorth(ModelInstance_t *ship)
{
	SINT32 shipCost, i, equipAmt;
	INT8 driveType;
	INT16 hullMass, intactMass;
	
	ShipDef_t *ship_def;

	ship_def = FUNC_001538_GetModelPtr(ship->model_num)->Shipdef_ptr;
	shipCost = ship_def->Price * 900;	// used ship

	driveType = ship_def->Drive;
	
	for (i = 0; i < 64; i++)
	{
		if (DATA_EquipmentData[i].id == driveType)
			shipCost -= DATA_EquipmentData[i].buyCost * 9;
	}

	for (i = 0; i < 64; i++)
	{
		equipAmt = FUNC_000392_GetEquipmentAmount((SINT8*)ship, i);

		// get nothing for integrated drives, or unsellable items
		if (DATA_EquipmentData[i].techLevel < 0xff)
			shipCost += equipAmt * DATA_EquipmentData[i].sellCost * 9;
	}
	
// subtract any hull repair costs

	hullMass = ship_def->Mass * 4;
	intactMass = ship->mass_x4;

	shipCost -= (hullMass - intactMass) * 5;

//	shipCost /= 1000;
//	shipCost *= 1000;

	return shipCost;
}

// Messy function - my apologies
extern "C" void RegenerateHull()
{
	INT16 maxHull, hullGain, *metalAlloys, totalAlloys, oldAlloys;

	ShipDef_t *ship_def;
	ModelInstance_t *ship;
	SINT8 extraAlloys;

	ship = DATA_PlayerObject;

	// need metal supply to repair.
	metalAlloys = DATA_PlayerCargo+0xf;
	oldAlloys = *metalAlloys;

	// keep track with unused space
	extraAlloys = ship->cargo_space;

	totalAlloys = (*metalAlloys)*ALLOY_MULT + extraAlloys;

	if (totalAlloys == 0)
		return;

	ship_def = FUNC_001538_GetModelPtr(ship->model_num)->Shipdef_ptr;
	maxHull = ship_def->Mass * 4;
	
	// hull regeneration rate here
	hullAccum += DATA_FrameTime * HULL_REGEN_RATE * sqrt(maxHull / 6000.0);

	// transfer integer values.
	hullGain = (INT16)hullAccum;
	hullAccum -= hullGain;
	
	if (totalAlloys < hullGain)
	{
		hullGain = totalAlloys;
		totalAlloys = 0;
	}
	else
		totalAlloys -= hullGain;

	extraAlloys = totalAlloys % ALLOY_MULT;
	*metalAlloys = totalAlloys / ALLOY_MULT;

	DATA_PlayerCargoSpace -= (oldAlloys - *metalAlloys);

	ship->mass_x4 += hullGain;
	if (ship->mass_x4 > maxHull)
		ship->mass_x4 = maxHull;

	// set timer to display hull percentage!
	if (hullGain > 0)
		DATA_008886_Unknown = 0x1e;
}

extern "C" void RegenerateShields(ModelInstance_t *ship)
{
	INT16 shields, maxShields, shipIdx;
	INT32 shieldGain;
	float sfArea, radius, volume, realGain;
	ShipDef_t *ship_def;

	ship_def = FUNC_001538_GetModelPtr(ship->model_num)->Shipdef_ptr;

	shipIdx = ship->index;
	shields = ship->globalvars.shields;
	maxShields = ship->globalvars.max_shields;

	// get shield surface area
	// cargo space takes up 8x cubic units per tonne
	volume = 2 * ship_def->Mass + 16 * ship_def->Capacity;
	
	radius = pow((0.75/PI) * volume,1/3.0);
	radius++;	// shields spaced 1 unit from hull

	sfArea = 4*PI*radius*radius;

	realGain = (float)DATA_FrameTime * SHIELD_REGEN_RATE * (maxShields / sfArea);	

	if (ship->globalvars.equip & EQUIP_ENERGY_BOOSTER)
		realGain *= 1.65;

	shieldRechargeAccum[shipIdx] += realGain;

	shieldGain = shieldRechargeAccum[shipIdx];
	shieldRechargeAccum[shipIdx] -= shieldGain;
	
	if ((shields + shieldGain) > maxShields)
		shields = maxShields;
	else
		shields += shieldGain;
}

extern "C" INT32 AIGetMissileToFire(ModelInstance_t *ship)
{
	ModelInstance_t *target;
	INT8 targetIdx, mType;
	INT16 maxHull, maxCargo, i, n, pylon, neededCargo, neededHull;
	INT16 thrust;
	ShipDef_t *ship_def;

	float rMult;

	targetIdx = ship->dest_index;
	target = FUNC_001532_GetModelInstancePtr(targetIdx, DATA_ObjectArray);

	ship_def = FUNC_001538_GetModelPtr(ship->model_num)->Shipdef_ptr;
	maxCargo = ship_def->Capacity;
	maxHull = ship_def->Mass;
	thrust = ship_def->ForwardThrust;

	n = DATA_RandomizerFunc() & 0x7;
	rMult = DATA_RandomizerFunc() / 65536.0;
	maxCargo *= 0.5 + rMult;
	maxHull *= 1.5 - rMult;

	for (i = 0; i < 8; i++, n++)
	{
		pylon = (n & 0x7);
		mType = ship->globalvars.missiles[pylon];

		neededHull = 0;
		switch (mType)
		{
		case 0x82:	// homing missile
			neededCargo = 0;
			neededHull = 0;
			break;
		case 0x83:	// smart missile
			neededCargo = 50;
			neededHull = 100;
			break;
		case 0x84:	// naval missile
			neededCargo = 150;
			neededHull = 200;
			break;
		case 0x85:	// light torpedo
			neededCargo = 900;
			neededHull = 700;
			break;
		case 0x86:	// heavy torpedo
			neededCargo = 900;
			neededHull = 1350;
			break;
		default:
			neededCargo = 0xffff;
			neededHull = 0xffff;
		}

		if (neededCargo <= maxCargo && neededHull <= maxHull)
			break;
	}

	if (i >= 8)
		return 0;
	
	pylon++;
	return pylon;
}

extern "C" INT32 GetBounty(ModelInstance_t *ship)
{
	SINT32 rand, randSeed, randSeed2, cash, cashdigits, cashunit;
	float cashFactor;
	ShipDef_t *ship_def;
	
	if (ship->object_type != OBJTYPE_PIRATE)
		return 0;

	randSeed = ship->globalvars.unique_Id;
	
	randSeed2 = randSeed = (randSeed << 15) | (randSeed >> 17);

	// pseudorandom effect, same all the time
	rand = DATA_FixedRandomFunc(0x10000, &randSeed, &randSeed2) & 0x7fff; 

	if (rand & 0x1)
		return 0;

	ship_def = FUNC_001538_GetModelPtr(ship->model_num)->Shipdef_ptr;

	cashFactor = 7*exp(ship_def->Price / -2000.0);
	cashFactor = pow((rand&0x7fff)/32768.0, cashFactor);

	cash = cashFactor*maxBounty;

	cashdigits = (SINT8)(log(cash) / log(10)) - 2;

	cashunit = pow(10, cashdigits);

	if (cashunit < 10)
		cashunit = 10;

	cash = (cash / cashunit) * cashunit;
	
	return cash;
}

// spawns hostile AI ships.
extern "C" INT8 SpawnHostileGroup(INT8 ships, INT8 *shipArray, INT32 targetName, INT8 shipType, INT8 object_type)
{
	ModelInstance_t *shipObj;
	INT8 parentShipIdx, i;
//	SINT8 j;
	INT64 tempVec[3];

	if (shipType == 0x13)
		DATA_CustomShipIndex = shipArray[DATA_RandomizerFunc() & 0xf];

	// store grouping vector...
	for (i = 0; i < 3; i++)
		tempVec[i] = DATA_GroupingVector[i];

	// spawn the leader ship.
	shipObj = FUNC_000772_AIShipSpawn(shipType);
	
	if (shipObj == 0)
		return 0;

	shipObj->dest_index = DATA_PlayerIndex;
	FUNC_000702_Unknown(shipObj, 0x315000);
	shipObj->ai_mode = AI_PIRATE_PREPARE;
	shipObj->target_index = 0x0;
	shipObj->object_type = object_type;

	if (targetName != 0)
	{
		shipObj->bounty = targetName;
		shipObj->cargo_fuel |= 0x8000;
	}

	FUNC_000048_Unknown(0x17, 0x0, shipObj->index);

	parentShipIdx = shipObj->index;

	// spawn the follower ships.
	for (i = 1; i < ships; i++)
	{
		if (shipType == 0x13)
			DATA_CustomShipIndex = shipArray[DATA_RandomizerFunc() & 0xf];

		shipObj = FUNC_000772_AIShipSpawn(shipType+1);
		
		if (shipObj == 0)
			return i;

		shipObj->dest_index= parentShipIdx;
		shipObj->ai_mode = AI_FORMATION;
		shipObj->target_index = DATA_PlayerIndex;
		shipObj->thrust_power++;
		shipObj->target_off_x = BoundRandom(2000) - 1000;
		shipObj->target_off_z = BoundRandom(2000) - 1000;
		shipObj->target_off_z = 0;
		shipObj->object_type = object_type;

		if (targetName != 0)
		{
			shipObj->bounty = targetName;
			shipObj->cargo_fuel |= 0x8000;
		}

		

/*		if (i & 0x1)
			for (j = (i & 0xfe); j >= 0; j--)
				FUNC_001662_Vec64Sub((INT64*)(shipObj+0x3e), tempVec);
		else
			for (j = (i & 0xfe); j > 0; j--)
				FUNC_001661_Vec64Add((INT64*)(shipObj+0x3e), tempVec); */

		FUNC_000048_Unknown(0x17, 0x0, shipObj->index);
	}

	return ships;
}

extern "C" INT32 GetTurningRate(SINT32 baseRate, INT32 model)
{
	return baseRate*TurningMultipliers[model];
}

extern "C" void CreateShipyardData(starport_t *starport)
{
	INT32 numShips, i, *shipyardArray;

	starport->numShips = 0;
	shipyardArray = starport->shipyard;

	if (DATA_NumStarports > 1)
	{

		numShips = (DATA_NumStarports + 3) / 4;
		for (i = 0; i < numShips; i++)
			FUNC_000634_AddToShipyard(0xd, starport, &shipyardArray);
	}

	numShips = DATA_CurrentPirates*3;
	for (i = 0; i < numShips; i++)
		FUNC_000634_AddToShipyard(0x0, starport, &shipyardArray);

	numShips = DATA_CurrentTraders*3;
	for (i = 0; i < numShips; i++)
		FUNC_000634_AddToShipyard(0x1, starport, &shipyardArray);
}

extern "C" void RefreshShipyardData(starport_t *starport)
{
	INT32 idx, *shipyardArray;
	
	// delete 1 ship from the shipyard, maybe.
	idx = BoundRandom(14);
	
	if (idx < starport->numShips)
	{
		memmove(starport->shipyard+idx, starport->shipyard+idx+1, (starport->numShips-idx)*4);
		starport->numShips--;
	}
	
	shipyardArray = starport->shipyard+starport->numShips;

	if (BoundRandom(9) < DATA_CurrentPirates)
		FUNC_000634_AddToShipyard(0x0, starport, &shipyardArray);

	if (BoundRandom(9) < DATA_CurrentTraders)
		FUNC_000634_AddToShipyard(0x1, starport, &shipyardArray);

	if (BoundRandom(17) < DATA_NumStarports)
		FUNC_000634_AddToShipyard(0xd, starport, &shipyardArray);
}

extern "C" void SpawnTraders(INT32 traderLevel, INT32 bInitial)
{
	SINT32 i, numTraders, curTraders, numFreeSlots;

	numTraders = (sqrt(traderLevel*9)*DATA_NumStarports)/2.25;

	numFreeSlots = GetNumFreeSlots();
	
	// leave some room for extra objects such as cargo
	if (numTraders > numFreeSlots)
		numTraders = numFreeSlots;

	curTraders = numTraders/4;
	// spawn traders incoming via hyperspace
	for (i = 0; i < curTraders; i++)
		FUNC_000689_SpawnHSTrader();

	// spawn thargoids?
	curTraders = BoundRandom(FUNC_000035_GetSpecialShips(0x16));
	for (i = 0; i < curTraders; i++)
		FUNC_000690_SpawnHSTrader2();

	if (!bInitial)
		return;

	// spawn traders already in dock
	curTraders = (2*numTraders)/3;
	for (i = 0; i < curTraders; i++)
		FUNC_000691_SpawnDockedTrader();

	// spawn bulk carriers, etc.
	curTraders = numTraders/12;
	for (i = 0; i < curTraders; i++)
		FUNC_000699_SpawnAuxTrader(); 
}

extern "C" void CreateShips()
{
	INT32 i, mission_idx, name;

	for (i = 0; i < DATA_NumPackages; i++)
	{
		mission_idx = DATA_PackageArray[i].mission_idx;

		// military delivery mission?
		if (DATA_PackageArray[i].passengers == 0 && mission_idx >= 0x20)
		{
			if (mission_idx & 0x40)
			{
				mission_idx = 0x21;
				name = 0xffffffff;
			}
			else
			{
				mission_idx = 0x20;
				name = 0xfffffffe;
			}
		}
		else
		{
			mission_idx >>= 1;
			name = DATA_PackageArray[i].name;
		}
		
		if (DATA_PackageArray[i].system == DATA_CurrentSystem)
			SpawnAssassins(DATA_PackageArray[i].ships, mission_idx, name);
		else
			SpawnAssassins(sqrt(DATA_PackageArray[i].ships), mission_idx, name);
	}

	SpawnPirates(DATA_CurrentPirates, 1);
	SpawnTraders(DATA_CurrentTraders, 1);
}

extern "C" void RefreshShips()
{
	SpawnPirates(DATA_CurrentPirates, 0);
	SpawnTraders(DATA_CurrentTraders, 0);
}