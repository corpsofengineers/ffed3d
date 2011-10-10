#include "ffescr.h"
#include "misc.h"

extern void CreateShips();
extern INT32 IsStarportLocked(ModelInstance_t *starport);
extern void FUNC_000924_DestroyObject(ModelInstance_t *object, INT8 inflictorIdx);

int getPlayer (void);
int getPlayerIndex (void);
int getObject (void);
int setLastAttackedIndex ();
int createShips_conv (void);
int checkDATAObjectArray (void);
int isStarportLocked_conv (void);
int destroyObject (void);

void scrInit (void)
{
	scriptSystem* scr = scriptSystem::getSingleton();

	//pushing game method`s
	scr->newFunction (getPlayer);            scr->registerVariable ("GetPlayer");
	scr->newFunction (getPlayerIndex);       scr->registerVariable ("GetPlayerIndex");
	scr->newFunction (getObject);			 scr->registerVariable ("GetObject");
	scr->newFunction (setLastAttackedIndex); scr->registerVariable ("SetLastAttackedIndex");
	scr->newFunction (createShips_conv);	 scr->registerVariable ("CreateShips");
	scr->newFunction (checkDATAObjectArray); scr->registerVariable ("CheckDATAObjectArray");
	scr->newFunction (isStarportLocked_conv);scr->registerVariable ("IsStarportLocked");
	scr->newFunction (destroyObject);        scr->registerVariable ("DestroyObject");
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
	int id = scriptSystem::getSingleton()->getAsInteger (1);

	DATA_LastAttackedIndex = id;

	return 0;
}

int createShips_conv (void)
{
	CreateShips();
	return 0;
}

int checkDATAObjectArray (void)
{
	int i = scriptSystem::getSingleton()->getAsInteger (1);
	__int64 r = scriptSystem::getSingleton()->getAsInteger (2);

	if (INT8_AT(DATA_ObjectArray+i) == r)
	{
		scriptSystem::getSingleton()->newBool (true);
	} else
	{
		scriptSystem::getSingleton()->newBool (false);
	}

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