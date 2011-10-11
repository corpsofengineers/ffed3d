#include <vector>
#include <process.h>
#include "xmath.h"
#include "XFile/Utility.h"
#include "XFile/XfileEntity.h"
#include "ffetypes.h"

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

extern ModelInstance_t *GetInstance(int index, InstanseList_t *list);
extern Model_t *GetModel(int index);

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

struct ffeVector {
	int x, y, z;
};

struct ffePoint {
	int v1x, v1y, v1z; // 0, 4, 8
	int v2x, v2y, v2z; // 12, 16, 20
	int v3x, v3y, v3z; // 24, 28, 32
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

struct xModel
{
	int id;
	std::string filename;
	CXFileEntity* object;
	LPDIRECT3DTEXTURE9 skin[10];
	MODELCONFIG config;
	__int64 counter;
	bool needLoad;
	bool haveFile;

	xModel(int idx)
	{
		id = idx;
		filename="";
		object = NULL;
		for(int i=0;i<10;i++)
			skin[i] = NULL;
		counter = 0;
		needLoad = false;
		haveFile = false;
	}

	int xModel::GetNumAnimationSets()
	{
		return object->GetNumAnimationSets();
	}

	void xModel::SetAnimationSet(unsigned int index)
	{
		return object->SetAnimationSet(index);
	}

	double xModel::GetAnimationSetLength(unsigned int index)
	{
		return object->GetAnimationSetLength(index);
	}

	void xModel::FrameMove(bool mode, float elapsedTime,const D3DXMATRIX *matWorld)
	{
		return object->FrameMove(mode, elapsedTime, matWorld);
	}

	void xModel::SetEffect(ID3DXEffect* effectF)
	{
		return object->SetEffect(effectF);
	}

	void xModel::Render()
	{
		return object->Render();
	}

	bool Exist()
	{
		if (needLoad == false && object)
			return true;
		else
			return false;
	}

	void loadSkins()
	{
		if (object) {
			char buf[1000];
			std::string dir;
			for(int j=0; j<10; j++) 
			{
				dir = CUtility::GetTheCurrentDirectory();
				if (j==0)
					sprintf_s(buf,"%s\\models\\%i\\skin.png",dir.c_str(),id);
				else
					sprintf_s(buf,"%s\\models\\%i\\skin%i.png",dir.c_str(),id,j);
				if (CUtility::DoesFileExist(buf)) {
					if (FAILED(D3DXCreateTextureFromFile(renderSystem->GetDevice(), buf, &skin[j]))) {
						skin[j]=NULL;
					}
				}
			}
		}
	}

	void DropModel()
	{
		if (object) {
			delete object;
			object = NULL;
		}
		for(int i=0;i<10;i++) {
			if (skin[i]) {
				skin[i]->Release();
				skin[i] = NULL;
			}
		}
	}

	~xModel()
	{
		DropModel();
	}
};