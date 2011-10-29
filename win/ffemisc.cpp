
#include "ffe3d.h"

extern "C" void BlitClipWrapper (void *pData, int xpos,
		int ypos, int width, int height, int jump,
		void (*BlitFunc)(void *, int, int, int, int, int))
{
	int srcx, srcy, t;
	int scanw = width + jump;

	if (xpos >= 0) srcx = 0;
	else { srcx = -xpos; width += xpos; xpos = 0; }
	if (ypos >= 0) srcy = 0;
	else { srcy = -ypos; height += ypos; ypos = 0; }

	if ((t = xpos + width - 320) > 0) { width -= t; }
	if ((t = ypos + height - 200) > 0) { height -= t; }

	jump = scanw - width;
	pData = (void *)((char *)pData + srcy*scanw + srcx);

	BlitFunc (pData, xpos, ypos, width, height, jump);
}	

typedef struct
{
	double x, y, z;
} DVector;

typedef struct
{
	double x1, x2, x3;	// _11, _12, _13
	double y1, y2, y3;	// _21, _22, _23
	double z1, z2, z3;	// _31, _32, _33
} DMatrix;


extern "C" double VecDot (DVector *v1, DVector *v2)
{
	return (v1->x*v2->x + v1->y*v2->y + v1->z*v2->z);
}

extern "C" void VecCross (DVector *v1, DVector *v2, DVector *res)
{
	res->x = v1->y*v2->z - v1->z*v2->y;
	res->y = v1->z*v2->x - v1->x*v2->z;
	res->z = v1->x*v2->y - v1->y*v2->x;
}

extern "C" void MatTMatMul (DMatrix *m1, DMatrix *m2, DMatrix *res)
{
	res->x1 = m1->x1*m2->x1 + m1->y1*m2->y1 + m1->z1*m2->z1;
	res->x2 = m1->x1*m2->x2 + m1->y1*m2->y2 + m1->z1*m2->z2;
	res->x3 = m1->x1*m2->x3 + m1->y1*m2->y3 + m1->z1*m2->z3;

	res->y1 = m1->x2*m2->x1 + m1->y2*m2->y1 + m1->z2*m2->z1;
	res->y2 = m1->x2*m2->x2 + m1->y2*m2->y2 + m1->z2*m2->z2;
	res->y3 = m1->x2*m2->x3 + m1->y2*m2->y3 + m1->z2*m2->z3;

	res->z1 = m1->x3*m2->x1 + m1->y3*m2->y1 + m1->z3*m2->z1;
	res->z2 = m1->x3*m2->x2 + m1->y3*m2->y2 + m1->z3*m2->z2;
	res->z3 = m1->x3*m2->x3 + m1->y3*m2->y3 + m1->z3*m2->z3;
}

#define JJ_CPU3D_PI 3.141592654

extern "C" void BuildMatrix (double xr, double yr, double zr, DMatrix *m1)
{
	const double dtr = JJ_CPU3D_PI / 180.0;

	double sx = sin (xr*dtr), cx = cos (xr*dtr);
	double sy = sin (yr*dtr), cy = cos (yr*dtr);
	double sz = sin (zr*dtr), cz = cos (zr*dtr);

	m1->x1 = cz*cy + sz*sx*sy;
	m1->x2 = sz*cx;
	m1->x3 = -cz*sy + sz*sx*cy;
	m1->y1 = -sz*cy + cz*sx*sy; 
	m1->y2 = cz*cx;
	m1->y3 = sz*sy + cz*sx*cy;
	m1->z1 = cx*sy;
	m1->z2 = -sx;
	m1->z3 = cx*cy;
}

const double fix31_max = 0x7fff8000;
const double fix31_max_inv = 1.0 / 0x7fff8000;

extern "C" void ConvF31MatToFP (int *pIn, DMatrix *pOut)
{
	int i; for (i=0; i<9; i++) ((double *)pOut)[i] = pIn[i] * fix31_max_inv;
}
extern "C" void ConvF31VecToFP (int *pIn, DVector *pOut)
{
	int i; for (i=0; i<3; i++) ((double *)pOut)[i] = pIn[i] * fix31_max_inv;
}
extern "C" void ConvFPMatToF31 (DMatrix *pIn, int *pOut)
{
	int i; for (i=0; i<9; i++) pOut[i] = (int)(((double *)pIn)[i] * fix31_max);
}
extern "C" void ConvFPVecToF31 (DMatrix *pIn, int *pOut)
{
	int i; for (i=0; i<3; i++) pOut[i] = (int)(((double *)pIn)[i] * fix31_max);
}


extern "C" void rotateobject (int *pMat, int time, int *xrot, int *yrot, int *zrot)
{
	DMatrix imat, rmat, omat;
	double rmul;

	if (*xrot == 0 && *yrot == 0 && *zrot == 0) return;
//time is cap for ?rot values...
//also a cap of 9fb... or not? Shouldn't be necessary.

	ConvF31MatToFP (pMat, &imat);
	if (*xrot < -time) *xrot = -time; else if (*xrot > time) *xrot = time;
	if (*yrot < -time) *yrot = -time; else if (*yrot > time) *yrot = time;
	if (*zrot < -time) *zrot = -time; else if (*zrot > time) *zrot = time;

	rmul = -180.0 / 0x7fff;
	BuildMatrix (*xrot*rmul, *yrot*rmul, *zrot*rmul, &rmat);
	MatTMatMul (&rmat, &imat, &omat);
	ConvFPMatToF31 (&omat, pMat);
}


extern "C" void testhook (int *pMat)
{
	int i;
	double t1,t2,t3;
	DVector pVec[3];
	DVector pVecT[3];
	double junk = 1.0, junk2 = 1.0 / 0x7fff8000;

	for (i=0; i<9; i++) ((double *)pVec)[i] = pMat[i] / ((double)0x7fff8000);

	t1 = VecDot (pVec+0, pVec+0);
	t2 = VecDot (pVec+1, pVec+1);
	t3 = VecDot (pVec+2, pVec+2);

	t1 = VecDot (pVec+0, pVec+1);
	t2 = VecDot (pVec+1, pVec+2);
	t3 = VecDot (pVec+0, pVec+2);

	pVecT[0].x = pVec[0].x; pVecT[0].y = pVec[1].x;	pVecT[0].z = pVec[2].x;
	pVecT[1].x = pVec[0].y; pVecT[1].y = pVec[1].y;	pVecT[1].z = pVec[2].y;
	pVecT[2].x = pVec[0].z; pVecT[2].y = pVec[1].z;	pVecT[2].z = pVec[2].z;

	t1 = VecDot (pVecT+0, pVecT+0);
	t2 = VecDot (pVecT+1, pVecT+1);
	t3 = VecDot (pVecT+2, pVecT+2);

	t1 = VecDot (pVecT+0, pVecT+1);
	t2 = VecDot (pVecT+0, pVecT+2);
	t3 = VecDot (pVecT+1, pVecT+2);

}

extern "C" void testhook2 (int *pIn, double *pOut)
{
	int i, a;
	for (i=0; i<3; i++)
	{
		if (pOut[i] < (double)0x7fff8000
			&& pOut[i] > -(double)0x7fff8000) continue;
		a = 1;
	}
}

//// used to set breakpoints in ASM code.
//extern "C" void DebugBreak()
//{
//	return;
//}

// used to test strings in ASM code.
extern "C" void StringBreak(char* c)
{
	if (strstr(c, "Cegreeth D") != 0)
	{
		c = 0;
	}

	return;
}

extern "C" void MakeCargoString(ModelInstance_t *cargoObj)
{
	char *nameStr, newStr[40];
	char *inStr, oldStr[40];
	char amount[10];

	nameStr = (char*)&cargoObj->name;

	inStr = strchr(nameStr, '(');

	amount[0] = 0;
	if (cargoObj->cargo_space > 1)
		sprintf(amount, "(%d)", cargoObj->cargo_space);

	if (inStr)
	{
		inStr[0] = 0;	// null-terminate

		strcpy(oldStr, nameStr); // get the beginning

		sprintf(newStr, "%s%s", oldStr, amount);
	}
	else
		sprintf(newStr, "%s %s", nameStr, amount);

	newStr[19] = 0;
	strncpy(nameStr, newStr, 20);

}

// determines whether a bribe is successful.
extern "C" u32 CanBribe(u32 maxFineAmt, u32 fineAmt, u32 policeIntegrity)
{
	float bribeFactor, allowedFactor;

	bribeFactor = fineAmt/(float)maxFineAmt;

	// can bribe up to maxFineAmt, depending
	allowedFactor = 1 - sin(policeIntegrity * (PI / 52000.0));
	allowedFactor *= 0.9 + 0.2*FloatRandom();

	if (bribeFactor < allowedFactor)
		return 1;

	return 0;
}

extern "C" u32 IsStarportLocked(ModelInstance_t *starport)
{
	ModelInstance_t *base;
	u8 i, baseIdx, specialID;

	// find a photography mission for this system.
	for (i = 0; i < DATA_NumContracts; i++)
	{
		specialID = DATA_ContractArray[i*52+4] & 0xf;

		if (specialID >= 7)
			continue;
		if (specialID < 3)
			continue;
		if (u32_AT(DATA_ContractArray+(i*52)+6) != DATA_CurrentSystem)
			continue;

		baseIdx = DATA_ContractArray[i*52+0x1a];
		base = GetInstance(baseIdx, DATA_ObjectArray);

		// starports on the same planet as the base are locked down
		if (base->parent_index == starport->parent_index)
			return 1;
	}

	return 0;
}

extern "C" ModelInstance_t *IsCloseToStarport(u32 offenseIdx)
{
	ModelInstance_t *starportObj, *nearestStarport;
	s8 i;
	u8 starportIdx, dist, nearestDist;

	nearestStarport = 0;
	nearestDist = 255;

	for (i = (DATA_NumStarports-1); i >= 0; i--)
	{
		starportIdx = DATA_StarportArray[i].objectIdx;
		starportObj = GetInstance(starportIdx, DATA_ObjectArray);

		dist = starportObj->dist_cam;

		if (dist <= nearestDist && !IsStarportLocked(starportObj))
		{
			nearestDist = dist;
			nearestStarport = starportObj;
		}
	}

	if (nearestDist < 0x13)
	{
		// unlawful discharge/piracy charge only once each per system
		if (offenseIdx == 3)
		{
			if (DATA_PlayerFlags & 0x4)
				return 0;
		}
/*		else if (offenseIdx == 10)
		{
			if (DATA_PlayerFlags & 0x200)
				return 0;
		}*/
		
		return nearestStarport;
	}
	// unlawful radioactive dumping can get you in trouble anyway
	if (offenseIdx == 0x8 && DATA_LastJettisonedCargoIndex == 0x1d && BoundRandom(10 + DATA_CurrentPirates*5) == 0)
		return nearestStarport;

	return 0;
}

extern "C" u32 ModifyEquipmentPrice(u32 in)
{
	u32 mult;

	mult = 9 + 9 * ((0xff - DATA_SystemTechLevel) / 256.0);
	
	return in*mult;
}

// returns 0 if any missiles are in flight
// also disallows acceleration with enemy ships nearby
extern "C" u32 ShouldAllowAcceleration(u32 accel)
{
	s8 i;
	ModelInstance_t *obj;

	// OK to play or pause
	if (accel <= 0)
		return 1;

	for (i = 0x72; i >= 0; i--)
	{
		if (u8_AT(DATA_ObjectArray+i) == 0)
			continue;

		obj = GetInstance(i, DATA_ObjectArray);

		if (obj->ai_mode == AI_MISSILE || (obj->dist_cam <= 14 && obj->dest_index == DATA_PlayerIndex && obj->ai_mode > AI_BASIC && obj->ai_mode <= AI_PIRATE_PREPARE))
			return 0;
	}

	return 1;
}

u32 g_nearbySystems[NUM_SYSTEMS];
u32 g_systemWeights[NUM_SYSTEMS];
u32 g_systemWeightSum, g_opposingWeightSum, g_friendlyWeightSum;
s8 g_systemOpposing[NUM_SYSTEMS];

extern "C" u32 FindDestID(s16 sectX, s16 sectY, s16 sectDist)
{
	s16 destX, destY, numStars;
	u8 randStarnum;
	float randAngle;

	if (sectDist <= 0)
	{
		destX = sectX;
		destY = sectY;
	}
	else
	{
		randAngle = FloatRandom()*2*PI;
		destX = sectX + sectDist*cos(randAngle);
		destY = sectY + sectDist*sin(randAngle);
	}

	numStars = (FUNC_000853_GetNumstars(destX, destY, 0) >> 10);

	if (numStars == 0)
		return 0;

	randStarnum = DATA_RandomizerFunc() % numStars;
	return ((destX & 0x1fff) | (((u32)destY & 0x1fff) << 0xd) | (((u32)randStarnum & 0x3f) << 0x1a));
}

// fill a global array that can be used	throughout a loop
extern "C" void FillSystemData()
{
	u32 population, tourism, allegiance, destID;
	s16 sectX, sectY, sectDist;
	s16 i, j;
	u32 numSystems;
	u8 opposingAllegiance;

	sectX = DATA_CurrentSystem & 0x1fff;
	sectY = (DATA_CurrentSystem >> 0xd) & 0x1fff;

	g_systemWeightSum = g_opposingWeightSum = g_friendlyWeightSum = 0;	
	numSystems = 0;
	
	memset(g_nearbySystems, 0, sizeof(g_nearbySystems));
	memset(g_systemOpposing, 0, sizeof(g_systemOpposing));
	memset(g_systemWeights, 0, sizeof(g_systemWeights));

	if (DATA_CurrentAllegiance == ALLY_FEDERAL)
		opposingAllegiance = ALLY_IMPERIAL;
	else if (DATA_CurrentAllegiance == ALLY_IMPERIAL)
		opposingAllegiance = ALLY_FEDERAL;
	else
		opposingAllegiance = 0;

	for (i = 0; i < NUM_SYSTEMS; i++)
	{
		sectDist = i/8 + BoundRandom(3) - 1;

		destID = FindDestID(sectX, sectY, sectDist);

		if (destID == DATA_CurrentSystem || destID == 0)
			continue;
		
		// duplicates have 0 weight
		for (j = numSystems-1; j >= 0; j--)
		{
			if (destID == g_nearbySystems[j])
				break;
		}

		if (j >= 0)
			continue;	// duplicate

		// special time-saving signal by setting p2 to 0 (hacky but useful)
		FUNC_000870_GetSystemDataExt(destID, 0, &population, &tourism, &allegiance);
		if (population < 3)
			continue;

		g_nearbySystems[numSystems] = destID;
		if (sectDist <= 0)
			sectDist = 4;
		else
			sectDist += 4;

		g_systemWeightSum += g_systemWeights[numSystems] = (16000*(population + tourism)) / (sectDist*sectDist);
		
		if (opposingAllegiance != 0 && allegiance == opposingAllegiance)
		{
			g_opposingWeightSum += g_systemWeights[numSystems];
			g_systemOpposing[numSystems] = 1;
		}
		else if (allegiance == DATA_CurrentAllegiance)
		{
			g_friendlyWeightSum += g_systemWeights[numSystems];
			g_systemOpposing[numSystems] = -1;
		}

		numSystems++;
	}

	if (DATA_CurrentAllegiance != ALLY_FEDERAL && DATA_CurrentAllegiance != ALLY_IMPERIAL)
		return;

	sectX = CoreX[opposingAllegiance];
	sectY = CoreY[opposingAllegiance];

	// for military systems, find more opposing systems starting from the enemy core
	for (i = 0; i < NUM_SYSTEMS && numSystems < NUM_SYSTEMS; i++)
	{
		sectDist = i/16 + BoundRandom(3);

		destID = FindDestID(sectX, sectY, sectDist);

		if (destID == DATA_CurrentSystem || destID == 0)
			continue;
		
		// duplicates have 0 weight
		for (j = numSystems-1; j >= 0; j--)
		{
			if (destID == g_nearbySystems[j])
				break;
		}

		if (j >= 0)
			continue;	// duplicate

		// special time-saving signal by setting p2 to 0 (hacky but useful)
		FUNC_000870_GetSystemDataExt(destID, 0, &population, &tourism, &allegiance);
		
		if (allegiance != opposingAllegiance)
			continue;
		if (population < 3)
			continue;

		g_nearbySystems[numSystems] = destID;
		if (sectDist <= 0)
			sectDist = 4;
		else
			sectDist += 4;

		g_opposingWeightSum += g_systemWeights[numSystems] = 500 / sectDist;
		g_systemOpposing[numSystems] = 2;

		numSystems++;
	}
		
}

// called on hyperjump for new system
extern "C" void CreateSystemData(u32 systemID)
{
	u8 i, *stockFlagsPtr;
	u32 d1, d2, techLevel, pirates, policeLevel, traders, d7, government;
	u32 population, danger, c4;
	u32 b1, b2, milActivity, b4;

	// get and store info on the new system.
	FUNC_000869_GetSystemData(DATA_CurrentSystem, &d1, &d2, &techLevel, &pirates, &policeLevel, &traders, &d7, &government);
	FUNC_000870_GetSystemDataExt(DATA_CurrentSystem, &stockFlagsPtr, &population, &danger, &c4);
	FUNC_000871_GetSystemDataExt2(DATA_CurrentSystem, &b1, &b2, &milActivity, &b4, &DATA_MilRankSub, &DATA_MilRankBase);

	for (i = 0; i < 38; i++)
		DATA_StockFlags[i] = stockFlagsPtr[i];

	DATA_CurrentPopulation = population;
	DATA_CurrentDanger = danger;
	DATA_SystemTechLevel = techLevel;
	DATA_CurrentPirates = pirates;
	DATA_PoliceLevel = policeLevel;
	DATA_CurrentTraders = traders;
	
	DATA_SystemMilitaryActivity = milActivity + population + 7;

	DATA_CurrentAllegiance = government >> 6;
	
	FillSystemData();

	for (i = 0; i < DATA_NumStarports; i++)
	{
		CreateBBSData(DATA_StarportArray+i);
		CreateMarketData(DATA_StarportArray+i);
		CreateShipyardData(DATA_StarportArray+i);
		FUNC_000034_Unknown(0x10, DATA_StarportArray[i].objectIdx);
		FUNC_000034_Unknown(0x10, 0);
	}

	if (DATA_CurrentAllegiance == ALLY_FEDERAL || DATA_CurrentAllegiance == ALLY_IMPERIAL)
		CreateMilitaryData();
}

// called on every new day
extern "C" void RefreshSystemData()
{
	u8 i;

	FillSystemData();

	for (i = 0; i < DATA_NumStarports; i++)
	{
		RefreshBBSData(DATA_StarportArray+i);
		RefreshMarketData(DATA_StarportArray+i);
		RefreshShipyardData(DATA_StarportArray+i);
		FUNC_000034_Unknown(0x10, DATA_StarportArray[i].objectIdx);
		FUNC_000034_Unknown(0x10, 0);
	}

	if (DATA_CurrentAllegiance == ALLY_FEDERAL || DATA_CurrentAllegiance == ALLY_IMPERIAL)
		RefreshMilitaryData();

	RefreshShips();
}

float HostileSnapAccum = 0.0;

#define MAX_HOSTILE_DELAY 2863311530	// approx. 16 hours in gametics

extern "C" void SystemTick()
{
	u32 hostileSub;

	if (DATA_PlayerState == 0x2a || DATA_PlayerState == 0x30)
		return;

	HostileSnapAccum += DATA_FrameTime*(65536.0/MAX_HOSTILE_DELAY);
	hostileSub = (u32)HostileSnapAccum;

	if (hostileSub > DATA_HostileTimer)
	{
		DATA_HostileTimer = 0;
		HostileSnapAccum = 0.0;
	}
	else
	{
		DATA_HostileTimer -= hostileSub;
		HostileSnapAccum -= hostileSub;
	}
}

extern "C" u32 GetNearbySystem(s8 bGetOpposing)
{
	u32 rnd, i, weight;

	rnd = ((u32)DATA_RandomizerFunc() << 16) | DATA_RandomizerFunc();
	
	if (bGetOpposing == 1)
		rnd %= g_opposingWeightSum + 1;
	else if (bGetOpposing == -1)
		rnd %= g_friendlyWeightSum + 1;
	else
		rnd %= g_systemWeightSum + 1;

	weight = 0;
	for (i = 0; i < NUM_SYSTEMS && g_nearbySystems[i] != 0; i++)
	{
		if (g_systemOpposing[i] == 2)
		{
			if (bGetOpposing != 1)
				continue;
		}
		else if (bGetOpposing != 0 && bGetOpposing != g_systemOpposing[i])
			continue;

		weight += g_systemWeights[i];
		if (weight > rnd)
			return g_nearbySystems[i];
	}

	return 0;
}

ModelInstance_t *GetInstance(u32 index, InstanseList_t *list)
{
	return &list->instances[index];
	//return (ModelInstance_t*)((u32)list+sizeof(ModelInstance_t)*index+0x74);
}

Model_t *GetModel(u32 index) 
{
	return *(ffeModelList+index);
}