/* Common definitions used in new source */

#pragma warning ( disable: 4244 4133 4305 4761 )

#define log(x) log(float(x))
#define pow(x,y) pow(float(x),float(y))
#define sqrt(x) sqrt(float(x))

typedef enum
{
	RATING_HARMLESS = 0,
	RATING_MOSTLYHARMLESS,
	RATING_POOR,
	RATING_BELOWAVG,
	RATING_AVERAGE,
	RATING_ABOVEAVG,
	RATING_COMPETENT,
	RATING_DANGEROUS,
	RATING_DEADLY,
	RATING_ELITE,
} eliteranking_t;


typedef struct
{
	u32 price;
	u32 pricevar;
	u32 stock;
	u32 stockvar;
} stockitem_t;

typedef struct
{
	u32 id;
	u32 weight;
} laserdata_t;

typedef struct
{
	u32 string;
	u32 data1;
	u32 data2;
	u32 data3;
	u32 data4;
	u32 data5; // ?
} militaryMission_t;

typedef struct
{
	u32 string;
	u32 data1;
	u32 data2;
	u32 data3;
	u32 data4;
	u32 data5;
} bbsAdvert_t;

typedef struct
{
	s32 avail;
	s32 price;
} stockinfo_t;

typedef struct
{
	u32 randseed1;	// from [D8956] - position dependent
	u32 randseed2;	// from [D8957] - also total mass?
	u32 numstars;	// from b[D8954] & 7
	u32 systemcode;	// from [D8950]
	u32 techLevel;	// from [D8962]
	char pName[20];	// from D8656
	u8 junk[16];
} sysGenParams_t;

typedef enum 
{
	ST_INDUSTRY = 0,
	ST_GREEN,
	ST_JUNGLE,
	ST_ICE,
	ST_DESERT,
	ST_MAX
} worldType_t;

typedef enum 
{
	GOV_NONE = 0,
	GOV_ANARCHY,
	GOV_FUEDAL,
	GOV_LOCALCORP,
	GOV_CORPSTATE,
	GOV_FEDERAL_DEMOCRACY,
	GOV_FEDERAL_COLONY,
	GOV_DICTATORSHIP,
	GOV_COMMUNIST,
	GOV_RELIGIOUS,
	GOV_DEMOCRACY,
	GOV_IMPERIAL_RULE,
	GOV_IMPERIAL_COLONY,
	GOV_NONE_STABLE,
	GOV_MARTIAL_LAW,
	GOV_ALLIANCE_DEMOCRACY,
} government_t;

typedef enum 
{
	EF_MINING = 0,
	EF_INDUSTRY,
	EF_WET,
	EF_AGRICULTURE,
	EF_TOURISM,
	EF_LIFE,
	EF_CRIME,
	EF_MAX,
} effectType_t;

typedef struct
{
	s32 effects[EF_MAX];
	u16 distFromCenter;
	worldType_t primaryWorld;	
	u32 development;
} sysType_t;

#define NUM_SYSTEMS 64

extern u32 g_nearbySystems[NUM_SYSTEMS];
extern u32 g_systemWeights[NUM_SYSTEMS];
extern u8 g_systemAllegiances[NUM_SYSTEMS];

// Thanks to John Jordan for this one...
#ifndef __GNUC__
#pragma pack(push)
#pragma pack(1)
typedef struct equipdata_s
{
	u32 buyCost;
	u32 sellCost;
	u32 weight;
	u32 three;
	u8 type;
	u8 eighty;
	u8 id;
	u8 techLevel;
	u8 something;
	u8 zero;
} equipdata_t;

typedef struct
{
	stockinfo_t marketData[38];
	u8 objectIdx;
	u8 flags;
	bbsAdvert_t adverts[18];
	u32 numAdverts;
	u32 shipyard[14];
	u32 numShips;
} starport_t;

typedef struct
{
	u32 cash;
	u8  passengers;
	u8  mission_idx;
	u16 ships;
	u32 system;
	u32 date;
	u32 name;
} packageEntry_t;

typedef struct
{
	u32 mass;			// in earth masses
	u32 randseed;
	u16 parentindex;
	u16 objType;		// 0x23 for starports, < 0x20 for others...
	u16 modelindex;
	u32 orbradius;		// something orbital-distance related
	u16 latitude;		// angle related
	u32 orbvel;			// units unknown
	u32 tempptr;		// points to PhysObj
	u32 lightoutput;	// not used in PhysObj
	u16 unknown;
	u16 longitude;		// angle related
	u16 temperature;		// in kelvin
	char pName[20];
	u16 unknown2;
	u16 unknown3;
	u8 rotspeed;
	u8 treelevel;		// copied to 0xcb of PhysObj
	u16 unknown4;
	u16 orientation;		// set to zero though.
} systemObject_t;

#pragma pack(pop)
#else
typedef struct equipdata_s
{
	u32 buyCost __attribute__ ((packed));
	u32 sellCost __attribute__ ((packed));
	u32 weight __attribute__ ((packed));
	u32 three __attribute__ ((packed));
	u8 type __attribute__ ((packed));
	u8 eighty __attribute__ ((packed));
	u8 id __attribute__ ((packed));
	u8 techLevel __attribute__ ((packed));
	u8 something __attribute__ ((packed));
	u8 zero __attribute__ ((packed));
} equipdata_t;

typedef struct
{
	stockinfo_t marketData[38] __attribute__ ((packed));
	u8 objectIdx __attribute__ ((packed));
	u8 flags __attribute__ ((packed));
	bbsAdvert_t adverts[18] __attribute__ ((packed));
	u32 numAdverts __attribute__ ((packed));
	u32 shipyard[14] __attribute__ ((packed));
	u32 numShips;
} starport_t;

typedef struct
{
	u32 cash __attribute__ ((packed));
	u8  passengers __attribute__ ((packed));
	u8  mission_idx __attribute__ ((packed));
	u16 ships __attribute__ ((packed));
	u32 system __attribute__ ((packed));
	u32 date __attribute__ ((packed));
	u32 name __attribute__ ((packed));
} packageEntry_t;

typedef struct
{
	u32 mass __attribute__ ((packed));			// in earth masses
	u32 randseed __attribute__ ((packed));
	u16 parentindex __attribute__ ((packed));
	u16 objType __attribute__ ((packed));		// 0x23 for starports, < 0x20 for others...
	u16 modelindex __attribute__ ((packed));
	u32 orbradius __attribute__ ((packed));		// something orbital-distance related
	u16 latitude __attribute__ ((packed));		// angle related
	u32 orbvel __attribute__ ((packed));			// units unknown
	u32 tempptr __attribute__ ((packed));		// points to PhysObj
	u32 lightoutput __attribute__ ((packed));	// not used in PhysObj
	u16 unknown __attribute__ ((packed));
	u16 longitude __attribute__ ((packed));		// angle related
	u16 temperature __attribute__ ((packed));		// in kelvin
	char pName[20] __attribute__ ((packed));
	u16 unknown2 __attribute__ ((packed));
	u16 unknown3 __attribute__ ((packed));
	u8 rotspeed __attribute__ ((packed));
	u8 treelevel __attribute__ ((packed));		// copied to 0xcb of PhysObj
	u16 unknown4 __attribute__ ((packed));
	u16 orientation __attribute__ ((packed));		// set to zero though.
} systemObject_t;

#endif

// maximum distance out in sectors (squared)
// we can find civilized systems up to 30 sector lengths out
#define MAX_SECT_DIST 30.0

#define PI 3.14159265359

// 1 kilometer in game units (approx)
#define KM_DIST  999539.7368756

#define ALLY_INDEPENDENT 0
#define ALLY_IMPERIAL 1
#define ALLY_FEDERAL 2
#define ALLY_ALLIANCE 3

// ugly pointer definitions.
#define VEC32_AT(p) (*((Vec32*)(p)))
#define u64_AT(p) (*((u64*)(p)))
#define s64_AT(p) (*((s64*)(p)))
#define u32_AT(p) (*(u32*)(p))
#define s32_AT(p) (*((s32*)(p)))
#define u16_AT(p) (*((u16*)(p)))
#define s16_AT(p) (*((s16*)(p)))
#define u8_AT(p) (*((u8*)(p)))
#define s8_AT(p) (*((s8*)(p)))

#define VOIDPTR_AT(p) (*((void**)(p)))
#define u16_PTR(p) ((u16*)(p))

#define NUM_ELEMENTS(a) (sizeof(a)/sizeof(a[0]))

extern "C" u32 RandomB2S(u32 max, u32 *seed1, u32 *seed2);

extern "C" u8 DATA_NameThingyByteTable[];
extern "C" u32 DATA_GLobalEventCounter;
extern "C" InstanseList_t* DATA_ObjectArray;
//extern WingmanList_t WingmanArray;
extern "C" Model_t **ffeModelList;
extern "C" u32 (*DATA_RandomizerFunc)();	// randomizer
extern "C" ModelInstance_t* DATA_PlayerObject;
extern "C" stockitem_t DATA_StockData[0x20];
extern "C" u8 DATA_CargoFromShip[0x20];
extern "C" u32 DATA_ShipLaserCapacity[];
extern "C" laserdata_t DATA_AILasers[0x9];
extern "C" u32 DATA_DriveCargoFuel2[];
extern "C" u32 DATA_DriveCargoFuel[];
extern "C" u32 DATA_DriveTankFuel[];
extern "C" u32 DATA_DriveMasses[];
extern "C" u32 DATA_ECM_Weight;
extern "C" u32 DATA_NECM_Weight;
extern "C" u8 DATA_AIMissiles[0x10];
extern "C" u8 DATA_FederalShips[16];
extern "C" u8 DATA_ImperialShips[16];
extern "C" u16 DATA_CurrentAllegiance;
extern "C" u8 DATA_PlayerIndex;
extern "C" equipdata_t DATA_EquipmentData[64];
extern "C" u16 DATA_DriveFuelConsumption[];
extern "C" u32 DATA_FrameTime;
extern "C" u32 DATA_008886_Unknown;
extern "C" u16 DATA_PlayerCargo[32];
extern "C" u32 DATA_PlayerCargoSpace;
extern "C" u8  DATA_LastJettisonedCargoIndex;
extern "C" u8  DATA_ExtendedUniverse;
//extern "C" void* (*DATA_GetStaticDataFunc)(int modelidx);	// finds ship static data
extern "C" u32* (*DATA_HasEquipmentFunc)(int unknown, u8 *equip);	// finds ship static data
extern "C" u32 (*DATA_FixedRandomFunc)(int num, int *seed1, int *seed2); 
extern "C" void (*DATA_DrawStringWrapShadowFunc)(u32 ffcode, void *vars, u32 col, u32 xpos, u32 ypos, u32 wrapWidth);

extern "C" void FUNC_001907_SoundPlaySampleLogVol(u32 soundidx, u32 logVol);
extern "C" void FUNC_001908_SoundPlaySampleLinVol(u32 soundidx, u32 volume);		

extern "C" u32 FUNC_000952_DestroyEquip(ModelInstance_t *ship);
extern "C" u32 FUNC_000953_TakeDamage(ModelInstance_t *victim, u32 damage, u8 inflictorIdx, u32 bContinuous);

extern "C" void FUNC_000148_Unknown(u32, u32);
extern "C" void FUNC_000048_BeginEvents(u32, u32, u32);
extern "C" void FUNC_000034_Unknown(u32, u32);
extern "C" u32 FUNC_000035_GetSpecialShips(u32);

extern ModelInstance_t* GetInstance(u32 index, InstanseList_t *list);
extern Model_t *GetModel(u32 index);

// this function is unreliable, replace
//extern u16 (*DATA_BoundRandom)(u16 max);	// returns a number between 0 and max
#define BoundRandom(m) (m == 0 ? 0 : (DATA_RandomizerFunc() % m))

extern "C" ShipsLoyalityTable DATA_ShipsLoyalityTable[];
extern "C" ModelInstance_t DATA_DummyInstance;
extern "C" u16 DATA_CurrentAllegiance;
extern "C" u8 *DATA_CustomShipList;
extern "C" u32 DATA_CustomShipIndex;
extern "C" starport_t DATA_StarportArray[18]; // 18 starport slots
extern "C" u32 DATA_NumStarports;
extern "C" u16 DATA_CurrentPopulation;
extern "C" u8 DATA_CurrentPirates;
extern "C" u32 DATA_GameTics;
extern "C" u32 DATA_GameDays;
extern "C" u32 DATA_PlayerSpaceAvail;
extern "C" s32 DATA_NumContracts;
extern "C" u8  DATA_ContractArray[312];
extern "C" u32 DATA_CurrentSystem;
extern "C" u32 DATA_NumObjects;
extern "C" u64 DATA_GroupingVector[];
extern "C" u8  DATA_StockFlags[38];
extern "C" u32 DATA_RandSeed1;
extern "C" u32 DATA_RandSeed2;
extern "C" u8 DATA_SystemMilitaryActivity;
extern "C" u32 DATA_MilRankBase;
extern "C" u32 DATA_MilRankSub;
extern "C" u32 DATA_PoliceLevel;
extern "C" u8 DATA_CurrentDanger;
extern "C" u8 DATA_CurrentTraders;
extern "C" u32 DATA_StarportRand;
extern "C" u8 DATA_SystemTechLevel;
extern "C" u8 DATA_LastAttackedIndex;
extern "C" u32 DATA_NumPackages;
extern "C" u8 DATA_JettisonedCargoTypes[33];
extern "C" u8 DATA_PlasmaMount[];
extern "C" u16 DATA_PlayerFlags;
extern "C" u32 DATA_PlayerCash;
extern "C" u8 DATA_PlayerState;
extern "C" u32 DATA_FederalRank;
extern "C" u32 DATA_ImperialRank;
extern "C" u16 DATA_HostileTimer;

extern "C" char DATA_009148_StringArray[612];

extern "C" militaryMission_t DATA_MilitaryMissions[26];
extern "C" packageEntry_t DATA_PackageArray[60];

// cargotype = 0x8e00 + type
// returns pointer to cargo physobj.
extern "C"  u8* FUNC_000926_SpawnCargo(u32 cargotype, ModelInstance_t *ship, u32 cargoAmount);
extern "C"  void FUNC_000924_DestroyObject(ModelInstance_t *object, u8 inflictorIdx);

// gets amount of specified equipment type owned by ship
extern "C"  u32 FUNC_000392_GetEquipmentAmount(char *ship, u32 equipIdx);
extern "C"  void FUNC_000349_ShowCommMessage(u8 *msgData);
extern "C"  void FUNC_000304_AddFederalRank(u32 addition, u8 *data, u32 medal);
extern "C"  void FUNC_000305_AddImperialRank(u32 addition, u8 *data, u32 medal);

// gets data on a specified system (danger, pop, etc)
extern "C"  void FUNC_000869_GetSystemData(u32 id, u32 *d1, u32 *d2, u32 *techLevel, u32 *pirates, u32 *policeLevel, u32 *traders, u32 *d7, u32 *government);
extern "C"  void FUNC_000869_NoOverride(u32 id, u32 *d1, u32 *d2, u32 *techLevel, u32 *pirates, u32 *policeLevel, u32 *traders, u32 *d7, u32 *government);

extern "C"  void FUNC_000870_GetSystemDataExt(u32 id, u8 **stockFlags, u32 *population, u32 *danger, u32 *d4);
extern "C"  void FUNC_000871_GetSystemDataExt2(u32 id, u32 *d1, u32 *d2, u32 *milActivity, u32 *d4, u32 *milRankSub, u32 *milRankBase);

extern "C"  u32 FUNC_000853_GetNumstars(s16, s16, u32);

extern "C"  u32 FUNC_000857_GetNeighborSystem(u32 centerSystem, u32 unknown);
extern "C"  void FUNC_000878_GetSysObjectData(systemObject_t *p1, sysGenParams_t *p2, u32 useAllegiance);
extern "C"  void FUNC_GetSysGenParams(sysGenParams_t *genParams, u32 systemcode);

// gets a system of the opposing military power.
// returns pop. level in eax, return value of F857 in ret
extern "C" u32 FUNC_000775_FindSourceSystem(ModelInstance_t *ship);
extern "C"  u32 FUNC_000625_FindOppositionSystem(u32 *ret);
extern "C"  ModelInstance_t *FUNC_000772_AIShipSpawn(u32 type);
extern "C"  void FUNC_000697_GetBounty(ModelInstance_t *ship, u32 bountyLevel);
extern "C"  void FUNC_000702_GeneratePosition(ModelInstance_t *ship, u32 distance);
extern "C"  void FUNC_000634_AddToShipyard(u32 type, starport_t *starport, u32 **shipyardPtr);
extern "C"  void FUNC_000688_SpawnTraders();

extern "C"  void FUNC_000689_SpawnHSTrader();
extern "C"  void FUNC_000690_SpawnHSTrader2();
extern "C"  void FUNC_000691_SpawnDockedTrader();
extern "C"  void FUNC_000699_SpawnAuxTrader();

extern "C"  void FUNC_001661_Vec64Add(u64 *v1, u64 *v2);
extern "C"  void FUNC_001662_Vec64Sub(u64 *v1, u64 *v2);
extern "C"  void FUNC_001344_StringExpandFFCode(char *dest, int ffcode, u8 *vars);
extern "C"  u32 GetStarRange(u32 systemcode);

extern "C" u16 CoreX[];
extern "C" u16 CoreY[];

#define FloatRandom() ((float)DATA_RandomizerFunc() / 65536.0f)

extern "C" u8 SpawnHostileGroup(u8 ships, u8 *shipArray, u32 targetName, u8 shipType, u8 shipIDByte);
extern "C" int _finite(double);

extern "C" void CreateMarketData(starport_t *starport);
extern "C" void CreateBBSData(starport_t *starport);
extern "C" void CreateMilitaryData();
extern "C" void CreateShipyardData(starport_t *starport);

extern "C" void RefreshMilitaryData();
extern "C" void RefreshBBSData(starport_t *starport);
extern "C" void RefreshMarketData(starport_t *starport);
extern "C" void RefreshShipyardData(starport_t *starport);

extern "C" u32 GetNearbySystem(s8 bGetOpposing);

extern "C" void DeleteBBSAdvert(u32 idx, starport_t *starport);
extern "C" u32 GetEmptyAdSlot(starport_t *starport);

extern "C" u32 BBSCreateRandom(starport_t *starport, bbsAdvert_t *slot);

extern "C" void CreateBBSAdvert(starport_t *starport, bbsAdvert_t *slot, u32 string, u32 d1, u32 d2, u32 d3, u32 d4, u32 d5);
extern "C" void CreateMilitaryEntry(u32 string, u32 data1, u32 data2, u32 data3, u32 data4, u32 data5);

extern "C" void SpawnPirates(u32 pirateLevel, u32 bInitial);
extern "C" void SpawnAssassins(u32 ships, u32 missionIdx, u32 name);

extern "C" void CreateShips();
extern "C" void RefreshShips();
extern "C" u32 GetInitialFuel(ModelInstance_t *ship, u8 driveType);
extern "C" u32 GetNumFreeSlots();
extern "C" u32 IsStarportLocked(ModelInstance_t *starport);
extern "C" void GetStarportSupply(starport_t *starport, u8 *supply);
