#include "lua/lua.hpp"

lua_State* luaVM;

void LuaStart(void)
{
	luaVM = lua_open();
	luaL_openlibs(luaVM);
}

void LuaClose(void)
{
	lua_close(luaVM);
}

void SpawnPirates(int pirateLevel, int bInitial)
{/*
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
	maxGroup = sqrt(numPirates);

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
*/
	return; 
}
