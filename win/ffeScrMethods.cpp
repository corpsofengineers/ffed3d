#include "ffe3d.h"

//decl
extern "C" ModelInstance_t *WingmanShipSpawn (u8 leader,  u32 modelNum);
extern "C" ModelInstance_t *SpawnEnemy (u32 modelNum, u8 target);

int getPlayer (void);
int getObject (void);
int object_spawn_wingmen (void);
int object_spawn_enemy (void);
int object_equipadd (void);
int object_equipremove (void);

void scrInit (void)
{
	scriptSystem* scr = scriptSystem::getSingleton();

	//кладем в стек материнскии функции
	scr->newFunction (getPlayer);  scr->registerVariable ("getPlayer");
	scr->newFunction (getObject);  scr->registerVariable ("getObject");
}

//CLASSES

void createObjectClass (ModelInstance_t* obj)
{
	scriptSystem* scr = scriptSystem::getSingleton();

	scr->newClass();
	//Переменные
        scr->newChildInteger ("object_type", obj->object_type);
        scr->newChildInteger ("ship_type", obj->ship_type);
        scr->newChildInteger ("object_type", obj->object_type);
        scr->newChildInteger ("index", obj->index);
        scr->newChildInteger ("dest_index", obj->dest_index);
        scr->newChildInteger ("ai_mode", obj->ai_mode);
	//методы объекта
	scr->newChildFunction ("spawn", object_spawn_wingmen);
	scr->newChildFunction ("spawnEnemy", object_spawn_enemy);
	scr->newChildFunction ("equipAdd", object_equipadd);
	scr->newChildFunction ("equipRemove", object_equipremove);
}

//**********

//OBJECT METHODS

int object_spawn_wingmen (void)
{
	//получаем индекс лидера
	u8 lead = scriptSystem::getSingleton()->getParentAsInteger ("index");

	//внутри класс так что arg = argId+1
	u32 objId = scriptSystem::getSingleton()->getAsInteger (2);

	//создаем ведомого
	ModelInstance_t* wingman = WingmanShipSpawn (lead, objId); 

	if (wingman == NULL)
		return 0;

	createObjectClass (wingman);
	scriptSystem::getSingleton()->AddToLog ("Wingman Created");
	return 1;
}

int object_spawn_enemy (void)
{
	u8 target = scriptSystem::getSingleton()->getParentAsInteger ("index");
	u32 enemyID = scriptSystem::getSingleton()->getAsInteger (2);
	
	ModelInstance_t* enemy = SpawnEnemy (enemyID, target);

	if (enemy == NULL)
		return 0;

	createObjectClass (enemy);
	scriptSystem::getSingleton()->AddToLog ("Enemy detected. Be prepared");
	return 1;
}

int object_equipadd (void)
{
	scriptSystem* scr = scriptSystem::getSingleton();
	//получаем переменную object_type	
	u32 objType = scr->getParentAsInteger ("object_type");
	
	int equip = -1;

	if (objType == OBJTYPE_PLAYER)
	{
		//не забываем о смещении внутри класса +1
		equip = scr->getAsInteger (2);
		if (equip > -1)
        		DATA_PlayerObject->globalvars.equip |= (1 << equip) & 0xffffffff;
	} else // Не забыть включить проверку на то, что это все таки шип, а не то.... ^_^
	{
		equip = scr->getAsInteger (2);
		if (equip > -1)
		{
		 	ModelInstance_t *ship_instance = GetInstance(scr->getParentAsInteger ("index") ,DATA_ObjectArray);
			ship_instance->globalvars.equip |= (1 << equip) & 0xffffffff;
		}
	}

	//Вывод результата в консоль
	scr->AddToLog ("Adding equipment to object");
	return 0;
}

int object_equipremove (void)
{
	scriptSystem* scr = scriptSystem::getSingleton();
	//получаем переменную object_type	
	u32 objType = scr->getParentAsInteger ("object_type");
	
	int equip = -1;

	if (objType == OBJTYPE_PLAYER)
	{
		//не забываем о смещении внутри класса +1
		equip = scr->getAsInteger (2);
		if (equip > -1)
        		DATA_PlayerObject->globalvars.equip &= (1 << equip) & 0xffffffff;
	} else // Не забыть включить проверку на то, что это все таки шип, а не то.... ^_^
	{
		equip = scr->getAsInteger (2);
		if (equip > -1)
		{
		 	ModelInstance_t *ship_instance = GetInstance(scr->getParentAsInteger ("index") ,DATA_ObjectArray);
			ship_instance->globalvars.equip &= (1 << equip) & 0xffffffff;
		}
	}

	//Вывод результата в консоль
	scr->AddToLog ("Remove equipьуте from object");
	return 0;
}
//**********

int getPlayer (void)
{
	ModelInstance_t* pl = GetInstance (DATA_PlayerIndex, DATA_ObjectArray);
	createObjectClass (pl);
	return 1;
}

int getObject (void)
{
	u32 objID = scriptSystem::getSingleton()->getAsInteger (1);
	ModelInstance_t* obj = GetInstance (objID, DATA_ObjectArray);
	createObjectClass (obj);
	return 1;	
}