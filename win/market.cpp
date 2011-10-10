/*	Market.c - Stockmarket functions and data */

#include <math.h>
#include "misc.h"

#pragma warning ( disable: 4305 4244 )

#define VAR_FACTOR(i) (MarketData[i].priceVar/MarketData[i].basePrice)

typedef struct
{
	float basePrice;
	float priceVar;
	INT32 baseAvail;
	INT32 availVar;
} marketitem_t;

float DangerVarMults[] =
	{1.0, 1.2, 1.3, 1.5, 1.7, 2.0, 2.2, 2.4, 2.6, 2.8, 3.0};

float PopAvailMults[] =
	{0.25, 0.5, 1.0, 1.5, 2.5, 4.0, 7.0, 10.0, 14.0, 18.0};

float PopVarMults[] =
	{2.5, 2.3, 2.1, 1.8, 1.7, 1.6, 1.5, 1.3, 1.2, 1.0};

float SupplyVarMults[] = 
	{0.0, 0.1, 0.2, 0.3, 0.4, 0.6, 0.8, 1.0};

// temp. variance between starports
float IllegalStabilityFactor = 0.25;
float LegalStabilityFactor = 0.05;
float IllegalRandomnessMult = 2.25;

// perm. variance between planets
float PlanetVarianceFactor = 0.1;

// perm. variance between starports
float StarportVarianceFactor = 0.08;

float IllegalBonus = 4.5;

marketitem_t MarketData[] =
{
	{3.0, 3.0, 5000, 12000},	// Water
	{10.0, 8.0, 1500, 3500},	// Liquid Oxygen
	{40.0, 24.0, 800, 1200},	// Grain
	{80.0, 32.0, 500, 400},		// Fruit and Veg.
	{140.0, 42.0, 300, 200},	// Animal Meat
	{20.0, 14.0, 1500, 2500},	// Synthetic Meat
	{400.0, 50.0, 200, 37},		// Liquor
	{800.0, 120.0, 120, 50},	// Narcotics
	{550.0, 55.0, 0, 100},		// Medicines
	{15.0, 10.8, 2000, 2500},	// Fertilizer
	{804.8, 80.48, 50, 200},	// Animal Skins
	{950.0, 95.0, 60, 80},		// Live Animals
	{980.0, 161.7, 25, 100},	// Slaves
	{1400.0, 168.0, 80, 64},	// Luxury Goods
	{35.0, 21.0, 1600, 2400},	// Heavy Plastics
	{35.0, 14.0, 2500, 1000},	// Metal Alloys
	{1700.0, 170.0, 42, 160},	// Precious Metals
	{3000.0, 240.0, 12, 80},	// Gem Stones
	{12.0, 6.0, 5000, 2000},	// Minerals
	{40.0, 16.0, 1000, 500},	// Hydrogen Fuel
	{150.0, 37.5, 300, 450},	// Military Fuel
	{300.0, 90.0, 200, 100},	// Hand Weapons
	{600.0, 132.0, 20, 100},	// Battle Weapons
	{800.0, 480.0, 10, 30},		// Nerve Gas
	{74.0, 32.56, 500, 600},	// Industrial Parts
	{450.0, 81.0, 100, 100},	// Computers
	{200.0, 60.0, 120, 300},	// Air Processors
	{75.0, 30.0, 500, 400},		// Farm Machinery
	{780.0, 109.2, 50, 80},		// Robots
	{-24.0, -12.0, 600, 400},	// Radioactives
	{-6.5, -2.5, 2000, 1000},	// Rubbish
	{3000.0, 600.0, 0, 0},		// Alien Artefacts
	{82.0, 2.0, 100, 550},		// Chaff
};

INT32 PirateLevels[] = 
	{0, 3, 6, 10, 14, 20, 22, 25, 28, 35, 100, 100, 100};

extern "C" float GetPopulationMult(starport_t *starport)
{
	SINT32 randSeed, randSeed2, rand;
	ModelInstance_t *starportObj;

	if (DATA_NumStarports > 1)
	{
		starportObj = FUNC_001532_GetModelInstancePtr(starport->objectIdx, DATA_ObjectArray);
		randSeed = starportObj->globalvars.unique_Id;
	
		randSeed2 = randSeed = (randSeed << 0x12) | (randSeed >> 0xe);

		// pseudorandom effect, same all the time
		rand = DATA_FixedRandomFunc(0x10000, &randSeed, &randSeed2); 
		
		return (0.5 + (((rand >> 0x4) & 0xf) / 16.0));
	}
	else
		return 1.0;
}

extern "C" INT32 GetSupplyLevel(INT32 itemIdx, starport_t *starport)
{
	SINT32 randSeed, randSeed2, rand;
	ModelInstance_t *starportObj;

	if (DATA_NumStarports > 1)
	{
		starportObj = FUNC_001532_GetModelInstancePtr(starport->objectIdx, DATA_ObjectArray);
		randSeed = starportObj->globalvars.unique_Id;
	
		randSeed2 = randSeed = (randSeed << itemIdx) | (randSeed >> (32-itemIdx));

		// pseudorandom effect, same all the time
		rand = DATA_FixedRandomFunc(0x10000, &randSeed, &randSeed2); 
		return (rand & 0xf);
	}
	else
		return 0;
}

// returns price in eax (low dword), avail in edx (high dword)
extern "C" INT64 GetMarketItemData(INT32 itemIdx, INT32 population, INT32 danger, INT32 supply, INT32 flags, starport_t *starport, INT8 *portSupply)
{
	float price, avail, var, randVar, portPopMult;
	INT32 starportSupply;

	avail = MarketData[itemIdx].baseAvail;

	var = 0.5*PopVarMults[population]*DangerVarMults[danger]*SupplyVarMults[supply];
	if (flags & 0x8)	// export or import?
		var = -var;

	portPopMult = GetPopulationMult(starport);

	starportSupply = GetSupplyLevel(itemIdx, starport);
	randVar = StarportVarianceFactor*sqrt(PopVarMults[population]*DangerVarMults[danger])*SupplyVarMults[starportSupply & 0x7];
	
	if (starportSupply & 0x8)
		randVar = -randVar;

	if (flags & 0x80)
		randVar += (FloatRandom() - 0.5) * IllegalStabilityFactor;
	else
		randVar += (FloatRandom() - 0.5) * LegalStabilityFactor;
	
	if (flags & 0x80)
		randVar *= IllegalRandomnessMult;

	// more variance in smaller-pop. starports
	randVar /= portPopMult;

	starportSupply = portSupply[itemIdx];
	if (starportSupply & 0x8)
		randVar -= PlanetVarianceFactor*sqrt(PopVarMults[population]*DangerVarMults[danger])*SupplyVarMults[starportSupply & 0x7];
	else
		randVar += PlanetVarianceFactor*sqrt(PopVarMults[population]*DangerVarMults[danger])*SupplyVarMults[starportSupply & 0x7];

	var += randVar;

	avail -= var * MarketData[itemIdx].availVar;

	if (flags & 0x80)	// is it illegal?
		avail *= 0.25;

	avail *= PopAvailMults[population];

	// has availability, now get price
	price = MarketData[itemIdx].basePrice;
	
//	var = MarketData[itemIdx].varFactor * (2.0*var);

	if (var < 0)	// export or import?
		var = pow(0.5, -var*2.0);
	else
		var = pow(1.5, var*2.0);

	var--;

	price = MarketData[itemIdx].basePrice + MarketData[itemIdx].priceVar*var;

	if (flags & 0x80)
		price *= pow(IllegalBonus + 0.2f * (9 - population), VAR_FACTOR(itemIdx));

	avail /= sqrt((float)DATA_NumStarports*18.0);	// distribute among starports
	avail *= portPopMult;

	// make sure price stays at the same sign, just in case
	if ((price < 0) ^ (MarketData[itemIdx].basePrice < 0))
	{
		if (price < 0)
			price = -0.1;
		else
			price = 0.1;
	}

	return (INT64)(price*10) + ((INT64)avail << 32);
}

extern "C" INT32 ItemAtMaxStock(INT32 itemIdx, starport_t *starport)
{
	float maxAvail;
	INT32 stockAmt;

	stockAmt = starport->marketData[itemIdx].avail;

	maxAvail = MarketData[itemIdx].baseAvail;
	maxAvail += 0.5*PopVarMults[DATA_CurrentPopulation]*DangerVarMults[DATA_CurrentPirates]*MarketData[itemIdx].availVar;

	// only used for alien artefacts
	if (maxAvail == 0)
		maxAvail = 5;

	maxAvail *= PopAvailMults[DATA_CurrentPopulation];

	// traders in contraband accept 1/2 of the normal amount
	if (!(DATA_StockFlags[itemIdx] & 0x80))
		maxAvail *= 2.0;

	maxAvail /= sqrt((float)DATA_NumStarports*18.0);	// distribute among starports

	maxAvail *= GetPopulationMult(starport);

	if (maxAvail < 1)
		maxAvail = 1;
	return (stockAmt >= maxAvail);
}


extern "C" void SpawnPirates(INT32 pirateLevel, INT32 bInitial)
{
	float piratePct, maxGroup, groupSize;
	float playerCargoValue, avgPrice;
	INT8 numPirates, numSpawned;
	INT8 i, j;
	INT32 numFreeSlots;

	numFreeSlots = GetNumFreeSlots();

	// first, find out the value of the player's cargo.
	// this determines how many pirates will be after him
	playerCargoValue = 0;

	for (i = 0; i <= 32; i++)
	{
		if (DATA_NumStarports > 0)
		{
			// if the system has starports, find the item's
			// avg. price per tonne between them all
			// otherwise, just use the base price
			avgPrice = 0;
			for (j = 0; j < DATA_NumStarports; j++)
				avgPrice += ((DATA_StarportArray[j].marketData[i].price) / 10.0);
			
			avgPrice /= (float)DATA_NumStarports;

			playerCargoValue += avgPrice * DATA_PlayerCargo[i];
		}
		else
			playerCargoValue += MarketData[i].basePrice * DATA_PlayerCargo[i];
	}

	piratePct = 0.3 + 0.7*(1 - exp(-playerCargoValue/61000.0));
	piratePct *= 0.5 + FloatRandom();


	numPirates = PirateLevels[pirateLevel]*piratePct;

	// OK, now let's spawn the psychos
	maxGroup = sqrt((float)numPirates);

	if (!bInitial)
		numPirates *= 0.1;

	if (numPirates > numFreeSlots)
		numPirates = numFreeSlots;

	while (numPirates > 0)
	{
		groupSize = FloatRandom();
		groupSize *= maxGroup;	// mult by groupsize for fewer large groups
		groupSize += 1.0;

		if (numPirates < groupSize)
			groupSize = numPirates;

		numSpawned = SpawnHostileGroup(groupSize, 0, 0, 0xa, 0xfb);
		if (numSpawned < (INT8)groupSize)
			return;	// no more object handles

		numPirates -= numSpawned;
	}

	return; 
}

// more profitable goods are better here...
extern "C" INT32 GetCargoWeighting(INT32 idx, INT8 flags)
{
	if (!(flags & 0x8))	// don't buy imports
		return 0;
	if (flags & 0x80)	// don't buy illegals
		return 0;
	if (MarketData[idx].priceVar < 0)
		return 0;

	return (10*MarketData[idx].priceVar*SupplyVarMults[flags & 0x7]);
}

// fills an array with the cargo stored in this ship.
// *MUST RETURN THE SAME VALUE EVERY TIME*
extern "C" void GetCargoAmounts(ModelInstance_t *ship, INT32 *cargoAmounts)
{
	INT32 totalCargo;
	INT32 amountToSpawn, objToSpawn;
	INT32 i, j;
	INT32 cargoToSpawn;
	INT8 cargoType, driveType;
	float cargoMult;
	SINT32 totalWeight, rand, randSeed, randSeed2;
	INT32 d1, d2, d3, pirates, d5, traders, d7, government;
	INT32 population, danger, c4;
	INT8 *stockFlags;
	ShipDef_t *ship_def;

	if (ship == DATA_PlayerObject)
	{
		for (i = 0; i <= 32; i++)
			cargoAmounts[i] = DATA_PlayerCargo[i];
		
		return;
	}


	for (i = 0; i <= 32; i++)
		cargoAmounts[i] = 0;

	// missiles have no cargo
	if (ship->model_num <= 0xd)
		return;

	// police have nothing either
	if (ship->ai == 0xf4)
		return;

	ship_def = FUNC_001538_GetModelPtr(ship->model_num)->Shipdef_ptr;

	// preliminary: spawn fuel.
	driveType = ship_def->IntegralDrive;
	if (driveType == 0x80)
		driveType = ship_def->Drive;

	amountToSpawn = ship->cargo_free_space;

	// totalCargo = radioactives, amountToSpawn = fuel
	if (driveType >= 0xa)
	{
		totalCargo = GetInitialFuel(ship, driveType);
		if (totalCargo < amountToSpawn)
			totalCargo = 0;
		else
			totalCargo -= amountToSpawn;
	}
	else
		totalCargo = 0;

	// Hydrogen/Military FUEL	
	if (amountToSpawn > 0)
	{
		if (driveType >= 0xa)
			cargoAmounts[0x14] += amountToSpawn;
		else
			cargoAmounts[0x13] += amountToSpawn;
	}

	// Radioactives
	if (totalCargo > 0)	
		cargoAmounts[0x1d] += totalCargo;

	// "professionals" don't carry cargo besides fuel
	if (ship->ai >= 0xf4 && ship->ai <= 0xf7)
		return; 

	// get info on the last system docked at.
	FUNC_000869_GetSystemData(ship->destroyBonus, &d1, &d2, &d3, &pirates, &d5, &traders, &d7, &government);
	FUNC_000870_GetSystemDataExt(ship->destroyBonus, &stockFlags, &population, &danger, &c4);

	if (population < 3)
		return;

	// spawn real cargo now
	amountToSpawn = (ship->cargo_free_space & 0x7fff); // internal capacity
	
	totalCargo = 0;
	if (amountToSpawn > 0)
	{
		objToSpawn = sqrt((float)amountToSpawn) / 2;
		if (objToSpawn < 1)
			objToSpawn = 1;

		if (objToSpawn > 12)
			objToSpawn = 12;

		
		for (i = 0; i < objToSpawn && totalCargo < amountToSpawn; i++)
		{
			totalWeight = 0;

			for (j = 0; j <= 32; j++)
			{
				if (cargoAmounts[j] > 0)
					continue;

				totalWeight += GetCargoWeighting(j, stockFlags[j]);
			}

			if (totalWeight == 0)
				return;

			randSeed = ship->globalvars.unique_Id;
	
			randSeed2 = randSeed = (randSeed << i) | (randSeed >> (32-i));

			// pseudorandom effect, same all the time
			rand = DATA_FixedRandomFunc(0x10000, &randSeed, &randSeed2); 
			rand %= totalWeight;

			totalWeight = 0;
			for (j = 0; j <= 32; j++)
			{
				if (cargoAmounts[j] > 0)
					continue;

				totalWeight += GetCargoWeighting(j, stockFlags[j]);
				if (totalWeight > rand)
					break;
			}

			if (j > 32)
				j = 32;

			cargoType = j;
			randSeed = ship->globalvars.unique_Id;
	
			randSeed2 = randSeed = (randSeed << (32-i)) | (randSeed >> i);

			// pseudorandom effect, same all the time
			rand = DATA_FixedRandomFunc(0x10000, &randSeed, &randSeed2); 

			// rubbish - alien artefacts
			if (j >= 0x1c && j <= 0x1e)
				cargoToSpawn = rand & 0x7;
			else
			{
				randSeed = ship->globalvars.unique_Id;
	
				randSeed2 = randSeed = (randSeed << (32-i)) | (randSeed >> i);

				// pseudorandom effect, same all the time
				cargoMult = (rand&0x7fff) / 32768.0;

				cargoToSpawn = cargoMult*(MarketData[j].baseAvail + 0.5*PopVarMults[population]*DangerVarMults[danger]*SupplyVarMults[j]*MarketData[j].availVar);
				cargoToSpawn *= sqrt(PopAvailMults[population]) / 3.0;
			}

			if (cargoToSpawn < 1)
				cargoToSpawn = 1;

			totalCargo += cargoToSpawn;

			if (totalCargo > amountToSpawn)
			{
				cargoToSpawn += amountToSpawn - totalCargo;
				totalCargo = amountToSpawn;
			}

			cargoAmounts[cargoType] += cargoToSpawn;
		}
	}
}

// spawn cargo from a soon-to-be destroyed ship.
extern "C" void DoSpawnCargo(ModelInstance_t *ship)
{
	INT8 *cargo_obj;
	INT32 cargoAmounts[33], i;
	INT16 hullMass, hullArmorPieces;
	ShipDef_t *ship_def;

	GetCargoAmounts(ship, cargoAmounts);

	for (i = 0; i <= 32; i++)
	{
		if (cargoAmounts[i] > 0)
			cargo_obj = FUNC_000926_SpawnCargo(0x8e00 + DATA_JettisonedCargoTypes[i], ship, cargoAmounts[i]);
	}

	ship_def = FUNC_001538_GetModelPtr(ship->model_num)->Shipdef_ptr;
	hullMass = ship_def->Mass;

	hullArmorPieces = sqrt((float)hullMass) / 8;
	
	if (hullArmorPieces < 2)
		hullArmorPieces = 2;

	if (hullArmorPieces > 5)
		hullArmorPieces = 5;

	hullMass /= hullArmorPieces; // approx. mass of each chunk
	if (hullMass < 1)
		hullMass = 1;

	for (i = 0; i < hullArmorPieces; i++)
	{
		// spawn metal alloys
		cargo_obj = FUNC_000926_SpawnCargo(0x8e0f, ship, hullMass * (DATA_RandomizerFunc() / 65536.0) + 1);
	}

	return;
}

extern "C" INT32 ShouldCatchSmuggler()
{
	ModelInstance_t *starportObj;
	INT8 starportIdx;
	SINT32 randSeed, randSeed2, rand;

	starportIdx = DATA_PlayerObject->destinationIndex;
	starportObj = FUNC_001532_GetModelInstancePtr(starportIdx, DATA_ObjectArray);

	randSeed = starportObj->globalvars.unique_Id;
	
	randSeed2 = randSeed = (randSeed << 2) | (randSeed >> 30);

	// pseudorandom effect, same all the time
	rand = DATA_FixedRandomFunc(0x10000, &randSeed, &randSeed2) >> 3; 
	rand &= 3;

	if ((DATA_CurrentPirates + rand) < (DATA_RandomizerFunc() & 7))
		return 1;
	else
		return 0;
}
	
extern "C" void PushDoublePriceAds(INT32 itemIdx, INT32 demand, starport_t *starport, INT32 bInitial)
{
	INT8 idx, idx2;
	float maxAvail, fDemand, baseProb, curProb, price;
	float fRand;
	INT32 ads;

	fDemand = demand;

	maxAvail = MarketData[itemIdx].baseAvail + MarketData[itemIdx].availVar;
	maxAvail *= PopAvailMults[DATA_CurrentPopulation];

	// double-price more likely for illegal goods
	if (!(DATA_StockFlags[itemIdx] & 0x80))
		maxAvail *= 2.0;
	else
		maxAvail *= 0.25;

	maxAvail /= sqrt((float)DATA_NumStarports*18.0);	// distribute among starports

	maxAvail *= GetPopulationMult(starport);

	if (maxAvail > 0)
		fDemand /= maxAvail;

	fDemand = 2*sqrt(fDemand*VAR_FACTOR(itemIdx));

	// spawn a BBS ad for this?
	curProb = 1 - exp(-fDemand);

	if (!bInitial)
		curProb *= 0.1;

	baseProb = curProb;

	fRand = FloatRandom();

	ads = 0;
	while (curProb > fRand && ads < 18)
	{
		curProb *= baseProb;
		ads++;

		price = starport->marketData[itemIdx].price;
		
		price *= 1.5 + pow(FloatRandom(), fDemand*2.0);

		idx = DATA_RandomizerFunc() % 18;
		if (starport->adverts[idx].string != 0)
		{
			idx2 = GetEmptyAdSlot(starport);
			starport->adverts[idx2] = starport->adverts[idx];
			starport->adverts[idx].string = 0;
		}
			
		CreateBBSAdvert(starport,
						starport->adverts+idx, 
						0x982a,
						0x8e00 + itemIdx,
						price,
						1 + sqrt(fDemand * (MarketData[itemIdx].baseAvail + MarketData[itemIdx].availVar) * (0.2 + 0.3*FloatRandom()))*1.5,
						0,
						DATA_RandSeed2);
	}
}

extern "C" void KillDoublePriceAds(starport_t *starport)
{
	INT8 i, itemIdx;

	for (i = 0; i < 18 && starport->numAdverts > 0; i++)
	{
		if (starport->adverts[i].string == 0x982a)
		{
			itemIdx = starport->adverts[i].data1 - 0x8e00;
			
			// if the item is no longer out of stock, surely get rid of it
			if (starport->marketData[itemIdx].avail > 0 || FloatRandom() < 0.075)
			{
				DeleteBBSAdvert(i, starport);
				i--; // check again
			}
		}
	}
}

extern "C" SINT32 GetSqrDistFromCenter(INT32 id)
{
	INT32 distX, distY;

	distX = (id & 0x1fff) - 0x1718;
	distY = ((id >> 0xd) & 0x1fff) - 0x1524;
	return (distX*distX + distY*distY);
}

extern "C" float GetFrontierPriceMult(SINT32 sqrDistFromCenter, INT32 itemIdx)
{
	if (itemIdx == 31)
		return 1.0;

	if (sqrDistFromCenter > 25.0)
	{
		sqrDistFromCenter -= 25.0;

		if (itemIdx <= 18 && itemIdx >= 15)
			return (1 - 0.33*sqrDistFromCenter/(MAX_SECT_DIST*MAX_SECT_DIST));
		else
			return (1 + 2.0*sqrDistFromCenter/(MAX_SECT_DIST*MAX_SECT_DIST));
	}

	return 1.0;
}

extern "C" void CreateMarketData(starport_t *starport)
{
	SINT8 i;
	SINT32 avail, sqrDistFromCenter;
	ModelInstance_t *starportObj;
	INT64 returnVal;
	INT8 starportSupply[33];

	GetStarportSupply(starport, starportSupply);

	starportObj = FUNC_001532_GetModelInstancePtr(starport->objectIdx, DATA_ObjectArray);
	sqrDistFromCenter = GetSqrDistFromCenter(DATA_CurrentSystem) - 25;

	for (i = 0; i <= 32; i++)
	{
		returnVal = GetMarketItemData(i,
									  DATA_CurrentPopulation,
									  DATA_CurrentPirates,
									  DATA_StockFlags[i] & 0x7,
									  DATA_StockFlags[i],
									  starport, starportSupply);
		
		starport->marketData[i].price = returnVal & 0xffffffff;
		starport->marketData[i].price *= GetFrontierPriceMult(sqrDistFromCenter, i);

		avail = (returnVal >> 32);

		if (avail < 0)
		{
			PushDoublePriceAds(i, -avail, starport, 1);
			avail = 0;
		}

		starport->marketData[i].avail = avail;

		FUNC_000048_Unknown(0x18, INT32_AT(starport+0xa0), i+1000);
	}

	DATA_StarportRand = (INT32_AT(starport+0xa0) >> 16) | (INT32_AT(starport+0xa0) << 16);
	
	// decide whether this starport should have Bulk Carriers
	// floating around it (is it an orbiting station?)
	if (!(starportObj->flags_14C & 0x20))
		starport->flags |= 0x10;
	else
		starport->flags &= ~0x10;
}

extern "C" void RefreshMarketData(starport_t *starport)
{
	INT64 returnVal;
	SINT8 i;
	SINT32 price, avail, sqrDistFromCenter;
	SINT32 priceDiff, availDiff, availAdd;
	INT8 starportSupply[33];

	GetStarportSupply(starport, starportSupply);

	KillDoublePriceAds(starport);

	sqrDistFromCenter = GetSqrDistFromCenter(DATA_CurrentSystem);

	// get new data on market stuff, edge prices and supplies toward it
	for (i = 0; i <= 32; i++)
	{
		returnVal = 
			GetMarketItemData(i, DATA_CurrentPopulation,
							  DATA_CurrentPirates, 
							  DATA_StockFlags[i] & 0x7,
							  DATA_StockFlags[i], starport, starportSupply);
		
		price = returnVal & 0xffffffff;
		price *= GetFrontierPriceMult(sqrDistFromCenter, i);
		avail = returnVal >> 32;

		if (avail < 0)
		{
			if (starport->marketData[i].avail == 0)
				PushDoublePriceAds(i, -avail, starport, 0);

			avail = 0;
		}

		availDiff = avail - starport->marketData[i].avail;
		priceDiff = price - starport->marketData[i].price;

		availDiff *= 0.2 + 0.2*FloatRandom();
		priceDiff *= 0.2 + 0.2*FloatRandom();

		availAdd = avail * 0.1;

		if (availAdd > labs(availDiff))
			availAdd = availDiff;
		else if (availDiff < 0)
			availAdd = -availAdd;


		starport->marketData[i].avail += availAdd;
		starport->marketData[i].price += priceDiff;
		FUNC_000048_Unknown(0x18, INT32_AT(starport+0xa0), i+1000);
	}
}


extern "C" INT32 RadarCargoDisplay(ModelInstance_t *ship, INT8 *vars)
{
	INT32 ypos, i;
	INT32 cargoAmounts[33];
	
	GetCargoAmounts(ship, cargoAmounts);

	ypos = 0x47;

	for (i = 0; i <= 32; i++) 
	{
		if (cargoAmounts[i] > 0)
		{
			if (ypos < 0x79)
			{
				INT32_AT(vars+0x8) = cargoAmounts[i];
				INT32_AT(vars+0x4) = 0x8e00 + i;
			}
			else	// condense all the rest into 'other'
			{
				INT32_AT(vars+0x8) = 0;
				for (; i <= 32; i++)
					INT32_AT(vars+0x8) += cargoAmounts[i];
				INT32_AT(vars+0x4) = 0x99ee;
			}

			DATA_DrawStringWrapShadowFunc(0x99ed, vars, 0x5f, 0xeb, ypos, 0x0);
			ypos += 10;
		}
	}

	return ypos;
}
