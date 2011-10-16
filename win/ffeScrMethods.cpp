#include "ffescr.h"
#include "misc.h"

extern void CreateShips();
extern INT32 IsStarportLocked(ModelInstance_t *starport);
extern void FUNC_000924_DestroyObject(ModelInstance_t *object, INT8 inflictorIdx);

extern "C" void SpawnTraders(INT32 traderLevel, INT32 bInitial);

int getPlayer (void);
int getPlayerIndex (void);
int getObject (void);



int setLastAttackedIndex (void);
int createShips_conv (void);
int getObjectState (void);
int isStarportLocked_conv (void);
int destroyObject (void);

int transportMissions (void);
int getTrsnsportMission (void);
int currentSystem (void);
int spawnAssassins_conv (void);
int currentPirates (void);
int currentTraders (void);
int spawnPirates_conv (void);
int spawnTraders_conv (void);


void scrInit (void)
{
	scriptSystem* scr = scriptSystem::getSingleton();

	//pushing game method`s
	scr->newFunction (getPlayer);            scr->registerVariable ("GetPlayer");
	scr->newFunction (getPlayerIndex);       scr->registerVariable ("GetPlayerIndex");
	scr->newFunction (getObject);			 scr->registerVariable ("GetObject");

	scr->newFunction (setLastAttackedIndex); scr->registerVariable ("SetLastAttackedIndex");
	scr->newFunction (createShips_conv);	 scr->registerVariable ("CreateShips");
	scr->newFunction (getObjectState);       scr->registerVariable ("GetObjectState");
	scr->newFunction (isStarportLocked_conv);scr->registerVariable ("IsStarportLocked");
	scr->newFunction (destroyObject);        scr->registerVariable ("DestroyObject");

	scr->newFunction (transportMissions);    scr->registerVariable ("TransportMissions");
	scr->newFunction (getTrsnsportMission);  scr->registerVariable ("GetTrsnsportMission");
	scr->newFunction (currentSystem);        scr->registerVariable ("CurrentSystem");
	scr->newFunction (spawnAssassins_conv);  scr->registerVariable ("SpawnAssassins");
	scr->newFunction (currentPirates);       scr->registerVariable ("CurrentPirates");
	scr->newFunction (currentTraders);       scr->registerVariable ("CurrentTraders");
	scr->newFunction (spawnPirates_conv);    scr->registerVariable ("SpawnPirates");
	scr->newFunction (spawnTraders_conv);    scr->registerVariable ("SpawnTraders");
}

//Game CLASSES Pushing To Lua
void createModelInstanceClass (ModelInstance_t* obj)
{
	scriptSystem* scr = scriptSystem::getSingleton();

	scr->newClass();
	scr->newChildInteger ("object_type", obj->object_type);
	scr->newChildInteger ("ship_type", obj->ship_type);
	scr->newChildInteger ("object_type", obj->object_type);
	scr->newChildInteger ("index", obj->index);
	scr->newChildInteger ("dest_index", obj->dest_index);
	scr->newChildInteger ("ai_mode", obj->ai_mode);
}

void createPackageEntryClass (packageEntry_t pck)
{
	scriptSystem* scr = scriptSystem::getSingleton();

	scr->newClass();
	scr->newChildInteger ("cash", pck.cash);
	scr->newChildInteger ("passengers", pck.passengers);
	scr->newChildInteger ("mission_idx", pck.mission_idx);
	scr->newChildInteger ("ships", pck.ships);
	scr->newChildInteger ("system", pck.system);
	scr->newChildInteger ("date", pck.date);
	scr->newChildInteger ("name", pck.name);
}

int getPlayer (void)
{
	ModelInstance_t* player;
	player = GetInstance (DATA_PlayerIndex, DATA_ObjectArray);
	createModelInstanceClass (player);
	return 1;
}

int getPlayerIndex (void)
{
	scriptSystem::getSingleton()->newInteger (DATA_PlayerIndex);
	return 1;
}

int getObject (void)
{
	int id = scriptSystem::getSingleton()->getAsInteger (1);
	ModelInstance_t* obj;
	obj = GetInstance (id, DATA_ObjectArray);
	createModelInstanceClass (obj);
	return 1;
}

int setLastAttackedIndex (void)
{
	INT8 id = scriptSystem::getSingleton()->getAsInteger (1);

	DATA_LastAttackedIndex = id;

	return 0;
}

int createShips_conv (void)
{
	CreateShips();
	return 0;
}

int getObjectState (void)
{
	int i = scriptSystem::getSingleton()->getAsInteger (1);
	
	u8 state = DATA_ObjectArray->state_flags[i];

	scriptSystem::getSingleton()->newInteger (state);

	return 1;
}

int isStarportLocked_conv (void)
{
	__int64 id = scriptSystem::getSingleton()->getParentAsInteger ("parent_index");
	ModelInstance_t* starport = GetInstance (id, DATA_ObjectArray);

	if (IsStarportLocked (starport) )
	{
		scriptSystem::getSingleton()->newBool (true);
	} else
	{
		scriptSystem::getSingleton()->newBool (false);
	}

	return 1;
}

int destroyObject (void)
{
	__int64 id = scriptSystem::getSingleton()->getParentAsInteger ("parent_index");
	INT8 inflictorIdx = scriptSystem::getSingleton()->getAsInteger (2); 
	ModelInstance_t* obj = GetInstance (id, DATA_ObjectArray);
	FUNC_000924_DestroyObject(obj, inflictorIdx);
	return 0;
}

//CreateShips

int transportMissions (void)
{
	scriptSystem::getSingleton()->newInteger (DATA_NumPackages);
	return 1;
}

int currentSystem (void)
{
	scriptSystem::getSingleton()->newInteger (DATA_CurrentSystem);
	return 1;
}

int getTrsnsportMission (void)
{
	int i = scriptSystem::getSingleton()->getAsInteger (1);
	createPackageEntryClass (DATA_PackageArray[i]);
	return 1;
}

int spawnAssassins_conv (void)
{
	int ships = scriptSystem::getSingleton()->getAsInteger (1);
	int idx = scriptSystem::getSingleton()->getAsInteger (2);
	int name = scriptSystem::getSingleton()->getAsInteger (3);
	SpawnAssassins (ships, idx, name);
	return 0;
}

int currentPirates (void)
{
	scriptSystem::getSingleton()->newInteger (DATA_CurrentPirates);
	return 1;
}

int currentTraders (void)
{
	scriptSystem::getSingleton()->newInteger (DATA_CurrentTraders);
	return 1;
}

int spawnPirates_conv (void)
{
	int pirateLevel = scriptSystem::getSingleton()->getAsInteger (1);
	int bInitial = scriptSystem::getSingleton()->getAsInteger (2);
	SpawnPirates (pirateLevel, bInitial);
	return 0;
}

int spawnTraders_conv (void)
{
	int traderLevel = scriptSystem::getSingleton()->getAsInteger (1);
	int bInitial = scriptSystem::getSingleton()->getAsInteger (2);
	SpawnTraders (traderLevel, bInitial);
	return 0;
}

//**********