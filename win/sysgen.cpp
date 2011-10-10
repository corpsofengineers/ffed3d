// Contains SysGen overrides for parameters.

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "misc.h"

typedef struct
{
	float baseAdditive;
	float randomAdditive;
} rangeEffect_t;

float effectScale[] =
{
	10.0,	// mining
	10.0,	// industry
	10.0,	// wet
	10.0,	// agriculture
	10.0,	// tourism
	10.0,	// life
	256.0,	// crime
};

rangeEffect_t agriEffects[33] =
{
	{-0.2, 0.1},		// Water
	{0.5, 0.2},			// Liquid Oxygen
	{0.9, 0.1},			// Grain
	{0.8, 0.2},			// Fruit and Veg.
	{0.0, 0.0},			// Animal Meat
	{-0.7, 0.1},		// Synthetic Meat
	{0.1, 0.05},		// Liquor
	{0.4, 0.5},			// Narcotics
	{0.0, 0.5},			// Medicines
	{-0.9, 0.1},		// Fertilizer
	{0.0, 0.0},			// Animal Skins
	{-0.2, 0.4},		// Live Animals
	{-0.5, 0.3},			// Slaves
	{-0.1, 0.2},		// Luxury Goods
	{0.0, 0.0},			// Heavy Plastics
	{-0.2, 0.1},		// Metal Alloys
	{-0.15, 0.15},		// Precious Metals
	{-0.15, 0.15},		// Gem Stones
	{0.0, 0.1},			// Minerals
	{-0.2, 0.3},		// Hydrogen Fuel
	{-0.2, 0.3},		// Military Fuel
	{0.0, 0.0},			// Hand Weapons
	{0.0, 0.0},			// Battle Weapons
	{0.0, 0.0},			// Nerve Gas
	{0.0, 0.0},			// Industrial Parts
	{-0.3, 0.15},		// Computers
	{0.2, 0.2},			// Air Processors
	{-0.9, 0.1},		// Farm Machinery
	{-0.65, 0.15},		// Robots
	{0.2, 0.1},			// Radioactives
	{0.2, 0.1},			// Rubbish
	{0.0, 0.0},			// Alien Artefacts
	{0.0, 0.0},			// Chaff
};

rangeEffect_t tourismEffects[33] =
{
	{0.0, 0.0},		// Water
	{0.7, 0.2},		// Liquid Oxygen
	{0.0, 0.0},		// Grain
	{0.0, 0.0},		// Fruit and Veg.
	{0.3, 0.1},		// Animal Meat
	{0.0, 0.0},		// Synthetic Meat
	{0.7, 0.1},		// Liquor
	{0.0, 0.0},		// Narcotics
	{0.2, 0.5},		// Medicines
	{0.0, 0.0},		// Fertilizer
	{0.3, 0.1},		// Animal Skins
	{0.0, 0.0},		// Live Animals
	{0.0, 0.0},		// Slaves
	{0.8, 0.1},		// Luxury Goods
	{0.0, 0.0},		// Heavy Plastics
	{0.0, 0.0},		// Metal Alloys
	{-0.3, 0.5},	// Precious Metals
	{-0.3, 0.5},	// Gem Stones
	{0.0, 0.1},		// Minerals
	{-0.2, 0.3},	// Hydrogen Fuel
	{-0.2, 0.3},	// Military Fuel
	{0.0, 0.0},		// Hand Weapons
	{0.0, 0.0},		// Battle Weapons
	{0.0, 0.0},		// Nerve Gas
	{0.0, 0.0},		// Industrial Parts
	{-0.8, 0.1},	// Computers
	{0.0, 0.0},		// Air Processors
	{0.0, 0.0},		// Farm Machinery
	{-0.35, 0.15},	// Robots
	{0.8, 0.1},		// Radioactives
	{0.4, 0.1},		// Rubbish
	{0.0, 0.0},		// Alien Artefacts
	{0.0, 0.0},		// Chaff
};

rangeEffect_t indEffects[33] =
{
	{0.0, 0.0},		// Water
	{-0.75, 0.15},	// Liquid Oxygen
	{-0.3, 0.1},	// Grain
	{-0.6, 0.2},	// Fruit and Veg.
	{-0.4, 0.1},	// Animal Meat
	{0.6, 0.2},		// Synthetic Meat
	{-0.2, 0.1},	// Liquor
	{0.4, 0.3},		// Narcotics
	{-0.2, 0.8},	// Medicines
	{0.65, 0.2},	// Fertilizer
	{0.0, 0.0},		// Animal Skins
	{0.0, 0.0},		// Live Animals
	{-0.5, 0.1},	// Slaves
	{-0.45, 0.15},	// Luxury Goods
	{0.4, 0.5},		// Heavy Plastics
	{-0.4, 0.1},	// Metal Alloys
	{0.0, 0.3},		// Precious Metals
	{0.0, 0.3},		// Gem Stones
	{-0.2, 0.6},	// Minerals
	{0.2, 0.3},		// Hydrogen Fuel
	{0.2, 0.3},		// Military Fuel
	{0.3, 0.2},		// Hand Weapons
	{0.3, 0.2},		// Battle Weapons
	{0.0, 0.3},		// Nerve Gas
	{-0.8, 0.1},	// Industrial Parts
	{0.8, 0.1},		// Computers
	{0.5, 0.3},		// Air Processors
	{0.75, 0.15},	// Farm Machinery
	{0.7, 0.2},		// Robots
	{-0.2, 0.1},	// Radioactives
	{-0.2, 0.1},	// Rubbish
	{0.0, 0.0},		// Alien Artefacts
	{0.0, 0.0},		// Chaff
};

rangeEffect_t mineEffects[33] =
{
	{-0.4, 0.1},	// Water
	{-0.75, 0.15},	// Liquid Oxygen
	{-0.7, 0.1},	// Grain
	{-0.3, 0.2},	// Fruit and Veg.
	{0.0, 0.0},		// Animal Meat
	{-0.9, 0.1},	// Synthetic Meat
	{-0.4, 0.2},	// Liquor
	{-0.2, 0.1},	// Narcotics
	{-0.6, 0.2},	// Medicines
	{0.0, 0.0},		// Fertilizer
	{0.0, 0.0},		// Animal Skins
	{0.0, 0.0},		// Live Animals
	{-0.9, 0.1},	// Slaves
	{-0.15, 0.15},	// Luxury Goods
	{-0.75, 0.15},	// Heavy Plastics
	{0.8, 0.1},		// Metal Alloys
	{0.6, 0.2},		// Precious Metals
	{0.6, 0.2},		// Gem Stones
	{0.9, 0.1},		// Minerals
	{0.6, 0.2},		// Hydrogen Fuel
	{0.6, 0.2},		// Military Fuel
	{0.0, 0.0},		// Hand Weapons
	{0.0, 0.0},		// Battle Weapons
	{0.0, 0.0},		// Nerve Gas
	{0.8, 0.1},		// Industrial Parts
	{-0.3, 0.1},	// Computers
	{-0.8, 0.1},	// Air Processors
	{0.0, 0.0},		// Farm Machinery
	{-0.5, 0.2},	// Robots
	{-0.4, 0.1},	// Radioactives
	{-0.4, 0.1},	// Rubbish
	{0.0, 0.0},		// Alien Artefacts
	{0.0, 0.0},		// Chaff
};

rangeEffect_t wetEffects[33] =
{
	{1.0, 0.0},	// Water
	{0.0, 0.0},
	{0.0, 0.0},
	{0.0, 0.0},
	{0.0, 0.0},
	{0.0, 0.0},
	{0.0, 0.0},
	{0.0, 0.0},
	{0.0, 0.0},
	{0.0, 0.0},
	{0.0, 0.0},
	{0.0, 0.0},
	{0.0, 0.0},
	{0.0, 0.0},
	{0.0, 0.0},
	{0.0, 0.0},
	{0.0, 0.0},
	{0.0, 0.0},
	{0.0, 0.0},
	{0.0, 0.0},
	{0.0, 0.0},
	{0.0, 0.0},
	{0.0, 0.0},
	{0.0, 0.0},
	{0.0, 0.0},
	{0.0, 0.0},
	{0.0, 0.0},
	{0.0, 0.0},
	{0.0, 0.0},
	{0.0, 0.0},
	{0.0, 0.0},
	{0.0, 0.0},
	{0.0, 0.0},
};

rangeEffect_t lifeEffects[33] =
{
	{0.0, 0.0},		// Water
	{0.0, 0.0},		// Liquid Oxygen
	{-0.1, 0.05},	// Grain
	{0.0, 0.0},		// Fruit and Veg.
	{0.5, 0.0},		// Animal Meat
	{0.0, 0.0},		// Synthetic Meat
	{0.0, 0.0},		// Liquor
	{0.0, 0.0},		// Narcotics
	{-0.05, 0.0},	// Medicines
	{0.1, 0.0},		// Fertilizer
	{0.5, 0.0},		// Animal Skins
	{0.5, 0.0},		// Live Animals
	{0.0, 0.0},		// Slaves
	{0.0, 0.0},		// Luxury Goods
	{0.0, 0.0},		// Heavy Plastics
	{0.0, 0.0},		// Metal Alloys
	{0.0, 0.0},		// Precious Metals
	{0.0, 0.0},		// Gem Stones
	{0.0, 0.0},		// Minerals
	{0.0, 0.0},		// Hydrogen Fuel
	{0.0, 0.0},		// Military Fuel
	{-0.05, 0.0},	// Hand Weapons
	{0.0, 0.0},		// Battle Weapons
	{0.0, 0.0},		// Nerve Gas
	{0.0, 0.0},		// Industrial Parts
	{0.0, 0.0},		// Computers
	{0.0, 0.0},		// Air Processors
	{0.0, 0.0},		// Farm Machinery
	{0.0, 0.0},		// Robots
	{0.0, 0.0},		// Radioactives
	{-0.05, 0.0},	// Rubbish
	{0.0, 0.0},		// Alien Artefacts
	{0.0, 0.0},		// Chaff
};

rangeEffect_t crimeEffects[33] =
{
	{0.0, 0.0},		// Water
	{0.0, 0.0},		// Liquid Oxygen
	{0.0, 0.0},		// Grain
	{0.0, 0.0},		// Fruit and Veg.
	{0.0, 0.0},		// Animal Meat
	{0.0, 0.0},		// Synthetic Meat
	{0.0, 0.0},		// Liquor
	{0.7, 0.1},		// Narcotics
	{-0.4, 0.1},	// Medicines
	{0.0, 0.0},		// Fertilizer
	{0.0, 0.0},		// Animal Skins
	{0.0, 0.0},		// Live Animals
	{0.6, 0.1},		// Slaves
	{-0.25, 0.15},	// Luxury Goods
	{0.0, 0.0},		// Heavy Plastics
	{0.0, 0.0},		// Metal Alloys
	{0.0, 0.0},		// Precious Metals
	{0.0, 0.0},		// Gem Stones
	{0.0, 0.0},		// Minerals
	{0.0, 0.0},		// Hydrogen Fuel
	{0.0, 0.0},		// Military Fuel
	{-0.8, 0.1},	// Hand Weapons
	{-0.6, 0.2},	// Battle Weapons
	{0.6, 0.3},		// Nerve Gas
	{0.0, 0.0},		// Industrial Parts
	{0.0, 0.0},		// Computers
	{0.0, 0.0},		// Air Processors
	{0.0, 0.0},		// Farm Machinery
	{0.0, 0.0},		// Robots
	{-0.4, 0.1},	// Radioactives
	{-0.4, 0.1},	// Rubbish
	{0.0, 0.0},		// Alien Artefacts
	{0.0, 0.0},		// Chaff
};

rangeEffect_t *pEffectTables[EF_MAX] =
{
	mineEffects,
	indEffects,
	wetEffects,
	agriEffects,
	tourismEffects,
	lifeEffects,
	crimeEffects,
};

rangeEffect_t govMorality[] =
{
	{0.0, 0.0},		// None
	{0.0, 0.0},		// Anarchy
	{3.0, 0.5},		// Fuedal
	{5.0, 0.5},		// Local Corporations
	{15.0, 2.5},	// Corp. State
	{25.0, 8.0},	// Federal Democracy
	{20.0, 2.0},	// Federal Colonial Rule
	{10.0, 5.0},	// Dictatorship
	{35.0, 5.0},	// Communist
	{60.0, 10.0},	// Religious
	{25.0, 8.0},	// Democracy
	{16.5, 1.0},	// Imperial Rule
	{7.0, 2.0},		// Imperial Colonial Rule
	{0.0, 0.0},		// None stable
	{10.0, 1.0},	// Martial Law
	{20.0, 8.0},	// Alliance Democracy
};	

// democracy wins again
float idealMorality = 25.0;

rangeEffect_t stockMorality[] =
{
	{80.0, 12.0},	// Water
	{80.0, 5.0},	// Liquid Oxygen
	{80.0, 9.0},	// Grain
	{80.0, 12.0},	// Fruit and Veg.
	{60.0, 6.0},	// Animal Meat
	{70.0, 5.0},	// Synthetic Meat
	{32.0, 3.0},	// Liquor
	{20.0, 8.0},	// Narcotics
	{68.0, 8.0},	// Medicines
	{80.0, 12.0},	// Fertilizer
	{24.0, 3.0},	// Animal Skins
	{20.0, 2.0},	// Live Animals
	{12.5, 1.0},	// Slaves
	{58.5, 5.0},	// Luxury Goods
	{80.0, 5.0},	// Heavy Plastics
	{80.0, 5.0},	// Metal Alloys
	{68.0, 0.0},	// Precious Metals
	{68.0, 0.0},	// Gem Stones
	{80.0, 5.0},	// Minerals
	{80.0, 5.0},	// Hydrogen Fuel
	{80.0, 12.0},	// Military Fuel
	{18.0, 2.0},	// Hand Weapons
	{8.0,  1.0},	// Battle Weapons
	{2.0,  2.0},	// Nerve Gas
	{80.0, 5.0},	// Industrial Parts
	{64.0, 1.0},	// Computers
	{80.0, 5.0},	// Air Processors
	{70.0, 1.0},	// Farm Machinery
	{56.0, 1.0},	// Robots
	{80.0, 12.5},	// Radioactives
	{80.0, 0.0},	// Rubbish
	{80.0, 0.0},	// Alien Artefacts
	{80.0, 0.0},	// Chaff
};

// higher ones are selected for in advanced systems, generally
government_t governments[] =
{
	GOV_ANARCHY,
	GOV_RELIGIOUS,
	GOV_FUEDAL,
	GOV_FUEDAL,
	GOV_LOCALCORP,
	GOV_LOCALCORP,
	GOV_LOCALCORP,
	GOV_LOCALCORP,
	GOV_COMMUNIST,
	GOV_DICTATORSHIP,
	GOV_DICTATORSHIP,
	GOV_CORPSTATE,
	GOV_CORPSTATE,
	GOV_CORPSTATE,
	GOV_DICTATORSHIP,
	GOV_DICTATORSHIP,
	GOV_CORPSTATE,
	GOV_DEMOCRACY,
};

float govVarMults[] =
{
	1.0,	
	1.2,	// anarchy
	1.0,	// fuedal
	1.1,	// local corporations
	1.1,	// corporate state
	1.0,
	1.0,
	1.0,	// dictatorship
	1.0,	// communist
	0.75,	// religious
	1.0,	// democracy
	1.0,
	1.0,
	1.0,
	1.0,
	1.0,
};

float govCrimeExp[] =
{
	2.0,	// None
	2.0,	// Anarchy
	1.5,	// Feudal
	1.3,	// Local Corp
	0.75,	// Corporate State
	0.4,	// Federal Democracy
	0.55,	// Federal Colonial Rule
	1.0,	// Dictatorship
	0.8,	// Communist
	0.65,	// Religious
	0.5,	// Democracy
	0.33,	// Imperial Rule
	0.4,	// Imperial Colonial Rule
	2.0,	// No stable government
	2.0,	// Martial Law
	0.5,	// Alliance Democracy
};

// indexed by allegiance, determines allegiance of generated systems
INT16 CoreX[] = {0x1718, 0x1718, 0x1718, 0x1717};
INT16 CoreY[] = {0x1524, 0x1520, 0x1524, 0x1528};
float CoreDistMult[] = {0.0, 1.0, 1.0, 6.0};

government_t fullAllyGov[] =
{
	GOV_NONE,
	GOV_IMPERIAL_RULE,
	GOV_FEDERAL_DEMOCRACY,
	GOV_ALLIANCE_DEMOCRACY,
};

government_t partAllyGov[] =
{
	GOV_NONE,
	GOV_IMPERIAL_COLONY,
	GOV_FEDERAL_COLONY,
	GOV_ALLIANCE_DEMOCRACY,
};

extern "C" void AddPlanetEffects(INT16 modelindex, SINT32 randSeed, SINT32 efScale, SINT32 *effects, INT8 *worldPorts, SINT32 *numWorldPorts)
{
	SINT32 randSeed2;
	SINT32 agriBoost, tourBoost, lifeBoost, indBoost; 
	INT16 rnd;

	// outdoor world?
	randSeed2 = randSeed = (randSeed << 0x6) | (randSeed >> 0x1a);
				
	rnd = DATA_FixedRandomFunc(0x10000, &randSeed, &randSeed2);
			
	agriBoost = 4 + (rnd & 0x7);
	tourBoost = 13 - (rnd & 0x7);

	lifeBoost = 4 + ((rnd >> 4) & 0x7);
	indBoost = 0;
	
	switch (modelindex)
	{
	case 0x83:	// jungle world
		effects[EF_WET] += 9;

		agriBoost *= 1.25;
		tourBoost *= 0.8;
		lifeBoost *= 2.0;

		worldPorts[ST_JUNGLE] += efScale;
		break;

	case 0x82:	// desert world
		effects[EF_WET] -= 30;

		agriBoost *= 0.5;
		tourBoost *= -1.0;
		lifeBoost *= 0.5;
				
		worldPorts[ST_DESERT] += efScale;
		break;

	case 0x7f:	// ice world
		effects[EF_WET] += 30;
		tourBoost *= 0.25;
		lifeBoost *= 1.25;
		agriBoost *= -1.0;

		worldPorts[ST_ICE] += efScale;
		break;

	case 0x81:	// earth
	case 0x7e:	// garden world
		effects[EF_WET] += 9;
		
		worldPorts[ST_GREEN] += efScale;
		break;
	default:
		effects[EF_INDUSTRY] += 10;
		effects[EF_WET]--;
		worldPorts[ST_INDUSTRY]++;
		(*numWorldPorts)++;
		return;
	}

	(*numWorldPorts) += efScale;
	effects[EF_TOURISM] += tourBoost * efScale;
	effects[EF_AGRICULTURE] += agriBoost * efScale;
	effects[EF_LIFE] += lifeBoost * efScale;
}

INT32 lastGeneratedSystem = 0;
sysType_t lastGeneratedSysType;
INT32 lastGeneratedTechLevel;
government_t lastGeneratedGovType;
INT8 lastGeneratedAllegiance;

// randomly give would-be boring systems civilization and classification.
extern "C" void GenSystemBasicType(INT32 id, sysType_t *type, INT32 *techLevel, government_t *govType, INT8 *allegiance)
{
	SINT32 i, sectX, sectY, techAdd, temp, indLevel, mineLevel;
	SINT32 numStarports, numWorldPorts;
	INT16 lostTourism, rand2, rand3, rand4;
	float distFromCenter, fTemp, fTemp2, distFromCore, lowestDist;
	INT8 worldPorts[ST_MAX], highestPorts, closestPower;
	systemObject_t sysObjects[60], *parentObj, *oldParent;
	sysGenParams_t genParams;

	// save a TON of processing time, really
	if (id == lastGeneratedSystem)
	{
		*type = lastGeneratedSysType;
		*govType = lastGeneratedGovType;
		*allegiance = lastGeneratedAllegiance;
		*techLevel = lastGeneratedTechLevel;

		return;
	}

		
	// get x,y coords of system
	sectX = id & 0x1fff;
	sectY = (id >> 0xd) & 0x1fff;

	temp = sectX - 0x1718;
	distFromCenter = temp*temp;
	temp = sectY - 0x1524;
	distFromCenter += temp*temp;

	srand(id);

	fTemp = 0.5 + 0.5*((rand() & 0x7fff) / 32768.0);
	rand2 = rand();
	rand3 = rand();
	rand4 = rand();
	techAdd = 0xfd*(fTemp - distFromCenter/(MAX_SECT_DIST*MAX_SECT_DIST));
	
	if (techAdd > 0)
		*techLevel += techAdd;

	if (*techLevel > 0xfd)
		*techLevel = 0xfd;

	memset(&genParams, 0, sizeof(genParams));
	FUNC_GetSysGenParams(&genParams, id);
	genParams.techLevel = *techLevel;
	
	FUNC_000878_GetSysObjectData(sysObjects, &genParams, 0);

	// iterate through major bodies in the system.
	numStarports = 0;
	numWorldPorts = 0;

	oldParent = 0;

	for (i = 0; i < ST_MAX; i++)
		worldPorts[i] = 0;

	memset(type, 0, sizeof(*type));

	type->distFromCenter = sqrt(distFromCenter);

	for (i = 0; i < 60 && sysObjects[i].mass != 0; i++)
	{
		if (sysObjects[i].objType >= 0x20)
			numStarports++;

		if (sysObjects[i].objType == 0x23)
		{
			parentObj = sysObjects + (sysObjects[i].parentindex-1);

			AddPlanetEffects(parentObj->modelindex, parentObj->randseed, (parentObj->objType == 0x9) ? 1 : 3, type->effects, worldPorts, &numWorldPorts);

			oldParent = parentObj;
		}
	}

	if (numWorldPorts == 0)
	{
		type->development = 0;
		if (*techLevel < 0xc)
		{
			*govType = GOV_NONE;
			type->effects[EF_CRIME] = 0;
			return;
		}

		type->effects[EF_CRIME] = 128.0*(distFromCenter/(MAX_SECT_DIST*MAX_SECT_DIST));
		type->effects[EF_CRIME] += 0.5*(0xff - type->development);
	
		fTemp = (rand3 & 0x7fff) / 32768.0;
		fTemp *= fTemp;

		if (type->effects[EF_CRIME] > 0xff)
			type->effects[EF_CRIME] = 0xff;

		type->effects[EF_CRIME] += (rand3 & 0x8000) ? (-64.0*fTemp) : (64.0*fTemp);

		if (type->effects[EF_CRIME] < 0)
			type->effects[EF_CRIME] = 0;
		else if (type->effects[EF_CRIME] > 0xff)
			type->effects[EF_CRIME] = 0xff;
		
		if (type->effects[EF_CRIME] > 164)
			*govType = GOV_ANARCHY;
		else if (type->effects[EF_CRIME] > 96)
			*govType = GOV_FUEDAL;
		else
			*govType = GOV_LOCALCORP;
		
		type->primaryWorld = ST_INDUSTRY;
		
		lastGeneratedSystem = id;
		lastGeneratedSysType = *type;
		lastGeneratedTechLevel = *techLevel;
		lastGeneratedGovType = *govType;
		lastGeneratedAllegiance = *allegiance;
		return;
	}

	highestPorts = 0;
	// decide on primary world type (most ports)
	for (i = 0; i < ST_MAX; i++)
	{
		if (worldPorts[i] >= highestPorts)
		{
			highestPorts = worldPorts[i];
			type->primaryWorld = (worldType_t)i;
		}
	}

	// boost the dominant effect
	if (type->primaryWorld == ST_INDUSTRY)
	{
		type->effects[EF_INDUSTRY] += 0.75*type->effects[EF_AGRICULTURE];
		type->effects[EF_INDUSTRY] += 0.75*type->effects[EF_TOURISM];
		type->effects[EF_AGRICULTURE] *= 0.25;
		type->effects[EF_TOURISM] *= 0.25;
	}
	else if (type->primaryWorld != ST_ICE && type->primaryWorld != ST_DESERT)
	{
		SINT32 effectSum, newEffects;

		effectSum = type->effects[EF_AGRICULTURE];
		effectSum = type->effects[EF_TOURISM];

		newEffects = effectSum + 0.75*type->effects[EF_INDUSTRY];
		type->effects[EF_AGRICULTURE] *= newEffects;
		type->effects[EF_AGRICULTURE] /= effectSum;
		type->effects[EF_TOURISM] *= newEffects;
		type->effects[EF_TOURISM] /= effectSum;
		
		type->effects[EF_INDUSTRY] *= 0.25;
	}


/*	temp = numStarports*25;
	if (type->development > (INT32)temp)
		type->development = (INT32)temp;*/

	type->development = (*techLevel / 4.0) + 128.0*(numStarports/18.0);

	type->development += 64.0*(1 - distFromCenter/(MAX_SECT_DIST*MAX_SECT_DIST));

	if (type->development > 0xfd)
		type->development = 0xfd;

	// allied to any particular power?
	
	lowestDist = (MAX_SECT_DIST*MAX_SECT_DIST);
	for (i = ALLY_IMPERIAL; i <= ALLY_ALLIANCE; i++)
	{
		temp = sectX - CoreX[i];
		distFromCore = temp*temp;
		temp = sectY - CoreY[i];
		distFromCore += temp*temp;
		distFromCore *= CoreDistMult[i];

		if (distFromCore < lowestDist)
		{
			lowestDist = distFromCore;
			closestPower = i;
		}
	}

	// should you be subjugated?
	// high-tech worlds are more tempting for takeover
	fTemp = (rand2 & 0x7fff) / 32768.0;
	fTemp = pow(fTemp, 24.0*lowestDist/(MAX_SECT_DIST*MAX_SECT_DIST));
	fTemp *= type->development / 256.0;

	*allegiance = closestPower;
	if (fTemp > 0.75)	// full-fledged member
		*govType = fullAllyGov[closestPower];
	else if (fTemp > 0.4)	// colony
		*govType = partAllyGov[closestPower];
	else
	{
		fTemp = (rand4 & 0x7fff) / 32768.0;
		fTemp2 = type->development / 768.0;

		fTemp = fTemp2 + (1 - fTemp2)*pow(fTemp, 14208.0 / (type->development*type->development)); 
		*govType = governments[(int)(NUM_ELEMENTS(governments)*fTemp)];
		*allegiance = ALLY_INDEPENDENT;
	}

	// indLevel divided between mining and industry, depending on development
	fTemp = (0xff - type->development) / 256.0;
	indLevel = type->effects[EF_INDUSTRY];
	mineLevel = indLevel * fTemp;
	indLevel -= mineLevel;

	// boost the dominant one
	if (indLevel > mineLevel)
	{
		indLevel += 0.75*mineLevel;
		mineLevel *= 0.25;
	}
	else
	{
		mineLevel += 0.75*indLevel;
		indLevel *= 0.25;
	}

	type->effects[EF_INDUSTRY] = indLevel;
	type->effects[EF_MINING] = mineLevel;

	// average the values out
	for (i = 0; i < EF_MAX; i++)
		type->effects[i] /= numWorldPorts;

	fTemp = 0.5*(distFromCenter/(MAX_SECT_DIST*MAX_SECT_DIST));
	fTemp += 0.5*(0xff - type->development) / 256.0;

	fTemp2 = (rand3 & 0x7fff) / 32768.0;
	fTemp2 *= fTemp2;

	fTemp += (rand3 & 0x8000) ? (-0.15*fTemp2) : (0.15*fTemp2); 
	if (fTemp > 1.0)
		fTemp = 1.0;
	else if (fTemp < 0.0)
		fTemp = 0.0;

	type->effects[EF_CRIME] = 256.0*(1 - pow(1 - fTemp, govCrimeExp[*govType]));

	if (type->effects[EF_CRIME] < 0)
		type->effects[EF_CRIME] = 0;
	else if (type->effects[EF_CRIME] > 0xff)
		type->effects[EF_CRIME] = 0xff;

	// lose tourism to high crime levels.  Make up for it in agriculture
	
	if (type->effects[EF_TOURISM] > 0)
	{
		lostTourism = (type->effects[EF_CRIME]/64.0);
		lostTourism *= lostTourism;
	
		if (lostTourism > type->effects[EF_TOURISM])
			lostTourism = type->effects[EF_TOURISM];

		if (type->effects[EF_AGRICULTURE] < 0)
			type->effects[EF_AGRICULTURE] -= lostTourism*0.8;
		else
			type->effects[EF_AGRICULTURE] += lostTourism*0.8;

		type->effects[EF_TOURISM] -= lostTourism;
	}

	lastGeneratedSystem = id;
	lastGeneratedSysType = *type;
	lastGeneratedTechLevel = *techLevel;
	lastGeneratedGovType = *govType;
	lastGeneratedAllegiance = *allegiance;
}

extern "C" void GetSupplyFromEffects(INT8 *supply, SINT32 effects[EF_MAX], government_t government)
{
	float stockLevels[33], fRand;
	SINT8 i, j;

	for (i = 0; i < 32; i++)
	{
		stockLevels[i] = 0.0;

		if (supply[i] & 0x80)
			continue;

		for (j = 0; j < EF_MAX; j++)
		{
			fRand = ((rand() & 0x7fff) / 16384.0) - 1.0;
			stockLevels[i] += (effects[j] / effectScale[j]) * ((pEffectTables[j])[i].baseAdditive + fRand*((pEffectTables[j])[i]).randomAdditive);
		}
		
		if (stockLevels[i] < 0)
			stockLevels[i] = -stockLevels[i];
		else
			supply[i] |= 0x8;


		if (stockLevels[i] > 1.0)
			stockLevels[i] = 1.0;

		if (government == GOV_RELIGIOUS)
			stockLevels[i] *= stockLevels[i];	// stifle development
		
		// make radioactives illegal if major export
		if (i == 29 && stockLevels[i] > 0.75 && supply[i] & 0x8)
			supply[i] = 0x81;
		else
			supply[i] |= (INT8)(7.99*stockLevels[i]);
	}

	supply[32] = 0xf; 
}

extern "C" void FlagIllegalGoods(INT8 *supply, sysType_t *type, government_t government)
{
	INT8 i;
	float stockMoral, govMoral, fRand, moralDiff;
	
	fRand = ((rand() & 0x7fff) / 16384.0) - 1.0;
	govMoral = govMorality[government].baseAdditive + (fRand*govMorality[government].randomAdditive);

	if (govMoral == 0.0)
		return; // anarchy

	// excessively stringent controls need low development
	if (govMoral > idealMorality && type->development > 0x62)
	{
		moralDiff = govMoral - idealMorality;

		moralDiff = pow(1 + moralDiff, 1 / (1 + ((type->development - 0x62) / 256.0))) - 1;
		govMoral = idealMorality + moralDiff;
	}
/*	else if (govMoral < idealMorality && type->development > 0x62)	// excessively lax controls less so, but still there
	{
		moralDiff = idealMorality - govMoral;

		moralDiff = pow(1 + moralDiff, sqrt(1 / (1 + ((type->development - 0x62) / 256.0)))) - 1;
		govMoral = idealMorality - moralDiff;
	}*/

	for (i = 0; i < 32; i++)
	{
		fRand = pow((rand() & 0x7fff) / 16384.0, 0.2*(9 - (supply[i] & 0x7))) - 1.0;
		stockMoral = stockMorality[i].baseAdditive + fRand*stockMorality[i].randomAdditive;
		
		if (stockMoral < govMoral)
			supply[i] = 0x81;
	}
}

// techLevel determines # of starports
extern "C" void Override_F869(INT32 id, INT32 *overStr, INT32 *descStr, INT32 *techLevel, INT32 *pirates, INT32 *policeLevel, INT32 *traders, INT32 *miners, INT32 *government)
{
	sysType_t type;
	government_t govType;
	INT8 allegiance;

	allegiance = *government >> 6;
	if (allegiance != ALLY_INDEPENDENT) 
		return;

	GenSystemBasicType(id, &type, techLevel, &govType, &allegiance);
		
	*government &= ~0x7f;
	*government |= govType;
	*government |= (INT16)allegiance << 6;

	*pirates = 10 * (type.effects[EF_CRIME] / 256.0);
	*traders = 10 * sqrt(type.development / 256.0);
	*policeLevel = 9 - *pirates;

	// get description string for system.
	switch (type.primaryWorld)
	{
	case ST_INDUSTRY:
		if (allegiance == ALLY_IMPERIAL)
		{
			*overStr = 0x8500;	// Imperial Industrial and Mining Colony.
			*descStr = 0x801d;
		}
		else if (allegiance == ALLY_ALLIANCE)
		{
			*overStr = 0x8509;	// Alliance Industrial and Mining System.
			*descStr = 0x8026;
		}
		else if (type.development >= 0xb0)
			*overStr = 0x84eb; // extensive mining and industrial development.
		else if (type.development >= 0x89)
			*overStr = 0x84ea; // mining and heavy manufacturing industry.
		else if (type.development >= 0x62)
			*overStr = 0x84e9; // mining and some ore refinement.
		else if (type.development > 0)
			*overStr = 0x84e8; // some small-scale mining operations.
		else if (*techLevel >= 0xc)
			*overStr = 0x84e7; // frontier system.  some prospecting and mining.
		else if (*techLevel >= 0x2)
			*overStr = 0x84e6; // Explored, no settlements
		else
			*overStr = 0x84e5; // Unexplored
		
		break;
	case ST_GREEN:
		if (allegiance == ALLY_FEDERAL)
		{
			if (type.effects[EF_AGRICULTURE] >= type.effects[EF_TOURISM]*2)
				*overStr = 0x84ef; // Terraformed agricultural world.  Federation Member.
			else if (type.development > 202.0)
				*overStr = 0x84fc; // High population outdoor world.  Federation Member.
			else
			{
				if (type.effects[EF_TOURISM] > 8)
					*overStr = 0x84f9; // terraformed world.  high-cost tourism.
				else
					*overStr = 0x84f8; // terraformed agricultural world.
			}
		}
		else if (allegiance == ALLY_IMPERIAL)
		{
			if (type.development > 202.0)
				*overStr = 0x84ff;	// high-population outdoor world under imp. rule
			else
				*overStr = 0x8501;	// imperial terraformed world.
		}
		else if (type.effects[EF_AGRICULTURE] >= type.effects[EF_TOURISM]*2)
			*overStr = 0x84f8; // terraformed agricultural world.
		else if (type.effects[EF_TOURISM] >= type.effects[EF_AGRICULTURE]*2)
			*overStr = 0x84f9; // terraformed world.  high-cost tourism.
		else if (type.distFromCenter > 5)
			*overStr = 0x84ee; // Frontier outdoor world.  Some farming and tourism.
		else
		{
			if (type.effects[EF_TOURISM] > type.effects[EF_AGRICULTURE])
				*overStr = 0x84f9; // terraformed world.  high-cost tourism.
			else
				*overStr = 0x84f8; // terraformed agricultural world.
		}
		
		break;
	case ST_JUNGLE:
		*overStr = 0x84f1;	// Outdoor jungle world.
//		*descStr = 0x800c;	// Wildlife can be a little dangerous.
		break;
	case ST_ICE:
		if (type.effects[EF_LIFE] <= 4)
			*overStr = 0x84fa;	// Terraformed Ice and Water world.  Tourism.
		else
			*overStr = 0x84f0;	// Outdoor Ice and Water world.  Tourism and fishing.
		break;
	case ST_DESERT:
		*overStr = 0x84f2;	// Outdoor desert world.  Some agriculture.
	}
}

INT32 lastStockGeneratedSystem = 0;
INT8 lastGeneratedStockFlags[33];

extern "C" void Override_F870(INT32 id, INT8 **stockFlags, INT32 *population, INT32 *danger, INT32 *d4)
{
	INT32 overStr, descStr, d3, pirates, techLevel, traders, d7, government;
	sysType_t type;
	government_t govType;
	INT8 allegiance;

	FUNC_000869_NoOverride(id, &overStr, &descStr, &techLevel, &pirates, &d3, &traders, &d7, &government);

	allegiance = government >> 6;
	if (allegiance != ALLY_INDEPENDENT) 
		return;

	GenSystemBasicType(id, &type, &techLevel, &govType, &allegiance);
	
	if (govType == GOV_ANARCHY || govType == GOV_NONE_STABLE)
		*danger = 2;
	else
		*danger = 0;

	if (type.development == 0)
	{
		if (techLevel >= 0xc)
			*population = 1 + (rand() & 0x1);
		else
			*population = 0;
		return;
	}

	*population = 2 + 8 * (type.development / 256.0);

	// signal
	if (stockFlags == 0)
	{
		// return allegiance in d4 instead
		*d4 = allegiance;
		
		// return tourism in d3 instead
		*danger = type.effects[EF_TOURISM] > 0 ? type.effects[EF_TOURISM] : 0;
		return;
	}

	// again, save loads of time
	if (lastStockGeneratedSystem == id)
	{
		memcpy(*stockFlags, lastGeneratedStockFlags, sizeof(lastGeneratedStockFlags));
		return;
	}
	
	memset(*stockFlags, 0, 33);
	FlagIllegalGoods(*stockFlags, (sysType_t *)(type.effects), govType);
	
	// force legal slavery
	if (allegiance == ALLY_IMPERIAL)
		(*stockFlags)[12] &= ~0x80;

	GetSupplyFromEffects(*stockFlags, type.effects, govType);
	(*stockFlags)[29] &= ~0x7;
	(*stockFlags)[30] &= ~0x7;

	lastStockGeneratedSystem = id;
	memcpy(lastGeneratedStockFlags, *stockFlags, sizeof(lastGeneratedStockFlags));
}

extern "C" void Override_F871(INT32 id, INT32 *d1, INT32 *d2, INT32 *milActivity, INT32 *allegiance, INT32 *milRankSub, INT32 *milRankBase)
{
	INT32 overStr, descStr, d3, pirates, techLevel, traders, d7, government;
	sysType_t type;
	government_t govType;
	INT8 ally;

	FUNC_000869_NoOverride(id, &overStr, &descStr, &techLevel, &pirates, &d3, &traders, &d7, &government);

	ally = government >> 6;
	if (ally != ALLY_INDEPENDENT) 
		return;

	GenSystemBasicType(id, &type, &techLevel, &govType, &ally);

	*allegiance = ally;
	if (*allegiance == ALLY_IMPERIAL)
	{
		*milRankSub = DATA_FederalRank;
		*milRankBase = DATA_ImperialRank;
	}
	else
	{
		*milRankSub = DATA_ImperialRank;
		*milRankBase = DATA_FederalRank;
	}

	*milActivity = 1; // no generated system is of great importance to the military
}

// starport supply depends on parent planet.
// for instance, Computers will probably be cheaper on an industrial world
extern "C" void GetStarportSupply(starport_t *starport, INT8 *supply)
{
	ModelInstance_t *starportObj, *parentObj;
	INT8 worldPorts[ST_MAX];
	SINT32 effects[EF_MAX], numWorldPorts;

	starportObj = FUNC_001532_GetModelInstancePtr(starport->objectIdx, DATA_ObjectArray);
	parentObj = FUNC_001532_GetModelInstancePtr(starportObj->parent_index, DATA_ObjectArray);

	srand(INT16_AT(starport+0xa0));
	memset(effects, 0, sizeof(effects));

	AddPlanetEffects(parentObj->model_num, parentObj->globalvars.unique_Id, 1, effects, worldPorts, &numWorldPorts);
	
	effects[EF_MINING] = (1.0 - DATA_CurrentPopulation/9.0)*effects[EF_INDUSTRY];
	effects[EF_INDUSTRY] -= effects[EF_MINING];

	memset(supply, 0, 33);
	GetSupplyFromEffects(supply, effects, GOV_NONE);
}
