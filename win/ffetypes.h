
typedef __int8  s8;
typedef __int16 s16;
typedef __int32 s32;
typedef __int64 s64;

typedef unsigned __int8  u8;
typedef unsigned __int16 u16;
typedef unsigned __int32 u32;
typedef unsigned __int64 u64;

struct ffeRGB {
	u8 r, g, b;
};

struct ffeMatrix3x3 {
	int _11, _12, _13;
	int _21, _22, _23;
	int _31, _32, _33;
};

struct Point64 {
	s64 x;
	s64 y;
	s64 z;
};

struct Point32 {
	s32 x;
	s32 y;
	s32 z;
};

struct ScreenCoord_t {
	u16 x, y;
};

struct ShipDef_t {
    short ForwardThrust;		// 0x00
    short RearThrust;			// 0x02
    char  Gunmountings;			// 0x03
    char  FuelScoop;			// 0x04
    short Mass;					// 0x06
    short Capacity;				// 0x08
    short Price;				// 0x0a
    short Scale;				// 0x0c
    short Description;			// 0x0e
    short Crew;					// 0x10
    short Missiles;				// 0x12
    char  Drive;				// 0x14
    char  IntegralDrive;		// 0x15
    short EliteBonus;			// 0x17
    short frontMount_x, frontMount_y, frontMount_z;
    short backMount_x, backMount_y, backMount_z;
    short leftMount_x, leftMount_y, leftMount_z;
    short rightMount_x, rightMount_y, rightMount_z;
};


struct Model_t {
    unsigned short * Mesh_ptr;
    signed char *    Vertices_ptr;
    int              NumVertices;
    signed char *    Normals_ptr;
    int              NumNormals;
    int              Scale;
    int              Scale2;
    int              Radius;
    int              Primitives;
    char	         DefaultColorR;
	char	         DefaultColorG;
	char	         DefaultColorB;
    char             padding;
    int              field_28;
    int              field_2C;
    int              field_30;
    unsigned short * Collision_ptr;
    ShipDef_t *      Shipdef_ptr;
    int              DefaultCharacter;
    unsigned short * Character[1];
};

typedef union 
{
  struct {
	u16 time;					// FIRST GLOBAL VAR, starTime >> 10
	u16 landingState;			// VAR1:Starmap Display:10=Trade;80=Text; 
								// 0 < <0x20 is cargo type; & 1: landing gear down
	u32 unique_Id;
	u16 num_stars_in_sector;	// GLOBAL[4]; may also be dword:forced scale?
								// may also be:laser color selector
	u8 TypeOfStar;				// for star sector model
	u8 NumMultipleStar;			// for star sector model
	u32 local_Startime;
	u32 local_Stardate;
	u16 ScaledWord_B0;			// planet_t.orbitalradius
	u16 ushort_B2;				// angle?
	u16 ushort_B4;				// rotation value x shr 16?
	u16 ushort_B6;				// rotation value y shr 16?
								// current top thrust?
	u16 ushort_B8;				// rotation value z shr 16?
								// current side thrust?
	u16 Current_Main_Thrust;
	u16 Thrust_BC;				// = minus rear thrust from model
	u16 Thrust_BE;				// = rear thrust from model
								// planet: dword = orbit angle << 10h
	u16 Thrust_C0;				// = minus rear thrust from model
	u16 Thrust_C2;				// = rear thrust from model
	u16 Max_Main_Thrust;
	u16 Thrust_C6;				// = rear thrust from model
								// planet_t.longitude
	u8 BitwiseEquip_0;			// planet_t.rotspeed
								// 04=Camera
								// 20=Navigation Computer
								// 40=Combat Computer
	u8 BitwiseEquip_1;			// Also Traffic Control Busy flag #1
								// FFh=Busy with Vipers
								// 02=Missile Viewer
								// 08=Chaff dispenser
	u8 BitwiseEquip_2;			// 02=Cargo Bay Life Support
								// 04=Scanner (draws the stalks)
								// 08=(!? ECM? See Con_UpdateVariousButtons)
								// 10=Fuel Scoop ??
								// 20=Cargo Scoop ??
								// 40=Radar mapper
								// 80=Naval ECM
								// Also Traffic Control Busy with model #
	u8 BitwiseEquip_3;			// 01:Hyp.Analyser
								// 04:Energy Bomb
								// 40:Cargo scoop?
								// planet_t.field_3f
	u16 negative_equipment_bits;// [!!??] gets un-set when you get a camera
								// planet_t.field42 (local rotation?)
	u16 TurretRot_1;			// ?? cant be rite. For Starports: WORD num_landingpads
	u8 current_hyperdrive;		// for starports: 1st of landing pads in use
	u8 num_gunmountings;		// from model
	u8 FrontGun;
	u8 RearGun;
	u8 TopTurretGun;			// in primitives: Ball Radius
	u8 BottomTurretGun;
	union 
	{
		struct
		{
			u8 missile_0;
			u8 missile_1;
			u8 missile_2;				// Also Traffic Control ready time?
			u8 missile_3;
			u8 missile_4;
			u8 missile_5;
			u8 missile_6;				// word:number of Vipers in starport
			u8 missile_7;
			u8 missile_8;				// Starport:dw 0 ->Launched Vipers
			u8 missile_9;
		};
		u8 missiles[10];
	};
	u16 shields;
	u16 max_shields;
  };
  s16 raw[36];
} GlobalVars;

struct ModelInstance_t {
	ffeMatrix3x3 rotMatrix;
	u8 uchar_24;			// set to 3Ch?
	u8 uchar_25;
	Point64 pos;
	Point64 rel_pos;		// relative position from owner
	u8 parent_index;		// Parent object of current
	u8 uchar_57;			// 1 if ON a planet? (eg., city, ship?)
	u8 uchar_58;
	u8 uchar_59;
	u8 uchar_5A;
	u8 uchar_5B;
	u8 uchar_5C;
	u8 uchar_5D;
	u8 uchar_5E;
	u8 uchar_5F;
	u8 uchar_60;
	u8 uchar_61;
	u8 uchar_62;
	u8 uchar_63;
	u8 uchar_64;
	u8 uchar_65;
	u8 uchar_66;
	u8 uchar_67;
	u8 uchar_68;
	u8 uchar_69;
	u8 uchar_6A;
	u8 uchar_6B;
	u32 field_6C;
	u8 uchar_70;
	u8 uchar_71;
	u8 uchar_72;
	u8 uchar_73;
	u8 uchar_74;
	u8 uchar_75;
	u8 uchar_76;
	u8 uchar_77;
	u8 uchar_78;
	u8 uchar_79;
	u8 uchar_7A;
	u8 uchar_7B;
	u8 uchar_7C;
	u8 uchar_7D;
	u32 model_scale;
	u16 model_num;
	u8 ambient_color_r;			// UNSURE! OVERLAPS OTHERSTUFF
	u8 uchar_85;				// undefined
	u8 index_number;			// of THIS model instance in list
	u8 uchar_87;				// if this is NOT 0Bh, no aggressive response? 
								// for planets: const 1
	u8 dist;					// distance?? from current target? [see explosion, mining machine]
	u8 uchar_89;				// undefined
	u8 uchar_8A;
	u8 uchar_8B;
	Point32 Point3d_t;			// ??
	ScreenCoord_t screenpos;
	GlobalVars globalvars;
	u16 mass_x4;				// from model, times 4
	u16 orgModelShip;			// for hyperspace warp
	u16 ushort_E8;
	u16 ushort_EA;				// fuel in tank?
	u16 ushort_EC;				// short x?
	u16 ushort_EE;				// short y?
	u16 vertex_level;			// Level counter for this model
	u8 HyperCloudSource;
	u8 uchar_F3;				// undefined
	ffeRGB color;
	u8 uchar_F7;
	u16 ushort_F8;
	u32 ulong_FA;				// random id?
	u8 destinationIndex;
	u8 attackFlag;				// Routine at 4DECC0 to call
								// 4 if attacking this?
	u8 targetIndex;				// another index number?
	u8 thrustPower;				// 0, 1, 2 for more powerful reverse thrusts?
	u16 ushort_102;
	u16 ushort_104;
	u16 ushort_106;
	u16 ushort_vx;
	u16 ushort_vy;
	u16 ushort_vz;
	u16 ushort_10E;				// vector scale?
	u16 ushort_110;
	u16 ushort_112;
	u16 ushort_114;
	u16 cargoAmount;
	u8 ai;						// 1 if not aggressive? [on 'haha' answer]
								// FB=Agressive (may have bonus)
								// FF=This is the player?
	u8 cargo_free_space;					// other ship's fuel amount?
	u32 destroyBonus;
	u32 ulong_11E;				// Amount of fuel in tank?
	u8 field_122[2];			// 2 dup(?)
	u8 name[20];				// 20 bytes
	u64 int64_138;
	u32 int64lo_140;
	u32 int64hi_144;
	u32 org_model_scale;		// Original model scale?
	u8 flags_14C;				// 10h -> can dock
								// 20h -> is orbiter
								// 40h -> has external pads
								// & 7: Star Ambient color number
	u8 uchar_14D;
	u8 uchar_14E;				// 6 for cargo pod
	u8 uchar_14F;				// == 2 for ECM/Naval ECM
								// == 7 for Energy Bomb
	u8 uchar_150;
	u8 flags_151;				// which guns are firing??
								// 20h:ECM/Naval ECM
								// 40h:Energy Bomb
};

struct DrawMdl_t {
	Model_t* model3d;			// Pointer to actual 3D Model_t used
	Point32 pos;				// actual center position
	u8 uchar10;					// undefined
	u8 uchar11;					// undefined
	u8 uchar12;					// undefined
	u8 uchar13;					// undefined
	ffeMatrix3x3 rotMatrix;
	Point32 ldir;				// might be Lighting vector!
	u32 scale;
	Point32 Vector;
	s32 localvars[7];
	u32 lpExtraVertex_Center;
	u32 lpExtraVertex_0;		// ..for subobjects
	u32 lpExtraVertex_1;		// ..for subobjects
	u32 lpExtraVertex_2;		// ..for subobjects
	u32 lpExtraVertex_3;		// ..for subobjects
	ffeRGB rgb3array[8];
	ffeMatrix3x3 altMatrix;
	Point32 textPoint;
	u32 packedcolor;			// 0000:0R:GB (pos in color cube)
	u8 ucharD0;					// undefined
	u8 ucharD1;					// undefined
	u8 ucharD2;					// undefined
	u8 ucharD3;					// undefined
	u32 radius;					// Last actual (shifted) radius for single ball
	u8 ucharD8;					// undefined
	u8 ucharD9;					// undefined
	u8 ucharDA;					// undefined
	u8 ucharDB;					// undefined
	u8 ucharDC;					// undefined
	u8 ucharDD;					// undefined
	u8 ucharDE;					// undefined
	u8 ucharDF;					// undefined
	u8 ucharE0;					// undefined
	u8 ucharE1;					// undefined
	u8 ucharE2;					// undefined
	u8 ucharE3;					// undefined
	u8 ucharE4;					// undefined
	u8 ucharE5;					// undefined
	u8 ucharE6;					// undefined
	u8 ucharE7;					// undefined
	u8 ucharE8;					// undefined
	u8 ucharE9;					// undefined
	u8 ucharEA;					// undefined
	u8 ucharEB;					// undefined
	u8 ucharEC;					// undefined
	u8 ucharED;					// undefined
	u8 ucharEE;					// undefined
	u8 ucharEF;					// undefined
	u32 counter;				// holds current vertexcalc level
	ffeRGB localColor;
	u8 ucharF7;					// undefined
	u32 forceMirror;
	u32 forceMirror_alt;
	u8 uchar100;				// undefined
	u8 uchar101;				// undefined
	u8 uchar102;				// undefined
	u8 uchar103;				// undefined
	u32 field_104;
	ScreenCoord_t screenpos1;
	u32 field_10C;
	ScreenCoord_t screenpos2;
	u8* lpVertexCache;
	u32 field_118;				// current num of calc. normals?
	Point32 tempVector;			// Rotated normal value??
	u32 field_128;				// -1 if end drawing?
	u32 field_12C;
	u32 scale2;					// init to 0, for planets to Scale2
	Point32 lpos;				// light source position
	u8 uchar140;				// undefined
	u8 uchar141;				// undefined
	u8 uchar142;				// undefined
	u8 uchar143;				// undefined
	u8 uchar144;				// undefined
	u8 uchar145;				// undefined
	u8 uchar146;				// undefined
	u8 uchar147;				// undefined
	u32 actualScale;			// ..from Model_t
	ModelInstance_t* modelInstance;
	u8* ptr_ModelVertices;
	u8* ptr_ModelNormals;
	u8 uchar158;				// undefined
	u8 uchar159;				// undefined
	u8 uchar15A;				// undefined
	u8 uchar15B;				// undefined
};
