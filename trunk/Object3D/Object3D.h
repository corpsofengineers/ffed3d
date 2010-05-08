#pragma once
#include "Defines.h"
#include "..\Render\RenderSystem.h"
class Object3D
{
protected:

	sVertexBuffer				vb;
	sIndexBuffer				ib;

	DWORD						dwFVF;

	DWORD						dwNumVerticies;					// Количество вершин
	DWORD						dwNumIndecies;					// Количество индексов
	DWORD						dwNumFaces;						// Количество полигонов

	float fBBoxLeft, fBBoxRight, fBBoxTop, fBBoxBottom, fBBoxFar, fBBoxNear;

	D3DXMATRIX					objMatr;						// Результирующая матрица
	D3DXMATRIX					objMatrR, objMatrT, objMatrS;	// Матрицы поворота, перемещения и масштабирования
	D3DXMATRIX					objMatrTT;						// Промежуточная матрица

	WORD						sizeOfVertex;					// Объем памяти занимаемый вершиной
	VOID						*t_pVertices, *t_pIndeces;		// Указатели на буфферы вершин и индеков
	DWORD						sizeOfAllVerteces;				// Объем памяти занимаемый вершинами
	DWORD						sizeOfAllIndeces;				// Объем памяти занимаемый индексами

	VERTEX_3DPNT1				*pVertices;						// Буффер преобразованных вершин
	VERTEX_3DPNT1				*pObjVert;						// Буффер исходных вершин
	WORD						*pObjInd;						// Буффер индексов

	//SGEMaterial					*pMaterial;

	bool bLoaded;
	LPD3DXMESH lpD3DXMesh;


public:
	Object3D(void);
	~Object3D(void);
	bool	Create		(char *filename);								// Создание объекта из файла
	void	Render		(void);											// Визуализация объекта
	void	Render(int vfrom, int vto, int ffrom, int fto);
	void	Move		(float x, float y, float z);					// Пермещение объекта
	void	RotateX		(float x);										// Поворот объекта вокруг оси X
	void	RotateY		(float y);										// Поворот объекта вокруг оси Y
	void	RotateZ		(float z);										// Поворот объекта вокруг оси Z
	void	Scale		(float x, float y, float z);					// Масштабирование объекта
	void	Transform	(void);											// Применение всех преобразований
	void	SetMaterial (void);
	bool	IsLoaded(){return bLoaded;}

};
