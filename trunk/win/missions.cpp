/*	Missions.c - In-game, non handcoded mission funcs */

#include "ffe3d.h"

typedef struct 
{
	u32 cash;
	u32 baseShips;
	u32 addShips;
	u8 *shipData;
	eliteranking_t rankRequirement;
} packageinfo_t;

typedef struct
{
	u32 cash;
	u32 stringIdx;
	u32 shipIdx;
	eliteranking_t ranking;
} assassinationInfo_t;

// opposing core systems for allegiances
u32 CoreSystems[] =
{
	0x12a49718,
	0x12a49718,
	0xaa41718,
	0xaa41718,
	0xaa41718,
	0xaa41718,
};

// ANISO - new civvie assassination tables.
// Goes: Cash, StringIdx, ShipIdx, Ranking (0 = Harmless)
// Up to 32000 credits is added randomly, in units of 8000
assassinationInfo_t AssassinationData[] =
{
	{ 4000, 0, 0x1e, RATING_MOSTLYHARMLESS }, // Viper Defence Craft
	{ 20000, 0, 0x28, RATING_MOSTLYHARMLESS }, // Asp Explorer
	{ 5000,  0, 0x35, RATING_MOSTLYHARMLESS }, // Transporter
	{ 12000, 0, 0x36, RATING_POOR			}, // Lion Transport
	{ 10000, 1, 0x35, RATING_POOR			}, // Transporter, again
	{ 15000, 1, 0x37, RATING_BELOWAVG		}, // Tiger Trader
	{ 18000, 2, 0x3a, RATING_BELOWAVG		}, // Imp. Trader
	{ 21000, 2, 0x35, RATING_BELOWAVG		}, // Transporter, again
	{ 28000, 3, 0x43, RATING_AVERAGE		}, // Mantis Freighter
	{ 32000, 3, 0x30, RATING_AVERAGE		}, // Harrier
	{ 35000, 4, 0x44, RATING_ABOVEAVG		}, // Griffin Hauler
	{ 42000, 4, 0x2d, RATING_COMPETENT		}, // Skeet Cruiser
	{ 45000, 4, 0x38, RATING_COMPETENT		}, // Imp. Courier
	{ 54000, 4, 0x3b, RATING_COMPETENT		}, // Anaconda Destroyer
	{ 75000, 5, 0x3e, RATING_DANGEROUS		}, // Panther Clipper
	{ 85000, 5, 0x41, RATING_DANGEROUS		}, // Imp. Explorer	
};

// cheap ships used by the Mafia.
u8 MafiaShips[] = 
{
	0x1c, 0x1c, 0x1c, 0x1c, 0x1a, 0x1a, 0x1b, 0x1b,
	0x24, 0x24, 0x25, 0x26, 0x1b, 0x1b, 0x1f, 0x1b,
};

// ships used by individuals with vendettas.
u8 IndyShips[] =
{
	0x1c, 0x1c, 0x1d, 0x1d, 0x26, 0x26, 0x26, 0x26,
	0x28, 0x28, 0x28, 0x28, 0x27, 0x27, 0x24, 0x24,
};

// sleek ships used by the Mega-Corps.
u8 CorporateShips[] =
{	
	0x30, 0x30, 0x2f, 0x29, 0x22, 0x22, 0x23, 0x23, 
	0x2a, 0x2a, 0x2a, 0x1f, 0x1f, 0x28, 0x28, 0x28,
};

// ships used by the cops.
u8 PoliceShips[] =
{
	0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e,
	0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22,
};

// ships used by the federal military.
u8 FederalShips[] =
{
	0x28, 0x28, 0x28, 0x28, 0x27, 0x27, 0x29, 0x29, 
	0x2f, 0x2f, 0x2f, 0x30, 0x30, 0x2a, 0x2a, 0x2d, 
};

// imperial military - watch out!
u8 ImperialShips[] =
{
	0x17, 0x19, 0x19, 0x30, 0x30, 0x2a, 0x2a, 0x2a,
	0x38, 0x38, 0x38, 0x29, 0x29, 0x2f, 0x2a, 0x2a,
};

// index of first package mission, all before it are passenger
// missions and all after it are package missions
u32 firstPackageMission = 16;

// New info table for new package/passenger missions.
// see below the table for calculation code
packageinfo_t PackageData[] =
{
	// harmless missions.
	{ 500, 0, 0, 0, RATING_HARMLESS },	// visiting a friend.
	{ 1500, 0, 0, 0, RATING_HARMLESS },	// business trip.
	{ 1200, 0, 0, 0, RATING_HARMLESS },	// travelling salesman.
	{ 1400, 0, 0, 0, RATING_HARMLESS },	// visiting sick relative.
	{ 1900, 0, 0, 0, RATING_HARMLESS },	// freelance journalist.
	{ 2800, 0, 0, 0, RATING_HARMLESS },	// corp. executive
	{ 2500, 0, 0, 0, RATING_HARMLESS },	// dream star

	// I'm a factory inspector making my rounds.
	{ 12000, 4, 6, MafiaShips, RATING_BELOWAVG },	// Factory Inspector
	// I'd rather someone didn't find me.
	{ 4000, 1, 1, IndyShips, RATING_POOR },
	// I owe someone money, and they're after me.
	{ 4500, 1, 2, IndyShips, RATING_POOR },
	// the police need help with their enquiries.
	{ 10000, 2, 6, PoliceShips, RATING_BELOWAVG },
	// an old rival is trying to kill me.
	{ 3000, 1, 0, IndyShips, RATING_POOR },
	// I think X Corporation has assassins on my trail.
	{ 40000, 8, 16, CorporateShips, RATING_COMPETENT },
	// I worked as a Federal spy...
	{ 72000, 16, 24, ImperialShips, RATING_DANGEROUS },
	// The Mafia want me dead.
	{ 18000, 8, 8, MafiaShips, RATING_AVERAGE },
	// The Federal Military is chasing me as they think I'm a spy.
	{ 72000, 24, 24, FederalShips, RATING_DANGEROUS },
	// **END PASSENGER MISSIONS**

	// **BEGIN PACKAGE DELIVERY MISSIONS**
	{ 2000, 0, 0, 0, RATING_HARMLESS },	// A friend needs components urgently.
	{ 1000, 0, 0, 0, RATING_HARMLESS },	// some old books for a friend.
	{ 1250, 0, 0, 0, RATING_HARMLESS },	// architectural plans.
	{ 750,  0, 0, 0, RATING_HARMLESS },	// Normal parcel.
	{ 450,	0, 0, 0, RATING_HARMLESS },	// Normal parcel.
	{ 1650, 0, 0, 0, RATING_HARMLESS },	// Corporate reports (not dangerous)
	{ 2250, 0, 0, 0, RATING_HARMLESS },	// latest dream card
	
	// tax inspection reports.
	{ 8000, 2, 4, MafiaShips, RATING_POOR },
	// some dangerous information cards.
	{ 4500, 1, 2, IndyShips, RATING_MOSTLYHARMLESS },
	// Normal parcel, friend pays on arrival.
	{ 2500, 0, 0, 0, RATING_HARMLESS },				
	// Documents proving police corruption.
	{ 8000, 2, 6, PoliceShips, RATING_POOR },
	// Secret diary being used for blackmail purposes.
	{ 4000, 1, 1, IndyShips, RATING_MOSTLYHARMLESS },
	// X Ind.'s research details.
	{ 34000, 8, 16, CorporateShips, RATING_AVERAGE },
	// Top-secret Imperial documents.
	{ 62000, 12, 20, ImperialShips, RATING_ABOVEAVG },
	// Listing of the members of the local mafia cell.
	{ 15000, 8, 12, MafiaShips, RATING_BELOWAVG },
	// Schematics for a new Federal battle weapon.
	{ 62000, 32, 24, FederalShips, RATING_ABOVEAVG },

	// Military Deliveries - only ship tables used
	{ 0, 0, 0, ImperialShips, RATING_HARMLESS },
	{ 0, 0, 0, FederalShips, RATING_HARMLESS },
};

float DangerPriceMults[] = 
	{1.0, 1.1, 1.2, 1.3, 1.4, 1.6, 1.7, 1.8, 1.9, 2.0};
float PopulPriceMults[] =
	{1.8, 1.7, 1.6, 1.5, 1.4, 1.3, 1.2, 1.1, 1.05, 1.0};
float DangerExponent[] =
	{4.0, 3.0, 2.0, 1.2, 0.8, 0.6, 0.5, 0.4, 0.3, 0.2};

// military mission cash is multiplied by this
// once Prince/Admiral is reached
float princeBonus = 1.5;

// exponential range limit for excellent photos
u8 excellentRange = 8;

// indexed by passenger number.
u16 passengerStrings[] =
{
	0x9918,	// passage for one
	0x9919,	// passage for two
	0x991a,	// passage for a small group
	0x991a, // passage for a small group
	0x991b,	// passage for a group
	0x991b,	// passage for a group
	0x991b,	// passage for a group
	0x991c, // a small package
};

extern "C" u32 GetPackageRankRequired(u32 mission_idx)
{
	return PackageData[mission_idx].rankRequirement;
}

extern "C" u32 GetPackageWeighting(u32 idx)
{
	float shipMean, mult, temp;

	shipMean = PackageData[idx].baseShips + (PackageData[idx].addShips / 2.0);

	mult = 1/(shipMean+1);

	// passenger missions appear more often
	if (idx < firstPackageMission)
		mult *= 2.0;

	// mafia/crime related missions appear more often in dangerous systems
	if (idx == 14)
	{
		temp = DATA_CurrentPirates + 1;

		mult *= (temp*temp) / 4.0;
	}
	else if (PackageData[idx].shipData == IndyShips)
		mult *= DATA_CurrentPirates;
	else // high-tech systems will have more corporate intrigue
	if (PackageData[idx].shipData == CorporateShips)
	{
		temp = (DATA_SystemTechLevel / 64.0);
		mult *= temp*temp;
	}
	else // more missions fleeing from the military in systems with said military
	if (PackageData[idx].shipData == FederalShips)
	{
		if (DATA_CurrentAllegiance == ALLY_FEDERAL)
			mult *= DATA_SystemMilitaryActivity/4.0;
		else if (DATA_CurrentAllegiance == ALLY_IMPERIAL)
			mult = 0.0;
	}
	else if (PackageData[idx].shipData == ImperialShips)
	{
		if (DATA_CurrentAllegiance == ALLY_IMPERIAL)
			mult *= 6.0;
		else if (DATA_CurrentAllegiance == ALLY_FEDERAL)
			mult = 0.0;
	}

	return (1000*mult);
}


// civvie package/passenger missions.
extern "C" u32 BBS_MakePassengerAd(starport_t *starport, bbsAdvert_t *slot)
{
	u8 *c1, mission_idx;
	s8 numPassengers;
	s32 cash, cashunit;
	u16 ships;
	s8 cashdigits;
	u32 basic_data;
	u32 d1, d2, d3, pirates, d5, traders, d7, government;
	u32 population, danger, c4, targetAllegiance;
	u32 targetSystem;
	u32 i, totalWeight, rand;
	float addCash, fTemp;

	packageinfo_t pInfo;

	targetSystem = GetNearbySystem(0);

	if (targetSystem == 0)
		return 0;

	// get info on the destination system.
	FUNC_000869_GetSystemData(targetSystem, &d1, &d2, &d3, &pirates, &d5, &traders, &d7, &government);
	FUNC_000870_GetSystemDataExt(targetSystem, &c1, &population, &danger, &c4);

	// no good, too small
	if (population < 3)
		return 0;

	targetAllegiance = government >> 6;

	// get total probability "weight"
	totalWeight = 0;
	for (i = 0; i < NUM_ELEMENTS(PackageData)-2; i++)
		totalWeight += GetPackageWeighting(i);

	rand = ((u32)DATA_RandomizerFunc() << 16) | DATA_RandomizerFunc();
	rand %= (totalWeight+1);
	totalWeight = 0;
	for (i = 0; i < NUM_ELEMENTS(PackageData)-2; i++)
	{
		totalWeight += GetPackageWeighting(i);
		if (totalWeight > rand)
			break;
	}

	mission_idx = i;
	if (mission_idx > 31)
		mission_idx = 31;

	if (mission_idx < 16)
	{
		numPassengers = DATA_RandomizerFunc() & 0x7;
		if (numPassengers >= 7)
			numPassengers = 6;
	}
	else
		numPassengers = 7;
	
	pInfo = PackageData[mission_idx];
	
	cash = pInfo.cash;

	ships = pInfo.baseShips;
	ships += DATA_RandomizerFunc() % (pInfo.addShips+1);

	if (ships > 0)
		cash *= 1 + sqrt(((ships - pInfo.baseShips) / (float)pInfo.baseShips));

	if (pInfo.baseShips > 0)
		addCash = pInfo.cash / sqrt(pInfo.baseShips);
	else
		addCash = pInfo.cash;

	if (mission_idx < firstPackageMission)
		cash += addCash * (sqrt(numPassengers+1) - 1);

	cash *= PopulPriceMults[DATA_CurrentPopulation];
	cash += 10.0*addCash*(sqrt(DangerPriceMults[pirates])*sqrt(DangerPriceMults[DATA_CurrentPirates]) - 1.0);
	
	// get range to system in LY
	fTemp = 0.01*GetStarRange(targetSystem);

	// pay more for long journeys
	if (fTemp > 16.0)
	{
		fTemp -= 16.0;
		cash += addCash*((fTemp*fTemp) / 121.0);
	}

	cashunit = sqrt(cash);
	cash += DATA_RandomizerFunc() % (cashunit+1);
	cash -= cashunit/2;
	
	cashdigits = (s8)(log(cash) / log(10)) - 2;

	cashunit = pow(10, cashdigits);

	if (cashunit < 10)
		cashunit = 10;

	cash = (cash / cashunit) * cashunit;
	 
	DATA_RandomizerFunc();

	basic_data = numPassengers & 0xff;
	basic_data |= (mission_idx & 0x7f) << 9;
	basic_data |= ((u32)ships & 0xffff) << 16;

	// hack
	if (cash < 0 || cash > 999999)
		return 0;

	CreateBBSAdvert(starport, slot, 0x981a, passengerStrings[numPassengers], targetSystem, basic_data, cash, DATA_RandSeed2);
	return 1;
}

// civvie assassinations.
extern "C" u32 BBS_MakeAssassinationAd(starport_t *starport, bbsAdvert_t *slot)
{
	u8 *c1, mission_idx;
	u32 cash, cashExtra, rand, date;
	u32 basic_data;
	u32 d1, d2, d3, pirates, d5, traders, d7, government;
	u32 population, danger, c4, targetAllegiance;
	u32 targetSystem;

	float temp;

	targetSystem = GetNearbySystem(0);

	if (targetSystem == 0)
		return 0;

	// get info on the destination system.
	FUNC_000869_GetSystemData(targetSystem, &d1, &d2, &d3, &pirates, &d5, &traders, &d7, &government);
	FUNC_000870_GetSystemDataExt(targetSystem, &c1, &population, &danger, &c4);
	targetAllegiance = government >> 6;

	// too small?
	if (population < 3)
		return 0;

	// missions typically more dangerous in dangerous systems
	temp = pow(FloatRandom(), DangerExponent[pirates]);
	mission_idx = (temp * 16.0);
	if (mission_idx >= 16)
		mission_idx = 15;

	date = DATA_GameDays - 0x121cf7;
	// 7-23 days in the future 
	date += 7 + (DATA_RandomizerFunc() & 0xf);

	cash = AssassinationData[mission_idx].cash;

	cash += (DATA_RandomizerFunc() % 32000);
	cash *= PopulPriceMults[DATA_CurrentPopulation];
	cash *= sqrt(DangerPriceMults[DATA_CurrentPirates]*DangerPriceMults[pirates]);

	cashExtra = cash - AssassinationData[mission_idx].cash;

	cashExtra = (cashExtra / 8000) * 8000;

	cash = AssassinationData[mission_idx].cash + cashExtra;

	rand = DATA_RandomizerFunc();

	// store extra data now
	basic_data = DATA_RandSeed1 & 0xffff;
	basic_data |= AssassinationData[mission_idx].shipIdx << 16;
	basic_data |= AssassinationData[mission_idx].ranking << 24;

	CreateBBSAdvert(starport, slot, 0x981f + (rand&0x3),
					date, targetSystem,
					0x99b4+AssassinationData[mission_idx].stringIdx,
					cash, basic_data);
	return 1;
}

extern "C" u32 MIL_MakePackage(u32 militaryRank)
{
	u32 cash, cashunit, cashdigits, date;
	u32 d1, d2, d3, pirates, d5, traders, d7, government;
	u8 *c1;
	u32 population, danger, c4;
	u32 missionAdditive, targetSystem, missionStr, rand;
	float fTemp;

	targetSystem = GetNearbySystem(-1);

	if (targetSystem == 0)
		return 0;

	FUNC_000869_GetSystemData(targetSystem, &d1, &d2, &d3, &pirates, &d5, &traders, &d7, &government);
	FUNC_000870_GetSystemDataExt(targetSystem, &c1, &population, &danger, &c4);
	
	if (population < 3)
		return 0;

	rand = DATA_RandomizerFunc() % 10;

	switch (rand)
	{
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
		missionStr = 0x996e;
		missionAdditive = BoundRandom(8);
		cash = 500;
		break;
	case 5:
	case 6:
	case 7:
		if (militaryRank < 16)
			return 0;

		missionStr = 0x9976;
		missionAdditive = BoundRandom(4);
		cash = 2000;
		break;
	case 8:
	case 9:
		if (militaryRank < 81)
			return 0;

		missionStr = 0x997a;
		missionAdditive = BoundRandom(4);
		cash = 6000;
		break;
	default:
		cash = 500;
	}

	date = DATA_GameDays + 10 + BoundRandom(14);

	cash *= 1 + (missionAdditive/6.0);
	
	cash *= PopulPriceMults[DATA_CurrentPopulation];
	cash *= sqrt(DangerPriceMults[DATA_CurrentPirates]);
	cash *= sqrt(DangerPriceMults[pirates]);

	fTemp = 0.01*GetStarRange(targetSystem);

	// pay more for long journeys, also give a bit more time
	if (fTemp > 16.0)
	{
		fTemp -= 16.0;
		cash += 0.25*cash*((fTemp*fTemp) / 121.0);
		
		// 7 extra days for each extra 20ly
		date += 0.35*fTemp;
	}

	if (militaryRank >= 25736)
		cash *= princeBonus;

	cashdigits = (s8)(log(cash) / log(10)) - 2;

	cashunit = pow(10, cashdigits);

	if (cashunit < 10)
		cashunit = 10;

	cash = (cash / cashunit) * cashunit;

	DATA_RandomizerFunc(); 
	CreateMilitaryEntry(0x9854, targetSystem, date, cash,
						missionStr + missionAdditive,
						DATA_RandSeed2);

	return 1;
}

extern "C" u32 MIL_MakeAssassination(u32 militaryRank)
{
	u16 baseName, nameAdditive, rand;
	u32 cashunit, cashdigits, range;
	u32 cash, date, name, targetSystem;
	
	rand = DATA_RandomizerFunc() & 0x3;
 
	// what level of mission?
	switch (rand)
	{
	case 0:
	case 1:
		if (militaryRank < 256)
			return 0;

		baseName = 0x99bc;
		nameAdditive = BoundRandom(3);
		cash = 15000 + nameAdditive*5000;
		break;
	case 2:
		if (militaryRank < 625)
			return 0;

		baseName = 0x99bf;
		nameAdditive = BoundRandom(3);
		cash = 40000 + nameAdditive*8000;
		break;
	case 3:
		if (militaryRank < 1296)
			return 0;

		baseName = 0x99c2;
		nameAdditive = BoundRandom(2);
		cash = 85000 + nameAdditive*20000;
		break;
	default: // should never happen
		cash = 100;
		baseName = 0x99bc;
		nameAdditive = 0;
	}

	targetSystem = GetNearbySystem(1);

	if (targetSystem == 0)	
		return 0;

	// better chances of appearing if the system is closer
	range = GetStarRange(targetSystem);
	if ((DATA_RandomizerFunc() & 0x1fff) < range)
		return 0;

	date = DATA_GameDays + 14 + BoundRandom(15);
	
	if (range > 1600)
		date += (range - 1600) / 285;

	name = baseName + nameAdditive;

	cash += 8000*(FloatRandom()-0.5);

	if (militaryRank >= 25736)
		cash *= princeBonus;

	cashdigits = (s8)(log(cash) / log(10)) - 2;

	cashunit = pow(10, cashdigits);

	if (cashunit < 10)
		cashunit = 10;

	cash = (cash / cashunit) * cashunit;

	DATA_RandomizerFunc();
	CreateMilitaryEntry(0x9857, targetSystem, date, cash,
						name, DATA_RandSeed2);
	return 1;
}

extern "C" u32 MIL_MakePhotography(u32 militaryRank)
{
	u16 baseName, nameAdditive, rand;
	u32 cashunit, cashdigits, range;
	u32 cash, date, name, targetSystem;

	date = DATA_GameDays + 30 + BoundRandom(43);
	
	rand = DATA_RandomizerFunc() % 9;

	switch (rand)
	{
	case 0:
	case 1:
	case 2:
	case 3:
		if (militaryRank < 2401)
			return 0;

		baseName = 0x9966;
		nameAdditive = BoundRandom(3);
		cash = 102000 + nameAdditive*12000;
		break;
	case 4:
	case 5:
	case 6:
		if (militaryRank < 4096)
			return 0;

		baseName = 0x9969;
		nameAdditive = BoundRandom(3);
		cash = 184000 + nameAdditive*15000;
		break;
	case 7:
	case 8:
		if (militaryRank < 6561)
			return 0;

		baseName = 0x996c;
		nameAdditive = BoundRandom(2);
		cash = 251000 + nameAdditive*25000;
		break;
	default: // should never happen
		cash = 100;
		baseName = 0x9966;
		nameAdditive = 0;
	}

	targetSystem = GetNearbySystem(1);
	
	if (targetSystem == 0)	
		return 0;

	// better chances of appearing if the system is closer
	range = GetStarRange(targetSystem);
	if ((DATA_RandomizerFunc() & 0x1fff) < range)
		return 0;

	if (range > 1600)
		date += (range - 1600) / 200;

	name = baseName + nameAdditive;

	cash += 20000*(FloatRandom()-0.5);

	if (militaryRank >= 25736)
		cash *= princeBonus;

	cashdigits = (s8)(log(cash) / log(10)) - 2;

	cashunit = pow(10, cashdigits);

	if (cashunit < 10)
		cashunit = 10;

	cash = (cash / cashunit) * cashunit;

	DATA_RandomizerFunc();
	CreateMilitaryEntry(0x9855, targetSystem, date, cash, name, DATA_RandSeed2);
	return 1;
}

extern "C" u32 MIL_MakeBombing(u32 militaryRank)
{
	u16 baseName, nameAdditive, rand;
	u32 cashunit, cashdigits, range;
	u32 cash, date, name, targetSystem;

	date = DATA_GameDays + 50 + BoundRandom(63);
	
	rand = DATA_RandomizerFunc() % 9;

	switch (rand)
	{
	case 0:
	case 1:
	case 2:
	case 3:
		if (militaryRank < 6561)
			return 0;

		baseName = 0x9966;
		nameAdditive = BoundRandom(3);
		cash = 225000 + nameAdditive*22000;
		break;
	case 4:
	case 5:
	case 6:
		if (militaryRank < 10000)
			return 0;

		baseName = 0x9969;
		nameAdditive = BoundRandom(3);
		cash = 355000 + nameAdditive*30000;
		break;
	case 7:
	case 8:
		if (militaryRank < 14641)
			return 0;

		baseName = 0x996c;
		nameAdditive = BoundRandom(2);
		cash = 565000 + nameAdditive*58000;
		break;
	default: // should never happen
		cash = 100;
		baseName = 0x9966;
		nameAdditive = 0;
	}

	targetSystem = GetNearbySystem(1);
	
	if (targetSystem == 0)	
		return 0;

	range = GetStarRange(targetSystem);
	if ((DATA_RandomizerFunc() & 0x1fff) < range)
		return 0;

	if (range > 1600)
		date += (range - 1600) / 150;

	name = baseName + nameAdditive;

	cash += 50000*(FloatRandom()-0.5);

	if (militaryRank >= 25736)
		cash *= princeBonus;

	cashdigits = (s8)(log(cash) / log(10)) - 2;

	cashunit = pow(10, cashdigits);

	if (cashunit < 10)
		cashunit = 10;

	cash = (cash / cashunit) * cashunit;

	DATA_RandomizerFunc();
	CreateMilitaryEntry(0x9856, targetSystem, date, cash, name, DATA_RandSeed2);

	return 1;
}

// gets data for a photo or bombing mission.
typedef struct 
{
	u32 totalShips;
	u32 maxShips;
	u32 shipFrequency;
} missionDifficulty_t;

// indexed by mission additive, photos then bombings.
missionDifficulty_t missionDiffs[] =
{
	{6, 3, 4},		// L1 Photo - Listening Station
	{7, 3, 4},		// L1 Photo - Research Station
	{8, 3, 4},		// L1 Photo - Interrogation centre
	{10, 4, 32},	// L2 Photo - Training Camp
	{12, 4, 32},	// L2 Photo - Weapons Factory
	{14, 4, 32},	// L2 Photo - Special forces base
	{20, 5, 64},	// L3 Photo - Bio-weapons facility
	{23, 5, 64},	// L3 Photo - Military installation

	{16, 4, 96},	// L1 Bombing - Listening Station
	{18, 4, 96},	// L1 Bombing - Research Station
	{20, 4, 96},	// L1 Bombing - Interrogation centre
	{24, 7, 140},	// L2 Bombing - Training Camp
	{26, 7, 140},	// L2 Bombing - Weapons Factory
	{28, 7, 140},	// L2 Bombing - Special forces base
	{40, 12, 255},	// L3 Bombing - Bio-weapons facility
	{46, 12, 255},	// L3 Bombing - Military installation
};	

extern "C" u32 GetNumFreeSlots()
{
	s32 numFreeSlots, i;
	u8 specialID, missionID, *mission;

	numFreeSlots = 0;
	for (i = 0x72; i >= 0x14; i--)
	{
		if (u8_AT(DATA_ObjectArray+i) == 0x0 || (u8_AT(DATA_ObjectArray+i) & 0x20))
			numFreeSlots++;
	}

	// military slots are "reserved"
	for (mission = DATA_ContractArray, i = 0; i < DATA_NumContracts; i++, mission += 52)
	{
		specialID = mission[4] & 0xf;

		if (specialID >= 7)
			continue;
		if (specialID < 3)
			continue;
		if (u32_AT(mission+6) != DATA_CurrentSystem)
			continue;
		
		missionID = mission[4] >> 4;

		// bombing?
		if (specialID > 4)
			missionID += 8;

		numFreeSlots -= missionDiffs[missionID].totalShips;
	}

	// reserve some basic slots for cargo, etc.
	numFreeSlots -= 20;
	
	if (numFreeSlots < 0)
		return 0;
	else
		return numFreeSlots;
}

char missionStrs[8][3] =
{
	"LS",
	"RS",
	"IC",
	"TC",
	"WF",
	"SF",
	"BW",
	"MI",
};

// called when a military base object is spawned.
extern "C" void MilitaryBaseInit(u8 *base, u8 *mission)
{
	u8 specialID, missionID;
	s32 randSeed, randSeed2, rnd;
	char *idStr;

	specialID = mission[4] & 0xf;
	missionID = mission[4] >> 4;

	// bombing?
	if (specialID > 4)
		missionID += 8;
	
	u16_AT(base+0xd8) = missionDiffs[missionID].totalShips;
	
	randSeed = u32_AT(base+0xa0);
	randSeed = randSeed2 = (randSeed << 0x4) | (randSeed >> 0x1c);

	rnd = DATA_FixedRandomFunc(0x10000, &randSeed, &randSeed2) % 1000;
	idStr = (char*)(base+0x124);

	sprintf(idStr, "+%s-%03d", missionStrs[mission[4] >> 4], rnd); 
}

extern "C" void SetBaseShipCounters()
{
	ModelInstance_t *obj;
	u8 specialID, missionID, *mission, i;

	// find photo/bombing mission and set base params.
	for (mission = DATA_ContractArray, i = 0; i < DATA_NumContracts; i++, mission += 52)
	{
		specialID = mission[4] & 0xf;

		if (specialID >= 7)
			continue;
		if (specialID < 3)
			continue;
		if (u32_AT(mission+6) != DATA_CurrentSystem)
			continue;

		obj = GetInstance(u8_AT(mission+0x1a), DATA_ObjectArray);

		if (obj != 0x0)
		{
			missionID = mission[4] >> 4;

			// bombing?
			if (specialID > 4)
				missionID += 8;

			u16_AT(obj+0xd8) = missionDiffs[missionID].totalShips;
		}
	}
}

// reset ship counter when you dock - no escape!
extern "C" void PlayerDocked()
{
	SetBaseShipCounters();
}

extern "C" void OnSystemInit()
{
	scriptSystem* scr = scriptSystem::getSingleton();

	if (scr->getFunction ("OnSystemInit_callback"))
	{
		scr->callFunction (0, 0);
		return;
	}

	ModelInstance_t *obj, *starport;

	DATA_LastAttackedIndex = 0;

	CreateShips();

	// destroy AI ships docked in locked starports
	for (u8 index = 1; index < 0x72; index++)
	{
		if (DATA_ObjectArray->state_flags[index] != IST_DOCKED)
			continue;

		if (index == DATA_PlayerIndex)
			continue;
		
		obj = GetInstance(index, DATA_ObjectArray);

		if (obj->ai_mode == AI_DOCKED_OR_LANDED)
		{
			starport = GetInstance(obj->dest_index, DATA_ObjectArray);

			if (IsStarportLocked(starport))
				FUNC_000924_DestroyObject(obj, 0);
		}
	} 

	return;
}

extern "C" void SpawnAssassins(u32 ships, u32 missionIdx, u32 name)
{
	u8 groupSize, numSpawned, *shipArray, shipIDByte;
	float maxGroup, fTemp;
	u32 numFreeSlots;

	maxGroup = sqrt(ships);
	shipArray = PackageData[missionIdx].shipData;

	if (shipArray == NULL)
		return;
	else if (shipArray == MafiaShips || shipArray == IndyShips)
		shipIDByte = 0xfb;	// bounties, cargo
	else
		shipIDByte = 0xf6;	// professionals, lawful

	numFreeSlots = GetNumFreeSlots();
	if (ships > numFreeSlots)
		ships = numFreeSlots;

	while (ships > 0)
	{
		fTemp = FloatRandom();
		fTemp *= maxGroup;
		groupSize = fTemp + 1.0;

		if (ships < groupSize)
			groupSize = ships;

		numSpawned = SpawnHostileGroup(groupSize, shipArray, name, 0x13, shipIDByte);
		if (numSpawned < groupSize)
			return;	// no more object handles

		ships -= numSpawned;
	}

}

// called every second by a "+rontier" type object
// returns 0 if a ship shouldn't be spawned, otherwise it
// returns a pointer to the appropriate ship table
extern "C" u8 *MilitaryBaseTick(ModelInstance_t *base)
{
	ModelInstance_t *obj;
	u8 maxShips, *shipArray, allegiance, specialID;
	u8 *mission, missionID;
	u16 *shipsLeft;
	s32 numFreeSlots, i, numChildShips;

	// too far away
//	if (base->dist > 0x26)
//		return 0;
	
	// find the photo/bombing mission linked to this base.
	for (mission = DATA_ContractArray, i = 0; i < DATA_NumContracts; i++, mission += 52)
	{
		specialID = mission[4] & 0xf;

		if (specialID >= 7)
			continue;
		if (specialID < 3)
			continue;
		if (u32_AT(mission+6) != DATA_CurrentSystem)
			continue;

		if (u8_AT(mission+0x1a) == base->index)
			break;
	}

	if (i >= DATA_NumContracts)
		return 0; // error, shouldn't happen

	missionID = mission[4] >> 4;

	// bombing?
	if (specialID > 4)
		missionID += 8;

	// no spawning while the player is docked
	if (DATA_PlayerState == 0x2a || DATA_PlayerState == 0x30)
		return 0;

	maxShips = (missionDiffs[missionID].maxShips & 0xf) + 1;
	shipsLeft = (u16*)(base+0xd8);

	if (maxShips > *shipsLeft)
		maxShips = *shipsLeft;

	allegiance = 1 + (specialID & 0x1);

	if (allegiance == ALLY_IMPERIAL)
		shipArray = FederalShips;
	else
		shipArray = ImperialShips;

	numFreeSlots = 0;
	numChildShips = 0;

	for (i = 0x72; i >= 0x14; i--)
	{
		if (u8_AT(DATA_ObjectArray+i) == 0x0 || (u8_AT(DATA_ObjectArray+i) & 0x20))
			numFreeSlots++;
		else if (u8_AT(DATA_ObjectArray+i) == 0x4f)
		{
			obj = GetInstance(i, DATA_ObjectArray);
			if (obj->object_type == OBJTYPE_ARMY)
				numChildShips++;
		}
	}

	if (maxShips <= numFreeSlots && maxShips > 0)
	{
		*shipsLeft -= SpawnHostileGroup(BoundRandom(maxShips) + 1, shipArray, 0, 0x13, OBJTYPE_MERCENARY);
	}

	// shouldn't spawn when too far away
	if (base->dist_cam >= 0x17)
		return 0;

	// go nuts...
	DATA_HostileTimer = 0;

	maxShips = missionDiffs[missionID].maxShips;
	// don't spawn unrestricted unless the player is really close
	if (base->dist_cam > 0x11 && numChildShips >= (maxShips*maxShips/2.0))
		return 0;

	if ((DATA_RandomizerFunc() & 0xff) < missionDiffs[missionID].shipFrequency)
		return shipArray;
	else
		return 0;
}

// called when nuclear missile destroys base.
// hurt all ships in radius, damage inversely proportional
// to the square of the distance
extern "C" void DoNukeDamage(ModelInstance_t *base)
{
	ModelInstance_t *obj;
	u8 i, expDist;
	float dist, temp, damage;

	for (i = 0; i < 0x72; i++)
	{
		if (u8_AT(DATA_ObjectArray+i) != 0 && !(u8_AT(DATA_ObjectArray+i) & 0x20))
		{
			if (i == DATA_PlayerIndex)
			{
				expDist = base->dist_cam;
				obj = DATA_PlayerObject;
			}
			else
			{
				obj = GetInstance(i, DATA_ObjectArray);
				expDist = abs((s16)base->dist_cam - (s16)obj->dist_cam);
			}

			if (expDist < 16 && obj->model_num >= 4 && obj->model_num <= 0x47)
			{
				temp = (base->rel_pos.x.full - obj->rel_pos.x.full) / KM_DIST;
				temp *= temp;
				dist = temp;

				temp = (base->rel_pos.y.full - obj->rel_pos.y.full) / KM_DIST;
				temp *= temp;
				dist += temp;

				temp = (base->rel_pos.z.full - obj->rel_pos.z.full) / KM_DIST;
				temp *= temp;
				dist += temp;

				// have square of distance in km
				if (dist > 1.0)
					damage = 1216000.0/dist;
				else
					damage = 1216000.0;

				if (damage >= 1.0)
					FUNC_000953_TakeDamage(obj, damage, 0, 0);
			}
		}
	}
}

u8 medalCodes[] =
{
	-1, // civ. assassination - no medal
	8,	// imp. assassination - Golden Spike
	2,	// fed. assassination - Purple Omega
	9,	// imp. photo - platinum cross, legion of honour
	3,	// fed. photo - vermillion crest, blue excelsior?
	11, // imp. bombing - celestial warrior
	5,	// fed. bombing - frontier medal
};

extern "C" u32 FinishMission(u8 *mission)
{
	u8 missionID, complete, allegiance;
	s32 pointGain;
	u32 baseSystem, missionTics, missionDays, string, cash, medal;
	u8 additive;
	u32 contractsAhead, strData[11], winData[11];

	missionID = u8_AT(mission+0x4) & 0xf;
	additive = u8_AT(mission+0x4) >> 4;
	complete = u8_AT(mission+0x5);
	baseSystem = u32_AT(mission+0x30);
	missionTics = u32_AT(mission+0xa);
	missionDays = u32_AT(mission+0xe);
	cash = u32_AT(mission+0x12) * 10;

	if (DATA_CurrentSystem != baseSystem)
		return 0;

	pointGain = 0;

	medal = medalCodes[missionID];

	// assassinations
	if (missionID <= 2)
	{
		allegiance = missionID;
		
		if (allegiance != ALLY_INDEPENDENT)
			switch (additive)
			{
			case 2:	// spying company official
				pointGain += 5;
			case 1:	// traitorous merchant
				pointGain += 5;
			case 0:	// double crossing spy
				pointGain += 60;
				break;
			case 5:	// disloyal company president
				pointGain += 8;
			case 4:	// mafia operative
				pointGain += 8;
			case 3:	// double agent and company director
				pointGain += 100;
				break;
			case 7:	// enemy senator
				pointGain += 20;
			case 6:	// enemy governor
				pointGain += 225;
			};

		if (complete == 0)
		{
			// is it incomplete while the target time has passed?
			if (DATA_GameDays > missionDays || (DATA_GameDays == missionDays && DATA_GameTics > missionTics))
			{
				pointGain *= -0.5;
				string = 0x9c29;
			}
			else
				return 0;
		}
		else
		{
			string = 0x9c2a;
			DATA_PlayerCash += cash;
		}
	}
	else if (missionID <= 4)	// photos
	{
		allegiance = missionID - 2;

		switch (additive)
		{
		case 2:	// interrogation centre
			pointGain += 20;
		case 1:	// research station
			pointGain += 20;
		case 0:	// listening station
			pointGain += 360;
			break;
		case 5:	// special forces base
			pointGain += 40;
		case 4:	// weapons factory
			pointGain += 40;
		case 3:	// training camp
			pointGain += 600;
			break;
		case 7:	// military installation
			pointGain += 80;
		case 6:	// bio-weapons facility
			pointGain += 1000;
		};

		// target time has passed?
		if (DATA_GameDays > missionDays || (DATA_GameDays == missionDays && DATA_GameTics > missionTics))
		{
			if (complete == 0)
			{
				pointGain *= -0.5;
				string = 0x9c29; // we're disappointed in your failure...
			}
			else
			{
				pointGain *= -0.1;
				string = 0x9c2c; // unfortunately you have returned too late.
			}
		}
		else
		{
			if (complete == 0)
				return 0; // not finished yet, but there's still time
			else // done!
			{
				DATA_PlayerCash += cash;
				
				if (u8_AT(mission+0x1b) > excellentRange)
				{
					pointGain *= 0.75;
					string = 0x9c2d; // The film is OK.
				}
				else
				{	
					string = 0x9c2e; // The film is excellent.
					medal++; // special medal for excellent photo
				}
			}
		}
	}
	else	// Bombings
	{
		allegiance = missionID - 4;

		switch (additive)
		{
		case 2:	// interrogation centre
			pointGain += 50;
		case 1:	// research station
			pointGain += 50;
		case 0:	// listening station
			pointGain += 800;
			break;
		case 5:	// special forces base
			pointGain += 100;
		case 4:	// weapons factory
			pointGain += 100;
		case 3:	// training camp
			pointGain += 1600;
			break;
		case 7:	// military installation
			pointGain += 500;
		case 6:	// bio-weapons facility
			pointGain += 4000;
		};

		// target time has passed?
		if (DATA_GameDays > missionDays || (DATA_GameDays == missionDays && DATA_GameTics > missionTics))
		{
			if (complete == 0)
			{
				pointGain *= -0.5;
				string = 0x9c29;
			}
			else
			{
				pointGain *= -0.1;
				string = 0x9c2c;
			}
		}
		else 
		{
			if (complete == 0)
				return 0; // not finished yet, but there's still time
			else // done!
			{
				DATA_PlayerCash += cash;
				string = 0x9c2a;
			}
		}
	}
	
	// have mission-specific data, now keep going.
	
	// remove this contract, shift the rest back
	DATA_NumContracts--;
	
	contractsAhead = DATA_NumContracts - (mission - DATA_ContractArray)/52;
	strData[7] = contractsAhead;

	if (contractsAhead > 0)
		memcpy(mission, mission+52, contractsAhead*52);

	strData[5] = cash;
	strData[6] = DATA_GameDays;
	strData[9] = string;
	
	// show congratulations message.
	FUNC_001344_StringExpandFFCode(DATA_009148_StringArray, 0x9c67+allegiance, (u8*)strData);
	winData[0] = strData[9];
	winData[1] = strData[5];
	winData[2] = strData[6];

	switch (allegiance)
	{
	case ALLY_IMPERIAL:
		winData[6] = 11;
		break;
	case ALLY_FEDERAL:
		winData[6] = 10;
		break;
	default:
		winData[6] = 0;
	};

	winData[7] = 0;

	// finally!
	FUNC_000349_ShowCommMessage((u8*)winData);

	// for civilian missions, we're finished
	if (allegiance == 0)
	{
		FUNC_000148_Unknown(0x19, 0x0);
		return 1;
	}
	
	// advance military rank/give medals
	if (allegiance == ALLY_FEDERAL)
		FUNC_000304_AddFederalRank(pointGain, (u8*)(strData+8), medal);
	else
		FUNC_000305_AddImperialRank(pointGain, (u8*)(strData+8), medal);

	FUNC_000148_Unknown(0x19, 0x0);
	return 1;
}

// called periodically when landed at a starport
// replaces F300, what a mess that was
extern "C" void FinishMissions()
{
	u8 i;

	// must be docked - doh!
	if (DATA_PlayerState != 0x2a && DATA_PlayerState != 0x30)
		return;

	for (i = 0; i < DATA_NumContracts; i++)
	{
		if (FinishMission(DATA_ContractArray+i*52))
			return;
	}
}
	










