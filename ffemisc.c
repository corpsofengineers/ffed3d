//#include <stdio.h>
#include <math.h>

void BlitClipWrapper (void *pData, int xpos,
		int ypos, int width, int height, int jump,
		void (*BlitFunc)(void *, int, int, int, int, int))
{
	int srcx, srcy, t;
	int scanw = width + jump;
	
	if (xpos >= 0) 
	  srcx = 0;
	else { 
	  srcx = -xpos; 
	  width += xpos; 
	  xpos = 0; 
	}
	if (ypos >= 0) 
	  srcy = 0;
	else { 
	  srcy = -ypos; 
	  height += ypos; 
	  ypos = 0; 
	}

	if ((t = 2*xpos + width - 640) > 0) { 
	  width = width - t; 
	}
	if ((t = 2*ypos + height - 400) > 0) { 
	  height = height - t; 
	}

	jump = scanw - width;
	pData = (void *)((char *)pData + srcy*scanw + srcx);

	BlitFunc (pData, 2*xpos, 2*ypos, width, height, jump);
}	

typedef struct
{
	double x, y, z;
} DVector;

typedef struct
{
	double x1, x2, x3;	// _11, _12, _13
	double y1, y2, y3;	// _21, _22, _23
	double z1, z2, z3;	// _31, _32, _33
} DMatrix;


double VecDot (DVector *v1, DVector *v2)
{
	return (v1->x*v2->x + v1->y*v2->y + v1->z*v2->z);
}

void VecCross (DVector *v1, DVector *v2, DVector *res)
{
	res->x = v1->y*v2->z - v1->z*v2->y;
	res->y = v1->z*v2->x - v1->x*v2->z;
	res->z = v1->x*v2->y - v1->y*v2->x;
}

void MatTMatMul (DMatrix *m1, DMatrix *m2, DMatrix *res)
{
	res->x1 = m1->x1*m2->x1 + m1->y1*m2->y1 + m1->z1*m2->z1;
	res->x2 = m1->x1*m2->x2 + m1->y1*m2->y2 + m1->z1*m2->z2;
	res->x3 = m1->x1*m2->x3 + m1->y1*m2->y3 + m1->z1*m2->z3;

	res->y1 = m1->x2*m2->x1 + m1->y2*m2->y1 + m1->z2*m2->z1;
	res->y2 = m1->x2*m2->x2 + m1->y2*m2->y2 + m1->z2*m2->z2;
	res->y3 = m1->x2*m2->x3 + m1->y2*m2->y3 + m1->z2*m2->z3;

	res->z1 = m1->x3*m2->x1 + m1->y3*m2->y1 + m1->z3*m2->z1;
	res->z2 = m1->x3*m2->x2 + m1->y3*m2->y2 + m1->z3*m2->z2;
	res->z3 = m1->x3*m2->x3 + m1->y3*m2->y3 + m1->z3*m2->z3;
}

#define JJ_CPU3D_PI 3.141592654

void BuildMatrix (double xr, double yr, double zr, DMatrix *m1)
{
	const double dtr = JJ_CPU3D_PI / 180.0;

	double sx = sin (xr*dtr), cx = cos (xr*dtr);
	double sy = sin (yr*dtr), cy = cos (yr*dtr);
	double sz = sin (zr*dtr), cz = cos (zr*dtr);

	m1->x1 = cz*cy + sz*sx*sy;
	m1->x2 = sz*cx;
	m1->x3 = -cz*sy + sz*sx*cy;
	m1->y1 = -sz*cy + cz*sx*sy; 
	m1->y2 = cz*cx;
	m1->y3 = sz*sy + cz*sx*cy;
	m1->z1 = cx*sy;
	m1->z2 = -sx;
	m1->z3 = cx*cy;
}

const double fix31_max = 0x7fff8000;
const double fix31_max_inv = 1.0 / 0x7fff8000;

void ConvF31MatToFP (int *pIn, DMatrix *pOut)
{
	int i; for (i=0; i<9; i++) ((double *)pOut)[i] = pIn[i] * fix31_max_inv;
}
void ConvF31VecToFP (int *pIn, DVector *pOut)
{
	int i; for (i=0; i<3; i++) ((double *)pOut)[i] = pIn[i] * fix31_max_inv;
}
void ConvFPMatToF31 (DMatrix *pIn, int *pOut)
{
	int i; for (i=0; i<9; i++) pOut[i] = (int)(((double *)pIn)[i] * fix31_max);
}
void ConvFPVecToF31 (DMatrix *pIn, int *pOut)
{
	int i; for (i=0; i<3; i++) pOut[i] = (int)(((double *)pIn)[i] * fix31_max);
}


void rotateobject (int *pMat, int time, int *xrot, int *yrot, int *zrot)
{
	DMatrix imat, rmat, omat;
	double rmul;

	if (*xrot == 0 && *yrot == 0 && *zrot == 0) return;
//time is cap for ?rot values...
//also a cap of 9fb... or not? Shouldn't be necessary.

	ConvF31MatToFP (pMat, &imat);
	if (*xrot < -time) *xrot = -time; else if (*xrot > time) *xrot = time;
	if (*yrot < -time) *yrot = -time; else if (*yrot > time) *yrot = time;
	if (*zrot < -time) *zrot = -time; else if (*zrot > time) *zrot = time;

	rmul = -180.0 / 0x7fff;
	BuildMatrix (*xrot*rmul, *yrot*rmul, *zrot*rmul, &rmat);
	MatTMatMul (&rmat, &imat, &omat);
	ConvFPMatToF31 (&omat, pMat);
}


void testhook (int *pMat)
{
	int i;
	double t1,t2,t3;
	DVector pVec[3];
	DVector pVecT[3];
	double junk = 1.0, junk2 = 1.0 / 0x7fff8000;

	for (i=0; i<9; i++) ((double *)pVec)[i] = pMat[i] / ((double)0x7fff8000);

	t1 = VecDot (pVec+0, pVec+0);
	t2 = VecDot (pVec+1, pVec+1);
	t3 = VecDot (pVec+2, pVec+2);

	t1 = VecDot (pVec+0, pVec+1);
	t2 = VecDot (pVec+1, pVec+2);
	t3 = VecDot (pVec+0, pVec+2);

	pVecT[0].x = pVec[0].x; pVecT[0].y = pVec[1].x;	pVecT[0].z = pVec[2].x;
	pVecT[1].x = pVec[0].y; pVecT[1].y = pVec[1].y;	pVecT[1].z = pVec[2].y;
	pVecT[2].x = pVec[0].z; pVecT[2].y = pVec[1].z;	pVecT[2].z = pVec[2].z;

	t1 = VecDot (pVecT+0, pVecT+0);
	t2 = VecDot (pVecT+1, pVecT+1);
	t3 = VecDot (pVecT+2, pVecT+2);

	t1 = VecDot (pVecT+0, pVecT+1);
	t2 = VecDot (pVecT+0, pVecT+2);
	t3 = VecDot (pVecT+1, pVecT+2);

}

void testhook2 (int *pIn, double *pOut)
{
	int i, a;
	for (i=0; i<3; i++)
	{
		if (pOut[i] < (double)0x7fff8000
			&& pOut[i] > -(double)0x7fff8000) continue;
		a = 1;
	}
}
