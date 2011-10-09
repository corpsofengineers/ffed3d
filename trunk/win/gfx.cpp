#include <windows.h>
#include <ddraw.h>
#include <stdio.h>
#include <math.h>
#include <direct.h>

#include <d3dx9.h>

#include "../ffeapi.h"
#include "../ffecfg.h"
#include "win32api.h"
#include "ffe3d.h"
#include "Xfile/XFileEntity.h"
#include "Render/RenderSystem.h"
#include <GL/glu.h>

#define u32 unsigned long
#define int32 signed long
#define u16 unsigned short
#define u8 unsigned char

#define N_0 0.0f
#define N_1 1.0f
#define WIDTH 640
#define HEIGHT 400
#define DIVIDER 10000

const bool callAsm=true;
const bool splineExport=false;

bool skipCurrentModel=false;
bool subObject=false;

float GetPlanetRadius (void *planet);

extern "C" ffeVertex *DATA_009200; // Vertex buffer?
extern "C" char *DATA_007892_Icosahedron;
extern "C" char *DATA_008861;
extern "C" char *DATA_006821;
extern "C" unsigned char *DATA_008874;
extern "C" PANELCLICKAREA *DATA_007684;
extern "C" int *DATA_008804; // game time
extern "C" unsigned int *instanceList; // 0x006ab9c0

extern "C" ffeVertex* FUNC_001470_getVertex(void *a, int num);
extern "C" ffeVertex* FUNC_001471(void *a, int num);
extern "C" ffeVertex* FUNC_001472(void *a, int num);
extern "C" int FUNC_001473_getVar(void *a, int cmd);
extern "C" void* FUNC_001758_doMath(void *a, void *b, short cmd);
extern "C" unsigned short* FUNC_001756_SkipIfBitClear(void *a, void *b, short cmd);
extern "C" unsigned short* FUNC_001757_SkipIfBitSet(void *a, void *b, short cmd);
extern "C" unsigned short* FUNC_001752_SkipIfNotVisible(void *a, void *b, short cmd);
extern "C" unsigned short* C_FUNC_001752_SkipIfNotVisible(char *gptr, unsigned short *var2, unsigned short var1);
extern "C" unsigned short* FUNC_001755_SkipIfVisible(void *a, void *b, short cmd);
extern "C" unsigned short* C_FUNC_001755_SkipIfVisible(char *gptr, unsigned short *var2, unsigned short var1);
extern "C" unsigned short* FUNC_GraphNull(void *a, void *b, short cmd);
extern "C" unsigned short FUNC_001474_getRadius(void *a, int cmd);
extern "C" int FUNC_000853_GetNumstars(int,int,int);
extern "C" unsigned char * FUNC_001532_GetModelInstancePtr(unsigned char, void *);
extern "C" Model_t * FUNC_001538_GetModelPtr(int);

//extern "C" char *FUNC_001344_StringExpandFFCode(char *dest, int ffcode, StrVars *vars);
//extern "C" char *FUNC_001345_StringExpandFF40Code(char *dest, int stridx);

extern int C_FUNC_001874_DrawPlanet(char *DrawMdl, char *cmd);

#define _GLUfuncptr void(__stdcall*)(void)

#define MIN(X,Y)  (((X) < (Y)) ? (X) : (Y))
#define MAX(X,Y)  (((X) > (Y)) ? (X) : (Y))

extern int aspectfix;

unsigned char Ambient2[7][8][3] = {
    0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0,  // none
    7,7,7, 7,7,7, 7,7,7, 6,6,6, 5,5,5, 4,4,4, 3,3,3, 2,2,2,  // default
    7,6,3, 7,5,2, 7,4,2, 6,3,1, 5,2,0, 4,1,0, 3,0,0, 2,0,0,  // Red
    7,7,5, 7,6,4, 7,5,2, 6,4,0, 5,3,0, 4,2,0, 3,2,0, 2,1,0,  // Orange
    7,7,7, 7,7,7, 6,6,6, 5,5,5, 4,4,4, 3,3,3, 2,2,2, 1,1,1,  // White
    7,7,7, 7,7,7, 6,7,7, 6,6,7, 5,5,7, 4,4,7, 3,3,6, 2,2,4,  // Cyan
    0,6,7, 0,5,7, 0,4,7, 0,3,7, 0,2,6, 0,1,5, 0,0,4, 0,0,2   // Blue
};

unsigned char Ambient[7][3] = {
    0,0,0,  // none
	7,7,7,  // 
    6,3,1,  // Red
    6,4,0,  // Orange
    5,5,5,  // White
    6,6,7,  // Cyan
    0,3,7   // Blue
};

unsigned char starColors[11][3] = {
    15,6,4,		// Type 'M' red star
	15,6,4,		// Giant red star
    15,9,5,		// Type 'K' orange star
    15,15,6,	// Type 'G' yellow star
    15,15,15,	// Type 'F' white star
    13,15,15,	// Type 'A' hot white star
    7,15,15,	// Type 'B' hot blue star
	15,11,6,	// Bright giant star
	14,9,2,		// Supergiant star
	7,15,15,	// Blue supergiant star
	8,13,15		// White dwarf star
};

//extern LPDIRECT3DDEVICE9 renderSystem->GetDevice();
std::vector<Sprite> sprites;
//extern LPDIRECT3DVERTEXBUFFER9 d3d_vb;
extern sVertexBuffer vertexBuffer;
CUSTOMVERTEX *Vertices;
extern VERTEXTYPE vertexType[MAXVERT];
extern int vertexNum;
extern MODEL modelList[6000];
extern int modelNum;
int mainModelNum=0;
int mainObjectNum=0;
int previousModel=0;
extern FFTEXT ffText[2000];
extern int textNum;
extern ULONG pWinPal32[256];
extern FFEsprite spriteList[400];
extern int maxsprite;
D3DXVECTOR3 mainModelCoord;
extern D3DXVECTOR3 playerLightPos;
extern bool playerLightPosEnable;

extern xModel* objectList[500];

extern "C" unsigned int **textColor;
extern "C" unsigned char **ambColor;
extern "C" void **model;
extern "C" void **textPool;
extern "C" unsigned int **DATA_007824;

unsigned int t_DATA_007824;

//extern "C" void C_PlaceStation (int *starport, int lat, int lon, int *objectList);

// Используются для перехвата в 2D
extern "C" void C_DrawScannerMarker(int x1, int y1, int x2, int y2, int col);
extern "C" void C_DrawHLine(int x, int y, int len, int color);
extern "C" void C_DrawCircle (u16 *center, int radius, int color);
extern "C" void C_DrawLine(u16 *point1, u16 *point2, int col);
extern "C" void C_DrawTriangle(u16 *point1, u16 *point2, u16 *point3, int color);
extern "C" void C_DrawQuad(u16 *point1, u16 *point2, u16 *point3, u16 *point4, int color);
void DrawTriangle(u16 *point1, u16 *point2, u16 *point3, u16 *t1, u16 *t2, u16 *t3, int color, int text, int z, bool intern);

Vect3f getNormal(unsigned char num);
Vect3f getReNormal(unsigned char num);
Vect3f GetTriangeNormal(Vect3f* vVertex1, Vect3f* vVertex2, Vect3f* vVertex3);

int textStartX=-1;
char *C_DrawText (char *pStr, int xpos, int ypos, int col, bool shadow, int ymin, int ymax, int height);

extern void RenderDirect3D(void);

extern bool awaitingRender;

struct Model_t *mod;

char *gptr;
char *iptr;
unsigned short *gcmd;

float expVert[3];
float expVertOrigin[3];
float prevPoint[3];

D3DXMATRIX camRot, camPos;
D3DXVECTOR3 camForward, camUp;
extern int range;
bool draw2D;

extern int curwidth;
extern int curheight;

unsigned char localColor[3];
Color4c currentColor(0,0,0,0);
int currentR=0;
int currentG=0;
int currentB=0;
int currentTex;
bool doLight=true;
bool useLocalColor=false;
short *globalvar;
short globalvar0;
int currentModel;
int t_localvar[7];
int p_localvar[7];
int *localvar;
D3DXMATRIX posMatrix, mainRotMatrix, mainRotMatrixO, currentMatrix;
D3DXMATRIX lightRotMatrix;
int currentScale;
int currentScale2;
int currentNormal;
unsigned char currentAmbientR=15;
unsigned char currentAmbientG=15;
unsigned char currentAmbientB=15;
DWORD fullAmbientColor;
D3DXVECTOR3 curLightPos;
D3DXVECTOR3 sunLightPos;
bool transparent;

int mxVertices;
D3DXVECTOR3 mVertices[MAXVERT];
int mxIndexes;
int mIndexes[MAXVERT];

bool exportb[500];

bool billboardMatrix;
float lastPlanetRadius;
int incabin;
extern bool panellight[20];
bool clearBeforeRender=false;

extern "C" void C_clearAll() {
	clearBeforeRender=true;
}

#define COUNTER_CLOCKWISE 0
#define CLOCKWISE 1

/*
 * orientation
 *
 * Return either clockwise or counter_clockwise for the orientation
 * of the polygon.
 */

static double ver[2000][3];
static double ver2[2000][3];

float vertbuf[256][4];
int vcount=0;

#define D3DFVF_MESH ( D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1)

bool ExportMesh(int exp) {
	if (mxIndexes==0)
		return false;

    ID3DXMesh* pMesh = NULL;
    D3DXCreateMeshFVF( mxIndexes / 3, mxVertices, 
                              D3DXMESH_MANAGED, D3DFVF_MESH, 
                              renderSystem->GetDevice(), &pMesh );

    BYTE *pVertex=NULL;
    pMesh->LockVertexBuffer( 0, (void**)&pVertex );
	DWORD vertSize=D3DXGetFVFVertexSize(D3DFVF_MESH);
	for(int i=0;i<mxVertices;i++) {
		D3DXVECTOR3 *vPtr=(D3DXVECTOR3 *) pVertex;
		vPtr->x=mVertices[i].x;
		vPtr->y=mVertices[i].y;
		vPtr->z=mVertices[i].z;
		pVertex+=vertSize;
	}
    pMesh->UnlockVertexBuffer();

    BYTE *pIndex=NULL;
    pMesh->LockIndexBuffer( 0, (void**)&pIndex );
	DWORD indSize=sizeof(USHORT);
	for(int j=0;j<mxIndexes;j++) {
		USHORT *iPtr=(USHORT *) pIndex;
		*iPtr=mIndexes[j];
		pIndex+=indSize;
	}
    pMesh->UnlockIndexBuffer();

	D3DXComputeNormals(pMesh, NULL); 

	char buf[200];
	sprintf(buf,"models\\_export");
	_mkdir(buf);
	sprintf(buf,"models\\_export\\%i",exp);
	_mkdir(buf);
	sprintf(buf,"models\\_export\\%i\\export.x",exp);
	D3DXSaveMeshToX(buf, pMesh, NULL, NULL, NULL, 0, D3DXF_FILEFORMAT_TEXT);
	sprintf(buf,"models\\%i\\exportflag",exp);
	remove(buf);

	return true;
}

char* C_GetModelInstancePtr(int indexNum, void *list)
{
	return (char*)(indexNum*338+(int)list+116);
}

int findVertex(Vect3f *curpoint) {
	for(int i=0;i<mxVertices;i++) {
		if (mVertices[i].x==curpoint->x && mVertices[i].y==curpoint->y && mVertices[i].z==curpoint->z) {
			return i;
		}
	}
	return -1;
}

void newIndex(Vect3f *curpoint) {
	//int f=findVertex(curpoint);
	//if (f>-1) {
	//	mIndexes[mxIndexes]=f;
	//	mxIndexes++;
	//} else {
		mVertices[mxVertices].x=curpoint->x;
		mVertices[mxVertices].y=curpoint->y;
		mVertices[mxVertices].z=curpoint->z;
		mIndexes[mxIndexes]=mxVertices;
		mxVertices++;
		mxIndexes++;
	//}
}

bool checkOrientation(float *p1, float *p2, float *p3) {
	/*
	float x1=p2[0]-p1[0];
	float y1=p2[1]-p1[1];
	float z1=p2[2]-p1[2];
	float x2=p3[0]-p1[0];
	float y2=p3[1]-p1[1];
	float z2=p3[2]-p1[2];

	float cf = (x1*x2 + y1*y2 + z1*z2) / sqrtf ((x1*x1 + y1*y1 + z1*z1) * (x2*x2 + y2*y2 + z2*z2));
	float ccf = acos(cf);
	if (cvf<0) return false;
*/
	
	Vect3f vNormal, oldNormal;
	float checkx, checky, checkz;

	vNormal = GetTriangeNormal(&Vect3f(p1[0], p1[1], p1[2]),
					&Vect3f(p2[0], p2[1], p2[2]),
					&Vect3f(p3[0], p3[1], p3[2]));


	oldNormal=getNormal(currentNormal);

	checkx = (oldNormal.x*vNormal.x)==-0 ? 0 : (oldNormal.x*vNormal.x);
	checky = (oldNormal.y*vNormal.y)==-0 ? 0 : (oldNormal.y*vNormal.y);
	checkz = (oldNormal.z*vNormal.z)==-0 ? 0 : (oldNormal.z*vNormal.z);

	if (checkx<0 || checky<0 || checkz<0)
		return false;

	return true;
}

// Попытки расчета ориентации полигона
int orientation(int s, int n)
{
    double area;
    int i;

    /* Do the wrap-around first */
    area = ver[n-1][2] * ver[s][1] - ver[s][2] * ver[n-1][1];

    /* Compute the area (times 2) of the polygon */
    for (i = s; i < n-2; i++) {
		area += ver[i][2] * ver[i+1][1] - ver[i+1][2] * ver[i][1];
	}

    if (area >= 0.0)
        return COUNTER_CLOCKWISE;
    else
        return CLOCKWISE;
} /* End of orientation */

int orientation2(int s, int n)
{
    double area;
    int i;

    /* Do the wrap-around first */
    //area = ver[n-1][0] * ver[s][1] - ver[s][0] * ver[n-1][1];
	//area = (x2 - x1) * (y3 - y1) - (x3 - x1) * (y2 - y1);
	area = 0;

    /* Compute the area (times 2) of the polygon */
    for (i = s; i < n-3; i++)
		area += (ver[i+1][0] - ver[i][0]) 
				* (ver[i+2][1] - ver[i][1]) 
				- (ver[i+2][0] - ver[i][0]) 
				* (ver[i+1][1] - ver[i][1]);
        //area += ver[i][0] * ver[i+1][1] - ver[i+1][0] * ver[i][1];

    if (area >= 0.0)
        return COUNTER_CLOCKWISE;
    else
        return CLOCKWISE;
} /* End of orientation */

void copyMatrix(D3DXMATRIX src, D3DXMATRIX *dst) {
	dst[0][0]=src[0];
	dst[0][1]=src[1];
	dst[0][2]=src[2];
	dst[0][3]=src[3];
	dst[0][4]=src[4];
	dst[0][5]=src[5];
	dst[0][6]=src[6];
	dst[0][7]=src[7];
	dst[0][8]=src[8];
	dst[0][9]=src[9];
	dst[0][10]=src[10];
	dst[0][11]=src[11];
	dst[0][12]=src[12];
	dst[0][13]=src[13];
	dst[0][14]=src[14];
	dst[0][15]=src[15];
}

#define INTERPOLATE(a,b,t) ((1.0f-t)*(a) + t*(b))

// Два варианта расчета кривой Безье
// Первый...
void eval_bezier2( float *out, float t, float ctrlpoints[4][3]) {
    float left, right, mid, left_mid, mid_right;

	// x
    left = INTERPOLATE(ctrlpoints[0][0], ctrlpoints[1][0], t);
    mid = INTERPOLATE(ctrlpoints[1][0], ctrlpoints[2][0], t);
    right = INTERPOLATE(ctrlpoints[2][0], ctrlpoints[3][0], t);
    left_mid = INTERPOLATE(left, mid, t);
    mid_right = INTERPOLATE(mid, right, t);
    out[0]=INTERPOLATE(left_mid, mid_right, t);

	// y
    left = INTERPOLATE(ctrlpoints[0][1], ctrlpoints[1][1], t);
    mid = INTERPOLATE(ctrlpoints[1][1], ctrlpoints[2][1], t);
    right = INTERPOLATE(ctrlpoints[2][1], ctrlpoints[3][1], t);
    left_mid = INTERPOLATE(left, mid, t);
    mid_right = INTERPOLATE(mid, right, t);
    out[1]=INTERPOLATE(left_mid, mid_right, t);

	// z
    left = INTERPOLATE(ctrlpoints[0][2], ctrlpoints[1][2], t);
    mid = INTERPOLATE(ctrlpoints[1][2], ctrlpoints[2][2], t);
    right = INTERPOLATE(ctrlpoints[2][2], ctrlpoints[3][2], t);
    left_mid = INTERPOLATE(left, mid, t);
    mid_right = INTERPOLATE(mid, right, t);
    out[2]=INTERPOLATE(left_mid, mid_right, t);
}

// И второй.
// P(t)=P_0*(1-t)^3+P_1*3(1-t)^2*t+P_2*3(1-t)*t2+P_3*t3
void eval_bezier(float *out, float t, float ctrlpoints[4][3])
{	
	float m,a1,a2,a3,a4,t2,t3;
	t2 = t*t;
	t3 = t*t*t;
	m = 1.0f-t;
	a1 = m*m*m;
	a2 = 3.0f*(m*m)*t;
	a3 = 3.0f*m*t2;
	a4 = t3;

	out[0]=ctrlpoints[0][0]*a1+
		   ctrlpoints[1][0]*a2+
		   ctrlpoints[2][0]*a3+
		   ctrlpoints[3][0]*a4;

	out[1]=ctrlpoints[0][1]*a1+
		   ctrlpoints[1][1]*a2+
		   ctrlpoints[2][1]*a3+
		   ctrlpoints[3][1]*a4;

	out[2]=ctrlpoints[0][2]*a1+
		   ctrlpoints[1][2]*a2+
		   ctrlpoints[2][2]*a3+
		   ctrlpoints[3][2]*a4;
			
}


Vect3f GetVertexNormal(Vect3f* vVertex) {
	//D3DXVECTOR3 vNormal;
	//D3DXVECTOR3 n = D3DXVECTOR3(0, 0, 0);

	//D3DXVec3Subtract(&vNormal, &n, vVertex);
	//vNormal.x = vVertex->x;
	//vNormal.y = vVertex->y;
	//vNormal.z = vVertex->z;

    //D3DXVec3Normalize(&vNormal, &vNormal);
	Vect3f normal(*vVertex);
    return normal.normalize();

}

Vect3f GetTriangeNormal(Vect3f* vVertex1, Vect3f* vVertex2, Vect3f* vVertex3)
{
    Vect3f vNormal;
    Vect3f v1;
    Vect3f v2;

    //D3DXVec3Subtract(&v1, vVertex1, vVertex3);
    //D3DXVec3Subtract(&v2, vVertex2, vVertex3);
	
    //D3DXVec3Cross(&vNormal, &v1, &v2);

    //D3DXVec3Normalize(&vNormal, &vNormal);

	v1 = *vVertex1 - *vVertex3;
	v2 = *vVertex2 - *vVertex3;

	vNormal.cross(v1,v2);

    return vNormal;
}

void cp(float *v,float *v1,float *v2)
{
  v[0] = v1[1]*v2[2] - v2[1]*v1[2];
  v[1] = v1[2]*v2[0] - v2[2]*v1[0];
  v[2] = v1[0]*v2[1] - v2[0]*v1[1];

  v[0] = v[0] == -0 ? 0 : v[0];
  v[1] = v[1] == -0 ? 0 : v[1];
  v[2] = v[2] == -0 ? 0 : v[2];
  return;
}

void norm(float *v)
{
  float c;

  c = 1/sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]);

  v[0] *= c; 
  v[1] *= c;
  v[2] *= c;

  v[0] = v[0] == -0 ? 0 : v[0];
  v[1] = v[1] == -0 ? 0 : v[1];
  v[2] = v[2] == -0 ? 0 : v[2];

  return;
}

D3DVECTOR GetPTriangeNormal(float *v1, float *v2, float *v3)
{
    D3DXVECTOR3 vNormal;
	float n[3];


	v1[0] = v1[0] == -0 ? 0 : v1[0];
	v1[1] = v1[1] == -0 ? 0 : v1[1];
	v1[2] = v1[2] == -0 ? 0 : v1[2];

	v2[0] = v2[0] == -0 ? 0 : v2[0];
	v2[1] = v2[1] == -0 ? 0 : v2[1];
	v2[2] = v2[2] == -0 ? 0 : v2[2];

	v3[0] = v3[0] == -0 ? 0 : v3[0];
	v3[1] = v3[1] == -0 ? 0 : v3[1];
	v3[2] = v3[2] == -0 ? 0 : v3[2];

	float edge1[3],edge2[3];
	int j;

	for (j=0;j<3;j++)  
	{  
		edge1[j] = v1[j] - v2[j];
		edge2[j] = v3[j] - v2[j];
	}

	cp(n,edge1,edge2);
	norm(n);

	vNormal.x=n[0];
	vNormal.y=n[1];
	vNormal.z=n[2];

    return vNormal;
}

#define RAD_2_DEG	57.295779513082323f

void DrawRealCircle(float *p1, float radius1, Vect3f normal, int m_nSegments) {
    int nCurrentSegment;
	D3DXMATRIX trans, trans2;

	D3DXMatrixIdentity(&trans);
	D3DXMatrixIdentity(&trans2);

	VertexXYZNDT1* Vertices = (VertexXYZNDT1*)renderSystem->LockVertexBuffer(vertexBuffer,vertexNum,(m_nSegments*2+10)*vertexBuffer.vertexSize);

	Color4c color = currentColor;

	int curver=vertexNum;
	int amount=0;

    float rDeltaSegAngle = (2.0f * D3DX_PI / m_nSegments);
    float rSegmentLength = 1.0f / (float)m_nSegments;

	//Create the triangle fan: Center
	Vertices->pos.set(0.0f+p1[0],0.0f - (radius1 / 2.0f)+p1[1],0.0f+p1[2]);
	Vertices->difuse = color;
	Vertices->u1() = 0.5f;
	Vertices->v1() = 0.5f;
	Vertices->n.set(0.0f,-1.0f,0.0f);
	Vertices++;
	vertexNum++;
	amount++;	

    //Create the triangle fan: Edges
    for(nCurrentSegment = m_nSegments; nCurrentSegment >= 0; nCurrentSegment--)
    {
        float x0 = radius1 * sinf(nCurrentSegment * rDeltaSegAngle);
        float z0 = radius1 * cosf(nCurrentSegment * rDeltaSegAngle);
    
		Vertices->pos.set(x0+p1[0],0.0f - (radius1 / 2.0f)+p1[1],z0+p1[2]);
		Vertices->difuse = color;
		Vertices->u1() =(0.5f * sinf(nCurrentSegment * rDeltaSegAngle)) + 0.5f;
		Vertices->v1() = (0.5f * cosf(nCurrentSegment * rDeltaSegAngle)) + 0.5f;
		Vertices->n.set(0.0f,-1.0f,0.0f);
		Vertices++;
		vertexNum++;
		amount++;	
    }

	vertexType[curver].type = GL_TRIANGLE_FAN;
	vertexType[curver].amount = amount;
	vertexType[curver].textNum = currentTex;
	vertexType[curver].doLight=doLight;
	vertexType[curver].transparent=transparent;

	renderSystem->UnlockVertexBuffer(vertexBuffer);
}

void DrawRealCylinder(float *p1, float *p2, float radius1, float radius2, bool s1, bool s2, bool s3)
{
    int nCurrentSegment;
	int m_nSegments=20;
	Vect3f diff;
	double h;
	float x0, z0;
	D3DXVECTOR3 v;
	D3DXMATRIX trans1, trans2, trans3;

	VertexXYZNDT1* Vertices = (VertexXYZNDT1*)renderSystem->LockVertexBuffer(vertexBuffer,vertexNum,(200)*vertexBuffer.vertexSize);

	diff.x = p2[0] - p1[0];
	diff.y = p2[1] - p1[1];
	diff.z = p2[2] - p1[2];

	h = sqrt (diff.x*diff.x + diff.y*diff.y + diff.z*diff.z);

	//glTranslatef (v1[0], v1[1], v1[2]);
	//glRotatef (-RAD_2_DEG * (atan2 (vdiff[2], vdiff[0]) - M_PI/2), 0.0f, 1.0f, 0.0f);
	//glRotatef (-RAD_2_DEG * asin (vdiff[1]/h), 1.0f, 0.0f, 0.0f);
	D3DXMatrixIdentity(&trans1);
	D3DXMatrixIdentity(&trans2);
	D3DXMatrixIdentity(&trans3);
	D3DXMatrixTranslation(&trans1, (p1[0]+p2[0])/2, (p1[1]+p2[1])/2,(p1[2]+p2[2])/2);
	float a1=-(atan2 (diff.z, diff.x) - M_PI/2);
	float a2=-asin (diff.y/h);
	D3DXMatrixRotationY(&trans2, a1);
	D3DXMatrixRotationX(&trans3, a2);
	//D3DXMatrixMultiply(&trans2, &trans3, &trans2);
	D3DXMatrixMultiply(&trans1, &trans2, &trans1);
	D3DXMatrixMultiply(&trans1, &trans3, &trans1);

	Color4c color = currentColor;

	int curver=vertexNum;
	int amount=0;

    float rDeltaSegAngle = (2.0f * D3DX_PI / m_nSegments);
    float rSegmentLength = 1.0f / (float)m_nSegments;

	if (s1) {
		//Create the sides triangle strip
		for(nCurrentSegment = 0; nCurrentSegment <= m_nSegments; nCurrentSegment++)
		{
			x0 = radius1 * sinf(nCurrentSegment * rDeltaSegAngle);
			z0 = radius1 * cosf(nCurrentSegment * rDeltaSegAngle);

			
			v.x=x0;
			v.z=0.0f+(h/2.0f);
			v.y=z0;
			D3DXVec3TransformCoord(&v,&v,&trans1);
			Vertices->pos.set(v.x, v.y, v.z);
			Vertices->difuse = color;
			Vertices->u1() = 1.0f - (rSegmentLength * (float)nCurrentSegment);
			Vertices->v1() = 0.0;
			Vertices->n.set(x0,0.0f,z0);
			Vertices++;
			vertexNum++;
			amount++;		

			x0 = radius2 * sinf(nCurrentSegment * rDeltaSegAngle);
			z0 = radius2 * cosf(nCurrentSegment * rDeltaSegAngle);

			v.x=x0;
			v.z=0.0f-(h/2.0f);
			v.y=z0;
			D3DXVec3TransformCoord(&v,&v,&trans1);
			Vertices->pos.set(v.x, v.y, v.z);
			Vertices->difuse = color;
			Vertices->u1() = 1.0f - (rSegmentLength * (float)nCurrentSegment);
			Vertices->v1() = 1.0f;
			Vertices->n.set(x0,0.0f,z0);
			Vertices++;
			vertexNum++;
			amount++;
		}
		vertexType[curver].type = GL_TRIANGLE_STRIP;
		vertexType[curver].amount = amount;
		vertexType[curver].textNum = currentTex;
		vertexType[curver].doLight=doLight;
		vertexType[curver].transparent=transparent;

		curver=vertexNum;
		amount=0;
	}

	if (s2) {
		//Create the top triangle fan: Center
		v.x=0.0f;
		v.z=0.0f+(h/2.0f);
		v.y=0.0f;
		D3DXVec3TransformCoord(&v,&v,&trans1);
		Vertices->pos.set(v.x, v.y, v.z);
		Vertices->difuse = color;
		Vertices->u1() = 0.5f;
		Vertices->v1() = 0.5f;
		Vertices->n.set(0.0f,1.0f,0.0f);
		Vertices++;
		vertexNum++;
		amount++;	

		//Create the top triangle fan: Edges
		for(nCurrentSegment = 0; nCurrentSegment <= m_nSegments; nCurrentSegment++)
		{
			x0 = radius1 * sinf(nCurrentSegment * rDeltaSegAngle);
			z0 = radius1 * cosf(nCurrentSegment * rDeltaSegAngle);
	    
			v.x=x0;
			v.z=0.0f+(h/2.0f);
			v.y=z0;
			D3DXVec3TransformCoord(&v,&v,&trans1);
			Vertices->pos.set(v.x, v.y, v.z);
			Vertices->difuse = color;
			Vertices->u1() =(0.5f * sinf(nCurrentSegment * rDeltaSegAngle)) + 0.5f;
			Vertices->v1() = (0.5f * cosf(nCurrentSegment * rDeltaSegAngle)) + 0.5f;
			Vertices->n.set(0.0f,1.0f,0.0f);
			Vertices++;
			vertexNum++;
			amount++;	
		}
	vertexType[curver].type = GL_TRIANGLE_FAN;
	vertexType[curver].amount = amount;
	vertexType[curver].textNum = currentTex;
	vertexType[curver].doLight=doLight;
	vertexType[curver].transparent=transparent;

	curver=vertexNum;
	amount=0;
	}

	if (s3) {
		//Create the bottom triangle fan: Center
		Vertices->pos.set(0.0f+p1[0],0.0f - (radius1 / 2.0f)+p1[1],0.0f+p1[2]);
		v.x=0.0f;
		v.z=0.0f-(h/2.0f);
		v.y=0.0f;
		D3DXVec3TransformCoord(&v,&v,&trans1);
		Vertices->pos.set(v.x, v.y, v.z);
		Vertices->difuse = color;
		Vertices->u1() = 0.5f;
		Vertices->v1() = 0.5f;
		Vertices->n.set(0.0f,-1.0f,0.0f);
		Vertices++;
		vertexNum++;
		amount++;	

		//Create the bottom triangle fan: Edges
		for(nCurrentSegment = m_nSegments; nCurrentSegment >= 0; nCurrentSegment--)
		{
			x0 = radius2 * sinf(nCurrentSegment * rDeltaSegAngle);
			z0 = radius2 * cosf(nCurrentSegment * rDeltaSegAngle);
	    
			v.x=x0;
			v.z=0.0f-(h/2.0f);
			v.y=z0;
			D3DXVec3TransformCoord(&v,&v,&trans1);
			Vertices->pos.set(v.x, v.y, v.z);
			Vertices->difuse = color;
			Vertices->u1() =(0.5f * sinf(nCurrentSegment * rDeltaSegAngle)) + 0.5f;
			Vertices->v1() = (0.5f * cosf(nCurrentSegment * rDeltaSegAngle)) + 0.5f;
			Vertices->n.set(0.0f,-1.0f,0.0f);
			Vertices++;
			vertexNum++;
			amount++;	
		}

		vertexType[curver].type = GL_TRIANGLE_FAN;
		vertexType[curver].amount = amount;
		vertexType[curver].textNum = currentTex;
		vertexType[curver].doLight=doLight;
		vertexType[curver].transparent=transparent;
	}

	renderSystem->UnlockVertexBuffer(vertexBuffer);
}

_inline float Noise(int x, int y)
{
  int n = x + y * 57;
  n = (n<<13) ^ n;
  float ret = ( 1.0f - ( (n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f);
  if (ret < 0) ret = 0;
  return ret;
}

void DrawPlanetSphere(float *p1, float radius, int tex, int n) {
	D3DXVECTOR3 vNormal;
	Color4c color;
	int i, j;
	double theta1,theta2,theta3;
	
	if (exportb[currentModel]==true && splineExport==true) 
		return;

	int amount=0;

	float cx=(float)*p1;
	float cy=(float)*(p1+1);
	float cz=(float)*(p1+2);

	int l=n/2*n*2+2;
    //Lock the vertex buffer
	//if(FAILED(d3d_vb->Lock(sizeof(CUSTOMVERTEX)*vertexNum,l,(BYTE**)&Vertices,0))) return;

	VertexXYZNDT1* Vertices = (VertexXYZNDT1*)renderSystem->LockVertexBuffer(vertexBuffer,vertexNum,l*vertexBuffer.vertexSize);

	//if (currentTex==0 || useLocalColor)
		color = currentColor;
	//else
	//	color.set(255,255,255);

	int curver=vertexNum;

    const float PI     = 3.14159265358979f;
    const float TWOPI  = 6.28318530717958f;
    const float PIDIV2 = 1.57079632679489f;

    float ex = 0.0f;
    float ey = 0.0f;
    float ez = 0.0f;

    float px = 0.0f;
    float py = 0.0f;
    float pz = 0.0f;

    // Disallow a negative number for radius.
    //if( r < 0 )
    //    r = -r;

    // Disallow a negative number for precision.
    //if( p < 0 )
    //    p = -p;

    // If the sphere is too small, just render a OpenGL point instead.
	/*
    if( p < 4 || r <= 0 ) 
    {
        glBegin( GL_POINTS );
        glVertex3f( cx, cy, cz );
        glEnd();
        return;
    }
*/
	float snt1, cnt1, snt2, cnt2, snt3, cnt3;
	float radius2;

	if (n<60)
		j = 1;
	else
		j = 0;

    for(;j < n/2; j++ )
    {
        theta1 = j * TWOPI / n - PIDIV2;
        theta2 = (j + 1) * TWOPI / n - PIDIV2;
		snt1 = sin(theta1);
		snt2 = sin(theta2);
		cnt1 = cos(theta1);
		cnt2 = cos(theta2);
		amount=0;
		curver=vertexNum;
        for( int i = 0; i <= n; i++ )
        {
            theta3 = i * (TWOPI) / n;
			snt3 = sin(theta3);
			cnt3 = cos(theta3);
			
			radius2 = radius;// + Noise(i/(double)n*100, 2*j/(double)n*100)*1000;
            ex = cnt1 * cnt3;
            ey = snt1;
            ez = cnt1 * snt3;
            px = cx + radius2 * ex;
            py = cy + radius2 * ey;
            pz = cz + radius2 * ez;

			Vertices->pos.set(px,py,pz);
			Vertices->difuse = color;
			Vertices->u1() = i/(double)n;
			Vertices->v1() = 2*j/(double)n;
			Vertices->n.set(ex,ey,ez);
			Vertices++;
			vertexNum++;
			amount++;
			
			radius2 = radius;// + Noise(i/(double)n*100, 2*(j+1)/(double)n*100)*1000;
            ex = cnt2 * cnt3;
            ey = snt2;
            ez = cnt2 * snt3;
            px = cx + radius2 * ex;
            py = cy + radius2 * ey;
            pz = cz + radius2 * ez;

			Vertices->pos.set(px,py,pz);
			Vertices->difuse = color;
			Vertices->u1() = i/(double)n;
			Vertices->v1() = 2*(j+1)/(double)n;
			Vertices->n.set(ex,ey,ez);
			Vertices++;
			vertexNum++;
			amount++;
        }
		vertexType[curver].type = GL_TRIANGLE_STRIP;
		vertexType[curver].amount = amount;
		vertexType[curver].textNum = tex;
		vertexType[curver].doLight=doLight;
		vertexType[curver].transparent=transparent;
    }

	renderSystem->UnlockVertexBuffer(vertexBuffer);
    //d3d_vb->Unlock();
}

// Кривовато
void DrawRealSphere(float *p1, float radius, int tex, int n) {
	D3DXVECTOR3 vNormal;
	Color4c color;
	int i, j;
	double theta1,theta2,theta3;
	
	if (exportb[currentModel]==true && splineExport==true) 
		return;

	int amount=0;

	float cx=(float)*p1;
	float cy=(float)*(p1+1);
	float cz=(float)*(p1+2);

	int l=n/2*n*2+2;
    //Lock the vertex buffer
	//if(FAILED(d3d_vb->Lock(sizeof(CUSTOMVERTEX)*vertexNum,l,(BYTE**)&Vertices,0))) return;

	VertexXYZNDT1* Vertices = (VertexXYZNDT1*)renderSystem->LockVertexBuffer(vertexBuffer,vertexNum,l*vertexBuffer.vertexSize);

	//if (currentTex==0 || useLocalColor)
		color = currentColor;
	//else
	//	color.set(255,255,255);

	int curver=vertexNum;

    const float PI     = 3.14159265358979f;
    const float TWOPI  = 6.28318530717958f;
    const float PIDIV2 = 1.57079632679489f;

    float ex = 0.0f;
    float ey = 0.0f;
    float ez = 0.0f;

    float px = 0.0f;
    float py = 0.0f;
    float pz = 0.0f;

    // Disallow a negative number for radius.
    //if( r < 0 )
    //    r = -r;

    // Disallow a negative number for precision.
    //if( p < 0 )
    //    p = -p;

    // If the sphere is too small, just render a OpenGL point instead.
	/*
    if( p < 4 || r <= 0 ) 
    {
        glBegin( GL_POINTS );
        glVertex3f( cx, cy, cz );
        glEnd();
        return;
    }
*/
	float snt1, cnt1, snt2, cnt2, snt3, cnt3;

	if (n<60)
		j = 1;
	else
		j = 0;

    for(;j < n/2; j++ )
    {
        theta1 = j * TWOPI / n - PIDIV2;
        theta2 = (j + 1) * TWOPI / n - PIDIV2;
		snt1 = sin(theta1);
		snt2 = sin(theta2);
		cnt1 = cos(theta1);
		cnt2 = cos(theta2);
		amount=0;
		curver=vertexNum;
        for( int i = 0; i <= n; i++ )
        {
            theta3 = i * (TWOPI) / n;
			snt3 = sin(theta3);
			cnt3 = cos(theta3);
			
            ex = cnt1 * cnt3;
            ey = snt1;
            ez = cnt1 * snt3;
            px = cx + radius * ex;
            py = cy + radius * ey;
            pz = cz + radius * ez;

			Vertices->pos.set(px,py,pz);
			Vertices->difuse = color;
			Vertices->u1() = i/(double)n;
			Vertices->v1() = 2*j/(double)n;
			Vertices->n.set(ex,ey,ez);
			Vertices++;
			vertexNum++;
			amount++;
			
            ex = cnt2 * cnt3;
            ey = snt2;
            ez = cnt2 * snt3;
            px = cx + radius * ex;
            py = cy + radius * ey;
            pz = cz + radius * ez;

			Vertices->pos.set(px,py,pz);
			Vertices->difuse = color;
			Vertices->u1() = i/(double)n;
			Vertices->v1() = 2*(j+1)/(double)n;
			Vertices->n.set(ex,ey,ez);
			Vertices++;
			vertexNum++;
			amount++;
        }
		vertexType[curver].type = GL_TRIANGLE_STRIP;
		vertexType[curver].amount = amount;
		vertexType[curver].textNum = tex;
		vertexType[curver].doLight=doLight;
		vertexType[curver].transparent=transparent;
    }

	renderSystem->UnlockVertexBuffer(vertexBuffer);
    //d3d_vb->Unlock();
}


void DrawRealLine(float *p1, float *p2)
{
	Color4c color;
	D3DXVECTOR3 vNormal;

	if (vertexNum>MAXVERT-100)
		return;

	//if(FAILED(d3d_vb->Lock(sizeof(CUSTOMVERTEX)*vertexNum,sizeof(CUSTOMVERTEX)*2,(BYTE**)&Vertices,0))) return;
	VertexXYZNDT1* Vertices = (VertexXYZNDT1*)renderSystem->LockVertexBuffer(vertexBuffer,vertexNum,2*vertexBuffer.vertexSize);

	color = currentColor;

	Vertices->pos.x = (float)*p1;
	Vertices->pos.y = (float)*(p1+1);
	Vertices->pos.z = (float)*(p1+2);
	Vertices->difuse = color;
	Vertices->u1() = (float)0;
	Vertices->v1() = (float)0;
	vertexType[vertexNum].type = GL_LINES;
	vertexType[vertexNum].amount = 2;
	vertexType[vertexNum].textNum = -1;
	vertexType[vertexNum].doLight=doLight;
	vertexType[vertexNum].transparent=false;

	//vNormal = GetVertexNormal(&(Vertices->pos)); 
	Vertices->n = GetVertexNormal(&(Vertices->pos));

	Vertices++;
	vertexNum++;

	Vertices->pos.x = (float)*p2;
	Vertices->pos.y = (float)*(p2+1);
	Vertices->pos.z = (float)*(p2+2);
	Vertices->difuse = color;
	Vertices->u1() = (float)1;
	Vertices->v1() = (float)1;

	//vNormal = GetVertexNormal(&D3DXVECTOR3(Vertices->pos.x, Vertices->pos.y, Vertices->pos.z)); 
	Vertices->n = GetVertexNormal(&(Vertices->pos));

	Vertices++;
	vertexNum++;

	renderSystem->UnlockVertexBuffer(vertexBuffer);
	//d3d_vb->Unlock();
}

int tc=0;
// Используется для возвратных вершин тесселятора
void addVertex(double *p1, int type)
{
	Vect3f vNormal;
	Color4c color;

	Vect3f curPoint;

	curPoint=Vect3f((float)p1[0], (float)p1[1], (float)p1[2]);
	newIndex(&curPoint);

	if (vertexNum>MAXVERT-100)
		return;

	//if(FAILED(d3d_vb->Lock(sizeof(CUSTOMVERTEX)*vertexNum,sizeof(CUSTOMVERTEX)*1,(BYTE**)&Vertices,0))) return;
	VertexXYZNDT1* Vertices = (VertexXYZNDT1*)renderSystem->LockVertexBuffer(vertexBuffer,vertexNum,1*vertexBuffer.vertexSize);

	//if (currentTex==0 || useLocalColor)
		color = currentColor;
	//else
	//	color.set(255,255,255);

	Vertices->pos.x = (float)p1[0];
	Vertices->pos.y = (float)p1[1];
	Vertices->pos.z = (float)p1[2];

	Vertices->difuse = color;
	Vertices->u1() = (float)0;
	Vertices->v1() = (float)0;
	vertexType[vertexNum].type = 0;
	vertexType[vertexNum].textNum = currentTex;
	vertexType[vertexNum].doLight=doLight;
	vertexType[vertexNum].transparent=transparent;

	if (currentNormal!=0) {
		//vNormal = getReNormal(currentNormal);
		//vNormal = getNormal(currentNormal);
		Vertices->n = getNormal(currentNormal);
	} else {
		Vertices->n.set(0,0,0);
	}

	Vertices++;
	vertexNum++;
	renderSystem->UnlockVertexBuffer(vertexBuffer);
	//d3d_vb->Unlock();

}

void DrawRealSquare(float *p1, float *p2, float *p3, float *p4, int normal)
{
	Color4c color;
	Vect3f vNormal;
	float corrector;

	if (exportb[currentModel]==true && splineExport==true) 
		return;

	if (vertexNum>MAXVERT-100)
		return;
	
	//vNormal = GetTriangeNormal(&D3DXVECTOR3((float)*p1, (float)*(p1+1), (float)*(p1+2))
	//	,&D3DXVECTOR3((float)*p2, (float)*(p2+1), (float)*(p2+2))
	//	,&D3DXVECTOR3((float)*p3, (float)*(p3+1), (float)*(p3+2)));

	Vect3f curPoint;

	curPoint=Vect3f((float)*p1, (float)*(p1+1), (float)*(p1+2));
	newIndex(&curPoint);
	curPoint=Vect3f((float)*p4, (float)*(p4+1), (float)*(p4+2));
	newIndex(&curPoint);
	curPoint=Vect3f((float)*p3, (float)*(p3+1), (float)*(p3+2));
	newIndex(&curPoint);

	curPoint=Vect3f((float)*p4, (float)*(p4+1), (float)*(p4+2));
	newIndex(&curPoint);
	curPoint=Vect3f((float)*p3, (float)*(p3+1), (float)*(p3+2));
	newIndex(&curPoint);
	curPoint=Vect3f((float)*p2, (float)*(p2+1), (float)*(p2+2));
	newIndex(&curPoint);

	//if(FAILED(d3d_vb->Lock(sizeof(CUSTOMVERTEX)*vertexNum,sizeof(CUSTOMVERTEX)*4,(BYTE**)&Vertices,0))) return;
	VertexXYZNDT1* Vertices = (VertexXYZNDT1*)renderSystem->LockVertexBuffer(vertexBuffer,vertexNum,4*vertexBuffer.vertexSize);

	//if (currentTex==0 || useLocalColor)
		color = currentColor;
	//else
	//	color.set(255,255,255);

	vertexType[vertexNum].vertCount=4;
	vertexType[vertexNum].type = GL_TRIANGLE_STRIP;
	vertexType[vertexNum].amount = 4;
	vertexType[vertexNum].textNum = currentTex;
	vertexType[vertexNum].doLight=doLight;
	vertexType[vertexNum].transparent=transparent;
	
	Vertices->pos.x = (float)*p1;
	Vertices->pos.y = (float)*(p1+1);
	Vertices->pos.z = (float)*(p1+2);
	Vertices->difuse = color;
	Vertices->u1() = (float)0;
	Vertices->v1() = (float)1;


	if (normal!=0) {
		//vNormal = getNormal(normal);
		Vertices->n = getNormal(normal);
	} else {
		Vertices->n.set(0,0,0);
	}

	Vertices++;
	vertexNum++;

	Vertices->pos.x = (float)*p4;
	Vertices->pos.y = (float)*(p4+1);
	Vertices->pos.z = (float)*(p4+2);
	Vertices->difuse = color;
	Vertices->u1() = (float)0;
	Vertices->v1() = (float)0;

	if (normal!=0) {
		//vNormal = getNormal(normal);
		Vertices->n = getNormal(normal);
	} else {
		Vertices->n.set(0,0,0);
	}

	Vertices++;
	vertexNum++;

	Vertices->pos.x = (float)*p3;
	Vertices->pos.y = (float)*(p3+1);
	Vertices->pos.z = (float)*(p3+2);
	Vertices->difuse = color;
	Vertices->u1() = (float)1;
	Vertices->v1() = (float)1;

	if (normal!=0) {
		//vNormal = getNormal(normal);
		Vertices->n = getNormal(normal);
	} else {
		Vertices->n.set(0,0,0);
	}

	Vertices++;
	vertexNum++;

	Vertices->pos.x = (float)*p2;
	Vertices->pos.y = (float)*(p2+1);
	Vertices->pos.z = (float)*(p2+2);
	Vertices->difuse = color;
	Vertices->u1() = (float)1;
	Vertices->v1() = (float)0;

	if (normal!=0) {
		//vNormal = getNormal(normal);
		Vertices->n = getNormal(normal);
	} else {
		Vertices->n.set(0,0,0);
	}

	Vertices++;
	vertexNum++;

	renderSystem->UnlockVertexBuffer(vertexBuffer);
	//d3d_vb->Unlock();
}

#define absd(a) ((a)>=0?(float)(a):(float)-(a))

void DrawRealTriangle(float *p1, float *p2, float *p3, int type, int normal);

void DrawRealSquare2(float *p1, float *p2, float *p3, float *p4, int normal)
{
	Color4c color;
	Vect3f vNormal;
	double ax, ay, az;
	double bx, by, bz;
	double cx, cy, cz;

	if (exportb[currentModel]==true && splineExport==true) 
		return;

	if (vertexNum>MAXVERT-100)
		return;

	//p1[1]-=0.002f;
	//p2[1]-=0.002f;
	//p3[1]-=0.002f;
	//p4[1]-=0.002f;

	//p1[2]+=0.02f;
	//p2[2]+=0.02f;
	//p3[2]+=0.02f;
	//p4[2]+=0.02f;

	Vect3f curPoint;

	curPoint=Vect3f((float)*p1, (float)*(p1+1), (float)*(p1+2));
	newIndex(&curPoint);
	curPoint=Vect3f((float)*p4, (float)*(p4+1), (float)*(p4+2));
	newIndex(&curPoint);
	curPoint=Vect3f((float)*p3, (float)*(p3+1), (float)*(p3+2));
	newIndex(&curPoint);

	curPoint=Vect3f((float)*p4, (float)*(p4+1), (float)*(p4+2));
	newIndex(&curPoint);
	curPoint=Vect3f((float)*p3, (float)*(p3+1), (float)*(p3+2));
	newIndex(&curPoint);
	curPoint=Vect3f((float)*p2, (float)*(p2+1), (float)*(p2+2));
	newIndex(&curPoint);
	

	ax=*p1/2+*p2/2;
	ay=*(p1+1)/2+*(p2+1)/2;
	az=*(p1+2)/2+*(p2+2)/2;

	bx=*p3/2+*p4/2;
	by=*(p3+1)/2+*(p4+1)/2;
	bz=*(p3+2)/2+*(p4+2)/2;

	cx=ax/2+bx/2;
	cy=ay/2+by/2;
	cz=az/2+bz/2;

	vNormal = getNormal(normal);

	color = currentColor;

	VertexXYZNDT1* Vertices = (VertexXYZNDT1*)renderSystem->LockVertexBuffer(vertexBuffer,vertexNum,6*vertexBuffer.vertexSize);

	Vertices->pos.x = (float)cx;
	Vertices->pos.y = (float)cy;
	Vertices->pos.z = (float)cz;
	Vertices->difuse = color;
	Vertices->u1() = 0.5f;
	Vertices->v1() = 0.5f;
	vertexType[vertexNum].type = GL_TRIANGLE_FAN;
	vertexType[vertexNum].amount = 6;
	vertexType[vertexNum].textNum = currentTex;
	vertexType[vertexNum].doLight=doLight;
	vertexType[vertexNum].transparent=transparent;

	Vertices->n.x = vNormal.x;
	Vertices->n.y = vNormal.y;
	Vertices->n.z = vNormal.z;

	Vertices++;
	vertexNum++;

	Vertices->pos.x = (float)*p1;
	Vertices->pos.y = (float)*(p1+1);
	Vertices->pos.z = (float)*(p1+2);
	Vertices->difuse = color;
	Vertices->u1() = (float)0;
	Vertices->v1() = (float)1;

	Vertices->n.x = vNormal.x;
	Vertices->n.y = vNormal.y;
	Vertices->n.z = vNormal.z;


	Vertices++;
	vertexNum++;

	Vertices->pos.x = (float)*p2;
	Vertices->pos.y = (float)*(p2+1);
	Vertices->pos.z = (float)*(p2+2);
	Vertices->difuse = color;
	Vertices->u1() = (float)0;
	Vertices->v1() = (float)0;

	Vertices->n.x = vNormal.x;
	Vertices->n.y = vNormal.y;
	Vertices->n.z = vNormal.z;

	Vertices++;
	vertexNum++;

	Vertices->pos.x = (float)*p3;
	Vertices->pos.y = (float)*(p3+1);
	Vertices->pos.z = (float)*(p3+2);
	Vertices->difuse = color;
	Vertices->u1() = (float)1;
	Vertices->v1() = (float)0;

	Vertices->n.x = vNormal.x;
	Vertices->n.y = vNormal.y;
	Vertices->n.z = vNormal.z;

	Vertices++;
	vertexNum++;

	Vertices->pos.x = (float)*p4;
	Vertices->pos.y = (float)*(p4+1);
	Vertices->pos.z = (float)*(p4+2);
	Vertices->difuse = color;
	Vertices->u1() = (float)1;
	Vertices->v1() = (float)1;

	Vertices->n.x = vNormal.x;
	Vertices->n.y = vNormal.y;
	Vertices->n.z = vNormal.z;

	Vertices++;
	vertexNum++;

	Vertices->pos.x = (float)*p1;
	Vertices->pos.y = (float)*(p1+1);
	Vertices->pos.z = (float)*(p1+2);
	Vertices->difuse = color;
	Vertices->u1() = (float)0;
	Vertices->v1() = (float)1;

	Vertices->n.x = vNormal.x;
	Vertices->n.y = vNormal.y;
	Vertices->n.z = vNormal.z;

	Vertices++;
	vertexNum++;

	renderSystem->UnlockVertexBuffer(vertexBuffer);

//	d3d_vb->Unlock();
}

void DrawRealTriangle(float *p1, float *p2, float *p3, int type, int normal)
{
	Color4c color;
	Vect3f vNormal;

	if (exportb[currentModel]==true && splineExport==true) 
		return;

	if (vertexNum>MAXVERT-100)
		return;
/*
	int zzz=0;
	D3DXVECTOR3 oldNormal;
	float pp1[3],pp2[3],pp3[3];

	vNormal = GetTriangeNormal(&Vect3f((float)*p1, (float)*(p1+1), (float)*(p1+2))
		,&Vect3f((float)*p2, (float)*(p2+1), (float)*(p2+2))
		,&Vect3f((float)*p3, (float)*(p3+1), (float)*(p3+2)));
	
	oldNormal=getNormal(currentNormal);

	if (currentNormal>0 && (oldNormal.x*vNormal.x<0 || oldNormal.y*vNormal.y<0 || oldNormal.z*vNormal.z<0))
	zzz=1;
	*/
/*
	if ((oldNormal.x>0 && vNormal.x<0) || (oldNormal.x<0 && vNormal.x>0))
		zzz=1;
	if ((oldNormal.y>0 && vNormal.y<0) || (oldNormal.y<0 && vNormal.y>0))
		zzz=1;
	if ((oldNormal.z>0 && vNormal.z<0) || (oldNormal.z<0 && vNormal.z>0))
		zzz=1;
*/
	/*
	if (zzz) {
		pp1[0]=p3[0];
		pp1[1]=p3[1];
		pp1[2]=p3[2];

		pp3[0]=p1[0];
		pp3[1]=p1[1];
		pp3[2]=p1[2];

		p1[0]=pp1[0];
		p1[1]=pp1[1];
		p1[2]=pp1[2];

		p3[0]=pp3[0];
		p3[1]=pp3[1];
		p3[2]=pp3[2];

	}
*/
	Vect3f curPoint;

	curPoint=Vect3f((float)*p1, (float)*(p1+1), (float)*(p1+2));
	newIndex(&curPoint);
	curPoint=Vect3f((float)*p2, (float)*(p2+1), (float)*(p2+2));
	newIndex(&curPoint);
	curPoint=Vect3f((float)*p3, (float)*(p3+1), (float)*(p3+2));
	newIndex(&curPoint);



	//if(FAILED(d3d_vb->Lock(sizeof(CUSTOMVERTEX)*vertexNum,sizeof(CUSTOMVERTEX)*3,(BYTE**)&Vertices,0))) return;
	VertexXYZNDT1* Vertices = (VertexXYZNDT1*)renderSystem->LockVertexBuffer(vertexBuffer,vertexNum,3*vertexBuffer.vertexSize);

	vNormal = getNormal(normal);

	//if (currentTex==0 || useLocalColor)
		color = currentColor;
	//else
	//	color.set(255,255,255);

	Vertices->pos.x = (float)*p1;
	Vertices->pos.y = (float)*(p1+1);
	Vertices->pos.z = (float)*(p1+2);
	Vertices->difuse = color;
	Vertices->u1() = (float)1;
	Vertices->v1() = (float)1;
	vertexType[vertexNum].type = GL_TRIANGLES;
	vertexType[vertexNum].amount = 3;
	vertexType[vertexNum].textNum = currentTex;
	vertexType[vertexNum].doLight=doLight;
	vertexType[vertexNum].transparent=transparent;

	Vertices->n.x = vNormal.x;
	Vertices->n.y = vNormal.y;
	Vertices->n.z = vNormal.z;


	Vertices++;
	vertexNum++;

	Vertices->pos.x = (float)*p2;
	Vertices->pos.y = (float)*(p2+1);
	Vertices->pos.z = (float)*(p2+2);
	Vertices->difuse = color;
	Vertices->u1() = (float)0;
	Vertices->v1() = (float)0;

	Vertices->n.x = vNormal.x;
	Vertices->n.y = vNormal.y;
	Vertices->n.z = vNormal.z;

	Vertices++;
	vertexNum++;

	Vertices->pos.x = (float)*p3;
	Vertices->pos.y = (float)*(p3+1);
	Vertices->pos.z = (float)*(p3+2);
	Vertices->difuse = color;
	Vertices->u1() = (float)0;
	Vertices->v1() = (float)1;

	Vertices->n.x = vNormal.x;
	Vertices->n.y = vNormal.y;
	Vertices->n.z = vNormal.z;

	Vertices++;
	vertexNum++;
	renderSystem->UnlockVertexBuffer(vertexBuffer);
	//d3d_vb->Unlock();
}

void DrawCustomTriangle(Vect3f *p1, Vect3f *p2, Vect3f *p3, Vect3f *n1, Vect3f *n2, Vect3f *n3, int color1, int color2, int color3)
{
	Color4c color;
	Vect3f vNormal;
	short r, g, b;

	if (vertexNum>MAXVERT-100)
		return;

	Vect3f curPoint;

	curPoint.x=p1->x;
	curPoint.y=p1->y;
	curPoint.z=p1->z;
	newIndex(&curPoint);
	curPoint.x=p2->x;
	curPoint.y=p2->y;
	curPoint.z=p2->z;
	newIndex(&curPoint);
	curPoint.x=p3->x;
	curPoint.y=p3->y;
	curPoint.z=p3->z;
	newIndex(&curPoint);

	VertexXYZNDT1* Vertices = (VertexXYZNDT1*)renderSystem->LockVertexBuffer(vertexBuffer,vertexNum,3*vertexBuffer.vertexSize);

	r=(color1&0x00FF0000)>>16;
	g=(color1&0x0000FF00)>>8;
	b=(color1&0x000000FF);
	color.set(b*31, g*31, r*31);

	Vertices->pos.x = p1->x;
	Vertices->pos.y = p1->y;
	Vertices->pos.z = p1->z;
	Vertices->difuse = color;
	Vertices->u1() = (float)1;
	Vertices->v1() = (float)1;
	vertexType[vertexNum].type = GL_TRIANGLES;
	vertexType[vertexNum].amount = 3;
	vertexType[vertexNum].textNum = currentTex;
	vertexType[vertexNum].doLight=doLight;
	vertexType[vertexNum].transparent=transparent;

	if (n1->x==0) {
		vNormal = GetTriangeNormal(p1,p2,p3); 
		Vertices->n.x = vNormal.x;
		Vertices->n.y = vNormal.y;
		Vertices->n.z = vNormal.z;
	} else {
		Vertices->n.x = n1->x;
		Vertices->n.y = n1->y;
		Vertices->n.z = n1->z;	
	}
	Vertices->n.normalize();

	Vertices++;
	vertexNum++;

	r=(color2&0x00FF0000)>>16;
	g=(color2&0x0000FF00)>>8;
	b=(color2&0x000000FF);
	color.set(b*31, g*31, r*31);

	Vertices->pos.x = p2->x;
	Vertices->pos.y = p2->y;
	Vertices->pos.z = p2->z;
	Vertices->difuse = color;
	Vertices->u1() = (float)0;
	Vertices->v1() = (float)0;
	vertexType[vertexNum].type = 0;
	vertexType[vertexNum].textNum = currentTex;
	vertexType[vertexNum].doLight=true;
	if (n1->x==0) {
		Vertices->n.x = vNormal.x;
		Vertices->n.y = vNormal.y;
		Vertices->n.z = vNormal.z;
	} else {
		Vertices->n.x = n2->x;
		Vertices->n.y = n2->y;
		Vertices->n.z = n2->z;	
	}
	Vertices->n.normalize();

	Vertices++;
	vertexNum++;

	r=(color3&0x00FF0000)>>16;
	g=(color3&0x0000FF00)>>8;
	b=(color3&0x000000FF);
	color.set(b*31, g*31, r*31);

	Vertices->pos.x = p3->x;
	Vertices->pos.y = p3->y;
	Vertices->pos.z = p3->z;
	Vertices->difuse = color;
	Vertices->u1() = (float)0;
	Vertices->v1() = (float)1;
	vertexType[vertexNum].type = 0;
	vertexType[vertexNum].textNum = currentTex;
	vertexType[vertexNum].doLight=true;
	if (n1->x==0) {
		Vertices->n.x = vNormal.x;
		Vertices->n.y = vNormal.y;
		Vertices->n.z = vNormal.z;
	} else {
		Vertices->n.x = n3->x;
		Vertices->n.y = n3->y;
		Vertices->n.z = n3->z;	
	}
	Vertices->n.normalize();

	Vertices++;
	vertexNum++;
	renderSystem->UnlockVertexBuffer(vertexBuffer);
}

// Лампочки
void DrawRealPoint(float *p1, unsigned int radius) {

	Color4c color;

	if (vertexNum>MAXVERT-100)
		return;

	//if(FAILED(d3d_vb->Lock(sizeof(CUSTOMVERTEX)*vertexNum,sizeof(CUSTOMVERTEX)*1,(BYTE**)&Vertices,0))) return;
	VertexXYZNDT1* Vertices = (VertexXYZNDT1*)renderSystem->LockVertexBuffer(vertexBuffer,vertexNum,1*vertexBuffer.vertexSize);

	//if (currentTex==0 || useLocalColor)
		color = currentColor;
	//else
	//	color.set(255,255,255);

	Vertices->pos.x = (float)*p1;
	Vertices->pos.y = (float)*(p1+1);
	Vertices->pos.z = (float)*(p1+2);
	Vertices->difuse = color;
	Vertices->u1() = (float)0;
	Vertices->v1() = (float)0;
	vertexType[vertexNum].type = GL_POINTS;
	vertexType[vertexNum].radius = (float)(radius<<currentScale)/DIVIDER/DIVIDER;
	//vertexType[vertexNum].radius = (float)radius/DIVIDER/50;
	vertexType[vertexNum].textNum = 602;
	vertexType[vertexNum].doLight=doLight;
	vertexType[vertexNum].transparent=transparent;

	Vertices->n.x = 0;
	Vertices->n.y = 0;
	Vertices->n.z = 0;

	Vertices++;
	vertexNum++;
	
	renderSystem->UnlockVertexBuffer(vertexBuffer);
	//d3d_vb->Unlock();	
}

// Кривовато
void DrawBillboard(float *p1, float width) {
	float bp1[3], bp2[3], bp3[3], bp4[3];

	if (exportb[currentModel]==true && splineExport==true) 
		return;

	D3DXMATRIX billMatrix;
	D3DXVECTOR3 v1, v2, v3, vt;

	D3DXMatrixInverse(&billMatrix,NULL, &mainRotMatrix);
	D3DXMatrixMultiply(&billMatrix, &posMatrix, &billMatrix);

	v1.x = billMatrix[12];
	v1.y = billMatrix[13];
	v1.z = billMatrix[14];

	D3DXVec3Normalize(&v1, &v1);

	vt.x = 0.0f;
	vt.y = 1.0f;
	vt.z = 0.0f;

    D3DXVec3Cross(&v2, &v1, &vt);
    D3DXVec3Normalize(&v2, &v2);
    D3DXVec3Cross(&v3, &v1, &v2);
    D3DXVec3Normalize(&v3, &v3);

	v2*=width/2;
	v3*=width/2;

	bp1[0]=p1[0]-v2.x-v3.x;	bp1[1]=p1[1]-v2.y-v3.y;	bp1[2]=p1[2]-v2.z-v3.z;
	bp2[0]=p1[0]-v2.x+v3.x;	bp2[1]=p1[1]-v2.y+v3.y;	bp2[2]=p1[2]-v2.z+v3.z;
	bp3[0]=p1[0]+v2.x-v3.x; bp3[1]=p1[1]+v2.y-v3.y; bp3[2]=p1[2]+v2.z-v3.z; 
	bp4[0]=p1[0]+v2.x+v3.x;	bp4[1]=p1[1]+v2.y+v3.y;	bp4[2]=p1[2]+v2.z+v3.z;
	
	DrawRealSquare(bp1, bp4, bp3, bp2, 0);
}

void drawBillLine(float *p1, float *p2, float width) {

	float bp1[3], bp2[3], bp3[3], bp4[3];

	D3DXVECTOR4 out;

	D3DXVec3Transform(&out,&D3DXVECTOR3(p1[0],p1[1],p1[2]), &mainRotMatrix);
	p1[0] = out.x;
	p1[1] = out.y;
	p1[2] = out.z;

	D3DXVec3Transform(&out,&D3DXVECTOR3(p2[0],p2[1],p2[2]), &mainRotMatrix);
	p2[0] = out.x;
	p2[1] = out.y;
	p2[2] = out.z;

	//D3DXMATRIX billMatrix;
	//D3DXMatrixInverse(&billMatrix,NULL, &mainRotMatrix);
	//D3DXMatrixMultiply(&billMatrix, &posMatrix, &billMatrix);

	D3DXVECTOR3 v1, v2, v3;

	v1.x=p1[0]+posMatrix[12];
	v1.y=p1[1]+posMatrix[13];
	v1.z=p1[2]+posMatrix[14];
	D3DXVec3Normalize(&v1, &v1);

	v2.x=p2[0]+posMatrix[12];
	v2.y=p2[1]+posMatrix[13];
	v2.z=p2[2]+posMatrix[14];
	D3DXVec3Normalize(&v2, &v2);

	D3DXVec3Cross(&v3, &v1, &v2);
	D3DXVec3Normalize(&v3, &v3);

	width/=2;

	bp1[0]=v3.x*width+p1[0];
	bp1[1]=v3.y*width+p1[1];
	bp1[2]=v3.z*width+p1[2];

	bp2[0]=(-v3.x)*width+p1[0];
	bp2[1]=(-v3.y)*width+p1[1];
	bp2[2]=(-v3.z)*width+p1[2];

	bp3[0]=bp1[0]+(p2[0]-p1[0]);
	bp3[1]=bp1[1]+(p2[1]-p1[1]);
	bp3[2]=bp1[2]+(p2[2]-p1[2]);

	bp4[0]=bp2[0]+(p2[0]-p1[0]);
	bp4[1]=bp2[1]+(p2[1]-p1[1]);
	bp4[2]=bp2[2]+(p2[2]-p1[2]);

	D3DXMATRIX invMatrix;
	D3DXMatrixInverse(&invMatrix,NULL, &mainRotMatrix);

	D3DXVec3Transform(&out,&D3DXVECTOR3(p1[0],p1[1],p1[2]), &invMatrix);
	p1[0] = out.x;
	p1[1] = out.y;
	p1[2] = out.z;

	D3DXVec3Transform(&out,&D3DXVECTOR3(p2[0],p2[1],p2[2]), &invMatrix);
	p2[0] = out.x;
	p2[1] = out.y;
	p2[2] = out.z;

	D3DXVec3Transform(&out,&D3DXVECTOR3(bp1[0],bp1[1],bp1[2]), &invMatrix);
	bp1[0] = out.x;
	bp1[1] = out.y;
	bp1[2] = out.z;
	D3DXVec3Transform(&out,&D3DXVECTOR3(bp2[0],bp2[1],bp2[2]), &invMatrix);
	bp2[0] = out.x;
	bp2[1] = out.y;
	bp2[2] = out.z;
	D3DXVec3Transform(&out,&D3DXVECTOR3(bp3[0],bp3[1],bp3[2]), &invMatrix);
	bp3[0] = out.x;
	bp3[1] = out.y;
	bp3[2] = out.z;
	D3DXVec3Transform(&out,&D3DXVECTOR3(bp4[0],bp4[1],bp4[2]), &invMatrix);
	bp4[0] = out.x;
	bp4[1] = out.y;
	bp4[2] = out.z;

	//DrawRealLine(p1, bp1);
	//DrawRealLine(p1, bp2);
	//DrawRealLine(p2, bp3);
	//DrawRealLine(p2, bp4);

	DrawRealSquare(bp2, bp3, bp4, bp1, 0);
}


void copyVertex(float *p1, float *p2) {
	p1[0]=p2[0];
	p1[1]=p2[1];
	p1[2]=p2[2];
}

void plusVertex(double *mass, float *p) {
	mass[0]=p[0];
	mass[1]=p[1];
	mass[2]=p[2];

}

// Получить глобальную или локальную переменную модели
int getVar(unsigned short cmd) {
	int t, result;

	if (callAsm) {
		return FUNC_001473_getVar(gptr, (int)cmd);
	} else {
		if ((unsigned char)(cmd&0xC0)==0xC0) {
			t=cmd&0x3F;
			return localvar[t];
			//return FUNC_001473_getVar(gptr, (int)cmd);
		} else if ((unsigned char)(cmd&0xC0)==0x80) {
			t=cmd&0x3F;
			result=globalvar[t];
			return result;
			//return FUNC_001473_getVar(gptr, (int)cmd);
		} else if ((unsigned char)(cmd&0xC0)==0x40) {
			//t=cmd&0x3F;
			//t=t<<10;
			//t=cmd<<10;
			//cmd=cmd&0x3F;
			t=((cmd & 0x7f)<<9)+((cmd >> 7) & 0x1ff);
			//t/=25;
			return t;
		} else if ((unsigned char)(cmd&0xC0)==0x00) {
			t=((cmd & 0x7f)<<9)+((cmd >> 7) & 0x1ff);
			//t/=25;
			return t;
		} else {
			return 0;
		}
	}
}

inline signed char getByte(void *offs, int num) {
	if (num%2>0)
		return *(((char *)offs+2)+num-1);
	else 
		return *(((char *)offs+2)+num+1);
}

signed char getByte2(void *offs, int num) {
	return *(((char *)offs)+num);
}

Vect3f getNormal(unsigned char num) {
	Vect3f normal;

	if (mod!=NULL && mod->Normals_ptr!=NULL && num>0) {
		signed char *normalPtr = mod->Normals_ptr+(num/2-1)*6;
		normal.x=(float)*(normalPtr+2);
		normal.y=(float)*(normalPtr+3);
		normal.z=(float)*(normalPtr+4);
		if (num%2>0)
			normal.x*=-1;
/*
		D3DXMATRIX tmpMatrix;
		D3DXMatrixIdentity(&tmpMatrix);
		tmpMatrix[12]=normal.x;
		tmpMatrix[13]=normal.y;
		tmpMatrix[14]=normal.z;
		D3DXMatrixMultiply(&tmpMatrix, &mainRotMatrix, &tmpMatrix);
		normal.x=tmpMatrix[12];
		normal.y=tmpMatrix[13];
		normal.z=tmpMatrix[14];
		*/
		//D3DXVec3Normalize(&normal, &normal);
		normal.normalize();
		
	} else {
		normal.x=0;
		normal.y=0;
		normal.z=0;
	}

	return normal;
}

Vect3f getReNormal(unsigned char num) {
	Vect3f vNormal1, vNormal2, normal;

	if (mod->Normals_ptr!=NULL && num>0) {
		vNormal1 = GetVertexNormal(&Vect3f(Vertices->p.x, Vertices->p.y, Vertices->p.z)); 

		signed char *normalPtr = mod->Normals_ptr+(num/2-1)*6;
		vNormal2.x=(float)*(normalPtr+2);
		vNormal2.y=(float)*(normalPtr+3);
		vNormal2.z=(float)*(normalPtr+4);
		if (num%2>0)
			vNormal2.x*=-1;
/*
		D3DXMATRIX tmpMatrix;
		D3DXMatrixIdentity(&tmpMatrix);
		tmpMatrix[12]=vNormal2.x;
		tmpMatrix[13]=vNormal2.y;
		tmpMatrix[14]=vNormal2.z;
		D3DXMatrixMultiply(&tmpMatrix, &mainRotMatrix, &tmpMatrix);
		vNormal2.x=tmpMatrix[12];
		vNormal2.y=tmpMatrix[13];
		vNormal2.z=tmpMatrix[14];
		*/
		//D3DXVec3Normalize(&vNormal2, &vNormal2);
		vNormal2.normalize();

		normal.x = (float)(vNormal1.x/2+vNormal2.x/6);
		normal.y = (float)(vNormal1.y/2+vNormal2.y/6);
		normal.z = (float)(vNormal1.z/2+vNormal2.z/6);

		//D3DXVec3Normalize(&normal, &normal);
		normal.normalize();

	} else {
		normal.x=0;
		normal.y=0;
		normal.z=0;
	}

	//return vNormal1;
	return normal;
}

signed char * getVertexPtr(unsigned char num) {
	return mod->Vertices_ptr+num/2*6;
}

// Получить вершину, переданную субобъекту предыдущим объектом. Не уверен, что правильно.
// Используется, например, для совмещения внутреннего помещения станции со входом.
void getExpVertex(float *p, int vertNum) {
	int num;

	num=vertNum-251;

		expVert[0]=(float)*(int*)(*(unsigned int*)(gptr+0x80-num*4)+4)/DIVIDER;
		expVert[1]=(float)*(int*)(*(unsigned int*)(gptr+0x80-num*4)+8)/DIVIDER;
		expVert[2]=(float)*(int*)(*(unsigned int*)(gptr+0x80-num*4)+12)/DIVIDER;
		expVertOrigin[0]=(float)*(int*)(*(unsigned int*)(gptr+0x70)+4)/DIVIDER;
		expVertOrigin[1]=(float)*(int*)(*(unsigned int*)(gptr+0x70)+8)/DIVIDER;
		expVertOrigin[2]=(float)*(int*)(*(unsigned int*)(gptr+0x70)+12)/DIVIDER;

		p[0]=expVert[0]-expVertOrigin[0];
		p[1]=expVert[1]-expVertOrigin[1];
		p[2]=expVert[2]-expVertOrigin[2];

		if (num==0) {
			prevPoint[0]=(float)((long)(p[0]*100*DIVIDER)>>currentScale);
			prevPoint[1]=(float)((long)(p[1]*100*DIVIDER)>>currentScale);
			prevPoint[2]=(float)((long)(p[2]*100*DIVIDER)>>currentScale);

			prevPoint[0]/=100;
			prevPoint[1]/=100;
			prevPoint[2]/=100;
		}

		D3DXMATRIX tmpMatrix, oneMatrix;
		D3DXMatrixIdentity(&tmpMatrix);
		D3DXMatrixIdentity(&oneMatrix);
		tmpMatrix[12]=p[0];
		tmpMatrix[13]=p[1];
		tmpMatrix[14]=p[2];
		D3DXMatrixInverse(&oneMatrix,NULL, &mainRotMatrix);
		D3DXMatrixMultiply(&tmpMatrix, &tmpMatrix, &oneMatrix);
		p[0]=tmpMatrix[12];
		p[1]=tmpMatrix[13];
		p[2]=tmpMatrix[14];

}

// Вершины имеют типы. См. доки от Jongware.
void getTransVertex(float *p, int vertNum) {
	int rnd;
	float pta[3], ptb[3], ptc[3];	
	signed char *vertOffs;

	vertOffs = getVertexPtr(vertNum);

	if (vertNum<=250) {

	p[0]=(float)*(vertOffs+2);
	p[1]=(float)*(vertOffs+3);
	p[2]=(float)*(vertOffs+4);

	if (*(vertOffs)==5) {	// Обычная вершина
			getTransVertex(pta, (unsigned char)p[2]);
			p[0]=pta[0]*-1;
			p[1]=pta[1]*-1;
			p[2]=pta[2]*-1;
	} else if (*(vertOffs)==7) { // Со случайной величиной
			getTransVertex(pta, (unsigned char)p[2]);
			rnd=(int)p[2]/10;
			p[0]=pta[0]+ (rnd > 0 ? (rand()%rnd-rnd/2) : 0.0f);
			p[1]=pta[1]+ (rnd > 0 ? (rand()%rnd-rnd/2) : 0.0f);
			p[2]=pta[2]+ (rnd > 0 ? (rand()%rnd-rnd/2) : 0.0f);
	} else if (*(vertOffs)==11) { // Среднее между
			getTransVertex(pta, (unsigned char)p[2]);
			getTransVertex(ptb, (unsigned char)p[1]);
			p[0]=pta[0]/2+ptb[0]/2;
			p[1]=pta[1]/2+ptb[1]/2;
			p[2]=pta[2]/2+ptb[2]/2;
	} else if (*(vertOffs)==15) { // Комбинация из вершин
			getTransVertex(pta, (unsigned char)p[0]);
			getTransVertex(ptb, (unsigned char)p[1]);
			getTransVertex(ptc, (unsigned char)p[2]);
			p[0]=ptb[0]+pta[0]-ptc[0];
			p[1]=ptb[1]+pta[1]-ptc[1];
			p[2]=ptb[2]+pta[2]-ptc[2];
	} else if (*(vertOffs)==17) { // Неизвестный тип. Экспериментирую.
			getTransVertex(pta, (unsigned char)p[0]);
			pta[2]+=p[1];
			pta[0]+=p[1];
			p[0]=pta[0];
			p[1]=pta[1];
			p[2]=pta[2];

			//p[0]=prevPoint[0];
			//p[1]=prevPoint[1];
			//p[2]=prevPoint[2]-4;
	} else if (*(vertOffs)==19) { // Интерполяция
		// Частично работает (шасси, опускающиеся крылья)
		// Не совпадает с оригиналом (туба на skeet cruiser)
		// Частично глючит (хвост у turner class). Хотя у него
		// может быть проблема с глоб. или лок. переменными.
			unsigned short var=getVar((unsigned char)p[0]);
			getTransVertex(pta, (unsigned char)p[2]);
			getTransVertex(ptb, (unsigned char)p[1]);
			p[0]=ptb[0]+(pta[0]-ptb[0])/0xFFFF*var;
			p[1]=ptb[1]+(pta[1]-ptb[1])/0xFFFF*var;
			p[2]=ptb[2]+(pta[2]-ptb[2])/0xFFFF*var;

	} else if (*(vertOffs)==23) { // Неизвестный тип. Экспериментирую
			getTransVertex(pta, (unsigned char)p[1]);
			getTransVertex(ptb, (unsigned char)p[2]);
			p[1]=pta[1];
			p[2]=ptb[2];
	}	
	
	if (vertNum%2>0) {
		p[0]*=-1;
	}

	} else {
		getExpVertex(p, vertNum);
	}
}
unsigned char rotate=0;
bool rotateNext=false;

// Для получения нужной вершины текущей модели
inline void getVertex(float *p, unsigned char num, float *origin, unsigned char scale, unsigned char orient) {
	int sFactor=1;
	ffeVertex* vertex;
	D3DXMATRIX tmpMatrix, oneMatrix, twoMatrix;
	D3DXVECTOR3 v;
	
	D3DXMatrixIdentity(&tmpMatrix);		
	D3DXMatrixIdentity(&oneMatrix);
	D3DXMatrixIdentity(&twoMatrix);
	
	if (num==255) {
		p[0]=0;
		p[1]=0;
		p[2]=0;
		return;
	}

	if (true) {
		//if (DATA_009200[num].unknown6 != 0) {
		//	vertex = &DATA_009200[num];
		//} else {
			vertex = FUNC_001470_getVertex(gptr, num);
		//}		

		v.x=(float)vertex->x/DIVIDER;
		v.y=(float)vertex->y/DIVIDER;
		v.z=(float)vertex->z/DIVIDER;

		D3DXMatrixInverse(&oneMatrix,NULL, &mainRotMatrixO);
		D3DXVec3TransformCoord(&v,&v,&oneMatrix);
		D3DXVec3TransformCoord(&v,&v,&currentMatrix);
		if (billboardMatrix) {
			D3DXVec3TransformCoord(&v,&v,&oneMatrix);
		}

		p[0]=v.x;
		p[1]=v.y;
		p[2]=v.z;

	} else {
	
		if (num<=250) {
			//getTransVertex(p, num);
			p[0]=vertbuf[num][0];
			p[1]=vertbuf[num][1];
			p[2]=vertbuf[num][2];
		} else {
			getExpVertex(p, num);
		}

		v.x = p[0];	
		v.y = p[1];
		v.z = p[2];
		
		D3DXVec3TransformCoord(&v,&v,&currentMatrix);
		if (billboardMatrix) {
			D3DXMatrixInverse(&oneMatrix,NULL, &mainRotMatrix);
			D3DXVec3TransformCoord(&v,&v,&oneMatrix);
		}

		p[0]=v.x;
		p[1]=v.y;
		p[2]=v.z;

	}

	if (currentModel<3) {
		//p[0]+=getNormal(2).x*10;
		//p[1]+=getNormal(2).y*10;
		//p[2]+=getNormal(2).z*10;
		if (currentModel<3 && (*(int *)(gptr+0xFC)==1902 || *(int *)(gptr+0xFC)==0xbe271aa0)) {
			//p[0]*=-1;
			//p[2]-=0.002f*sqrt(posMatrix[12]*posMatrix[12]+posMatrix[13]*posMatrix[13]+posMatrix[14]*posMatrix[14]);
		} else {
			//p[2]+=0.002f*sqrt(posMatrix[12]*posMatrix[12]+posMatrix[13]*posMatrix[13]+posMatrix[14]*posMatrix[14]);
			p[0]+=0.01f;
			p[1]+=0.01f;
			p[2]+=0.01f;
		}
	}
	if ((!currentTex && (currentColor.r==0 && currentColor.g==0 && currentColor.b==0)) || 
		//currentTex==72 || 
		//(currentTex>=90 && currentTex<=93) ||
		//currentTex==93 || 
		//currentTex==441 || 
		(currentTex>=460 && currentTex<=462)) {
		//int scaleFactor = abs(10-mod->Scale);
		//float r=(float)((short)mod->Radius>>scaleFactor)*2/DIVIDER;
		p[0]*=1.015;
		p[1]*=1.015;
		p[2]*=1.015;
	}
	if (currentColor.r==0 && currentColor.g==238 && currentColor.b==238 && currentTex==0) {
		if (currentModel==27) {
			p[0]*=1.08;
			p[1]*=1.08;
			p[2]*=1.08;
		} else {
			p[0]*=1.02;
			p[1]*=1.02;
			p[2]*=1.02;
		}
	}

}

#ifndef CALLBACK
# ifdef WIN32
#  define CALLBACK __attribute__ ((__stdcall__))
# else
#  define CALLBACK
# endif
#endif /* CALLBACK */

int ucount;
int curVer;
int addit;
short currWhich;
static D3DXVECTOR3 pNormals[2000];
static D3DXVECTOR3 pNormals2[2000];
static D3DXVECTOR3 pVertices[2000];
static D3DXVECTOR3 pcenter;

// Тесселяция стартанула
void CALLBACK beginCallback(GLenum which)
{
	curVer=vertexNum;
	currWhich=which;
	ucount=0;
	addit=0;
}

void CALLBACK errorCallback(GLenum errorCode)
{
   const GLubyte *estring;

   estring = gluErrorString(errorCode);
   fprintf(stderr, "Tessellation Error: %s\n", estring);
}

// Тесселяция закончена. Пересчитаем нормали (глючно)
void CALLBACK endCallback(void)
{
	if (ucount == 0) return;

	vertexType[curVer].type=currWhich;
	vertexType[curVer].amount = ucount;
	vertexType[curVer].textNum = currentTex;
	vertexType[curVer].doLight = doLight;

	Vect3f vNormal,oldNormal;
	oldNormal = getNormal(currentNormal);
	oldNormal.normalize();

	vNormal = GetTriangeNormal(&Vect3f(pVertices[1].x, pVertices[1].y, pVertices[1].z),
								&Vect3f(pVertices[2].x, pVertices[2].y, pVertices[2].z),
								&Vect3f(pVertices[3].x, pVertices[3].y, pVertices[3].z));

	/*
	for(int i=0;i<ucount;i+=3) {
	
		vNormal = GetTriangeNormal(&Vect3f(pVertices[i].x, pVertices[i].y, pVertices[i].z),
									&Vect3f(pVertices[i+1].x, pVertices[i+1].y, pVertices[i+1].z),
									&Vect3f(pVertices[i+2].x, pVertices[i+2].y, pVertices[i+2].z));

		if ((oldNormal.x>0 && vNormal.x<0) || (oldNormal.x<0 && vNormal.x>0))
			vNormal.x*=-1;
		if ((oldNormal.y>0 && vNormal.y<0) || (oldNormal.y<0 && vNormal.y>0))
			vNormal.y*=-1;
		if ((oldNormal.z>0 && vNormal.z<0) || (oldNormal.z<0 && vNormal.z>0))
			vNormal.z*=-1;

		pNormals[i].x=vNormal.x;
		pNormals[i].y=vNormal.y;
		pNormals[i].z=vNormal.z;
		pNormals[i+1].x=vNormal.x;
		pNormals[i+1].y=vNormal.y;
		pNormals[i+1].z=vNormal.z;
		pNormals[i+2].x=vNormal.x;
		pNormals[i+2].y=vNormal.y;
		pNormals[i+2].z=vNormal.z;
		pNormals2[i].x=vNormal.x;
		pNormals2[i].y=vNormal.y;
		pNormals2[i].z=vNormal.z;
		pNormals2[i+1].x=vNormal.x;
		pNormals2[i+1].y=vNormal.y;
		pNormals2[i+1].z=vNormal.z;
		pNormals2[i+2].x=vNormal.x;
		pNormals2[i+2].y=vNormal.y;
		pNormals2[i+2].z=vNormal.z;

	}
	
	Vect3f n;
	int a;
	for(int i=0;i<ucount;i++) {
		
		n.x=0;
		n.y=0;
		n.z=0;
		a=0;
		for(int k=0;k<ucount;k++) {
			if (pVertices[i].x==pVertices[k].x &&
				pVertices[i].y==pVertices[k].y &&
				pVertices[i].z==pVertices[k].z) {
				n.x+=pNormals2[k].x;
				n.y+=pNormals2[k].y;
				n.z+=pNormals2[k].z;
				a++;
			}
		}
		n.x/=a;
		n.y/=a;
		n.z/=a;

		// Из-за кривой тесселяции приходится извращаться с нормалями

		// Добавим среднее от вершины
		n.x=n.x/2+(pVertices[i].x-pcenter.x)/2;
		n.y=n.y/2+(pVertices[i].y-pcenter.y)/2;
		n.z=n.z/2+(pVertices[i].z-pcenter.z)/2;
		// Добавим среднее от оригинальной нормали FFE
		n.x=n.x/2+oldNormal.x/2;
		n.y=n.y/2+oldNormal.y/2;
		n.z=n.z/2+oldNormal.z/2;
		n.x=oldNormal.x;
		n.y=oldNormal.y;
		n.z=oldNormal.z;
		n.normalize();
		//D3DXVec3Normalize(&n, &n);
		pNormals[i].x=n.x;
		pNormals[i].y=n.y;
		pNormals[i].z=n.z;
	}
	*/
	//if(FAILED(d3d_vb->Lock(sizeof(CUSTOMVERTEX)*curVer,sizeof(CUSTOMVERTEX)*ucount,(BYTE**)&Vertices,0))) return;
	VertexXYZNDT1* Vertices = (VertexXYZNDT1*)renderSystem->LockVertexBuffer(vertexBuffer,curVer,ucount*vertexBuffer.vertexSize);
	for(int i=0;i<ucount;i++) {
		(Vertices+i)->n.x = oldNormal.x;//pNormals[i].x;
		(Vertices+i)->n.y = oldNormal.y;//pNormals[i].y;
		(Vertices+i)->n.z = oldNormal.z;//pNormals[i].z;
	}
	renderSystem->UnlockVertexBuffer(vertexBuffer);
	
	//d3d_vb->Unlock();

	// Можно посмотреть на нормали
/*
	float p1[3],p2[3];
	for(int i=0;i<ucount;i++) {
		p1[0]=pVertices[i].x;
		p1[1]=pVertices[i].y;
		p1[2]=pVertices[i].z;
		p2[0]=pVertices[i].x+pNormals[i].x;
		p2[1]=pVertices[i].y+pNormals[i].y;
		p2[2]=pVertices[i].z+pNormals[i].z;
		DrawRealLine(p1,p2);
	}
*/	
}

// Получаем вершину из тесселятора
void CALLBACK vertexCallback(double *vertex)
{
   double p[3];
   p[0]=vertex[0];
   p[1]=vertex[1];
   p[2]=vertex[2];
   //p[0]-=1000;
   //p[1]-=1000;
   //p[2]-=1000;

   if (!skipCurrentModel)
		addVertex(p,0);
   else
	   return;

   pVertices[ucount].x=(float)p[0];
   pVertices[ucount].y=(float)p[1];
   pVertices[ucount].z=(float)p[2];

   ucount++;
}

void CALLBACK edgeFlag(GLboolean flag)
{
    // empty
}



/*  combineCallback is used to create a new vertex when edges
 *  intersect.  coordinate location is trivial to calculate,
 *  but weight[4] may be used to average color, normal, or texture
 *  coordinate data.
 */
GLUtesselator *tobj = NULL;
GLdouble vertex[1000][3];
void CALLBACK combineCallback(GLdouble coords[3], 
                     GLdouble *vertex_data[4],
                     GLfloat weight[4], GLdouble **dataOut )
{
	// В туториалах используют malloc, но это была бы явная
	// утечка памяти. Лучше через доп. буфер.
	vertex[addit][0] = coords[0];
	vertex[addit][1] = coords[1];
	vertex[addit][2] = coords[2];
	*dataOut = &vertex[addit][0];
	addit++;
}

// Скрипт задает цвет и текстуру
void setMaterial(short cmd) {

	useLocalColor=false;
	currentTex=-1;

	if (currentModel==166) {
		int a=0;
	}
	
	if ((short)(cmd & 0x2000)==0x2000) {
		doLight=false;
	} else {
		doLight=true;
	}

	if ((short)(cmd & 0x1000)==0x1000) {
		currentR=localColor[0]*17;
		currentG=localColor[1]*17;
		currentB=localColor[2]*17;
		currentColor.set(currentR,
			currentG,
			currentB);
		useLocalColor=true;
	}

	if ((short)(cmd & 0x4000)==0x4000) {
		currentTex=400+(cmd & 0x0FFF);
		if (currentColor.r==0 && currentColor.g==0 && currentColor.b==0) {
			currentColor.set(255,255,255);
		}
	} else if (!useLocalColor) {
		currentTex=-1;
		short rgb=(cmd & 0x0FFF);
		short r=(rgb&0x0F00)>>8;
		short g=(rgb&0x00F0)>>4;
		short b=(rgb&0x000F);
		currentR=r*17;
		currentG=g*17;
		currentB=b*17;
		currentColor.set(currentR,currentG,currentB);
	}
}

// Математика скрипта. Возможно, не совсем корректно работает.
// Смотри доки от Jongware.
void doMath(unsigned short *command) {
	int t;
	int result;
	unsigned char dest=(*command&0xFF00)>>8;

	int operand = (*command&0xF0)>>4;

	int source1 = getVar(*(command+1)&0xFF);
	int source2 = getVar((*(command+1)&0xFF00)>>8);

	switch (operand) {
	case 0:
		result=source1+source2;
		break;
	case 1:
		result=source1-source2;
		break;
	case 2:
		result=source1*source2;
		break;
	case 3:
		result=source1/source2;
		break;
	case 4:
		result=source1>>source2;
		break;
	case 5:
		result=source1<<source2;
		break;
	case 6:
		result=max(source1,source2);
		break;
	case 7:
		result=min(source1,source2);
		break;
	case 8:
		result=source1 * 0x10000 / source2;
		break;
	case 9:
		result=source1>>source2;
	case 10:
		result=source1<<source2;
	case 11:
		result=(source1<source2) ? 0 : source1-source2;
		break;
	case 12:
		result=(source1>source2) ? source2 : source1;
		break;
	case 13:
		result=0;//(int)(source1 * sin(source2));
		break;
	case 14:
		result=0;//(int)(source1 * cos(source2));
		break;
	case 15:
		result=source1&source2;
		break;
	default: 
		result=0;
		break;
	}

	if ((unsigned char)(dest&0xC0)==0xC0) {
		t=dest&0x3F;
		localvar[t]=result;
	} else if ((unsigned char)(dest&0x80)==0x80) {
		t=dest&0x3F;
		globalvar[t]=(short)result;
	}

}

// Нифига не понял, как правильно обрабатывать флаги
void setMatrix(unsigned char orient) {
	D3DXVECTOR3 v1,v2;
/*
#166  starmap grid
0100    009C 0000 ; Rotate -- default 0000
        C01E ; Cmd 1Eh (parm. 0600h)
        FFE6 ; Cmd 6 [7]: 2047

#170  purple circle
1001    8019 ; Cmd 19, no-op
        C01E ; Cmd 1Eh (parm. 0600h)
        013C 0000 ; Rotate -- default 0000
        C01E ; Cmd 1Eh (parm. 0600h)

#173  starmap nebula
1100    019C 0000 ; Rotate -- default 0000
        C01E ; Cmd 1Eh (parm. 0600h)
        FFC6 ; Cmd 6 [7]: 2046

	FROM JONGWARE:
	No surprise  ... bits 0..2 mirror, 3..5 rotate around one of the axes (and 
	special value 6 uses a pre-set rotation matrix). In my wireframe mesh viewer 
	you can see some correct rotations, some bad... and the latest version draws 
	some of the Radiation warning signs on the Panther upside down -- again! 


	AND 38h negates rows per bit, AND 07h gives 5 additional rotations. 
	Bits 20h negates the top row of your matrix, 10h the second row, and 08h the 
	third row. 
	01h to 05h rotate 90deg along the x, y, and z axes. I had some problems 
	interpreting this until I found you have to do the negate first (if 
	necessary), then the rotation. Value 06h is a special (arbitrary) rotation 
	angle, and 07h seems unused (though I'm getting increasingly careful stating 
	something is "unused" :) 
	I still have problems with this rotation, though -- I'll try to update 
	galaxy7.html a.s.a. I have a fully working wireframe ... 
	At the mo' I don't even dare to *think* of planet code... 
*/
	if (currentModel==166)
		D3DXMatrixRotationZ(&currentMatrix, 3.14f/2);
	if (currentModel==170)
		D3DXMatrixRotationX(&currentMatrix, 3.14f/2);
	if (currentModel==173)
		D3DXMatrixRotationY(&currentMatrix, 3.14f/2);
}

float nullOrigin[3]={0.0f, 0.0f, 0.0f};
float dist, radius;


//	Смотри "The Frontier Galaxy VII.htm" от Jongware
extern void drawModel(int num, float *origin, unsigned char orient, int scale)
{
	int i,numpoly, n, m, startpoly;
	float p1[3], p2[3], p3[3], p4[3];
	D3DXMATRIX tmpMatrix, oneMatrix;
	ffeVertex* vertex;
	unsigned short *command;
	int cn=0;
	numpoly=0;

	if (num!=-1) {
		//mod = (Model_t *)*((unsigned int *)model+num);
		mod = (Model_t *)*(unsigned int *)gptr;
		currentModel=num;
		command=gcmd;
	}

/*
	for(int i=0;i<mod->NumVertices;i++) {
		//if (DATA_009200[num].unknown6 != 0) {
		//	vertex = &DATA_009200[num];
		//} else {
			vertex = FUNC_001472(gptr, i);
		//}		

		vertbuf[i][0]=(float)vertex->x/DIVIDER;
		vertbuf[i][1]=(float)vertex->y/DIVIDER;
		vertbuf[i][2]=(float)vertex->z/DIVIDER;

		tmpMatrix[12]=vertbuf[i][0];
		tmpMatrix[13]=vertbuf[i][1];
		tmpMatrix[14]=vertbuf[i][2];
		D3DXMatrixInverse(&oneMatrix,NULL, &mainRotMatrixO);
		D3DXMatrixMultiply(&tmpMatrix, &tmpMatrix, &oneMatrix);
		vertbuf[i][0]=tmpMatrix[12];
		vertbuf[i][1]=tmpMatrix[13];
		vertbuf[i][2]=tmpMatrix[14];
		*/
		/*
		getTransVertex(vertbuf[i], i);

		// Нужно, для не потерять десятые доли от деления. Если убрать, потеряем плавность.
		if (currentScale<10) {
			vertbuf[i][0]*=10;
			vertbuf[i][1]*=10;
			vertbuf[i][2]*=10;
		}

		vertbuf[i][0]=(float)((long)vertbuf[i][0]<<(currentScale))/DIVIDER;
		vertbuf[i][1]=(float)((long)vertbuf[i][1]<<(currentScale))/DIVIDER;
		vertbuf[i][2]=(float)((long)vertbuf[i][2]<<(currentScale))/DIVIDER;

		if (currentScale<10) {
			vertbuf[i][0]/=10;
			vertbuf[i][1]/=10;
			vertbuf[i][2]/=10;
		}
		*/
	//}
/*
	if (mod!=NULL)
		command = mod->Mesh_ptr;
	else
		return;
*/
	//localColor[0]=mod->DefaultColorR*17;
	//localColor[1]=mod->DefaultColorG*17;
	//localColor[2]=mod->DefaultColorB*17;
	localColor[0]=*(gptr+0xF4);
	localColor[1]=*(gptr+0xF5);
	localColor[2]=*(gptr+0xF6);

	if (localColor[0]==localColor[1]==localColor[2]) {
		localColor[0]=mod->DefaultColorR;
		localColor[1]=mod->DefaultColorG;
		localColor[2]=mod->DefaultColorB;
	}

	//currentColor.set(localColor[0],localColor[1],localColor[2]);
	currentColor.set(255,255,255);
	currentTex=-1;

	modelList[modelNum].splineR=0;
	modelList[modelNum].splineG=0;
	modelList[modelNum].splineB=0;

	int vert=vertexNum;
	
	// Этот кусочек использовался для забивания на далекие мелкие объекты.
	// Например, куски города на марсе.
	//if (currentModel>=81 && currentModel<=102) {
		/*
		D3DXMATRIX tmpMatrix;
		D3DXMatrixIdentity(&tmpMatrix);
		tmpMatrix[12]=(float)origin[0];
		tmpMatrix[13]=(float)origin[1];
		tmpMatrix[14]=(float)origin[2];
		D3DXMatrixMultiply(&tmpMatrix, &mainRotMatrix, &tmpMatrix);
		D3DXMatrixMultiply(&tmpMatrix, &posMatrix, &tmpMatrix);
		*/
		//double dist=sqrt(posMatrix[12]*posMatrix[12]+posMatrix[13]*posMatrix[13]+posMatrix[14]*posMatrix[14]);
		//if (currentMatrix>0 && dist>10000)
		//	return;
		//if (currentMatrix>1 && dist>1500)
		//	return;
		//if (posMatrix[13]<2000 && dist>800)
		//	return;
//	}

	while(1) {
		if (mod==NULL) {
			return;
		}
		if (*(command)==0x0000)
			return;
		if (*(unsigned int*)DATA_007824==0)
			return;
		if (currentModel==2)
			transparent=true;
		else
			transparent=false;
		doLight=true;
		currentTex=false;
		cn++;
		if (*command==0x0001) {		// Ball
			// Есть проблемы с радиусом
			setMaterial(*(command+1));
			getVertex(p1, getByte(command, 4), origin, scale, orient);
			float r;
			//int scaleFactor = currentScale + currentScale2 - 8;

			/*
			if (scaleFactor < 0) {
				radius -= (dist << -scaleFactor);
			} else if (scaleFactor < 0x10) { 
				radius = (radius << scaleFactor) - dist; 
				scaleFactor = 0; 
			} else { 
				scaleFactor -= 0x10; 
				radius = (radius << 0x10) - (dist >> scaleFactor);
			}
			*/

			if (*(command+2) >> 8 == 0)
				r=(float)getVar(*(command+2))/DIVIDER;
			else {
				int scaleFactor = abs(10-currentScale);
				r=(float)((short)*(command+2)>>scaleFactor)*2/DIVIDER;
			}

			if (getByte(command, 5)&0x80) { // Лампочка
				transparent=true;
/*
				D3DXVECTOR4 out;
				D3DXMATRIX invMatrix;

				D3DXVec3Transform(&out,&D3DXVECTOR3(p1[0],p1[1],p1[2]), &mainRotMatrix);
				out.z-=0.01f;
				D3DXMatrixInverse(&invMatrix, NULL, &mainRotMatrix);
				D3DXVec3Transform(&out,&D3DXVECTOR3(out.x,out.y,out.z), &invMatrix);

				p1[0]=out.x;
				p1[1]=out.y;
				p1[2]=out.z;
*/
				DrawRealPoint(p1, *(command+2));

				for(int ii=0;ii<10;ii++) {
					if (modelList[mainModelNum].light[ii].enable) continue;

					D3DXVECTOR4 out;

					D3DXVec3Transform(&out,&D3DXVECTOR3(p1[0],p1[1],p1[2]), &mainRotMatrix);

					out.x+=posMatrix[12];
					out.y+=posMatrix[13];
					out.z+=posMatrix[14];

					modelList[mainModelNum].light[ii].enable=true;
					modelList[mainModelNum].light[ii].r=1.0f/255*currentR;
					modelList[mainModelNum].light[ii].g=1.0f/255*currentG;
					modelList[mainModelNum].light[ii].b=1.0f/255*currentB;
					modelList[mainModelNum].light[ii].pos.x=out.x;
					modelList[mainModelNum].light[ii].pos.y=out.y;
					modelList[mainModelNum].light[ii].pos.z=out.z;
					modelList[mainModelNum].light[ii].dist=3.0f;

					break;
				}

			} else { // Шарик	
				if (!skipCurrentModel)
					DrawRealSphere(p1, r, currentTex, 40);
			}
			command+=4;
			continue;
		}
		if (*command==0x0002) {		// Line
			setMaterial(*(command+1));
			getVertex(p1, getByte(command, 2), origin, scale, orient);
			getVertex(p2, getByte(command, 3), origin, scale, orient);

			if (currentModel==156 || 					// Laser
				currentModel==319 ||						// Hyperspace warp
				(currentModel>353 && currentModel<361)) {	// Laser intro
				float width;
				if (posMatrix[14]<1 && posMatrix[14]>-1)
					p1[1]-=0.4;
				if (currentModel!=319) {
					width=0.125f;
/*
					for(int ii=0;ii<10;ii++) {
						if (modelList[mainModelNum].light[ii].enable) continue;

						D3DXVECTOR4 out;

						D3DXVec3Transform(&out,&D3DXVECTOR3(p1[0],p1[1],p1[2]), &mainRotMatrix);

						out.x+=posMatrix[12];
						out.y+=posMatrix[13];
						out.z+=posMatrix[14];

						modelList[mainModelNum].light[ii].enable=true;
						modelList[mainModelNum].light[ii].r=1.0f/255*currentR;
						modelList[mainModelNum].light[ii].g=1.0f/255*currentG;
						modelList[mainModelNum].light[ii].b=1.0f/255*currentB;
						modelList[mainModelNum].light[ii].pos.x=out.x;
						modelList[mainModelNum].light[ii].pos.y=out.y;
						modelList[mainModelNum].light[ii].pos.z=out.z;
						modelList[mainModelNum].light[ii].dist=10.0f;

						break;
					}
*/
				} else {
					width=0.025f;
				}
				currentTex=705;
				doLight=false;
				transparent=true;
				if (!skipCurrentModel) {
					drawBillLine(p1, p2, width);
					//currentTex=0;
					DrawRealLine(p1, p2);
				}
				if (currentModel!=319) {
					currentTex=706;
					//getVertex(p1, getByte(command, 2), origin, scale, orient);
					if (!skipCurrentModel)
						DrawBillboard(p1, width*4);
				}
			} else {
				currentTex=-1;
				if (!skipCurrentModel)
					DrawRealLine(p1, p2);
			}
			command+=3;
			continue;
		}
		if (*command==0x0003) {		// Triangle
			setMaterial(*(command+1));
			getVertex(p1, getByte(command, 2), origin, scale, orient);
			getVertex(p2, getByte(command, 3), origin, scale, orient);
			getVertex(p3, getByte(command, 4), origin, scale, orient);
			currentNormal=getByte(command, 5);
			//if (checkOrientation(p1,p2,p3)==true) {
				if (!skipCurrentModel)
					DrawRealTriangle(p1, p2, p3, 2, getByte(command, 5));
			//} else {
			//	if (!skipCurrentModel)
			//		DrawRealTriangle(p3, p2, p1, 2, getByte(command, 5));
			//}
			command+=4;
			continue;
		}	
		if (*command==0x0004) {		// Square
			setMaterial(*(command+1));

			getVertex(p1, getByte(command, 2), origin, scale, orient);
			getVertex(p2, getByte(command, 5), origin, scale, orient);
			getVertex(p3, getByte(command, 3), origin, scale, orient);
			getVertex(p4, getByte(command, 4), origin, scale, orient);
			currentNormal=getByte(command, 7);

			if (currentModel==336) { // smoke
				transparent=true;
			}
			if (currentModel==169 || currentModel==170) { // cross
				doLight=false;
				currentColor.set(200,200,200);
				transparent=true;
			}

			if (currentTex>=464 && currentTex<=493) { // explode
				currentTex=464+(localvar[1]/1536);
				if (currentTex>493)
					currentTex=493;
				useLocalColor=false;
				transparent=true;
			}

			//if (checkOrientation(p1,p2,p3)==true) {
				if (!skipCurrentModel)
					DrawRealSquare2(p1, p2, p3, p4, getByte(command, 7));
			//} else {
			//	if (!skipCurrentModel)
			//		DrawRealSquare2(p4, p3, p2, p1, getByte(command, 7));
			//}
			command+=5;
			continue;
		}

		// Используется GluTess. Смотри "ProgZ_ru - gluTessVertex.htm"
		if (*command==0x0005) {		// Polygon
			int pcc=0;
			int spl=16;
			int firstVertex=-1;
			int lastVertex=0;

			vcount=0;		
			startpoly=0;
			setMaterial(*(command+1));
			command+=1;
			int c=getByte(command, 0);
			currentNormal=getByte(command, 1);
			numpoly++;

			if (modelList[modelNum].splineR==0 && modelList[modelNum].splineG==0 && modelList[modelNum].splineB==0) {
				modelList[modelNum].splineR=currentColor.r;
				modelList[modelNum].splineG=currentColor.g;
				modelList[modelNum].splineB=currentColor.b;
			}

			if (tobj == NULL) {
				tobj = gluNewTess ();
				//gluTessNormal (tobj, 0, 0, 1);
				//gluTessProperty(tobj, GLU_TESS_BOUNDARY_ONLY, GL_TRUE);				
				gluTessCallback(tobj, GLU_TESS_VERTEX, (_GLUfuncptr) vertexCallback);
				gluTessCallback(tobj, GLU_TESS_BEGIN, (_GLUfuncptr) beginCallback);
				gluTessCallback(tobj, GLU_TESS_END, (_GLUfuncptr) endCallback);
				gluTessCallback(tobj, GLU_TESS_ERROR, (_GLUfuncptr) errorCallback);
				gluTessCallback(tobj, GLU_TESS_COMBINE, (_GLUfuncptr) combineCallback);
				gluTessCallback(tobj, GLU_TESS_EDGE_FLAG, (_GLUfuncptr) edgeFlag);
			}

			Vect3f oldNormal;
			oldNormal = getNormal(currentNormal);
			//gluTessNormal (tobj, oldNormal.x, oldNormal.y, oldNormal.z);
			gluTessProperty(tobj, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_ODD);

			gluTessBeginPolygon (tobj, NULL);

			for(i=2;i<=c;i++) {
				if (getByte(command, i)==0x04) {	// start line
					getVertex(p1, getByte(command, i+1), origin, scale, orient);
					if (firstVertex==-1)
						firstVertex=getByte(command, i+1);
					ver[vcount][0]=p1[0];
					ver[vcount][1]=p1[1];
					ver[vcount][2]=p1[2];
					vcount++;

					getVertex(p1, getByte(command, i+3), origin, scale, orient);
					lastVertex=getByte(command, i+3);
					ver[vcount][0]=p1[0];
					ver[vcount][1]=p1[1];
					ver[vcount][2]=p1[2];
					vcount++;
					
					i+=3;
					continue;
				}
				if (getByte(command, i)==0x06) {	// continue line					
					getVertex(p1, getByte(command, i+1), origin, scale, orient);
					lastVertex=getByte(command, i+1);
					ver[vcount][0]=p1[0];
					ver[vcount][1]=p1[1];
					ver[vcount][2]=p1[2];
					vcount++;

					i+=1;
					continue;
				}
				if (getByte(command, i)==0x02) {	// start curve
					float out[3];
					float ctrlpoints[4][3];

					getVertex(ctrlpoints[0], getByte(command, i+2), origin, scale, orient);
					getVertex(ctrlpoints[1], getByte(command, i+3), origin, scale, orient);
					getVertex(ctrlpoints[2], getByte(command, i+4), origin, scale, orient);
					getVertex(ctrlpoints[3], getByte(command, i+5), origin, scale, orient);

					/*
					int oldColor = currentColor;
					currentColor = D3DCOLOR_XRGB(255,255,0);
					DrawRealLine(ctrlpoints[0], ctrlpoints[1]);
					DrawRealLine(ctrlpoints[1], ctrlpoints[2]);
					DrawRealLine(ctrlpoints[2], ctrlpoints[3]);
					currentColor = oldColor;
					*/

					if (firstVertex==-1)
						firstVertex=getByte(command, i+2);
					lastVertex=getByte(command, i+5);

					for (m=0; m<=spl; m++) {
						eval_bezier (out, (float)m/(float)spl, ctrlpoints);
						ver[vcount][0]=out[0];
						ver[vcount][1]=out[1];
						ver[vcount][2]=out[2];
						vcount++;
					}

					i+=5;
					continue;
				}
				if (getByte(command, i)==0x08) {	// continue curve
					float out[3];
					float ctrlpoints[4][3];

					getVertex(ctrlpoints[0], lastVertex, origin, scale, orient);
					getVertex(ctrlpoints[1], getByte(command, i+1), origin, scale, orient);
					getVertex(ctrlpoints[2], getByte(command, i+2), origin, scale, orient);
					getVertex(ctrlpoints[3], getByte(command, i+3), origin, scale, orient);
					lastVertex=getByte(command, i+3);

					for (m=1; m<=spl; m++) {
						eval_bezier (out, (float)m/(float)spl, ctrlpoints);
						ver[vcount][0]=out[0];
						ver[vcount][1]=out[1];
						ver[vcount][2]=out[2];
						vcount++;
					}

					i+=3;
					continue;
				}
				if (getByte(command, i)==0x0A &&	// End inner contour
					getByte(command, i+1)==0x00) {
					
					
					int zzz;
					zzz=0;
					
					
					if (lastVertex!=firstVertex) {
						getVertex(p1, firstVertex, origin, scale, orient);
						ver[vcount][0]=p1[0];
						ver[vcount][1]=p1[1];
						ver[vcount][2]=p1[2];
						vcount++;
					}
					
					//if (lastVertex==firstVertex)
					//	vcount--;
/*
					// Центр полигона по Y. Небольшой патч для пересчета нормалей.
					p1[0]=0;
					p1[1]=0;
					p1[2]=0;
					for(n=startpoly;n<vcount;n++) {
						p1[0]=MIN(p1[0],(float)ver[n][0]);
						p1[1]=MIN(p1[1],(float)ver[n][1]);
						p1[2]=MIN(p1[2],(float)ver[n][2]);
					}

					p2[0]=0;
					p2[1]=0;
					p2[2]=0;
					for(n=startpoly;n<vcount;n++) {
						p2[0]=MAX(p2[0],(float)ver[n][0]);
						p2[1]=MAX(p2[1],(float)ver[n][1]);
						p2[2]=MAX(p2[2],(float)ver[n][2]);
					}

					pcenter.x=(p1[0]+p2[0])/2;
					pcenter.y=(p1[1]+p2[1])/2;
					pcenter.z=(p1[2]+p2[2])/2;

					p1[0]=pcenter.x;
					p1[1]=pcenter.y;
					p1[2]=pcenter.z;
					//DrawRealPoint(p1, 10);
*/
					// Тут я пытался определить направление обхода вершин полигона.
					// Не сработало, так что не используется
					
//					D3DXVECTOR3 vNormal, oldNormal;
//					vNormal = GetTriangeNormal(&D3DXVECTOR3((float)ver[vcount-2][0], (float)ver[vcount-2][1], (float)ver[vcount-2][2]),
//									&D3DXVECTOR3((float)pcenter.x, (float)pcenter.y, (float)pcenter.z),
//									&D3DXVECTOR3((float)ver[startpoly][0], (float)ver[startpoly][1], (float)ver[startpoly][2]));

//					vNormal = GetTriangeNormal(&D3DXVECTOR3((float)ver[startpoly][0], (float)ver[startpoly][1], (float)ver[startpoly][2]),
//									&D3DXVECTOR3((float)pcenter.x, (float)pcenter.y, (float)pcenter.z),
//									&D3DXVECTOR3((float)ver[vcount-2][0], (float)ver[vcount-2][1], (float)ver[vcount-2][2]));
					
					//p2[0]=vNormal.x;
					//p2[1]=vNormal.y;
					//p2[2]=vNormal.z;
					//currentColor=D3DCOLOR_XRGB(255,0,0);
					//DrawRealLine(p1,p2);

//					oldNormal=getNormal(currentNormal);
					//p2[0]=oldNormal.x;
					//p2[1]=oldNormal.y;
					//p2[2]=oldNormal.z;
					//currentColor=D3DCOLOR_XRGB(255,255,0);
					//DrawRealLine(p1,p2);
//
//					if ((oldNormal.x>0 && vNormal.x<0) || (oldNormal.x<0 && vNormal.x>0))
//						zzz=1;
//					if ((oldNormal.y>0 && vNormal.y<0) || (oldNormal.y<0 && vNormal.y>0))
//						zzz=1;
//					if ((oldNormal.z>0 && vNormal.z<0) || (oldNormal.z<0 && vNormal.z>0))
//						zzz=1;

					p1[0]=0;
					p1[2]=0;

					// Обход бага тесселятора. Пусьть все вершины будут положительными. 
					// После выборки из тесселятора уменьшим на ту же величину.

					//for(n=startpoly;n<vcount;n++) {
						//ver[n][0]+=1000;
						//ver[n][1]+=1000;
						//ver[n][2]+=1000;
					//}

					/*
					for(n=startpoly;n<vcount;n++) {
						ver[n][0]+=0.00001f;
						ver[n][1]+=0.00001f;
						ver[n][2]+=0.00001f;
					}
					*/

					gluTessBeginContour (tobj);
					if (!zzz) {
						for(n=startpoly;n<vcount;n++) {
							gluTessVertex (tobj, &ver[n][0], &ver[n][0]);
						}
					} else {
						for(n=vcount-1;n!=startpoly;n--) {
							gluTessVertex (tobj, &ver[n][0], &ver[n][0]);
						}
					}
					gluTessEndContour (tobj);
					startpoly=vcount;
					firstVertex=-1;

					i+=1;					
					continue;
				}
				if (getByte(command, i)==0x0C) {	// Circle
					getVertex(p1, getByte(command, i+1), origin, scale, orient);
					int scaleFactor = abs(10-currentScale);
					radius=(float)((short)getByte(command, i+2)>>scaleFactor)*2/DIVIDER;
					//DrawRealCircle(p1, 100, getNormal(getByte(command, i+3)), 12);
					i+=3;
					continue;
				}
				if (getByte(command, i)==0x00) {	// End poly
					int zzz;
					zzz=0;
					
					
					if (lastVertex!=firstVertex) {
						getVertex(p1, firstVertex, origin, scale, orient);
						ver[vcount][0]=p1[0];
						ver[vcount][1]=p1[1];
						ver[vcount][2]=p1[2];
						vcount++;
					}
					
					//if (lastVertex==firstVertex)
					//	vcount--;

					// Центр полигона по Y. Небольшой патч для пересчета нормалей.
					p1[0]=0;
					p1[1]=0;
					p1[2]=0;
					for(n=startpoly;n<vcount;n++) {
						p1[0]=MIN(p1[0],(float)ver[n][0]);
						p1[1]=MIN(p1[1],(float)ver[n][1]);
						p1[2]=MIN(p1[2],(float)ver[n][2]);
					}

					p2[0]=0;
					p2[1]=0;
					p2[2]=0;
					for(n=startpoly;n<vcount;n++) {
						p2[0]=MAX(p2[0],(float)ver[n][0]);
						p2[1]=MAX(p2[1],(float)ver[n][1]);
						p2[2]=MAX(p2[2],(float)ver[n][2]);
					}

					pcenter.x=(p1[0]+p2[0])/2;
					pcenter.y=(p1[1]+p2[1])/2;
					pcenter.z=(p1[2]+p2[2])/2;

					p1[0]=pcenter.x;
					p1[1]=pcenter.y;
					p1[2]=pcenter.z;
					//DrawRealPoint(p1, 10);

					// Тут я пытался определить направление обхода вершин полигона.
					// Не сработало, так что не используется
					
//					D3DXVECTOR3 vNormal, oldNormal;
//					vNormal = GetTriangeNormal(&D3DXVECTOR3((float)ver[vcount-2][0], (float)ver[vcount-2][1], (float)ver[vcount-2][2]),
//									&D3DXVECTOR3((float)pcenter.x, (float)pcenter.y, (float)pcenter.z),
//									&D3DXVECTOR3((float)ver[startpoly][0], (float)ver[startpoly][1], (float)ver[startpoly][2]));

//					vNormal = GetTriangeNormal(&D3DXVECTOR3((float)ver[startpoly][0], (float)ver[startpoly][1], (float)ver[startpoly][2]),
//									&D3DXVECTOR3((float)pcenter.x, (float)pcenter.y, (float)pcenter.z),
//									&D3DXVECTOR3((float)ver[vcount-2][0], (float)ver[vcount-2][1], (float)ver[vcount-2][2]));
					
					//p2[0]=vNormal.x;
					//p2[1]=vNormal.y;
					//p2[2]=vNormal.z;
					//currentColor=D3DCOLOR_XRGB(255,0,0);
					//DrawRealLine(p1,p2);

//					oldNormal=getNormal(currentNormal);
					//p2[0]=oldNormal.x;
					//p2[1]=oldNormal.y;
					//p2[2]=oldNormal.z;
					//currentColor=D3DCOLOR_XRGB(255,255,0);
					//DrawRealLine(p1,p2);

//					if ((oldNormal.x>0 && vNormal.x<0) || (oldNormal.x<0 && vNormal.x>0))
//						zzz=1;
//					if ((oldNormal.y>0 && vNormal.y<0) || (oldNormal.y<0 && vNormal.y>0))
//						zzz=1;
//					if ((oldNormal.z>0 && vNormal.z<0) || (oldNormal.z<0 && vNormal.z>0))
//						zzz=1;

					p1[0]=0;
					p1[2]=0;

					// Обход бага тесселятора. Пусьть все вершины будут положительными. 
					// После выборки из тесселятора уменьшим на ту же величину.

					//for(n=startpoly;n<vcount;n++) {
						//ver[n][0]+=1000;
						//ver[n][1]+=1000;
						//ver[n][2]+=1000;
					//}

					/*
					for(n=startpoly;n<vcount;n++) {
						ver[n][0]+=0.00001f;
						ver[n][1]+=0.00001f;
						ver[n][2]+=0.00001f;
					}
					*/

					// Тесселируем
					gluTessBeginContour (tobj);
					if (!zzz) {
						for(n=startpoly;n<vcount;n++) {
							gluTessVertex (tobj, &ver[n][0], &ver[n][0]);
						}
					} else {
						for(n=vcount-1;n!=startpoly;n--) {
							gluTessVertex (tobj, &ver[n][0], &ver[n][0]);
						}
					}
					gluTessEndContour (tobj);					
/*
					D3DXVECTOR3 oldNormal;
					oldNormal = getNormal(currentNormal);

					float p1[3], p2[3], p3[3], p0[3];
					for(n=startpoly;n<vcount-1;n++) {
						p1[0]=ver[n][0];
						p1[1]=ver[n][1];
						p1[2]=ver[n][2];
						p2[0]=ver[n+1][0];
						p2[1]=ver[n+1][1];
						p2[2]=ver[n+1][2];
						DrawRealLine(p1,p2);
					}

					p0[0]=0;
					p0[1]=0;
					p0[2]=0;

					p3[0]=0;
					p3[1]=0;
					p3[2]=0;
					for(n=startpoly;n<vcount;n++) {
						p3[0]+=ver[n][0];
						p3[1]+=ver[n][1];
						p3[2]+=ver[n][2];
					}
					p3[0]/=vcount-1-startpoly;
					p3[1]/=vcount-1-startpoly;
					p3[2]/=vcount-1-startpoly;
					//DrawRealLine(p0,p3);

					for(n=startpoly;n<vcount-1;n++) {
						p1[0]=(ver[n][0]-p3[0])/2+(p3[0]);
						p1[1]=(ver[n][1]-p3[1])/2+(p3[1]);
						p1[2]=(ver[n][2]-p3[2])/2+(p3[2]);
						p2[0]=(ver[n+1][0]-p3[0])/2+(p3[0]);
						p2[1]=(ver[n+1][1]-p3[1])/2+(p3[1]);
						p2[2]=(ver[n+1][2]-p3[2])/2+(p3[2]);
						DrawRealLine(p1,p2);
					}
*/

					i+=2;
					command +=i/2;
					break;
				}
			}
			gluTessEndPolygon (tobj);

			command++;
			continue;
		}
		if (*command==0x0007) {		// Mirror triangle
			setMaterial(*(command+1));
			getVertex(p1, getByte(command, 2), origin, scale, orient);
			getVertex(p2, getByte(command, 3), origin, scale, orient);
			getVertex(p3, getByte(command, 4), origin, scale, orient);
			currentNormal=getByte(command, 5);
			//if (checkOrientation(p1,p2,p3)==true) {
				if (!skipCurrentModel)
					DrawRealTriangle(p1, p2, p3, 2, getByte(command, 5));
			//} else {
			//	if (!skipCurrentModel)
			//		DrawRealTriangle(p3, p2, p1, 2, getByte(command, 5));
			//}
			getVertex(p1, getByte(command, 2)^1, origin, scale, orient);
			getVertex(p2, getByte(command, 3)^1, origin, scale, orient);
			getVertex(p3, getByte(command, 4)^1, origin, scale, orient);
			currentNormal=getByte(command, 5)^1;
			//if (checkOrientation(p1,p2,p3)==true) {
				if (!skipCurrentModel)
					DrawRealTriangle(p3, p2, p1, 2, getByte(command, 5)^1);
			//} else {
			//	if (!skipCurrentModel)
			//		DrawRealTriangle(p3, p2, p1, 2, getByte(command, 5)^1);
			//}
			command+=4;
			continue;
		}
		if (*command==0x0008) {		// Mirror square
			setMaterial(*(command+1));
			if (currentModel==156) {
				command+=5;
				continue;
			}
			getVertex(p1, getByte(command, 2), origin, scale, orient);
			getVertex(p2, getByte(command, 5), origin, scale, orient);
			getVertex(p3, getByte(command, 3), origin, scale, orient);
			getVertex(p4, getByte(command, 4), origin, scale, orient);
			currentNormal=getByte(command, 7);
			//if (checkOrientation(p2,p3,p4)==true) {
				if (!skipCurrentModel)
					DrawRealSquare2(p1, p2, p3, p4, getByte(command, 7));
			//} else {
			//	if (!skipCurrentModel)
			//		DrawRealSquare2(p4, p3, p2, p1, getByte(command, 7));
			//}
			getVertex(p1, getByte(command, 2)^1, origin, scale, orient);
			getVertex(p2, getByte(command, 5)^1, origin, scale, orient);
			getVertex(p3, getByte(command, 3)^1, origin, scale, orient);
			getVertex(p4, getByte(command, 4)^1, origin, scale, orient);
			currentNormal=getByte(command, 7)^1;
			//if (checkOrientation(p2,p3,p4)==true) {
				if (!skipCurrentModel)
					DrawRealSquare2(p1, p2, p3, p4, getByte(command, 7)^1);
			//} else {
			//	if (!skipCurrentModel)
			//		DrawRealSquare2(p4, p3, p2, p1, getByte(command, 7)^1);
			//}
			command+=5;
			continue;
		}
		if (*command==0x0009) {		// Thruster (pine)

			// Не совсем корректно работает. Плюс надо добавить круг билбордом,
			// как в оригинале. Он нужен в случаях, когда смотришь на выхлопы прямо сзади.
			D3DXVECTOR4 out;

			setMaterial(*(command+1));
			getVertex(p2, getByte(command, 2), origin, scale, orient);
			getVertex(p1, getByte(command, 3), origin, scale, orient);

			out.x=p2[0]-p1[0];
			out.y=p2[1]-p1[1];
			out.z=p2[2]-p1[2];
			float dist = sqrtf(out.x*out.x+out.y*out.y+out.z*out.z);

			for(int ii=0;ii<10;ii++) {
				if (modelList[mainModelNum].light[ii].enable) continue;

				D3DXVec3Transform(&out,&D3DXVECTOR3(p1[0],p1[1],p1[2]), &mainRotMatrix);

				out.x+=posMatrix[12];
				out.y+=posMatrix[13];
				out.z+=posMatrix[14];

				modelList[mainModelNum].light[ii].enable=true;
				modelList[mainModelNum].light[ii].pos.x=out.x;
				modelList[mainModelNum].light[ii].pos.y=out.y;
				modelList[mainModelNum].light[ii].pos.z=out.z;
				out.x=p2[0]-p1[0];
				out.y=p2[1]-p1[1];
				out.z=p2[2]-p1[2];
				modelList[mainModelNum].light[ii].dist=dist;

				modelList[mainModelNum].light[ii].r=1.0f/255*currentR/3.0f*modelList[mainModelNum].light[ii].dist;
				modelList[mainModelNum].light[ii].g=1.0f/255*currentG/3.0f*modelList[mainModelNum].light[ii].dist;
				modelList[mainModelNum].light[ii].b=1.0f/255*currentB/3.0f*modelList[mainModelNum].light[ii].dist;

				break;
			}
			
			float width;
			/*
			if (getByte(command, 5)==0) {
				width=(float)(getByte(command, 4))/4;				
			} else {
				width=(float)(getByte(command, 5)<<currentScale)/DIVIDER/10;
			}
			*/

			width=dist/5;
			

			p4[0]=p1[0];
			p4[1]=p1[1];
			p4[2]=p1[2];

			doLight=false;
			transparent=true;
			useLocalColor=true;

			if (currentModel==335) {
				p2[0]*=1000;
				p2[1]*=1000;
				p2[2]*=1000;
				if (posMatrix[14]<1 && posMatrix[14]>-1)
					p1[1]-=0.2;

				currentTex=705;
				doLight=false;
				drawBillLine(p1, p2, width/2);

			}

			currentTex=101;
			drawBillLine(p1, p2, width);

			D3DXVECTOR4 pos1, pos2;
			D3DXVec3Transform(&pos1,&D3DXVECTOR3(p1[0],p1[1],p1[2]), &mainRotMatrix);
			D3DXVec3Transform(&pos2,&D3DXVECTOR3(p2[0],p2[1],p2[2]), &mainRotMatrix);
		
			/*
			out.x=pos2.x-pos1.x;
			out.y=pos2.y-pos1.y;
			out.z=0;
			float dist2d = sqrtf(out.x*out.x+out.y*out.y);
			*/

			width=dist/5;
			//if (dist2d<width) {
				//useLocalColor=true;
				currentTex=291;
				//currentColor.r=currentR;
				//currentColor.g=currentG;
				//currentColor.b=currentB;
				float wp=(float)(rand()%10000)/95000;
				DrawBillboard(p4, width*2*0.9f-wp);
			//}

			command+=4;
			continue;
		}
		if (*command==0x000A) {		// Text
			rotateNext=false;
			command+=5;
			continue;
		}
		if (*command==0x0011) {		// Cone
			float radius2;
			getVertex(p1, getByte(command, 2), origin, scale, orient);
			getVertex(p2, getByte(command, 3), origin, scale, orient);
			int scaleFactor = *(char*)((int)gptr+0x148);//(8-currentScale);
			int r1 = (unsigned char)getByte(command, 5);
			int r2 = (unsigned char)getByte(command, 7);
			radius = (float)(r1 << (scaleFactor/2))/DIVIDER;
			radius2 = (float)(r2 << (scaleFactor/2))/DIVIDER;

			if (r1==2 && r2!=0) {
				radius=(float)((mod->Radius*2)<<(scaleFactor/2))/DIVIDER;
				radius2=radius/2;
			}
			if (r2==0)
				radius2=radius;
			if (r1==138) {
				radius/=1.5;
			}
			if (r2==138) {
				radius2/=1.5;
			}
			if (r1>=140 && r1!=154) {
				radius/=5;
			}
			if (r2>=140 && r2!=154) {
				radius2/=5;
			}
			if (r1==5) {
				radius*=5;
			}
			if (r2==5) {
				radius2*=5;
			}
			if (r1==6) {
				radius*=6;
			}
			if (r2==6) {
				radius2*=6;
			}
			setMaterial(*(command+1));
			if (!skipCurrentModel)
				DrawRealCylinder(p1, p2, radius, radius2, 0, 1, 0);

			setMaterial(*(command+5));
			if (!skipCurrentModel)
				DrawRealCylinder(p1, p2, radius, radius2, 1, 0, 0);
		
			setMaterial(*(command+6));
			if (!skipCurrentModel)
				DrawRealCylinder(p1, p2, radius, radius2, 0, 0, 1);

			//DrawRealLine(p1, p2);
			command+=7;
			continue;
		}
		if (*command==0x0016) {		// Curve
			int spl=16;
			float ctrlpoints[4][3];

			setMaterial(*(command+1));
			getVertex(ctrlpoints[0], getByte(command, 2), origin, scale, orient);
			getVertex(ctrlpoints[2], getByte(command, 3), origin, scale, orient);
			getVertex(ctrlpoints[1], getByte(command, 4), origin, scale, orient);
			getVertex(ctrlpoints[3], getByte(command, 5), origin, scale, orient);

			//DrawRealPoint(ctrlpoints[0], 10);
			//DrawRealPoint(ctrlpoints[1], 10);
			//DrawRealPoint(ctrlpoints[2], 10);
			//DrawRealPoint(ctrlpoints[3], 10);
			//DrawRealLine(ctrlpoints[0], ctrlpoints[1]);
			//DrawRealLine(ctrlpoints[3], ctrlpoints[1]);
			//DrawRealLine(ctrlpoints[1], ctrlpoints[0]);

			if (!skipCurrentModel)
			for (m=0; m<spl && currentModel!=354; m++) {
				eval_bezier (p1, (float)m/(float)spl, ctrlpoints);
				eval_bezier (p2, (float)(m+1)/(float)spl, ctrlpoints);
				DrawRealLine(p1, p2);
			}
			
			command+=5;
			continue;
		}
		if (*command==0x0018) {		// Ball array
			setMaterial(*(command+1));
			//float radius=(float)(getVar(*(command+2)))/DIVIDER/50;
			/*
			int rad=FUNC_001474_getRadius(gptr,*(command+2));
			float radius=(float)rad/DIVIDER;
*/
			if (num==154) {
				localvar[1] -= localvar[2];
			}

			if (*(command+2) >> 8 == 0)
				radius=(float)getVar(*(command+2))/DIVIDER;
			else {
				int scaleFactor = abs(10-currentScale);
				radius=(float)((short)*(command+2)>>scaleFactor)*2/DIVIDER;
			}

			command+=3;
			useLocalColor=true;
			transparent=true;

			if (num==316) {
				//radius /= 400;
				currentTex=707;
			} else if (num==109) {
				if (radius<0.03)
					radius=0.03;
				currentTex=704;
			} else if (num==154) {
				currentTex=721;
				D3DXMatrixRotationZ(&tmpMatrix, (float)*DATA_008804/60000.0f * (globalvar[0] == 0 ? 1 : -1));
				D3DXMatrixMultiply(&mainRotMatrix, &mainRotMatrix, &tmpMatrix);
			} else {	
				currentTex=704;
			}

			if (!skipCurrentModel)
			while(1) {
				if ((char)(*command>>8)==0x7f) {
					break;
				}
				getVertex(p1, *command>>8, origin, scale, orient);
				//if (num!=154)
					DrawBillboard(p1, radius);
					//DrawRealPoint(p1,radius);

				if ((char)(*command&0xFF)==0x7f) {
					//DrawRealSphere(p1, radius, currentTex, 40);
					break;
				}
				getVertex(p1, *command&0xFF, origin, scale, orient);
				//if (num!=154)
					DrawBillboard(p1, radius);
					//DrawRealPoint(p1,radius);
				
				command++;
			}
			command++;
			continue;
		}
		if (*command==0x001A) {		// Set Color
			if (getByte(command, 3) == 0 && getByte(command, 2)!=0) {
				//int var=FUNC_001473_getVar(gptr, command+1)&0x7;
				int var=getVar(getByte(command, 2))&0x7;
				if (var==0) {
					short rgb=(*(command+1)&0x0FFF);
					short r=(rgb&0x0F00)>>8;
					short g=(rgb&0x00F0)>>4;
					short b=(rgb&0x000F);
					//currentColor=D3DCOLOR_XRGB(r*16,g*16,b*16);
					localColor[0]=(char)r*17;
					localColor[1]=(char)g*17;
					localColor[2]=(char)b*17;
				} else {
					short rgb=(*(command+2+var)&0x0FFF);
					short r=(rgb&0x0F00)>>8;
					short g=(rgb&0x00F0)>>4;
					short b=(rgb&0x000F);
					//currentColor=D3DCOLOR_XRGB(r*16,g*16,b*16);
					localColor[0]=(char)r*17;
					localColor[1]=(char)g*17;
					localColor[2]=(char)b*17;
				}
				command+=10;
			} else {
				short rgb=(*(command+1)&0x0FFF);
				short r=(rgb&0x0F00)>>8;
				short g=(rgb&0x00F0)>>4;
				short b=(rgb&0x000F);
				localColor[0]=(char)r;
				localColor[1]=(char)g;
				localColor[2]=(char)b;
				command+=3;
			}
			continue;
		}
		if (*command==0x001F) {		// Planet?
			// Тут еще работать и работать
			if (currentModel>=138 && currentModel<=148) {
				// Звезда
				setMaterial(*(command+1));
				modelList[modelNum].ambientB=currentColor.b;
				modelList[modelNum].ambientG=currentColor.g;
				modelList[modelNum].ambientR=currentColor.r;

				getTransVertex(p1, 6);
				dist=sqrtf(posMatrix[12]*posMatrix[12]+posMatrix[13]*posMatrix[13]+posMatrix[14]*posMatrix[14]);
				radius=(float)(*(mod->Vertices_ptr+2)<<currentScale)/DIVIDER;
				//DrawRealSphere(p1, radius, 0, 60);

				doLight=false;
				useLocalColor=false;
				currentTex=701;
				transparent=false;
				DrawBillboard(p1, radius*8);


			} else {
				if (currentModel==445 || 
					currentModel==134) {
					int a=0;
				}
				int a=timeGetTime();
				// Планета
				setMaterial(*(command+1));
				C_FUNC_001874_DrawPlanet(gptr,(char *)(command+1));
				
				//useLocalColor=true;
				getTransVertex(p1, 6);
				/*
				// Большая Проблема с Радиусом (БПР)
				// Радиус беру по одной из вершин планеты
				dist=sqrtf(posMatrix[12]*posMatrix[12]+posMatrix[13]*posMatrix[13]+posMatrix[14]*posMatrix[14]);

				doLight=true;
				if (dist>radius*2) {
					modelList[modelNum].zwrite=true;
				}
				*/
				//currentColor=D3DCOLOR_XRGB(255,255,255);
				//DrawRealSphere(p1, radius, 0, 60);
				
				currentTex=704;
				useLocalColor=false;
				transparent=false;
				float radius=(float)(*(mod->Vertices_ptr+2)<<currentScale)/DIVIDER;

				p1[0]=posMatrix[12];
				p1[1]=posMatrix[13];
				p1[2]=posMatrix[14];

				D3DXVECTOR3 v;

				v.x = p1[0];	
				v.y = p1[1];
				v.z = p1[2];
				
				//D3DXVec3TransformCoord(&v,&v,&mainRotMatrixO);

				p1[0]=v.x;
				p1[1]=v.y;
				p1[2]=v.z;

				//DrawBillboard(p1, radius*2.5f);
				//modelList[currentModel].zclear=true;
				
			}
			command+=7;
			while(1) {
				if (*(command)==0x0000) {
					command++;
					break;
				}
				command+=8;
			}
			continue;
		}

		if (*command==0x0000 || *command==0x0720) {
			break;
		}
		if ((short)(*command & 0x001D)==0x001D) {	// Math
			if (callAsm)
				FUNC_001758_doMath(gptr, command+1, *command);
			else
				doMath(command);
			//command=FUNC_GraphNull(gptr, command+1, *command);
			command+=2;
			continue;
		}
		if ((short)(*command & 0x001E)==0x001E) {	// unknown!
			command+=1;
			continue;
		}
		if ((short)(*command & 0x001C)==0x001C) {	// Rotate
			// фикс ми
			
			unsigned char orient=*command>>5;
			if (*(command+1)==0)
				setMatrix(orient);
			//rotateNext=true;
			
			if (*command==0xC01C)
				command++;
			command+=2;
			continue;
		}
		if ((short)(*command & 0x001B)==0x001B) {	// Scaled sub-object
			// Так как я теперь перехватываю не только целые объекты, а еще и
			// субобъекты, эта команда не нужна.

			if ((unsigned char)(getByte(command, 0)&0x80) == 0x80) {
				command+=6;
			} else {
				command+=4;
			}
			continue;
		}
		if ((short)(*command & 0x0019)==0x0019) {	// Indentity matrix
			billboardMatrix=true;
			//D3DXMATRIX one;
			//D3DXMatrixInverse(&one,NULL, &mainRotMatrix);
			//D3DXMatrixMultiply(&currentMatrix,&currentMatrix,&one);
			command+=1;
			continue;
		}
		if ((short)(*command & 0x0015)==0x0015) {	// Process Vertex?
			command+=1;
			continue;
		}
		if ((short)(*command & 0x0014)==0x0014) { // Skip If Bit Set
			/*
			unsigned char bit=getByte(command, 0);
			unsigned short var=getVar(getByte(command, 1));
			unsigned short tst=var;
			if (bit==0) {
				tst=var|-1;
			} else {
				tst=var&(short)(1 << (bit-1));
			}
			//if (tst!=0) {
			//	var=0;
			//}
			//var&=1;
			if (var==0) {
				if (*command>>5==0)
					return;
				command+=*command>>5;
			}
			*/
			/*
			if ((bit==0 && var!=0) || (bit>0 && (var&(short)(1 << (bit-1)))==0)) {
				if (*command>>5==0)
					return;
				command+=*command>>5;
			}
			*/
			command=FUNC_001757_SkipIfBitSet(gptr, command+1, *command);
			//command+=2;
			continue;
		}
		if ((short)(*command & 0x0013)==0x0013) { // Skip If Bit Clear
			/*
			unsigned char bit=getByte(command, 0);
			unsigned short var=getVar(getByte(command, 1));
			unsigned short tst=var;
			if (bit!=0) {
				tst=var&(short)(1 << (bit-1));				
			}
			if (tst==0) {
				var=0;
			}
			var&=1;
			if (var==0) {
				if (*command>>5==0)
					return;
				command+=*command>>5;
			}
			*/
			/*
			if ((bit==0 && var==0) || (bit>0 && (var&(short)(1 << (bit-1)))==0)) {
				if (*command>>5==0)
					return;
				command+=*command>>5;
			}
			*/
			command=FUNC_001756_SkipIfBitClear(gptr, command+1, *command);
			//command+=2;
			continue;
		}
		if ((short)(*command & 0x0012)==0x0012) {
			command+=2;
			continue;
		}
		if ((short)(*command & 0x000F)==0x000F) {
			command+=1;
			continue;
		}
		if ((short)(*command & 0x000E)==0x000E) {	// Sub-object
			// Так как я теперь перехватываю не только целые объекты, а еще и
			// субобъекты, эта команда не нужна.

			if ((unsigned char)(getByte(command, 0)&0x80) == 0x80) {
				command+=4;
			} else {
				command+=2;
			}
			continue;
		}
		if ((short)(*command & 0x000D)==0x000D) {	// Math
			if (callAsm)
				FUNC_001758_doMath(gptr, command+1, *command);
			else
				doMath(command);
			command+=2;
			continue;
		}
		if ((short)(*command & 0x000C)==0x000C) {	// Skip If Visible/Skip If Closer Than
			//if ((*(command+1)&0x8000)==0) {
				/*
				D3DXMATRIX tmpMatrix;
				D3DXMatrixIdentity(&tmpMatrix);
				tmpMatrix[12]=(float)origin[0];
				tmpMatrix[13]=(float)origin[1];
				tmpMatrix[14]=(float)origin[2];
				D3DXMatrixMultiply(&tmpMatrix, &mainRotMatrix, &tmpMatrix);
				D3DXMatrixMultiply(&tmpMatrix, &posMatrix, &tmpMatrix);
				double dist=sqrt(tmpMatrix[12]*tmpMatrix[12]+tmpMatrix[13]*tmpMatrix[13]+tmpMatrix[14]*tmpMatrix[14]);
				//dist /=152;
				if (dist/10<(double)(*(command+1)&0x7FFF))
				*/
					//command+=*command>>5;
					
			//} else {
				//command+=*command>>5;
			//}
			//if (exportb[currentModel]==false) {
				//if ((*(command+1) & 0x8000) == 0)
					command=C_FUNC_001755_SkipIfVisible(gptr, command+1, *command);
				//else {
				//	command+=*command>>5;
				//	command+=2;
				//}
			//} else {
			//	command+=*command>>5;
			//	command+=2;
			//}
			//command+=2;
			continue;
		}
		if ((short)(*command & 0x000B)==0x000B) {	// Skip If Not Visible/Skip If Further Than
			//if ((*(command+1)&0x8000)==0) {
				/*
				D3DXMATRIX tmpMatrix;
				D3DXMatrixIdentity(&tmpMatrix);
				tmpMatrix[12]=(float)origin[0];
				tmpMatrix[13]=(float)origin[1];
				tmpMatrix[14]=(float)origin[2];
				D3DXMatrixMultiply(&tmpMatrix, &mainRotMatrix, &tmpMatrix);
				D3DXMatrixMultiply(&tmpMatrix, &posMatrix, &tmpMatrix);
				double dist=sqrt(tmpMatrix[12]*tmpMatrix[12]+tmpMatrix[13]*tmpMatrix[13]+tmpMatrix[14]*tmpMatrix[14]);
				if (dist/10>(double)(*(command+1)&0x7FFF))
					command+=*command>>5;
				*/
				//if (currentModel<81 || currentModel>102) {
				//	//command+=*command>>5;
				//}
				
			//} else {
				//command+=*command>>5;
			//}
			//if (exportb[currentModel]==false) {
				//if ((*(command+1) & 0x8000) == 0)
				//	command=FUNC_001752_SkipIfNotVisible(gptr, command+1, *command);
				//else
				//	command+=2;
				command=C_FUNC_001752_SkipIfNotVisible(gptr, command+1, *command);

			//} else {
				//command+=*command>>5;
			//	command+=2;
			//}
			//command+=2;
			continue;
		}
		if ((short)(*command & 0x0006)==0x0006) {	// TRANSFORMATION?
			// Полный скип
			switch (*command >> 13)
			{
				case 0: D3DXMatrixIdentity(&currentMatrix);
						break;
				case 1:
				case 3:
					if ((unsigned char)getByte(command,0)==0xff) {
						while(1) {
							for (i=0;(unsigned char)getByte(command,i)==0xff;i+=2) {}
							command+=i/2;
							break;
						}
					}
					command++;
					break;
				case 2: break;
				case 7: 
					if (*command >> 5 == 2045)
						command +=1;
					else if (*command >> 5 == 2046)
						command +=0;
					else if (*command >> 5 == 2047)
						command +=0;
					else {
						command +=1;
						break;
					}
				default: 
					break;
			}
			command++;
			continue;
		}
		return;
	}
}

D3DCOLOR getD3DColor(DWORD color) {
	unsigned char argb[4];

	*(DWORD *)argb=color;
	//return D3DCOLOR_XRGB(255,255,255);
	return D3DCOLOR_XRGB(argb[2],argb[1],argb[0]);
}

void DrawClipSprite(int index, int x, int y, int z, int h, int w, float NPx1, float NPx2, float NPy1, float NPy2) {
	VertexXYZNDT1* Vertices = (VertexXYZNDT1*)renderSystem->LockVertexBuffer(vertexBuffer,vertexNum,4*vertexBuffer.vertexSize);

	modelList[modelNum].index=0;
	modelList[modelNum].doMatrix=2;
	modelList[modelNum].vertStart=vertexNum;
	modelList[modelNum].zwrite=false;
	modelList[modelNum].backsprite=false;

	Vertices->pos.x = (float)x;
	Vertices->pos.y = (float)y;
	Vertices->pos.z = (float)z;
	Vertices->u1() = NPx1;
	Vertices->v1() = NPy1;
	vertexType[vertexNum].type = GL_TRIANGLE_STRIP;
	vertexType[vertexNum].amount = 4;
	vertexType[vertexNum].textNum = index;
	Vertices->difuse.RGBA() = (DWORD)pWinPal32[15];
	vertexType[vertexNum].doLight=false;

	Vertices->n.x = 1.0f;
	Vertices->n.y = 1.0f;
    Vertices->n.z = 1.0f;

	Vertices++;
	vertexNum++;


	Vertices->pos.x = (float)x;
	Vertices->pos.y = (float)h;
	Vertices->pos.z = (float)z;
	Vertices->u1() = NPx1;
	Vertices->v1() = NPy2;
	vertexType[vertexNum].type = 0;
	vertexType[vertexNum].textNum = index;
	Vertices->difuse.RGBA() = (DWORD)pWinPal32[15];
	vertexType[vertexNum].doLight=false;

	Vertices->n.x = 1.0f;
	Vertices->n.y = 1.0f;
    Vertices->n.z = 1.0f;

	Vertices++;
	vertexNum++;

	Vertices->pos.x = (float)w;
	Vertices->pos.y = (float)y;
	Vertices->pos.z = (float)z;
	Vertices->u1() = NPx2;
	Vertices->v1() = NPy1;
	vertexType[vertexNum].type = 0;
	vertexType[vertexNum].textNum = index;
	Vertices->difuse.RGBA() = (DWORD)pWinPal32[15];
	vertexType[vertexNum].doLight=false;

	Vertices->n.x = 1.0f;
	Vertices->n.y = 1.0f;
    Vertices->n.z = 1.0f;

	Vertices++;
	vertexNum++;

	Vertices->pos.x = (float)w;
	Vertices->pos.y = (float)h;
	Vertices->pos.z = (float)z;
	Vertices->u1() = NPx2;
	Vertices->v1() = NPy2;
	vertexType[vertexNum].type = 0;
	vertexType[vertexNum].textNum = index;
	Vertices->difuse.RGBA() = (DWORD)pWinPal32[15];
	vertexType[vertexNum].doLight=false;

	Vertices->n.x = 1.0f;
	Vertices->n.y = 1.0f;
    Vertices->n.z = 1.0f;

	Vertices++;
	vertexNum++;
	modelList[modelNum].vertEnd=vertexNum;
	modelNum++;
	
	renderSystem->UnlockVertexBuffer(vertexBuffer);
}

extern int panelnum;

// Осталась используемой для рисования спрайтов
extern "C" void C_BlitClipInternal (int index, int xpos, int ypos, char *ptr, int xmin, int ymin, int xmax, int ymax)
{
		short x, y, width, height, w, h;
		float z;
		float NPx1, NPx2;
		float NPy1, NPy2;

		int aspectfactor=(curheight-curwidth/1.6f);
		vertexType[vertexNum].transparent=true;

		if (vertexNum>MAXVERT-100)
			return;

		if (index==162 || index==177)
			panelnum=0;
		if (index==111 || index==188)
			panelnum=1;
		if (index==197 || index==208)
			panelnum=2;
		if (index==140 || index==151)
			panelnum=3;

		switch(index) {
			// F1
			case 167: panellight[1]=true; break;

			case 178: panellight[5]=true; break;
			case 177: panellight[6]=true; break;
			case 176: panellight[7]=true; break;
			case 175: panellight[8]=true; break;
			case 174: panellight[9]=true; break;
			case 173: panellight[10]=true; break;
			case 172: panellight[11]=true; break;
			case 170: panellight[13]=true; break;
			case 169: panellight[14]=true; break;

			// F2
			case 166: panellight[2]=true; break;

			case 189: panellight[5]=true; break;
			case 188: panellight[6]=true; break;
			case 187: panellight[7]=true; break;
			case 186: panellight[8]=true; break;
			case 185: panellight[9]=true; break;
			case 184: panellight[10]=true; break;
			case 183: panellight[11]=true; break;
			case 182: panellight[12]=true; break;

			// F3
			case 165: panellight[3]=true; break;

			case 211: panellight[5]=true; break;
			case 210: panellight[6]=true; break;
			case 209: panellight[7]=true; break;
			case 208: panellight[8]=true; break;
			case 207: panellight[9]=true; break;
			case 206: panellight[10]=true; break;
			case 205: panellight[11]=true; break;
			case 204: panellight[12]=true; break;
			case 203: panellight[13]=true; break;

			// F4
			case 164: panellight[4]=true; break;

			case 152: panellight[5]=true; break;
			case 151: panellight[6]=true; break;
			case 150: panellight[7]=true; break;
			case 149: panellight[8]=true; break;
			case 148: panellight[9]=true; break;

			default: break;
		}

		if (index >=102 && index <=211) // Основные кнопки панели
			return;

		if (index ==268)
			return;

		NPx1 = N_0;
		NPx2 = N_1;
		NPy1 = N_0;
		NPy2 = N_1;

		x = xpos;
		y = ypos;

		xmax = xmax == -1 ? 320 : xmax;
		ymax = ymax == -1 ? 200 : ymax;

		if (index==273) {
			x-=8;
			y-=1;
			if (xmin>0) xmin-=8;
			if (xmax>0) xmax-=8;
			if (ymin>0) ymin-=1;
			if (ymax>0) ymax-=1;
		}

		if (index==274) {
			x-=3;
			//y-=1;
			if (xmin>0) xmin-=3;
			if (xmax>0) xmax-=3;
			//if (ymin>0) ymin-=1;
			//if (ymax>0) ymax-=1;
		}

		if (index>0) {
			width = *((unsigned short *)ptr+1);
			height = *(ptr+1);
		} else {
			width = 13;
			height = 8;
		}
		width+=x;
		height+=y;

		z = 0;
		
		if (index == 0) { // pointer
			z = 0;
		}
		if (index==96) { // cabin
			vertexType[vertexNum].transparent=false;
			modelList[modelNum].backsprite=true;
			//width=320;
			height=158;
			z=1;
			incabin++;
		}
		
		if (index==66) { // "First Encounters"
			x=0;
			width=320;
		}

		if (index==47 || index==48) {
			z = 1;
		}
		if (index==97 || index==98) {
			z = 0;
			vertexType[vertexNum].transparent=true;
		}

		if (y>=158) {
			z=0;
		}
		if (index>=254 && index<=261) {
			z = 1;
			vertexType[vertexNum].transparent=false;
		}

	if (index==96) {
		xmin-=3;
		ymin-=2;
		xmax+=3;
		ymax+=2;
		x-=3;
		y-=2;
		width+=3;
		height+=2;
	}

	if (xmin < x) {xmin=x;}
	if (ymin < y) {ymin=y;}
	if (xmax > width) {xmax=width;}
	if (ymax > height) {ymax=height;}
	NPx1 = 1.0f/(width-x)*(xmin-x);
	NPx2 = 1.0f/(width-x)*(xmax-x);
	NPy1 = 1.0f/(height-y)*(ymin-y);
	NPy2 = 1.0f/(height-y)*(ymax-y);
	x =  xmin < x ? x : xmin;
	y =  ymin < y ? y : ymin;
	width = xmax > width ? width : xmax;
	height = ymax > height ? height : ymax;

	if (index==96) {
		D3DRECT rect;

		if (aspectfix) {
			rect.x1=(int)((float)curwidth/320*x);
			rect.y1=(int)((float)(curheight-aspectfactor)/200*y)+aspectfactor/2+2;
			rect.x2=(int)((float)curwidth/320*width);
			rect.y2=(int)((float)(curheight-aspectfactor)/200*height)+aspectfactor/2;
		} else {
			rect.x1=(int)((float)curwidth/320*x);
			rect.y1=(int)((float)curheight/200*y);
			rect.x2=(int)((float)curwidth/320*width);
			rect.y2=(int)((float)curheight/200*height);
		}

		if (renderSystem->GetDevice()!=NULL)
			renderSystem->GetDevice()->Clear(1,(D3DRECT *)&rect,D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,D3DCOLOR_XRGB(0,0,0),1.0f,0);	
	}

	if (false) {
		spriteList[maxsprite].pos.x=x;
		spriteList[maxsprite].pos.y=y;
	//	spriteList[maxsprite].pos.z=z;
		spriteList[maxsprite].rect.left=x/320*curwidth;
		spriteList[maxsprite].rect.top=y/200*curheight;
		spriteList[maxsprite].rect.right=width/320*curwidth;
		spriteList[maxsprite].rect.bottom=height/200*curheight;
		spriteList[maxsprite].tex=index;
		maxsprite++;
	} else {
		x =  x*curwidth/320 - curwidth / 2;
		y =  ~(y*curheight/200 - curheight / 2);
		w =  width*curwidth/320 - curwidth / 2;
		h =  ~(height*curheight/200 - curheight / 2);

		//sprites.push_back(Sprite(x,y,w,h,index));
	}


	DrawClipSprite(index, x, y, z, h, w, NPx1, NPx2, NPy1, NPy2);
}

// Уже не используется
extern "C" void C_DrawQuadVClip(u16 *p1, u16 *p2, u16 *p3, u16 *p4, int color) {
		unsigned short point1[2], point2[2], point3[2], point4[2];
		point1[0]=*p1;
		point1[1]=*(p1+1);
		point2[0]=*p2;
		point2[1]=*(p2+1);
		point3[0]=*p3;
		point3[1]=*(p3+1);
		point4[0]=*p4;
		point4[1]=*(p4+1);

		C_DrawTriangle(point1, point2, point3, color);
		C_DrawTriangle(point1, point3, point4, color);
}

// Уже не используется
extern "C" void C_DrawQuad(u16 *p1, u16 *p2, u16 *p3, u16 *p4, int color)
{
		unsigned short point1[2], point2[2], point3[2], point4[2];
		point1[0]=*p1;
		point1[1]=*(p1+1);
		point2[0]=*p2;
		point2[1]=*(p2+1);
		point3[0]=*p3;
		point3[1]=*(p3+1);
		point4[0]=*p4;
		point4[1]=*(p4+1);

		C_DrawTriangle(point1, point2, point3, color);
		C_DrawTriangle(point1, point3, point4, color);
}

// Уже не используется
extern "C" void C_DrawTexTriangleDet(u16 *p1, u16 *p2, u16 *p3, 
									  u16 *t1, u16 *t2, u16 *t3, int text)
{		
		unsigned short point1[2], point2[2], point3[2];
		point1[0]=*p1*2;
		point1[1]=*(p1+1)*2;
		point3[0]=*p2*2;
		point3[1]=*(p2+1)*2;
		point2[0]=*p3*2;
		point2[1]=*(p3+1)*2;

		DrawTriangle(point1, point2, point3, t1, t2, t3, 15, text, 1, false);
}

// Уже не используется
extern "C" void C_DrawTexQuadDet(u16 *p1, u16 *p2, u16 *p3, u16 *p4, 
									  u16 *t1, u16 *t2, u16 *t3, u16 *t4, int text)
{		return;
		unsigned short point1[2], point2[2], point3[2], point4[2];
		point1[0]=*p1*2;
		point1[1]=*(p1+1)*2;
		point2[0]=*p2*2;
		point2[1]=*(p2+1)*2;
		point3[0]=*p3*2;
		point3[1]=*(p3+1)*2;
		point4[0]=*p4*2;
		point4[1]=*(p4+1)*2;

		DrawTriangle(point1, point2, point3, t1, t2, t3, 15, text, 1, false);
		DrawTriangle(point1, point3, point4, t1, t3, t4, 15, text, 1, false);
}

// Уже не используется
extern "C" void C_DrawTriangle(u16 *p1, u16 *p2, u16 *p3, int color)
{
		return;
		unsigned short point1[2], point2[2], point3[2];
		unsigned short t1[2], t2[2], t3[2];
		point1[0]=*p1*2;
		point1[1]=*(p1+1)*2;
		point2[0]=*p2*2;
		point2[1]=*(p2+1)*2;
		point3[0]=*p3*2;
		point3[1]=*(p3+1)*2;
		t1[0]=0;
		t1[1]=0;
		t2[0]=1;
		t2[1]=0;
		t3[0]=1;
		t3[1]=1;
		DrawTriangle(point1, point2, point3, t1, t2, t3, color, 0, 1, false);
}

extern "C" void C_DrawTexTriangleInternal (long* texels, SHORT p1[2], SHORT p2[2], SHORT p3[2], SHORT t1[2], SHORT t2[2], SHORT t3[2], long text)
{
	int v1_x=0, v1_y=0, v2_x=0, v2_y=0, v3_x=0, v3_y=0;
	int texture, color;
	int z=1;

	int sizey	= *( ((UCHAR*)texels) - 7 );
	int sizex	= *( (USHORT*) (((UCHAR*)texels)-6) );
	if(sizey==0) {
		sizey = 512;
	} 
	
	t1[0] = max(1,min(sizex-1,t1[0]));
	t2[0] = max(1,min(sizex-1,t2[0]));
	t3[0] = max(1,min(sizex-1,t3[0]));
	t1[1] = max(1,min(sizey-1,t1[1]));
	t2[1] = max(1,min(sizey-1,t2[1]));
	t3[1] = max(1,min(sizey-1,t3[1]));

	if (vertexNum>MAXVERT-100)
		return;

	v1_x = *p1;
	v1_y = *(p1+1);
	v2_x = *p2;
	v2_y = *(p2+1);
	v3_x = *p3;
	v3_y = *(p3+1);

	texture = (unsigned short)text;


	color = pWinPal32[max(*(unsigned char *)(ambColor),15)];

	
	if (v1_x>=30000)
		v1_x=(65535-v1_x)*-1;
	if (v2_x>=30000)
		v2_x=(65535-v2_x)*-1;
	if (v3_x>=30000)
		v3_x=(65535-v3_x)*-1;
	if (v1_y>=30000)
		v1_y=(65535-v1_y)*-1;
	if (v2_y>=30000)
		v2_y=(65535-v2_y)*-1;
	if (v3_y>=30000)
		v3_y=(65535-v3_y)*-1;

	v1_x = (v1_x*curwidth/320-curwidth/2);
	v1_y = ~(v1_y*curheight/200-curheight/2);

	v2_x = (v2_x*curwidth/320-curwidth/2);
	v2_y = ~(v2_y*curheight/200-curheight/2);

	v3_x = (v3_x*curwidth/320-curwidth/2);
	v3_y = ~(v3_y*curheight/200-curheight/2);

	VertexXYZNDT1* Vertices = (VertexXYZNDT1*)renderSystem->LockVertexBuffer(vertexBuffer,vertexNum,3*vertexBuffer.vertexSize);

	modelList[modelNum].doMatrix=2;
	modelList[modelNum].vertStart=vertexNum;
	modelList[modelNum].zwrite=false;
	modelList[modelNum].cull=CULL_NONE;
	modelList[modelNum].material=0;

	Vertices->pos.x = (float)v1_x;
	Vertices->pos.y = (float)v1_y;
	Vertices->pos.z = (float)z;
	Vertices->difuse.RGBA() = color;
	Vertices->u1() = ((float)t1[0]) / (float)sizex;
	Vertices->v1() = ((float)t1[1]) / (float)sizey;
	vertexType[vertexNum].type = GL_TRIANGLES;
	vertexType[vertexNum].amount = 3;
	vertexType[vertexNum].textNum = texture;
	vertexType[vertexNum].doLight=false;
	vertexType[vertexNum].transparent=false;


	Vertices->n.x = 0.0f;
	Vertices->n.y = 0.0f;
    Vertices->n.z = 0.0f;

	Vertices++;
	vertexNum++;

	Vertices->pos.x = (float)v2_x;
	Vertices->pos.y = (float)v2_y;
	Vertices->pos.z = (float)z;
	Vertices->difuse.RGBA() = color;
	Vertices->u1() = ((float)t2[0]) / (float)sizex;
	Vertices->v1() = ((float)t2[1]) / (float)sizey;
	vertexType[vertexNum].type = 2;
	vertexType[vertexNum].textNum = texture;
	vertexType[vertexNum].doLight=false;

	Vertices->n.x = 0.0f;
	Vertices->n.y = 0.0f;
    Vertices->n.z = 0.0f;

	Vertices++;
	vertexNum++;

	Vertices->pos.x = (float)v3_x;
	Vertices->pos.y = (float)v3_y;
	Vertices->pos.z = (float)z;
	Vertices->difuse.RGBA() = color;
	Vertices->u1() = ((float)t3[0]) / (float)sizex;
	Vertices->v1() = ((float)t3[1]) / (float)sizey;
	vertexType[vertexNum].type = 2;
	vertexType[vertexNum].textNum = texture;
	vertexType[vertexNum].doLight=false;

	Vertices->n.x = 0.0f;
	Vertices->n.y = 0.0f;
    Vertices->n.z = 0.0f;

	Vertices++;
	vertexNum++;
	modelList[modelNum].vertEnd=vertexNum;
	modelNum++;

	renderSystem->UnlockVertexBuffer(vertexBuffer);
}

// 
void DrawTriangle(u16 *p1, u16 *p2, u16 *p3, u16 *t1, u16 *t2, u16 *t3, int color, int text, int z, bool intern)
{
	int v1_x=0, v1_y=0, v2_x=0, v2_y=0, v3_x=0, v3_y=0;
	int texture;
	//int p1_x, p1_y, p2_x, p2_y, p3_x, p3_y;
	//int o1, o2, o3;

	//if (!intern)
	//	return;

	if (vertexNum>MAXVERT-100)
		return;

	v1_x = *p1;
	v1_y = *(p1+1);
	v2_x = *p2;
	v2_y = *(p2+1);
	v3_x = *p3;
	v3_y = *(p3+1);

	texture = (unsigned short)text;

	if (texture>500)
		texture=(unsigned char)text;

	if (texture>0) {
		color = pWinPal32[*(unsigned char *)(ambColor)];
	} else {
		color = pWinPal32[(unsigned char)color];
	}

	//texture = 88;
	//color = pWinPal32[*(unsigned char *)(ambColor)];
/*
	if (texture>0 && color==15) {
		color = pWinPal32[max(*(unsigned char *)(ambColor),15)];
	} else {
		color = pWinPal32[(unsigned char)color];
	}


	switch(texture) {
	case 53:
	case 54:
	case 55:
	case 56:
	case 57:
	case 58:
	case 101:
	case 291:
			color = pWinPal32[15];
	default:
			break;
	}
*/	
	
	if (v1_x>=30000)
		v1_x=(65535-v1_x)*-1;
	if (v2_x>=30000)
		v2_x=(65535-v2_x)*-1;
	if (v3_x>=30000)
		v3_x=(65535-v3_x)*-1;
	if (v1_y>=30000)
		v1_y=(65535-v1_y)*-1;
	if (v2_y>=30000)
		v2_y=(65535-v2_y)*-1;
	if (v3_y>=30000)
		v3_y=(65535-v3_y)*-1;

/*
    v1_x =  v1_x - WIDTH / 2;
    v1_y =  ~(v1_y - HEIGHT / 2);

    v2_x =  v2_x - WIDTH / 2;
    v2_y =  ~(v2_y - HEIGHT / 2);

    v3_x =  v3_x - WIDTH / 2;
    v3_y =  ~(v3_y - HEIGHT / 2);
*/
	v1_x = (v1_x*curwidth/640-curwidth/2);
	v1_y = ~(v1_y*curheight/400-curheight/2);

	v2_x = (v2_x*curwidth/640-curwidth/2);
	v2_y = ~(v2_y*curheight/400-curheight/2);

	v3_x = (v3_x*curwidth/640-curwidth/2);
	v3_y = ~(v3_y*curheight/400-curheight/2);

	//if(FAILED(d3d_vb->Lock(sizeof(CUSTOMVERTEX)*vertexNum,sizeof(CUSTOMVERTEX)*3,(BYTE**)&Vertices,0))) return;
	VertexXYZNDT1* Vertices = (VertexXYZNDT1*)renderSystem->LockVertexBuffer(vertexBuffer,vertexNum,3*vertexBuffer.vertexSize);

	modelList[modelNum].doMatrix=2;
	modelList[modelNum].vertStart=vertexNum;
	modelList[modelNum].zwrite=false;
	modelList[modelNum].cull=CULL_NONE;
	modelList[modelNum].material=GALAXY;
	modelList[modelNum].backsprite=false;

	Vertices->pos.x = (float)v1_x;
	Vertices->pos.y = (float)v1_y;
	Vertices->pos.z = (float)z;
	Vertices->difuse.RGBA() = (DWORD)color;
	Vertices->u1() = (float)(*t1 > 0 ? 1 : 0);
	Vertices->v1() = (float)(*(t1+1) > 0 ? 1 : 0);
	vertexType[vertexNum].type = GL_TRIANGLES;
	vertexType[vertexNum].amount = 3;
	vertexType[vertexNum].textNum = texture;
	vertexType[vertexNum].doLight=false;
	vertexType[vertexNum].transparent=false;


	Vertices->n.x = 0.0f;
	Vertices->n.y = 0.0f;
    Vertices->n.z = 0.0f;

	Vertices++;
	vertexNum++;

	Vertices->pos.x = (float)v2_x;
	Vertices->pos.y = (float)v2_y;
	Vertices->pos.z = (float)z;
	Vertices->difuse.RGBA() = (DWORD)color;
	Vertices->u1() = (float)(*t2 > 0 ? 1 : 0);
	Vertices->v1() = (float)(*(t2+1) > 0 ? 1 : 0);
	vertexType[vertexNum].type = 2;
	vertexType[vertexNum].textNum = texture;
	vertexType[vertexNum].doLight=false;

	Vertices->n.x = 0.0f;
	Vertices->n.y = 0.0f;
    Vertices->n.z = 0.0f;

	Vertices++;
	vertexNum++;

	Vertices->pos.x = (float)v3_x;
	Vertices->pos.y = (float)v3_y;
	Vertices->pos.z = (float)z;
	Vertices->difuse.RGBA() = (DWORD)color;
	Vertices->u1() = (float)(*t3 > 0 ? 1 : 0);
	Vertices->v1() = (float)(*(t3+1) > 0 ? 1 : 0);
	vertexType[vertexNum].type = 2;
	vertexType[vertexNum].textNum = texture;
	vertexType[vertexNum].doLight=false;

	Vertices->n.x = 0.0f;
	Vertices->n.y = 0.0f;
    Vertices->n.z = 0.0f;

	Vertices++;
	vertexNum++;
	modelList[modelNum].vertEnd=vertexNum;
	modelNum++;

	renderSystem->UnlockVertexBuffer(vertexBuffer);
	//d3d_vb->Unlock();
}

// Уже не используется
extern "C" void C_DrawCircle (u16 *center, int radius, int color)
{

	return;
	int xc = *center;
	int yc = *center+1;
	int r = radius;
	int x=r,y=0,d=3-2*r,da=6,db=10-4*r;
	while (x>y)
	{
		C_DrawHLine(xc+x, yc+y, 1, color);
		C_DrawHLine(xc+y, yc+x, 1, color);
		C_DrawHLine(xc-x, yc+y, 1, color);
		C_DrawHLine(xc-y, yc+x, 1, color);
		C_DrawHLine(xc+x, yc-y, 1, color);
		C_DrawHLine(xc+y, yc-x, 1, color);
		C_DrawHLine(xc-x, yc-y, 1, color);
		C_DrawHLine(xc-y, yc-x, 1, color);
		if(d<0)
		{
			d+=da;
			db+=4;
		}
		else
		{
			d+=db;
			db+=8;
			x--;
		}
		da+=4;
		y++;
	}
}


extern "C" void C_DrawScannerMarkerUp(int x1, int y1, int x2, int y2, int col)
{
	
	unsigned short point1[2], point2[2], point3[2], point4[2];
	unsigned short t1[2], t2[2], t3[2];
	int t;

	x1*=2;
	y1*=2;
	x2*=2;
	y2*=2;

	point1[0]=x1;
	point1[1]=y1;
	point2[0]=x2;
	point2[1]=y1;
	point3[0]=x1;
	point3[1]=y2;
	point4[0]=x2;
	point4[1]=y2;

	t1[0]=0;
	t1[1]=0;
	t2[0]=1;
	t2[1]=0;
	t3[0]=1;
	t3[1]=1;

	DrawTriangle(point1, point2, point3, t1, t2, t3, col, 0, 0, 1);
	DrawTriangle(point2, point3, point4, t1, t2, t3, col, 0, 0, 1);

	int size=x2-x1;

	point1[0]=x2;
	point1[1]=y1;
	point2[0]=x2+size*2;
	point2[1]=y1;
	point3[0]=x2;
	point3[1]=y1+size;
	point4[0]=x2+size*2;
	point4[1]=y1+size;

	DrawTriangle(point1, point2, point3, t1, t2, t3, col, 0, 0, 1);
	DrawTriangle(point2, point3, point4, t1, t2, t3, col, 0, 0, 1);
}

extern "C" void C_DrawScannerMarkerDown(int x1, int y1, int x2, int y2, int col)
{
	
	unsigned short point1[2], point2[2], point3[2], point4[2];
	unsigned short t1[2], t2[2], t3[2];
	int t;

	x1*=2;
	y1*=2;
	x2*=2;
	y2*=2;

	point1[0]=x1;
	point1[1]=y1;
	point2[0]=x2;
	point2[1]=y1;
	point3[0]=x1;
	point3[1]=y2;
	point4[0]=x2;
	point4[1]=y2;

	t1[0]=0;
	t1[1]=0;
	t2[0]=1;
	t2[1]=0;
	t3[0]=1;
	t3[1]=1;

	DrawTriangle(point1, point2, point3, t1, t2, t3, col, 0, 0, 1);
	DrawTriangle(point2, point3, point4, t1, t2, t3, col, 0, 0, 1);

	int size=x2-x1;

	point1[0]=x2;
	point1[1]=y2;
	point2[0]=x2+size*2;
	point2[1]=y2;
	point3[0]=x2;
	point3[1]=y2-size;
	point4[0]=x2+size*2;
	point4[1]=y2-size;

	DrawTriangle(point1, point2, point3, t1, t2, t3, col, 0, 0, 1);
	DrawTriangle(point2, point3, point4, t1, t2, t3, col, 0, 0, 1);
}
// Еще используется
extern "C" void C_DrawLine(u16 *p1, u16 *p2, int col)
{
	
	int start_x, end_x, start_y, end_y;
	
	if (vertexNum>MAXVERT-100)
		return;

	start_x = *p1*2;
	start_y = *(p1+1)*2;
	end_x = *p2*2;
	end_y = *(p2+1)*2;

	VertexXYZNDT1* Vertices = (VertexXYZNDT1*)renderSystem->LockVertexBuffer(vertexBuffer,vertexNum,2*vertexBuffer.vertexSize);
	
	Color4c color;
	color.RGBA() = (DWORD)pWinPal32[col];

	modelList[modelNum].doMatrix=2;
	modelList[modelNum].vertStart=vertexNum;
	modelList[modelNum].backsprite=false;
	modelList[modelNum].zwrite=false;

	Vertices->pos.x = (float)(start_x*curwidth/640-curwidth/2);
	Vertices->pos.y = (float)~(start_y*curheight/400-curheight/2);
	Vertices->pos.z = 0.0f;
	Vertices->difuse = color;
	vertexType[vertexNum].type = GL_LINES;
	vertexType[vertexNum].amount = 2;
	vertexType[vertexNum].textNum = 0;
	vertexType[vertexNum].doLight=false;

	Vertices->n.x = 1.0f;
	Vertices->n.y = 1.0f;
    Vertices->n.z = 1.0f;

	Vertices++;
	vertexNum++;

	Vertices->pos.x = (float)(end_x*curwidth/640-curwidth/2);
	Vertices->pos.y = (float)~(end_y*curheight/400-curheight/2);
	Vertices->pos.z = 0.0f;
	Vertices->difuse = color;

	Vertices->n.x = 1.0f;
	Vertices->n.y = 1.0f;
    Vertices->n.z = 1.0f;

	Vertices++;
	vertexNum++;

	modelList[modelNum].vertEnd=vertexNum;
	modelNum++;
	
	renderSystem->UnlockVertexBuffer(vertexBuffer);
}

extern "C" void C_DrawBoxToBuf(int x1, int y1, int x2, int y2, int col) {

	//renderSystem->GetDevice()->Clear(0,NULL,D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,D3DCOLOR_XRGB(0,0,0),1.0f,0);	

	D3DRECT rect;
	int afk=curwidth/1.6f;
	if (aspectfix) {
		rect.x1=(int)((float)curwidth/320*x1)-2;
		rect.y1=(int)((float)afk/200*y1)+(long)(curheight-curwidth/1.6f)/2-2;
		rect.x2=(int)((float)curwidth/320*x2)+2;
		rect.y2=(int)((float)afk/200*y2)+(long)(curheight-curwidth/1.6f)/2+2;
	} else {
		rect.x1=(int)((float)curwidth/320*x1)-2;
		rect.y1=(int)((float)curheight/200*y1)-2;
		rect.x2=(int)((float)curwidth/320*x2)+2;
		rect.y2=(int)((float)curheight/200*y2)+2;
	}

	Color4c color;
	color.RGBA() = (DWORD)pWinPal32[col];

	if (renderSystem->GetDevice()!=NULL)
		renderSystem->GetDevice()->Clear(1,(D3DRECT *)&rect,D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,D3DCOLOR_XRGB(color.r,color.g,color.b),1.0f,0);	

}

extern "C" void C_DrawBoxToBufNew(int x1, int y1, int x2, int y2, int col) {

	//renderSystem->GetDevice()->Clear(0,NULL,D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,D3DCOLOR_XRGB(0,0,0),1.0f,0);	
	D3DRECT rect;

	if (aspectfix) {		
		rect.x1=x1;
		rect.y1=y1+(curheight-curwidth/1.6f)/2;
		rect.x2=x2;
		rect.y2=y2+(curheight-curwidth/1.6f)/2;
	} else {
		rect.x1=x1;
		rect.y1=y1;
		rect.x2=x2;
		rect.y2=y2;
	}

	D3DCOLOR rgb = D3DCOLOR_XRGB(col,col,col);

	if (renderSystem->GetDevice()!=NULL)
		renderSystem->GetDevice()->Clear(1,(D3DRECT *)&rect,D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,rgb,1.0f,0);	

}

void C_Galaxy(int a, int b, int c, int size) {
    int edi = a * size / ((float)curwidth/320);
    int edx = b >> 16; 
    int eax = c >> 16;
	int color;
	int esi, ebx, local_1, local_2, xcof, ycof;
    
	edi = edi > 0 ? edi : 1;
	
	xcof=edi;
	if (aspectfix)
		ycof=edi;
	else
		ycof=edi * (curwidth/1.6f)/curheight;

	ycof = ycof > 0 ? ycof : 1;

	int width = curwidth;
	int height;
	if (aspectfix)
		height = curwidth/1.6f*0.80f;
	else
		height = curheight*0.80f;

	edx = edx - (xcof>>1)+xcof;
	eax = eax - (ycof>>1)-ycof;

    b = edx + xcof * (width/2/size);
    local_1 = eax - ycof * (height/2/size-1);
	if (aspectfix)
		esi = curwidth/1.6f*0.78f;
	else
		esi = curheight*0.78f;
    do {
        local_2 = b;
        ebx = width;
        do {
			color = FUNC_000853_GetNumstars(local_2, local_1, xcof) >> 8;
            C_DrawBoxToBufNew(ebx, esi, ebx + size, esi + size, color);
            local_2 = local_2 - xcof;
            ebx = ebx - size;
        } while(ebx >= 0);
        local_1 = local_1 + ycof;
        esi = esi - size;
    } while(esi >= 0);
}

extern "C" unsigned short* C_FUNC_001752_SkipIfNotVisible(char *gptr, unsigned short *var2, unsigned short var1) 
{
	if ((*var2 & 0x8000) == 0 || currentModel==235 || currentModel<3) {
		return FUNC_001752_SkipIfNotVisible(gptr, var2, var1);
	} else {
		return var2+1;
	}
}

extern "C" unsigned short* C_FUNC_001755_SkipIfVisible(char *gptr, unsigned short *var2, unsigned short var1) 
{
	//if ((*var2 & 0x8000) == 0 || currentModel==235) {
		return FUNC_001755_SkipIfVisible(gptr, var2, var1);
	//} else {
	//	return var2+1+(var1>>5);
	//}
}

extern "C" void C_FUNC_000847_Galaxy(int a, int b, int c)
{
	C_Galaxy(a, b, c, curwidth/320);
}

extern "C" void C_FUNC_000848_Galaxy(int a, int b, int c)
{
	C_Galaxy(a, b, c, curwidth/320*2);
}


extern "C" void C_FUNC_000849_Galaxy(int a, int b, int c)
{
	C_Galaxy(a, b, c, curwidth/320*4);
}

extern "C" void C_DrawParticle(short *ptr, int size, int col);

extern "C" void C_ConsoleSetButtonImage(int image, int value)
{	
	//if (image == 56 && value==1) {
		short coord[2];
		coord[0] = 100;
		coord[1] = 170;
		//C_DrawParticle((unsigned short *)&coord[0], 10, 15);
	//}
}

extern "C" void C_DrawSprite(int xcent, int ycent, int num, int flag, int col)
{
	float size;
	int aspectfactor=(curheight-curwidth/1.6f);

	if (vertexNum>MAXVERT-100)
		return;

	if (flag==0) {
		size = (num+2);

		Color4c color;
		color.RGBA() = (DWORD)pWinPal32[col];

		modelList[modelNum].doMatrix=2;
		modelList[modelNum].vertStart=vertexNum;
		modelList[modelNum].backsprite=true;
		modelList[modelNum].zwrite=false;

		VertexXYZNDT1* Vertices = (VertexXYZNDT1*)renderSystem->LockVertexBuffer(vertexBuffer,vertexNum,1*vertexBuffer.vertexSize);
		Vertices->pos.x = (float)(xcent*curwidth/320-curwidth/2);
		Vertices->pos.y = (float)~(ycent*(curheight)/200-(curheight)/2);//-size*2*curheight/200;
		if (aspectfix) {
			Vertices->pos.y = (float)~(ycent*(curheight)/200-(curheight)/2);//-size*2*curheight/200;
		}
		if (size!=0) {
			int b=0;
		}
		Vertices->pos.z = 0.0f;
		Vertices->difuse = color;
		Vertices->u1() = (float)0;
		Vertices->v1() = (float)0;
		vertexType[vertexNum].type = GL_POINTS2;
		vertexType[vertexNum].radius = (float)0.1f*size;
		vertexType[vertexNum].textNum = 602;
		vertexType[vertexNum].doLight=false;
		vertexType[vertexNum].transparent=false;

		Vertices->n.x = 0;
		Vertices->n.y = 0;
		Vertices->n.z = 0;

		Vertices++;
		vertexNum++;

		modelList[modelNum].vertEnd=vertexNum;
		modelNum++;

		renderSystem->UnlockVertexBuffer(vertexBuffer);
	}
}

extern "C" void C_DrawSprite2(short *ptr, int size, int col)
{
	C_DrawSprite((int)ptr[0]-1, (int)ptr[1]-1, -20, 0, col);
}

extern "C" void C_DrawParticle(short *ptr, int size, int col)
{
	int aspectfactor=(curheight-curwidth/1.6f);

	if (vertexNum>MAXVERT-100)
		return;

	size = size == 0 ? 1 : size;

	Color4c color;
	color.RGBA() = (DWORD)pWinPal32[col];

	modelList[modelNum].doMatrix=2;
	modelList[modelNum].vertStart=vertexNum;

	ptr[1]+=size/2;
	VertexXYZNDT1* Vertices = (VertexXYZNDT1*)renderSystem->LockVertexBuffer(vertexBuffer,vertexNum,1*vertexBuffer.vertexSize);
	Vertices->pos.x = (float)(ptr[0]*curwidth/320-curwidth/2);
	Vertices->pos.y = (float)~(ptr[1]*(curheight)/200-(curheight)/2);//-size*2*curheight/200;
	if (aspectfix) {
		Vertices->pos.y = (float)~(ptr[1]*(curheight)/200-(curheight)/2);//-size*2*curheight/200;
	}
	if (size!=0) {
		int b=0;
	}
	Vertices->pos.z = 0.0f;
	Vertices->difuse = color;
	Vertices->u1() = (float)0;
	Vertices->v1() = (float)0;
	vertexType[vertexNum].type = GL_POINTS;
	vertexType[vertexNum].radius = (float)0.1f*size;
	vertexType[vertexNum].textNum = 602;
	vertexType[vertexNum].doLight=false;
	vertexType[vertexNum].transparent=true;

	Vertices->n.x = 0;
	Vertices->n.y = 0;
	Vertices->n.z = 0;

	Vertices++;
	vertexNum++;

	modelList[modelNum].vertEnd=vertexNum;
	modelNum++;

	renderSystem->UnlockVertexBuffer(vertexBuffer);
}

static int prevx=0, prevy=0, prevlen=0, prevcol=0, prevIndex=0;

// Использовалась много где. Например, для закрашивания сплайновых
// полигонов. Вряд ли еще будет нужна.
extern "C" void C_DrawHLine(int x, int y, int len, int col)
{	
	int start_x, end_x, start_y, end_y;
	
	return;
	if (vertexNum>MAXVERT-100)
		return;

	start_x = x;
	start_y = y;
	end_x = x+len;
	end_y = y;

	VertexXYZNDT1* Vertices = (VertexXYZNDT1*)renderSystem->LockVertexBuffer(vertexBuffer,vertexNum,2*vertexBuffer.vertexSize);
	
	Color4c color;
	color.RGBA() = (DWORD)pWinPal32[col];

	if (color.r!=color.b)
		int a=0;
	modelList[modelNum].doMatrix=2;
	modelList[modelNum].vertStart=vertexNum;

	Vertices->pos.x = (float)(start_x*curwidth/640-curwidth/2);
	Vertices->pos.y = (float)~(start_y*curheight/400-curheight/2);
	Vertices->pos.z = 1.0f;
	Vertices->difuse = color;
	vertexType[vertexNum].type = GL_LINES;
	vertexType[vertexNum].amount = 2;
	vertexType[vertexNum].textNum = -1;
	vertexType[vertexNum].doLight=false;

	Vertices->n.x = 1.0f;
	Vertices->n.y = 1.0f;
    Vertices->n.z = 1.0f;

	Vertices++;
	vertexNum++;

	Vertices->pos.x = (float)(end_x*curwidth/640-curwidth/2);
	Vertices->pos.y = (float)~(end_y*curheight/400-curheight/2);
	Vertices->pos.z = 1.0f;
	Vertices->difuse = color;

	Vertices->n.x = 1.0f;
	Vertices->n.y = 1.0f;
    Vertices->n.z = 1.0f;

	Vertices++;
	vertexNum++;

	modelList[modelNum].vertEnd=vertexNum;
	modelNum++;
	
	renderSystem->UnlockVertexBuffer(vertexBuffer);

}

// Для отображения текста. Есть некоторые мелкие проблемы.
char *C_DrawText (char *pStr, int xpos, int ypos, int col, bool shadow, int ymin, int ymax, int height) {
	char *text;
	int color, num;
	FFTEXT *buf;
	bool stop=false;
	int newLine=10;

	if (textStartX==-1) {
		textStartX=xpos;
	}

	buf = ffText; 
	num = textNum;

	if (num>1900)
		return pStr;

	if (ymax > 0 && ypos>ymax-10) {
		return pStr;
	}

	if (ypos>158) {
		newLine=8;
	}
	if (height>0) {
		newLine=height;
	}

	textNum++;

	text = pStr;
	if (!shadow) {
		color = col;
	} else {
		color = 0;
	}

	buf[num].x = xpos*2;
	buf[num].y = ypos*2;

	buf[num].color = getD3DColor(pWinPal32[color]);


	for(int i=0;stop==false;i++) {
//	0x0 0x??: Write image (bitmap index)
//	0x1 0x??: Set colour (pal colour)
//	0xd: Newline
//	0x1d 0x?? 0x??: Set position (xpos/2, ypos)
//	0x1e 0x??: Set xpos (xpos/2)
//	0xff: not sure...

		switch((unsigned char)*text) {
			case 1:	// color
					if (!shadow) {
						buf[num].text[i]=0;
						return C_DrawText (text+2, xpos+i*4, ypos, 
							*((unsigned int *)textColor+*(text+1)), false, ymin, ymax, height);
					} else {
						buf[num].text[i]=0;
						return C_DrawText (text+2, xpos+i*4, ypos, 
							0, true, ymin, ymax, height);
					}
			case 13: // new line
					buf[num].text[i]=0;
					return C_DrawText (text+1, textStartX, ypos+newLine, 
						col, shadow, ymin, ymax, height);
			case 30: // set x pos
					buf[num].text[i]=0;
					return C_DrawText (text+2,(unsigned char)*(text+1)*2+shadow, ypos, 
						col, shadow, ymin, ymax, height);
			case 31: // set xy pos
					textStartX=(unsigned char)*(text+1)*2+shadow;
					buf[num].text[i]=0;
					return C_DrawText (text+3,(unsigned char)*(text+1)*2+shadow, (unsigned char)*(text+2)+shadow, 
						col, shadow, ymin, ymax, height);
			case 255:
					text++;
					i--;
					break;
			case 0: // image?
					buf[num].text[i]=0;
					textStartX=-1;
					stop=true;
					break;
			case '{':
					buf[num].text[i]='/';
					buf[num].text[i+1]='h';
					i++;
					text++;
					break;					
			case '|':
					buf[num].text[i]='/';
					buf[num].text[i+1]='s';
					i++;
					text++;
					break;
			case 129:
					buf[num].text[i]='[';
					buf[num].text[i+1]=' ';
					buf[num].text[i+2]=' ';
					buf[num].text[i+3]=']';
					i+=3;
					text++;
					break;
			default:
					if (ymin<0 || ypos>=ymin) {
						buf[num].text[i]=*text;
					} else {
						buf[num].text[i]=0;
					}
					text++;
					break;
		}		
	}
	return text+1;
}

// Текст
extern "C" char *C_PersistTextWrite (char *pStr, int xpos, int ypos, int col) {
	return NULL;
}

// Текст
extern "C" char *C_WriteStringShadowed (char *pStr, int xpos, int ypos, int col) {
	C_DrawText (pStr, xpos+1, ypos+1, 0, true, -1, -1, -1);
	return C_DrawText (pStr, xpos, ypos, (unsigned char)col, false, -1, -1, -1);		
}

// Текст
extern "C" char *C_TextWriteInternal(char *pStr, int xpos, int ypos, int col, char *charfunc,
		int ymin, int ymax, int height)
{
	return C_DrawText (pStr, xpos, ypos, (unsigned char)col, false, ymin, ymax, height);

}

void DrawClouds(float *p1, float radius, int tex, int n) {
	D3DXVECTOR3 vNormal;
	Color4c color;
	int i, j;
	double theta1,theta2,theta3;
	float pp[3];
	
	if (exportb[currentModel]==true && splineExport==true) 
		return;

	float cx=(float)*p1;
	float cy=(float)*(p1+1);
	float cz=(float)*(p1+2);

	int l=n/2*n*2+2;

	//VertexXYZNDT1* Vertices = (VertexXYZNDT1*)renderSystem->LockVertexBuffer(vertexBuffer,vertexNum,l*vertexBuffer.vertexSize);

	color.set(255,255,255);

	currentTex=704;
	doLight=0;
	transparent=0;

    const float PI     = 3.14159265358979f;
    const float TWOPI  = 6.28318530717958f;
    const float PIDIV2 = 1.57079632679489f;

    float ex = 0.0f;
    float ey = 0.0f;
    float ez = 0.0f;

	float snt1, cnt1, snt2, cnt2, snt3, cnt3;

	if (n<60)
		j = 1;
	else
		j = 0;

	D3DXMatrixIdentity(&posMatrix);
	D3DXMatrixIdentity(&mainRotMatrix);
	D3DXMatrixMultiply(&mainRotMatrix, &mainRotMatrix, &mainRotMatrixO);

    for(;j < n/2; j++ )
    {
        theta1 = j * TWOPI / n - PIDIV2;
        theta2 = (j + 1) * TWOPI / n - PIDIV2;
		snt1 = sin(theta1);
		snt2 = sin(theta2);
		cnt1 = cos(theta1);
		cnt2 = cos(theta2);

        for( int i = 0; i <= n; i++ )
        {
            theta3 = i * (TWOPI) / n;
			snt3 = sin(theta3);
			cnt3 = cos(theta3);
			
            ex = cnt1 * cnt3;
            ey = snt1;
            ez = cnt1 * snt3;
            pp[0] = cx + radius * ex;
            pp[1] = cy + radius * ey;
            pp[2] = cz + radius * ez;
			
			posMatrix[12]=pp[0];
			posMatrix[13]=pp[1];
			posMatrix[14]=pp[2];

			DrawBillboard(&pp[0], 1000);
        }
    }

	//renderSystem->UnlockVertexBuffer(vertexBuffer);
    //d3d_vb->Unlock();
}


D3DXVECTOR3 a, b, c;
D3DXVECTOR3 pos;
D3DXMATRIX matScale;

bool inAtmo;
float atmoW;
float atmoR;

void DrawAtmosphere(char *ptr, float c=1.0f) 
{

	return;

		D3DXMATRIX atm, pposMatrix;
		float scal=1.0f;
		
		D3DXMatrixIdentity(&pposMatrix);
		D3DXMatrixIdentity(&matScale);
		pposMatrix[12]=pos.x;
		pposMatrix[13]=pos.y;
		pposMatrix[14]=pos.z;

		modelList[modelNum].index=445;
		modelList[modelNum].doMatrix=1;
		modelList[modelNum].subObject=false;
		transparent=false;
		doLight=true;

		modelList[modelNum].vertStart=vertexNum;

		float dist=sqrtf(pos.x*pos.x+pos.y*pos.y+pos.z*pos.z);
		float radius2=(float)(*(mod->Vertices_ptr+2)<<currentScale)/DIVIDER;
		//radius2*=1.04f;

		modelList[modelNum].lightPos.x=-(float)*(int*)(ptr+308)*1500;
		modelList[modelNum].lightPos.y=-(float)*(int*)(ptr+312)*1500;
		modelList[modelNum].lightPos.z=-(float)*(int*)(ptr+316)*1500;


		modelList[modelNum].ambientR=currentAmbientR*17/2+128;//min(255,currentAmbientR*36);
		modelList[modelNum].ambientG=currentAmbientG*17/2+128;//min(255,currentAmbientG*36);
		modelList[modelNum].ambientB=currentAmbientB*17/2+128;//min(255,currentAmbientB*36);

		currentTex=-1;
		currentColor.set(0,0,0);

		if (currentModel!=445 && currentModel!=447 && currentModel!=449) {
			if (dist>radius2*1.04f) {
				scal=0.01f*(*(int*)(ptr+0x130));
				pposMatrix[12]*=scal;
				pposMatrix[13]*=scal;
				pposMatrix[14]*=scal;
			}
		} else {
			scal=0.03f;
			pposMatrix[12]*=scal;
			pposMatrix[13]*=scal;
			pposMatrix[14]*=scal;
		}
		scal*=45;
		pposMatrix[12]*=45;
		pposMatrix[13]*=45;
		pposMatrix[14]*=45;

		// Пусть облака движутся.
		D3DXMatrixRotationY(&atm, (float)*DATA_008804/1200000000.0f);
		D3DXMatrixMultiply(&atm, &atm, &mainRotMatrixO);
		D3DXMatrixMultiply(&modelList[modelNum].world, &atm, &pposMatrix);
		D3DXMatrixScaling(&matScale, scal, scal, scal);
		D3DXMatrixMultiply(&modelList[modelNum].world, &matScale, &modelList[modelNum].world);

		if (dist<radius2*c) {
			inAtmo=true;
			atmoW=(radius2*c)/dist;
			atmoR=(radius2*c);
			modelList[modelNum].cull=CULL_CW;
			modelList[modelNum].zwrite=false;
			modelList[modelNum].zenable=false;
			modelList[modelNum].material=ATMO2;
			DrawRealSphere(nullOrigin, radius2 * c, 699, 60);
		} else {
			modelList[modelNum].cull=CULL_CCW;
			modelList[modelNum].zwrite=true;
			modelList[modelNum].material=ATMO;		
			DrawRealSphere(nullOrigin, radius2 * c, 700, 60);
		}

		modelList[modelNum].dist=dist;
		previousModel=currentModel;

		modelList[modelNum].vertEnd=vertexNum;
		modelNum++;
}

float mPosX=0;
float mPosY=0;
float mPosZ=0;

extern void RenderDirect3D(void);

bool Rings(int modelNum)
{
	return (modelNum==314 || modelNum==319);
}
bool Planet(int modelNum)
{
	return ((modelNum>=113 && modelNum<=136) || modelNum==445 || modelNum==447 || modelNum==449);
}
bool PlanetWithLife(int modelNum)
{
	return ((modelNum>=121 && modelNum<=131) || modelNum==449);
}

// Основная функция перехвата трехмерных объектов
// Получает указатель на структуру DrawMdl_t. Смотри письмо от Jongware.
extern "C" int C_Break(char *ptr, unsigned short *cmd)
{
	FILE *pLog = NULL;
	int i=0;
	int curVer;
	int objNum;
	float scale1=1.0f;
	float scale2=1.0f;
	float radius2;
	float dis;
	unsigned int* oiList=(unsigned int*)*instanceList;
	

	skipCurrentModel=false;
	// На всякий пожарный
	if (vertexNum>MAXVERT-3000)
		return 0;

	char *iPtr;
	char *mod3D;

	//if (subObject==false) {
		for (int ii=0;ii<10;ii++)
			modelList[modelNum].light[ii].enable=false;
	//}


	// Ссылка на список указателей на модели
	mod = (Model_t *)*(int *)ptr;
	// указатель на структуру ModelInstance_t. Смотри письмо от Jongware.
	iPtr = (char *)*(int *)(ptr+0x14c);
	// Когда-то была нужна
	mod3D= (char *)*(int *)(iPtr+0);

	// Глобальные указатели
	gptr=ptr;
	iptr=iPtr;
	gcmd=cmd;

	t_DATA_007824 = *(unsigned int*)DATA_007824;

	// Извращенский способ получить точный индекс модели
	for(i=0;i<=465;i++) {
		if (*(int *)ptr==*((int *)model+i)) {
			objNum=i;
			break;
		}
	}

	modelList[modelNum].landingGear=*(unsigned short *)(iPtr+158);

	mainObjectNum=*(unsigned short *)(iPtr+130);

	if (mainObjectNum==objNum) {
		subObject=false;
		mainModelNum=modelNum;
	} else {
		subObject=true;		
	}

	// Локальный цвет модели
	localColor[0]=*(ptr+244);
	localColor[1]=*(ptr+245);
	localColor[2]=*(ptr+246);

	// Локальные переменные модели
	localvar=(int *)(ptr+0x54);

	// Запоминаем состояние локальных переменных модели.
	// Потом надо будет их вернуть.
	if (!((currentModel>=113 && currentModel<=136) || currentModel==445 || currentModel==447 || currentModel==449)) {
		t_localvar[0]=localvar[0];
		t_localvar[1]=localvar[1];
		t_localvar[2]=localvar[2];
		t_localvar[3]=localvar[3];
		t_localvar[4]=localvar[4];
		t_localvar[5]=localvar[5];
		t_localvar[6]=localvar[6];
	}

	if (subObject) {
		//localvar[0]++;
		/*
		localvar[0]=p_localvar[0]+1;
		localvar[1]=p_localvar[1];
		localvar[2]=p_localvar[2];
		//localvar[3]=0;
		//localvar[4]=0;
		//localvar[5]=0;
		//localvar[6]=0;
		*/
	}

	// Глобальные переменные модели
	globalvar=(short *)(iPtr+156);	

	// Текущая модель
	currentModel=objNum;

	float origin[3];
	origin[0]=0;
	origin[1]=0;
	origin[2]=0;

	// rotate
	// Получаем матрицу поворота модели
	unsigned int dd=0x80009fff;
	a=D3DXVECTOR3((float)((double)*(int*)(ptr+20)/dd),
				  (float)((double)*(int*)(ptr+24)/dd),
				  (float)((double)*(int*)(ptr+28)/dd));
	b=D3DXVECTOR3((float)((double)*(int*)(ptr+32)/dd),
				  (float)((double)*(int*)(ptr+36)/dd),
				  (float)((double)*(int*)(ptr+40)/dd));
	c=D3DXVECTOR3((float)((double)*(int*)(ptr+44)/dd),
				  (float)((double)*(int*)(ptr+48)/dd),
				  (float)((double)*(int*)(ptr+52)/dd));
	//D3DXVec3Normalize(&a, &a);
	//D3DXVec3Normalize(&b, &b);
	//D3DXVec3Normalize(&c, &c);

	mainRotMatrix[0]=mainRotMatrixO[0]=a.x;
	mainRotMatrix[1]=mainRotMatrixO[1]=a.y;
	mainRotMatrix[2]=mainRotMatrixO[2]=a.z;

	mainRotMatrix[4]=mainRotMatrixO[4]=b.x;
	mainRotMatrix[5]=mainRotMatrixO[5]=b.y;
	mainRotMatrix[6]=mainRotMatrixO[6]=b.z;

	mainRotMatrix[8]=mainRotMatrixO[8]=c.x;
	mainRotMatrix[9]=mainRotMatrixO[9]=c.y;
	mainRotMatrix[10]=mainRotMatrixO[10]=c.z;

	mainRotMatrix[12]=mainRotMatrixO[12]=0.0f;
	mainRotMatrix[13]=mainRotMatrixO[13]=0.0f;
	mainRotMatrix[14]=mainRotMatrixO[14]=0.0f;

	mainRotMatrix[3]=mainRotMatrixO[3]=0.0f;
	mainRotMatrix[7]=mainRotMatrixO[7]=0.0f;
	mainRotMatrix[11]=mainRotMatrixO[11]=0.0f;
	mainRotMatrix[15]=mainRotMatrixO[15]=1.0f;

	D3DXMatrixIdentity(&currentMatrix);
	billboardMatrix=false;
	//D3DXMatrixIdentity(&mainRotMatrix);
	// position
	// Получаем координаты модели
	//if (*(int*)(ptr+0x130)) {
		//pos.x=(float)((*(int*)(ptr+4)<<*(int*)(ptr+0x130))>>8)/DIVIDER;
		//pos.y=(float)((*(int*)(ptr+8)<<*(int*)(ptr+0x130))>>8)/DIVIDER;
		//pos.z=(float)((*(int*)(ptr+12)<<*(int*)(ptr+0x130))>>8)/DIVIDER;
	//} else {
		pos.x=(float)*(int*)(ptr+4)/DIVIDER;
		pos.y=(float)*(int*)(ptr+8)/DIVIDER;
		pos.z=(float)*(int*)(ptr+12)/DIVIDER;
	//}

	//pos.x=(float)*(int*)(iPtr+0x8C)/DIVIDER;
	//pos.y=(float)*(int*)(iPtr+0x8C+4)/DIVIDER;
	//pos.z=(float)*(int*)(iPtr+0x8C+8)/DIVIDER;

/*
	int x1=*(int*)(iPtr+0x3E);
	int x2=*(int*)(iPtr+0x3E+4);
	int y1=*(int*)(iPtr+0x3E+8);
	int y2=*(int*)(iPtr+0x3E+8+4);
	int z1=*(int*)(iPtr+0x3E+16);
	int z2=*(int*)(iPtr+0x3E+16+4);

	int x3 = *(int*)(iPtr+0x26);
	int y3 = *(int*)(iPtr+0x2E);
	int z3 = *(int*)(iPtr+0x36);

	pos.x = ((float)x3/100);
	pos.y = ((float)y3/100);
	pos.z = ((float)z3/100);
*/
	//pos.x = (float)x1;
	//pos.y = (float)y1;
	//pos.z = (float)z1;

	//pos.x /= 100;
	//pos.y /= 100;
	//pos.z /= 100;

	if (Planet(currentModel) || currentModel==314)
		dis=sqrtf(pos.x*pos.x+pos.y*pos.y+pos.z*pos.z);
	else
		dis=pos.z;

	if (currentModel>=113 && currentModel<=136) {
		//scale1=32.0f;
		//scale2=32.0f;
	}
/*
	if (objNum==86) {
		char buf[256];
		sprintf(buf,"starport x = %f, y=%f, z=%f",pos.x,pos.y,pos.z);
		DrawText (buf, 10, 30, 7, false, -1, -1, -1);
		//sprintf(buf,"starport scale44 = %i",*(int*)(ptr+0x44));
		//DrawText (buf, 10, 40, 7, false, -1, -1, -1);
	}
*/

	// 
	posMatrix[0]=1.0f;
	posMatrix[1]=0.0f;
	posMatrix[2]=0.0f;

	posMatrix[4]=0.0f;
	posMatrix[5]=1.0f;
	posMatrix[6]=0.0f;

	posMatrix[8]=0.0f;
	posMatrix[9]=0.0f;
	posMatrix[10]=1.0f;

	posMatrix[12]=mainModelCoord.x=pos.x*scale1;
	posMatrix[13]=mainModelCoord.y=pos.y*scale1;
	posMatrix[14]=mainModelCoord.z=pos.z*scale1;

	posMatrix[3]=0.0f;
	posMatrix[7]=0.0f;
	posMatrix[11]=0.0f;
	posMatrix[15]=1.0f;

	// Ниже идут некоторы дебажные скипы моделей


	//if (objNum>80 && objNum<=100)
	//	return 0;

	// Пропускаем отрисовку векторного текста на внешних моделях
	/*
	if (objNum<3 && 
		subObject==true && 
		objectList[previousModel] && 
		(*(unsigned char*)(iPtr+0x14c)& 0x10)==0 &&
		(*(unsigned char*)(iPtr+0x14c)& 0x20)==0 &&
		(*(unsigned char*)(iPtr+0x14c)& 0x40)==0)
		return 0;
	*/
	// Пропускаем отрисовку всего кроме ракет на внешних моделях кораблей
	//if (objNum>=179 && objNum<=206 && objNum!=201 && subObject==true && (previousModel >= 14 && previousModel < 70) && objectList[previousModel])
	//	return 0;

//	if (modelconfig[mainObjectNum].skip)
//		return 0;

	if (subObject==true && objNum<3 && objectList[mainObjectNum]->config.notdrawtext)
		return 0;

	if (subObject==true && objNum<3 && objectList[previousModel]->config.notdrawtext)
		return 0;

	//if (subObject==true && objNum>=3 && objectList[mainObjectNum]->config.notdrawsubmodels)
	//	return 0;

	if (subObject==true && objNum>=3 && objectList[previousModel]->config.notdrawsubmodels)
		skipCurrentModel=true;

	if (objNum>=233 && objNum<=235)
		clearBeforeRender=true;

    //planet = FUNC_001532_GetModelInstancePtr (prevIndex, objectList);
	//modelNum = *(unsigned short *)((char *)planet+0x82);
   // planetModel = FUNC_001538_GetModelPtr ((int)modelNum);    
	char * zaza;
	Model_t *zazamodel;
	int zazaindex;

	char txt[256];

	if (0 && *DATA_008874!=0) {
		zaza=C_GetModelInstancePtr(*DATA_008874, oiList);
		zazaindex=*(unsigned short *)((char *)zaza+0x82);
		zazamodel=FUNC_001538_GetModelPtr(zazaindex);

		if (zaza==iptr) {
		sprintf(txt, "%f", dis);
		C_DrawText (&txt[0], 100, 100, 15, false, 0, 0, 20);

		sprintf(txt, "x: %i, y: %i, z: %i",*(int*)(ptr+4), *(int*)(ptr+8), *(int*)(ptr+12));
		C_DrawText (&txt[0], 100, 110, 15, false, 0, 0, 20);

		sprintf(txt, "%i, %i",*(int*)(ptr+0x130), *(int*)(ptr+0x148));
		C_DrawText (&txt[0], 100, 120, 15, false, 0, 0, 20);

		}

	}

	

	//if (objNum>=113 && objNum<=148)
	//	return 0;

	//if (objNum==173)	// Galactic Nebula
	//	return 0;

	//if (objNum==170)	// 
	//	return 0;

	//if (objNum==315)	// galaxy background
	//	return 0;

	//if (objNum==444)	// galaxy background (intro)
	//	return 0;

	//if (objNum==158)	// smoke
	//	return 0;

	//if (objNum==334)	// laser flash
	//	return 0;

	//if (objNum==84) {
	//	return 0;
	//}
	//if (objNum==109)
	//	return 0;

	//if (objNum==59)	// anaconda
	//	return 0;

	//if (objNum==156)	// laser beam
	//	return 0;
/*
	if (objNum==314) {
		posMatrix[12]*=10;
		posMatrix[13]*=10;
		posMatrix[14]*=10;
		scale2=10;
	}
*/
	// Получаем коэффициент ресайза модели
	currentScale=*(int*)(ptr+0x148);
	currentScale2=*(int*)(ptr+0x130);

	radius2=(float)(*(mod->Vertices_ptr+2)<<currentScale)/DIVIDER;

	// Это планета. Добавим атмосферу (если в радиусе, то до планеты).	
	if (PlanetWithLife(currentModel)) {
		if (dis <= radius2*1.04f) {
			DrawAtmosphere(ptr, 1.04f);
			if (dis <= radius2*1.02f)
				DrawAtmosphere(ptr, 1.02f);
		}
	}

	modelList[modelNum].subObject=subObject;

	// Материал 
	modelList[modelNum].zwrite=true;
	modelList[modelNum].zenable=true;
	modelList[modelNum].zclear=false;
	modelList[modelNum].backsprite=false;
	modelList[modelNum].cull=CULL_NONE;

	if (objNum==315 || objNum==444)	{// galaxy background
		modelList[modelNum].zwrite=false;
		modelList[modelNum].material=GALAXY;
		modelList[modelNum].cull=CULL_CCW;
	} else if (currentModel>=137 && currentModel<=148) {
		modelList[modelNum].material=SUN;
		modelList[modelNum].cull=CULL_CCW;
		modelList[modelNum].zwrite=false;
	} else if (Planet(currentModel)==true) {
		modelList[modelNum].material=PLANET;
		modelList[modelNum].cull=CULL_NONE;
		modelList[modelNum].zwrite=true;
	} else if (currentModel==314) {
		modelList[modelNum].material=-1;
		modelList[modelNum].cull=CULL_NONE;
	} else if (currentModel==169 || currentModel==170) { // cross
		modelList[modelNum].material=GALAXY;
		modelList[modelNum].zwrite=false;
	} else {
		modelList[modelNum].material=0;
		modelList[modelNum].cull=CULL_NONE;
	}

	if (currentModel==227)
		modelList[modelNum].cull=CULL_CCW;

	if (currentModel==156 ||						// Laser
		currentModel==319 ||						// Hyperspace warp
		(currentModel>353 && currentModel<361)) {	// Laser intro
			modelList[modelNum].material=SUN;
	}

	modelList[modelNum].ambientR=currentAmbientR*32;
	modelList[modelNum].ambientG=currentAmbientG*32;
	modelList[modelNum].ambientB=currentAmbientB*32;

	modelList[modelNum].localR=*(unsigned char*)(ptr+0xf4)*32;
	modelList[modelNum].localG=*(unsigned char*)(ptr+0xf5)*32;
	modelList[modelNum].localB=*(unsigned char*)(ptr+0xf6)*32;

	// Получаем позицию источника света для текущей модели
	modelList[modelNum].lightPos.x=-(float)*(int*)(ptr+308)*1500;
	modelList[modelNum].lightPos.y=-(float)*(int*)(ptr+312)*1500;
	modelList[modelNum].lightPos.z=-(float)*(int*)(ptr+316)*1500;



	unsigned short mf;
	int mn=0;
	for (i=29;i<=33;i++) {
		mf=globalvar[i];
		modelList[modelNum].missile[mn]=mf>>8;
		mn++;
		modelList[modelNum].missile[mn]=mf&0xFF;
		mn++;
	}
	
	// Текущая вершина
	curVer=vertexNum;

	mxVertices=0;
	mxIndexes=0;

	modelList[modelNum].index=currentModel;
	modelList[modelNum].vertStart=vertexNum;


	// Если существует внешняя модель, то пропускаем отрисовку из скрипта
	if (exportb[currentModel]==false && (objectList[currentModel]->Exist() || objectList[currentModel]->config.skip!=0)) {
		skipCurrentModel=true;
	}
//	} else {
//		skipCurrentModel=false;
//	}
	
	if ((currentModel==186 || currentModel==187) && objectList[mainObjectNum]->Exist())
		skipCurrentModel=true;

	if (objNum==166) { // starmap grid
		//D3DXMatrixRotationX(&mainRotMatrix, 90*(3.14f/180));	
		//D3DXMatrixIdentity(&mainRotMatrix);
		int a=0;
	}

	if (Planet(currentModel) || Rings(currentModel) || currentModel==154) {
		if (Planet(currentModel))
			D3DXMatrixIdentity(&mainRotMatrix);
		//D3DXMatrixIdentity(&posMatrix);

		if (currentModel!=445 && currentModel!=447 && currentModel!=449) {
				scale2=*(int*)(ptr+0x130);
				posMatrix[12]*=scale2;
				posMatrix[13]*=scale2;
				posMatrix[14]*=scale2;
				//posMatrix[12]*=0.01f;
				//posMatrix[13]*=0.01f;
				//posMatrix[14]*=0.01f;
			if (dis > radius2*1.04) {
				if (pos.z<0)
					return 0;
				if ((currentModel<134 && currentModel>137) && currentModel!=148 && currentModel!=445) {
					modelList[modelNum].zclear=true;
				}
			} else {
				modelList[modelNum].zclear=true;
			}
		} else {
			//modelList[modelNum].zclear=true;
			//modelList[modelNum].zwrite=false;

			//scale2=0.03f;
			//posMatrix[12]*=scale2;
			//posMatrix[13]*=scale2;
			//posMatrix[14]*=scale2;
		}
/*
		//if (!incabin) {
			scale2*=50;
			if (PlanetWithLife(currentModel)) {
				posMatrix[12]*=50.0001;
				posMatrix[13]*=50.0001;
				posMatrix[14]*=50.0001;
			} else {
				posMatrix[12]*=50.0025;
				posMatrix[13]*=50.0025;
				posMatrix[14]*=50.0025;
			}
			*/
		//}
		//if (incabin)
			//modelList[modelNum].zclear=true;
	}


 // 	if (mainObjectNum==154) { // 
	//	scale2=100;//*(int*)(ptr+0x148);
	//	posMatrix[12]*=scale2;
	//	posMatrix[13]*=scale2;
	//	posMatrix[14]*=scale2;
	//}
 // 	if (currentModel==314) { // planet ring
	//	scale2=0.01f*(*(int*)(ptr+0x130));
	//	posMatrix[12]*=scale2;
	//	posMatrix[13]*=scale2;
	//	posMatrix[14]*=scale2;

	//	if (dis > lastPlanetRadius*2) {
	//		//modelList[modelNum].zclear=true;
	//	}
	//	scale2*=50;
	//	posMatrix[12]*=50;
	//	posMatrix[13]*=50;
	//	posMatrix[14]*=50;
	//	//modelList[modelNum].zclear=true;
	//}

	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!PARSING!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//if (skipCurrentModel==false) {
		drawModel(objNum, origin, 0, 0);
	//}
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	
	/*
	p_localvar[0]=localvar[0];
	p_localvar[1]=localvar[1];
	p_localvar[2]=localvar[2];
	p_localvar[3]=localvar[3];
	p_localvar[4]=localvar[4];
	p_localvar[5]=localvar[5];
	p_localvar[6]=localvar[6];
*/
	// Возвращаем локальные переменные модели на место. Их позже точно так-же
	// обработает оригинальный код на асме

	if (!((currentModel>=113 && currentModel<=136) || currentModel==445 || currentModel==447 || currentModel==449)) {
		localvar[0]=t_localvar[0];
		localvar[1]=t_localvar[1];
		localvar[2]=t_localvar[2];
		localvar[3]=t_localvar[3];
		localvar[4]=t_localvar[4];
		localvar[5]=t_localvar[5];
		localvar[6]=t_localvar[6];
	}

	*(unsigned int*)DATA_007824 = t_DATA_007824;

	if (exportb[currentModel]==true) {
		if (ExportMesh(currentModel)) {
			exportb[currentModel]=false;
		}
	}

	// rotate patches
	D3DXMATRIX rot;
	D3DXMatrixIdentity(&rot);

	if (objNum==166) { // starmap grid
		//D3DXMatrixRotationX(&mainRotMatrix, 90*(3.14f/180));	
		//D3DXMatrixIdentity(&mainRotMatrix);
	}
	if (objNum==170) { // purple circle
		//D3DXMatrixRotationX(&rot, 3.14f/2.0f);		
	}
	if (objNum==173) { // nebula
		//D3DXMatrixRotationY(&rot, 3.14f/2.0f);		
	}
	//if (currentModel<3 && (*(int *)(gptr+0xFC)==1902 || *(int *)(gptr+0xFC)==0xbe271aa0)) {
	//	D3DXMatrixRotationY(&rot, 3.14f);
		//posMatrix[14]*=-1;
	//}
	//D3DXMatrixMultiply(&mainRotMatrix, &rot, &mainRotMatrix);
	// end rotate patches

	// effects rotation
	if (objNum==329 || objNum==330 || objNum==331 || objNum==332) {
		D3DXMatrixRotationZ(&rot,  (float)(posMatrix[12] + posMatrix[13] + posMatrix[14])/50);
	}
	D3DXMatrixMultiply(&mainRotMatrix, &mainRotMatrix, &rot);
	// end effects rotation

	// position patches
	D3DXMATRIX posit;
	D3DXMatrixIdentity(&posit);

	D3DXMatrixMultiply(&posMatrix, &posit, &posMatrix);
	// end position patches

	if (currentModel>=117 && currentModel<=148) {
		lastPlanetRadius=(float)(*(mod->Vertices_ptr+2)<<currentScale)/DIVIDER;
	}


	if (subObject==true && currentModel>=3) {
		Vect3f ncorr = getNormal(2);
		posMatrix[12]*=1.001;
		posMatrix[13]*=1.001;
		posMatrix[14]*=1.001;
	}
	if (currentModel==387 || currentModel==388) {
		posMatrix[13]-=0.01;
	}
	if (currentModel==371) {
		posMatrix[13]+=0.01;
	}

	//if (skipCurrentModel==false) {
	//	D3DXMatrixIdentity(&modelList[modelNum].world);
	//	modelList[modelNum].world[12]=posMatrix[12];
	//	modelList[modelNum].world[13]=posMatrix[13];
	//	modelList[modelNum].world[14]=posMatrix[14];
	//} else {
		D3DXMatrixMultiply(&modelList[modelNum].world, &mainRotMatrix, &posMatrix);
	//}
	D3DXMatrixScaling(&matScale, scale2, scale2, scale2);
	D3DXMatrixMultiply(&modelList[modelNum].world, &matScale, &modelList[modelNum].world);

	modelList[modelNum].doMatrix=1;
	modelList[modelNum].vertEnd=vertexNum;
	modelList[modelNum].backsprite=false;

	modelList[modelNum].dist=dis;

	previousModel=currentModel;

	modelNum++;

	// Это планета. Добавим атмосферу (если в радиусе, то до планеты).	
	if ((currentModel>=125 && currentModel<=132) || currentModel==449) {
		radius2=(float)(*(mod->Vertices_ptr+2)<<currentScale)/DIVIDER;
		if (dis > radius2*1.04f) {
			DrawAtmosphere(ptr, 1.04f);
		}
	}


	D3DXMatrixIdentity(&currentMatrix);
	return 1;
}

extern "C" void C_Break2() {
	currentAmbientR=15;
	currentAmbientG=15;
	currentAmbientB=15;
}

D3DXMATRIX tempRotMatrix;
unsigned char *objInstance;

void mMatrix(unsigned char* inst) {		
	D3DXVECTOR3 a, b, c;

	a=D3DXVECTOR3((float)*(int*)(inst+0x0)/DIVIDER,
				  (float)*(int*)(inst+0x4)/DIVIDER,
				  (float)*(int*)(inst+0x8)/DIVIDER);
	b=D3DXVECTOR3((float)*(int*)(inst+0xc)/DIVIDER,
				  (float)*(int*)(inst+0x10)/DIVIDER,
				  (float)*(int*)(inst+0x14)/DIVIDER);
	c=D3DXVECTOR3((float)*(int*)(inst+0x18)/DIVIDER,
				  (float)*(int*)(inst+0x1c)/DIVIDER,
				  (float)*(int*)(inst+0x20)/DIVIDER);
				  
	D3DXVec3Normalize(&a, &a);
	D3DXVec3Normalize(&b, &b);
	D3DXVec3Normalize(&c, &c);

	tempRotMatrix[0]=a.x;
	tempRotMatrix[1]=a.y;
	tempRotMatrix[2]=a.z;

	tempRotMatrix[4]=b.x;
	tempRotMatrix[5]=b.y;
	tempRotMatrix[6]=b.z;

	tempRotMatrix[8]=c.x;
	tempRotMatrix[9]=c.y;
	tempRotMatrix[10]=c.z;

	tempRotMatrix[12]=0.0f;
	tempRotMatrix[13]=0.0f;
	tempRotMatrix[14]=0.0f;

	tempRotMatrix[3]=0.0f;
	tempRotMatrix[7]=0.0f;
	tempRotMatrix[11]=0.0f;
	tempRotMatrix[15]=1.0f;

	D3DXMatrixMultiply(&lightRotMatrix, &lightRotMatrix, &tempRotMatrix);
}


extern "C" void C_Break3(unsigned char *ptr) {
	int objNum;
	D3DXVECTOR3 a, b, c;
	D3DXVECTOR3 v, v2;

	unsigned char *tmpInstance;
	unsigned char prevIndex;
	unsigned char *plptr=(unsigned char*)*(unsigned int*)DATA_008861;
	unsigned int* oiList=(unsigned int*)*instanceList;
	if (playerLightPosEnable==false)
		D3DXMatrixIdentity(&lightRotMatrix);	
	objNum=*(unsigned short *)(ptr+0x82);

	if (objNum>=138 && objNum<=148) {
		if (playerLightPosEnable==false) {
			v2.x=-(double)*(__int64 *)(plptr+0x3e)/DIVIDER;
			v2.y=-(double)*(__int64 *)(plptr+0x46)/DIVIDER;
			v2.z=-(double)*(__int64 *)(plptr+0x4e)/DIVIDER;
			
			objInstance = plptr;
			//mMatrix();
			while ((prevIndex = *((unsigned char*)objInstance+0x56))!=0) {				
				prevIndex = *((unsigned char*)objInstance+0x56);
				objInstance = FUNC_001532_GetModelInstancePtr (prevIndex, oiList);
				v.x=-(double)*(__int64 *)(objInstance+0x3e)/DIVIDER;
				v.y=-(double)*(__int64 *)(objInstance+0x46)/DIVIDER;
				v.z=-(double)*(__int64 *)(objInstance+0x4e)/DIVIDER;
				v2.x+=v.x;
				v2.y+=v.y;
				v2.z+=v.z;
				//mMatrix();
			}
			//
			D3DXVec3Normalize(&v2, &v2);
			playerLightPos.x=v2.x*100000;
			playerLightPos.y=v2.y*100000;
			playerLightPos.z=v2.z*100000;

			//prevIndex = *((unsigned char*)plptr+0x56);
			//plptr = FUNC_001532_GetModelInstancePtr (prevIndex, oiList);
			mMatrix(plptr);
			//D3DXVec3Normalize(&playerLightPos, &playerLightPos);

			D3DXVECTOR4 out;
					
			D3DXMatrixInverse(&lightRotMatrix,NULL, &lightRotMatrix);
			D3DXVec3Transform(&out,&playerLightPos, &lightRotMatrix);
			playerLightPos.x=out.x;
			playerLightPos.y=out.y;
			playerLightPos.z=out.z;
			playerLightPosEnable=true;
		}

		// Нам бы цвет света звезды задать
		currentAmbientR=starColors[objNum-138][0];
		currentAmbientG=starColors[objNum-138][1];
		currentAmbientB=starColors[objNum-138][2];
		fullAmbientColor=(currentAmbientR*17)<<16;
		fullAmbientColor|=(currentAmbientG*17)<<8;
		fullAmbientColor|=(currentAmbientB*17);

	}

}

extern "C" void C_Break4() {
	int a=0;
}


