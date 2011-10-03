#pragma once
#include <vector>
#include "xmath.h"

#define MAXVERT	3000000

#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_DIFFUSE|D3DFVF_TEX1)

/* Primitives */
#define GL_POINTS				0x0000
#define GL_LINES				0x0001
#define GL_LINE_LOOP			0x0002
#define GL_LINE_STRIP			0x0003
#define GL_TRIANGLES			0x0004
#define GL_TRIANGLE_STRIP		0x0005
#define GL_TRIANGLE_FAN			0x0006
#define GL_QUADS				0x0007
#define GL_QUAD_STRIP			0x0008
#define GL_POLYGON				0x0009
#define GL_POINTS2				0x000A

// Materials
#define NONE					0x00
#define SUN						0x01
#define PLANET					0x02
#define ATMO					0x03
#define ATMOSTAR				0x04
#define TRANSP  				0x05
#define ATMO2					0x06
#define GALAXY					0x0A
#define SPLINE					0x0B
#define NOTDRAW					0x0F

// Cull
#define CULL_NONE				0x01
#define CULL_CW					0x02
#define CULL_CCW				0x03

struct CUSTOMVERTEX {
	D3DXVECTOR3 p;	// позиция вершины
	D3DXVECTOR3 n; //Lighting Normal
	DWORD color;		// Цвет вершины
	FLOAT tu,tv;

};

struct MODELCONFIG {
	float scale, scale_x, scale_y, scale_z;
	float offset_x,offset_y,offset_z;
	int cullmode;
	int notdrawtext;
	int notdrawsubmodels;
	int skip;
	float missile_scale;
	D3DXVECTOR3 missile[10];
};

struct FFELIGHT {
	bool enable;
	float r;
	float g;
	float b;
	D3DXVECTOR3 pos;
	float dist;
};

struct MODEL {
	int index;
	bool subObject;
	D3DXMATRIX world;
	char doMatrix;
	bool zwrite;
	bool zenable;
	bool zclear;
	bool backsprite;
	D3DXVECTOR3 lightPos;
	unsigned char ambientR;
	unsigned char ambientG;
	unsigned char ambientB;
	unsigned char splineR;
	unsigned char splineG;
	unsigned char splineB;
	unsigned char localR;
	unsigned char localG;
	unsigned char localB;
	int vertStart;
	int vertEnd;
	char material;
	char cull;
	unsigned char missile[10];
	float dist;
	FFELIGHT light[10];
	unsigned short landingGear;
};

struct VERTEXTYPE {
	short type;
	short amount;
	int textNum;
	int vertCount;
	bool doLight;
	float radius;
	bool transparent;
	bool specular;
	D3DXVECTOR3 tangent;
};

struct FFTEXT {
	int x,y;
	DWORD color;
	char text[1000];
};

struct PANELCLICKAREA {
	int x1, y1;
	int x2, y2;
	int index;
};

struct Triangle {
	int p1,p2,p3;
};

struct ShipDef_t {
    short ForwardThrust;
    short RearThrust;
    char  Gunmountings;
    char  FuelScoop;
    short Mass;
    short Capacity;
    short Price;
    short Scale;
    short Description;
    short Crew;
    short Missiles;
    char  Drive;
    char  IntegralDrive;
    short EliteBonus;
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

struct ffeVector {
	int x, y, z;
};

struct ffePoint {
	int v1x, v1y, v1z; // 0, 4, 8
	int v2x, v2y, v2z; // 12, 16, 20
	int v3x, v3y, v3z; // 24, 28, 32
};

struct ffeMatrix {
	__int64 _11, _12, _13;
	__int64 _21, _22, _23;
	__int64 _31, _32, _33;
};
/*
struct ffeVertex {
	short orto_x;	// 0				36
	short orto_y;	// 2				38
	int nx, ny, nz; // 4, 8, 12			40, 44, 48
	int unknown5;	// 16				52
	int unknown6;	// 20 - buf flag?	56
	int x, y, z;	// 24, 28, 32		60, 64, 68
};
*/
struct ffeVertex {
	short orto_x;	// 0
	short orto_y;	// 2
	int nx, ny, nz; // 4, 8, 12
	int unknown5;	// 16
	int unknown6;	// 20
	int x, y, z;	// 24, 28, 32

	short orto_x_2;						// 36
	short orto_y_2;						// 38
	int normal_x, normal_y, normal_z;	// 40, 44, 48
	int unknown5_2;						// 52
	int unknown6_2;						// 56
	int x_2;	// 60
	int y_2;	// 64
	// до какой глубины деления делить
	int z_2;	// 68

};

struct customVertex {
	Vect3f v;
	Vect3f n;
	int color;
};

struct FFEsprite
{
	D3DXVECTOR2 pos;
	RECT rect;
	int tex;
};

struct UnknownStruct1
{
	int var1;
	int var2;
	int var3;
	int var4;
	int var5;
	int var6;
};

struct Sprite
{
	Sprite(int x1_,int y1_,int x2_,int y2_, int tex_)
	{
		x1 = x1_;
		y1 = y1_;
		x2 = x2_;
		y2 = y2_;
		tex = tex_;
	}
	Sprite()
	{
		x1 = 0;
		y1 = 0;
		x2 = 0;
		y2 = 0;
		tex =0;
	}
	int x1,y1,x2,y2,tex;
};

extern std::vector<Sprite> sprites;

