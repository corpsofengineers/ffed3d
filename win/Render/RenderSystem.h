#ifndef _RenderSystem_H_
#define _RenderSystem_H_

#include <d3d9.h>
#include <d3dx9.h>
#include "xmath.h"
#include "VertexFormat.h"
#include <vector>
#define RENDER_WINDOWED 1<<0

#define RENDERSTATE_MAX 210

struct sVertexBuffer
{
	LPDIRECT3DVERTEXBUFFER9 vb;
	IDirect3DVertexDeclaration9* declaration;
	short vertexSize;
};
typedef std::vector< std::pair<IDirect3DVertexDeclaration9**, D3DVERTEXELEMENT9*> > VertexDeclarations;
static VertexDeclarations vertexDeclarations;

struct sIndexBuffer
{
	LPDIRECT3DINDEXBUFFER9 ib;
};

class RenderSystem
{
public:
	RenderSystem();
	~RenderSystem();

	bool Initialize(int xScr,int yScr,int mode,HWND hWnd,int RefreshRateInHz=0);

	HRESULT SetRenderState(D3DRENDERSTATETYPE State,DWORD Value);
	DWORD GetRenderState(D3DRENDERSTATETYPE State);
	void SetVertexDeclaration(IDirect3DVertexDeclaration9* declaration);
	void SetVertexDeclaration(sVertexBuffer& vb);
	IDirect3DVertexDeclaration9* RenderSystem::GetVertexDeclaration();
	void SetIndices(LPDIRECT3DINDEXBUFFER9 indexBuffer);
	void SetIndices(sIndexBuffer& indexBuffer);
	void SetStreamSource(sVertexBuffer& vb);

	void DrawIndexedPrimitive(sVertexBuffer& vb, int offestVertex, int nVertex, sIndexBuffer& ib, int offsetPolygon, int nPolygon);
	
	int BeginScene();
	int EndScene();

	void CreateVertexDeclaration(D3DVERTEXELEMENT9* pVertexElements,IDirect3DVertexDeclaration9** ppDecl);
	void CreateVertexBuffer(sVertexBuffer& vb, int numVertex, IDirect3DVertexDeclaration9* declaration);
	void CreateIndexBuffer(sIndexBuffer& ib,int NumberPolygon);
	int GetSizeFromDeclaration(IDirect3DVertexDeclaration9* declaration);
	static void RegisterVertexDeclaration(IDirect3DVertexDeclaration9*& declaration, D3DVERTEXELEMENT9* elements);

	void* RenderSystem::LockVertexBuffer(sVertexBuffer &vb,int vertexNum=0, int size=0,bool readonly=false);
	void RenderSystem::UnlockVertexBuffer(sVertexBuffer &vb);

	LPDIRECT3DDEVICE9 GetDevice(){return lpD3DDevice;}
protected:
	LPDIRECT3D9					lpD3D;
	LPDIRECT3DDEVICE9			lpD3DDevice;

	DWORD renderStates[RENDERSTATE_MAX];
	IDirect3DVertexDeclaration9* currentDeclaration;
	LPDIRECT3DINDEXBUFFER9 currentIndexBuffer;

	HWND hWnd_;
};

extern class RenderSystem* renderSystem;

#endif //_RenderSystem_H_