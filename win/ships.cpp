
#include "ffe3d.h"

// number of 250-kg damages that one tonne of metal alloys will replace
#define ALLOY_MULT 16
#define HULL_REGEN_RATE 0.00000075
#define SHIELD_REGEN_RATE 0.002

long double HullDiff[2] = {0.0,0.0}, ShieldDiff[2] = {0.0,0.0};	// for storage
long double hullAccum = 0.0, shieldRechargeAccum[116] = {0.0};

// absorption goes from 85% (full shields) to 40% (no shields)
float c = -0.45, b = -0.40;

// hull absorption goes from 95% to 80%
float hc = -0.15, hb = -0.80;

// lasers take up 25%-50% of total space
float baseLaserPct = 0.25, extLaserPct = 0.25;

// Determines how quickly a ship can turn!  OH BOY!
float TurningMultipliers[] =
{
	1.0, 1.0, 1.0, 1.0,
	// Missiles 0x4-0xd
	1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
	// Escape Capsule and other worthless ships
	0.33, 0.33, 0.33, 0.33, 0.33,
	1.5,	// Osprey interceptor
	1.4,	// Falcon interceptor
	1.325,	// Hawk Airfighter
	1.25,	// Kestrel Airfighter
	1.4,	// Eagle long range fighter
	1.3,	// Eagle MKII
	1.25,	// Eagle MKIII
	1.0,	// Sidewinder
	0.92,	// Krait
	0.88,	// Gecko
	0.68,	// Adder
	1.3,	// Viper Defence Craft
	1.2,	// Saker MKIII
	1.1,	// Osprey 'X' Interceptor
	1.15,	// Merlin Interceptor
	1.2, 	// Viper MKII
	1.1,	// Gyr attack fighter
	0.70,	// Cobra MKI
	0.485,	// Moray Starboat
	0.665,	// Cobra MKIII
	0.60,	// Constrictor
	1.0,	// Asp Explorer
	0.81,	// Lanner
	1.05,	// Harris Fighter
	1.0,	// Spar Interceptor
	0.775,	// Wyvern Explorer
	0.65,	// Skeet Cruiser
	0.70,	// Turner Class
	0.75,	// Lanner II
	0.70,	// Harrier
	1.0, 1.0, 1.0, 1.0,		// Unknowns
	0.5,	// Transporter
	0.6,	// Lion Transport
	0.5,	// Tiger Trader
	0.58,	// Imp. Courier
	0.44,	// Python Trader
	0.46,	// Imp. Trader
	0.41,	// Anaconda Destroyer
	0.25,	// Puma Freighter
	0.31,	// Boa Trader
	0.35,	// Panther Clipper
	1.0,	// Unknown
	0.31,	// Tiercel Freighter
	0.328,	// Imp. Explorer
	0.25,	// INRA Command Ship
	0.36,	// Mantis Freighter
	0.21,	// Griffin Hauler
	1.0,	// Thargoid Transport
	0.5,	// Lynx Bulk Carrier
	0.5		// Long-range Cruiser
};

u32 maxBounty = 12500;

// New shield value calculate function
// Handles integral of ds/dd = b + cs:
// d = [ln|cs+b| - ln|ci+b|]/c
// s = [e^(cd)*(ci+b) - b]/c (infinitely small interval)
extern "C" float CALC_NewShieldValue (float initial, float damage)
{
	
	// restrict integration to shield values above 0
	// (negative shield values do not exist and could skew)
	float MaxDamage = ((log(fabs(b)) - log(fabs((c*initial) + b))) / c);
	
	if (initial <= 0 || !_finite(MaxDamage))
		return 0; 

	if (damage > MaxDamage)
		damage = MaxDamage;

	return (((exp(c * damage) * ((c*initial) + b)) - b) / c);
}
 
extern "C" float CALC_NewHullValue (float initial, float damage)
{
	float MaxDamage = ((log(fabs(hb)) - log(fabs((hc*initial) + hb))) / hc);
	
	if (initial <= 0 || !_finite(MaxDamage))
		return 0; 

	if (damage > MaxDamage)
		damage = MaxDamage;

	return (((exp(hc * damage) * ((hc*initial) + hb)) - hb) / hc);
}

extern "C" u32 GetVol(u32 logVol)
{
	if (logVol < 4)
		return 0x7e00;

	logVol >>= 2;
	
	return (0x7e00 >> logVol);
}

u32 GetRandomModelNum(u8 object_type)//, u32 *pIndex)
{
	u32 num;
	u8 table_index = object_type;

	if (object_type == 1 || object_type == 0) 
		table_index = 2 * DATA_CurrentAllegiance;		// current system?

	num = DATA_ShipsLoyalityTable[table_index].size;		// table size

	if (object_type == 1 && DATA_CurrentPopulation < 4) 
		num = DATA_CurrentPopulation * num / 4;

	//if (tablenum != 16) {
		num = BoundRandom(num);
	//	pIndex[0] = DATA_RandSeed2;
	//} else { 
	//	num = pIndex[0] * DATA_ShipsLoyalityTable[tablenum].size >> 16;
	//}

	return DATA_ShipsLoyalityTable[object_type].ships[num-1];
}

extern "C" void FUNC_001341_Int64ArithShift(__int64 *, int);

void InitInstance(ModelInstance_t *new_instance, ModelInstance_t *old_instance, u32 model_num)
{
    if(old_instance != new_instance) {
        memcpy(new_instance, old_instance, 0x152);
        new_instance->index = model_num != 0 ? new_instance->index : 0;
    }
    if(model_num != 0) {
		new_instance->globalvars.local_Startime = DATA_GameTics;
		new_instance->globalvars.local_Stardate = DATA_GameDays;
		new_instance->model_num = model_num;
        Model_t* model = GetModel(model_num);

		u32 scale = (model->Scale + model->Scale2 + 7);

		new_instance->interract_radius.full = model->interract_radius;
		FUNC_001341_Int64ArithShift(&new_instance->interract_radius.full, scale - 0xf);
		new_instance->interract_radius.hi = model->interract_radius >> 0x1f;

		new_instance->collision_radius.full = model->collision_radius;
		FUNC_001341_Int64ArithShift(&new_instance->collision_radius.full, scale - 0xf);
		new_instance->collision_radius.hi = model->collision_radius >> 0x1f;

		new_instance->laser_flags = 0;
		new_instance->uchar_25 = 0;
		new_instance->name[0] = 0;
		u8 dl = ( model->DefaultColorR & 0xff) >> 4 & 7;
		u16 *collision = model->Collision_ptr;
        if(collision != 0 && *collision != 0) {
            dl = dl | 0x80;
        }
		new_instance->flags = dl;
		new_instance->dist_cam = 0xff;
		ShipDef_t* ship_def = model->Shipdef_ptr;
        if(ship_def != 0) {
			new_instance->globalvars.Thrust_C4 = ship_def->ForwardThrust;
            new_instance->globalvars.Thrust_C6 = ship_def->RearThrust;
            new_instance->globalvars.Thrust_C2 = ship_def->RearThrust;
            new_instance->globalvars.Thrust_BE = ship_def->RearThrust;
            new_instance->globalvars.Thrust_C0 = ~ship_def->RearThrust;
            new_instance->globalvars.Thrust_BC = ~ship_def->RearThrust;
			new_instance->thrust_power = ~ship_def->RearThrust >= 0x1329 ? 2 : ~ship_def->RearThrust >= 0xaa5 ? 1 : 0;
			new_instance->globalvars.num_lasers = ship_def->Lasers;
			new_instance->mass_x4 = ship_def->Mass * 4;
			u16 capacity = ship_def->Capacity; // capacity
			if(ship_def->Drive != 0) {
				new_instance->globalvars.drive = ship_def->Drive;
                capacity = capacity - DATA_DriveMasses[new_instance->globalvars.drive];
            }
			new_instance->cargo_space = capacity;
			new_instance->globalvars.shields = 0;
			new_instance->globalvars.max_shields = 0;
        }
    }
}

ModelInstance_t *CreateObject(ModelInstance_t *ship_instance, u8 state, u32 model)
{
	u8 index;

 	for(index = 1; index <= 94; index++) {
        if (DATA_ObjectArray->state_flags[index] == 0)
			break;
    }

	if (index > 94) { // 20 reserved
		for(index = 1; index <= 94; index++) {
			if (DATA_ObjectArray->state_flags[index] & 0x41) { // smoke
				DATA_NumObjects--;
				break;
			}
		}
	}

	if (index > 94) { // 20 reserved
		for(index = 1; index <= 94; index++) {
			if (!(DATA_ObjectArray->state_flags[index] & 0x20)) {
				DATA_NumObjects--;
				break;
			}
		}
	}

	if (index <= 94) {
        DATA_ObjectArray->state_flags[index] = state;
        if(!(state & 0x20)) {
            DATA_NumObjects++;
        }

		ModelInstance_t *instance = &DATA_ObjectArray->instances[index];
        InitInstance(instance, ship_instance, model);
        instance->model_num = model;
        instance->index = (u8)index;
		return instance;
	}

	return NULL;
}

u32 AttachLaser(ModelInstance_t *ship, u16 *cargo_space)
{
	u32 i, maxLasers;
	u32 weight = 0;//BoundRandom(0x33);
	weight = DATA_ShipLaserCapacity[ship->object_type];
	weight = weight * *cargo_space >> 8;

	if (DATA_PlasmaMount[ship->model_num] == 0x0)
		maxLasers = 0x6;
	else
		maxLasers = 0x8;

	for (i = 1; ; i++) {
		if (weight < DATA_AILasers[i].weight) break;
		if (i == maxLasers) break;
	}

	u32 laser = i;

	if (i != 1)
		laser = BoundRandom(i)-1;

	*cargo_space -= DATA_AILasers[i].weight;
	return DATA_AILasers[i].id;
}

extern "C" void FUNC_000791_GenerateName(u32, u8*, u32, u8*);

// names
// 0 - "????"
// 1 - human full name
// 2 - human full name
// 3 - human full name
// 4 - human second name
// 5 - human first name
// 6 - starport
// 7 - station
// 8 - city name
// 9 - world name
// 10 - merchant name
// 11 - battleships?
// 12 - corporate name
// 13 - merchant name
// 14 - "police"
// 15 - "????"
// 16 - city name
// 17 - town name
// 18 - ship reg num
// 19 - star reg num?
// 20 - star reg num?

void GenerateShipName(ModelInstance_t *ebx, u8 *tptr, u32 type)
{
	DATA_RandomizerFunc();
	ebx->globalvars.unique_Id = DATA_RandSeed1;
	FUNC_000791_GenerateName(type, tptr, ebx->globalvars.unique_Id, ebx->name);
}

ModelInstance_t *CreateShip(ModelInstance_t *copyfrom, u8 object_type, int modelnum)
{
	ModelInstance_t *ship_instance = CreateObject(copyfrom, 0x4f, modelnum);	// create object
	if (ship_instance == NULL) return NULL;

	ship_instance->ship_type = 0xb;					// AI ship
	ship_instance->object_type = object_type;			// store shiptable

	u32 namegentype = 18;

	if (object_type == 0xe) { // police
		namegentype = 14;
	} else if (object_type == 0xc) {
		namegentype = BoundRandom(2)+1;
	} else if (modelnum == 0x11 ||
			   modelnum == 0x31 || 
			   modelnum == 0x32 || 
			   modelnum == 0x33 || 
			   modelnum == 0x34 || 
			   modelnum == 0x3f || 
			   modelnum == 0x45) {
		namegentype = 0;
	}

	GenerateShipName(ship_instance, &DATA_NameThingyByteTable[object_type], namegentype);		// ship name?
	
	ShipDef_t* ship_def = GetModel(ship_instance->model_num)->Shipdef_ptr;		// stats pointer
	
	u16 max_tech_level = DATA_RandomizerFunc() & 0xffff;				// tech/wealth?

	if (object_type == 0xe) {			// police
		ship_instance->globalvars.unique_Id = 0;		// Uniform colour
		max_tech_level = DATA_SystemTechLevel << 8;
	}

	if (object_type == 0x10) 
		max_tech_level = 0xffff;	// assass. targets

	u16 cargo_space = ship_def->Capacity;							// remain. int cap
	u32 equipment = 0;
	u32 drive = ship_def->Drive;							// drive
	
	u32 edx = DATA_RandomizerFunc();

	if (object_type == 0xc) // bounty hunters?
		edx = 0xffff;		
	if (edx > 0xf000 && object_type != 1) { 
		//cargo_space--; 
		equipment |= EQUIP_HYPER_CLOUD_ANALYZER; 
	}

	if (object_type == 0xe) { // police
		drive = 0x1; // interplanetary drive
	}

	if (drive >= 0) {
		//if (edx > 0xc000) 
		//	drive >>= 8;			// 1/3 chance of altern.
		//drive &= 0xf;
		ship_instance->globalvars.drive = drive;
		cargo_space -= DATA_DriveMasses[drive];					// actual drive
	}

	ship_instance->fuel_tank = DATA_DriveTankFuel[drive] << 0x18;		// fuel
	u32 cargo_fuel = DATA_DriveCargoFuel[drive] * 2;
	if (cargo_fuel >= cargo_space) 
		cargo_fuel = cargo_space;		
	cargo_space -= cargo_fuel;	// J3101
	ship_instance->cargo_fuel = cargo_fuel;	// cargo fuel

	// for pirates, 10%-20% goes to cargo space... unoccupied
	// for traders, 20%-40% is occupied
	// for assassination targets, 10%-20% is occupied
	// for military ships, assassins, and police, no extra cargo
	if (object_type < 0x13 && object_type != 0xe)
	{
		u16 usedWeight = 0.1f + 0.1f*FloatRandom();
		
		if (object_type < 0xa)
			usedWeight *= 2;

		usedWeight *= cargo_space;
		cargo_space -= usedWeight;
	}

	ship_instance->globalvars.laser_front = AttachLaser(ship_instance, &cargo_space);				// laser

	u32 shields = cargo_space;
	//if (shields > 0x12c) shields = 0x12c;

	if (object_type == 1) 
		shields = cargo_space * 0.2f + 0.2f*BoundRandom(10);
	else if (object_type == 0xc)
		shields = cargo_space * 0.8f + 0.2f*BoundRandom(10);
	else
		shields = cargo_space * 0.4f + 0.2f*BoundRandom(10);

	shields = (shields >> 2); // round to 4t
	//shields = shields * max_tech_level / 0xffff;
	//shields = BoundRandom(shields);
	//shields = BoundRandom(shields) * 2;
	cargo_space -= shields << 2;
	ship_instance->globalvars.shields = ship_instance->globalvars.max_shields = shields << 5; // 1 shield = 4t = 64 units

	if (cargo_space >= 5) {
		u32 techlevel = BoundRandom(max_tech_level);
		if (techlevel >= 0xc000) {
			equipment |= EQUIP_NAVAL_ECM;
			cargo_space -= 5;							// naval ECM
		}
		else if (techlevel >= 0x4000) {
			equipment |= EQUIP_ECM;
			cargo_space -= 5;							// ECM
		}
	}	

	//if (cargo_space > 0) {
		u16 missiles = ship_def->Missiles;						// missiles
		missiles = BoundRandom(missiles);
		if (missiles >= 1) {
			u8 missile_type = DATA_AIMissiles[max_tech_level >> 0xc];
			while (/*cargo_space > 0 && */missiles != 0) {
				ship_instance->globalvars.missiles[missiles] = missile_type;
				//cargo_space--; 
				missiles--;
			}
		}
	//}
	// J3108
	//if (cargo_space > 0) {
		//u32 techlevel = BoundRandom(max_tech_level);
		//if (techlevel >= 0x800) { 
		//	//cargo_space--; 
			equipment |= EQUIP_SCANNER; 
		//}
		//techlevel = BoundRandom(max_tech_level);
		//if (techlevel >= 0xfc00 && cargo_space >= 4 && ship_instance->object_type != 0xe) {
		//	cargo_space--; equipment |= EQUIP_ENERGY_BOMB;
		//}
	//}
	equipment |= EQUIP_ATMOSPHERIC_SHIELD;								// atmos. shield

	//u32 more_fuel = cargo_space * DATA_DriveCargoFuel2[ship_instance->object_type];
	//if (more_fuel > 0xaf) 
	//	more_fuel = 0xaf;
	//cargo_space -= more_fuel;
	//ship_instance->cargo_fuel += more_fuel;						// Cargo fuel again?
	//ship_instance->cargo_space = cargo_space;						// remaining capacity

	ship_instance->globalvars.equip = equipment;		// equipment

	return ship_instance;
}

// Object gen of some kind
extern "C" ModelInstance_t *AIShipSpawn(u8 object_type)
{
	if (object_type >= 0x13)
		object_type -= 0x9;

	u32 modelNum = GetRandomModelNum (object_type);
	return CreateShip(&DATA_DummyInstance, object_type, modelNum);
}

//SpawnObjects

//WindMans!

extern "C" ModelInstance_t *WingmanShipSpawn (u8 leader,  u32 modelNum)
{
	ModelInstance_t *leadShip = GetInstance (leader, DATA_ObjectArray);
	ModelInstance_t* ship = CreateShip (leadShip, 0xc, modelNum);

	if (ship == NULL)
		return NULL;

	ship->dest_index = leader;
	ship->target_index = 0; //надо будет покурить потом
	ship->ai_mode = AI_FORMATION;

	//выбираем место для создания объедка

	if (leadShip->ai_mode == AI_DOCKED_OR_LANDED)
	{
		//спауним в космопорте
	} else //спауним в положении Delta
	{
		ship->thrust_power++;
		ship->object_type = 0xc;
	}

	ship->globalvars.equip |= EQUIP_TRACKING_DEVICE;

	FUNC_000048_BeginEvents(0x17, 0x0, ship->index);
	return ship;
}

extern "C" ModelInstance_t *SpawnEnemy (u32 modelNum, u8 target)
{
	ModelInstance_t* ship = CreateShip (&DATA_DummyInstance, 0xa, modelNum);

	if (ship == NULL)
		return NULL;

	ship->dest_index = target;
	FUNC_000702_GeneratePosition(ship, 0x315000);
	ship->ai_mode = AI_PIRATE_PREPARE;
	ship->target_index = target;
	ship->thrust_power++;
	ship->object_type = OBJTYPE_PIRATE;

	FUNC_000048_BeginEvents(0x17, 0x0, ship->index);
	return ship;
}


// do damage.  
extern "C" u32 DoShipDamage(ModelInstance_t *ship, u32 damage, u32 src_index, u8 bContinuous)
{
	float temp;
	float oldShields;
	float newShields;
	float newHull, oldHull;
	float shieldAbsorb, hullAbsorb;
	float realDamage, equipAbsorb;
	long double damageProbability, baseProbability;
	u16 shields, hull, totalShields, totalHull;
	u8  idx, numDamages;
	ShipDef_t *ship_def;
	u32 rand, iProbability, iResult = 0, vol;

	if (damage == 0) 
		return 0;

	if (src_index == ship->index)
		return 0;

	idx = ((ship == DATA_PlayerObject) ? 1 : 0);

	if (idx) return 0; // god mode

	//ModelInstance_t *src = GetInstance(src_index, DATA_ObjectArray);

	shields = ship->globalvars.shields;
	totalShields = ship->globalvars.max_shields;
	hull = ship->mass_x4;
	
	ship_def = GetModel(ship->model_num)->Shipdef_ptr;
	totalHull = ship_def->Mass * 4;
	
	oldHull = hull;// + HullDiff[idx];
	oldShields = shields;// + ShieldDiff[idx];

	if ((totalShields > 0) && (oldShields > 0))
	{
		temp = c/b;
		realDamage = damage/(float)totalShields;

		newShields = CALC_NewShieldValue(oldShields / (float)totalShields, realDamage);

		// get the new shield strength!
		newShields *= totalShields;

		shieldAbsorb = oldShields - newShields;
	}
	else
	{
		shieldAbsorb = 0.0;
		newShields = 0.0;
	}

	if (newShields < 0)
	{
		hullAbsorb = newShields;
		newShields = 0;
	}
	else
		hullAbsorb = 0.0;

	hullAbsorb += (float)damage - shieldAbsorb;

	newHull = (float)totalHull * CALC_NewHullValue(oldHull/(float)totalHull, hullAbsorb/(float)totalHull);
	equipAbsorb = hullAbsorb + (newHull - oldHull);

	if (newHull < 0.0)
		newHull = 0.0;
	
	ship->mass_x4 = (u16)newHull;
	ship->globalvars.shields = (u16)newShields;
	
	// store small differences to avoid rounding-error magnification.
	//HullDiff[idx] = newHull - hull;
	//ShieldDiff[idx] = newShields - shields;
	
	if (hull == 0)
		return 0;	// death

	realDamage = hullAbsorb + shieldAbsorb;
	hullAbsorb /= realDamage;
	shieldAbsorb /= realDamage;

	if (hullAbsorb > 0)
	{
		ship->smoke_timeout = 10;
		
		vol = idx ? 0x7e00 : GetVol(ship->dist_cam);
		FUNC_001908_SoundPlaySampleLinVol(0x1d, (u32)(vol * hullAbsorb) | 0x80000000);
	}

	if (shieldAbsorb > 0)
	{
		ship->laser_flags |= 0x1;
		
		vol = idx ? 0x7e00 : GetVol(ship->dist_cam);
		FUNC_001908_SoundPlaySampleLinVol(0x1c, (u32)(vol * shieldAbsorb) | 0x80000000);
	}

	// calculate probability of equipment damage here (1-FFFF).

	baseProbability = damageProbability = 1 - pow(0.6, 20.0*equipAbsorb / (float)totalHull);

	rand = DATA_RandomizerFunc();

	iProbability = (u32)(damageProbability * 65536.0);

	numDamages = 0;
	//while (rand < iProbability)
	if (rand < iProbability)
	{
		iResult = FUNC_000952_DestroyEquip(ship);
		
		// another chance...
		damageProbability *= baseProbability;
		iProbability = (u32)(damageProbability * 65536.0);
		numDamages++;
	}
	
	// I don't know what this does, but it would be wise
	// to maintain the status quo :)
	if ((numDamages == 0) && (idx == 1))
	{
		FUNC_000148_Unknown(0x17, 0x0); // ???
		return 1;
	}
	else
		return iResult;
}

extern "C" u32 GetInitialFuel(ModelInstance_t *ship, u8 driveType)
{
	float fuelSpace;
	s32 rand, randSeed, randSeed2;

	// police have no fuel...
	if (ship->object_type== OBJTYPE_POLICE || driveType == 0x1)
		return 0;

	driveType = ship->globalvars.drive;
	randSeed = ship->globalvars.unique_Id;
	
	randSeed2 = randSeed = (randSeed << 16) | (randSeed >> 16);

	// pseudorandom effect, same all the time
	rand = DATA_FixedRandomFunc(0x10000, &randSeed, &randSeed2) & 0x7fff; 
	
	fuelSpace = DATA_DriveFuelConsumption[driveType];
	fuelSpace += sqrt(DATA_DriveFuelConsumption[driveType]*1.5*(rand/32768.0)) * 4;

	return fuelSpace;
}

extern "C" ModelInstance_t *AIChooseEquipment(ModelInstance_t *ship, u32 shipType)
{
	u16 spaceUsed, spaceAvail, hullMass, i;
	float usedWeight, allowedWeight;
	u16 maxLasers, fuelSpace;
	u8 driveType, numPylons;
	ShipDef_t *ship_def;
	float laserPct;
	u32 laserID, oldSystem;
	
	spaceUsed = 0;
	if (shipType >= 0xa && DATA_RandomizerFunc() < 48000)
		oldSystem = DATA_CurrentSystem;
	else
	{
		oldSystem = FUNC_000857_GetNeighborSystem(DATA_CurrentSystem, BoundRandom(3));
		if (oldSystem == 0)
			oldSystem = DATA_CurrentSystem;
		else
		{
			u8 *c1;
			u32 population, danger, c4;
			
			FUNC_000870_GetSystemDataExt(oldSystem, &c1, &population, &danger, &c4);
			if (population < 3)
				oldSystem = DATA_CurrentSystem;
		}
	}

	ship->bounty = oldSystem;

	ship->globalvars.laser_front = 0;
	ship->globalvars.shields = ship->globalvars.max_shields = 0;

	ship_def = GetModel(ship->model_num)->Shipdef_ptr;
	hullMass = ship_def->Mass * 4;
	spaceAvail = ship_def->Capacity;

	driveType = ship_def->IntegralDrive;
	if (driveType == 0x80)
		driveType = ship_def->Drive;
	
	// police use interplanetary drive for more space, so do station helpers
	if (shipType == 0xe)
	{
		driveType = 0x1;
		ship->object_type = OBJTYPE_POLICE;	// police marking
	}
	else if (shipType == 0xf)
		driveType = 0x1;

	ship->globalvars.drive = driveType;

	usedWeight = DATA_DriveMasses[driveType];
	spaceAvail -= usedWeight;
	spaceUsed = usedWeight;

	// allow space for fuel, but don't overdo it.
	fuelSpace = GetInitialFuel(ship, driveType);
	
	if (fuelSpace > spaceAvail)
	{
		spaceUsed += spaceAvail;
		spaceAvail = 0;
	}
	else
	{
		spaceAvail -= fuelSpace;
		spaceUsed += fuelSpace;
	}

	// Bulk Carriers and other helpers are all about the cargo
	if (shipType == 0xf)
	{
		ship->cargo_space = ship_def->Capacity - spaceUsed;
		return ship;
	}

	// for pirates, 10%-20% goes to cargo space... unoccupied
	// for traders, 20%-40% is occupied
	// for assassination targets, 10%-20% is occupied
	// for military ships, assassins, and police, no extra cargo
	if (shipType < 0x13 && shipType != 0xe)
	{
		usedWeight = 0.1 + 0.1*FloatRandom();
		usedWeight *= spaceAvail;

		if (shipType < 0xa)
			usedWeight *= 2;
		else if (shipType != 0x10)
			spaceUsed += usedWeight;

		spaceAvail -= usedWeight;
	}

	if (driveType == 0x1)
		fuelSpace = 0.0;

	ship->cargo_fuel = fuelSpace;
	spaceAvail -= fuelSpace;
	spaceUsed += fuelSpace;

	
	laserPct = baseLaserPct + extLaserPct * (DATA_RandomizerFunc() / 65536.0);
	
	allowedWeight = laserPct * spaceAvail;

	laserID = 0;
	usedWeight = 0;

	if (DATA_PlasmaMount[ship->model_num] == 0x0)
		maxLasers = 0x7;
	else
		maxLasers = 0x9;

	// no gun mounts?
	if (ship->globalvars.num_lasers != 0)
		for (i = 0; i < maxLasers; i++)
		{
			if (DATA_AILasers[i].weight > allowedWeight)
				break;

			laserID = DATA_AILasers[i].id;
			usedWeight = DATA_AILasers[i].weight;
		}

	ship->globalvars.laser_front = laserID;

	spaceUsed += usedWeight;
	spaceAvail -= usedWeight;

	ship->globalvars.equip &= ~(EQUIP_ECM | EQUIP_NAVAL_ECM);

	// install misc. equipment
	// allow up to 50% of the remaining space.
	allowedWeight = 0.707107 * (DATA_RandomizerFunc() / 65536.0);
	allowedWeight *= allowedWeight * spaceAvail;
	
	if (allowedWeight >= DATA_NECM_Weight)
	{
		ship->globalvars.equip |= EQUIP_NAVAL_ECM;
		usedWeight = DATA_NECM_Weight;
		allowedWeight -= DATA_NECM_Weight;
	}
	else if (allowedWeight >= DATA_ECM_Weight)
	{
		ship->globalvars.equip |= EQUIP_ECM;
		usedWeight = DATA_ECM_Weight;
		allowedWeight -= DATA_ECM_Weight;
	}
	else
		usedWeight = 0;

	// laser cooling booster
	if (allowedWeight >= 5)
	{
		ship->globalvars.equip |= EQUIP_LASER_COOLING_BOOSTER;
		usedWeight += 5;
		allowedWeight -= 5;
	}

	spaceAvail -= usedWeight;
	spaceUsed += usedWeight;

	// install shields using whatever's left...
	
	usedWeight = spaceAvail;

	usedWeight = ((u32)usedWeight / 4) * 4; // 4x increments
	
	ship->globalvars.shields = ship->globalvars.max_shields = usedWeight * 16;
	spaceAvail -= usedWeight;
	spaceUsed += usedWeight;

	if (ship_def->Capacity < spaceUsed || shipType == 0xe)
		ship->cargo_space = 0;
	else
		ship->cargo_space = ship_def->Capacity - spaceUsed;

	// equip missiles.

	numPylons = ship_def->Missiles;
	for (i = 0; i < numPylons; i++)
	{
		ship->globalvars.missiles[i] = DATA_AIMissiles[DATA_RandomizerFunc() & 0xf];
	}

	return ship;
}

// returns the exchange rate of a ship (unit: $1000cr).
extern "C" u32 GetShipWorth(ModelInstance_t *ship)
{
	s32 shipCost, i, equipAmt;
	u8 driveType;
	u16 hullMass, intactMass;
	
	ShipDef_t *ship_def;

	ship_def = GetModel(ship->model_num)->Shipdef_ptr;
	shipCost = ship_def->Price * 900;	// used ship

	driveType = ship_def->Drive;
	
	for (i = 0; i < 64; i++)
	{
		if (DATA_EquipmentData[i].id == driveType)
			shipCost -= DATA_EquipmentData[i].buyCost * 9;
	}

	for (i = 0; i < 64; i++)
	{
		equipAmt = FUNC_000392_GetEquipmentAmount((s8*)ship, i);

		// get nothing for integrated drives, or unsellable items
		if (DATA_EquipmentData[i].techLevel < 0xff)
			shipCost += equipAmt * DATA_EquipmentData[i].sellCost * 9;
	}
	
// subtract any hull repair costs

	hullMass = ship_def->Mass * 4;
	intactMass = ship->mass_x4;

	shipCost -= (hullMass - intactMass) * 5;

//	shipCost /= 1000;
//	shipCost *= 1000;

	return shipCost;
}

// Messy function - my apologies
extern "C" void RegenerateHull()
{
	u16 maxHull, hullGain, *metalAlloys, totalAlloys, oldAlloys;

	ShipDef_t *ship_def;
	ModelInstance_t *ship;
	s8 extraAlloys;

	ship = DATA_PlayerObject;

	// need metal supply to repair.
	metalAlloys = DATA_PlayerCargo+0xf;
	oldAlloys = *metalAlloys;

	// keep track with unused space
	extraAlloys = ship->cargo_space;

	totalAlloys = (*metalAlloys)*ALLOY_MULT + extraAlloys;

	if (totalAlloys == 0)
		return;

	ship_def = GetModel(ship->model_num)->Shipdef_ptr;
	maxHull = ship_def->Mass * 4;
	
	// hull regeneration rate here
	hullAccum += DATA_FrameTime * HULL_REGEN_RATE * sqrt(maxHull / 6000.0);

	// transfer integer values.
	hullGain = (u16)hullAccum;
	hullAccum -= hullGain;
	
	if (totalAlloys < hullGain)
	{
		hullGain = totalAlloys;
		totalAlloys = 0;
	}
	else
		totalAlloys -= hullGain;

	extraAlloys = totalAlloys % ALLOY_MULT;
	*metalAlloys = totalAlloys / ALLOY_MULT;

	DATA_PlayerCargoSpace -= (oldAlloys - *metalAlloys);

	ship->mass_x4 += hullGain;
	if (ship->mass_x4 > maxHull)
		ship->mass_x4 = maxHull;

	// set timer to display hull percentage!
	if (hullGain > 0)
		DATA_008886_Unknown = 0x1e;
}

extern "C" void RegenerateShields(ModelInstance_t *ship)
{
	u16 shields, maxShields, shipIdx;
	u32 shieldGain;
	float sfArea, radius, volume, realGain;
	ShipDef_t *ship_def;

	ship_def = GetModel(ship->model_num)->Shipdef_ptr;

	if (!ship_def) return;

	shipIdx = ship->index;
	shields = ship->globalvars.shields;
	maxShields = ship->globalvars.max_shields;

	// get shield surface area
	// cargo space takes up 8x cubic units per tonne
	volume = 2 * ship_def->Mass + 16 * ship_def->Capacity;
	
	radius = pow((0.75/PI) * volume,1/3.0);
	radius++;	// shields spaced 1 unit from hull

	sfArea = 4*PI*radius*radius;

	realGain = (float)DATA_FrameTime * SHIELD_REGEN_RATE * (maxShields / sfArea);	

	if (ship->globalvars.equip & EQUIP_ENERY_BOOSTER_UNIT)
		realGain *= 1.65;

	shieldRechargeAccum[shipIdx] += realGain;

	shieldGain = shieldRechargeAccum[shipIdx];
	shieldRechargeAccum[shipIdx] -= shieldGain;
	
	if ((shields + shieldGain) > maxShields)
		ship->globalvars.shields = maxShields;
	else
		ship->globalvars.shields += shieldGain;
}

extern "C" u32 AIGetMissileToFire(ModelInstance_t *ship)
{
	ModelInstance_t *target;
	u8 targetIdx, mType;
	u16 maxHull, maxCargo, i, n, pylon, neededCargo, neededHull;
	u16 thrust;
	ShipDef_t *ship_def;

	float rMult;

	targetIdx = ship->dest_index;
	target = GetInstance(targetIdx, DATA_ObjectArray);

	ship_def = GetModel(ship->model_num)->Shipdef_ptr;
	maxCargo = ship_def->Capacity;
	maxHull = ship_def->Mass;
	thrust = ship_def->ForwardThrust;

	n = DATA_RandomizerFunc() & 0x7;
	rMult = DATA_RandomizerFunc() / 65536.0;
	maxCargo *= 0.5 + rMult;
	maxHull *= 1.5 - rMult;

	for (i = 0; i < 8; i++, n++)
	{
		pylon = (n & 0x7);
		mType = ship->globalvars.missiles[pylon];

		neededHull = 0;
		switch (mType)
		{
		case 0x82:	// homing missile
			neededCargo = 0;
			neededHull = 0;
			break;
		case 0x83:	// smart missile
			neededCargo = 50;
			neededHull = 100;
			break;
		case 0x84:	// naval missile
			neededCargo = 150;
			neededHull = 200;
			break;
		case 0x85:	// light torpedo
			neededCargo = 900;
			neededHull = 700;
			break;
		case 0x86:	// heavy torpedo
			neededCargo = 900;
			neededHull = 1350;
			break;
		default:
			neededCargo = 0xffff;
			neededHull = 0xffff;
		}

		if (neededCargo <= maxCargo && neededHull <= maxHull)
			break;
	}

	if (i >= 8)
		return 0;
	
	pylon++;
	return pylon;
}

extern "C" u32 GetBounty(ModelInstance_t *ship)
{
	s32 rand, randSeed, randSeed2, cash, cashdigits, cashunit;
	float cashFactor;
	ShipDef_t *ship_def;
	
	if (ship->object_type != OBJTYPE_PIRATE)
		return 0;

	randSeed = ship->globalvars.unique_Id;
	
	randSeed2 = randSeed = (randSeed << 15) | (randSeed >> 17);

	// pseudorandom effect, same all the time
	rand = DATA_FixedRandomFunc(0x10000, &randSeed, &randSeed2) & 0x7fff; 

	if (rand & 0x1)
		return 0;

	ship_def = GetModel(ship->model_num)->Shipdef_ptr;

	cashFactor = 7*exp(ship_def->Price / -2000.0);
	cashFactor = pow((rand&0x7fff)/32768.0, cashFactor);

	cash = cashFactor*maxBounty;

	cashdigits = (s8)(log(cash) / log(10)) - 2;

	cashunit = pow(10, cashdigits);

	if (cashunit < 10)
		cashunit = 10;

	cash = (cash / cashunit) * cashunit;
	
	return cash;
}

// spawns hostile AI ships.
extern "C" u8 SpawnHostileGroup(u8 ships, u8 *shipArray, u32 targetName, u8 shipType, u8 object_type)
{
	ModelInstance_t *shipObj;
	u8 parentShipIdx, i;
//	s8 j;
	u64 tempVec[3];

	if (shipType == 0x13)
		DATA_CustomShipIndex = shipArray[DATA_RandomizerFunc() & 0xf];

	// store grouping vector...
	for (i = 0; i < 3; i++)
		tempVec[i] = DATA_GroupingVector[i];

	// spawn the leader ship.
	shipObj = AIShipSpawn(shipType);
	
	if (shipObj == 0)
		return 0;

	shipObj->dest_index = DATA_PlayerIndex;
	FUNC_000702_GeneratePosition(shipObj, 0x315000);
	shipObj->ai_mode = AI_PIRATE_PREPARE;
	shipObj->target_index = DATA_PlayerIndex;
	shipObj->thrust_power++;
	shipObj->object_type = object_type;

	if (targetName != 0)
	{
		shipObj->bounty = targetName;
		shipObj->cargo_fuel |= 0x8000;
	}

	FUNC_000048_BeginEvents(0x17, 0x0, shipObj->index);

	parentShipIdx = shipObj->index;

	// spawn the follower ships.
	for (i = 1; i < ships; i++)
	{
		if (shipType == 0x13)
			DATA_CustomShipIndex = shipArray[DATA_RandomizerFunc() & 0xf];

		shipObj = AIShipSpawn(shipType+1);
		
		if (shipObj == 0)
			return i;

		shipObj->dest_index= parentShipIdx;
		shipObj->ai_mode = AI_FORMATION;
		shipObj->target_index = DATA_PlayerIndex;
		shipObj->thrust_power++;
		shipObj->target_off_x = BoundRandom(2000) - 1000;
		shipObj->target_off_y = BoundRandom(2000) - 1000;
		shipObj->target_off_z = BoundRandom(2000) - 1000;
		shipObj->object_type = object_type;

		if (targetName != 0)
		{
			shipObj->bounty = targetName;
			shipObj->cargo_fuel |= 0x8000;
		}

		

/*		if (i & 0x1)
			for (j = (i & 0xfe); j >= 0; j--)
				FUNC_001662_Vec64Sub((u64*)(shipObj+0x3e), tempVec);
		else
			for (j = (i & 0xfe); j > 0; j--)
				FUNC_001661_Vec64Add((u64*)(shipObj+0x3e), tempVec); */

		FUNC_000048_BeginEvents(0x17, 0x0, shipObj->index);
	}

	return ships;
}

extern "C" u32 GetTurningRate(s32 baseRate, u32 model)
{
	return baseRate*TurningMultipliers[model];
}

extern "C" void CreateShipyardData(starport_t *starport)
{
	u32 numShips, i, *shipyardArray;

	starport->numShips = 0;
	shipyardArray = starport->shipyard;

	if (DATA_NumStarports > 1)
	{

		numShips = (DATA_NumStarports + 3) / 4;
		for (i = 0; i < numShips; i++)
			FUNC_000634_AddToShipyard(0xd, starport, &shipyardArray);
	}

	numShips = DATA_CurrentPirates*3;
	for (i = 0; i < numShips; i++)
		FUNC_000634_AddToShipyard(0x0, starport, &shipyardArray);

	numShips = DATA_CurrentTraders*3;
	for (i = 0; i < numShips; i++)
		FUNC_000634_AddToShipyard(0x1, starport, &shipyardArray);
}

extern "C" void RefreshShipyardData(starport_t *starport)
{
	u32 idx, *shipyardArray;
	
	// delete 1 ship from the shipyard, maybe.
	idx = BoundRandom(14);
	
	if (idx < starport->numShips)
	{
		memmove(starport->shipyard+idx, starport->shipyard+idx+1, (starport->numShips-idx)*4);
		starport->numShips--;
	}
	
	shipyardArray = starport->shipyard+starport->numShips;

	if (BoundRandom(9) < DATA_CurrentPirates)
		FUNC_000634_AddToShipyard(0x0, starport, &shipyardArray);

	if (BoundRandom(9) < DATA_CurrentTraders)
		FUNC_000634_AddToShipyard(0x1, starport, &shipyardArray);

	if (BoundRandom(17) < DATA_NumStarports)
		FUNC_000634_AddToShipyard(0xd, starport, &shipyardArray);
}

void SpawnHSTrader()
{
    ModelInstance_t *trader = AIShipSpawn(1);
    if(trader != 0) {
        u32 ebp_2 = 0x33;
		FUNC_000702_GeneratePosition(trader,20480);
		trader->object_type = 0x11;
		trader->post_hs_ai = 8;
        if(DATA_NumStarports != 0) {
			ModelInstance_t *starportObj = GetInstance(DATA_StarportArray[BoundRandom(DATA_NumStarports)].objectIdx, DATA_ObjectArray);
			trader->target_index = starportObj->index;
            DATA_RandomizerFunc();
			trader->globalvars.local_Startime = DATA_RandSeed1 << 2;
            trader->globalvars.local_Stardate = DATA_RandSeed1 >> 0x1e;
            if(!(DATA_GameTics & 0x80) && !( trader->globalvars.local_Startime & 0x80)) {
                trader->globalvars.local_Stardate++;
            }
            trader->globalvars.local_Startime += DATA_GameTics;
            trader->globalvars.local_Stardate += DATA_GameDays;
            if(FUNC_000775_FindSourceSystem(trader) == 0) {
				if (DATA_ObjectArray->state_flags[trader->index] == 0x20) {
					DATA_NumObjects--;
				}
				DATA_ObjectArray->state_flags[trader->index] = 0;
            } else {
                DATA_ObjectArray->state_flags[trader->index] = 2;
                FUNC_000048_BeginEvents(0x17, 0, trader->index);
            }
        }
    }
}

extern "C" void SpawnTraders(u32 traderLevel, u32 bInitial)
{
	s32 i, numTraders, curTraders, numFreeSlots;

	numTraders = (sqrt(traderLevel*9)*DATA_NumStarports)/2.25;

	numFreeSlots = GetNumFreeSlots();
	
	// leave some room for extra objects such as cargo
	if (numTraders > numFreeSlots)
		numTraders = numFreeSlots;

	curTraders = numTraders/4;
	// spawn traders incoming via hyperspace
	for (i = 0; i < curTraders; i++)
		FUNC_000689_SpawnHSTrader();

	// spawn thargoids?
	curTraders = BoundRandom(FUNC_000035_GetSpecialShips(0x16));
	for (i = 0; i < curTraders; i++)
		FUNC_000690_SpawnHSTrader2();

	if (!bInitial)
		return;

	// spawn traders already in dock
	curTraders = (2*numTraders)/3;
	for (i = 0; i < curTraders; i++)
		FUNC_000691_SpawnDockedTrader();

	// spawn bulk carriers, etc.
	curTraders = numTraders/12;
	for (i = 0; i < curTraders; i++)
		FUNC_000699_SpawnAuxTrader(); 
}

extern "C" void CreateShips()
{

	scriptSystem* scr = scriptSystem::getSingleton();

	if (scr->getFunction ("CreateShips_callback"))
	{
		scr->callFunction (0, 0);
		return;
	}

	u32 i, mission_idx, name;

	for (i = 0; i < DATA_NumPackages; i++)
	{
		mission_idx = DATA_PackageArray[i].mission_idx;

		// military delivery mission?
		if (DATA_PackageArray[i].passengers == 0 && mission_idx >= 0x20)
		{
			if (mission_idx & 0x40)
			{
				mission_idx = 0x21;
				name = 0xffffffff;
			}
			else
			{
				mission_idx = 0x20;
				name = 0xfffffffe;
			}
		}
		else
		{
			mission_idx >>= 1;
			name = DATA_PackageArray[i].name;
		}
		
		if (DATA_PackageArray[i].system == DATA_CurrentSystem)
			SpawnAssassins(DATA_PackageArray[i].ships, mission_idx, name);
		else
			SpawnAssassins(sqrt(DATA_PackageArray[i].ships), mission_idx, name);
	}

	SpawnPirates(DATA_CurrentPirates, 1);
	SpawnTraders(DATA_CurrentTraders, 1);
}

extern "C" void RefreshShips()
{
	SpawnPirates(DATA_CurrentPirates, 0);
	SpawnTraders(DATA_CurrentTraders, 0);
}