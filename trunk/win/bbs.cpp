#include <math.h>
#include "misc.h"

// I may never understand the syntax for C function pointers...
typedef INT32 (*pBBSFunc)(starport_t*, bbsAdvert_t*);						  

// Possible BBS constructors, important ones are in missions.c
extern "C" INT32 BBS_MakeSmugglerAd(starport_t *starport, bbsAdvert_t *slot);
extern "C" INT32 BBS_MakeFakeSmugglerAd(starport_t *starport, bbsAdvert_t *slot);
extern "C" INT32 BBS_MakeMilitaryAd(starport_t *starport, bbsAdvert_t *slot);
extern "C" INT32 BBS_MakeMissingPersonAd(starport_t *starport, bbsAdvert_t *slot);
extern "C" INT32 BBS_MakePassengerAd(starport_t *starport, bbsAdvert_t *slot);
extern "C" INT32 BBS_MakeAssassinationAd(starport_t *starport, bbsAdvert_t *slot);
extern "C" INT32 BBS_MakeCrewAd(starport_t *starport, bbsAdvert_t *slot);
extern "C" INT32 BBS_MakeDonationAd(starport_t *starport, bbsAdvert_t *slot);

// constructors are pulled out of here at random
pBBSFunc BBSConstructors[] = 
{
	BBS_MakeSmugglerAd,
	BBS_MakeSmugglerAd,
	BBS_MakeSmugglerAd,
	BBS_MakeSmugglerAd,
	BBS_MakeSmugglerAd,
	
	BBS_MakeFakeSmugglerAd,
	
	BBS_MakeMilitaryAd,
	BBS_MakeMilitaryAd,
	BBS_MakeMilitaryAd,
	BBS_MakeMilitaryAd,
	
	BBS_MakePassengerAd,
	BBS_MakePassengerAd,
	BBS_MakePassengerAd,
	BBS_MakePassengerAd,
	BBS_MakePassengerAd,
	BBS_MakePassengerAd,
	BBS_MakePassengerAd,
	BBS_MakePassengerAd,
	BBS_MakePassengerAd,
	BBS_MakePassengerAd,
	BBS_MakePassengerAd,
	
	BBS_MakeAssassinationAd,
	
	BBS_MakeDonationAd,

	BBS_MakeCrewAd,
	BBS_MakeCrewAd,
};

extern "C" void CreateBBSAdvert(starport_t *starport, bbsAdvert_t *slot, INT32 string, INT32 d1, INT32 d2, INT32 d3, INT32 d4, INT32 d5)
{
	starport->numAdverts++;

	if (starport->numAdverts > 18)
		starport->numAdverts = 18;

	slot->string = string;
	slot->data1 = d1;
	slot->data2 = d2;
	slot->data3 = d3;
	slot->data4 = d4;
	slot->data5 = d5;
}

extern "C" INT32 BBS_MakeSmugglerAd(starport_t *starport, bbsAdvert_t *slot)
{
	ModelInstance_t *starportObj;
	INT8 i;
	INT32 randSeed, rand;

	if (starport->flags & 0x1)
		return 0;
	
	starport->flags |= 0x1;
	
	// are there any illegal goods to trade in?
	for (i = 0; i <= 32; i++)
	{
		if (DATA_StockFlags[i] & 0x80)
			break;
	}

	if (i > 32)
		return 0;

	// use pseudorandom name, same all the time
	starportObj = FUNC_001532_GetModelInstancePtr(starport->objectIdx, DATA_ObjectArray);
	randSeed = starportObj->globalvars.unique_Id;
	rand = (randSeed >> 0xf) | (randSeed << 0x11);

	CreateBBSAdvert(starport, slot, 0x9829,	0, 0, 0, rand, rand);
	return 1;
}

extern "C" INT32 BBS_MakeFakeSmugglerAd(starport_t *starport, bbsAdvert_t *slot)
{
	INT8 i;
	INT32 rand;

	if (starport->flags & 0x2)
		return 0;
	
	starport->flags |= 0x2;
	
	// are there any illegal goods to trade in?
	for (i = 0; i <= 32; i++)
	{
		if (DATA_StockFlags[i] & 0x80)
			break;
	}

	if (i > 32)
		return 0;


	// use random name, different each time
	DATA_RandomizerFunc();
	rand = DATA_RandSeed1;

	CreateBBSAdvert(starport, slot, 0x9829,	0xff, 0, 0, rand, rand);
	return 1;
}

extern "C" INT32 BBS_MakeMilitaryAd(starport_t *starport, bbsAdvert_t *slot)
{
	switch (DATA_CurrentAllegiance)
	{
	case ALLY_FEDERAL:
		if (starport->flags & 0x4)
			return 0;
		
		starport->flags |= 0x4;

		CreateBBSAdvert(starport, slot, 0x9827, 0, 0, 0, 0, 0);
		return 1;
	case ALLY_IMPERIAL:
		if (starport->flags & 0x8)
			return 0;
		
		starport->flags |= 0x8;

		CreateBBSAdvert(starport, slot, 0x9828, 0, 0, 0, 0, 0);
		return 1;
	default:
		return 0;
	}
}

extern "C" INT32 BBS_MakeMissingPersonAd(starport_t *starport, bbsAdvert_t *slot)
{
	INT32 targetSystem, stringID, cash, rand;
	float temp;

	DATA_RandomizerFunc();
	rand = DATA_RandSeed2;

	targetSystem = GetNearbySystem(0);
	if (targetSystem == 0)
		return 0;

	if (rand & 0x80000000)
		stringID = 0x981e;
	else
		stringID = 0x981d;

	temp = FloatRandom();
	temp *= temp;

	cash = 500 + ((INT32)(20*temp))*800;

	CreateBBSAdvert(starport, slot, stringID, 1, targetSystem, 0, cash, rand);
	return 1;
}

extern "C" INT32 BBS_MakeDonationAd(starport_t *starport, bbsAdvert_t *slot)
{
	DATA_RandomizerFunc();

	CreateBBSAdvert(starport, slot, 
					0x982c + ((DATA_RandSeed1 >> 0x10) & 0x1),
					0x99d7 + (DATA_RandSeed1 & 0x7),
					0, 0, DATA_RandSeed2, DATA_RandSeed1);
	return 1;
}

extern "C" INT32 BBS_MakeCrewAd(starport_t *starport, bbsAdvert_t *slot)
{
	DATA_RandomizerFunc();

	CreateBBSAdvert(starport, slot, 
					0x9823 + ((DATA_RandSeed2 >> 0x10) & 0x3),
					0, 0, 0, 0, DATA_RandSeed2);
	return 1;
}

extern "C" INT32 BBSCreateRandom(starport_t *starport, bbsAdvert_t *slot)
{
	INT32 idx;

	idx = DATA_RandomizerFunc() % (sizeof(BBSConstructors) / sizeof(BBSConstructors[0]));

	return BBSConstructors[idx](starport, slot);
}


extern "C" void DeflagAdvert(bbsAdvert_t *ad, starport_t *starport)
{
	// if you're killing off a military ad, watch out
	if (ad->string == 0x9827)
		starport->flags &= ~0x4;
	else if (ad->string == 0x9828)
		starport->flags &= ~0x8;
	else
	// illegal dealer ads
	if (ad->string == 0x9829)
	{
		if (ad->data1 == 0)
			starport->flags &= ~0x1;
		else
			starport->flags &= ~0x2;
	}
}

// deletes an ad at any index
extern "C" void DeleteBBSAdvert(INT32 idx, starport_t *starport)
{
	if (starport->adverts[idx].string == 0)
		return;

	DeflagAdvert(starport->adverts+idx, starport);

	starport->adverts[idx].string = 0;
	starport->numAdverts--;
}

extern "C" INT32 GetEmptyAdSlot(starport_t *starport)
{
	INT8 k;

	// search for empty slots
	for (k = 0; k < 18; k++)
	{
		if (starport->adverts[k].string == 0)
			break;
	}

	// no empty slots?
	if (k == 18)
	{
		k = DATA_RandomizerFunc() % 18;
		DeleteBBSAdvert(k, starport);
	}

	return k;
}

extern "C" void CreateBBSData(starport_t *starport)
{
	SINT32 numAds, i, j;
	
	numAds = DATA_CurrentPopulation + 9;
	if (numAds > 18)
		numAds = 18;

	for (i = 0; i < 18; i++)
		starport->adverts[i].string = 0;
	starport->flags &= ~0xf;

	starport->numAdverts = 0;
	for (i = numAds*5, j = 0; i > 0 && j < numAds; i--)
	{
		if (BBSCreateRandom(starport, starport->adverts+j))
			j++;
	}
}

// remove & replace ads
extern "C" void RefreshBBSData(starport_t *starport)
{
	SINT32 i, j, idx, numModify, temp;
	bbsAdvert_t prospectiveAd;

	numModify = (DATA_CurrentPopulation + 2) * (0.3 + 1.4*FloatRandom());

	for (i = numModify; i > 0; i--)
	{
		idx = DATA_RandomizerFunc() % 18;
		
		// double-price ads deleted by other mechanisms
		if (starport->adverts[idx].string != 0x982a)
			DeleteBBSAdvert(idx, starport);
	}

	temp = DATA_RandomizerFunc() & 0x7;
	if (temp >= 5)
		numModify += temp - 6;

	for (i = numModify*8, j = numModify; i > 0 && j > 0; i--)
	{
		if (BBSCreateRandom(starport, &prospectiveAd))
		{
			if (prospectiveAd.string == 0x9827 || prospectiveAd.string == 0x9828)
				starport->flags |= 0x4;
			else if (prospectiveAd.string == 0x9829)
			{
				if (prospectiveAd.data1 == 0)
					starport->flags |= 0x1;
				else
					starport->flags |= 0x2;
			}
			
			idx = GetEmptyAdSlot(starport);
			starport->adverts[idx] = prospectiveAd;

			j--;
		}

	}

}

typedef INT32 (*pMilFunc)(INT32);						  

extern "C" INT32 MIL_MakePackage(INT32 rank);
extern "C" INT32 MIL_MakeAssassination(INT32 rank);
extern "C" INT32 MIL_MakePhotography(INT32 rank);
extern "C" INT32 MIL_MakeBombing(INT32 rank);

pMilFunc MilConstructors[] =
{
	MIL_MakePackage,
	MIL_MakePackage,
	MIL_MakePackage,
	MIL_MakePackage,
	MIL_MakePackage,

	MIL_MakeAssassination,
	MIL_MakeAssassination,
	MIL_MakeAssassination,
	
	MIL_MakePhotography,
	MIL_MakePhotography,
	
	MIL_MakeBombing,
};

extern "C" void CreateMilitaryEntry(INT32 string, INT32 data1, INT32 data2, INT32 data3, INT32 data4, INT32 data5)
{
	INT8 i;

	for (i = 0; i < 26; i++)
	{
		if (DATA_MilitaryMissions[i].string == 0)
			break;
	}

	if (i >= 26)
		i = DATA_RandomizerFunc() % 26;

	DATA_MilitaryMissions[i].string = string;
	DATA_MilitaryMissions[i].data1 = data1;
	DATA_MilitaryMissions[i].data2 = data2;
	DATA_MilitaryMissions[i].data4 = data3;
	DATA_MilitaryMissions[i].data3 = data4;
	DATA_MilitaryMissions[i].data5 = data5;

//	DATA_MilitaryMissions[i+1].string = 0;
}

extern "C" INT32 MilCreateRandom()
{
	INT8 i;
	SINT32 milRank;

	for (i = 0; i < 26; i++)
	{
		if (DATA_MilitaryMissions[i].string == 0)
			break;
	}

	// no slots?
	if (i >= 26)
		return 0;

	// Systems with higher activity get more high-level missions
//	i = (float)NUM_ELEMENTS(MilConstructors)*0.99*pow(FloatRandom(), 3.0/(float)DATA_SystemMilitaryActivity);
	
	i = DATA_RandomizerFunc() % NUM_ELEMENTS(MilConstructors);
	milRank = DATA_MilRankBase - DATA_MilRankSub;
	if (milRank < 0)
		milRank = 0;

	return MilConstructors[i](milRank);
}
	
extern "C" void CreateMilitaryData()
{
	SINT32 i, j;
	INT32 b1, b2, milActivity, numMissions, b4;
	
	FUNC_000871_GetSystemDataExt2(DATA_CurrentSystem, &b1, &b2, &milActivity, &b4, &DATA_MilRankSub, &DATA_MilRankBase);

	// clear 'em
	for (i = 0; i < 26; i++)
		DATA_MilitaryMissions[i].string = 0;

	numMissions = DATA_SystemMilitaryActivity;
	for (i = numMissions*5, j = numMissions; i > 0 && j > 0; i--)
	{
		if (MilCreateRandom())
			j--;
	}
}

extern "C" void RefreshMilitaryData()
{
	SINT32 i, j, idx, numModify, temp;
	
	numModify = (DATA_SystemMilitaryActivity/4 + 2) * (0.5+FloatRandom());

	for (i = numModify; i > 0; i--)
	{
		idx = DATA_RandomizerFunc() % DATA_SystemMilitaryActivity;
		DATA_MilitaryMissions[idx].string = 0;			
	}

	temp = DATA_RandomizerFunc() & 0x7;
	if (temp >= 5)
		numModify += temp - 6;

	for (i = numModify*8, j = numModify; i > 0 && j > 0; i--)
	{
		if (MilCreateRandom())
			j--;
	}
	
}
	

