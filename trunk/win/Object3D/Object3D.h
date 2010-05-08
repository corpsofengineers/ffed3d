#pragma once
#include "Defines.h"
#include "..\Render\RenderSystem.h"
class Object3D
{
protected:

	sVertexBuffer				vb;
	sIndexBuffer				ib;

	DWORD						dwFVF;

	DWORD						dwNumVerticies;					// ���������� ������
	DWORD						dwNumIndecies;					// ���������� ��������
	DWORD						dwNumFaces;						// ���������� ���������

	float fBBoxLeft, fBBoxRight, fBBoxTop, fBBoxBottom, fBBoxFar, fBBoxNear;

	D3DXMATRIX					objMatr;						// �������������� �������
	D3DXMATRIX					objMatrR, objMatrT, objMatrS;	// ������� ��������, ����������� � ���������������
	D3DXMATRIX					objMatrTT;						// ������������� �������

	WORD						sizeOfVertex;					// ����� ������ ���������� ��������
	VOID						*t_pVertices, *t_pIndeces;		// ��������� �� ������� ������ � �������
	DWORD						sizeOfAllVerteces;				// ����� ������ ���������� ���������
	DWORD						sizeOfAllIndeces;				// ����� ������ ���������� ���������

	VERTEX_3DPNT1				*pVertices;						// ������ ��������������� ������
	VERTEX_3DPNT1				*pObjVert;						// ������ �������� ������
	WORD						*pObjInd;						// ������ ��������

	//SGEMaterial					*pMaterial;

	bool bLoaded;
	LPD3DXMESH lpD3DXMesh;


public:
	Object3D(void);
	~Object3D(void);
	bool	Create		(char *filename);								// �������� ������� �� �����
	void	Render		(void);											// ������������ �������
	void	Render(int vfrom, int vto, int ffrom, int fto);
	void	Move		(float x, float y, float z);					// ���������� �������
	void	RotateX		(float x);										// ������� ������� ������ ��� X
	void	RotateY		(float y);										// ������� ������� ������ ��� Y
	void	RotateZ		(float z);										// ������� ������� ������ ��� Z
	void	Scale		(float x, float y, float z);					// ��������������� �������
	void	Transform	(void);											// ���������� ���� ��������������
	void	SetMaterial (void);
	bool	IsLoaded(){return bLoaded;}

};
