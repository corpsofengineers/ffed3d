#include <windows.h>
#include <ddraw.h>
#include <stdio.h>
#include <math.h>
#include <direct.h>
#include <d3dx9.h>
#include "win32api.h"

#include "ffe3d.h"
#include "planet.h"
#include "Render/RenderSystem.h"
#include "xmath.h"

void Vec32to64 (__int64 *dst, int *src) {
	dst[0] = (__int64)src[0];
	dst[1] = (__int64)src[1];
	dst[2] = (__int64)src[2];
}

extern "C" void C_PlaceStation (void *starport, int lat, int lon, void *objectList)
{
	__int64 vVector[3]; // один из трех векторов матрицы планеты
	__int64 relPos[3]; // вектор из центра планеты
	int dist; // дистанция
	int scaleFactor;
	int radius;
	unsigned char *planet;
	Model_t *planetModel;
	unsigned char prevIndex;
	unsigned short modelNum;

	// Получаем указатель на предыдущий объект старпорта, т.е. планету
	prevIndex = *((unsigned char*)starport+0x56);
    planet = FUNC_001532_GetModelInstancePtr (prevIndex, objectList);
	modelNum = *(unsigned short *)((char *)planet+0x82);
    planetModel = FUNC_001538_GetModelPtr ((int)modelNum);    

	// field_2C он же unknown_2
	radius = planetModel->field_2C - 1;                // parent "radius"
	scaleFactor = planetModel->Scale + planetModel->Scale2 - 8;

	// Старпорт расположен в координатах
	// Vec1 = 0
	// Vec2 = 2147483637 (0x7FFFFFF5) // почему не 0x7FFFFFFF? Учитываем размеры старпорта?
	// Vec3 = 0
	// Предположительно, это максимально возможная позиция вверх (по Y)

	// Передвигает координаты старпорта по широте и долготе
    FUNC_001674_MatBuildOdd (starport, lat, lon);

	// Переводим 32-битные координаты старпорта в 64-битные
    Vec32to64 (&vVector[0], (int*)starport+3);

	// Часть А, снижаем старпорт до радиуса планеты
	// relPos - позиция старпорта относительно центра планеты


	relPos[0] = vVector[0]*radius;
	relPos[1] = vVector[1]*radius;
	relPos[2] = vVector[2]*radius;

	FUNC_001341_Int64ArithShift(&relPos[0], scaleFactor-0x1f);
	FUNC_001341_Int64ArithShift(&relPos[1], scaleFactor-0x1f);
	FUNC_001341_Int64ArithShift(&relPos[2], scaleFactor-0x1f);

	// Конец части А

	// Проверяем, на поверхности ли старпорт
	// Gets distance between two objects
    FUNC_000574 (&dist, &relPos[0], starport, planet);    // void F574 (int *distance, Vec64 *relpos, PhysObj *obj1, PhysObj *obj2)

	// Часть В, корректируем координаты
    dist -= (dist >> 4);            // FIX: change from >> 4 to >> 3
	if (scaleFactor < 0) {
		radius -= (dist << -scaleFactor);
	} else if (scaleFactor < 0x10) { 
		radius = (radius << scaleFactor) - dist; 
		scaleFactor = 0; 
	} else { 
		scaleFactor -= 0x10; 
		radius = (radius << 0x10) - (dist >> scaleFactor);
	}

	relPos[0] = vVector[0] * radius;
	relPos[1] = vVector[1] * radius;
	relPos[2] = vVector[2] * radius;

	FUNC_001341_Int64ArithShift(&relPos[0], scaleFactor-0x1f);
	FUNC_001341_Int64ArithShift(&relPos[1], scaleFactor-0x1f);
	FUNC_001341_Int64ArithShift(&relPos[2], scaleFactor-0x1f);

	// Конец части В

	// Опять проверяем, на поверхности ли старпорт
	// Gets distance between two objects
    FUNC_000574 (&dist, &relPos[0], starport, planet);    // void F574 (int *distance, Vec64 *relpos, PhysObj *obj1, PhysObj *obj2)

	// Часть С, последние корректировки координат
	if (scaleFactor < 0) {
		radius -= (dist << -scaleFactor);
	} else {
		radius -= (dist >> scaleFactor);
	}

	relPos[0] = vVector[0] * radius;
	relPos[1] = vVector[1] * radius;
	relPos[2] = vVector[2] * radius;

	FUNC_001341_Int64ArithShift(&relPos[0], scaleFactor-0x1f);
	FUNC_001341_Int64ArithShift(&relPos[1], scaleFactor-0x1f);
	FUNC_001341_Int64ArithShift(&relPos[2], scaleFactor-0x1f);

    memcpy ((char*)starport+0x3e, &relPos[0], 3*8 ); 
}

// Randomizer
unsigned int C_FUNC_001824_Random(unsigned int A8)
{
	unsigned int edx;

    edx = ((A8 >> 27) | (A8 << 5)) + A8;
    return A8 + ((edx >> 16) | (edx << 16));
}


int fix31 (__int64 in) {
	return (int)(in >> 31);
}

// Multiple with normalize
int C_FUNC_001521_MulWithNormalize(int A8, int AC)
{
	return FUNC_001521(A8, AC);
	/*
		mov eax,[ebp+0x8]
		imul dword [ebp+0xc]
		shl eax,1
		rcl edx,1
		mov eax,edx
	*/
	/*
	__int64 res;

	res = (__int64)A8 * AC;
	//res = res >> 31;
	res /= 0x7FFF8000; // 2147450880
	return (int)res;
	*/
	
}

// how many bits used
unsigned int C_FUNC_001656_FindMSB(unsigned int A8)
{
	unsigned int eax, edx;

    eax = A8;
    if(eax >= 65536) {
		eax /= 65535;
        edx = 16;
    } else {
        edx = 0;
    }
    if(eax >= 256) {
		eax /= 256;
        edx = edx + 8;
    }
    if(eax >= 16) {
		eax /= 16;
        edx = edx + 4;
    }
    if(eax >= 4) {
		eax /= 4;
        edx = edx + 2;
    }
    if(eax >= 2) {
        return edx + 1;
    }
    return edx;
}


int C_FUNC_001825_MaxUsedBitsInXYZ(int *A8)
{
	int x, y, z;
	unsigned int w;
	
	x = *A8;
	y = *(A8+1);
	z = *(A8+2);

	w = abs(x) | abs(y) | abs(z);

    return C_FUNC_001656_FindMSB(w);
}

//C_FUNC_001479_doOrtoXY((int*)&CurrentPlanetArray[counter * 2].orto_x, (ffeVector*)&CurrentPlanetArray[counter * 2].nx);

int tricounter=0;
int segmcounter=0;
// ortogonation?

#define w_orto 160
#define h_orto (80-1)

extern "C" int C_FUNC_001479_doOrtoXY(int *orto_xy, ffeVector *vertex)
{
	int eax, ecx, edx;

	//if(vertex->z < 64) {
    //    return 1;
    //}
	if(vertex->z > 65536) {
		ecx = w_orto + vertex->x / (vertex->z >> 8); // 0xa0 == 160
		eax = h_orto - vertex->y / (vertex->z >> 8); // 0x4f == 79
    } else {
		ecx = w_orto + (vertex->x << 8) / vertex->z; // (a << 8) == (a / 256)
		eax = h_orto - (vertex->y << 8) / vertex->z;
    }

	edx = abs(ecx) | abs(eax);
    //while (edx > 0x1f00) { // 7936
    //    ecx = ecx / 2;
    //    eax = eax / 2;
    //    edx = edx / 2;
    //}
	
	*orto_xy = (eax<<16) | ecx;


/*
	tessVerts[tricounter].x=(float)vertex->x / DIVIDER;
	tessVerts[tricounter].y=(float)vertex->y / DIVIDER;
	tessVerts[tricounter].z=(float)vertex->z / DIVIDER;

	///////////////////////
	//tessVerts[0].n.x=(float)A8->normal_x / DIVIDER;
	//tessVerts[0].n.y=(float)A8->normal_y / DIVIDER;
	//tessVerts[0].n.z=(float)A8->normal_z / DIVIDER;
	//D3DXVec3Normalize(&tessVerts[0].n, &tessVerts[0].n);

	tessVerts[tricounter].color=0x0f0f0f0f;

	tricounter++;

	if (tricounter==3) {
		tessVerts[8].x=tessVerts[1].x;
		tessVerts[8].y=tessVerts[1].y;
		tessVerts[8].z=tessVerts[1].z;
		tessVerts[9].x=tessVerts[2].x;
		tessVerts[9].y=tessVerts[2].y;
		tessVerts[9].z=tessVerts[2].z;
	}

	if (tricounter==6) {
		DrawCustomTriangle2(&tessVerts[5],&tessVerts[2],&tessVerts[1]);
		DrawCustomTriangle2(&tessVerts[5],&tessVerts[1],&tessVerts[4]);
		tessVerts[1].x=tessVerts[4].x;
		tessVerts[1].y=tessVerts[4].y;
		tessVerts[1].z=tessVerts[4].z;
		tessVerts[2].x=tessVerts[5].x;
		tessVerts[2].y=tessVerts[5].y;
		tessVerts[2].z=tessVerts[5].z;
		tricounter=3;
		segmcounter++;
	}
	if (segmcounter==31) {
		DrawCustomTriangle2(&tessVerts[9],&tessVerts[2],&tessVerts[1]);
		DrawCustomTriangle2(&tessVerts[9],&tessVerts[1],&tessVerts[8]);
	}
*/
    return 0;
}

extern "C" int FUNC_001876(int, int, int, int, int, int, int, int, int, int, int, int);
int C_FUNC_001876(int A8, int Ac, char *A10_ModelInstance_t, char *A14_DrawModel_t, int A18, ffeVector *CurrentPlanetPosition, ffePoint *CurrentPlanetMatrix, int A24_extra1, int A28_extra3, int A2c_extra2, int A30_rgbColor, int A34_flag);

extern "C" int FUNC_001874_DrawPlanet(unsigned char *DrawMdl, char *cmd)
{
	short *command;

	command = (short*)cmd;

	command+=6;
	while(1) {
		if (*(command)==0x0000) {
			command++;
			break;
		}
		command+=8;
	}

	return (int)command;
}

ffeVertex *vertex1ptr, *vertex2ptr, *vertex3ptr, *originptr;
ffePoint center; 
int diam;

int C_FUNC_001874_DrawPlanet(char *DrawMdl, char *cmd)
{
	char modOrigin, modv1, modv2, modv3;

	// All vertexes returns transformated

	modOrigin = *(cmd + 2);
	if (DATA_009200[modOrigin/2].unknown6 == *(DrawMdl + 0xf0)) {
		originptr = &DATA_009200[modOrigin/2];
	} else {
		originptr = FUNC_001470_getVertex(DrawMdl, modOrigin);
	}

	modv1 = *(cmd + 3);
	if (DATA_009200[modv1/2].unknown6 != 0) {
		vertex1ptr = &DATA_009200[modv1/2];
	} else {
		vertex1ptr = FUNC_001472(DrawMdl, modv1);
	}

	modv2 = *(cmd + 4);
	if (DATA_009200[modv2/2].unknown6 != 0) {
		vertex2ptr = &DATA_009200[modv2/2];
	} else {
		vertex2ptr = FUNC_001472(DrawMdl, modv2);
	}

	modv3 = *(cmd + 5);
	if (DATA_009200[modv3/2].unknown6 != 0) {
		vertex3ptr = &DATA_009200[modv3/2];
	} else {
		vertex3ptr = FUNC_001472(DrawMdl, modv3);
	}

	center.v1x = (vertex1ptr->x - originptr->x);
	center.v1y = (vertex1ptr->y - originptr->y);
	center.v1z = (vertex1ptr->z - originptr->z);
	center.v2x = (vertex2ptr->x - originptr->x);
	center.v2y = (vertex2ptr->y - originptr->y);
	center.v2z = (vertex2ptr->z - originptr->z);
	center.v3x = (vertex3ptr->x - originptr->x);
	center.v3y = (vertex3ptr->y - originptr->y);
	center.v3z = (vertex3ptr->z - originptr->z);

	//return FUNC_001876(0, 0, *(int *)(DrawMdl + 0x14c), (int)DrawMdl, (int)(cmd + 0xc), (int)&originptr->nx, (int)&center.v1x, (int)*(short *)(cmd + 6), (int)*(short *)(cmd + 10), (int)(*(short *)(cmd + 8) << 0x10), (int)*(short*)(cmd), 0);
    return C_FUNC_001876(0, 0, (char *)*(int *)(DrawMdl + 0x14c), (char *)DrawMdl, (int)(cmd + 0xc), (ffeVector *)&originptr->nx, (ffePoint *)&center.v1x, (int)*(short *)(cmd + 6), (int)*(short *)(cmd + 10), (int)(*(short *)(cmd + 8) << 0x10), (int)*(short*)(cmd), 0);
}

extern int vertexNum;
extern sVertexBuffer vertexBuffer;
extern VERTEXTYPE vertexType[MAXVERT];

extern void newIndex(Vect3f *curpoint);

VertexXYZNDT1* Vertices;

//D3DXVECTOR3 vNormal, t1, t2, t3, r1, r2;

D3DXVECTOR3 CalculationTangent( CUSTOMVERTEX *A, CUSTOMVERTEX *B, CUSTOMVERTEX *C)
{
    D3DXVECTOR3 vAB = B->p - A->p;
    D3DXVECTOR3 vAC = C->p - A->p;
    D3DXVECTOR3 n  = A->n;

    D3DXVECTOR3 vProjAB = vAB - (D3DXVec3Dot(&n, &vAB) * n);
    D3DXVECTOR3 vProjAC = vAC - (D3DXVec3Dot(&n, &vAC) * n);

    FLOAT duAB = B->tu - A->tu;
    FLOAT duAC = C->tu - A->tu;
    FLOAT dvAB = B->tv - A->tv;
    FLOAT dvAC = C->tv - A->tv;

    if( duAC*dvAB > duAB*dvAC )
    {
        duAC = -duAC;
        duAB = -duAB;
    }

    D3DXVECTOR3 vTangent = duAC*vProjAB - duAB*vProjAC;
    D3DXVec3Normalize( &vTangent, &vTangent );

    return vTangent;
}

int etex=94;
void inline DrawCustomTriangle2(CUSTOMVERTEX *p1, CUSTOMVERTEX *p2, CUSTOMVERTEX *p3)
{
	Color4c color;

	short r, g, b;
	
	if (vertexNum>MAXVERT-100)
		return;

	Vect3f curPoint;

	int colMult;
	int colAdd;
	int colDiv;

	D3DXVECTOR3 tangent = CalculationTangent(p1, p2, p3);

	Vertices = (VertexXYZNDT1*)renderSystem->LockVertexBuffer(vertexBuffer,vertexNum,3*vertexBuffer.vertexSize);
/*
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
*/
	switch(currentModel) {
		case 119: vertexType[vertexNum].textNum = 719; break;
		case 120: vertexType[vertexNum].textNum = 711; break;
		case 121: vertexType[vertexNum].textNum = 719; break;
		case 122: vertexType[vertexNum].textNum = 711; break;
		case 123: vertexType[vertexNum].textNum = 712; break;
		case 124: vertexType[vertexNum].textNum = 712; break;
		case 125: vertexType[vertexNum].textNum = etex; break;
		case 126: vertexType[vertexNum].textNum = etex; break;
		case 127: vertexType[vertexNum].textNum = etex; break;
		case 128: vertexType[vertexNum].textNum = etex; break;
		case 129: vertexType[vertexNum].textNum = etex; break;
		case 130: vertexType[vertexNum].textNum = etex; break;
		case 131: vertexType[vertexNum].textNum = etex; break;
		case 132: vertexType[vertexNum].textNum = etex; break;
		case 133: vertexType[vertexNum].textNum = 714; break;
		case 134: vertexType[vertexNum].textNum = 715; break;
		case 135: vertexType[vertexNum].textNum = 718; break;
		case 136: vertexType[vertexNum].textNum = 715; break;
		case 445: vertexType[vertexNum].textNum = 718; break; 
		case 447: vertexType[vertexNum].textNum = etex; break;
		case 449: vertexType[vertexNum].textNum = etex; break;
		default: vertexType[vertexNum].textNum = 719; break;
	}

	if (vertexType[vertexNum].textNum!=etex) {
		colMult=32;
		colAdd=100;
		colDiv=2;
	} else {
		colMult=24;
		colAdd=50;
		colDiv=1;
	}

	b=(p1->color&0x00FF0000)>>16;
	g=(p1->color&0x0000FF00)>>8;
	r=(p1->color&0x000000FF);
	color.set(r*colMult/colDiv+colAdd, g*colMult/colDiv+colAdd, b*colMult/colDiv+colAdd);
	if (b>r*2 && b>g*2 && vertexType[vertexNum].textNum == etex) {
		vertexType[vertexNum].specular=true;
	} else {
		vertexType[vertexNum].specular=false;
	}

	vertexType[vertexNum].type = GL_TRIANGLES;
	vertexType[vertexNum].amount = 3;
	vertexType[vertexNum].doLight=true;
	vertexType[vertexNum].transparent=false;
	vertexType[vertexNum].tangent = tangent;
	
	Vertices->pos.x = (p1->p.x-originptr->nx) / DIVIDER;
	Vertices->pos.y = (p1->p.y-originptr->ny) / DIVIDER;
	Vertices->pos.z = (p1->p.z-originptr->nz) / DIVIDER;
	Vertices->difuse = color;
	Vertices->u1() = p1->tu;
	Vertices->v1() = p1->tv;
	Vertices->n.x = p1->n.x;
	Vertices->n.y = p1->n.y;
	Vertices->n.z = p1->n.z;

	Vertices++;
	vertexNum++;

	b=(p2->color&0x00FF0000)>>16;
	g=(p2->color&0x0000FF00)>>8;
	r=(p2->color&0x000000FF);
	color.set(r*colMult/colDiv+colAdd, g*colMult/colDiv+colAdd, b*colMult/colDiv+colAdd);

	Vertices->pos.x = (p2->p.x-originptr->nx) / DIVIDER;
	Vertices->pos.y = (p2->p.y-originptr->ny) / DIVIDER;
	Vertices->pos.z = (p2->p.z-originptr->nz) / DIVIDER;
	Vertices->difuse = color;
	Vertices->u1() = p2->tu;
	Vertices->v1() = p2->tv;
	Vertices->n.x = p2->n.x;
	Vertices->n.y = p2->n.y;
	Vertices->n.z = p2->n.z;

	Vertices++;
	vertexNum++;

	b=(p3->color&0x00FF0000)>>16;
	g=(p3->color&0x0000FF00)>>8;
	r=(p3->color&0x000000FF);
	color.set(r*colMult/colDiv+colAdd, g*colMult/colDiv+colAdd, b*colMult/colDiv+colAdd);

	Vertices->pos.x = (p3->p.x-originptr->nx) / DIVIDER;
	Vertices->pos.y = (p3->p.y-originptr->ny) / DIVIDER;
	Vertices->pos.z = (p3->p.z-originptr->nz) / DIVIDER;
	Vertices->difuse = color;
	Vertices->u1() = p3->tu;
	Vertices->v1() = p3->tv;
	Vertices->n.x = p3->n.x;
	Vertices->n.y = p3->n.y;
	Vertices->n.z = p3->n.z;

	Vertices++;
	vertexNum++;

	renderSystem->UnlockVertexBuffer(vertexBuffer);
}

#define FFE_PI 3.141592654
#define FFE_TWOPI 6.283185308
#define M_1_PI 0.318309886183790671538

void SphereMap2(double x,double y,double z,float &u,float &v)
{
	D3DXVECTOR3 p;
	D3DXMATRIX tmpMatrix,tmpMatrix2;

	p.x=x/DIVIDER;
	p.y=y/DIVIDER;
	p.z=z/DIVIDER;
	D3DXVec3Normalize(&p, &p);
	
	D3DXMatrixInverse(&tmpMatrix,NULL, &mainRotMatrixO);
	D3DXVec3TransformCoord(&p, &p, &tmpMatrix);

	u = asinf(p.x)/D3DX_PI+0.5f; 
	v = 1.0f-(asinf(p.y)/D3DX_PI+0.5f);
}

inline int classify_point2D(ffeVertex *p0, ffeVertex *p1, ffeVertex *p2) {
	return (p1->orto_x-p0->orto_x)*(p2->orto_y-p0->orto_y) - (p2->orto_x-p0->orto_x)*(p1->orto_y-p0->orto_y);
}

void DrawRealPoint(float *p1, unsigned int radius);
void DrawRealTriangle(float *p1, float *p2, float *p3, int type, int normal);
void DrawCustomTriangle(Vect3f *p1, Vect3f *p2, Vect3f *p3, Vect3f *n1, Vect3f *n2, Vect3f *n3, int color1, int color2, int color3);

int C_FUNC_001823(int *A8, short *Ac);

int C_FUNC_001876(int A8, int Ac, char *A10_ModelInstance_t, char *A14_DrawModel_t, int A18, ffeVector *CurrentPlanetPosition, ffePoint *CurrentPlanetMatrix, int A24_extra1, int A28_extra3, int A2c_extra2, int A30_rgbColor, int A34_flag)
//  A8; // 0
//  Ac; // 0
//  A10_ModelInstance_t;
//  A14_DrawModel_t;
//  A18; // pointer to 0807 000C 0040 00F4 FFB3 FFFF 00CE FFFF ; 7,8,12, 0x0040,0x00F4,0xFFB3,0xFFFF,0x00CE,0xFFFF
//  CurrentPlanetPosition; // origin?
//  CurrentPlanetMatrix; // vertex array pointer?
//  A24_extra1; // *(ebx + 6) /Cmd 1Fh(RGB(6,6,7), 6,2,4,0, -->0x0000<--,0x0B85,0x0478/
//  A28_extra3; // radius factor? /Cmd 1Fh(RGB(6,6,7), 6,2,4,0, 0x0000,0x0B85,-->0x0478<--/
//  A2c_extra2; // *(ebx + 8) << 16 /Cmd 1Fh(RGB(6,6,7), 6,2,4,0, 0x0000,-->0x0B85<--,0x0478/
//  A30_rgbColor; // rgb color
//  A34_flag; // flag?
{
	double x,y,z,w;
	int unique_Id; // ebp - 4
	int dist1; // &ebp_8
	int minPlanetRadius; // ebp - 12
	int scaleFactor; // ebp - 16
	int ebp_20; // ebp - 20
	int ebp_24; // ebp - 24
	int ebp_28; // ebp - 28
	int ebp_32; // ebp - 32
	int dist3; // ebp - 52
	UnknownStruct1 faces[20]; // ebp - 636  4*120
	ffeVertex vertices[12]; // ebp - 2076
	int array2[60]; // ebp - 2316  4*60
	ffeVertex** edges[30]; // ebp - 2436 // Массив указателей на вершины
	int eax, ebx;
	

	memcpy (&faces[0], DATA_007893, 4*6*20);
	memcpy (&array2[0], DATA_007894, 4*60);

	unique_Id = *(int*)(A10_ModelInstance_t+0xa0);

	if (A34_flag != 0) { 
		// ... not need for call from FUNC_001874
		*(int*)DATA_009283 = 0x7FFFFFFF; // 2147483647
		*(int*)DATA_009285 = Ac;
		*(int*)DATA_009284 = 16384; // 0x4000
	} else {
		scaleFactor = *(int*)(A14_DrawModel_t+0x148) - 8; 
		if (scaleFactor < 0) {
			A28_extra3 = A28_extra3 >> -scaleFactor;
		} else {
			A28_extra3 = A28_extra3 << scaleFactor;
		}
		diam = A28_extra3;
		//FUNC_001692(&ebp_48[0], A30_rgbColor);
		// Prepare texture?
		//FUNC_001691(DATA_009259, &ebp_48[0], &ebp_48[1], &ebp_48[2], &ebp_48[3]); 
		// Atmosphere?
		tricounter=0;
		segmcounter=0;
		//FUNC_001877(A14_DrawModel_t, (int*)&CurrentPlanetPosition->x, (int*)&CurrentPlanetMatrix->v1x, *(int*)*(int*)DATA_009259, A2c_extra2, A28_extra3);

	}

	x = CurrentPlanetMatrix->v1x;
	y = CurrentPlanetMatrix->v1y;
	z = CurrentPlanetMatrix->v1z;
	w = x*x+y*y+z*z;	
	minPlanetRadius = (int)sqrt(w);

	x = CurrentPlanetMatrix->v2x;
	y = CurrentPlanetMatrix->v2y;
	z = CurrentPlanetMatrix->v2z;
	w = x*x+y*y+z*z;	
	dist1 = (int)sqrt(w);

	minPlanetRadius = min(dist1, minPlanetRadius);

	x = CurrentPlanetMatrix->v3x;
	y = CurrentPlanetMatrix->v3y;
	z = CurrentPlanetMatrix->v3z;
	w = x*x+y*y+z*z;	
	dist1 = (int)sqrt(w);

	minPlanetRadius = min(dist1, minPlanetRadius);

	dist1 = C_FUNC_001521_MulWithNormalize(minPlanetRadius, 0x54000000);
	dist1 *=2;

	if(A34_flag == 0 && currentModel==449) { //
		*(int*)DATA_007837 = *(int*)DATA_008812_GraphicsDetailRelated - 1;

		x = CurrentPlanetPosition->x;
		y = CurrentPlanetPosition->y;
		z = CurrentPlanetPosition->z;
		w = x*x+y*y+z*z;	
		dist3 = (int)sqrt(w);

		if((minPlanetRadius >> 4) + minPlanetRadius < dist3) {
			*(int*)DATA_007837 = *(int*)DATA_007837 + 1;
			//*(int*)DATA_007837 = *(int*)DATA_007837 - 1;
			if((minPlanetRadius >> 3) + minPlanetRadius < dist3) {
				*(int*)DATA_007837 = *(int*)DATA_007837 + 1;
				//*(int*)DATA_007837 = *(int*)DATA_007837 - 1;
			}
		}

	} else { //
		// ... not need for call from FUNC_001874
		// DATA_008812_GraphicsDetailRelated - graphics detail related - -1/0/1
		*(int*)DATA_007837 = *(int*)DATA_008812_GraphicsDetailRelated - 1;
	}
	// set level of detail
	//*(int*)DATA_009269 = C_FUNC_001656_FindMSB(dist1) + *(int*)DATA_007837;
	*(int*)DATA_009269 = *(int*)DATA_007837;

//	if (C_FUNC_001656_FindMSB(dist1)>=8)
//		*(int*)DATA_009269 -= 10-C_FUNC_001656_FindMSB(abs(CurrentPlanetPosition->z-CurrentPlanetMatrix->v1z));
	
	if (currentModel>=125 && currentModel<=132) { // earth type
		*(int*)DATA_009269 += max_divide_deep;
	} else if (currentModel==445) { // gas giant intro
		*(int*)DATA_009269 += 3;
	} else if (currentModel==447) { // earth intro #1
		*(int*)DATA_009269 = C_FUNC_001656_FindMSB(dist1) + *(int*)DATA_007837 + 4;
	} else if (currentModel==449) { // earth intro #2
		*(int*)DATA_009269 = C_FUNC_001656_FindMSB(dist1) + *(int*)DATA_007837 + 3;
	} else { // simple type
		*(int*)DATA_009269 += max_divide_deep;
	}

	int counter = 0;
	ffeVector *icosahedron = (ffeVector *)DATA_007892_Icosahedron; // икосаэдр
	__int64 mv_x, mv_y, mv_z; // matrix vector column
	ffeMatrix m;

	m._11 = CurrentPlanetMatrix->v1x;
	m._12 = CurrentPlanetMatrix->v1y;
	m._13 = CurrentPlanetMatrix->v1z;
	m._21 = CurrentPlanetMatrix->v2x;
	m._22 = CurrentPlanetMatrix->v2y;
	m._23 = CurrentPlanetMatrix->v2z;
	m._31 = CurrentPlanetMatrix->v3x;
	m._32 = CurrentPlanetMatrix->v3y;
	m._33 = CurrentPlanetMatrix->v3z;

	do { //
		mv_x = icosahedron[counter].x;
		mv_y = icosahedron[counter].y;
		mv_z = icosahedron[counter].z;

		// multiple icosahedron on planet matrix?
		vertices[counter].nx = (int)(fix31(mv_x * m._11) + fix31(mv_y * m._21) + fix31(mv_z * m._31));
		vertices[counter].ny = (int)(fix31(mv_x * m._12) + fix31(mv_y * m._22) + fix31(mv_z * m._32));
		vertices[counter].nz = (int)(fix31(mv_x * m._13) + fix31(mv_y * m._23) + fix31(mv_z * m._33));

		vertices[counter].normal_x = vertices[counter].nx;
		vertices[counter].normal_y = vertices[counter].ny;
		vertices[counter].normal_z = vertices[counter].nz;

		vertices[counter].nx = vertices[counter].nx + CurrentPlanetPosition->x;
		vertices[counter].ny = vertices[counter].ny + CurrentPlanetPosition->y;
		vertices[counter].nz = vertices[counter].nz + CurrentPlanetPosition->z;

		if(A34_flag != 1) { //
			// calculate orto xy
			C_FUNC_001479_doOrtoXY((int*)&vertices[counter].orto_x, (ffeVector*)&vertices[counter].nx);
			//FUNC_001479((int*)&CurrentPlanetArray[counter].orto_x, (int*)&CurrentPlanetArray[counter].nx);
		}

		// get pseudo random value
		unique_Id = C_FUNC_001824_Random(unique_Id);

		// A24_extra1 - planet type?
		int aaa=*((int*)DATA_007843 + A24_extra1*12);

		if ((int*)aaa != FUNC_001840_addr) {
			// mars type
			vertices[counter].x_2 = (icosahedron[counter].y >> 5) + 0x4000000; // 67108864
		} else {
			// earth type
			vertices[counter].x_2 = unique_Id >> 4;
		}

		counter = counter + 1;

	} while (counter < 12);

	A18 = FUNC_001823(&ebp_20, (short *)A18);

	ebp_24 = CurrentPlanetPosition->z > minPlanetRadius ? 1 : 0; // Находимся в атмосфере планеты
	eax = 0;//SetJmp(DATA_009271);
	ebp_32 = eax;
	if(ebp_32 != 0) {
		FUNC_001817();  // *DATA_009272=0;
		eax = A18;
	} else {
		C_FUNC_001816_ArrayInit();
		ebx = 0;
		// hmmm... index buffer?
		do {
			edges[ebx] = C_FUNC_001820_ArrayCreateNewElement();//(unsigned int)C_FUNC_001820_ArrayCreateNewElement();
			eax = array2[ebx*2];
			*edges[ebx] = &vertices[eax];
			ebx = ebx + 1;
		} while(ebx < 30);

		ebp_28 = C_FUNC_001833(A14_DrawModel_t, dist1, A24_extra1, A34_flag);


		if(A34_flag == 1) { //
			// ... not need for call from FUNC_001874
		} else { //
//			Vect3f normal;
			ebx = 0;
			maxTessVerts=0;
			do {		
					C_FUNC_001835(&vertices[faces[ebx].var1], 
								  &vertices[faces[ebx].var2], 
								  &vertices[faces[ebx].var3], 
								  edges[faces[ebx].var4],
								  edges[faces[ebx].var5],
								  edges[faces[ebx].var6],
								  dist1, 
								  *(int*)((int*)ebp_20 + ebx + 2), 
								  ebp_24, 
								  ebp_28, 
								  A34_flag);
				//}
				ebx = ebx + 1;
			} while(ebx < 20);
		}

		//FUNC_001817(); // *DATA_009272=0;
		ebx = 0;
		do {
			C_FUNC_001821_ArrayRemoveElement(edges[ebx]);//(unsigned int)C_FUNC_001820_ArrayCreateNewElement();
			ebx = ebx + 1;
		} while(ebx < 30);

		if(A34_flag != 0) { //
			// ... not need for call from FUNC_001874
		}
		eax = A18;
	}

	return eax;
}

void C_FUNC_001835(ffeVertex* A8, ffeVertex* Ac, ffeVertex* A10, ffeVertex** A14, ffeVertex** A18, ffeVertex** A1c, int A20, int A24, int A28, int A2c, int A30)
{
	int eax;

    C_FUNC_001834(A8, Ac, A10, A24, A28, A2c);

	eax = A30 - 1;

	C_FUNC_001869(A8, Ac, A10, A14, A18, A1c, 0, A20, A24);
	/*
    if(!((char)A2c & 1) && A2c > 8) {
		if (eax < 0) {
			C_FUNC_001869((ffeVertex*)A8, (ffeVertex*)Ac, (ffeVertex*)A10, A14, A18, A1c, 0, A20, A24);
		} else if (eax == 0) {
			//FUNC_001871(A8, Ac, A10, A14, A18, A1c, 0, A20, A24);
		} else {
			eax--;
			if (eax==0) {
				//FUNC_001873(A8, Ac, A10, A14, A18, A1c, 0, A20, A24);
			}
		}
    } else {
		if (eax < 0) {
			C_FUNC_001869((ffeVertex*)A8, (ffeVertex*)Ac, (ffeVertex*)A10, A14, A18, A1c, 0, A20, A24);
		} else if (eax == 0) {
			//FUNC_001870(A8, Ac, A10, A14, A18, A1c, 0, A20, A24);
		} else {
			eax--;
			if (eax==0) {
				//FUNC_001872(A8, Ac, A10, A14, A18, A1c, 0, A20, A24);
			}
		}
    }
	*/
}

int C_FUNC_001823(int *A8, short *Ac)
// void  A8; // result
// void  Ac; // // pointer to 0807 000C 0040 00F4 FFB3 FFFF 00CE FFFF ; 7,8,12, 0x0040,0x00F4,0xFFB3,0xFFFF,0x00CE,0xFFFF
{
    char	*local3; // ebp - 0x14				(int*)DATA_009258
    int		local4;  // &ebp_10				0x58 - counter

    local4 = 0x58; // 88
    local3 = DATA_009258; // DATA_009258 - array 4096 bytes
/*
	// *-- Обнуление
    edx = 0;
    eax = local3;
    do {
        *(int*)eax = 0;
        edx = edx + 1;
        eax = eax + 4;
    } while(edx < 0x16);
	// --*
    
    local7 = *(short *)Ac; // pointer to 0807 000C 0040 00F4 FFB3 FFFF 00CE FFFF ; 7,8,12, 0x0040,0x00F4,0xFFB3,0xFFFF,0x00CE,0xFFFF
	while(local7 != 0) {
        local5 = *(short *)(Ac + 2);
        local2 = FUNC_001656_FindMSB(local7);
		local7 = (local7 << (char)(16 - local2)) & 0xffff;
        ebx = local3; // (int*)DATA_009258
        local2 = (int)ceil((float)(local2 - 3) / 2);
        while(local2 > 0) {
            local6 = local7 >> 0xb; // 11 - color?
            local1 = ebx;
            ebx = (char*)*(int*)(local1 + local6*4 + 8); //(ebx+color+2)?
            if(ebx == 0) {
                ebx = (char*)(DATA_009258 + local4);
                local4 = local4 + 0x18; // 24

				// *-- Обнуление
                edx = 0;
                eax = ebx + 8;
                do {
                    *(int*)eax = 0;
                    edx = edx + 1;
                    eax = eax + 4;
                } while(edx < 4);
				// --*

				// *-- Заполнение
                edx = 0;
                ecx = ebx;
                eax = local1;
                do {
                    esi = *(int*)eax;
					if (esi > 0) {
						*(int*)ecx = -( *(int*)(DATA_007842 + (esi + esi * 2)*4) - 1);
					} else {
						if (esi>=0) {
							esi = 0;
							*(int*)ecx = 0;
						} else {
							esi = esi + 1;
							*(int*)ecx = esi;
						}
                    }
                    edx = edx + 1;
                    ecx = ecx + 4;
                    eax = eax + 4;
                } while(edx < 2);
				// --*

                *(int*)(local1 + local6*4 + 8) = (int)ebx;
            }
            local7 = (local7 << 2) & 0x1fff;
            local2 = local2 - 1;
        };
        edx = 0;
        eax = ebx;
        do {
            if(*(int*)eax == 0) {
                break;
            }
            edx = edx + 1;
            eax = eax + 4;
        } while(edx < 2);

        if(edx < 2) {
            eax = (DATA_009258 + local4);
            local4 = local4 + 0x10; // 16   next line of data?
            *(int*)(ebx + edx*4) = (int)eax;
            *(int*)eax = local5;
            *(short*)(eax + 4) = *(short*)(Ac + 4);
            *(short*)(eax + 6) = *(short*)(Ac + 6);
            *(short*)(eax + 8) = *(short*)(Ac + 8);
            *(short*)(eax + 0xa) = *(short*)(Ac + 10);
            *(short*)(eax + 0xc) = *(short*)(Ac + 12);
            *(short*)(eax + 0xe) = *(short*)(Ac + 14);
        }
        Ac = Ac + 16;
        local7 = *(short*)Ac;
    }//while;
	*/
    *A8 = (int)local3;
    return (int)(Ac + 2);
}

// Инициализация массивов
extern "C" void C_FUNC_001816_ArrayInit(void)
{
	int		eax;
    int		ebx;
	int		edx;
	int		ecx;

    ecx = (int)&C_DATA_009274[0];
    ebx = (int)&C_DATA_009277[0];
    if(*(int*)DATA_009272 == 0) {
        *(int*)DATA_009272 = ecx;
        eax = 0;
        edx = ecx;

		do {
			*(int*)edx = edx + 72;

            eax = eax + 1;
            edx = edx + 72;
        } while(eax < MAX_PLANET_VERT/2 - 1 ); // 1499

        *(int*)(ecx + (MAX_PLANET_VERT/2 - 1) * 72) = 0; //0x1a598
        *(int*)DATA_009275 = ebx; // указатель на текущий пустой элемент массива
        eax = 0;
        edx = ebx;

		do {
			*(int*)edx = edx + 16;
            eax = eax + 1;
            edx = edx + 16;
        } while(eax < MAX_PLANET_VERT - 1); //0xbb7, 3000

        *(int*)(ebx + (MAX_PLANET_VERT - 1) * 16) = 0; //0xbb70
        *(int*)DATA_009273 = 0;
        eax = 0;
        *(int*)DATA_009276_ArraySize = 0;
    }
}

int C_FUNC_001818_ArrayRemoveFirstElement() // Удалить вершину из начала буфера?
{
	//return FUNC_001818();

	int eax, ebx;

	ebx = *(int*)DATA_009272;
	*(int*)DATA_009273=*(int*)DATA_009273 + 1;
	//if (*(int*)DATA_009273 < 1500) {
		//push byte +0x1
		//push dword DATA_009271
		//call _LongJmp
		//add esp,byte +0x8
	//}
	eax = *(int*)DATA_009272;
	eax = *(int*)eax;
	*(int*)DATA_009272 = eax;
	return ebx;
	
}

void C_FUNC_001819_ArrayInsertFirstElement(int A8) // Вставить вершину в начало буфера?
{
	int eax, edx;

	eax = A8;
	*(int*)DATA_009273 = *(int*)DATA_009273 - 1;
	edx = *(int*)DATA_009272;
	*(int*)eax = edx;
	*(int*)DATA_009272 = eax;
}

// Создание нового элемента массива вершин?
ffeVertex** C_FUNC_001820_ArrayCreateNewElement(void)
{
	int		eax;
    ffeVertex** ebx;

	eax = *(int*)DATA_009275; // Получили указатель на пустой эллемент массива
    ebx = (ffeVertex**)eax; // его функция и будет возвращать
    *(int*)DATA_009275 = *(int*)eax; // Теперь переменная будет хранить указатель на следующий пустой элемент массива
    *(int*)DATA_009276_ArraySize = *(int*)DATA_009276_ArraySize + 1; // Увеличиваем колличество вершин
    if(*(int*)DATA_009276_ArraySize >= 3000) { // 3000 вершин максимум?
        //LongJmp((int*)DATA_009271, 2);
    }
    ebx[1] = 0;
    ebx[2] = 0;
    ebx[3] = 0;
    return (ffeVertex**)ebx;
}

//Удаление эллемента массива вершина
int C_FUNC_001821_ArrayRemoveElement(ffeVertex** A8)
{
    ffeVertex** ebx;
	int eax;

    ebx = A8;
    do {
        eax = (int)ebx[1];
        if(eax != 0) { // если это не последний эллемент массива, сначала удалим все, что после него
            C_FUNC_001821_ArrayRemoveElement((ffeVertex**)eax);
        }
        eax = (int)ebx[2];
        if(eax != 0) { // хз что делает, потом посмотрю
            FUNC_001819(eax);
        }

        *(int*)DATA_009276_ArraySize = *(int*)DATA_009276_ArraySize - 1; // уменьшение количества вершин

        eax = *(int*)DATA_009275; // Указатель на последний(пустой) эллемент массива
        *ebx = (ffeVertex*)eax; // Теперь указатель будет указывает на следущий эллемент массива (т.е. будет пустым)
        *(int*)DATA_009275 = (int)ebx; // Корректируем указатель на пустой эллемент массива
        ebx = (ffeVertex **)ebx[3]; /// ???
    } while(ebx != 0);

	return 0;
}

char* (*shiftCall)(int);

extern "C" void FUNC_001831(int);
extern "C" void FUNC_001832(int);
extern "C" void FUNC_001848(int);

extern "C" void FUNC_001715(int);
extern "C" void FUNC_001716(int);
extern "C" void FUNC_001847(int);

int C_FUNC_001833(char *DrawModel_t, int dist, int extra1, int flag)
{
    int		ebx;
	int		ecx;



    ebx = (extra1 << 4) + (extra1 << 4) * 2 + (int)DATA_007843;
    if(flag != 0) {
        *(int*)DATA_009280 = (int)FUNC_001831;
        *(int*)DATA_009281 = (int)FUNC_001832;
        *(int*)DATA_009282 = (int)FUNC_001848;
    } else {
        *(int*)DATA_009280 = (int)FUNC_001715;
        *(int*)DATA_009281 = (int)FUNC_001716;
        *(int*)DATA_009282 = (int)FUNC_001847;
    }
    ecx = (int)DrawModel_t != 0 ? *(int*)(DrawModel_t + 0x130) : 0; // alt_Scale
    *(int*)DATA_009264 = 500000000 >> (unsigned char)ecx;
    *(int*)DATA_009270 = *(int*)ebx;
	shiftCall = (char *(*)(int)) *(int*)((char*)ebx + 4); // pointer to func
    shiftCall(ebx);
    *(int*)DATA_009261 = (int)DrawModel_t;
    return *(int*)((char*)ebx + 8);
}

int (*shiftCall2) (short);
/*
A8		(int*)&CurrentPlanetArray[array1[ebx].var1 * 2].orto_x 
Ac		(int*)&CurrentPlanetArray[array1[ebx].var2 * 2].orto_x 
A10		(int*)&CurrentPlanetArray[array1[ebx].var3 * 2].orto_x 
A14		*(int*)((int*)ebp_20 + ebx + 2) 
A18		ebp_24 
A1c		ebp_28 
*/
int funcNum;

void C_FUNC_001834(ffeVertex *A8, ffeVertex *Ac, ffeVertex *A10, int A14, int A18, int A1c)
{
	int		eax;

    eax = A1c;
    if(A14 != 0) {
        eax = eax + 8;
    }
    if(A18 == 0) {
        eax = eax + 2;
    }
	if (eax > 27)
		eax = 27;
    *(int*)DATA_009279 = *(int*)(eax * 4 + DATA_007844);
    *(int*)DATA_009278 = *(int*)(eax * 4 + DATA_007845);
	funcNum = eax;

    //C_FUNC_001829_GetTriangulateDepth(A8);
	C_FUNC_001829_GetTriangulateDepth_2(A8, A8, Ac, A10);

	shiftCall2 = (int(*)(short)) *(int*)DATA_009280;
	A8->orto_x_2 = (short)shiftCall2((short)*(int*)A8);

	shiftCall2 = (int(*)(short)) *(int*)DATA_009281;
	A8->orto_y_2 = (short)shiftCall2((short)*((char*)A8 + 2));

    //C_FUNC_001829_GetTriangulateDepth(Ac);
	C_FUNC_001829_GetTriangulateDepth_2(Ac, A8, Ac, A10);

	shiftCall2 = (int(*)(short)) *(int*)DATA_009280;
	Ac->orto_x_2 = (short)shiftCall2((short)*(int*)Ac);

	shiftCall2 = (int(*)(short)) *(int*)DATA_009281;
	Ac->orto_y_2 = (short)shiftCall2((short)*((char*)Ac + 2));

	//C_FUNC_001829_GetTriangulateDepth(A10);
	C_FUNC_001829_GetTriangulateDepth_2(A10, A8, Ac, A10);

	shiftCall2 = (int(*)(short)) *(int*)DATA_009280;
	A10->orto_x_2 = (short)shiftCall2((short)*(int*)A10);

	shiftCall2 = (int(*)(short)) *(int*)DATA_009281;
	A10->orto_y_2 = (short)shiftCall2((short)*((char*)A10 + 2));
/*
    if(A14 != 0) {
        local3 = 0;
        local0 = A14;
        do {
            if(*(int*)local0 > 0) {
                eax = *(int*)local0;
                local1 = ( *(int*)eax << 2) + ( *(int*)eax << 2) * 2 + (int)DATA_007841;
                eax = eax + 4;
                if(*(int*)eax < 0x80 && *(int*)(eax + 2) < 0x80) {
                    edx = *(int*)eax;
                    local2 = edx + ( *(int*)(eax + 2) << 7);
                    *(int*)((char*)A8 + 0x3c) = *(int*)((char*)A8 + 0x3c) >> 7;
                    if(*(int*)(local1 + 4) != 0) {
                        edx = *(int*)( *(int*)(local1 + 4) + local2) & 0xff;
                        *(int*)((char*)A8 + 0x3c) = *(int*)((char*)A8 + 0x3c) * *(int*)((char*)A8 + 0x3c);
                    }
                    *(int*)((char*)A8 + 0x3c) = *(int*)((char*)A8 + 0x3c) + ( *(int*)( *(int*)local1 + local2) << 0x15);
                }
                if(*(int*)(eax + 4) < 0x80 && *(int*)(eax + 6) < 0x80) {
                    edx = *(int*)(eax + 4);
                    local2 = edx + ( *(int*)(eax + 6) << 7);
                    *(int*)((char*)Ac + 0x3c) = *(int*)((char*)Ac + 0x3c) >> 7;
                    if(*(int*)(local1 + 4) != 0) {
                        edx = *(int*)( *(int*)(local1 + 4) + local2) & 0xff;
                        *(int*)((char*)Ac + 0x3c) = *(int*)((char*)Ac + 0x3c) * *(int*)((char*)Ac + 0x3c);
                    }
                    *(int*)((char*)Ac + 0x3c) = *(int*)((char*)Ac + 0x3c) + ( *(int*)( *(int*)local1 + local2) << 0x15);
                }
                if(*(int*)(eax + 8) < 0x80 && *(int*)(eax + 0xa) < 0x80) {
                    edx = *(int*)(eax + 8);
                    local2 = edx + ( *(int*)(eax + 0xa) << 7);
                    *(int*)((char*)A10 + 0x3c) = *(int*)((char*)A10 + 0x3c) >> 7;
                    eax = *(int*)(local1 + 4);
                    if(eax != 0) {
                        eax = *(int*)(eax + local2) & 0xff;
                        *(int*)((char*)A10 + 0x3c) = *(int*)((char*)A10 + 0x3c) * *(int*)((char*)A10 + 0x3c);
                    }
                    *(int*)((char*)A10 + 0x3c) = *(int*)((char*)A10 + 0x3c) + ( *(int*)( *(int*)local1 + local2) << 0x15);
                }
            }
            local3 = local3 + 1;
            local0 = local0 + 4;
        } while(local3 < 2);
    }
*/	
	A8->y_2 = A8->x_2 < 0 ? 0 : A8->x_2;
	Ac->y_2 = Ac->x_2 < 0 ? 0 : Ac->x_2;
	A10->y_2 = A10->x_2 < 0 ? 0 : A10->x_2;

}

int (*shiftCall3)(int*,int*,int*,int*,int);

/*
A8				(int*)&CurrentPlanetArray[array1[ebx].var1 * 2].orto_x, 
Ac				(int*)&CurrentPlanetArray[array1[ebx].var2 * 2].orto_x, 
A10				(int*)&CurrentPlanetArray[array1[ebx].var3 * 2].orto_x, 
A14				array3[array1[ebx].var4],
A18				array3[array1[ebx].var5],
A1c				array3[array1[ebx].var6],
A20				0
A24(dist)		dist1, 
A28				*(int*)((int*)ebp_20 + ebx + 2), 
*/

void callTriangleDeliver(int* A8, ffeVertex* Ac, ffeVertex* A10, ffeVertex* A14, int A18, int num);
inline int GetColor(ffeVertex* V, int type);
int C_FUNC_001840(ffeVertex *A8, ffeVertex* Ac, ffeVertex* A10, ffeVector* A14, int A18);

inline void C_FUNC_001869(ffeVertex *A8, ffeVertex *Ac, ffeVertex *A10, ffeVertex **A14, ffeVertex **A18, ffeVertex **A1c, int A20, int dist, int A28)
{



    //esp = esp + -148;

	int		eax;
	int		ecx;
	int		edx;
	int		esi;
	int		edi;

	ffeVertex**	ebp_8;
	ffeVertex**	ebp_c;
	ffeVertex**	ebp_10;
	int		ebp_1c;
	int		distToVertex;
	int		ebp_24;
	int		ebp_30[3];
	int		ebp_38[3];
	int		ebp_40[3];
	int		ebp_48;
	int		ebp_50;
	int		ebp_54;
	int		ebp_58;
	int		ebp_5c;
	int		ebp_60;
	int		ebp_64;
	int		ebp_68;
	int		ebp_6c;
	int		ebp_70;
	int		ebp_74;
	int		ebp_78;
	int		ebp_7c;
	ffeVector	newVertexXYZ;
	ffeVector	normalizedTo13BitsXYZ;

	int		temp1;			

	//if (A8->z_2 < 6 && Ac->z_2 < 6 && A10->z_2 < 6 && classify_point2D(A8, Ac, A10) >= 7000)
	//	return 0;

	//if (A20 > 2 && classify_point2D(A8, Ac, A10) > 3000)
	//	return 0;

    esi = A20;
    if(esi < 10) {
        edi = dist;
    } else {
		if(Ac->y_2 > A8->y_2) {
			ebp_24 = A8->y_2;
			edi = Ac->y_2;
        } else {
			ebp_24 = Ac->y_2;
			edi = A8->y_2;
        }
		if(A10->y_2 < ebp_24) {
			ebp_24 = A10->y_2;
        } else {
			if(edi < A10->y_2) {
				edi = A10->y_2;
            }
        }
		temp1 = (edi - ebp_24) >> 3;
		edi = abs(A8->normal_x) + abs(A8->normal_y) + abs(A8->normal_z);
        edi = C_FUNC_001521_MulWithNormalize(edi, temp1) + dist;
    }

	eax = A8->nz + edi;
    ebp_1c = eax;
	if(ebp_1c >= 64) 
	{
        eax = abs(A8->nx) - edi;
		if(eax <= ebp_1c) 
		{
            eax = abs(A8->ny) - edi;
			if(eax <= ebp_1c) 
			{
                if(A8->nz < 64 || Ac->nz < 64 || A10->nz < 64) {
                    goto L00468c26;
                }
/*
				// Определение ориентации треугольника (classify point 2d)
				ebp_44 = (Ac->orto_y - A8->orto_y) * (A10->orto_x - A8->orto_x)
					- (Ac->orto_x - A8->orto_x) * (A10->orto_y - A8->orto_y);

*/				
				{ // фикс. затыкает злостный баг, но надо искать истинную причину
					int big,sml;
					if(Ac->nz > A8->nz) {
						big = Ac->nz;
						sml = A8->nz;
					} else {
						big = A8->nz;
						sml = Ac->nz;
					}
					if(A10->nz > big) {
						big = A10->nz;
					} else {
						if(A10->nz < sml) {
							sml = A10->nz;
						}
					}

					if (big - sml > dist*2)
						return;
				}

				eax = (A10->x_2 <= 0) + (Ac->x_2 <= 0) + (A8->x_2 <= 0);

				// зависимость, ровная ли поверхность?
				if ( eax && eax != 3 )
					eax = esi - 1;
				else
					eax = esi;

                if(esi <= 10) {
					// Если еще остались треугольники которые надо поделить...
					if (eax <= A8->z_2 || eax <= Ac->z_2 || eax <= A10->z_2) {
						// До этой глубины деления делим, иначе рисуем
						if (eax <= 8)
							goto L00468ad1;
					}
				}
				//if (classify_point2D(A8, Ac, A10) > 100)
				//	return;
/*
				// Попробуем нормальный фруструм сделать
				if (A8->orto_x < -64 && Ac->orto_x < -64 && A10->orto_x < -64)
					return;
				if (A8->orto_x > 320+64 && Ac->orto_x > 320+64 && A10->orto_x > 320+64)
					return;
				if (A8->orto_y < -64 && Ac->orto_y < -64 && A10->orto_y < -64)
					return;
				if (A8->orto_y > 160+64 && Ac->orto_y > 160+64 && A10->orto_y > 160+64)
					return;

				if (A8->nz <= -100 && Ac->nz <= -100 && A10->nz <= -100)
					return;				
*/

				//if (A8->nz <= 200 || Ac->nz <= 200 || A10->nz <= 200) {
					//if (A8->orto_y>100)
						//A8->ny-=10000;
					//if (Ac->orto_y>100)
						//Ac->ny-=10000;
					//if (A10->orto_y>100)
						//A10->ny-=10000;
				//}

				eax =	Ac->orto_x_2 | A8->orto_x_2 | A10->orto_x_2;

				if(*(int*)(eax * 4 + (int)DATA_007838) == 0) {

					eax =	Ac->orto_y_2 | A8->orto_y_2 | A10->orto_y_2;

					if(*(int*)(eax * 4 + DATA_007838) == 0) {
				        eax = (A10->x_2 <= 0) + (Ac->x_2 <= 0) + (A8->x_2 <= 0);
						ebp_48 = (A10->x_2 <= 0) + (Ac->x_2 <= 0) + (A8->x_2 <= 0);

						if(1) {//ebp_48 == 1 || ebp_48 == 2 || ebp_44 > 0) {
							
							//newVertexXYZ.x = A8->nx;
							//newVertexXYZ.y = A8->ny;
							//newVertexXYZ.z = A8->nz;
                            //ebp_80 = (int)DATA_009264 + ebp_80;
							//newVertexXYZ.z += *(int*)DATA_009264;
							//ebp_4c = FUNC_001777((int*)*(int*)DATA_009261, (ffeVector*)&newVertexXYZ.x);
                            //*(int*)DATA_009286 = esi;
							//C_FUNC_001827_CalculateNewVertexXYZ((ffeVector*)&newVertexXYZ.x, (ffeVector*)&A8->nx, (ffeVector*)&Ac->nx, (ffeVector*)&A10->nx);
							
							shiftCall3 = (int (*)(int*,int*,int*,int*,int)) *(int*)DATA_009270;
							
							/*							
							eax = shiftCall3((int*)A8,
											 (int*)Ac, 
											 (int*)A10, 
											 (int*)&newVertexXYZ, 
											 ebp_4c);
							*/
							//return eax;

							// Тут была 2D отрисовка
							/*
							eax = C_FUNC_001840(A8,
											 Ac, 
											 A10, 
											 (ffeVector *)&newVertexXYZ, 
											 ebp_4c);
							*/
			
							ffeVertex *V1, *V2, *V3;
							int vn1, vn2, vn3;


							if ((currentModel>=125 && currentModel<=132) || currentModel==449) {
								int vv1, vv2, vv3;
								int eax;

								//old_A8 = A8;
								//old_Ac = Ac;
								//old_A10 = A10;

								// A8 > Ac > A10
								/*
								if(Ac->x_2 > A8->x_2) {
									V = A8;
									A8 = Ac;
									Ac = V;
								}
								if(A10->x_2 > A8->x_2) {
									V = A8;
									A8 = A10;
									A10 = V;
								}

								if(Ac->x_2 < A10->x_2) {
									V = Ac;
									Ac = A10;
									A10 = V;
								}
								*/
								if(A8->x_2 > Ac->x_2) {
									V1 = A8;
									V2 = Ac;
									vn1 = 0;
									vn2 = 1;
								} else {
									V1 = Ac;
									V2 = A8;
									vn1 = 1;
									vn2 = 0;
								}
								if(A10->x_2 > V1->x_2) {
									V3 = V2;
									V2 = V1;
									V1 = A10;
									vn3 = vn2;
									vn2 = vn1;
									vn1 = 2;
								} else if (A10->x_2 < V2->x_2) {
									V3 = A10;
									vn3 = 2;
								} else {
									V3 = V2;
									V2 = A10;
									vn3 = vn2;
									vn2 = 2;
								}


								eax = (A8->x_2<=0) + (Ac->x_2<=0) + (A10->x_2<=0) - 1;

								etex=0;
								if (eax > 0) {
									//if (eax-1 > 0) {
										if (V1->x_2 >= 0x400000) {
											/*
											*(int*)(esi + 0x10) = 0;
											*(int*)(esi + 0x12) = 0;
											*(int*)(esi + 0x14) = 0x3f; // 63
											*(int*)(esi + 0x16) = 0;
											*(int*)(esi + 0x18) = 0;
											*(int*)(esi + 0x1a) = (edi[1].x >> 0x12) & 65535;
											if(*(int*)(esi + 0x1a) > 0x1ff) {
												*(int*)(esi + 0x1a) = 0x1ff; // 511
											}
											*/
											tessVerts[vn1].tu=0;
											tessVerts[vn1].tv=0;
											tessVerts[vn2].tu=1;
											tessVerts[vn2].tv=0;
											tessVerts[vn3].tu=0;
												vv3 = (V1->x_2 >> 0x12) & 65535;
												vv3 = vv3 > 0x1ff ? 0x1ff : vv3;
											tessVerts[vn3].tv=1.0f/511*vv3;

											etex=94;
										} else { // вода
											
											//tessVerts[0].tu=0;
											//tessVerts[0].tv=0;
											//tessVerts[1].tu=1;
											//tessVerts[1].tv=0;
											//tessVerts[2].tu=1;
											//tessVerts[2].tv=1;
											
											SphereMap2(V1->nx-originptr->nx, V1->ny-originptr->ny, V1->nz-originptr->nz, tessVerts[vn1].tu, tessVerts[vn1].tv);
											SphereMap2(V2->nx-originptr->nx, V2->ny-originptr->ny, V2->nz-originptr->nz, tessVerts[vn2].tu, tessVerts[vn2].tv);
											SphereMap2(V3->nx-originptr->nx, V3->ny-originptr->ny, V3->nz-originptr->nz, tessVerts[vn3].tu, tessVerts[vn3].tv);
											tessVerts[vn1].tu*=1000;
											tessVerts[vn1].tv*=1000;
											tessVerts[vn2].tu*=1000;
											tessVerts[vn2].tv*=1000;
											tessVerts[vn3].tu*=1000;
											tessVerts[vn3].tv*=1000;
											//tessVerts[vn1].tu+=*DATA_008804/600000.0f;
											//tessVerts[vn1].tv+=*DATA_008804/600000.0f;
											//tessVerts[vn2].tu+=*DATA_008804/600000.0f;
											//tessVerts[vn2].tv+=*DATA_008804/600000.0f;
											//tessVerts[vn3].tu+=*DATA_008804/600000.0f;
											//tessVerts[vn3].tv+=*DATA_008804/600000.0f;
											

											etex=720;
											
										}
									//}
								} else if (eax == 0) {
										if (V1->x_2 >= 0x400000) {
											/*
											*(int*)(local1 + 0x14) = 0;
											*(int*)(local1 + 0x16) = (edi[1].x >> 0x12) & 65535;
											*(int*)(local1 + 0x18) = 0x3f;
											*(int*)(local1 + 0x1a) = Ac[1].x >> 0x12;
											*(int*)(local1 + 0x1c) = 0x3f;
											*(int*)(local1 + 0x1e) = 0;
											*(int*)(local1 + 0x20) = 0;
											*(int*)(local1 + 0x22) = 0;
											if(*(int*)(local1 + 0x16) > 0x1ff) {
												*(int*)(local1 + 0x16) = 0x1ff;
											}
											if(*(int*)(local1 + 0x1a) > 0x1ff) {
												*(int*)(local1 + 0x1a) = 0x1ff;
											}
											*/
											tessVerts[vn1].tu=0;
												vv1 = (V1->x_2 >> 0x12) & 65535;
												vv1 = vv1 > 0x1ff ? 0x1ff : vv1;
											tessVerts[vn1].tv=1.0f/511*vv1;
											tessVerts[vn2].tu=1;
												vv2 = (V2->x_2 >> 0x12);
												vv2 = vv2 > 0x1ff ? 0x1ff : vv2;
											tessVerts[vn2].tv=1.0f/511*vv2;
											tessVerts[vn3].tu=1;
											tessVerts[vn3].tv=0;

											etex=94;
										} else { 
											//tessVerts[vn1].tu=0;
											//tessVerts[vn1].tv=0;
											//tessVerts[vn2].tu=1;
											//tessVerts[vn2].tv=1;
											//tessVerts[vn3].tu=0;
											//tessVerts[vn3].tv=1;

											SphereMap2(V1->nx-originptr->nx, V1->ny-originptr->ny, V1->nz-originptr->nz, tessVerts[vn1].tu, tessVerts[vn1].tv);
											SphereMap2(V2->nx-originptr->nx, V2->ny-originptr->ny, V2->nz-originptr->nz, tessVerts[vn2].tu, tessVerts[vn2].tv);
											SphereMap2(V3->nx-originptr->nx, V3->ny-originptr->ny, V3->nz-originptr->nz, tessVerts[vn3].tu, tessVerts[vn3].tv);
											tessVerts[vn1].tu*=1000;
											tessVerts[vn1].tv*=1000;
											tessVerts[vn2].tu*=1000;
											tessVerts[vn2].tv*=1000;
											tessVerts[vn3].tu*=1000;
											tessVerts[vn3].tv*=1000;

											etex=64;
										}
								} else {
									if (V1->x_2 - V3->x_2 >= 0x400000) {
										/*
										*(int*)(esi + 0x10) = 0;
										*(int*)(esi + 0x12) = (edi[1].x >> 0x12) & 65535;
										*(int*)(esi + 0x14) = 0x3f; // 63
										*(int*)(esi + 0x16) = (Ac[1].x >> 0x12) & 65535;
										*(int*)(esi + 0x18) = 0;
										*(int*)(esi + 0x1a) = (A10[1].x >> 0x12);
										if(*(int*)(esi + 0x12) > 0x1ff) {
											*(int*)(esi + 0x12) = 0x1ff;
										}
										if(*(int*)(esi + 0x16) > 0x1ff) {
											*(int*)(esi + 0x16) = 0x1ff;
										}
										if(*(int*)(esi + 0x1a) > 0x1ff) {
											*(int*)(esi + 0x1a) = 0x1ff;
										}
										*/
										tessVerts[vn1].tu=0;
											vv1 = (V1->x_2 >> 0x12) & 65535;
											vv1 = vv1 > 0x1ff ? 0x1ff : vv1;
										tessVerts[vn1].tv=1.0f/511*vv1;
										tessVerts[vn2].tu=1;
											vv2 = (V2->x_2 >> 0x12) & 65535;
											vv2 = vv2 > 0x1ff ? 0x1ff : vv2;
										tessVerts[vn2].tv=1.0f/511*vv2;
										tessVerts[vn3].tu=0;
											vv3 = (V3->x_2 >> 0x12);
											vv3 = vv3 > 0x1ff ? 0x1ff : vv3;
										tessVerts[vn3].tv=1.0f/511*vv3;

										etex=94;
									} else { // ровная поверхность
										//if ((A8->x_2 >> 0x12) & 65535 < 0x1ff) {
											//tessVerts[vn1].tu=0;
											//tessVerts[vn1].tv=0;
											//tessVerts[vn2].tu=1;
											//tessVerts[vn2].tv=0;
											//tessVerts[vn3].tu=1;
											//tessVerts[vn3].tv=1;

											SphereMap2(V1->nx-originptr->nx, V1->ny-originptr->ny, V1->nz-originptr->nz, tessVerts[vn1].tu, tessVerts[vn1].tv);
											SphereMap2(V2->nx-originptr->nx, V2->ny-originptr->ny, V2->nz-originptr->nz, tessVerts[vn2].tu, tessVerts[vn2].tv);
											SphereMap2(V3->nx-originptr->nx, V3->ny-originptr->ny, V3->nz-originptr->nz, tessVerts[vn3].tu, tessVerts[vn3].tv);
											tessVerts[vn1].tu*=1000;
											tessVerts[vn1].tv*=1000;
											tessVerts[vn2].tu*=1000;
											tessVerts[vn2].tv*=1000;
											tessVerts[vn3].tu*=1000;
											tessVerts[vn3].tv*=1000;

											etex=64;
										//}
									}
								}
								//A8 = old_A8;
								//Ac = old_Ac;
								//A10 = old_A10;
							} else {

								SphereMap2(A8->nx-originptr->nx, A8->ny-originptr->ny, A8->nz-originptr->nz, tessVerts[0].tu, tessVerts[0].tv);
								SphereMap2(Ac->nx-originptr->nx, Ac->ny-originptr->ny, Ac->nz-originptr->nz, tessVerts[1].tu, tessVerts[1].tv);
								SphereMap2(A10->nx-originptr->nx, A10->ny-originptr->ny, A10->nz-originptr->nz, tessVerts[2].tu, tessVerts[2].tv);																

								if (abs(tessVerts[0].tu-tessVerts[1].tu) >= 0.5 && abs(tessVerts[0].tu-tessVerts[2].tu) >= 0.5) {
									if (tessVerts[0].tu>=0.5)
										tessVerts[0].tu-=1;
									else
										tessVerts[0].tu+=1;
								}
								if (abs(tessVerts[1].tu-tessVerts[0].tu) >= 0.5 && abs(tessVerts[1].tu-tessVerts[2].tu) >= 0.5) {
									if (tessVerts[1].tu>=0.5)
										tessVerts[1].tu-=1;
									else
										tessVerts[1].tu+=1;
								}
								if (abs(tessVerts[2].tu-tessVerts[0].tu) >= 0.5 && abs(tessVerts[2].tu-tessVerts[1].tu) >= 0.5) {
									if (tessVerts[2].tu>=0.5)
										tessVerts[2].tu-=1;
									else
										tessVerts[2].tu+=1;
								}

								if (abs(tessVerts[0].tv-tessVerts[1].tv) >= 0.5 && abs(tessVerts[0].tv-tessVerts[2].tv) >= 0.5) {
									if (tessVerts[0].tv>=0.5)
										tessVerts[0].tv-=1;
									else
										tessVerts[0].tv+=1;
								}
								if (abs(tessVerts[1].tv-tessVerts[0].tv) >= 0.5 && abs(tessVerts[1].tv-tessVerts[2].tv) >= 0.5) {
									if (tessVerts[1].tv>=0.5)
										tessVerts[1].tv-=1;
									else
										tessVerts[1].tv+=1;
								}
								if (abs(tessVerts[2].tv-tessVerts[0].tv) >= 0.5 && abs(tessVerts[2].tv-tessVerts[1].tv) >= 0.5) {
									if (tessVerts[2].tv>=0.5)
										tessVerts[2].tv-=1;
									else
										tessVerts[2].tv+=1;
								}
							}

							//SphereMap2((A8->nx-originptr->nx), (A8->ny-originptr->ny), (A8->nz-originptr->nz), tessVerts[0].tu, tessVerts[0].tv);
							//SphereMap2((Ac->nx-originptr->nx), (Ac->ny-originptr->ny), (Ac->nz-originptr->nz), tessVerts[1].tu, tessVerts[1].tv);
							//SphereMap2((A10->nx-originptr->nx), (A10->ny-originptr->ny), (A10->nz-originptr->nz), tessVerts[2].tu, tessVerts[2].tv);								
							//tessVerts[0].tu*=1000;
							//tessVerts[0].tv*=1000;
							//tessVerts[1].tu*=1000;
							//tessVerts[1].tv*=1000;
							//tessVerts[2].tu*=1000;
							//tessVerts[2].tv*=1000;
							//etex == 720;

							tessVerts[0].p.x=(float)(A8->nx);
							tessVerts[0].p.y=(float)(A8->ny);
							tessVerts[0].p.z=(float)(A8->nz);

							tessVerts[1].p.x=(float)(Ac->nx);
							tessVerts[1].p.y=(float)(Ac->ny);
							tessVerts[1].p.z=(float)(Ac->nz);

							tessVerts[2].p.x=(float)(A10->nx);
							tessVerts[2].p.y=(float)(A10->ny);
							tessVerts[2].p.z=(float)(A10->nz);
							///////////////////////

							tessVerts[0].n.x=(float)A8->normal_x / DIVIDER;
							tessVerts[0].n.y=(float)A8->normal_y / DIVIDER;
							tessVerts[0].n.z=(float)A8->normal_z / DIVIDER;
							D3DXVec3Normalize(&tessVerts[0].n, &tessVerts[0].n);

							tessVerts[1].n.x=(float)Ac->normal_x / DIVIDER;
							tessVerts[1].n.y=(float)Ac->normal_y / DIVIDER;
							tessVerts[1].n.z=(float)Ac->normal_z / DIVIDER;
							D3DXVec3Normalize(&tessVerts[1].n, &tessVerts[1].n);

							tessVerts[2].n.x=(float)A10->normal_x / DIVIDER;
							tessVerts[2].n.y=(float)A10->normal_y / DIVIDER;
							tessVerts[2].n.z=(float)A10->normal_z / DIVIDER;
							D3DXVec3Normalize(&tessVerts[2].n, &tessVerts[2].n);

							if ((int)shiftCall3==*(int*)DATA_007843) {
								tessVerts[0].color=GetColor(A8,0);
								tessVerts[1].color=GetColor(Ac,0);
								tessVerts[2].color=GetColor(A10,0);
							} else {
								tessVerts[0].color=GetColor(A8,1);
								tessVerts[1].color=GetColor(Ac,1);
								tessVerts[2].color=GetColor(A10,1);
							}

							DrawCustomTriangle2(&tessVerts[0],&tessVerts[1],&tessVerts[2]);
						
                            return;
L00468ad1:

							if(esi >= 2) {
                                goto L00468ce0;
                            }

                            FUNC_001827((ffeVector*)&newVertexXYZ, (ffeVector*)&A8->nx, (ffeVector*)&Ac->nx, (ffeVector*)&A10->nx);
							
                            C_FUNC_001826_NormalizeTo13Bits((ffeVector*)&normalizedTo13BitsXYZ, (ffeVector*)&A8->nx);
                            distToVertex = C_FUNC_001465_GetDist((ffeVector*)&normalizedTo13BitsXYZ);
                            ecx = esi;
							eax = distToVertex * (0xffffb000 >> ecx);
							edx = newVertexXYZ.x * normalizedTo13BitsXYZ.x;
                            edx = edx + newVertexXYZ.y * normalizedTo13BitsXYZ.y;
							edx = edx + newVertexXYZ.z * normalizedTo13BitsXYZ.z;
                            if(eax <= edx) {
                                goto L00468ce0;
                            }

                            C_FUNC_001826_NormalizeTo13Bits((ffeVector*)&normalizedTo13BitsXYZ, (ffeVector*)&Ac->nx);
                            distToVertex = C_FUNC_001465_GetDist((ffeVector*)&normalizedTo13BitsXYZ);
                            ecx = esi;
                            eax = 0xffffb000 >> ecx;
							eax = eax * distToVertex;
							edx = newVertexXYZ.x * normalizedTo13BitsXYZ.x;
							edx = edx + newVertexXYZ.y * normalizedTo13BitsXYZ.y;
							edx = edx + newVertexXYZ.z * normalizedTo13BitsXYZ.z;
                            if(eax <= edx) {
                                goto L00468ce0;
                            }

                            C_FUNC_001826_NormalizeTo13Bits((ffeVector*)&normalizedTo13BitsXYZ, (ffeVector*)&A10->nx);
                            distToVertex = C_FUNC_001465_GetDist((ffeVector*)&normalizedTo13BitsXYZ);
                            ecx = esi;
                            eax = 0xffffb000 >> ecx;
							eax = eax * distToVertex;
							edx = newVertexXYZ.x * normalizedTo13BitsXYZ.x;
							edx = edx + newVertexXYZ.y * normalizedTo13BitsXYZ.y;
							edx = edx + newVertexXYZ.z * normalizedTo13BitsXYZ.z;

							if(eax <= edx) {
                                goto L00468ce0;
							}

L00468c26:
							if(dist >= 500) { //500
                                    edi = abs(A8->nz);
                                    ebp_50 = abs(Ac->nz);
                                    eax = abs(A10->nz);
									if(edi >= A8->nx || Ac->nx <= ebp_50 || eax >= A10->nx) {
                                        if(~edi <= A8->nx || Ac->nx >= ~ebp_50) {
                                            goto L00468ca0;
                                        }
                                        edx = ~eax;
										if(edx <= A10->nx) {
L00468ca0:
											if(edi >= A8->ny || Ac->ny <= ebp_50 || eax >= A10->ny) {
                                                if(~edi <= A8->ny || Ac->ny >= ~ebp_50) {
                                                    goto L00468ce0;
                                                }
                                                eax = ~eax;
												if(eax <= A10->ny) {
L00468ce0:
													if(A28 != 0) {
														ebp_24 = 0;
														ebp_7c = (int)&ebp_40;
														ebp_78 = (int)&A10->unknown5_2;
														ebp_74 = (int)&ebp_38;
														ebp_70 = (int)&Ac->unknown5_2;
														ebp_6c = (int)&ebp_30;
														ebp_68 = (int)&A8->unknown5_2;
														ebp_64 = (int)DATA_009260;
														ebp_60 = A28;
														do {
															if(*(int*)ebp_60 > 0) {
															  ebp_54 = *(int*)ebp_60;
																ebp_58 = (int)DATA_007841 + *(int*)ebp_54 * 12;
															  // В edi пишется офсет для DATA_009260
																edi = *(int*)(ebp_58 + 8) + esi - 1;
																eax = ebp_64 + edi * 8;
																do {
																// в DATA_009260 переписывается по 2 байта из DATA_007841
																	*(int*)eax = *(int*)ebp_58;
																	*(int*)(eax + 4) = *(int*)(ebp_58 + 4);
																	edi = edi - 1;
																	eax = eax - 8;
																} while(esi <= edi); // пока глубина деления (esi) <= edi
																*(int*)ebp_6c = *(int*)ebp_68; // в ebp_40 пишем A8->unknown5_2
																*(int*)ebp_74 = *(int*)ebp_70; // в ebp_38 пишем Ac->unknown5_2
																*(int*)ebp_7c = *(int*)ebp_78; // в ebp_30 пишем A10->unknown5_2
																*(int*)ebp_68 = *(int*)(ebp_54 + 4); // в A8->unknown5_2 из A28
																*(int*)ebp_70 = *(int*)(ebp_54 + 8); // в Ac->unknown5_2 из A28
																*(int*)ebp_78 = *(int*)(ebp_54 + 12); // в A10->unknown5_2 из A28
															}
															ebp_24 = ebp_24 + 1; 	// Увеличиваем счетчик
															ebp_7c = ebp_7c + 4; 	// ebp_40
															ebp_78 = ebp_78 + 4; 	// A10->unknown5_2
															ebp_74 = ebp_74 + 4; 	// ebp_38
															ebp_70 = ebp_70 + 4; 	// Ac->unknown5_2
															ebp_6c = ebp_6c + 4; 	// ebp_30
															ebp_68 = ebp_68 + 4; 	// A8->unknown5_2
															ebp_64 = ebp_64 + 0x100;  // DATA_009260
															ebp_60 = ebp_60 + 4; 	// A28
														} while(ebp_24 < 2); 	// три прохода   
													}
                                                    if(esi < 5) { // Сколько раз тесселлировать (скруглять) экосаэдр
														//callTriangleDeliver((int*)A18, Ac, A10, A8, esi, funcNum+4);
														//callTriangleDeliver((int*)A1c, A10, A8, Ac, esi, funcNum+4);
														//callTriangleDeliver((int*)A14, A8, Ac, A10, esi, funcNum+4);
														shiftCall3 = (int (*)(int*,int*,int*,int*,int)) *(int*)DATA_009278;
                                                        shiftCall3((int*)A18, (int*)Ac, (int*)A10, (int*)A8, esi);
                                                        shiftCall3((int*)A1c, (int*)A10, (int*)A8, (int*)Ac, esi);
                                                        shiftCall3((int*)A14, (int*)A8, (int*)Ac, (int*)A10, esi);
                                                    } else {
														//callTriangleDeliver((int*)A18, Ac, A10, A8, esi, funcNum);
														//callTriangleDeliver((int*)A1c, A10, A8, Ac, esi, funcNum);
														//callTriangleDeliver((int*)A14, A8, Ac, A10, esi, funcNum);
														shiftCall3 = (int (*)(int*,int*,int*,int*,int)) *(int*)DATA_009279;
                                                        shiftCall3((int*)A18, (int*)Ac, (int*)A10, (int*)A8, esi);
                                                        shiftCall3((int*)A1c, (int*)A10, (int*)A8, (int*)Ac, esi);
														shiftCall3((int*)A14, (int*)A8, (int*)Ac, (int*)A10, esi);
                                                    }
                                                    ebp_8 = C_FUNC_001820_ArrayCreateNewElement();
                                                    *ebp_8 = A18[2];
                                                    ebp_c = C_FUNC_001820_ArrayCreateNewElement();
                                                    *ebp_c = A1c[2];
                                                    ebp_10 = C_FUNC_001820_ArrayCreateNewElement();
                                                    *ebp_10 = A14[2];
                                                    esi = esi + 1;
                                                    dist = dist >> 1;
													if(A28 != 0) {
                                                        C_FUNC_001869(A8,
																	A14[2],
																	A1c[2],
																	A8 == A14[0] ? (ffeVertex**)A14[1] : (ffeVertex**)A14[3],
																	(ffeVertex**)ebp_10,
																	A8 == A1c[0] ? (ffeVertex**)A1c[1] : (ffeVertex**)A1c[3],
																	esi,
																	dist,
																	*(int*)((char*)A28 + 8));
                                                        C_FUNC_001869(Ac,
																	A18[2],
																	A14[2],
																	A18[0] == Ac ? (ffeVertex**)A18[1] : (ffeVertex**)A18[3],
																	(ffeVertex**)ebp_8,
																	A14[0] == Ac ? (ffeVertex**)A14[1] : (ffeVertex**)A14[3],
																	esi,
																	dist,
																	*(int*)((char*)A28 + 0xc));
                                                        C_FUNC_001869(A10,
																	A1c[2],
																	A18[2],
																	A1c[0] == A10 ? (ffeVertex**)A1c[1] : (ffeVertex**)A1c[3],
																	(ffeVertex**)ebp_c,
																	A18[0] == A10 ? (ffeVertex**)A18[1] : (ffeVertex**)A18[3],
																	esi,
																	dist,
																	*(int*)((char*)A28 + 0x10));
                                                        C_FUNC_001869(A18[2],
																	A1c[2], 
																	A14[2], 
																	(ffeVertex**)ebp_c, 
																	(ffeVertex**)ebp_10, 
																	(ffeVertex**)ebp_8, 
																	esi, 
																	dist, 
																	*(int*)((char*)A28 + 0x14));
                                                        esi = esi - 1;
														ebp_24 = 0;
														ebp_60 = (int)&A10->unknown5_2;
														ebp_64 = (int)&ebp_40;
														ebp_68 = (int)&Ac->unknown5_2;
														ebp_6c = (int)&ebp_38;
														ebp_70 = (int)&A8->unknown5_2;
														ebp_74 = (int)&ebp_30;
														ebp_78 = (int)DATA_009260;
														ebp_7c = A28;
														do {
															if(*(int*)ebp_7c > 0) {
																ebp_5c = *(int*)ebp_7c;
																edi = *(int*)((int)DATA_007842 + *(int*)ebp_5c * 12) + esi - 1;
																eax = ebp_78 + edi * 8;
																do {
																// обнуляется DATA_009260 по 2 байта
																	*(int*)eax = 0;
																	*(int*)(eax + 4) = 0;
																	edi = edi - 1;
																	eax = eax - 8;
																} while(esi <= edi); // пока глубина деления (esi) <= edi
																//*(int*)ebp_70 = *(int*)ebp_74; // в A8->unknown5_2 из ebp_30
																//*(int*)ebp_68 = *(int*)ebp_6c; // в Ac->unknown5_2 из ebp_38
																//*(int*)ebp_60 = *(int*)ebp_64; // в A10->unknown5_2 из ebp_40
																*(int*)ebp_70 = *(int*)ebp_64; // в A8->unknown5_2 из ebp_40
																*(int*)ebp_68 = *(int*)ebp_6c; // в Ac->unknown5_2 из ebp_38
																*(int*)ebp_60 = *(int*)ebp_74; // в A10->unknown5_2 из ebp_30
															}
															ebp_24 = ebp_24 + 1;	 // Увеличиваем счетчик
															ebp_60 = ebp_60 + 4;	 // A10->unknown5_2;
															ebp_64 = ebp_64 + 4;	 // ebp_40
															ebp_68 = ebp_68 + 4;	 // Ac->unknown5_2
															ebp_6c = ebp_6c + 4;	 // ebp_38
															ebp_70 = ebp_70 + 4;	 // A8->unknown5_2
															ebp_74 = ebp_74 + 4;	 // ebp_30
															ebp_78 = ebp_78 + 0x100;   // DATA_009260
															ebp_7c = ebp_7c + 4;       // A28
														} while(ebp_24 < 2);	// три прохода
                                                    } else {
                                                        C_FUNC_001869(A8,
																	A14[2],
																	A1c[2],
																	A8 == A14[0] ? (ffeVertex**)A14[1] : (ffeVertex**)A14[3],
																	(ffeVertex**)ebp_10,
																	A8 == A1c[0] ? (ffeVertex**)A1c[1] : (ffeVertex**)A1c[3],
																	esi,
																	dist,
																	0);
                                                        C_FUNC_001869(Ac,
																	A18[2],
																	A14[2],
																	A18[0] == Ac ? (ffeVertex**)A18[1] : (ffeVertex**)A18[3],
																	(ffeVertex**)ebp_8,
																	A14[0] == Ac ? (ffeVertex**)A14[1] : (ffeVertex**)A14[3],
																	esi,
																	dist,
																	0);
                                                        C_FUNC_001869(A10,
																	A1c[2],
																	A18[2],
																	A1c[0] == A10 ? (ffeVertex**)A1c[1] : (ffeVertex**)A1c[3],
																	(ffeVertex**)ebp_c,
																	A18[0] == A10 ? (ffeVertex**)A18[1] : (ffeVertex**)A18[3],
																	esi,
																	dist,
																	0);
                                                        C_FUNC_001869(A18[2],
																	A1c[2], 
																	A14[2], 
																	(ffeVertex**)ebp_c, 
																	(ffeVertex**)ebp_10, 
																	(ffeVertex**)ebp_8, 
																	esi, 
																	dist, 
																	0);
                                                    }
                                                    C_FUNC_001821_ArrayRemoveElement(ebp_8);
                                                    C_FUNC_001821_ArrayRemoveElement(ebp_c);
                                                    C_FUNC_001821_ArrayRemoveElement(ebp_10);
                                                }
                                            }
                                        }
                                    }
                                }
                            
                        }
                    }
                }
            }
        }
    }
}

int C_FUNC_001840(ffeVertex* A8, ffeVertex* Ac, ffeVertex* A10, ffeVector* A14, int A18)
{
    int		eax;
    int		ebx;
    int		ecx;
	int		edx;
	int		esi;
	ffeVertex*	edi;
	ffeVertex*	tmpVertexPtr;

    int		local0;
    int		local2;
    int		local3;



    esi = (int)A18;
    eax = (int)A14;
    edi = A8;
    ebx = *(int*)(*(int*)DATA_009261 + 0x134); // light source x ?
    ebx = ebx * A14->x;
    edx = *(int*)(*(int*)DATA_009261 + 0x138); // light source y ?
    edx = edx * A14->y;
	ebx = ebx + edx;
    edx = *(int*)(*(int*)DATA_009261 + 0x13c); // light source z ?
    edx = edx * A14->z;
	ebx = ebx + edx;
    ebx = ~ebx;

    if(ebx < 0) {
        ebx = ebx >> 0x1b;
        if(ebx < 0x18) {
            ebx = 0;
        }
        eax = (ebx & 7) + (ebx & 7) * 2;
        edx = (int)DATA_009261;
        local2 = *(int*)(edx + eax + 0x84);
        local3 = *(int*)(edx + eax + 0x86);
    } else {
        local2 = 0;
    }

	if(Ac->x_2 > edi->x_2) {
        tmpVertexPtr = edi;
        edi = Ac;
        Ac = tmpVertexPtr;
    }
	if(A10->x_2 > edi->x_2) {
        tmpVertexPtr = edi;
        edi = A10;
        A10 = tmpVertexPtr;
    }

	if(Ac->x_2 < A10->x_2) {
        tmpVertexPtr = Ac;
        Ac = A10;
        A10 = tmpVertexPtr;
    }

    if(*(int*)DATA_009262 != 0) {
		ebx = ((abs(edi->nx) + abs(edi->ny)) / 8) + edi->nz;
        ecx = *(int*)DATA_009263;
        if(ecx != 0) {
            eax = ebx - ecx;
            //asm("cdq");
            eax = eax / *(int*)DATA_009267;			
            edx = eax % *(int*)DATA_009267;
            ebx = eax;
        } else {
            eax = ebx;
            //asm("cdq");
            eax = eax / *(int*)DATA_009267;			
            edx = eax % *(int*)DATA_009267;
            ebx = eax;
        }
        if(ebx < 0) {
            ebx = 0;
        }
        if(ebx > 8) {
            ebx = 8;
        }
    } else {
        ebx = 0;
    }
    //*(edi + 0x3c) :: 0;
    //asm("setng al");
	if (edi->x_2<=0) {
		eax=1;
	}
    eax = eax & 1;
    //*(edx + 0x3c) :: 0;
    //asm("setng dl");
	if (Ac->x_2<=0) {
		edx=1;
	}
    eax = eax + (edx & 1);
    //*(edx + 0x3c) :: 0;
    //asm("setng dl");
	if (A10->x_2<=0) {
		edx=1;
	}
    eax = eax + (edx & 1);
	
/*	
    if(*(int*)DATA_008812_GraphicsDetailRelated >= 0) {
        if(esi <= 0x1fffffff) {
            goto L0046342d;
        }
        if(eax != 3) {
L0046342d:
			if(esi <= 0x7fffffff || edi[1].x - A10[1].x >= 0x1fffffff) {
                goto L0046355c;
            }
        }
    }
*/	
	eax = edi->x_2;
    edx = eax >> 18;
    ecx = edx;
	edx = edx * 8;
	edx = edx - ecx;
	edx = edx + edx * 2;
    //edx = edx * 8 - ecx + (edx * 8 - ecx) * 2;
    if(edx < 0) {
        edx = edx + 0x1ff;
    }
    local0 = edx >> 9;
    if(eax <= 0) {
        local0 = local0 + 3;
        if(local0 < 0) {
            local0 = 0;
        }
        if(ebx != 0) {
            eax = *(int*)((local0 << 5) + (int)DATA_007846);
            edx = 8 - ebx;
            //asm("imul edx");
			eax = eax * edx;
            edx = *(int*)DATA_009265;
			edx = edx * ebx;
			eax = eax + edx;
			eax = eax >> 3;
			eax = eax & 0xf0f0f;
			local2 = local2 + eax;
            //local2 = local2 + (eax + ebx * ebx >> 3 & 0xf0f0f);
        } else {
            local2 = local2 + *(int*)((local0 << 5) + (int)DATA_007846);
        }
    } else {
        if(local0 > 0x14) {
            local0 = 0x14;
        }
        if(ebx != 0) {
            eax = local0;
            edx = eax;
            eax = *(int*)(((eax << 3) - edx) * 4 + (int)DATA_007849);
            edx = 8 - ebx;
            //asm("imul edx");
			eax = eax * edx;
            edx = *(int*)DATA_009265;
			edx = edx * ebx;
			eax = eax + edx;
			eax = eax >> 3;
			eax = eax & 0xf0f0f;
			local2 = local2 + eax;
            //local2 = local2 + (eax + ebx * ebx >> 3 & 0xf0f0f);
        } else {
            eax = local0;
            edx = eax;
            local2 = local2 + *(int*)(((eax << 3) - edx) * 4 + (int)DATA_007849);
        }
    }
    esi = FUNC_001778(0xc, esi, 0);
    *(int*)esi = (int)FUNC_001804;
	*(int*)(esi + 4) = *(int*)&edi[0].orto_x;
	*(int*)(esi + 8) = *(int*)&Ac[0].orto_x;
	*(int*)(esi + 0xc) = *(int*)&A10[0].orto_x;
    eax = FUNC_001326_DPal32to16(local2);
    eax = eax & 65535;
    *(int*)(esi + 0x10) = eax;

    return eax;
/*
L0046355c:
	eax = eax - 1;
    if(eax > 0) {
        if(eax = eax - 1) {
            goto L004635ec;
        }
        if(!(eax = eax - 1)) {
			eax = Ac[1].x + edi[1].x;
			local0 = ((eax + A10[1].x) >> 0x19) + 3;
            if(local0 < 0) {
                local0 = 0;
            }
            esi = FUNC_001778(0xc, esi, 0);
            *(int*)esi = (int)FUNC_001807;
			*(int*)(esi + 4) = *(int*)&edi[0].orto_x;
			*(int*)(esi + 8) = *(int*)&Ac[0].orto_x;
			*(int*)(esi + 0xc) = *(int*)&A10[0].orto_x;
            *(int*)(esi + 0x10) = 0x121; // 289
            eax = FUNC_001838(esi + 0x14, ebx, local2, (local0 << 5) + (int)DATA_007846);
            return eax;
L004635ec:
            local1 = FUNC_001778(0xd, esi, 0);
            *(int*)local1 = (int)FUNC_001805;
            *(int*)(local1 + 4) = *(int*)&A10[0].orto_x;
            *(int*)(local1 + 8) = *(int*)&Ac[0].orto_x;
			FUNC_001837(local1 + 0xc, (int)Ac, (int)edi);
            FUNC_001837(local1 + 0x10, (int)A10, (int)edi);
            *(int*)(local1 + 0x14) = 0x121; // 289
            FUNC_001838(local1 + 0x18, ebx, local2, (int)DATA_007847);
			if(edi[1].x < 0x400000) {
                esi = FUNC_001778(0xc, esi, 0);
                *(int*)esi = (int)FUNC_001807;
                *(int*)(esi + 4) = *(int*)(local1 + 0xc);
                *(int*)(esi + 8) = *(int*)(local1 + 0x10);
				*(int*)(esi + 0xc) = *(int*)&edi[0].orto_x;
                *(int*)(esi + 0x10) = 0x40; // 64
                eax = FUNC_001838(esi + 0x14, ebx, local2, (int)DATA_007849);
            } else {
                esi = FUNC_001778(0x14, esi, 0);
                *(int*)esi = (int)FUNC_001808;
                *(int*)(esi + 4) = *(int*)(local1 + 0xc);
                *(int*)(esi + 8) = *(int*)(local1 + 0x10);
                *(int*)(esi + 0xc) = *(int*)&edi[0].orto_x;
                *(int*)(esi + 0x10) = 0;
                *(int*)(esi + 0x12) = 0;
                *(int*)(esi + 0x14) = 0x3f; // 63
                *(int*)(esi + 0x16) = 0;
                *(int*)(esi + 0x18) = 0;
                *(int*)(esi + 0x1a) = (edi[1].x >> 0x12) & 65535;
                if(*(int*)(esi + 0x1a) > 0x1ff) {
                    *(int*)(esi + 0x1a) = 0x1ff; // 511
                }
                *(int*)(esi + 0x1c) = 0x5e; // 94
                *(int*)(esi + 0x1e) = 0;
                *(int*)(esi + 0x20) = 0x16; // 22
                eax = FUNC_001839(esi + 0x24, ebx, local2, (int)DATA_007848, 0x16);
                return eax;
            }
        }
	} else if (eax==0) {
        if(edi[1].x < 0x400000) {
            local1 = FUNC_001778(0xd, esi, 0);
            *(int*)local1 = (int)FUNC_001805;
			*(int*)(local1 + 4) = *(int*)&edi[0].orto_x;
			*(int*)(local1 + 8) = *(int*)&Ac[0].orto_x;
            FUNC_001837(local1 + 0xc, (int)A10, (int)Ac);
            FUNC_001837(local1 + 0x10, (int)A10, (int)edi);
            *(int*)(local1 + 0x14) = 0x40; // 64
            FUNC_001838(local1 + 0x18, ebx, local2, (int)DATA_007849);
        } else {
            local1 = FUNC_001778(0x16, esi, 0);
            *(int*)local1 = (int)FUNC_001806;
            *(int*)(local1 + 4) = *(int*)edi;
            *(int*)(local1 + 8) = *(int*)Ac;
            FUNC_001837(local1 + 0xc, (int)A10, (int)Ac);
            FUNC_001837(local1 + 0x10, (int)A10, (int)edi);
            *(int*)(local1 + 0x14) = 0;
            *(int*)(local1 + 0x16) = (edi[1].x >> 0x12) & 65535;
            *(int*)(local1 + 0x18) = 0x3f;
            *(int*)(local1 + 0x1a) = Ac[1].x >> 0x12;
            *(int*)(local1 + 0x1c) = 0x3f;
            *(int*)(local1 + 0x1e) = 0;
            *(int*)(local1 + 0x20) = 0;
            *(int*)(local1 + 0x22) = 0;
            if(*(int*)(local1 + 0x16) > 0x1ff) {
                *(int*)(local1 + 0x16) = 0x1ff;
            }
            if(*(int*)(local1 + 0x1a) > 0x1ff) {
                *(int*)(local1 + 0x1a) = 0x1ff;
            }
            *(int*)(local1 + 0x24) = 0x5e; // 94 - texture number
            *(int*)(local1 + 0x26) = 0;
            *(int*)(local1 + 0x28) = 0x16;
            FUNC_001839(local1 + 0x2c, ebx, local2, (int)DATA_007848, 0x16);
        }
        esi = FUNC_001778(0xc, esi, 0);
        *(int*)esi = (int)FUNC_001807;
		*(int*)(esi + 4) = *(int*)&A10[0].orto_x;
        *(int*)(esi + 8) = *(int*)(local1 + 0xc);
        *(int*)(esi + 0xc) = *(int*)(local1 + 0x10);
        *(int*)(esi + 0x10) = 0x121;
        eax = FUNC_001838(esi + 0x14, ebx, local2, (int)DATA_007847);
    } else {
        if(edi[1].x - A10[1].x < 0x400000) {
            esi = FUNC_001778(0xc, esi, 0);
            *(int*)esi = (int)FUNC_001807;
			*(int*)(esi + 4) = *(int*)&edi[0].orto_x;
			*(int*)(esi + 8) = *(int*)&Ac[0].orto_x;
			*(int*)(esi + 0xc) = *(int*)&A10[0].orto_x;
            *(int*)(esi + 0x10) = 0x40;
            eax = A10[1].x >> 0x12;
            edx = eax;
            eax = (eax << 3) - edx + ((eax << 3) - edx) * 2; // ??? eax*21?
            if(eax < 0) {
                eax = eax + 0x1ff;
            }
            local0 = eax >> 9;
            if(local0 > 0x14) {
                local0 = 0x14; // 20
            }
            eax = local0;
            edx = eax;
            eax = FUNC_001838(esi + 0x14, ebx, local2, (((eax << 3) - edx) << 2) + (int)DATA_007849);
        } else {
            esi = FUNC_001778(0x14, esi, 0);
            *(int*)esi = (int)FUNC_001808;
			*(int*)(esi + 4) = *(int*)&edi[0].orto_x;
			*(int*)(esi + 8) = *(int*)&Ac[0].orto_x;
			*(int*)(esi + 0xc) = *(int*)&A10[0].orto_x;
            *(int*)(esi + 0x10) = 0;
            *(int*)(esi + 0x12) = (edi[1].x >> 0x12) & 65535;
            *(int*)(esi + 0x14) = 0x3f; // 63
            *(int*)(esi + 0x16) = (Ac[1].x >> 0x12) & 65535;
            *(int*)(esi + 0x18) = 0;
            *(int*)(esi + 0x1a) = (A10[1].x >> 0x12);
            if(*(int*)(esi + 0x12) > 0x1ff) {
                *(int*)(esi + 0x12) = 0x1ff;
            }
            if(*(int*)(esi + 0x16) > 0x1ff) {
                *(int*)(esi + 0x16) = 0x1ff;
            }
            if(*(int*)(esi + 0x1a) > 0x1ff) {
                *(int*)(esi + 0x1a) = 0x1ff;
            }
            *(int*)(esi + 0x1c) = 0x5e; // 94 - texture number
            *(int*)(esi + 0x1e) = 0;
            *(int*)(esi + 0x20) = 0x16;
            eax = FUNC_001839(esi + 0x24, ebx, local2, (int)DATA_007848, 0x16);
        }
    }
return eax;
*/
}

inline int GetColor(ffeVertex* V, int type)
{
     int		eax;
    int		ebx;
    int		ecx;
	int		edx;
	ffeVertex*	edi;

    int		local0;
    int		local2;

    local2 = 0;

    edi = V;


    if(*(int*)DATA_009262 != 0) {
		ebx = ((abs(edi->nx) + abs(edi->ny)) / 8) + edi->nz;
        ecx = *(int*)DATA_009263;
        if(ecx != 0) {
            eax = ebx - ecx;
            //asm("cdq");
            eax = eax / *(int*)DATA_009267;			
            edx = eax % *(int*)DATA_009267;
            ebx = eax;
        } else {
            eax = ebx;
            //asm("cdq");
            eax = eax / *(int*)DATA_009267;			
            edx = eax % *(int*)DATA_009267;
            ebx = eax;
        }
        if(ebx < 0) {
            ebx = 0;
        }
        if(ebx > 8) {
            ebx = 8;
        }
    } else {
        ebx = 0;
    }

    eax = edi->x_2;
    edx = eax >> 18;
    ecx = edx;
	edx = edx * 8;
	edx = edx - ecx;
	edx = edx + edx * 2;
    //edx = edx * 8 - ecx + (edx * 8 - ecx) * 2;
    if(edx < 0) {
        edx = edx + 0x1ff;
    }
    local0 = edx >> 9;
    if(eax <= 0) {
        local0 = local0 + 3;
        if(local0 < 0) {
            local0 = 0;
        }
        if(ebx != 0) {
            eax = *(int*)((local0 << 5) + (int)DATA_007846);
            edx = 8 - ebx;
            //asm("imul edx");
			eax = eax * edx;
            edx = *(int*)DATA_009265;
			edx = edx * ebx;
			eax = eax + edx;
			eax = eax >> 3;
			eax = eax & 0xf0f0f;
			local2 = local2 + eax;
            //local2 = local2 + (eax + ebx * ebx >> 3 & 0xf0f0f);
        } else {
            local2 = local2 + *(int*)((local0 << 5) + (int)DATA_007846);
        }
    } else {
        if(local0 > 0x14) {
            local0 = 0x14;
        }
        if(ebx != 0) {
            eax = local0;
            edx = eax;
            eax = *(int*)(((eax << 3) - edx) * 4 + (int)DATA_007849);
            edx = 8 - ebx;
            //asm("imul edx");
			eax = eax * edx;
            edx = *(int*)DATA_009265;
			edx = edx * ebx;
			eax = eax + edx;
			eax = eax >> 3;
			eax = eax & 0xf0f0f;
			local2 = local2 + eax;
            //local2 = local2 + (eax + ebx * ebx >> 3 & 0xf0f0f);
        } else {
            eax = local0;
            edx = eax;
            local2 = local2 + *(int*)(((eax << 3) - edx) * 4 + (int)DATA_007849);
        }
    }
	if (type==1)
		local2 = *(int*)(((local0 << 2) - local0) * 4 + (int)DATA_007852);

	return local2;

}

/*
int GetColor2(ffeVertex* V)
{
    int		eax;
    int		ebx;
    int		ecx;
	int		edx;
	int		esi;
	ffeVertex*	edi;
	ffeVertex*	tmpVertexPtr;

    int		local0;
    int		local1;
    int		local2;
    int		local3;

    local2 = 0;

    eax = FUNC_001778(0x19, *(ebp + 0x18), 0);
    *eax = FUNC_001808;
    *(eax + 4) = *ebx;
    *(eax + 8) = *esi;
    *(eax + 0xc) = *( *(ebp + 0x10));
    *(eax + 0x10) = 0;
    *(eax + 0x12) = (*(ebx + 0x3c) >> 0x12) & 65535;
    *(eax + 0x14) = 0x3f;
    *(eax + 0x16) = (*(esi + 0x3c) >> 0x12) & 65535;
    *(eax + 0x18) = 0;
    edx = *( *(ebp + 0x10) + 0x3c) >> 0x12;
    *(eax + 0x1a) = dx;
    ecx = *(eax + 0x12);
    if(ecx - dx < 0x10) { // < 16
        *(eax + 0x1a) = *(eax + 0x1a) - 8;
        if(*(eax + 0x1a) < 0) {
            *(eax + 0x1a) = 0;
            *(eax + 0x12) = 0x10;
        } else {
            dx = *(eax + 0x1a) + 0x10;
            *(eax + 0x12) = dx;
            if(*(eax + 0x12) > 0x1ff) {
                *(eax + 0x12) = 0x1ff;
                *(eax + 0x1a) = 0x1ef;
            }
        }
    }
    *(eax + 0x1c) = 0x5f;
    *(eax + 0x1e) = 0;
    *(eax + 0x20) = 0x20;
    eax = FUNC_001839(eax + 0x24, 0, local2, DATA_007852, 0x20);
    (restore)esi;
    (restore)ebx;
    (restore)ecx;
    (restore)ecx;
    (restore)ebp;
}
*/

void C_FUNC_001826_NormalizeTo13Bits(ffeVector* New, ffeVector* A8)
{
    int		eax;

	eax = abs(A8->x) | abs(A8->y) | abs(A8->z);
    eax = FUNC_001656_FindMSB(eax) - 0xd;
    if(eax >= 0) {
        New->x = A8->x >> (char)eax;
        New->y = A8->y >> (char)eax;
        New->z = A8->z >> (char)eax;
    } else {
        New->x = A8->x;
        New->y = A8->y;
        New->z = A8->z;
    }
}


void C_FUNC_001827_CalculateNewVertexXYZ(ffeVector* New, ffeVector* A8, ffeVector* Ac, ffeVector* A10)
{
    int		eax;
	int		edx;

    int  x2;	// Vffffffe8
    int  y2;	// Vffffffec
    int  z2;	// Vfffffff0
    int  x1;	// Vfffffff4
    int  y1;	// Vfffffff8
    int  z1;	// Vfffffffc

	x1 = Ac->x + ~(A8->x);
	y1 = Ac->y + ~(A8->y);
    z1 = Ac->z + ~(A8->z);

    x2 = A10->x + ~(A8->x);
    y2 = A10->y + ~(A8->y);
    z2 = A10->z + ~(A8->z);

    eax = C_FUNC_001825_MaxUsedBitsInXYZ(&x1) - 6;
    if(eax > 0) {
        x1 = x1 >> eax;
        y1 = y1 >> eax;
        z1 = z1 >> eax;
    }
    eax = C_FUNC_001825_MaxUsedBitsInXYZ(&x2) - 6;
    if(eax > 0) {
        x2 = x2 >> eax;
        y2 = y2 >> eax;
        z2 = z2 >> eax;
    }
    edx = z1;
	New->x = y1 * z2 - y2 * y2;
    edx = x1;
	New->y = z1 * x2 - z2 * z2;
    edx = y1;
	New->z = x1 * y2 - x2 * x2;
    //ecx = FUNC_001465_GetDist((int*)New) + 1;
	eax = New->x << 0xf;
    //ecx = ecx / ecx;
    //edx = ecx % ecx;
	New->x = eax;
	eax = New->y << 0xf;
    //ecx = ecx / ecx;
    //edx = ecx % ecx;
	New->y = eax;
	//eax = New->z << 0xf;
    //edx = ecx / ecx % ecx / ecx;
	New->z = eax;
}

/*
void C_FUNC_001827_CalculateNewVertexXYZ(ffeVector* New, ffeVector* A8, ffeVector* Ac, ffeVector* A10)
{
	New->x = (A8->x + Ac->x + A10->x)/3;
	New->y = (A8->y + Ac->y + A10->y)/3;
	New->z = (A8->z + Ac->z + A10->z)/3;
}
*/
int C_FUNC_001465_GetDist(ffeVector* A8)
{
    return (int)sqrt((double)(A8->x*A8->x + A8->y*A8->y + A8->z*A8->z));
}

int C_FUNC_001777(int *Data, ffeVector* A8)
{
    int		eax;
	int		ebx;
    int		ecx;
	int		edx;

	if(A8->z <= 0) {
        eax = 1;
    } else {
        ebx = ((abs(A8->x) + abs(A8->y)) >> 3) + A8->z;
        ecx = *(int*)((int)Data + 0x130); // 00000130 alt_Scale from DrawMdl_t?
        edx = 3 - ecx;
        if(edx >= 0) {
            eax = ebx >> (char)edx;
        } else {
            edx = -edx;
            if(ebx < (0x80000000 >> (char)edx)) {
                eax = ebx << (char)edx;
            } else {
                edx = (edx << 0x19) | 0x80000000;
				eax = (A8->z >> 7) | edx;
            }
        }
    }
	return eax;
}

extern "C" int C_FUNC_001846(int A8, int Ac, int A10)
{
    int  eax, ebx;

    if(A10 <= 1) {
        eax = C_FUNC_001824_Random(A8 ^ Ac) >> 4;
    } else {
        ebx = A8 - Ac;
        if(ebx < 0) {
            ebx = ~ebx;
        }
        eax = C_FUNC_001521_MulWithNormalize(FUNC_001824(A8 ^ Ac), ebx) >> 1;
    }
	return eax;
}

extern "C" int C_FUNC_001849(int* A8, ffeVertex* Ac, ffeVertex* A10, ffeVertex* A14, int A18)
{
    int		eax;
	ffeVertex*		ebx;
    int		ecx;
	ffeVertex*		edx;
	ffeVertex*		esi;
	int*		edi;

	int		ebp_4;
	int		ebp_8;

    esi = Ac;
    edi = A8;
	eax = *(edi+2);
    if(eax == 0) {
        eax = C_FUNC_001818_ArrayRemoveFirstElement();
        *(int*)(edi+2) = eax;
        ebx = (ffeVertex *)*(int*)(edi+2);
        if(esi == (ffeVertex*)*edi) {
            eax = (int)C_FUNC_001820_ArrayCreateNewElement();
            *(int*)(edi+1) = eax;
            *(int*)*(int*)(edi+1) = (int)esi;
            eax = (int)C_FUNC_001820_ArrayCreateNewElement();
            *(int*)(edi+3) = eax;
            *(int*)eax = (int)A10;
        } else {
            eax = (int)C_FUNC_001820_ArrayCreateNewElement();
            *(int*)(edi+1) = eax;
            *(int*)eax = (int)A10;
            eax = (int)C_FUNC_001820_ArrayCreateNewElement();
            *(int*)(edi+3) = eax;
            *(int*)*(int*)(edi+3) = (int)esi;
        }
		eax = esi->normal_x + A10->normal_x;
		ebx->normal_x = C_FUNC_001521_MulWithNormalize(eax, *(int*)(A18 * 4 + (int)DATA_007840));
		eax = esi->normal_y + A10->normal_y;
		ebx->normal_y = C_FUNC_001521_MulWithNormalize(eax, *(int*)(A18 * 4 + (int)DATA_007840));
		eax = esi->normal_z + A10->normal_z;
		ebx->normal_z = C_FUNC_001521_MulWithNormalize(eax, *(int*)(A18 * 4 + (int)DATA_007840));
        edi = (int*)*(int*)(A18 * 4 + DATA_007839);
		if(A18 > esi->z_2 && A18 > A10->z_2) {
			eax = esi->nx;
			ebx->nx = (eax + A10->nx) >> 1;
			eax = esi->ny;
			ebx->ny = (eax + A10->ny) >> 1;
			eax = esi->nz;
			ebx->nz = (eax + A10->nz) >> 1;
			if(esi->nz < 64 || A10->nz < 64) {
                //*L006c7348(ebx); //call near [DATA_009282]
				shiftCall = (char *(*)(int)) *(int*)(DATA_009282); // pointer to func
				shiftCall((int)ebx);

            } else {
				eax = esi->orto_x;
				eax = (eax + A10->orto_x) >> 1;
                ebx->orto_x = eax;
                eax = esi->orto_y;
                ebx->orto_y = (eax + A10->orto_y) >> 1;
                eax = (int)A14;
				if(((ffeVertex*)eax)->nz > 64) {
					eax = esi->orto_y;
					edx = A10;
					if (eax == edx->orto_y)
						eax = 1;
					else
						eax = 0;
						
                    ebp_4 = eax;

                    eax = esi->orto_x;
					edx = A10;
					if (eax == edx->orto_x)
						eax = 1;
					else
						eax = 0;
						
                    ebp_8 = eax;

					eax = (A14->orto_x - esi->orto_x) * (A10->orto_y - esi->orto_y);				
					int edx2 = (A10->orto_x - esi->orto_x) * (A14->orto_y - esi->orto_y);

					if (eax == edx2)
						eax = 1;
					else
						eax = 0;

					edx2 = ebp_4 *16;
                    ecx = (edx2 + (int)DATA_007854);
					ebx->orto_x += *(int*)(ecx + ebp_8 * 8 + eax * 4);
					ebx->orto_y += *(int*)(edx2 + (int)DATA_007855 + ebp_8 * 8 + eax * 4);
                }
				shiftCall2 = (int(*)(short)) *(int*)DATA_009280;
				ebx->orto_x_2 = (short)shiftCall2(ebx->orto_x);
				shiftCall2 = (int(*)(short)) *(int*)DATA_009281;
				ebx->orto_y_2 = (short)shiftCall2(ebx->orto_y);
				
            }
        } else {
			eax = C_FUNC_001521_MulWithNormalize(ebx->normal_x, (int)edi);
			ebx->nx = eax + ((esi->nx + A10->nx) >> 1);
			eax = C_FUNC_001521_MulWithNormalize(ebx->normal_y, (int)edi);
			ebx->ny = eax + ((esi->ny + A10->ny) >> 1);
			eax = C_FUNC_001521_MulWithNormalize(ebx->normal_z, (int)edi);
			ebx->nz = eax + ((esi->nz + A10->nz) >> 1);
            //*L006c7348(ebx); // call near [DATA_009282]
			shiftCall = (char *(*)(int)) *(int*)(DATA_009282); // pointer to func
			shiftCall((int)ebx);
        }
        C_FUNC_001829_GetTriangulateDepth(ebx);
    }
	return 0;
}
//fix me
extern "C" int C_FUNC_001850(int* A8, ffeVertex* Ac, ffeVertex* A10, ffeVertex* A14, int A18)
{
    int		eax;
	ffeVertex*		ebx;
    int		ecx;
	ffeVertex*		edx;
	int*		edi;

	int		ebp_4;
	int		ebp_8;

    edi = A8;
	eax = *(edi+2);
    if(eax == 0) {
        eax = C_FUNC_001818_ArrayRemoveFirstElement();
        *(int*)(edi+2) = eax;
        ebx = (ffeVertex *)*(int*)(edi+2);
        if(Ac == (ffeVertex*)*edi) {
            eax = (int)C_FUNC_001820_ArrayCreateNewElement();
            *(int*)(edi+1) = eax;
            *(int*)*(int*)(edi+1) = (int)Ac;
            eax = (int)C_FUNC_001820_ArrayCreateNewElement();
            *(int*)(edi+3) = eax;
            *(int*)eax = (int)A10;
        } else {
            eax = (int)C_FUNC_001820_ArrayCreateNewElement();
            *(int*)(edi+1) = eax;
            *(int*)eax = (int)A10;
            eax = (int)C_FUNC_001820_ArrayCreateNewElement();
            *(int*)(edi+3) = eax;
            *(int*)*(int*)(edi+3) = (int)Ac;
        }
		ebx->normal_x = Ac->normal_x;
		ebx->normal_y = Ac->normal_y;
		ebx->normal_z = Ac->normal_z;
		edi=0;
		if(A18 > Ac->z_2 && A18 > A10->z_2) {
			eax = Ac->x_2;
			ebx->x_2 = (eax + A10->x_2) >> 1;
			eax = Ac->nx;
			ebx->nx = (eax + A10->nx) >> 1;
			eax = Ac->ny;
			ebx->ny = (eax + A10->ny) >> 1;
			eax = Ac->nz;
			ebx->nz = (eax + A10->nz) >> 1;
			if(Ac->nz < 0x40 || A10->nz < 0x40) {
                //*L006c7348(ebx); //call near [DATA_009282]
				shiftCall = (char *(*)(int)) *(int*)(DATA_009282); // pointer to func
				shiftCall((int)ebx);

            } else {

				ebx->orto_x = (Ac->orto_x + A10->orto_x) >> 1;
				ebx->orto_y = (Ac->orto_y + A10->orto_y) >> 1;
				eax = (int)A14;

				if(((ffeVertex*)eax)->nz > 0x40) {
					eax = Ac->orto_y;
					edx = A10;
					if (eax == edx->orto_y)
						eax = 1;
					else
						eax = 0;
						
                    ebp_8 = eax;

                    eax = Ac->orto_x;
					edx = A10;
					if (eax == edx->orto_x)
						eax = 1;
					else
						eax = 0;
						
                    ebp_4 = eax;

					eax = (A14->orto_x - Ac->orto_x) * (A10->orto_y - Ac->orto_y);				
					int edx2 = (A10->orto_x - Ac->orto_x) * (A14->orto_y - Ac->orto_y);

					if (eax == edx2)
						eax = 1;
					else
						eax = 0;

					edx2 = ebp_8 * 16;
                    ecx = (edx2 + (int)DATA_007856);
					ebx->orto_x += *(int*)(ecx + ebp_8 * 8 + eax * 4);
					ebx->orto_y += *(int*)(edx2 + (int)DATA_007857 + ebp_8 * 8 + eax * 4);
                }
				shiftCall2 = (int(*)(short)) *(int*)DATA_009280;
				ebx->orto_x_2 = (short)shiftCall2(ebx->orto_x);
				shiftCall2 = (int(*)(short)) *(int*)DATA_009281;
				ebx->orto_y_2 = (short)shiftCall2(ebx->orto_y);

            }
        } else {
			int ebp_12 = (Ac->x_2 + A10->x_2) >> 1;
			ebx->x_2 = C_FUNC_001846( Ac->x_2, A10->x_2, A18) + ebp_12;

			eax = C_FUNC_001521_MulWithNormalize(ebx->normal_x, (int)edi);
			ebx->nx = eax + ((Ac->nx + A10->nx) >> 1);
			eax = C_FUNC_001521_MulWithNormalize(ebx->normal_y, (int)edi);
			ebx->ny = eax + ((Ac->ny + A10->ny) >> 1);
			eax = C_FUNC_001521_MulWithNormalize(ebx->normal_z, (int)edi);
			ebx->nz = eax + ((Ac->nz + A10->nz) >> 1);
            //*L006c7348(ebx); // call near [DATA_009282]
			shiftCall = (char *(*)(int)) *(int*)(DATA_009282); // pointer to func
			shiftCall((int)ebx);
        }
        C_FUNC_001829_GetTriangulateDepth(ebx);
    }
	return 0;
}

extern "C" int C_FUNC_001851(int* A8, ffeVertex* Ac, ffeVertex* A10, ffeVertex* A14, int A18)
{
	int result; // eax@1
	int v6; // edi@1
	int v7; // esi@1
	int v8; // ebx@2
	int v9; // eax@6
	int v10; // eax@2
	int v11; // eax@3
	int v12; // eax@3
	int v13; // eax@4
	int v14; // eax@4
	bool v15; // eax@14
	bool v17; // [sp+10h] [bp-4h]@14
	bool v18; // [sp+Ch] [bp-8h]@14

	v7 = (int)Ac;
	v6 = (int)A8;
	result = *(int *)((int)A8 + 8);
	if ( !result )
	{
		v10 = (int)C_FUNC_001818_ArrayRemoveFirstElement();
		*(int *)(v6 + 8) = v10;
		v8 = v10;
		if ( v7 == *(int *)v6 )
		{
			v11 = (int)C_FUNC_001820_ArrayCreateNewElement();
			*(int *)(v6 + 4) = v11;
			*(int *)v11 = v7;
			v12 = (int)C_FUNC_001820_ArrayCreateNewElement();
			*(int *)(v6 + 12) = v12;
			*(int *)v12 = (int)A10;
		}
		else
		{
			v13 = (int)C_FUNC_001820_ArrayCreateNewElement();
			*(int *)(v6 + 4) = v13;
			*(int *)v13 = (int)A10;
			v14 = (int)C_FUNC_001820_ArrayCreateNewElement();
			*(int *)(v6 + 12) = v14;
			*(int *)v14 = v7;
		}
		*(int *)(v8 + 40) = *(int *)(v7 + 40);
		*(int *)(v8 + 44) = *(int *)(v7 + 44);
		*(int *)(v8 + 48) = *(int *)(v7 + 48);
		if ( ((*(int *)((int)A10 + 60) - 1) ^ (*(int *)(v7 + 60) - 1)) >= 0 )
			v9 = A18;
		else
			v9 = A18 - 1;
		if ( v9 <= *(int *)(v7 + 68) || v9 <= *(int *)((int)A10 + 68) )
		{
			*(int *)(v8 + 60) = ((*(int *)((int)A10 + 60) + *(int *)(v7 + 60)) >> 1)
				+ C_FUNC_001846(*(int *)(v7 + 60), *(int *)((int)A10 + 60), A18);
			*(int *)(v8 + 4) = ((*(int *)((int)A10 + 4) + *(int *)(v7 + 4)) >> 1) + C_FUNC_001521_MulWithNormalize(*(int *)(v8 + 40), 0);
			*(int *)(v8 + 8) = ((*(int *)((int)A10 + 8) + *(int *)(v7 + 8)) >> 1) + C_FUNC_001521_MulWithNormalize(*(int *)(v8 + 44), 0);
			*(int *)(v8 + 12) = ((*(int *)((int)A10 + 12) + *(int *)(v7 + 12)) >> 1) + C_FUNC_001521_MulWithNormalize(*(int *)(v8 + 48), 0);
			shiftCall = (char *(*)(int)) *(int*)(DATA_009282);
			shiftCall((int)v8);
		}
		else
		{
			*(int *)(v8 + 60) = (*(int *)((int)A10 + 60) + *(int *)(v7 + 60)) >> 1;
			*(int *)(v8 + 4) = (*(int *)((int)A10 + 4) + *(int *)(v7 + 4)) >> 1;
			*(int *)(v8 + 8) = (*(int *)((int)A10 + 8) + *(int *)(v7 + 8)) >> 1;
			*(int *)(v8 + 12) = (*(int *)((int)A10 + 12) + *(int *)(v7 + 12)) >> 1;
			if ( *(int *)(v7 + 12) >= 64 && *(int *)((int)A10 + 12) >= 64 )
			{
				*(short *)v8 = (*(short *)(int)A10 + (signed int)*(short *)v7) >> 1;
				*(short *)(v8 + 2) = (*(short *)((int)A10 + 2) + (signed int)*(short *)(v7 + 2)) >> 1;
				if ( *(int *)((int)A14 + 12) > 64 )
				{
					v17 = *(short *)(v7 + 2) > *(short *)((int)A10 + 2);
					v18 = *(short *)v7 > *(short *)(int)A10;
					v15 = (*(short *)((int)A10 + 2) - (signed int)*(short *)(v7 + 2)) * (*(short *)(int)A14 - (signed int)*(short *)v7) > (*(short *)((int)A14 + 2) - (signed int)*(short *)(v7 + 2)) * (*(short *)(int)A10 - (signed int)*(short *)v7);
					((ffeVertex*)v8)->orto_x += *(int*)((int)DATA_007858 + v17 * 16 + v18 * 8 + v15 * 4);
					((ffeVertex*)v8)->orto_y += *(int*)((int)DATA_007859 + v17 * 16 + v18 * 8 + v15 * 4);
				}
				shiftCall2 = (int(*)(short)) *(int*)DATA_009280;
				((ffeVertex*)v8)->orto_x_2 = (short)shiftCall2(((ffeVertex*)v8)->orto_x);
				shiftCall2 = (int(*)(short)) *(int*)DATA_009281;
				((ffeVertex*)v8)->orto_y_2 = (short)shiftCall2(((ffeVertex*)v8)->orto_y);
			}
			else
			{
				shiftCall = (char *(*)(int)) *(int*)(DATA_009282);
				shiftCall((int)v8);
			}
		}
		C_FUNC_001829_GetTriangulateDepth((ffeVertex*)v8);
	}
	return 0;
}

extern "C" int C_FUNC_001852(int* A8, ffeVertex* Ac, ffeVertex* A10, ffeVertex* A14, int A18)
{
	int result; // eax@1
	int v6; // edi@1
	int v7; // esi@1
	int v8; // ebx@2
	int v9; // eax@2
	int v10; // eax@3
	int v11; // eax@3
	int v12; // eax@4
	int v13; // eax@4
	bool v14; // eax@11
	int v16; // edi@13
	int v17; // eax@13
	int v18; // edi@13
	int v19; // [sp+14h] [bp-4h]@5
	bool v20; // [sp+10h] [bp-8h]@11
	bool v21; // [sp+Ch] [bp-Ch]@11

	v7 = (int)Ac;
	v6 = (int)A8;
	result = *(int *)((int)A8 + 8);
	if ( !result )
	{
		v9 = (int)C_FUNC_001818_ArrayRemoveFirstElement();
		*(int *)(v6 + 8) = v9;
		v8 = v9;
		if ( v7 == *(int *)v6 )
		{
			v10 = (int)C_FUNC_001820_ArrayCreateNewElement();
			*(int *)(v6 + 4) = v10;
			*(int *)v10 = v7;
			v11 = (int)C_FUNC_001820_ArrayCreateNewElement();
			*(int *)(v6 + 12) = v11;
			*(int *)v11 = (int)A10;
		}
		else
		{
			v12 = (int)C_FUNC_001820_ArrayCreateNewElement();
			*(int *)(v6 + 4) = v12;
			*(int *)v12 = (int)A10;
			v13 = (int)C_FUNC_001820_ArrayCreateNewElement();
			*(int *)(v6 + 12) = v13;
			*(int *)v13 = v7;
		}
		*(int *)(v8 + 40) = *(int *)(v7 + 40);
		*(int *)(v8 + 44) = *(int *)(v7 + 44);
		*(int *)(v8 + 48) = *(int *)(v7 + 48);
		v19 = 0;
		if ( A18 <= *(int *)(v7 + 68) || A18 <= *(int *)((int)A10 + 68) )
		{
			v16 = (*(int *)((int)A10 + 60) + *(int *)(v7 + 60)) >> 1;
			*(int *)(v8 + 60) = v16 + C_FUNC_001846(*(int *)(v7 + 60), *(int *)(A10 + 60), A18);
			v17 = v19 + ((*(int *)(v8 + 60) - v16) >> 3);
			v18 = v19 + ((*(int *)(v8 + 60) - v16) >> 3);
			*(int *)(v8 + 4) = ((*(int *)((int)A10 + 4) + *(int *)(v7 + 4)) >> 1) + C_FUNC_001521_MulWithNormalize(*(int *)(v8 + 40), v17);
			*(int *)(v8 + 8) = ((*(int *)((int)A10 + 8) + *(int *)(v7 + 8)) >> 1) + C_FUNC_001521_MulWithNormalize(*(int *)(v8 + 44), v18);
			*(int *)(v8 + 12) = ((*(int *)((int)A10 + 12) + *(int *)(v7 + 12)) >> 1)
				+ C_FUNC_001521_MulWithNormalize(*(int *)(v8 + 48), v18);
			shiftCall = (char *(*)(int)) *(int*)(DATA_009282);
			shiftCall((int)v8);
		}
		else
		{
			*(int *)(v8 + 60) = (*(int *)((int)A10 + 60) + *(int *)(v7 + 60)) >> 1;
			*(int *)(v8 + 4) = (*(int *)((int)A10 + 4) + *(int *)(v7 + 4)) >> 1;
			*(int *)(v8 + 8) = (*(int *)((int)A10 + 8) + *(int *)(v7 + 8)) >> 1;
			*(int *)(v8 + 12) = (*(int *)((int)A10 + 12) + *(int *)(v7 + 12)) >> 1;
			if ( *(int *)(v7 + 12) >= 64 && *(int *)((int)A10 + 12) >= 64 )
			{
				*(short *)v8 = (*(short *)(int)A10 + (signed int)*(short *)v7) >> 1;
				*(short *)(v8 + 2) = (*(short *)((int)A10 + 2) + (signed int)*(short *)(v7 + 2)) >> 1;
				if ( *(int *)((int)A14 + 12) > 64 )
				{
					v20 = *(short *)(v7 + 2) > *(short *)((int)A10 + 2);
					v21 = *(short *)v7 > *(short *)(int)A10;
					v14 = (*(short *)((int)A10 + 2) - (signed int)*(short *)(v7 + 2)) * (*(short *)(int)A14 - (signed int)*(short *)v7) > (*(short *)((int)A14 + 2) - (signed int)*(short *)(v7 + 2)) * (*(short *)(int)A10 - (signed int)*(short *)v7);
					((ffeVertex*)v8)->orto_x += *(int*)((int)DATA_007860 + v20 * 16 + v21 * 8 + v14 * 4);
					((ffeVertex*)v8)->orto_y += *(int*)((int)DATA_007861 + v20 * 16 + v21 * 8 + v14 * 4);
				}
				shiftCall2 = (int(*)(short)) *(int*)DATA_009280;
				((ffeVertex*)v8)->orto_x_2 = (short)shiftCall2(((ffeVertex*)v8)->orto_x);
				shiftCall2 = (int(*)(short)) *(int*)DATA_009281;
				((ffeVertex*)v8)->orto_y_2 = (short)shiftCall2(((ffeVertex*)v8)->orto_y);
			}
			else
			{
				shiftCall = (char *(*)(int)) *(int*)(DATA_009282);
				shiftCall((int)v8);
			}
		}
		C_FUNC_001829_GetTriangulateDepth((ffeVertex*)v8);
	}
	return 0;
}


extern "C" int C_FUNC_001853(int* A8, ffeVertex* Ac, ffeVertex* A10, ffeVertex* A14, int A18)
{
	int result; // eax@1
	int v6; // edi@1
	int v7; // esi@1
	int v8; // ebx@2
	int v9; // eax@6
	int v10; // eax@10
	int v11; // eax@21
	int v12; // eax@2
	int v13; // eax@3
	int v14; // eax@3
	int v15; // eax@4
	int v16; // eax@4
	int v17; // eax@12
	bool v18; // eax@18
	int v20; // edi@23
	bool v21; // [sp+10h] [bp-4h]@18
	bool v22; // [sp+Ch] [bp-8h]@18

	v7 = (int)Ac;
	v6 = (int)A8;
	result = *(int *)((int)A8 + 8);
	if ( !result )
	{
		v12 = (int)C_FUNC_001818_ArrayRemoveFirstElement();
		*(int *)(v6 + 8) = v12;
		v8 = v12;
		if ( v7 == *(int *)v6 )
		{
			v13 = (int)C_FUNC_001820_ArrayCreateNewElement();
			*(int *)(v6 + 4) = v13;
			*(int *)v13 = v7;
			v14 = (int)C_FUNC_001820_ArrayCreateNewElement();
			*(int *)(v6 + 12) = v14;
			*(int *)v14 = (int)A10;
		}
		else
		{
			v15 = (int)C_FUNC_001820_ArrayCreateNewElement();
			*(int *)(v6 + 4) = v15;
			*(int *)v15 = (int)A10;
			v16 = (int)C_FUNC_001820_ArrayCreateNewElement();
			*(int *)(v6 + 12) = v16;
			*(int *)v16 = v7;
		}
		*(int *)(v8 + 40) = *(int *)(v7 + 40);
		*(int *)(v8 + 44) = *(int *)(v7 + 44);
		*(int *)(v8 + 48) = *(int *)(v7 + 48);
		if ( ((*(int *)((int)A10 + 60) - 1) ^ (*(int *)(v7 + 60) - 1)) >= 0 )
			v9 = (int)A18;
		else
			v9 = (int)A18 - 1;
		if ( v9 <= *(int *)(v7 + 68) || v9 <= *(int *)((int)A10 + 68) )
		{
			*(int *)(v8 + 60) = ((*(int *)((int)A10 + 60) + *(int *)(v7 + 60)) >> 1)
				+ C_FUNC_001846(*(int *)(v7 + 60), *(int *)((int)A10 + 60), A18);
			if ( *(int *)(v8 + 60) < 0 )
				v11 = 0;
			else
				v11 = *(int *)(v8 + 60);
			*(int *)(v8 + 64) = v11;
			v20 = (v11 - ((*(int *)((int)A10 + 64) + *(int *)(v7 + 64)) >> 1)) >> 3;
			*(int *)(v8 + 4) = ((*(int *)((int)A10 + 4) + *(int *)(v7 + 4)) >> 1)
				+ C_FUNC_001521_MulWithNormalize(
				*(int *)(v8 + 40),
				(v11 - ((*(int *)((int)A10 + 64) + *(int *)(v7 + 64)) >> 1)) >> 3);
			*(int *)(v8 + 8) = ((*(int *)((int)A10 + 8) + *(int *)(v7 + 8)) >> 1) + C_FUNC_001521_MulWithNormalize(*(int *)(v8 + 44), v20);
			*(int *)(v8 + 12) = ((*(int *)((int)A10 + 12) + *(int *)(v7 + 12)) >> 1)
				+ C_FUNC_001521_MulWithNormalize(*(int *)(v8 + 48), v20);
			shiftCall = (char *(*)(int)) *(int*)(DATA_009282);
			shiftCall((int)v8);
		}
		else
		{
			v10 = *(int *)(v7 + 64);
			if ( v10 || *(int *)((int)A10 + 64) )
			{
				v17 = (*(int *)((int)A10 + 64) + v10) >> 1;
				*(int *)(v8 + 64) = v17;
				*(int *)(v8 + 60) = v17;
			}
			else
			{
				*(int *)(v8 + 60) = (*(int *)((int)A10 + 60) + *(int *)(v7 + 60)) >> 1;
				*(int *)(v8 + 64) = 0;
			}
			*(int *)(v8 + 4) = (*(int *)((int)A10 + 4) + *(int *)(v7 + 4)) >> 1;
			*(int *)(v8 + 8) = (*(int *)((int)A10 + 8) + *(int *)(v7 + 8)) >> 1;
			*(int *)(v8 + 12) = (*(int *)((int)A10 + 12) + *(int *)(v7 + 12)) >> 1;
			if ( *(int *)(v7 + 12) >= 64 && *(int *)((int)A10 + 12) >= 64 )
			{
				*(short *)v8 = (*(short *)(int)A10 + (signed int)*(short *)v7) >> 1;
				*(short *)(v8 + 2) = (*(short *)((int)A10 + 2) + (signed int)*(short *)(v7 + 2)) >> 1;
				if ( *(int *)((int)A14 + 12) > 64 )
				{
					v21 = *(short *)(v7 + 2) > *(short *)((int)A10 + 2);
					v22 = *(short *)v7 > *(short *)(int)A10;
					v18 = (*(short *)((int)A10 + 2) - (signed int)*(short *)(v7 + 2)) * (*(short *)(int)A14 - (signed int)*(short *)v7) > (*(short *)((int)A14 + 2) - (signed int)*(short *)(v7 + 2)) * (*(short *)(int)A10 - (signed int)*(short *)v7);
					((ffeVertex*)v8)->orto_x += *(int*)((int)DATA_007862 + v21 * 16 + v22 * 8 + v18 * 4);
					((ffeVertex*)v8)->orto_y += *(int*)((int)DATA_007863 + v21 * 16 + v22 * 8 + v18 * 4);
				}
				shiftCall2 = (int(*)(short)) *(int*)DATA_009280;
				((ffeVertex*)v8)->orto_x_2 = (short)shiftCall2(((ffeVertex*)v8)->orto_x);
				shiftCall2 = (int(*)(short)) *(int*)DATA_009281;
				((ffeVertex*)v8)->orto_y_2 = (short)shiftCall2(((ffeVertex*)v8)->orto_y);
			}
			else
			{
				shiftCall = (char *(*)(int)) *(int*)(DATA_009282);
				shiftCall((int)v8);
			}
		}
		C_FUNC_001829_GetTriangulateDepth((ffeVertex*)v8);
	}
	return 0;
}

extern "C" int C_FUNC_001854(int* A8, ffeVertex* Ac, ffeVertex* A10, ffeVertex* A14, int A18)
{
	int result; // eax@1
	int v6; // edi@1
	int v7; // esi@1
	int v8; // ebx@2
	int v9; // eax@2
	int v10; // eax@3
	int v11; // eax@3
	int v12; // eax@4
	int v13; // eax@4
	bool v14; // eax@11
	int v16; // [sp+18h] [bp-4h]@5
	bool v17; // [sp+10h] [bp-Ch]@11
	bool v18; // [sp+Ch] [bp-10h]@11
	int v19; // [sp+14h] [bp-8h]@13

	v6 = (int)A18;
	v7 = (int)Ac;
	result = *(int *)((int)A8 + 8);
	if ( !result )
	{
		v9 = (int)C_FUNC_001818_ArrayRemoveFirstElement();
		*(int *)((int)A8 + 8) = v9;
		v8 = v9;
		if ( v7 == *(int *)(int)A8 )
		{
			v10 = (int)C_FUNC_001820_ArrayCreateNewElement();
			*(int *)((int)A8 + 4) = v10;
			*(int *)v10 = v7;
			v11 = (int)C_FUNC_001820_ArrayCreateNewElement();
			*(int *)((int)A8 + 12) = v11;
			*(int *)v11 = (int)A10;
		}
		else
		{
			v12 = (int)C_FUNC_001820_ArrayCreateNewElement();
			*(int *)((int)A8 + 4) = v12;
			*(int *)v12 = (int)A10;
			v13 = (int)C_FUNC_001820_ArrayCreateNewElement();
			*(int *)((int)A8 + 12) = v13;
			*(int *)v13 = v7;
		}
		*(int *)(v8 + 40) = C_FUNC_001521_MulWithNormalize(*(int *)((int)A10 + 40) + *(int *)(v7 + 40), *(int*)(A18 * 4 + (int)DATA_007840));
		*(int *)(v8 + 44) = C_FUNC_001521_MulWithNormalize(*(int *)((int)A10 + 44) + *(int *)(v7 + 44), *(int*)(A18 * 4 + (int)DATA_007840));
		*(int *)(v8 + 48) = C_FUNC_001521_MulWithNormalize(*(int *)((int)A10 + 48) + *(int *)(v7 + 48), *(int*)(A18 * 4 + (int)DATA_007840));
		v16 = *(int*)(A18 * 4 + (int)DATA_007839);
		if ( v6 <= *(int *)(v7 + 68) || v6 <= *(int *)((int)A10 + 68) )
		{
			v19 = (*(int *)((int)A10 + 60) + *(int *)(v7 + 60)) >> 1;
			*(int *)(v8 + 60) = v19 + C_FUNC_001846(*(int *)(v7 + 60), *(int *)((int)A10 + 60), v6);
			*(int *)(v8 + 4) = ((*(int *)((int)A10 + 4) + *(int *)(v7 + 4)) >> 1) + C_FUNC_001521_MulWithNormalize(*(int *)(v8 + 40), v16);
			*(int *)(v8 + 8) = ((*(int *)((int)A10 + 8) + *(int *)(v7 + 8)) >> 1) + C_FUNC_001521_MulWithNormalize(*(int *)(v8 + 44), v16);
			*(int *)(v8 + 12) = ((*(int *)((int)A10 + 12) + *(int *)(v7 + 12)) >> 1)
				+ C_FUNC_001521_MulWithNormalize(*(int *)(v8 + 48), v16);
			shiftCall = (char *(*)(int)) *(int*)(DATA_009282);
			shiftCall((int)v8);
		}
		else
		{
			*(int *)(v8 + 60) = (*(int *)((int)A10 + 60) + *(int *)(v7 + 60)) >> 1;
			*(int *)(v8 + 4) = (*(int *)((int)A10 + 4) + *(int *)(v7 + 4)) >> 1;
			*(int *)(v8 + 8) = (*(int *)((int)A10 + 8) + *(int *)(v7 + 8)) >> 1;
			*(int *)(v8 + 12) = (*(int *)((int)A10 + 12) + *(int *)(v7 + 12)) >> 1;
			if ( *(int *)(v7 + 12) >= 64 && *(int *)((int)A10 + 12) >= 64 )
			{
				*(short *)v8 = (*(short *)(int)A10 + (signed int)*(short *)v7) >> 1;
				*(short *)(v8 + 2) = (*(short *)((int)A10 + 2) + (signed int)*(short *)(v7 + 2)) >> 1;
				if ( *(int *)((int)A14 + 12) > 64 )
				{
					v17 = *(short *)(v7 + 2) > *(short *)((int)A10 + 2);
					v18 = *(short *)v7 > *(short *)(int)A10;
					v14 = (*(short *)((int)A10 + 2) - (signed int)*(short *)(v7 + 2)) * (*(short *)(int)A14 - (signed int)*(short *)v7) > (*(short *)((int)A14 + 2) - (signed int)*(short *)(v7 + 2)) * (*(short *)(int)A10 - (signed int)*(short *)v7);
					((ffeVertex*)v8)->orto_x += *(int*)((int)DATA_007864 + v17 * 16 + v18 * 8 + v14 * 4);
					((ffeVertex*)v8)->orto_y += *(int*)((int)DATA_007865 + v17 * 16 + v18 * 8 + v14 * 4);
				}
				shiftCall2 = (int(*)(short)) *(int*)DATA_009280;
				((ffeVertex*)v8)->orto_x_2 = (short)shiftCall2(((ffeVertex*)v8)->orto_x);
				shiftCall2 = (int(*)(short)) *(int*)DATA_009281;
				((ffeVertex*)v8)->orto_y_2 = (short)shiftCall2(((ffeVertex*)v8)->orto_y);
			}
			else
			{
				shiftCall = (char *(*)(int)) *(int*)(DATA_009282);
				shiftCall((int)v8);
			}
		}
		C_FUNC_001829_GetTriangulateDepth((ffeVertex*)v8);
	}
	return 0;
}

extern "C" int C_FUNC_001855(int* A8, ffeVertex* Ac, ffeVertex* A10, ffeVertex* A14, int A18)
{
	int result; // eax@1
	int v6; // edi@1
	int v7; // esi@1
	int v8; // ebx@2
	int v9; // edi@5
	int v10; // eax@6
	int v11; // eax@2
	int v12; // eax@3
	int v13; // eax@3
	int v14; // eax@4
	int v15; // eax@4
	bool v16; // eax@14
	bool v18; // [sp+10h] [bp-4h]@14
	bool v19; // [sp+Ch] [bp-8h]@14

	v7 = (int)Ac;
	v6 = (int)A8;
	result = *(int *)((int)A8 + 8);
	if ( !result )
	{
		v11 = (int)C_FUNC_001818_ArrayRemoveFirstElement();
		*(int *)(v6 + 8) = v11;
		v8 = v11;
		if ( v7 == *(int *)v6 )
		{
			v12 = (int)C_FUNC_001820_ArrayCreateNewElement();
			*(int *)(v6 + 4) = v12;
			*(int *)v12 = v7;
			v13 = (int)C_FUNC_001820_ArrayCreateNewElement();
			*(int *)(v6 + 12) = v13;
			*(int *)v13 = (int)A10;
		}
		else
		{
			v14 = (int)C_FUNC_001820_ArrayCreateNewElement();
			*(int *)(v6 + 4) = v14;
			*(int *)v14 = (int)A10;
			v15 = (int)C_FUNC_001820_ArrayCreateNewElement();
			*(int *)(v6 + 12) = v15;
			*(int *)v15 = v7;
		}
		*(int *)(v8 + 40) = C_FUNC_001521_MulWithNormalize(*(int *)((int)A10 + 40) + *(int *)(v7 + 40), *(int*)(A18 * 4 + (int)DATA_007840));
		*(int *)(v8 + 44) = C_FUNC_001521_MulWithNormalize(*(int *)((int)A10 + 44) + *(int *)(v7 + 44), *(int*)(A18 * 4 + (int)DATA_007840));
		*(int *)(v8 + 48) = C_FUNC_001521_MulWithNormalize(*(int *)((int)A10 + 48) + *(int *)(v7 + 48), *(int*)(A18 * 4 + (int)DATA_007840));
		v9 = *(int*)(A18 * 4 + (int)DATA_007839);
		if ( ((*(int *)((int)A10 + 60) - 1) ^ (*(int *)(v7 + 60) - 1)) >= 0 )
			v10 = (int)A18;
		else
			v10 = (int)A18 - 1;
		if ( v10 <= *(int *)(v7 + 68) || v10 <= *(int *)((int)A10 + 68) )
		{
			*(int *)(v8 + 60) = ((*(int *)((int)A10 + 60) + *(int *)(v7 + 60)) >> 1)
				+ C_FUNC_001846(*(int *)(v7 + 60), *(int *)((int)A10 + 60), A18);
			*(int *)(v8 + 4) = ((*(int *)((int)A10 + 4) + *(int *)(v7 + 4)) >> 1) + C_FUNC_001521_MulWithNormalize(*(int *)(v8 + 40), v9);
			*(int *)(v8 + 8) = ((*(int *)((int)A10 + 8) + *(int *)(v7 + 8)) >> 1) + C_FUNC_001521_MulWithNormalize(*(int *)(v8 + 44), v9);
			*(int *)(v8 + 12) = ((*(int *)((int)A10 + 12) + *(int *)(v7 + 12)) >> 1)
				+ C_FUNC_001521_MulWithNormalize(*(int *)(v8 + 48), v9);
			shiftCall = (char *(*)(int)) *(int*)(DATA_009282);
			shiftCall((int)v8);
		}
		else
		{
			*(int *)(v8 + 60) = (*(int *)((int)A10 + 60) + *(int *)(v7 + 60)) >> 1;
			*(int *)(v8 + 4) = (*(int *)((int)A10 + 4) + *(int *)(v7 + 4)) >> 1;
			*(int *)(v8 + 8) = (*(int *)((int)A10 + 8) + *(int *)(v7 + 8)) >> 1;
			*(int *)(v8 + 12) = (*(int *)((int)A10 + 12) + *(int *)(v7 + 12)) >> 1;
			if ( *(int *)(v7 + 12) >= 64 && *(int *)((int)A10 + 12) >= 64 )
			{
				*(short *)v8 = (*(short *)(int)A10 + (signed int)*(short *)v7) >> 1;
				*(short *)(v8 + 2) = (*(short *)((int)A10 + 2) + (signed int)*(short *)(v7 + 2)) >> 1;
				if ( *(int *)((int)A14 + 12) > 64 )
				{
					v18 = *(short *)(v7 + 2) > *(short *)((int)A10 + 2);
					v19 = *(short *)v7 > *(short *)(int)A10;
					v16 = (*(short *)((int)A10 + 2) - (signed int)*(short *)(v7 + 2)) * (*(short *)(int)A14 - (signed int)*(short *)v7) > (*(short *)((int)A14 + 2) - (signed int)*(short *)(v7 + 2)) * (*(short *)(int)A10 - (signed int)*(short *)v7);
					((ffeVertex*)v8)->orto_x += *(int*)((int)DATA_007866 + v18 * 16 + v19 * 8 + v16 * 4);
					((ffeVertex*)v8)->orto_y += *(int*)((int)DATA_007867 + v18 * 16 + v19 * 8 + v16 * 4);
				}
				shiftCall2 = (int(*)(short)) *(int*)DATA_009280;
				((ffeVertex*)v8)->orto_x_2 = (short)shiftCall2(((ffeVertex*)v8)->orto_x);
				shiftCall2 = (int(*)(short)) *(int*)DATA_009281;
				((ffeVertex*)v8)->orto_y_2 = (short)shiftCall2(((ffeVertex*)v8)->orto_y);
			}
			else
			{
				shiftCall = (char *(*)(int)) *(int*)(DATA_009282);
				shiftCall((int)v8);
			}
		}
		C_FUNC_001829_GetTriangulateDepth((ffeVertex*)v8);
	}
	return 0;
}

extern "C" int C_FUNC_001856(int* A8, ffeVertex* Ac, ffeVertex* A10, ffeVertex* A14, int A18)
{
	int result; // eax@1
	int v6; // edi@1
	int v7; // esi@1
	int v8; // ebx@2
	int v9; // eax@2
	int v10; // eax@3
	int v11; // eax@3
	int v12; // eax@4
	int v13; // eax@4
	bool v14; // eax@11
	int v16; // eax@13
	int v17; // [sp+18h] [bp-4h]@5
	bool v18; // [sp+10h] [bp-Ch]@11
	bool v19; // [sp+Ch] [bp-10h]@11
	int v20; // [sp+14h] [bp-8h]@13

	v6 = (int)A18;
	v7 = (int)Ac;
	result = *(int *)((int)A8 + 8);
	if ( !result )
	{
		v9 = (int)C_FUNC_001818_ArrayRemoveFirstElement();
		*(int *)((int)A8 + 8) = v9;
		v8 = v9;
		if ( v7 == *(int *)(int)A8 )
		{
			v10 = (int)C_FUNC_001820_ArrayCreateNewElement();
			*(int *)((int)A8 + 4) = v10;
			*(int *)v10 = v7;
			v11 = (int)C_FUNC_001820_ArrayCreateNewElement();
			*(int *)((int)A8 + 12) = v11;
			*(int *)v11 = (int)A10;
		}
		else
		{
			v12 = (int)C_FUNC_001820_ArrayCreateNewElement();
			*(int *)((int)A8 + 4) = v12;
			*(int *)v12 = (int)A10;
			v13 = (int)C_FUNC_001820_ArrayCreateNewElement();
			*(int *)((int)A8 + 12) = v13;
			*(int *)v13 = v7;
		}
		*(int *)(v8 + 40) = C_FUNC_001521_MulWithNormalize(*(int *)((int)A10 + 40) + *(int *)(v7 + 40), *(int*)(A18 * 4 + (int)DATA_007840));
		*(int *)(v8 + 44) = C_FUNC_001521_MulWithNormalize(*(int *)((int)A10 + 44) + *(int *)(v7 + 44), *(int*)(A18 * 4 + (int)DATA_007840));
		*(int *)(v8 + 48) = C_FUNC_001521_MulWithNormalize(*(int *)((int)A10 + 48) + *(int *)(v7 + 48), *(int*)(A18 * 4 + (int)DATA_007840));
		v17 = *(int*)(A18 * 4 + (int)DATA_007839);
		if ( v6 <= *(int *)(v7 + 68) || v6 <= *(int *)((int)A10 + 68) )
		{
			v20 = (*(int *)((int)A10 + 60) + *(int *)(v7 + 60)) >> 1;
			*(int *)(v8 + 60) = v20 + C_FUNC_001846(*(int *)(v7 + 60), *(int *)((int)A10 + 60), v6);
			v16 = (*(int *)(v8 + 60) - v20) >> 3;
			v20 = v17 + v16;
			*(int *)(v8 + 4) = ((*(int *)((int)A10 + 4) + *(int *)(v7 + 4)) >> 1)
				+ C_FUNC_001521_MulWithNormalize(*(int *)(v8 + 40), v17 + v16);
			*(int *)(v8 + 8) = ((*(int *)((int)A10 + 8) + *(int *)(v7 + 8)) >> 1) + C_FUNC_001521_MulWithNormalize(*(int *)(v8 + 44), v20);
			*(int *)(v8 + 12) = ((*(int *)((int)A10 + 12) + *(int *)(v7 + 12)) >> 1)
				+ C_FUNC_001521_MulWithNormalize(*(int *)(v8 + 48), v20);
			shiftCall = (char *(*)(int)) *(int*)(DATA_009282);
			shiftCall((int)v8);
		}
		else
		{
			*(int *)(v8 + 60) = (*(int *)((int)A10 + 60) + *(int *)(v7 + 60)) >> 1;
			*(int *)(v8 + 4) = (*(int *)((int)A10 + 4) + *(int *)(v7 + 4)) >> 1;
			*(int *)(v8 + 8) = (*(int *)((int)A10 + 8) + *(int *)(v7 + 8)) >> 1;
			*(int *)(v8 + 12) = (*(int *)((int)A10 + 12) + *(int *)(v7 + 12)) >> 1;
			if ( *(int *)(v7 + 12) >= 64 && *(int *)(A10 + 12) >= 64 )
			{
				*(short *)v8 = (*(short *)(int)A10 + (signed int)*(short *)v7) >> 1;
				*(short *)(v8 + 2) = (*(short *)((int)A10 + 2) + (signed int)*(short *)(v7 + 2)) >> 1;
				if ( *(int *)((int)A14 + 12) > 64 )
				{
					v18 = *(short *)(v7 + 2) > *(short *)((int)A10 + 2);
					v19 = *(short *)v7 > *(short *)(int)A10;
					v14 = (*(short *)((int)A10 + 2) - (signed int)*(short *)(v7 + 2)) * (*(short *)(int)A14 - (signed int)*(short *)v7) > (*(short *)((int)A14 + 2) - (signed int)*(short *)(v7 + 2)) * (*(short *)(int)A10 - (signed int)*(short *)v7);
					((ffeVertex*)v8)->orto_x += *(int*)((int)DATA_007868 + v18 * 16 + v19 * 8 + v14 * 4);
					((ffeVertex*)v8)->orto_y += *(int*)((int)DATA_007869 + v18 * 16 + v19 * 8 + v14 * 4);
				}
				shiftCall2 = (int(*)(short)) *(int*)DATA_009280;
				((ffeVertex*)v8)->orto_x_2 = (short)shiftCall2(((ffeVertex*)v8)->orto_x);
				shiftCall2 = (int(*)(short)) *(int*)DATA_009281;
				((ffeVertex*)v8)->orto_y_2 = (short)shiftCall2(((ffeVertex*)v8)->orto_y);
			}
			else
			{
				shiftCall = (char *(*)(int)) *(int*)(DATA_009282);
				shiftCall((int)v8);
			}
		}
		C_FUNC_001829_GetTriangulateDepth((ffeVertex*)v8);
	}
	return 0;
}

extern "C" int C_FUNC_001857(int* A8, ffeVertex* Ac, ffeVertex* A10, ffeVertex* A14, int A18)
{
	int result; // eax@1
	int v6; // edi@1
	ffeVertex* v8; // ebx@2
	int v9; // edi@5
	int v10; // eax@6
	int v12; // eax@21
	int v13; // eax@2
	int v14; // eax@3
	int v15; // eax@3
	int v16; // eax@4
	int v17; // eax@4
	int v18; // eax@12
	bool v19; // eax@18
	int v21; // edi@23
	bool v22; // [sp+10h] [bp-4h]@18
	bool v23; // [sp+Ch] [bp-8h]@18

	v6 = (int)A8;
	result = *(int *)(v6 + 8);
	if ( !result )
	{
		v13 = (int)C_FUNC_001818_ArrayRemoveFirstElement();
		*(int *)(v6 + 8) = v13;
		v8 = (ffeVertex*)v13;
		if ((int)Ac == *(int *)v6 )
		{
			v14 = (int)C_FUNC_001820_ArrayCreateNewElement();
			*(int *)(v6 + 4) = v14;
			*(int *)v14 = (int)Ac;
			v15 = (int)C_FUNC_001820_ArrayCreateNewElement();
			*(int *)(v6 + 12) = v15;
			*(int *)v15 = (int)A10;
		}
		else
		{
			v16 = (int)C_FUNC_001820_ArrayCreateNewElement();
			*(int *)(v6 + 4) = v16;
			*(int *)v16 = (int)A10;
			v17 = (int)C_FUNC_001820_ArrayCreateNewElement();
			*(int *)(v6 + 12) = v17;
			*(int *)v17 = (int)Ac;
		}
		v8->normal_x = (int)C_FUNC_001521_MulWithNormalize(A10->normal_x + Ac->normal_x, *(int*)(A18 * 4 + (int)DATA_007840));//v8->nx_2 = 0xfffa3ce9
		v8->normal_y = (int)C_FUNC_001521_MulWithNormalize(A10->normal_y + Ac->normal_y, *(int*)(A18 * 4 + (int)DATA_007840));//v8->ny_2 = 0x000a735b
		v8->normal_z = (int)C_FUNC_001521_MulWithNormalize(A10->normal_z + Ac->normal_z, *(int*)(A18 * 4 + (int)DATA_007840));//v8->nz_2 = 0x0000e7c7
		v9 = *(int*)(A18 * 4 + (int)DATA_007839);//v9 = 0x00500a5d

		if ( ((A10->x_2 - 1) ^ (Ac->x_2 - 1)) >= 0 )
			v10 = (int)A18;
		else
			v10 = (int)A18 - 1;
		if (v10 <= Ac->z_2 || v10 <= A10->z_2 )
		{
			v8->x_2 = ((A10->x_2 + Ac->x_2) >> 1) + C_FUNC_001846(Ac->x_2, A10->x_2, A18);//v8->x_2 = 0xfcaa8f20
			if (v8->x_2 < 0 )
				v12 = 0;
			else
				v12 = v8->x_2;
			v8->y_2 = v12;//v8->y_2 = 0x00000000
			v21 = ((v12 - ((A10->y_2 + Ac->y_2) >> 1)) >> 3) + v9;//v21 = 0xffdf019e
			v8->nx = ((A10->nx + Ac->nx) >> 1) + (int)C_FUNC_001521_MulWithNormalize(v8->normal_x, v21);//v8->nx = 0xfb64abe4
			v8->ny = ((A10->ny + Ac->ny) >> 1) + (int)C_FUNC_001521_MulWithNormalize(v8->normal_y, v21);//v8->ny = 0xfd4fb7d7
			v8->nz = ((A10->nz + Ac->nz) >> 1) + (int)C_FUNC_001521_MulWithNormalize(v8->normal_z, v21);//v8->nz = 0x00b938c8
			shiftCall = (char *(*)(int)) *(int*)(DATA_009282);
			shiftCall((int)v8);
		}
		else
		{
			if (Ac->y_2 || A10->y_2)
			{
				v18 = (A10->y_2 + Ac->y_2) >> 1;
				v8->x_2 = v18;
				v8->y_2 = v18;
			}
			else
			{
				v8->x_2 = (A10->x_2 + Ac->x_2) >> 1;
				v8->y_2 = 0;
			}
			v8->nx = (A10->nx + Ac->nx) >> 1;
			v8->ny = (A10->ny + Ac->ny) >> 1;
			v8->nz = (A10->nz + Ac->nz) >> 1;
			if (Ac->nz >= 64 && A10->nz >= 64)
			{
				v8->orto_x = (A10->orto_x + Ac->orto_x) >> 1;
				v8->orto_y = (A10->orto_y + Ac->orto_y) >> 1;
				if (A14->nz > 64 )
				{
					v22 = Ac->orto_y > A10->orto_y;
					v23 = Ac->orto_x > A10->orto_x;
					v19 = (A10->orto_y - Ac->orto_y) * (A14->orto_x - Ac->orto_x)
					 > 
					 (A14->orto_y - Ac->orto_y) * (A10->orto_x - Ac->orto_x);
					v8->orto_x += *(int*)((int)DATA_007870 + v22 * 16 + v23 * 8 + v19 * 4);
					v8->orto_y += *(int*)((int)DATA_007871 + v22 * 16 + v23 * 8 + v19 * 4);
				}
				shiftCall2 = (int(*)(short)) *(int*)DATA_009280;
				v8->orto_x_2 = (short)shiftCall2(v8->orto_x);
				shiftCall2 = (int(*)(short)) *(int*)DATA_009281;
				v8->orto_y_2 = (short)shiftCall2(v8->orto_y);
			}
			else
			{
				shiftCall = (char *(*)(int)) *(int*)(DATA_009282);
				shiftCall((int)v8);
			}
		}
		C_FUNC_001829_GetTriangulateDepth(v8);
	}
	return 0;
}

extern "C" int C_FUNC_001858(int* A8, ffeVertex* Ac, ffeVertex* A10, ffeVertex* A14, int A18)
{
	int v5; // eax@1
	int v6; // ebx@1
	int result; // eax@2
	int v8; // edx@2
	int v9; // ecx@2
	signed int v10; // esi@2
	int v11; // eax@10
	int v12; // edx@10
	int v13; // ecx@10
	signed int v14; // esi@10
	int v15; // eax@22
	int v16; // edx@22
	signed int v17; // esi@22
	int v18; // ecx@26
	int v19; // eax@7
	int v20; // eax@8
	int v21; // eax@8
	int v22; // eax@9
	int v23; // eax@9
	int v24; // eax@14
	bool v25; // eax@20
	bool v26; // edx@20
	bool v27; // ecx@20
	int v28; // [sp+10h] [bp-Ch]@2
	int v29; // [sp+18h] [bp-4h]@7
	int v30; // [sp+Ch] [bp-10h]@10
	int v31; // [sp+14h] [bp-8h]@14

	v6 = (int)A8;
	v5 = *(int *)(v6 + 8);
	if ( v5 )
	{
		v10 = 0;
		v9 = (int)&((ffeVertex*)v5)->unknown5_2;
		v8 = (int)&A10->unknown5_2;
		result = (int)&Ac->unknown5_2;
		v28 = A18 * 8 + (int)DATA_009260;
		do
		{
			if (*(int *)v28)
			{
				*(short *)v9 = (*(short *)v8 + (signed int)*(short *)result) >> 1;
				*(short *)(v9 + 2) = (*(short *)(v8 + 2) + (signed int)*(short *)(result + 2)) >> 1;
			}
			++v10;
			v9 += 4;
			v8 += 4;
			result += 4;
			v28 += 256;
		}
		while ( v10 < 2 );
	}
	else
	{
		v19 = (int)C_FUNC_001818_ArrayRemoveFirstElement();
		*(int *)(v6 + 8) = v19;
		v29 = v19;
		if ( *(int *)v6 == (int)Ac )
		{
			v20 = (int)C_FUNC_001820_ArrayCreateNewElement();
			*(int *)(v6 + 4) = v20;
			*(int *)v20 = (int)Ac;
			v21 = (int)C_FUNC_001820_ArrayCreateNewElement();
			*(int *)(v6 + 12) = v21;
			*(int *)v21 = (int)A10;
		}
		else
		{
			v22 = (int)C_FUNC_001820_ArrayCreateNewElement();
			*(int *)(v6 + 4) = v22;
			*(int *)v22 = (int)A10;
			v23 = (int)C_FUNC_001820_ArrayCreateNewElement();
			*(int *)(v6 + 12) = v23;
			*(int *)v23 = (int)Ac;
		}
		v14 = 0;
		v12 = (int)&((ffeVertex*)v29)->unknown5_2;
		v11 = (int)&A10->unknown5_2;
		v13 = (int)&Ac->unknown5_2;
		v30 = A18 * 8 + (int)DATA_009260;
		do
		{
			if (*(int *)v30)
			{
				*(short *)v12 = (*(short *)v11 + (signed int)*(short *)v13) >> 1;
				*(short *)(v12 + 2) = (*(short *)(v11 + 2) + (signed int)*(short *)(v13 + 2)) >> 1;
			}
			++v14;
			v12 += 4;
			v11 += 4;
			v13 += 4;
			v30 += 256;
		}
		while ( v14 < 2 );
		v24 = (int)Ac;
		*(int *)(v29 + 40) = *(int *)(v24 + 40);
		*(int *)(v29 + 44) = *(int *)(v24 + 44);
		*(int *)(v29 + 48) = *(int *)(v24 + 48);
		v31 = 0;
		if (Ac->z_2 >= A18 || A10->z_2 >= A18 )
		{
			*(int *)(v29 + 60) = C_FUNC_001846(Ac->x_2, A10->x_2, A18)
				+ ((A10->x_2 + Ac->x_2) >> 1);
			v17 = 0;
			v15 = (int)&((ffeVertex*)v29)->unknown5_2;
			v16 = A18 * 8 + (int)DATA_009260;
			do
			{
				if (*(int *)v16)
				{
					if ( (unsigned int)*(short *)v15 < 0x80 )
					{
						if ( (unsigned int)*(short *)(v15 + 2) < 0x80 )
						{
							*(int *)(v29 + 60) >>= 7;
							v18 = (*(short *)(v15 + 2) << 7) + *(short *)v15;
							if(*((int*)v16 + 1)) {
								((ffeVertex*)v29)->x_2 *= *(int*)(*((int*)v16 + 1) + v18) & 0xff;
							}
							((ffeVertex*)v29)->x_2 += *(int*)(*(int*)v16 + v18) & 0xff << 21;
						}
					}
				}
				++v17;
				v15 += 4;
				v16 += 256;
			}
			while ( v17 < 2 );
			*(int *)(v29 + 4) = ((A10->nx + Ac->nx) >> 1)
				+ C_FUNC_001521_MulWithNormalize(*(int *)(v29 + 40), v31);
			*(int *)(v29 + 8) = ((A10->ny + Ac->ny) >> 1)
				+ C_FUNC_001521_MulWithNormalize(*(int *)(v29 + 44), v31);
			*(int *)(v29 + 12) = ((A10->nz + Ac->nz) >> 1)
				+ C_FUNC_001521_MulWithNormalize(*(int *)(v29 + 48), v31);
			shiftCall = (char *(*)(int)) *(int*)(DATA_009282);
			shiftCall(v29);
		}
		else
		{
			*(int *)(v29 + 60) = (A10->x_2 + Ac->x_2) >> 1;
			*(int *)(v29 + 4) = (A10->nx + Ac->nx) >> 1;
			*(int *)(v29 + 8) = (A10->ny + Ac->ny) >> 1;
			*(int *)(v29 + 12) = (A10->nz + Ac->nz) >> 1;
			if (Ac->nz >= 64 && A10->nz >= 64 )
			{
				*(short *)v29 = (A10->orto_x + Ac->orto_x) >> 1;
				*(short *)(v29 + 2) = (A10->orto_y + Ac->orto_y) >> 1;
				if (A14->nz > 64 )
				{
					v25 = Ac->orto_y > A10->orto_y;
					v26 = Ac->orto_x > A10->orto_x;
					v27 = (A10->orto_y - Ac->orto_y) * (A14->orto_x - Ac->orto_x)
		> 
		(A14->orto_y - Ac->orto_y) * (A10->orto_x - Ac->orto_x);
					((ffeVertex*)v29)->orto_x += *(int*)((int)DATA_007872 + 16 * v25 + 8 * v26 + 4 * v27);
					((ffeVertex*)v29)->orto_y += *(int*)((int)DATA_007873 + 16 * v25 + 8 * v26 + 4 * v27);
				}
				shiftCall2 = (int(*)(short)) *(int*)DATA_009280;
				((ffeVertex*)v29)->orto_x_2 = (short)shiftCall2(((ffeVertex*)v29)->orto_x);
				shiftCall2 = (int(*)(short)) *(int*)DATA_009281;
				((ffeVertex*)v29)->orto_y_2 = (short)shiftCall2(((ffeVertex*)v29)->orto_y);
			}
			else
			{
				shiftCall = (char *(*)(int)) *(int*)(DATA_009282);
				shiftCall(v29);
			}
		}
		C_FUNC_001829_GetTriangulateDepth((ffeVertex *)v29);
	}
	return 0;
}

extern "C" int C_FUNC_001859(int* A8, ffeVertex* Ac, ffeVertex* A10, ffeVertex* A14, int A18)
{
	int v5; // eax@1
	int v6; // ebx@1
	int result; // eax@2
	int v8; // edx@2
	int v9; // ecx@2
	signed int v10; // esi@2
	int v11; // eax@10
	int v12; // edx@10
	int v13; // ecx@10
	signed int v14; // esi@10
	int v15; // eax@15
	int v16; // eax@25
	int v17; // edx@25
	signed int v18; // esi@25
	int v19; // ecx@29
	int v20; // eax@7
	int v21; // eax@8
	int v22; // eax@8
	int v23; // eax@9
	int v24; // eax@9
	int v25; // eax@14
	bool v26; // eax@23
	bool v27; // edx@23
	bool v28; // ecx@23
	int v29; // [sp+10h] [bp-10h]@2
	int v30; // [sp+1Ch] [bp-4h]@7
	int v31; // [sp+Ch] [bp-14h]@10
	int v32; // [sp+18h] [bp-8h]@14

	v6 = (int)A8;
	v5 = *(int *)(v6 + 8);
	if (v5)
	{
		v10 = 0;
		v9 = (int)&((ffeVertex*)v5)->unknown5_2;
		v8 = (int)&A10->unknown5_2;
		result = (int)&Ac->unknown5_2;
		v29 = A18 * 8 + (int)DATA_009260;
		do
		{
			if (*(int *)v29)
			{
				*(short *)v9 = (*(short *)v8 + (signed int)*(short *)result) >> 1;
				*(short *)(v9 + 2) = (*(short *)(v8 + 2) + (signed int)*(short *)(result + 2)) >> 1;
			}
			++v10;
			v9 += 4;
			v8 += 4;
			result += 4;
			v29 += 256;
		}
		while ( v10 < 2 );
	}
	else
	{
		v20 = (int)C_FUNC_001818_ArrayRemoveFirstElement();
		*(int *)(v6 + 8) = v20;
		v30 = v20;
		if ( *(int *)v6 == (int)Ac )
		{
			v21 = (int)C_FUNC_001820_ArrayCreateNewElement();
			*(int *)(v6 + 4) = v21;
			*(int *)v21 = (int)Ac;
			v22 = (int)C_FUNC_001820_ArrayCreateNewElement();
			*(int *)(v6 + 12) = v22;
			*(int *)v22 = (int)A10;
		}
		else
		{
			v23 = (int)C_FUNC_001820_ArrayCreateNewElement();
			*(int *)(v6 + 4) = v23;
			*(int *)v23 = (int)A10;
			v24 = (int)C_FUNC_001820_ArrayCreateNewElement();
			*(int *)(v6 + 12) = v24;
			*(int *)v24 = (int)Ac;
		}
		v14 = 0;
		v12 = (int)&((ffeVertex*)v30)->unknown5_2;
		v11 = (int)&A10->unknown5_2;
		v13 = (int)&Ac->unknown5_2;
		v31 = A18 * 8 + (int)DATA_009260;;
		do
		{
			if (*(int *)v31)
			{
				*(short *)v12 = (*(short *)v11 + (signed int)*(short *)v13) >> 1;
				*(short *)(v12 + 2) = (*(short *)(v11 + 2) + (signed int)*(short *)(v13 + 2)) >> 1;
			}
			++v14;
			v12 += 4;
			v11 += 4;
			v13 += 4;
			v31 += 256;
		}
		while ( v14 < 2 );
		v25 = (int)Ac;
		*(int *)(v30 + 40) = *(int *)(v25 + 40);
		*(int *)(v30 + 44) = *(int *)(v25 + 44);
		*(int *)(v30 + 48) = *(int *)(v25 + 48);
		v32 = 0;
		if ( ((A10->x_2 - 1) ^ (Ac->x_2 - 1)) >= 0 )
			v15 = (int)A18;
		else
			v15 = (int)A18 - 1;
		if ( v15 <= Ac->z_2 || v15 <= A10->z_2 )
		{
			*(int *)(v30 + 60) = ((A10->x_2 + Ac->x_2) >> 1)
				+ C_FUNC_001846(Ac->x_2, A10->x_2, A18);
			v18 = 0;
			v16 = (int)&((ffeVertex*)v30)->unknown5_2;
			v17 = A18 * 8 + (int)DATA_009260;;
			do
			{
				if (*(int *)v17)
				{
					if ( (unsigned int)*(short *)v16 < 0x80 )
					{
						if ( (unsigned int)*(short *)(v16 + 2) < 0x80 )
						{
							*(int *)(v30 + 60) >>= 7;
							v19 = (*(short *)(v16 + 2) << 7) + *(short *)v16;

							if(*((int*)v17 + 1)) {
								((ffeVertex*)v30)->x_2 *= *(int*)(*((int*)v17 + 1) + v19) & 0xff;
							}
							((ffeVertex*)v30)->x_2 += *(int*)(*(int*)v17 + v19) & 0xff << 21;
						}
					}
				}
				++v18;
				v16 += 4;
				v17 += 256;
			}
			while ( v18 < 2 );
			*(int *)(v30 + 4) = ((A10->nx + Ac->nx) >> 1)
				+ C_FUNC_001521_MulWithNormalize(*(int *)(v30 + 40), v32);
			*(int *)(v30 + 8) = ((A10->ny + Ac->ny) >> 1)
				+ C_FUNC_001521_MulWithNormalize(*(int *)(v30 + 44), v32);
			*(int *)(v30 + 12) = ((A10->nz + Ac->nz) >> 1)
				+ C_FUNC_001521_MulWithNormalize(*(int *)(v30 + 48), v32);
			shiftCall = (char *(*)(int)) *(int*)(DATA_009282);
			shiftCall(v30);
		}
		else
		{
			*(int *)(v30 + 60) = (A10->x_2 + Ac->x_2) >> 1;
			*(int *)(v30 + 4) = (A10->nx + Ac->nx) >> 1;
			*(int *)(v30 + 8) = (A10->ny + Ac->ny) >> 1;
			*(int *)(v30 + 12) = (A10->nz + Ac->nz) >> 1;
			if ( Ac->nz >= 64 && A10->nz >= 64 )
			{
				*(short *)v30 = (A10->orto_x + Ac->orto_x) >> 1;
				*(short *)(v30 + 2) = (A10->orto_y + Ac->orto_y) >> 1;
				if (A14->nz > 64 )
				{
					v26 = Ac->orto_y > A10->orto_y;
					v27 = Ac->orto_x > A10->orto_x;
					v28 = (A10->orto_y - Ac->orto_y) * (A14->orto_x - Ac->orto_x)
							> 
						  (A14->orto_y - Ac->orto_y) * (A10->orto_x - Ac->orto_x);
					((ffeVertex*)v30)->orto_x += *(int*)((int)DATA_007874 + 16 * v26 + 8 * v27 + 4 * v28);
					((ffeVertex*)v30)->orto_y += *(int*)((int)DATA_007875 + 16 * v26 + 8 * v27 + 4 * v28);
				}
				shiftCall2 = (int(*)(short)) *(int*)DATA_009280;
				((ffeVertex*)v30)->orto_x_2 = (short)shiftCall2(((ffeVertex*)v30)->orto_x);
				shiftCall2 = (int(*)(short)) *(int*)DATA_009281;
				((ffeVertex*)v30)->orto_y_2 = (short)shiftCall2(((ffeVertex*)v30)->orto_y);

			}
			else
			{
				shiftCall = (char *(*)(int)) *(int*)(DATA_009282);
				shiftCall(v30);
			}
		}
		C_FUNC_001829_GetTriangulateDepth((ffeVertex *)v30);
	}
	return 0;
}

extern "C" int C_FUNC_001860(int* A8, ffeVertex* Ac, ffeVertex* A10, ffeVertex* A14, int A18)
{
	int v5; // eax@1
	int v6; // ebx@1
	int result; // eax@2
	int v8; // edx@2
	int v9; // ecx@2
	int v10; // esi@2
	int v11; // eax@10
	int v12; // edx@10
	int v13; // ecx@10
	int v14; // esi@10
	int v15; // eax@22
	int v16; // edx@22
	int v17; // ebx@22
	int v18; // esi@22
	int v19; // eax@7
	int v20; // eax@8
	int v21; // eax@8
	int v22; // eax@9
	int v23; // eax@9
	int v24; // eax@14
	int v25; // eax@20
	int v26; // edx@20
	int v27; // ecx@20
	int v28; // eax@30
	int v29; // ebx@30
	int v30; // [sp+10h] [bp-10h]@2
	int v31; // [sp+18h] [bp-8h]@7
	int v32; // [sp+Ch] [bp-14h]@10
	int v33; // [sp+14h] [bp-Ch]@14
	int v34; // [sp+1Ch] [bp-4h]@26

	v6 = (int)A8;
	v5 = *(int *)(v6 + 8);
	if ( v5 )
	{
		v10 = 0;
		v9 = v5 + 52;
		v8 = (int)&A10->unknown5_2;
		result = (int)&Ac->unknown5_2;
		v30 = A18 * 8 + (int)DATA_009260;
		do
		{
			if ( *(int *)v30)
			{
				*(short *)v9 = (*(short *)v8 + (signed int)*(short *)result) >> 1;
				*(short *)(v9 + 2) = (*(short *)(v8 + 2) + (signed int)*(short *)(result + 2)) >> 1;
			}
			++v10;
			v9 += 4;
			v8 += 4;
			result += 4;
			v30 += 256;
		}
		while ( v10 < 2 );
	}
	else
	{
		v19 = (int)C_FUNC_001818_ArrayRemoveFirstElement();
		*(int *)(v6 + 8) = v19;
		v31 = v19;
		if ( *(int *)v6 == (int)Ac )
		{
			v20 = (int)C_FUNC_001820_ArrayCreateNewElement();
			*(int *)(v6 + 4) = v20;
			*(int *)v20 = (int)Ac;
			v21 = (int)C_FUNC_001820_ArrayCreateNewElement();
			*(int *)(v6 + 12) = v21;
			*(int *)v21 = (int)A10;
		}
		else
		{
			v22 = (int)C_FUNC_001820_ArrayCreateNewElement();
			*(int *)(v6 + 4) = v22;
			*(int *)v22 = (int)A10;
			v23 = (int)C_FUNC_001820_ArrayCreateNewElement();
			*(int *)(v6 + 12) = v23;
			*(int *)v23 = (int)Ac;
		}
		v14 = 0;
		v12 = v31 + 52;
		v11 = (int)&A10->unknown5_2;
		v13 = (int)&Ac->unknown5_2;
		v32 = A18 * 8 + (int)DATA_009260;
		do
		{
			if ( *(int *)v32)
			{
				*(short *)v12 = (*(short *)v11 + (signed int)*(short *)v13) >> 1;
				*(short *)(v12 + 2) = (*(short *)(v11 + 2) + (signed int)*(short *)(v13 + 2)) >> 1;
			}
			++v14;
			v12 += 4;
			v11 += 4;
			v13 += 4;
			v32 += 256;
		}
		while ( v14 < 2 );
		v24 = (int)Ac;
		*(int *)(v31 + 40) = *(int *)(v24 + 40);
		*(int *)(v31 + 44) = *(int *)(v24 + 44);
		*(int *)(v31 + 48) = *(int *)(v24 + 48);
		v33 = 0;
		if ( *(int *)((int)Ac + 68) >= A18 || *(int *)((int)A10 + 68) >= A18 )
		{
			v17 = (*(int *)((int)A10 + 60) + *(int *)((int)Ac + 60)) >> 1;
			*(int *)(v31 + 60) = v17 + (int)C_FUNC_001846(*(int *)((int)Ac + 60), *(int *)((int)A10 + 60), A18);
			v18 = 0;
			v15 = v31 + 52;
			v16 = A18 * 8 + (int)DATA_009260;
			do
			{
				if ( *(int *)v16)
				{
					if ( (unsigned int)*(short *)v15 < 0x80 )
					{
						if ( (unsigned int)*(short *)(v15 + 2) < 0x80 )
						{
							*(int *)(v31 + 60) >>= 7;
							v34 = (*(short *)(v15 + 2) << 7) + *(short *)v15;
							if(*((int*)v16 + 1)) {
								((ffeVertex*)v31)->x_2 *= *(unsigned char*)(*((int*)v16 + 1) + v34);
							}
							((ffeVertex*)v31)->x_2 += *(unsigned char*)(*(int*)v16 + v34) << 21;
						}
					}
				}
				++v18;
				v15 += 4;
				v16 += 256;
			}
			while ( v18 < 2 );
			v28 = v33 + ((*(int *)(v31 + 60) - v17) >> 3);
			v29 = v33 + ((*(int *)(v31 + 60) - v17) >> 3);
			*(int *)(v31 + 4) = ((*(int *)((int)A10 + 4) + *(int *)((int)Ac + 4)) >> 1)
				+ (int)C_FUNC_001521_MulWithNormalize(*(int *)(v31 + 40), v28);
			*(int *)(v31 + 8) = ((*(int *)((int)A10 + 8) + *(int *)((int)Ac + 8)) >> 1)
				+ (int)C_FUNC_001521_MulWithNormalize(*(int *)(v31 + 44), v29);
			*(int *)(v31 + 12) = ((*(int *)((int)A10 + 12) + *(int *)((int)Ac + 12)) >> 1)
				+ (int)C_FUNC_001521_MulWithNormalize(*(int *)(v31 + 48), v29);
			shiftCall = (char *(*)(int)) *(int*)(DATA_009282);
			shiftCall((int)v31);
		}
		else
		{
			*(int *)(v31 + 60) = (*(int *)((int)A10 + 60) + *(int *)((int)Ac + 60)) >> 1;
			*(int *)(v31 + 4) = (*(int *)((int)A10 + 4) + *(int *)((int)Ac + 4)) >> 1;
			*(int *)(v31 + 8) = (*(int *)((int)A10 + 8) + *(int *)((int)Ac + 8)) >> 1;
			*(int *)(v31 + 12) = (*(int *)((int)A10 + 12) + *(int *)((int)Ac + 12)) >> 1;
			if ( *(int *)((int)Ac + 12) >= 64 && *(int *)((int)A10 + 12) >= 64 )
			{
				*(short *)v31 = (*(short *)(int)A10 + (signed int)*(short *)(int)Ac) >> 1;
				*(short *)(v31 + 2) = (*(short *)((int)A10 + 2) + (signed int)*(short *)((int)Ac + 2)) >> 1;
				if ( *(int *)((int)A14 + 12) > 64 )
				{
					v25 = *(short *)((int)Ac + 2) > *(short *)((int)A10 + 2);
					v26 = *(short *)(int)Ac > *(short *)(int)A10;
					v27 = (*(short *)((int)A10 + 2) - (signed int)*(short *)((int)Ac + 2)) * (*(short *)(int)A14 - (signed int)*(short *)(int)Ac) 
						  > 
						  (*(short *)((int)A14 + 2) - (signed int)*(short *)((int)Ac + 2)) * (*(short *)(int)A10 - (signed int)*(short *)(int)Ac);
					((ffeVertex*)v31)->orto_x += *(int*)((int)DATA_007876 + 16 * v25 + 8 * v26 + 4 * v27);
					((ffeVertex*)v31)->orto_y += *(int*)((int)DATA_007877 + 16 * v25 + 8 * v26 + 4 * v27);
				}
				shiftCall2 = (int(*)(short)) *(int*)DATA_009280;
				((ffeVertex*)v31)->orto_x_2 = (short)shiftCall2(((ffeVertex*)v31)->orto_x);
				shiftCall2 = (int(*)(short)) *(int*)DATA_009281;
				((ffeVertex*)v31)->orto_y_2 = (short)shiftCall2(((ffeVertex*)v31)->orto_y);
			}
			else
			{
				shiftCall = (char *(*)(int)) *(int*)(DATA_009282);
				shiftCall(v31);
			}
		}
		FUNC_001829((ffeVertex*)v31);
	}
	return 0;
}

extern "C" int C_FUNC_001861(int* A8, ffeVertex* Ac, ffeVertex* A10, ffeVertex* A14, int A18)
{
	int v5; // eax@1
	int v6; // esi@1
	int result; // eax@2
	int v8; // edx@2
	int v9; // ecx@2
	signed int v10; // esi@2
	int v11; // ebx@7
	int v12; // eax@10
	int v13; // edx@10
	signed int v14; // esi@10
	int v15; // eax@15
	int v16; // eax@19
	signed int v17; // eax@29
	int v18; // edx@29
	signed int v19; // esi@29
	int v20; // eax@38
	int v21; // eax@7
	int v22; // eax@8
	int v23; // eax@8
	int v24; // eax@9
	int v25; // eax@9
	int v26; // eax@14
	int v27; // eax@21
	bool v28; // eax@27
	bool v29; // edx@27
	int v30; // esi@40
	int v31; // [sp+14h] [bp-10h]@2
	int v32; // [sp+Ch] [bp-18h]@10
	int v33; // [sp+10h] [bp-14h]@10
	int v34; // [sp+1Ch] [bp-8h]@14
	bool v35; // [sp+18h] [bp-Ch]@27
	int v36; // [sp+20h] [bp-4h]@33

	v6 = (int)A8;
	v5 = *(int *)((int)A8 + 8);
	if ( v5 )
	{
		v10 = 0;
		v9 = v5 + 52;
		v8 = (int)A10 + 52;
		result = (int)Ac + 52;
		v31 = A18 * 8 + (int)DATA_009260;
		do
		{
			if ( *(int *)v31)
			{
				*(short *)v9 = (*(short *)v8 + (signed int)*(short *)result) >> 1;
				*(short *)(v9 + 2) = (*(short *)(v8 + 2) + (signed int)*(short *)(result + 2)) >> 1;
			}
			++v10;
			v9 += 4;
			v8 += 4;
			result += 4;
			v31 += 256;
		}
		while ( v10 < 2 );
	}
	else
	{
		v21 = (int)C_FUNC_001818_ArrayRemoveFirstElement();
		*(int *)(v6 + 8) = v21;
		v11 = v21;
		if ( *(int *)v6 == (int)Ac )
		{
			v22 = (int)C_FUNC_001820_ArrayCreateNewElement();
			*(int *)(v6 + 4) = v22;
			*(int *)v22 = (int)Ac;
			v23 = (int)C_FUNC_001820_ArrayCreateNewElement();
			*(int *)(v6 + 12) = v23;
			*(int *)v23 = (int)A10;
		}
		else
		{
			v24 = (int)C_FUNC_001820_ArrayCreateNewElement();
			*(int *)(v6 + 4) = v24;
			*(int *)v24 = (int)A10;
			v25 = (int)C_FUNC_001820_ArrayCreateNewElement();
			*(int *)(v6 + 12) = v25;
			*(int *)v25 = (int)Ac;
		}
		v14 = 0;
		v13 = v11 + 52;
		v12 = (int)A10 + 52;
		v32 = (int)Ac + 52;
		v33 = A18 * 8 + (int)DATA_009260;
		do
		{
			if ( *(int *)v33)
			{
				*(short *)v13 = (*(short *)v12 + (signed int)*(short *)v32) >> 1;
				*(short *)(v13 + 2) = (*(short *)(v12 + 2) + (signed int)*(short *)(v32 + 2)) >> 1;
			}
			++v14;
			v13 += 4;
			v12 += 4;
			v32 += 4;
			v33 += 256;
		}
		while ( v14 < 2 );
		v26 = (int)Ac;
		*(int *)(v11 + 40) = *(int *)((int)Ac + 40);
		*(int *)(v11 + 44) = *(int *)(v26 + 44);
		*(int *)(v11 + 48) = *(int *)(v26 + 48);
		v34 = 0;
		if ( ((*(int *)((int)A10 + 60) - 1) ^ (*(int *)((int)Ac + 60) - 1)) >= 0 )
			v15 = A18;
		else
			v15 = A18 - 1;
		if ( v15 <= *(int *)((int)Ac + 68) || v15 <= *(int *)((int)A10 + 68) )
		{
			*(int *)(v11 + 60) = ((*(int *)((int)A10 + 60) + *(int *)((int)Ac + 60)) >> 1)
				+ C_FUNC_001846(*(int *)((int)Ac + 60), *(int *)((int)A10 + 60), A18);
			v19 = 0;
			v17 = v11 + 52;
			v18 = A18 * 8 + (int)DATA_009260;
			do
			{
				if ( *(int *)v18)
				{
					if ( (unsigned int)*(short *)v17 < 0x80 )
					{
						if ( (unsigned int)*(short *)(v17 + 2) < 0x80 )
						{
							*(int *)(v11 + 60) >>= 7;
							v36 = (*(short *)(v17 + 2) << 7) + *(short *)v17;
							if(*((int*)v18 + 1)) {
								((ffeVertex*)v11)->x_2 *= *(unsigned char*)(*((int*)v18 + 1) + v36);
							}
							((ffeVertex*)v11)->x_2 += *(unsigned char*)(*(int*)v18 + v36) << 21;
						}
					}
				}
				++v19;
				v17 += 4;
				v18 += 256;
			}
			while ( v19 < 2 );
			if ( *(int *)(v11 + 60) < 0 )
				v20 = 0;
			else
				v20 = *(int *)(v11 + 60);
			*(int *)(v11 + 64) = v20;
			v30 = v34 + ((v20 - ((*(int *)((int)A10 + 64) + *(int *)((int)Ac + 64)) >> 1)) >> 3);
			*(int *)(v11 + 4) = ((*(int *)((int)A10 + 4) + *(int *)((int)Ac + 4)) >> 1)
				+ C_FUNC_001521_MulWithNormalize(
				*(int *)(v11 + 40),
				v34 + ((v20 - ((*(int *)((int)A10 + 64) + *(int *)((int)Ac + 64)) >> 1)) >> 3));
			*(int *)(v11 + 8) = ((*(int *)((int)A10 + 8) + *(int *)((int)Ac + 8)) >> 1)
				+ C_FUNC_001521_MulWithNormalize(*(int *)(v11 + 44), v30);
			*(int *)(v11 + 12) = ((*(int *)((int)A10 + 12) + *(int *)((int)Ac + 12)) >> 1)
				+ C_FUNC_001521_MulWithNormalize(*(int *)(v11 + 48), v30);
			shiftCall = (char *(*)(int)) *(int*)(DATA_009282);
			shiftCall((int)v11);
		}
		else
		{
			v16 = *(int *)((int)Ac + 64);
			if ( v16 || *(int *)((int)A10 + 64) )
			{
				v27 = (*(int *)((int)A10 + 64) + v16) >> 1;
				*(int *)(v11 + 64) = v27;
				*(int *)(v11 + 60) = v27;
			}
			else
			{
				*(int *)(v11 + 60) = (*(int *)((int)A10 + 60) + *(int *)((int)Ac + 60)) >> 1;
				*(int *)(v11 + 64) = 0;
			}
			*(int *)(v11 + 4) = (*(int *)((int)A10 + 4) + *(int *)((int)Ac + 4)) >> 1;
			*(int *)(v11 + 8) = (*(int *)((int)A10 + 8) + *(int *)((int)Ac + 8)) >> 1;
			*(int *)(v11 + 12) = (*(int *)((int)A10 + 12) + *(int *)((int)Ac + 12)) >> 1;
			if ( *(int *)((int)Ac + 12) >= 64 && *(int *)((int)A10 + 12) >= 64 )
			{
				*(short *)v11 = (*(short *)(int)A10 + (signed int)*(short *)(int)Ac) >> 1;
				*(short *)(v11 + 2) = (*(short *)((int)A10 + 2) + (signed int)*(short *)((int)Ac + 2)) >> 1;
				if ( *(int *)((int)A14 + 12) > 64 )
				{
					v28 = *(short *)((int)Ac + 2) > *(short *)((int)A10 + 2);
					v35 = *(short *)(int)Ac > *(short *)(int)A10;
					v29 = (*(short *)((int)A10 + 2) - (signed int)*(short *)((int)Ac + 2)) * (*(short *)(int)A14 - (signed int)*(short *)(int)Ac) > (*(short *)((int)A14 + 2) - (signed int)*(short *)((int)Ac + 2)) * (*(short *)(int)A10 - (signed int)*(short *)(int)Ac);
					((ffeVertex*)v11)->orto_x += *(int*)((int)DATA_007878 + 16 * v28 + 8 * v35 + 4 * v29);
					((ffeVertex*)v11)->orto_y += *(int*)((int)DATA_007879 + 16 * v28 + 8 * v35 + 4 * v29);
				}
				shiftCall2 = (int(*)(short)) *(int*)DATA_009280;
				((ffeVertex*)v11)->orto_x_2 = (short)shiftCall2(((ffeVertex*)v11)->orto_x);
				shiftCall2 = (int(*)(short)) *(int*)DATA_009281;
				((ffeVertex*)v11)->orto_y_2 = (short)shiftCall2(((ffeVertex*)v11)->orto_y);
			}
			else
			{
				shiftCall = (char *(*)(int)) *(int*)(DATA_009282);
				shiftCall(v11);
			}
		}
		C_FUNC_001829_GetTriangulateDepth((ffeVertex*)v11);
	}
	return 0;
}

extern "C" int C_FUNC_001862(int* A8, ffeVertex* Ac, ffeVertex* A10, ffeVertex* A14, int A18)
{
	int v5; // eax@1
	int v6; // ebx@1
	int result; // eax@2
	int v8; // edx@2
	int v9; // ecx@2
	signed int v10; // esi@2
	int v11; // eax@10
	int v12; // edx@10
	int v13; // ecx@10
	signed int v14; // esi@10
	int v15; // eax@22
	int v16; // edx@22
	signed int v17; // esi@22
	int v18; // ecx@26
	int v19; // eax@7
	int v20; // eax@8
	int v21; // eax@8
	int v22; // eax@9
	int v23; // eax@9
	bool v24; // eax@20
	bool v25; // edx@20
	bool v26; // ecx@20
	int v27; // [sp+10h] [bp-Ch]@2
	int v28; // [sp+18h] [bp-4h]@7
	int v29; // [sp+Ch] [bp-10h]@10
	int v30; // [sp+14h] [bp-8h]@14

	v6 = (int)A8;
	v5 = *(int *)((int)A8 + 8);
	if ( v5 )
	{
		v10 = 0;
		v9 = v5 + 52;
		v8 = (int)A10 + 52;
		result = (int)Ac + 52;
		v27 = A18 * 8 + (int)DATA_009260;
		do
		{
			if ( *(int *)v27)
			{
				*(short *)v9 = (*(short *)v8 + (signed int)*(short *)result) >> 1;
				*(short *)(v9 + 2) = (*(short *)(v8 + 2) + (signed int)*(short *)(result + 2)) >> 1;
			}
			++v10;
			v9 += 4;
			v8 += 4;
			result += 4;
			v27 += 256;
		}
		while ( v10 < 2 );
	}
	else
	{
		v19 = (int)C_FUNC_001818_ArrayRemoveFirstElement();
		*(int *)(v6 + 8) = v19;
		v28 = v19;
		if ( *(int *)v6 == (int)Ac )
		{
			v20 = (int)C_FUNC_001820_ArrayCreateNewElement();
			*(int *)(v6 + 4) = v20;
			*(int *)v20 = (int)Ac;
			v21 = (int)C_FUNC_001820_ArrayCreateNewElement();
			*(int *)(v6 + 12) = v21;
			*(int *)v21 = (int)A10;
		}
		else
		{
			v22 = (int)C_FUNC_001820_ArrayCreateNewElement();
			*(int *)(v6 + 4) = v22;
			*(int *)v22 = (int)A10;
			v23 = (int)C_FUNC_001820_ArrayCreateNewElement();
			*(int *)(v6 + 12) = v23;
			*(int *)v23 = (int)Ac;
		}
		v14 = 0;
		v12 = v28 + 52;
		v11 = (int)A10 + 52;
		v13 = (int)Ac + 52;
		v29 = A18 * 8 + (int)DATA_009260;
		do
		{
			if ( *(int *)v29)
			{
				*(short *)v12 = (*(short *)v11 + (signed int)*(short *)v13) >> 1;
				*(short *)(v12 + 2) = (*(short *)(v11 + 2) + (signed int)*(short *)(v13 + 2)) >> 1;
			}
			++v14;
			v12 += 4;
			v11 += 4;
			v13 += 4;
			v29 += 256;
		}
		while ( v14 < 2 );
		*(int *)(v28 + 40) = C_FUNC_001521_MulWithNormalize(*(int *)((int)A10 + 40) + *(int *)((int)Ac + 40), *(int*)(A18 * 4 + (int)DATA_007840));
		*(int *)(v28 + 44) = C_FUNC_001521_MulWithNormalize(*(int *)((int)A10 + 44) + *(int *)((int)Ac + 44), *(int*)(A18 * 4 + (int)DATA_007840));
		*(int *)(v28 + 48) = C_FUNC_001521_MulWithNormalize(*(int *)((int)A10 + 48) + *(int *)((int)Ac + 48), *(int*)(A18 * 4 + (int)DATA_007840));
		v30 = *(int*)(A18 * 4 + (int)DATA_007839);
		if ( *(int *)((int)Ac + 68) >= A18 || *(int *)((int)A10 + 68) >= A18 )
		{
			*(int *)(v28 + 60) = C_FUNC_001846(*(int *)((int)Ac + 60), *(int *)((int)A10 + 60), A18)
				+ ((*(int *)((int)A10 + 60) + *(int *)((int)Ac + 60)) >> 1);
			v17 = 0;
			v15 = v28 + 52;
			v16 = A18 * 8 + (int)DATA_009260;
			do
			{
				if ( *(int *)v16)
				{
					if ( (unsigned int)*(short *)v15 < 0x80 )
					{
						if ( (unsigned int)*(short *)(v15 + 2) < 0x80 )
						{
							*(int *)(v28 + 60) >>= 7;
							v18 = (*(short *)(v15 + 2) << 7) + *(short *)v15;
							if(*((int*)v16 + 1)) {
								((ffeVertex*)v28)->x_2 *= *(int*)(*((int*)v16 + 1) + v18) & 0xff;
							}
							((ffeVertex*)v28)->x_2 += *(int*)(*(int*)v16 + v18) & 0xff << 21;
						}
					}
				}
				++v17;
				v15 += 4;
				v16 += 256;
			}
			while ( v17 < 2 );
			*(int *)(v28 + 4) = ((*(int *)((int)A10 + 4) + *(int *)((int)Ac + 4)) >> 1)
				+ C_FUNC_001521_MulWithNormalize(*(int *)(v28 + 40), v30);
			*(int *)(v28 + 8) = ((*(int *)((int)A10 + 8) + *(int *)((int)Ac + 8)) >> 1)
				+ C_FUNC_001521_MulWithNormalize(*(int *)(v28 + 44), v30);
			*(int *)(v28 + 12) = ((*(int *)((int)A10 + 12) + *(int *)((int)Ac + 12)) >> 1)
				+ C_FUNC_001521_MulWithNormalize(*(int *)(v28 + 48), v30);
			shiftCall = (char *(*)(int)) *(int*)(DATA_009282);
			shiftCall((int)v28);
		}
		else
		{
			*(int *)(v28 + 60) = (*(int *)((int)A10 + 60) + *(int *)((int)Ac + 60)) >> 1;
			*(int *)(v28 + 4) = (*(int *)((int)A10 + 4) + *(int *)((int)Ac + 4)) >> 1;
			*(int *)(v28 + 8) = (*(int *)((int)A10 + 8) + *(int *)((int)Ac + 8)) >> 1;
			*(int *)(v28 + 12) = (*(int *)((int)A10 + 12) + *(int *)((int)Ac + 12)) >> 1;
			if ( *(int *)((int)Ac + 12) >= 64 && *(int *)((int)A10 + 12) >= 64 )
			{
				*(short *)v28 = (*(short *)(int)A10 + (signed int)*(short *)(int)Ac) >> 1;
				*(short *)(v28 + 2) = (*(short *)((int)A10 + 2) + (signed int)*(short *)((int)Ac + 2)) >> 1;
				if ( *(int *)((int)A14 + 12) > 64 )
				{
					v24 = *(short *)((int)Ac + 2) > *(short *)((int)A10 + 2);
					v25 = *(short *)(int)Ac > *(short *)(int)A10;
					v26 = (*(short *)((int)A10 + 2) - (signed int)*(short *)((int)Ac + 2)) * (*(short *)(int)A14 - (signed int)*(short *)(int)Ac) > (*(short *)((int)A14 + 2) - (signed int)*(short *)((int)Ac + 2)) * (*(short *)(int)A10 - (signed int)*(short *)(int)Ac);
					((ffeVertex*)v28)->orto_x += *(int*)((int)DATA_007880 + 16 * v24 + 8 * v25 + 4 * v26);
					((ffeVertex*)v28)->orto_y += *(int*)((int)DATA_007881 + 16 * v24 + 8 * v25 + 4 * v26);
				}
				shiftCall2 = (int(*)(short)) *(int*)DATA_009280;
				((ffeVertex*)v28)->orto_x_2 = (short)shiftCall2(((ffeVertex*)v28)->orto_x);
				shiftCall2 = (int(*)(short)) *(int*)DATA_009281;
				((ffeVertex*)v28)->orto_y_2 = (short)shiftCall2(((ffeVertex*)v28)->orto_y);
			}
			else
			{
				shiftCall = (char *(*)(int)) *(int*)(DATA_009282);
				shiftCall(v28);
			}
		}
		C_FUNC_001829_GetTriangulateDepth((ffeVertex*)v28);
	}
	return 0;
}

extern "C" int C_FUNC_001863(int* A8, ffeVertex* Ac, ffeVertex* A10, ffeVertex* A14, int A18)
{
	int v5; // eax@1
	int v6; // ebx@1
	int result; // eax@2
	int v8; // edx@2
	int v9; // ecx@2
	signed int v10; // esi@2
	int v11; // eax@10
	int v12; // edx@10
	int v13; // ecx@10
	signed int v14; // esi@10
	int v15; // eax@15
	int v16; // eax@25
	int v17; // edx@25
	signed int v18; // esi@25
	int v19; // ecx@29
	int v20; // eax@7
	int v21; // eax@8
	int v22; // eax@8
	int v23; // eax@9
	int v24; // eax@9
	bool v25; // eax@23
	bool v26; // edx@23
	bool v27; // ecx@23
	int v28; // [sp+10h] [bp-10h]@2
	int v29; // [sp+1Ch] [bp-4h]@7
	int v30; // [sp+Ch] [bp-14h]@10
	int v31; // [sp+18h] [bp-8h]@14

	v6 = (int)A8;
	v5 = *(int *)((int)A8 + 8);
	if ( v5 )
	{
		v10 = 0;
		v9 = v5 + 52;
		v8 = (int)A10 + 52;
		result = (int)Ac + 52;
		v28 = A18 * 8 + (int)DATA_009260;
		do
		{
			if ( *(int *)v28)
			{
				*(short *)v9 = (*(short *)v8 + (signed int)*(short *)result) >> 1;
				*(short *)(v9 + 2) = (*(short *)(v8 + 2) + (signed int)*(short *)(result + 2)) >> 1;
			}
			++v10;
			v9 += 4;
			v8 += 4;
			result += 4;
			v28 += 256;
		}
		while ( v10 < 2 );
	}
	else
	{
		v20 = (int)C_FUNC_001818_ArrayRemoveFirstElement();
		*(int *)(v6 + 8) = v20;
		v29 = v20;
		if ( *(int *)v6 == (int)Ac )
		{
			v21 = (int)C_FUNC_001820_ArrayCreateNewElement();
			*(int *)(v6 + 4) = v21;
			*(int *)v21 = (int)Ac;
			v22 = (int)C_FUNC_001820_ArrayCreateNewElement();
			*(int *)(v6 + 12) = v22;
			*(int *)v22 = (int)A10;
		}
		else
		{
			v23 = (int)C_FUNC_001820_ArrayCreateNewElement();
			*(int *)(v6 + 4) = v23;
			*(int *)v23 = (int)A10;
			v24 = (int)C_FUNC_001820_ArrayCreateNewElement();
			*(int *)(v6 + 12) = v24;
			*(int *)v24 = (int)Ac;
		}
		v14 = 0;
		v12 = v29 + 52;
		v11 = (int)A10 + 52;
		v13 = (int)Ac + 52;
		v30 = A18 * 8 + (int)DATA_009260;
		do
		{
			if ( *(int *)v30)
			{
				*(short *)v12 = (*(short *)v11 + (signed int)*(short *)v13) >> 1;
				*(short *)(v12 + 2) = (*(short *)(v11 + 2) + (signed int)*(short *)(v13 + 2)) >> 1;
			}
			++v14;
			v12 += 4;
			v11 += 4;
			v13 += 4;
			v30 += 256;
		}
		while ( v14 < 2 );
		*(int *)(v29 + 40) = C_FUNC_001521_MulWithNormalize(*(int *)((int)A10 + 40) + *(int *)((int)Ac + 40), *(int*)(A18 * 4 + (int)DATA_007840));
		*(int *)(v29 + 44) = C_FUNC_001521_MulWithNormalize(*(int *)((int)A10 + 44) + *(int *)((int)Ac + 44), *(int*)(A18 * 4 + (int)DATA_007840));
		*(int *)(v29 + 48) = C_FUNC_001521_MulWithNormalize(*(int *)((int)A10 + 48) + *(int *)((int)Ac + 48), *(int*)(A18 * 4 + (int)DATA_007840));
		v31 = *(int*)(A18 * 4 + (int)DATA_007839);
		if ( ((*(int *)((int)A10 + 60) - 1) ^ (*(int *)((int)Ac + 60) - 1)) >= 0 )
			v15 = A18;
		else
			v15 = A18 - 1;
		if ( v15 <= *(int *)((int)Ac + 68) || v15 <= *(int *)((int)A10 + 68) )
		{
			*(int *)(v29 + 60) = ((*(int *)((int)A10 + 60) + *(int *)((int)Ac + 60)) >> 1)
				+ C_FUNC_001846(*(int *)((int)Ac + 60), *(int *)((int)A10 + 60), A18);
			v18 = 0;
			v16 = v29 + 52;
			v17 = A18 * 8 + (int)DATA_009260;
			do
			{
				if ( *(int *)v17)
				{
					if ( (unsigned int)*(short *)v16 < 0x80 )
					{
						if ( (unsigned int)*(short *)(v16 + 2) < 0x80 )
						{
							*(int *)(v29 + 60) >>= 7;
							v19 = (*(short *)(v16 + 2) << 7) + *(short *)v16;
							if(*((int*)v17 + 1)) {
								((ffeVertex*)v29)->x_2 *= *(int*)(*((int*)v17 + 1) + v19) & 0xff;
							}
							((ffeVertex*)v29)->x_2 += *(int*)(*(int*)v17 + v19) & 0xff << 21;
						}
					}
				}
				++v18;
				v16 += 4;
				v17 += 256;
			}
			while ( v18 < 2 );
			*(int *)(v29 + 4) = ((*(int *)((int)A10 + 4) + *(int *)((int)Ac + 4)) >> 1)
				+ C_FUNC_001521_MulWithNormalize(*(int *)(v29 + 40), v31);
			*(int *)(v29 + 8) = ((*(int *)((int)A10 + 8) + *(int *)((int)Ac + 8)) >> 1)
				+ C_FUNC_001521_MulWithNormalize(*(int *)(v29 + 44), v31);
			*(int *)(v29 + 12) = ((*(int *)((int)A10 + 12) + *(int *)((int)Ac + 12)) >> 1)
				+ C_FUNC_001521_MulWithNormalize(*(int *)(v29 + 48), v31);
			shiftCall = (char *(*)(int)) *(int*)(DATA_009282);
			shiftCall((int)v29);
		}
		else
		{
			*(int *)(v29 + 60) = (*(int *)((int)A10 + 60) + *(int *)((int)Ac + 60)) >> 1;
			*(int *)(v29 + 4) = (*(int *)((int)A10 + 4) + *(int *)((int)Ac + 4)) >> 1;
			*(int *)(v29 + 8) = (*(int *)((int)A10 + 8) + *(int *)((int)Ac + 8)) >> 1;
			*(int *)(v29 + 12) = (*(int *)((int)A10 + 12) + *(int *)((int)Ac + 12)) >> 1;
			if ( *(int *)((int)Ac + 12) >= 64 && *(int *)((int)A10 + 12) >= 64 )
			{
				*(short *)v29 = (*(short *)(int)A10 + (signed int)*(short *)(int)Ac) >> 1;
				*(short *)(v29 + 2) = (*(short *)((int)A10 + 2) + (signed int)*(short *)((int)Ac + 2)) >> 1;
				if ( *(int *)((int)A14 + 12) > 64 )
				{
					v25 = *(short *)((int)Ac + 2) > *(short *)((int)A10 + 2);
					v26 = *(short *)(int)Ac > *(short *)(int)A10;
					v27 = (*(short *)((int)A10 + 2) - (signed int)*(short *)((int)Ac + 2)) * (*(short *)(int)A14 - (signed int)*(short *)(int)Ac) > (*(short *)((int)A14 + 2) - (signed int)*(short *)((int)Ac + 2)) * (*(short *)(int)A10 - (signed int)*(short *)(int)Ac);
					((ffeVertex*)v29)->orto_x += *(int*)((int)DATA_007882 + 16 * v25 + 8 * v26 + 4 * v27);
					((ffeVertex*)v29)->orto_y += *(int*)((int)DATA_007883 + 16 * v25 + 8 * v26 + 4 * v27);
				}
				shiftCall2 = (int(*)(short)) *(int*)DATA_009280;
				((ffeVertex*)v29)->orto_x_2 = (short)shiftCall2(((ffeVertex*)v29)->orto_x);
				shiftCall2 = (int(*)(short)) *(int*)DATA_009281;
				((ffeVertex*)v29)->orto_y_2 = (short)shiftCall2(((ffeVertex*)v29)->orto_y);
			}
			else
			{
				shiftCall = (char *(*)(int)) *(int*)(DATA_009282);
				shiftCall(v29);
			}
		}
		C_FUNC_001829_GetTriangulateDepth((ffeVertex*)v29);
	}
	return 0;
}

extern "C" int C_FUNC_001864(int* A8, ffeVertex* Ac, ffeVertex* A10, ffeVertex* A14, int A18)
{
	int v5; // eax@1
	int v6; // ebx@1
	int result; // eax@2
	int v8; // edx@2
	int v9; // ecx@2
	signed int v10; // esi@2
	int v11; // eax@10
	int v12; // edx@10
	int v13; // ecx@10
	signed int v14; // esi@10
	int v15; // eax@22
	int v16; // edx@22
	int v17; // ebx@22
	signed int v18; // esi@22
	int v19; // eax@7
	int v20; // eax@8
	int v21; // eax@8
	int v22; // eax@9
	int v23; // eax@9
	bool v24; // eax@20
	bool v25; // edx@20
	bool v26; // ecx@20
	int v27; // eax@30
	int v28; // ebx@30
	int v29; // [sp+10h] [bp-10h]@2
	int v30; // [sp+18h] [bp-8h]@7
	int v31; // [sp+Ch] [bp-14h]@10
	int v32; // [sp+14h] [bp-Ch]@14
	int v33; // [sp+1Ch] [bp-4h]@26

	v6 = (int)A8;
	v5 = *(int *)((int)A8 + 8);
	if ( v5 )
	{
		v10 = 0;
		v9 = v5 + 52;
		v8 = (int)A10 + 52;
		result = (int)Ac + 52;
		v29 = A18 * 8 + (int)DATA_009260;
		do
		{
			if ( *(int *)v29)
			{
				*(short *)v9 = (*(short *)v8 + (signed int)*(short *)result) >> 1;
				*(short *)(v9 + 2) = (*(short *)(v8 + 2) + (signed int)*(short *)(result + 2)) >> 1;
			}
			++v10;
			v9 += 4;
			v8 += 4;
			result += 4;
			v29 += 256;
		}
		while ( v10 < 2 );
	}
	else
	{
		v19 = (int)C_FUNC_001818_ArrayRemoveFirstElement();
		*(int *)(v6 + 8) = v19;
		v30 = v19;
		if ( *(int *)v6 == (int)Ac )
		{
			v20 = (int)C_FUNC_001820_ArrayCreateNewElement();
			*(int *)(v6 + 4) = v20;
			*(int *)v20 = (int)Ac;
			v21 = (int)C_FUNC_001820_ArrayCreateNewElement();
			*(int *)(v6 + 12) = v21;
			*(int *)v21 = (int)A10;
		}
		else
		{
			v22 = (int)C_FUNC_001820_ArrayCreateNewElement();
			*(int *)(v6 + 4) = v22;
			*(int *)v22 = (int)A10;
			v23 = (int)C_FUNC_001820_ArrayCreateNewElement();
			*(int *)(v6 + 12) = v23;
			*(int *)v23 = (int)Ac;
		}
		v14 = 0;
		v12 = v30 + 52;
		v11 = (int)A10 + 52;
		v13 = (int)Ac + 52;
		v31 = A18 * 8 + (int)DATA_009260;
		do
		{
			if ( *(int *)v31)
			{
				*(short *)v12 = (*(short *)v11 + (signed int)*(short *)v13) >> 1;
				*(short *)(v12 + 2) = (*(short *)(v11 + 2) + (signed int)*(short *)(v13 + 2)) >> 1;
			}
			++v14;
			v12 += 4;
			v11 += 4;
			v13 += 4;
			v31 += 256;
		}
		while ( v14 < 2 );
		*(int *)(v30 + 40) = C_FUNC_001521_MulWithNormalize(*(int *)((int)A10 + 40) + *(int *)((int)Ac + 40), *(int*)(A18 * 4 + (int)DATA_007840));
		*(int *)(v30 + 44) = C_FUNC_001521_MulWithNormalize(*(int *)((int)A10 + 44) + *(int *)((int)Ac + 44), *(int*)(A18 * 4 + (int)DATA_007840));
		*(int *)(v30 + 48) = C_FUNC_001521_MulWithNormalize(*(int *)((int)A10 + 48) + *(int *)((int)Ac + 48), *(int*)(A18 * 4 + (int)DATA_007840));
		v32 = *(int*)(A18 * 4 + (int)DATA_007839);
		if ( *(int *)((int)Ac + 68) >= A18 || *(int *)((int)A10 + 68) >= A18 )
		{
			v17 = (*(int *)((int)A10 + 60) + *(int *)((int)Ac + 60)) >> 1;
			*(int *)(v30 + 60) = v17 + C_FUNC_001846(*(int *)((int)Ac + 60), *(int *)((int)A10 + 60), A18);
			v18 = 0;
			v15 = v30 + 52;
			v16 = A18 * 8 + (int)DATA_009260;
			do
			{
				if ( *(int *)v16)
				{
					if ( (unsigned int)*(short *)v15 < 0x80 )
					{
						if ( (unsigned int)*(short *)(v15 + 2) < 0x80 )
						{
							*(int *)(v30 + 60) >>= 7;
							v33 = (*(short *)(v15 + 2) << 7) + *(short *)v15;
							if(*((int*)v16 + 1)) {
								((ffeVertex*)v30)->x_2 *= *(int*)(*((int*)v16 + 1) + v33) & 0xff;
							}
							((ffeVertex*)v30)->x_2 += *(int*)(*(int*)v16 + v33) & 0xff << 21;
						}
					}
				}
				++v18;
				v15 += 4;
				v16 += 256;
			}
			while ( v18 < 2 );
			v27 = v32 + ((*(int *)(v30 + 60) - v17) >> 3);
			v28 = v32 + ((*(int *)(v30 + 60) - v17) >> 3);
			*(int *)(v30 + 4) = ((*(int *)((int)A10 + 4) + *(int *)((int)Ac + 4)) >> 1)
				+ C_FUNC_001521_MulWithNormalize(*(int *)(v30 + 40), v27);
			*(int *)(v30 + 8) = ((*(int *)((int)A10 + 8) + *(int *)((int)Ac + 8)) >> 1)
				+ C_FUNC_001521_MulWithNormalize(*(int *)(v30 + 44), v28);
			*(int *)(v30 + 12) = ((*(int *)((int)A10 + 12) + *(int *)((int)Ac + 12)) >> 1)
				+ C_FUNC_001521_MulWithNormalize(*(int *)(v30 + 48), v28);
			shiftCall = (char *(*)(int)) *(int*)(DATA_009282);
			shiftCall((int)v30);
		}
		else
		{
			*(int *)(v30 + 60) = (*(int *)((int)A10 + 60) + *(int *)((int)Ac + 60)) >> 1;
			*(int *)(v30 + 4) = (*(int *)((int)A10 + 4) + *(int *)((int)Ac + 4)) >> 1;
			*(int *)(v30 + 8) = (*(int *)((int)A10 + 8) + *(int *)((int)Ac + 8)) >> 1;
			*(int *)(v30 + 12) = (*(int *)((int)A10 + 12) + *(int *)((int)Ac + 12)) >> 1;
			if ( *(int *)((int)Ac + 12) >= 64 && *(int *)((int)A10 + 12) >= 64 )
			{
				*(short *)v30 = (*(short *)(int)A10 + (signed int)*(short *)(int)Ac) >> 1;
				*(short *)(v30 + 2) = (*(short *)((int)A10 + 2) + (signed int)*(short *)((int)Ac + 2)) >> 1;
				if ( *(int *)((int)A14 + 12) > 64 )
				{
					v24 = *(short *)((int)Ac + 2) > *(short *)((int)A10 + 2);
					v25 = *(short *)(int)Ac > *(short *)(int)A10;
					v26 = (*(short *)((int)A10 + 2) - (signed int)*(short *)((int)Ac + 2)) * (*(short *)(int)A14 - (signed int)*(short *)(int)Ac) > (*(short *)((int)A14 + 2) - (signed int)*(short *)((int)Ac + 2)) * (*(short *)(int)A10 - (signed int)*(short *)(int)Ac);
					((ffeVertex*)v30)->orto_x += *(int*)((int)DATA_007884 + 16 * v24 + 8 * v25 + 4 * v26);
					((ffeVertex*)v30)->orto_y += *(int*)((int)DATA_007885 + 16 * v24 + 8 * v25 + 4 * v26);
				}
				shiftCall2 = (int(*)(short)) *(int*)DATA_009280;
				((ffeVertex*)v30)->orto_x_2 = (short)shiftCall2(((ffeVertex*)v30)->orto_x);
				shiftCall2 = (int(*)(short)) *(int*)DATA_009281;
				((ffeVertex*)v30)->orto_y_2 = (short)shiftCall2(((ffeVertex*)v30)->orto_y);
			}
			else
			{
				shiftCall = (char *(*)(int)) *(int*)(DATA_009282);
				shiftCall(v30);
			}
		}
		C_FUNC_001829_GetTriangulateDepth((ffeVertex*)v30);
	}
	return 0;
}

extern "C" int C_FUNC_001865(int* A8, ffeVertex* Ac, ffeVertex* A10, ffeVertex* A14, int A18)
{
	int v5; // eax@1
	int v6; // esi@1
	int result; // eax@2
	int v8; // edx@2
	int v9; // ecx@2
	signed int v10; // esi@2
	int v11; // ebx@7
	int v12; // eax@10
	int v13; // edx@10
	signed int v14; // esi@10
	int v15; // eax@15
	int v16; // eax@19
	int v17; // eax@29
	int v18; // edx@29
	signed int v19; // esi@29
	int v20; // eax@38
	int v21; // eax@7
	int v22; // eax@8
	int v23; // eax@8
	int v24; // eax@9
	int v25; // eax@9
	int v26; // eax@21
	bool v27; // eax@27
	bool v28; // edx@27
	int v29; // esi@40
	int v30; // [sp+14h] [bp-10h]@2
	int v31; // [sp+Ch] [bp-18h]@10
	int v32; // [sp+10h] [bp-14h]@10
	int v33; // [sp+1Ch] [bp-8h]@14
	bool v34; // [sp+18h] [bp-Ch]@27
	int v35; // [sp+20h] [bp-4h]@33

	v6 = (int)A8;
	v5 = *(int *)((int)A8 + 8);
	if ( v5 )
	{
		v10 = 0;
		v9 = v5 + 52;
		v8 = (int)A10 + 52;
		result = (int)Ac + 52;
		v30 = A18 * 8 + (int)DATA_009260;
		do
		{
			if ( *(int *)v30)
			{
				*(short *)v9 = (*(short *)v8 + (signed int)*(short *)result) >> 1;
				*(short *)(v9 + 2) = (*(short *)(v8 + 2) + (signed int)*(short *)(result + 2)) >> 1;
			}
			++v10;
			v9 += 4;
			v8 += 4;
			result += 4;
			v30 += 256;
		}
		while ( v10 < 2 );
	}
	else
	{
		v21 = (int)C_FUNC_001818_ArrayRemoveFirstElement();
		*(int *)(v6 + 8) = v21;
		v11 = v21;
		if ( *(int *)v6 == (int)Ac )
		{
			v22 = (int)C_FUNC_001820_ArrayCreateNewElement();
			*(int *)(v6 + 4) = v22;
			*(int *)v22 = (int)Ac;
			v23 = (int)C_FUNC_001820_ArrayCreateNewElement();
			*(int *)(v6 + 12) = v23;
			*(int *)v23 = (int)A10;
		}
		else
		{
			v24 = (int)C_FUNC_001820_ArrayCreateNewElement();
			*(int *)(v6 + 4) = v24;
			*(int *)v24 = (int)A10;
			v25 = (int)C_FUNC_001820_ArrayCreateNewElement();
			*(int *)(v6 + 12) = v25;
			*(int *)v25 = (int)Ac;
		}
		v14 = 0;
		v13 = v11 + 52;
		v12 = (int)A10 + 52;
		v31 = (int)Ac + 52;
		v32 = A18 * 8 + (int)DATA_009260;
		do
		{
			if ( *(int *)v32)
			{
				*(short *)v13 = (*(short *)v12 + (signed int)*(short *)v31) >> 1;
				*(short *)(v13 + 2) = (*(short *)(v12 + 2) + (signed int)*(short *)(v31 + 2)) >> 1;
			}
			++v14;
			v13 += 4;
			v12 += 4;
			v31 += 4;
			v32 += 256;
		}
		while ( v14 < 2 );
		*(int *)(v11 + 40) = C_FUNC_001521_MulWithNormalize(*(int *)((int)A10 + 40) + *(int *)((int)Ac + 40), *(int*)(A18 * 4 + (int)DATA_007840));
		*(int *)(v11 + 44) = C_FUNC_001521_MulWithNormalize(*(int *)((int)A10 + 44) + *(int *)((int)Ac + 44), *(int*)(A18 * 4 + (int)DATA_007840));
		*(int *)(v11 + 48) = C_FUNC_001521_MulWithNormalize(*(int *)((int)A10 + 48) + *(int *)((int)Ac + 48), *(int*)(A18 * 4 + (int)DATA_007840));
		v33 = *(int*)(A18 * 4 + (int)DATA_007839);
		if ( ((*(int *)((int)A10 + 60) - 1) ^ (*(int *)((int)Ac + 60) - 1)) >= 0 )
			v15 = A18;
		else
			v15 = A18 - 1;
		if ( v15 <= *(int *)((int)Ac + 68) || v15 <= *(int *)((int)A10 + 68) )
		{
			*(int *)(v11 + 60) = ((*(int *)((int)A10 + 60) + *(int *)((int)Ac + 60)) >> 1)
				+ C_FUNC_001846(*(int *)((int)Ac + 60), *(int *)((int)A10 + 60), A18);
			v19 = 0;
			v17 = v11 + 52;
			v18 = A18 * 8 + (int)DATA_009260;
			do
			{
				if ( *(int *)v18)
				{
					if ( (unsigned int)*(short *)v17 < 0x80 )
					{
						if ( (unsigned int)*(short *)(v17 + 2) < 0x80 )
						{
							*(int *)(v11 + 60) >>= 7;
							v35 = (*(short *)(v17 + 2) << 7) + *(short *)v17;
							if(*((int*)v18 + 1)) {
								((ffeVertex*)v11)->x_2 *= *(int*)(*((int*)v18 + 1) + v35) & 0xff;
							}
							((ffeVertex*)v11)->x_2 += *(int*)(*(int*)v18 + v35) & 0xff << 21;
						}
					}
				}
				++v19;
				v17 += 4;
				v18 += 256;
			}
			while ( v19 < 2 );
			if ( *(int *)(v11 + 60) < 0 )
				v20 = 0;
			else
				v20 = *(int *)(v11 + 60);
			*(int *)(v11 + 64) = v20;
			v29 = v33 + ((v20 - ((*(int *)((int)A10 + 64) + *(int *)((int)Ac + 64)) >> 1)) >> 3);
			*(int *)(v11 + 4) = ((*(int *)((int)A10 + 4) + *(int *)((int)Ac + 4)) >> 1)
				+ C_FUNC_001521_MulWithNormalize(
				*(int *)(v11 + 40),
				v33 + ((v20 - ((*(int *)((int)A10 + 64) + *(int *)((int)Ac + 64)) >> 1)) >> 3));
			*(int *)(v11 + 8) = ((*(int *)((int)A10 + 8) + *(int *)((int)Ac + 8)) >> 1)
				+ C_FUNC_001521_MulWithNormalize(*(int *)(v11 + 44), v29);
			*(int *)(v11 + 12) = ((*(int *)((int)A10 + 12) + *(int *)((int)Ac + 12)) >> 1)
				+ C_FUNC_001521_MulWithNormalize(*(int *)(v11 + 48), v29);
			shiftCall = (char *(*)(int)) *(int*)(DATA_009282);
			shiftCall((int)v11);
		}
		else
		{
			v16 = *(int *)((int)Ac + 64);
			if ( v16 || *(int *)((int)A10 + 64) )
			{
				v26 = (*(int *)((int)A10 + 64) + v16) >> 1;
				*(int *)(v11 + 64) = v26;
				*(int *)(v11 + 60) = v26;
			}
			else
			{
				*(int *)(v11 + 60) = (*(int *)((int)A10 + 60) + *(int *)((int)Ac + 60)) >> 1;
				*(int *)(v11 + 64) = 0;
			}
			*(int *)(v11 + 4) = (*(int *)((int)A10 + 4) + *(int *)((int)Ac + 4)) >> 1;
			*(int *)(v11 + 8) = (*(int *)((int)A10 + 8) + *(int *)((int)Ac + 8)) >> 1;
			*(int *)(v11 + 12) = (*(int *)((int)A10 + 12) + *(int *)((int)Ac + 12)) >> 1;
			if ( *(int *)((int)Ac + 12) >= 64 && *(int *)((int)A10 + 12) >= 64 )
			{
				*(short *)v11 = (*(short *)(int)A10 + (signed int)*(short *)(int)Ac) >> 1;
				*(short *)(v11 + 2) = (*(short *)((int)A10 + 2) + (signed int)*(short *)((int)Ac + 2)) >> 1;
				if ( *(int *)((int)A14 + 12) > 64 )
				{
					v27 = *(short *)((int)Ac + 2) > *(short *)((int)A10 + 2);
					v34 = *(short *)(int)Ac > *(short *)(int)A10;
					v28 = (*(short *)((int)A10 + 2) - (signed int)*(short *)((int)Ac + 2)) * (*(short *)(int)A14 - (signed int)*(short *)(int)Ac) > (*(short *)((int)A14 + 2) - (signed int)*(short *)((int)Ac + 2)) * (*(short *)(int)A10 - (signed int)*(short *)(int)Ac);
					((ffeVertex*)v11)->orto_x += *(int*)((int)DATA_007886 + 16 * v27 + 8 * v34 + 4 * v28);
					((ffeVertex*)v11)->orto_y += *(int*)((int)DATA_007887 + 16 * v27 + 8 * v34 + 4 * v28);
				}
				shiftCall2 = (int(*)(short)) *(int*)DATA_009280;
				((ffeVertex*)v11)->orto_x_2 = (short)shiftCall2(((ffeVertex*)v11)->orto_x);
				shiftCall2 = (int(*)(short)) *(int*)DATA_009281;
				((ffeVertex*)v11)->orto_y_2 = (short)shiftCall2(((ffeVertex*)v11)->orto_y);
			}
			else
			{
				shiftCall = (char *(*)(int)) *(int*)(DATA_009282);
				shiftCall(v11);
			}
		}
		C_FUNC_001829_GetTriangulateDepth((ffeVertex*)v11);
	}
	return 0;
}

extern "C" int C_FUNC_001866(int* A8, ffeVertex* Ac, ffeVertex* A10, ffeVertex* A14, int A18)
{
	int result; // eax@1
	int v6; // edi@1
	int v7; // esi@1
	int v8; // ebx@2
	int v9; // eax@2
	int v10; // eax@3
	int v11; // eax@3
	int v12; // eax@4
	int v13; // eax@4
	bool v14; // eax@11
	bool v16; // [sp+10h] [bp-4h]@11
	bool v17; // [sp+Ch] [bp-8h]@11

	v7 = (int)Ac;
	v6 = (int)A8;
	result = *(int *)((int)A8 + 8);
	if ( !result )
	{
		v9 = C_FUNC_001818_ArrayRemoveFirstElement();
		*(int *)(v6 + 8) = v9;
		v8 = v9;
		if ( v7 == *(int *)v6 )
		{
			v10 = (int)C_FUNC_001820_ArrayCreateNewElement();
			*(int *)(v6 + 4) = v10;
			*(int *)v10 = v7;
			v11 = (int)C_FUNC_001820_ArrayCreateNewElement();
			*(int *)(v6 + 12) = v11;
			*(int *)v11 = (int)A10;
		}
		else
		{
			v12 = (int)C_FUNC_001820_ArrayCreateNewElement();
			*(int *)(v6 + 4) = v12;
			*(int *)v12 = (int)A10;
			v13 = (int)C_FUNC_001820_ArrayCreateNewElement();
			*(int *)(v6 + 12) = v13;
			*(int *)v13 = v7;
		}
		*(int *)(v8 + 40) = *(int *)(v7 + 40);
		*(int *)(v8 + 44) = *(int *)(v7 + 44);
		*(int *)(v8 + 48) = *(int *)(v7 + 48);
		if ( A18 <= *(int *)(v7 + 68) || A18 <= *(int *)((int)A10 + 68) )
		{
			*(int *)(v8 + 60) = (*(int *)((int)A10 + 60) + *(int *)(v7 + 60)) >> 1;
			*(int *)(v8 + 4) = ((*(int *)((int)A10 + 4) + *(int *)(v7 + 4)) >> 1) + C_FUNC_001521_MulWithNormalize(*(int *)(v8 + 40), 0);
			*(int *)(v8 + 8) = ((*(int *)((int)A10 + 8) + *(int *)(v7 + 8)) >> 1) + C_FUNC_001521_MulWithNormalize(*(int *)(v8 + 44), 0);
			*(int *)(v8 + 12) = ((*(int *)((int)A10 + 12) + *(int *)(v7 + 12)) >> 1) + C_FUNC_001521_MulWithNormalize(*(int *)(v8 + 48), 0);
			shiftCall = (char *(*)(int)) *(int*)(DATA_009282);
			shiftCall((int)v8);
		}
		else
		{
			*(int *)(v8 + 60) = (*(int *)((int)A10 + 60) + *(int *)(v7 + 60)) >> 1;
			*(int *)(v8 + 4) = (*(int *)((int)A10 + 4) + *(int *)(v7 + 4)) >> 1;
			*(int *)(v8 + 8) = (*(int *)((int)A10 + 8) + *(int *)(v7 + 8)) >> 1;
			*(int *)(v8 + 12) = (*(int *)((int)A10 + 12) + *(int *)(v7 + 12)) >> 1;
			if ( *(int *)(v7 + 12) >= 64 && *(int *)((int)A10 + 12) >= 64 )
			{
				*(short *)v8 = (*(short *)(int)A10 + (signed int)*(short *)v7) >> 1;
				*(short *)(v8 + 2) = (*(short *)((int)A10 + 2) + (signed int)*(short *)(v7 + 2)) >> 1;
				if ( *(int *)((int)A14 + 12) > 64 )
				{
					v16 = *(short *)(v7 + 2) > *(short *)((int)A10 + 2);
					v17 = *(short *)v7 > *(short *)(int)A10;
					v14 = (*(short *)((int)A10 + 2) - (signed int)*(short *)(v7 + 2)) * (*(short *)(int)A14 - (signed int)*(short *)v7) > (*(short *)((int)A14 + 2) - (signed int)*(short *)(v7 + 2)) * (*(short *)(int)A10 - (signed int)*(short *)v7);
					((ffeVertex*)v8)->orto_x += *(int*)((int)DATA_007888 + 16 * v16 + 8 * v17 + 4 * v14);
					((ffeVertex*)v8)->orto_y += *(int*)((int)DATA_007889 + 16 * v16 + 8 * v17 + 4 * v14);
				}
				shiftCall2 = (int(*)(short)) *(int*)DATA_009280;
				((ffeVertex*)v8)->orto_x_2 = (short)shiftCall2(((ffeVertex*)v8)->orto_x);
				shiftCall2 = (int(*)(short)) *(int*)DATA_009281;
				((ffeVertex*)v8)->orto_y_2 = (short)shiftCall2(((ffeVertex*)v8)->orto_y);
			}
			else
			{
				shiftCall = (char *(*)(int)) *(int*)(DATA_009282);
				shiftCall(v8);
			}
		}
		C_FUNC_001829_GetTriangulateDepth((ffeVertex*)v8);
	}
	return 0;
}

extern "C" int C_FUNC_001867(int* A8, ffeVertex* Ac, ffeVertex* A10, ffeVertex* A14, int A18)
{
	int result; // eax@1
	int v6; // edi@1
	int v7; // esi@1
	int v8; // ebx@2
	int v9; // edi@5
	int v10; // eax@2
	int v11; // eax@3
	int v12; // eax@3
	int v13; // eax@4
	int v14; // eax@4
	bool v15; // eax@11
	bool v17; // [sp+10h] [bp-4h]@11
	bool v18; // [sp+Ch] [bp-8h]@11

	v7 = (int)Ac;
	v6 = (int)A8;
	result = *(int *)((int)A8 + 8);
	if ( !result )
	{
		v10 = C_FUNC_001818_ArrayRemoveFirstElement();
		*(int *)(v6 + 8) = v10;
		v8 = v10;
		if ( v7 == *(int *)v6 )
		{
			v11 = (int)C_FUNC_001820_ArrayCreateNewElement();
			*(int *)(v6 + 4) = v11;
			*(int *)v11 = v7;
			v12 = (int)C_FUNC_001820_ArrayCreateNewElement();
			*(int *)(v6 + 12) = v12;
			*(int *)v12 = (int)A10;
		}
		else
		{
			v13 = (int)C_FUNC_001820_ArrayCreateNewElement();
			*(int *)(v6 + 4) = v13;
			*(int *)v13 = (int)A10;
			v14 = (int)C_FUNC_001820_ArrayCreateNewElement();
			*(int *)(v6 + 12) = v14;
			*(int *)v14 = v7;
		}
		*(int *)(v8 + 40) = C_FUNC_001521_MulWithNormalize(*(int *)((int)A10 + 40) + *(int *)(v7 + 40), *(int*)(A18 * 4 + (int)DATA_007840));
		*(int *)(v8 + 44) = C_FUNC_001521_MulWithNormalize(*(int *)((int)A10 + 44) + *(int *)(v7 + 44), *(int*)(A18 * 4 + (int)DATA_007840));
		*(int *)(v8 + 48) = C_FUNC_001521_MulWithNormalize(*(int *)((int)A10 + 48) + *(int *)(v7 + 48), *(int*)(A18 * 4 + (int)DATA_007840));
		v9 = *(int*)(A18 * 4 + (int)DATA_007839);
		if ( A18 <= *(int *)(v7 + 68) || A18 <= *(int *)((int)A10 + 68) )
		{
			*(int *)(v8 + 60) = (*(int *)((int)A10 + 60) + *(int *)(v7 + 60)) >> 1;
			*(int *)(v8 + 4) = ((*(int *)((int)A10 + 4) + *(int *)(v7 + 4)) >> 1) + C_FUNC_001521_MulWithNormalize(*(int *)(v8 + 40), v9);
			*(int *)(v8 + 8) = ((*(int *)((int)A10 + 8) + *(int *)(v7 + 8)) >> 1) + C_FUNC_001521_MulWithNormalize(*(int *)(v8 + 44), v9);
			*(int *)(v8 + 12) = ((*(int *)((int)A10 + 12) + *(int *)(v7 + 12)) >> 1)
				+ C_FUNC_001521_MulWithNormalize(*(int *)(v8 + 48), v9);
			shiftCall = (char *(*)(int)) *(int*)(DATA_009282);
			shiftCall((int)v8);
		}
		else
		{
			*(int *)(v8 + 60) = (*(int *)((int)A10 + 60) + *(int *)(v7 + 60)) >> 1;
			*(int *)(v8 + 4) = (*(int *)((int)A10 + 4) + *(int *)(v7 + 4)) >> 1;
			*(int *)(v8 + 8) = (*(int *)((int)A10 + 8) + *(int *)(v7 + 8)) >> 1;
			*(int *)(v8 + 12) = (*(int *)((int)A10 + 12) + *(int *)(v7 + 12)) >> 1;
			if ( *(int *)(v7 + 12) >= 64 && *(int *)((int)A10 + 12) >= 64 )
			{
				*(short *)v8 = (*(short *)(int)A10 + (signed int)*(short *)v7) >> 1;
				*(short *)(v8 + 2) = (*(short *)((int)A10 + 2) + (signed int)*(short *)(v7 + 2)) >> 1;
				if ( *(int *)((int)A14 + 12) > 64 )
				{
					v17 = *(short *)(v7 + 2) > *(short *)((int)A10 + 2);
					v18 = *(short *)v7 > *(short *)(int)A10;
					v15 = (*(short *)((int)A10 + 2) - (signed int)*(short *)(v7 + 2)) * (*(short *)(int)A14 - (signed int)*(short *)v7) > (*(short *)((int)A14 + 2) - (signed int)*(short *)(v7 + 2)) * (*(short *)(int)A10 - (signed int)*(short *)v7);
					((ffeVertex*)v8)->orto_x += *(int*)((int)DATA_007890 + 16 * v17 + 8 * v18 + 4 * v15);
					((ffeVertex*)v8)->orto_y += *(int*)((int)DATA_007891 + 16 * v17 + 8 * v18 + 4 * v15);
				}
				shiftCall2 = (int(*)(short)) *(int*)DATA_009280;
				((ffeVertex*)v8)->orto_x_2 = (short)shiftCall2(((ffeVertex*)v8)->orto_x);
				shiftCall2 = (int(*)(short)) *(int*)DATA_009281;
				((ffeVertex*)v8)->orto_y_2 = (short)shiftCall2(((ffeVertex*)v8)->orto_y);
			}
			else
			{
				shiftCall = (char *(*)(int)) *(int*)(DATA_009282);
				shiftCall(v8);
			}
		}
		C_FUNC_001829_GetTriangulateDepth((ffeVertex*)v8);
	}
	return 0;
}

/*
FUNC_001829:			; Pos = 62a34

		push ebp
		mov ebp,esp
		push ebx
		push esi
		push edi
		mov ebx,[ebp+0x8]
		mov esi,[ebx+0xc]
		cmp esi,0x400
		jnl JUMP_007017
		mov esi,0x400
	JUMP_007017:			; Pos = 62a4d
		push esi
		call FUNC_001656_FindMSB
		pop ecx
		mov edi,[DATA_009269]
		sub edi,eax
		mov [ebx+0x44],edi
		dec edi
		jnl JUMP_007018
		mov dword [ebx+0x44],0x1
	JUMP_007018:			; Pos = 62a69
		pop edi
		pop esi
		pop ebx
		pop ebp
		ret

*/

extern "C" inline void C_FUNC_001829_GetTriangulateDepth_2(ffeVertex* AN, ffeVertex* A8, ffeVertex* Ac, ffeVertex* A10)
{
    int  esi;
    int  edi;
	int c_dist;
	int m_d_deep;
	int opt_c;

	C_FUNC_001829_GetTriangulateDepth(AN);
	/*
	int scaleFactor = (mod->Scale + mod->Scale2 - 8);
	int radius=(mod->field_2C-1);
	//A8->x_2=mod->field_2C;
	//Ac->x_2=mod->field_2C;
	//A10->x_2=mod->field_2C;
	if (scaleFactor < 0) {
		radius = (radius << -scaleFactor);
	} else {
		radius = (radius >> scaleFactor);
	}
	AN->x_2=radius;
	*/
	c_dist = control_dist;
	opt_c = smart_optimizer_c;

	esi = AN->nz;
	edi = *(int*)DATA_009269;

	if (currentModel==447 || currentModel==449) {
		C_FUNC_001829_GetTriangulateDepth(AN);
	} else {
		if (esi<c_dist) {
			edi = edi;
		} else if (esi<c_dist*2) {
			edi = edi-1;
		} else if (esi<c_dist*4) {
			edi = edi-2;
		} else if (esi<c_dist*8) {
			edi = edi-3;
		} else if (esi<c_dist*16) {
			edi = edi-4;
		} else if (esi<c_dist*32) {
			edi = edi-5;
		} else if (esi<c_dist*64) {
			edi = edi-6;
		} else if (esi<c_dist*128) {
			edi = edi-7;
		} else if (esi<c_dist*256) {
			edi = edi-8;
		} else if (esi<c_dist*512) {
			edi = edi-9;
		} else {
			edi = 1;
		}
		if (smart_optimizer) {
			edi -= C_FUNC_001656_FindMSB(*(int*)DATA_009276_ArraySize)/2-opt_c;
		}
		AN->z_2 = edi;
		edi--;
		if(edi<=0) {
			AN->z_2 = 1;
		}
	}
}



extern "C" inline void C_FUNC_001829_GetTriangulateDepth(ffeVertex* A8)
{
	int  eax;
    int  esi;
    int  edi;

	esi = A8->nz;

    if(esi < 0x400) {
        esi = 0x400;
    }
    eax = C_FUNC_001656_FindMSB(esi);
	edi = *(int*)DATA_009269 - eax;
	A8->z_2 = edi;
	edi--;
    if(edi<=0) {
		A8->z_2 = 1;
    }
}

void callTriangleDeliver(int* A8, ffeVertex* Ac, ffeVertex* A10, ffeVertex* A14, int A18, int num)
{
	
	switch(num) {

		case 0:		C_FUNC_001849(A8, Ac, A10, A14, A18); break;
		case 1:		C_FUNC_001849(A8, Ac, A10, A14, A18); break;
		case 2:		C_FUNC_001849(A8, Ac, A10, A14, A18); break;
		case 3:		C_FUNC_001849(A8, Ac, A10, A14, A18); break;
		case 4:		C_FUNC_001849(A8, Ac, A10, A14, A18); break;
		case 5:		C_FUNC_001849(A8, Ac, A10, A14, A18); break;
		case 6:		C_FUNC_001849(A8, Ac, A10, A14, A18); break;
		case 7:		C_FUNC_001849(A8, Ac, A10, A14, A18); break;
		case 8:		C_FUNC_001850(A8, Ac, A10, A14, A18); break;
		case 9:		C_FUNC_001851(A8, Ac, A10, A14, A18); break;
		case 10:	C_FUNC_001852(A8, Ac, A10, A14, A18); break;
		case 11:	C_FUNC_001853(A8, Ac, A10, A14, A18); break;
		case 12:	C_FUNC_001854(A8, Ac, A10, A14, A18); break;
		case 13:	C_FUNC_001855(A8, Ac, A10, A14, A18); break;
		case 14:	C_FUNC_001856(A8, Ac, A10, A14, A18); break;
		case 15:	C_FUNC_001857(A8, Ac, A10, A14, A18); break;
		case 16:	C_FUNC_001858(A8, Ac, A10, A14, A18); break;
		case 17:	C_FUNC_001859(A8, Ac, A10, A14, A18); break;
		case 18:	C_FUNC_001860(A8, Ac, A10, A14, A18); break;
		case 19:	C_FUNC_001861(A8, Ac, A10, A14, A18); break;
		case 20:	C_FUNC_001862(A8, Ac, A10, A14, A18); break;
		case 21:	C_FUNC_001863(A8, Ac, A10, A14, A18); break;
		case 22:	C_FUNC_001864(A8, Ac, A10, A14, A18); break;
		case 23:	C_FUNC_001865(A8, Ac, A10, A14, A18); break;
		case 24:	C_FUNC_001866(A8, Ac, A10, A14, A18); break;
		case 25:	C_FUNC_001866(A8, Ac, A10, A14, A18); break;
		case 26:	C_FUNC_001866(A8, Ac, A10, A14, A18); break;
		case 27:	C_FUNC_001866(A8, Ac, A10, A14, A18); break;
		case 28:	C_FUNC_001867(A8, Ac, A10, A14, A18); break;
		case 29:	C_FUNC_001867(A8, Ac, A10, A14, A18); break;
		case 30:	C_FUNC_001867(A8, Ac, A10, A14, A18); break;
		case 31:	C_FUNC_001867(A8, Ac, A10, A14, A18); break;

		default:	C_FUNC_001849(A8, Ac, A10, A14, A18); break;
	}
}

/*
FUNC_1848(A8)
{



    if(*(A8 + 4) == 0) {
        *(A8 + 0x24) = 2;
    } else {
        dx = 1;
        if(*(A8 + 4) >= 0) {
            edx = 4;
        }
        *(A8 + 0x24) = dx;
    }
    if(*(A8 + 8) == 0) {
        *(A8 + 0x26) = 2;
        return;
    }
    dx = 1;
    if(*(A8 + 8) >= 0) {
        edx = 4;
    }
    *(A8 + 0x26) = dx;
}
*/

/*

OnePlanetVertex
{
	ffeVertex[0]
	{
		short orto_x;	// 2D coordinates
		short orto_y;	//
		int nx, ny, nz; // 3D coordinates
		int unknown5;	// 
		int unknown6;	// 
		int x;			// 
		int y;			// 
		int z;			// 
	}
	ffeVertex[1]
	{
		short orto_x;	//
		short orto_y;	//
		int nx, ny, nz; // Normal
		int unknown5;	// 
		int unknown6;	// 
		int x;			// Surface height?
		int y;			// 
		int z;			// 
	}
}


*/