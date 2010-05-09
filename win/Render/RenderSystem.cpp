#include "RenderSystem.h"
#include <assert.h>
class RenderSystem* renderSystem;

RenderSystem::RenderSystem()
{
	lpD3D = NULL;
	lpD3DDevice = NULL;
}

RenderSystem::~RenderSystem()
{

}

bool RenderSystem::Initialize(int xScr,int yScr,int mode,HWND hWnd,int RefreshRateInHz/* =0 */)
{
	memset(renderStates,0xEF,sizeof(renderStates));

	hWnd_ = hWnd;

	if(!lpD3D)
		lpD3D=Direct3DCreate9(D3D_SDK_VERSION);
	
	if(!lpD3D)
	{
		MessageBox (NULL, "Failed Direct3DCreate9",
			"WinFFE startup error", MB_OK | MB_ICONWARNING);
		return false;
	}

	D3DDISPLAYMODE d3ddm;
	if(FAILED(lpD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT,&d3ddm)))
	{
		MessageBox (NULL, "Failed GetAdapterDisplayMode",
			"WinFFE startup error", MB_OK | MB_ICONWARNING);
		return false;
	}
	D3DPRESENT_PARAMETERS d3dpp; 
	ZeroMemory(&d3dpp, sizeof(d3dpp));

	if (mode&RENDER_WINDOWED) {
		d3dpp.Windowed = true;
		d3dpp.SwapEffect = D3DSWAPEFFECT_COPY ;
		d3dpp.BackBufferFormat = d3ddm.Format;
	} else {
		d3dpp.Windowed = false;
		d3dpp.SwapEffect = D3DSWAPEFFECT_COPY;
		d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;
		d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
		d3dpp.BackBufferWidth = xScr;
		d3dpp.BackBufferHeight = yScr;
		d3dpp.BackBufferCount = 1;
		d3dpp.FullScreen_RefreshRateInHz = RefreshRateInHz;
		//d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
	}

	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;

	if(FAILED(lpD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
		D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &lpD3DDevice))) 
	{
		if(FAILED(lpD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
			D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &lpD3DDevice)))
		{
			if(FAILED(lpD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_REF, hWnd,
				D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &lpD3DDevice))) {
					MessageBox (NULL, "Failed CreateDevice",
						"WinFFE startup error", MB_OK | MB_ICONWARNING);
					return false;
				}
		}
	}

	for(unsigned int i = 0; i < vertexDeclarations.size(); ++i)
		lpD3DDevice->CreateVertexDeclaration(vertexDeclarations[i].second, vertexDeclarations[i].first);

	return true;
}

int RenderSystem::BeginScene()
{
	HRESULT hr = lpD3DDevice->BeginScene();
	return hr;
}

int RenderSystem::EndScene()
{
	HRESULT hr = lpD3DDevice->EndScene();
	if(hr != D3D_OK)
		return hr;
	hr=lpD3DDevice->Present(NULL,NULL,hWnd_,NULL);
	return hr;
}

HRESULT RenderSystem::SetRenderState(D3DRENDERSTATETYPE State,DWORD Value)
{
	if(renderStates[State]!=Value) 
		return lpD3DDevice->SetRenderState(State,renderStates[State]=Value);
	return D3D_OK;
}

DWORD RenderSystem::GetRenderState(D3DRENDERSTATETYPE State)
{
	return renderStates[State];
}

void RenderSystem::SetVertexDeclaration(IDirect3DVertexDeclaration9* declaration)
{
	if(declaration != currentDeclaration)
	{
		HRESULT hr = lpD3DDevice->SetVertexDeclaration(currentDeclaration = declaration);
		assert(hr==D3D_OK);
	}

}

IDirect3DVertexDeclaration9* RenderSystem::GetVertexDeclaration()
{
	return currentDeclaration;

}

void RenderSystem::SetVertexDeclaration(sVertexBuffer& vb)
{
	SetVertexDeclaration(vb.declaration);
}

inline void RenderSystem::SetIndices(LPDIRECT3DINDEXBUFFER9 indexBuffer)
{
	if(indexBuffer != currentIndexBuffer)
	{
		currentIndexBuffer = indexBuffer;
		HRESULT hr = lpD3DDevice->SetIndices(indexBuffer);
		assert(hr==D3D_OK);

	}
}
inline void RenderSystem::SetIndices(sIndexBuffer& indexBuffer)
{
	SetIndices(indexBuffer.ib);
}

void RenderSystem::SetStreamSource(sVertexBuffer& vb)
{
	HRESULT hr = lpD3DDevice->SetStreamSource(0,vb.vb,0,vb.vertexSize);
	assert(hr==D3D_OK);
}

void RenderSystem::DrawIndexedPrimitive(sVertexBuffer& vb, int offestVertex, int nVertex, sIndexBuffer& ib, int offsetPolygon, int nPolygon)
{
	SetVertexDeclaration(vb);
	SetIndices(ib);
	SetStreamSource(vb);
	HRESULT hr = lpD3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,0,offestVertex,nVertex,3*offsetPolygon,nPolygon);
	assert(hr== D3D_OK);
}

void RenderSystem::CreateVertexDeclaration(D3DVERTEXELEMENT9* pVertexElements,IDirect3DVertexDeclaration9** ppDecl)
{
	HRESULT hr = lpD3DDevice->CreateVertexDeclaration(pVertexElements,ppDecl);
	assert(hr==D3D_OK);
}
int RenderSystem::GetSizeFromDeclaration(IDirect3DVertexDeclaration9* declaration)
{
	if(!declaration)
		return 0;

	D3DVERTEXELEMENT9 elements[256];
	memset(elements, 0, sizeof(elements));
	unsigned int elementsCount = 0;
	declaration->GetDeclaration(elements, &elementsCount);
	return D3DXGetDeclVertexSize(&elements[0],0);
}

void RenderSystem::CreateVertexBuffer(sVertexBuffer& vb, int numVertex, IDirect3DVertexDeclaration9* declaration)
{
	int size = GetSizeFromDeclaration(declaration);
	vb.vertexSize = size;
	vb.declaration = declaration;
	HRESULT hr = lpD3DDevice->CreateVertexBuffer(numVertex*size,0,0,D3DPOOL_MANAGED,&vb.vb,NULL);
	assert(hr==D3D_OK);
}
void RenderSystem::CreateIndexBuffer(sIndexBuffer& ib,int NumberPolygon)
{
	lpD3DDevice->CreateIndexBuffer(NumberPolygon*(3*sizeof(WORD)),D3DUSAGE_WRITEONLY,D3DFMT_INDEX16,D3DPOOL_MANAGED,&ib.ib,NULL);
}

void RenderSystem::RegisterVertexDeclaration(IDirect3DVertexDeclaration9*& declaration, D3DVERTEXELEMENT9* elements)
{
	//int old_size = vertexDeclarations.size();
	vertexDeclarations.push_back(std::make_pair(&declaration, elements));
}
