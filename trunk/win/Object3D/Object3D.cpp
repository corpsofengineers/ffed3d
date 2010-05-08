
#include "Object3D.h"
#include "assert.h"
//extern LPDIRECT3DDEVICE9 renderSystem->GetDevice();

Object3D::Object3D(void)
{
	pVertices = NULL;
	pObjVert = NULL;
	pObjInd = NULL;

	dwNumVerticies = dwNumFaces = dwNumFaces = 0;
	fBBoxLeft = fBBoxRight = fBBoxTop = fBBoxBottom = fBBoxFar = fBBoxNear = 0.0f;
	D3DXMatrixIdentity (&objMatr);
	D3DXMatrixIdentity (&objMatrR);
	D3DXMatrixIdentity (&objMatrT);
	D3DXMatrixIdentity (&objMatrS);
	D3DXMatrixIdentity (&objMatrTT);
	sizeOfVertex = sizeof(VERTEX_3DPNT1);
	sizeOfAllVerteces=0;
	sizeOfAllIndeces=0;
	bLoaded = false;
	dwFVF = 0;
	lpD3DXMesh = NULL;
}

Object3D::~Object3D(void)
{
}

//--------------------------------------------------------------------------------------------
// ���:		 Create()
// ��������: �������� ������� �� �����
//--------------------------------------------------------------------------------------------
bool Object3D::Create(char *filename)
{
//	D3DXMATERIAL* D3DXMaterial;
	LPD3DXBUFFER MatBuffer;
	DWORD		numMaterials;	

	// �������� ������ �� �����
	if (D3D_OK != D3DXLoadMeshFromX (filename, D3DXMESH_SYSTEMMEM, renderSystem->GetDevice(),NULL, NULL, &MatBuffer, &numMaterials, &lpD3DXMesh))
		return false;

	// ���� ���� ��������� �� ��������� ��
	/*
	if (numMaterials > 0)
	{
		pMaterial = new SGEMaterial;
		D3DXMaterial = (D3DXMATERIAL*)MatBuffer->GetBufferPointer();
		pMaterial->InitMaterial(D3DXMaterial->MatD3D);
	}
	*/
	// ������� ������ ������ ���������� �������� �������
	sizeOfVertex=(short)lpD3DXMesh->GetNumBytesPerVertex();;
	//if(lpD3DXMesh->GetFVF() & D3DFVF_XYZ) sizeOfVertex += sizeof(float)*3;
	//if(lpD3DXMesh->GetFVF() & D3DFVF_NORMAL) sizeOfVertex += sizeof(float)*3;
	//if(lpD3DXMesh->GetFVF() & D3DFVF_TEX1) sizeOfVertex += sizeof(float)*2;

	// ���������� �������� ������ � ��������
	lpD3DXMesh->GetVertexBuffer (&vb.vb);
	lpD3DXMesh->GetIndexBuffer	(&ib.ib);

	//D3DVERTEXELEMENT9* pDecl=NULL;
	D3DVERTEXELEMENT9 pDecl[MAX_FVF_DECL_SIZE];
	HRESULT hr = lpD3DXMesh->GetDeclaration(pDecl);
	assert(hr==D3D_OK);
	renderSystem->CreateVertexDeclaration(&pDecl[0],&vb.declaration);
	// ��������� ������ � ���������� ������, �������� � ���������
	dwNumVerticies	= lpD3DXMesh->GetNumVertices();
	dwNumIndecies	= lpD3DXMesh->GetNumFaces()*3;
	dwNumFaces		= lpD3DXMesh->GetNumFaces();
	vb.vertexSize  = sizeOfVertex;
	//vb.fvf = lpD3DXMesh->GetFVF();

	// ������ ������ ������ ��� ���� ������ � ��������
	sizeOfAllVerteces = dwNumVerticies*sizeOfVertex;
	sizeOfAllIndeces = dwNumIndecies*sizeof(WORD);

	// ��������� ������ ��� ������� ������������������ ������, ������� ������ � ��������
	pVertices = new VERTEX_3DPNT1 [dwNumVerticies];
	pObjVert = new VERTEX_3DPNT1 [dwNumVerticies];
	pObjInd = new WORD [dwNumIndecies];

	// ����������� ������ � �������
	//lpVertexBuffer->Lock (0, sizeOfAllVerteces, &t_pVertices, D3DLOCK_READONLY);
	//memcpy (pVertices, t_pVertices, sizeOfAllVerteces);
	//lpVertexBuffer->Unlock();
	//lpIndexBuffer->Lock (0, sizeOfAllIndeces, &t_pIndeces, 0);
	//memcpy (pObjInd, t_pIndeces, sizeOfAllIndeces);
	//lpIndexBuffer->Unlock();

	//memcpy (pObjVert, pVertices, sizeOfAllVerteces);

	bLoaded = true;
	return true;
}

//--------------------------------------------------------------------------------------------
// ���:		 Render()
// ��������: ������������ �������
//--------------------------------------------------------------------------------------------
void Object3D::Render(void)
{
	if(!bLoaded)
		return;
	//renderSystem->GetDevice()->SetVertexShader(D3DFVF_3DPNT1);
	//renderSystem->GetDevice()->SetStreamSource (0, lpVertexBuffer, sizeOfVertex);
	//renderSystem->GetDevice()->SetIndices (lpIndexBuffer, 0);
	//renderSystem->GetDevice()->DrawIndexedPrimitive (D3DPT_TRIANGLELIST, 0, dwNumVerticies, 0, dwNumFaces); 
	//renderSystem->GetDevice()->DrawPrimitive(D3DPT_TRIANGLELIST, 200000, GetTriangleCount());
	renderSystem->DrawIndexedPrimitive(vb,0,dwNumVerticies,ib,0,dwNumFaces);
	//renderSystem->GetDevice()->SetPixelShader(NULL);
	//renderSystem->GetDevice()->SetVertexShader(NULL);
	//lpD3DXMesh->DrawSubset(0);
}

void Object3D::Render(int vfrom, int vto, int ffrom, int fto)
{
	if(!bLoaded)
		return;
	//renderSystem->GetDevice()->SetVertexShader(D3DFVF_3DPNT1);
	//renderSystem->GetDevice()->SetStreamSource (0, lpVertexBuffer, sizeOfVertex);
	//renderSystem->GetDevice()->SetIndices (lpIndexBuffer, 0);
	//renderSystem->GetDevice()->DrawIndexedPrimitive (D3DPT_TRIANGLELIST, 0, dwNumVerticies, 0, dwNumFaces); 
	//renderSystem->GetDevice()->DrawPrimitive(D3DPT_TRIANGLELIST, 200000, GetTriangleCount());
	renderSystem->DrawIndexedPrimitive(vb, vfrom, vto, ib, ffrom, fto);
	//renderSystem->GetDevice()->SetPixelShader(NULL);
	//renderSystem->GetDevice()->SetVertexShader(NULL);
	//lpD3DXMesh->DrawSubset(0);
}

//--------------------------------------------------------------------------------------------
// ���:		 Move()
// ��������: ����������� �������
//--------------------------------------------------------------------------------------------
void Object3D::Move(float x, float y, float z)
{
	D3DXMatrixTranslation (&objMatrTT, x, y, z);
	objMatrT *= objMatrTT;
}

//--------------------------------------------------------------------------------------------
// ���:		 RotateX()
// ��������: �������� ������� ������ ��� X
//--------------------------------------------------------------------------------------------
void Object3D::RotateX(float x)
{
	D3DXMatrixRotationX (&objMatrTT, x);
	objMatrR *= objMatrTT;
}

//--------------------------------------------------------------------------------------------
// ���:		 RotateY()
// ��������: �������� ������� ������ ��� Y
//--------------------------------------------------------------------------------------------
void Object3D::RotateY(float y)
{
	D3DXMatrixRotationY (&objMatrTT, y);
	objMatrR *= objMatrTT;
}

//--------------------------------------------------------------------------------------------
// ���:		 RotateZ()
// ��������: �������� ������� ������ ��� Z
//--------------------------------------------------------------------------------------------
void Object3D::RotateZ(float z)
{
	D3DXMatrixRotationZ (&objMatrTT, z);
	objMatrR *= objMatrTT;
}

//--------------------------------------------------------------------------------------------
// ���:		 Scale()
// ��������: ��������������� �������
//--------------------------------------------------------------------------------------------
void Object3D::Scale(float x, float y, float z)
{
	D3DXMatrixScaling (&objMatrTT, x, y, z);
	objMatrS *= objMatrTT;
}

//--------------------------------------------------------------------------------------------
// ���:		 Transform()
// ��������: ���������� ��������������
//--------------------------------------------------------------------------------------------
void Object3D::Transform()
{
    memcpy (pVertices, pObjVert, sizeOfAllVerteces);

	objMatr = objMatrS*objMatrR*objMatrT;

	for(DWORD i=0; i<dwNumVerticies; i++)
	{
		D3DXVec3TransformCoord	(&pVertices[i].position, &pVertices[i].position, &objMatr);
		D3DXVec3TransformNormal (&pVertices[i].normal,	 &pVertices[i].normal,   &objMatr);
		D3DXVec3Normalize		(&pVertices[i].normal,   &pVertices[i].normal);
	};
}

//--------------------------------------------------------------------------------------------
// ���:		 SetMaterial()
// ��������: ��������� ���������
//--------------------------------------------------------------------------------------------
void Object3D::SetMaterial()
{
	//pMaterial->SetMaterial();
}
