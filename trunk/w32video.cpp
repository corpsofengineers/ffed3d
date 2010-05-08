#include <windows.h>
#include <ddraw.h>
#include <stdio.h>
#include <d3dx9.h>
#include <sys/stat.h>

#include "../ffeapi.h"
#include "../ffecfg.h"
#include "win32api.h"
#include "ffe3d.h"
#include "XFile/XfileEntity.h"
#include "Macros.h"
#include "XFile/Utility.h"
#include "ini/ini.h"
#include "Render/RenderSystem.h"
#include <assert.h>

D3DXVECTOR3 pl;
float plsize;

static int exclusive = 0;
static int fullscreen = 0;
int aspectfix = 1;
static int wireframe = 0;

static int fswidth = 640;
static int fsheight = 400;
static int fsbpp = 16;

static int winwidth = 640;
static int winheight = 400;

static int vidInit = 0;
static int active = 1;
static int ptrEnabled = 1;
static int error = 0;
static int bpp = 16;

static int wndwidth;		// window width/height including borders
static int wndheight;
int curwidth;		// width/height of front surface
int curheight;

extern int control_dist;
extern int max_divide_deep;
extern int smart_optimizer;
extern int smart_optimizer_c;

static int modelexport = 0;

static RECT clrect;
static RECT wndrect;
static POINT cursorpos;

HWND hWnd = NULL;
static HINSTANCE hInst = NULL;

ULONG pWinPal16[256];
ULONG pWinPal15[256];
ULONG pWinPal32[256];

LPD3DXSPRITE textSprite;
LPDIRECT3D9 D3D = NULL;
//LPDIRECT3DDEVICE9 renderSystem->GetDevice() = NULL;
//LPDIRECT3DVERTEXBUFFER9 d3d_vb;
sVertexBuffer vertexBuffer;
sVertexBuffer spriteVB;
LPDIRECT3DTEXTURE9 textures[1000];
LPDIRECT3DTEXTURE9 skyboxtex[6];
D3DMATERIAL9 m_matMaterial;
D3DLIGHT9 d3dLight;
LPD3DXFONT m_pFont;

VERTEXTYPE vertexType[MAXVERT];
int vertexNum=0;
MODEL modelList[6000];
int modelNum=0;
FFTEXT ffText[2000];
int textNum=0;
FFEsprite spriteList[400];
int maxsprite;

//Set up the time stuff
double starttime;
LARGE_INTEGER nowtime;
LONGLONG ticks;

LARGE_INTEGER time;

bool InitD3D(HWND hWnd);
void Render();
void setupLight();
void CreateLocalFont();
void ViewPort();
void loadTextures();
void checkExport();

static FILE *pLog = NULL;

extern "C" int asmmain (int argc, char **argv);

extern "C" char *DATA_008804;
extern "C" char *DATA_008861;
extern "C" char *DATA_008872;
extern "C" char *DATA_008870;
extern "C" char *DATA_008835;
extern "C" char *DATA_008809;
extern "C" char *DATA_008810;
extern "C" char *DATA_009054;
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow)
{
	hInst = hInstance;

	LARGE_INTEGER time;
	QueryPerformanceCounter(&time);
	starttime = (double)time.QuadPart;

	QueryPerformanceFrequency(&time);
	ticks = time.QuadPart;

	return asmmain(0, NULL);
}

void Win32MsgHandler (void)
{
	while (1) 
	{	
		MSG msg;
		if (PeekMessage (&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT) {
				SystemCleanup();
				exit(0);
			}
			TranslateMessage (&msg);
			DispatchMessage (&msg);
		}
		else if (active) return;
		else WaitMessage();
	}
}

HWND Win32GetWnd()
{
	return hWnd;
}

HINSTANCE Win32GetInst()
{
	return hInst;
}

extern "C" void InputMouseReadPos (long *pXPos, long *pYPos)
{
	POINT p;
	int x,y;

	if (curwidth && curheight) {
		GetCursorPos (&p);
		ScreenToClient (Win32GetWnd(), &p);
		x = p.x;
		y = p.y;
		if (aspectfix) {
			y-=(curheight-(int)(curwidth/1.6f))/2;
			//y=(int)(y*1.2f);			
		}
		/*
		if (aspectfix) {
			viewport.X = 0;
			viewport.Y = (long)(curheight-curwidth/1.6f)/2;
			viewport.Width = curwidth;
			viewport.Height = (long)(curwidth/1.6f); 
		} else {
			viewport.X = 0;
			viewport.Y = 0;
			viewport.Width = curwidth;
			viewport.Height = curheight; 
		}
		*/
 		*pXPos = (x * 320) / curwidth;
		if (aspectfix) {
			*pYPos = (y * 200) / (int)(curwidth/1.6f);
		} else {
			*pYPos = (y * 200) / curheight;
		}
	}
}

static void ConvPalette (PALETTEENTRY *pPal)
{
	int i;
	for (i=0; i<256; i++) {
		pPal[i].peRed = (UCHAR)(pWinPal32[i] >> 16 & 0xfc);
		pPal[i].peGreen = (UCHAR)(pWinPal32[i] >> 8 & 0xfc);
		pPal[i].peBlue = (UCHAR)(pWinPal32[i] & 0xfc);
	}
}

char *ppFSError[] = {
	{ "Unknown error\n" },
	{ "Failed on SetCooperativeLevel\n" },
	{ "Failed on SetDisplayMode\n" },
	{ "Failed on primary surface creation\n" },
	{ "Failed on backbuffer selection\n" },
	{ "Failed on intermediate surface creation\n" },
	{ "Failed on palette creation\n" },
	{ "Failed on palette attachment\n" }
	};


static void InitFullscreen (void)
{
	exclusive = 1;
	curwidth = fswidth; curheight = fsheight;
}

char *ppWinError[] = {
	{ "Unknown error\n" },
	{ "Failed on SetCooperativeLevel\n" },
	{ "Failed on primary surface creation\n" },
	{ "Failed on clipper creation\n" },
	{ "Failed on clipper window setting\n" },
	{ "Failed on clipper attachment\n" },
	{ "Failed on backbuffer creation\n" },
	{ "Failed on intermediate surface creation\n" },
	{ "Failed to find usable bit depth\n" },
	};


static void InitWindowed (void)
{
	curwidth = winwidth; curheight = winheight;
}

static void ResetWindowParams()
{
	POINT pos = { 0, 0 };
	WINDOWPLACEMENT wp;

	if (exclusive) return;
	wp.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement (hWnd, &wp);
	if (wp.showCmd == SW_SHOWMINIMIZED) return;

	GetClientRect (hWnd, &clrect);
	ClientToScreen (hWnd, &pos);
	OffsetRect (&clrect, pos.x, pos.y);

	wndrect.left = clrect.left - GetSystemMetrics(SM_CXSIZEFRAME);
	wndrect.top = clrect.top - GetSystemMetrics(SM_CYSIZEFRAME)
		- GetSystemMetrics(SM_CYMENU);
	wndrect.right = clrect.right + GetSystemMetrics(SM_CXSIZEFRAME);
	wndrect.bottom = clrect.bottom + GetSystemMetrics(SM_CYSIZEFRAME);
}

static void VideoReset (void)
{
	Win32SetMouseNorm();

	if (fullscreen) 
		InitFullscreen();
	else 
		InitWindowed();

	if (InitD3D(hWnd)==false) {
		PostQuitMessage(0);
		return;
	}

}

static void RestoreSurfaces()
{
}

static LRESULT CALLBACK WndProc (HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch(msg)
	{
		case WM_LBUTTONDOWN:
		case WM_ACTIVATEAPP:
		case WM_ACTIVATE:
			Win32InputReacquire();
			Win32SoundReacquire();
			break;

		case WM_SIZE:
			if (SIZE_MAXHIDE==wparam || SIZE_MINIMIZED==wparam) {
				active = 0;
				TimerCleanup();
				Win32SetMouseNorm();
				ResetWindowParams();
			}
			else {
				active = 1;
				TimerInit();
			}
			break;

		case WM_CLOSE:
			PostQuitMessage(0);
			return 0L;

		case WM_DESTROY:
			return 0L;

		case WM_GETMINMAXINFO:
			((MINMAXINFO*)lparam)->ptMinTrackSize.x = wndwidth;
			((MINMAXINFO*)lparam)->ptMinTrackSize.y = wndheight;
			((MINMAXINFO*)lparam)->ptMaxTrackSize.x = wndwidth;
			((MINMAXINFO*)lparam)->ptMaxTrackSize.y = wndheight;
			break;

		case WM_MOVE:
			ResetWindowParams ();
			break;

		case WM_KEYDOWN:
			if (GetAsyncKeyState(VK_CONTROL) & 0x8000) {
				if (wparam == VK_F9) {
					modelexport = 1;
					checkExport();
					return 0L;
				}
				if (wparam == VK_F10) {
					loadTextures();
					return 0L;
				}
				if (wparam == VK_F11) {
					wireframe ^= 1;
					return 0L;
				}
				//if (wparam == VK_F12) {
				//	fullscreen ^= 1;
				//	VideoReset();
				//	return 0L;
				//}
				if (wparam == VK_HOME){
					UpVolume();
					return 0L;
				}
				if (wparam == VK_END){
					DownVolume();
					return 0L;
				}
			} else {
				if (wparam == VK_DOWN){
					pl.y-=0.01;
				}
				if (wparam == VK_UP){
					pl.y+=0.01;
				}
				if (wparam == VK_LEFT){
					pl.x-=0.01;
				}
				if (wparam == VK_RIGHT){
					pl.x+=0.01;
				}
				if (wparam == VK_HOME){
					pl.z+=0.01;
				}
				if (wparam == VK_END){
					pl.z-=0.01;
				}
				if (wparam == VK_INSERT){
					plsize+=0.01;
				}
				if (wparam == VK_DELETE){
					plsize-=0.01;
				}

			}
		case WM_SYSKEYDOWN:
			return 0L;
	}
	return DefWindowProc (wnd, msg, wparam, lparam);
}


static UCHAR pPalTable[3][64];

void VideoInit()
{
	CfgStruct cfg;
	int pPalBase[3] = { 0, 0, 0 };
	int step, i, j, col;

	if (vidInit != 0) return;

	memset (pWinPal15, 0, 256*4);
	memset (pWinPal16, 0, 256*4);
	memset (pWinPal32, 0, 256*4);

	CfgOpen (&cfg, __CONFIGFILE__);
	CfgFindSection (&cfg, "VIDEO");
	CfgGetKeyVal (&cfg, "fullscreen", &fullscreen);
	CfgGetKeyVal (&cfg, "aspectfix", &aspectfix);
	CfgGetKeyVal (&cfg, "wireframe", &wireframe);
	CfgGetKeyVal (&cfg, "modelexport", &modelexport);

	CfgGetKeyVal (&cfg, "fswidth", &fswidth);
	CfgGetKeyVal (&cfg, "fsheight", &fsheight);
	CfgGetKeyVal (&cfg, "fsbpp", &fsbpp);

	CfgGetKeyVal (&cfg, "winwidth", &winwidth);
	CfgGetKeyVal (&cfg, "winheight", &winheight);

	CfgGetKeyVal (&cfg, "RedBase", pPalBase);
	CfgGetKeyVal (&cfg, "GreenBase", pPalBase+1);
	CfgGetKeyVal (&cfg, "BlueBase", pPalBase+2);

	CfgGetKeyVal (&cfg, "control_dist", &control_dist);
	CfgGetKeyVal (&cfg, "max_divide_deep", &max_divide_deep);
	CfgGetKeyVal (&cfg, "smart_optimizer", &smart_optimizer);
	CfgGetKeyVal (&cfg, "smart_optimizer_c", &smart_optimizer_c);

	CfgClose (&cfg);

	for (i=0; i<3; i++) 
	{
		col = pPalBase[i] << 8;
		step = ((64<<8) - col) / 63;

		for (j=0; j<64; j++) {
			int tcol = col >> 8;
			if (tcol < 0) tcol = 0;
			pPalTable[i][j] = (UCHAR)tcol;
			col += step;
		}
	}


    WNDCLASSEX wc = {sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, 
                     hInst, NULL, LoadCursor (NULL, IDC_ARROW), NULL, NULL,
                     "FFED3D", NULL};

	
    int r=RegisterClassEx(&wc);

	if (winwidth < 160) winwidth = 160;
	if (winheight < 100) winheight = 100;
	wndwidth = winwidth + GetSystemMetrics(SM_CXSIZEFRAME)*2;
	wndheight = winheight + GetSystemMetrics(SM_CYSIZEFRAME)*2
		+ GetSystemMetrics(SM_CYMENU);

    hWnd = CreateWindow("FFED3D", "FFED3D", 
                              WS_OVERLAPPEDWINDOW, 100, 100, wndwidth, wndheight,
                              NULL, NULL, hInst, NULL);

	ResetWindowParams();

	VideoReset();

    ShowWindow(hWnd, SW_SHOWNORMAL);
    UpdateWindow(hWnd);

	D3DXCreateSprite(renderSystem->GetDevice(),&textSprite);
    
	vidInit = 1;
}

void VideoCleanup()
{
    if(renderSystem->GetDevice() != NULL)
    {
        renderSystem->GetDevice()->Release();
        //renderSystem->GetDevice() = NULL;
    }

    if(D3D != NULL)
    {
        D3D->Release();
        D3D = NULL;
    }

	if (hWnd) { DestroyWindow (hWnd); hWnd = NULL; }
}

extern "C" void VideoPointerEnable()
{
	if (ptrEnabled == 0) {
		SetCursorPos (cursorpos.x, cursorpos.y);
		ClipCursor (NULL);
		ShowCursor (TRUE);
		ptrEnabled = 1;
	}
}

extern "C" void VideoPointerDisable()
{
	if (ptrEnabled != 0) {
		GetCursorPos (&cursorpos);
		ShowCursor (FALSE);
		ClipCursor (&clrect);
		ptrEnabled = 0;
	}
}

extern "C" long VideoPointerExclusive()
{
	return 0;
	return exclusive;
}

extern "C" void VideoBlit (UCHAR *pData, long x, long y, long w, long h, long jump)
{
	Render();
}

extern "C" void VideoMaskedBlit (UCHAR *pData, long x, long y,
	long w, long h, long jump)
{
}

extern "C" void VideoReverseBlit (UCHAR *pData, long x, long y,
	long w, long h, long jump)
{
}

extern "C" void VideoGetPalValue (long palindex, UCHAR *pVal)
{
	ULONG col = pWinPal32[palindex];
	pVal[0] = (UCHAR)(col >> 18 & 0x3f);
	pVal[1] = (UCHAR)(col >> 10 & 0x3f);
	pVal[2] = (UCHAR)(col >> 2 & 0x3f);
}

extern "C" void VideoSetPalValue (long palindex, UCHAR *pVal)
{
	UCHAR pCol[4];
	pCol[0] = pPalTable[0][pVal[0]];
	pCol[1] = pPalTable[1][pVal[1]];
	pCol[2] = pPalTable[2][pVal[2]];

	if (bpp == 8) {
		PALETTEENTRY pe;
		pe.peRed = pCol[0] << 2;
		pe.peGreen = pCol[1] << 2;
		pe.peBlue = pCol[2] << 2;
	}

	pWinPal15[palindex] = (ULONG)((pCol[0] << 9 & 0x7c00)
		+ (pCol[1] << 4 & 0x03e0) + (pCol[2] >> 1));
	pWinPal16[palindex] = (ULONG)((pCol[0] << 10 & 0xf800)
		+ (pCol[1] << 5 & 0x07e0) + (pCol[2] >> 1));
	pWinPal32[palindex] = (ULONG)((pCol[0] << 18 & 0xfc0000)
		+ (pCol[1] << 10 & 0xfc00) + (pCol[2] << 2));
	pWinPal15[palindex] |= pWinPal15[palindex] << 16;
	pWinPal16[palindex] |= pWinPal16[palindex] << 16;
}

bool FFED3DCreateTextureFromFile(LPDIRECT3DDEVICE9 pDevice, LPCTSTR buf, LPDIRECT3DTEXTURE9 *texture) {

	return FAILED(D3DXCreateTextureFromFileEx(
			pDevice,					// pDevice
			buf,						// pSrcFile
			D3DX_DEFAULT,				// Width
			D3DX_DEFAULT,				// Height
			D3DX_DEFAULT,				// MipLevels
			0,							// Usage
			D3DFMT_UNKNOWN,				// Format
            D3DPOOL_MANAGED,			// Pool
			D3DX_FILTER_POINT,			// Filter
			D3DX_DEFAULT,				// MipFilter
			0,							// ColorKey
			NULL,						// pSrcInfo
			NULL,						// pPalette
			texture						// ppTexture
		));
	//D3DX_FILTER_POINT
}

bool FFED3DCreateTextureFromFileRT(LPDIRECT3DDEVICE9 pDevice, LPCTSTR buf, LPDIRECT3DTEXTURE9 *texture) {

	return FAILED(D3DXCreateTextureFromFileEx(
			pDevice,					// pDevice
			buf,						// pSrcFile
			D3DX_DEFAULT,				// Width
			D3DX_DEFAULT,				// Height
			1,							// MipLevels
			D3DUSAGE_RENDERTARGET,		// Usage
			D3DFMT_UNKNOWN,				// Format
            D3DPOOL_DEFAULT,			// Pool
			D3DX_FILTER_POINT,			// Filter
			D3DX_DEFAULT,				// MipFilter
			0,							// ColorKey
			NULL,						// pSrcInfo
			NULL,						// pPalette
			texture						// ppTexture
		));
	//D3DX_FILTER_POINT
}

//CMD2Model g_model[500];
//Object3D objectList[500];
//Object3D splineList[500];
CXFileEntity* panelObj;
CXFileEntity* sphereObj;
CXFileEntity* atmoObj;
LPDIRECT3DTEXTURE9 panelTex[5];
LPDIRECT3DTEXTURE9 panelIcons[30];
LPDIRECT3DSURFACE9 texSurface, oldSurface;
int panelnum=0;
CXFileEntity* objectList[500];
CXFileEntity* splineList[500];

LPDIRECT3DTEXTURE9 g_texture[500];
MODELCONFIG modelconfig[500];

extern bool exportb[500];

void loadTextures() {
	char buf[1000];

	sprintf_s(buf,"textures/panelicons/1_f13.png");
	if (FAILED(FFED3DCreateTextureFromFile(renderSystem->GetDevice(), buf, &panelIcons[0])))
		panelIcons[0]=NULL;

	for (int i=0;i<1000;i++) {
		sprintf_s(buf,"textures/tex%i.png",i);
		if (FAILED(FFED3DCreateTextureFromFile(renderSystem->GetDevice(), buf, &textures[i]))) {
			sprintf_s(buf,"textures/tex%i.tga",i);
			if (FAILED(D3DXCreateTextureFromFile(renderSystem->GetDevice(), buf, &textures[i]))) {
				textures[i]=NULL;
			}
		}
	}
	for (int i=0;i<6;i++) {
		sprintf_s(buf,"textures/skybox%i.tga",i);
		if (FAILED(D3DXCreateTextureFromFile(renderSystem->GetDevice(), buf, &skyboxtex[i]))) {
			skyboxtex[i]=NULL;
		}
	}

	sprintf_s(buf,"models\\panel.png");
	if (FAILED(FFED3DCreateTextureFromFileRT(renderSystem->GetDevice(), buf, &panelTex[0]))) {
		panelTex[0]=NULL;
	}	
	sprintf_s(buf,"models\\panelF2.png");
	if (FAILED(D3DXCreateTextureFromFile(renderSystem->GetDevice(), buf, &panelTex[1]))) {
		panelTex[1]=NULL;
	}	
	sprintf_s(buf,"models\\panelF3.png");
	if (FAILED(D3DXCreateTextureFromFile(renderSystem->GetDevice(), buf, &panelTex[2]))) {
		panelTex[2]=NULL;
	}	
	sprintf_s(buf,"models\\panelF4.png");
	if (FAILED(D3DXCreateTextureFromFile(renderSystem->GetDevice(), buf, &panelTex[3]))) {
		panelTex[3]=NULL;
	}	

	for(int i=3;i<500;i++) {
		if (objectList[i]) {
			if (i==38) {
				//objectList[i]->ExportX(i);
			}
			sprintf_s(buf,"models\\%i\\skin.png",i);
			if (FAILED(D3DXCreateTextureFromFile(renderSystem->GetDevice(), buf, &g_texture[i]))) {
				sprintf_s(buf,"models\\%i\\skin.tga",i);
				if (FAILED(D3DXCreateTextureFromFile(renderSystem->GetDevice(), buf, &g_texture[i]))) {
					g_texture[i]=NULL;
				}
			}
		}
	}
}



void checkExport() {
	char buf[200];
	struct stat st;
	int e;

	for(int i=3;i<500;i++) {
		if (modelexport==0) {
			exportb[i]=false;
			continue;
		}

		sprintf_s(buf,"models\\_export\\%i\\export.x",i);
		e = stat (buf, &st);
		if (e==0 || e==0xffffffff) {
			exportb[i]=true;
		}
	}
}

void loadDirectXModel() {
	char buf[1000];
	CIniFile file;

	panelObj = new CXFileEntity(renderSystem->GetDevice());
	sprintf_s(buf,"/models/panel.x");
	if (!panelObj->LoadXFile(CUtility::GetTheCurrentDirectory()+buf,0))
		SAFE_DELETE(panelObj);

	sphereObj = new CXFileEntity(renderSystem->GetDevice());
	sprintf_s(buf,"/models/sphere.x");
	if (!sphereObj->LoadXFile(CUtility::GetTheCurrentDirectory()+buf,0))
		SAFE_DELETE(sphereObj);

	atmoObj = new CXFileEntity(renderSystem->GetDevice());
	sprintf_s(buf,"/models/atmo.x");
	if (!atmoObj->LoadXFile(CUtility::GetTheCurrentDirectory()+buf,0))
		SAFE_DELETE(atmoObj);

	for(int i=3;i<500;i++) {
		sprintf_s(buf,"/models/%i/spline.x",i);
		//splineList[i].Create(buf);
		//make new
		splineList[i] = new CXFileEntity(renderSystem->GetDevice());
		//Load the new
		if (!splineList[i]->LoadXFile(CUtility::GetTheCurrentDirectory()+buf,0))
			SAFE_DELETE(splineList[i]);

		sprintf_s(buf,"/models/%i/model.x",i);
		objectList[i] = new CXFileEntity(renderSystem->GetDevice());
		if (objectList[i]->LoadXFile(CUtility::GetTheCurrentDirectory()+buf,0)) {
			if (i==38) {
				//objectList[i]->ExportX(i);
			}

			sprintf_s(buf,"models\\%i\\tris.ini",i);
			file.SetFileName (buf);
			file.GetFloatValue (modelconfig[i].scale,"MODEL", "scale");
			file.GetFloatValue (modelconfig[i].scale_x,"MODEL", "scale_x");
			file.GetFloatValue (modelconfig[i].scale_y,"MODEL", "scale_y");
			file.GetFloatValue (modelconfig[i].scale_z,"MODEL", "scale_z");
			modelconfig[i].scale = modelconfig[i].scale > 0 ? modelconfig[i].scale : 1.0f;
			modelconfig[i].scale_x = modelconfig[i].scale_x > 0 ? modelconfig[i].scale_x : 1.0f;
			modelconfig[i].scale_y = modelconfig[i].scale_y > 0 ? modelconfig[i].scale_y : 1.0f;
			modelconfig[i].scale_z = modelconfig[i].scale_z > 0 ? modelconfig[i].scale_z : 1.0f;
			file.GetFloatValue (modelconfig[i].offset_x,"MODEL", "offset_x");
			file.GetFloatValue (modelconfig[i].offset_y,"MODEL", "offset_y");
			file.GetFloatValue (modelconfig[i].offset_z,"MODEL", "offset_z");
			file.GetIntValue (modelconfig[i].cullmode,"MODEL", "cullmode");

			file.GetFloatValue (modelconfig[i].missile_scale,"MODEL", "missile_scale");

			file.GetFloatValue (modelconfig[i].missile[0].x,"MISSILE0", "x");
			file.GetFloatValue (modelconfig[i].missile[0].y,"MISSILE0", "y");
			file.GetFloatValue (modelconfig[i].missile[0].z,"MISSILE0", "z");
			file.GetFloatValue (modelconfig[i].missile[1].x,"MISSILE1", "x");
			file.GetFloatValue (modelconfig[i].missile[1].y,"MISSILE1", "y");
			file.GetFloatValue (modelconfig[i].missile[1].z,"MISSILE1", "z");
			file.GetFloatValue (modelconfig[i].missile[2].x,"MISSILE2", "x");
			file.GetFloatValue (modelconfig[i].missile[2].y,"MISSILE2", "y");
			file.GetFloatValue (modelconfig[i].missile[2].z,"MISSILE2", "z");
			file.GetFloatValue (modelconfig[i].missile[3].x,"MISSILE3", "x");
			file.GetFloatValue (modelconfig[i].missile[3].y,"MISSILE3", "y");
			file.GetFloatValue (modelconfig[i].missile[3].z,"MISSILE3", "z");
			file.GetFloatValue (modelconfig[i].missile[4].x,"MISSILE4", "x");
			file.GetFloatValue (modelconfig[i].missile[4].y,"MISSILE4", "y");
			file.GetFloatValue (modelconfig[i].missile[4].z,"MISSILE4", "z");
			file.GetFloatValue (modelconfig[i].missile[5].x,"MISSILE5", "x");
			file.GetFloatValue (modelconfig[i].missile[5].y,"MISSILE5", "y");
			file.GetFloatValue (modelconfig[i].missile[5].z,"MISSILE5", "z");
			file.GetFloatValue (modelconfig[i].missile[6].x,"MISSILE6", "x");
			file.GetFloatValue (modelconfig[i].missile[6].y,"MISSILE6", "y");
			file.GetFloatValue (modelconfig[i].missile[6].z,"MISSILE6", "z");
			file.GetFloatValue (modelconfig[i].missile[7].x,"MISSILE7", "x");
			file.GetFloatValue (modelconfig[i].missile[7].y,"MISSILE7", "y");
			file.GetFloatValue (modelconfig[i].missile[7].z,"MISSILE7", "z");
			file.GetFloatValue (modelconfig[i].missile[8].x,"MISSILE8", "x");
			file.GetFloatValue (modelconfig[i].missile[8].y,"MISSILE8", "y");
			file.GetFloatValue (modelconfig[i].missile[8].z,"MISSILE8", "z");
			file.GetFloatValue (modelconfig[i].missile[9].x,"MISSILE9", "x");
			file.GetFloatValue (modelconfig[i].missile[9].y,"MISSILE9", "y");
			file.GetFloatValue (modelconfig[i].missile[9].z,"MISSILE9", "z");

		} else {
			SAFE_DELETE(objectList[i]);
		}
	}
	
}

LPDIRECT3DVERTEXBUFFER9 vb;

bool InitD3D(HWND hWnd)
{
	renderSystem = new RenderSystem();
	
	renderSystem->Initialize(curwidth,curheight,fullscreen?0:RENDER_WINDOWED,hWnd);
	renderSystem->CreateVertexBuffer(vertexBuffer,MAXVERT,VertexXYZNDT1::declaration);
	renderSystem->CreateVertexBuffer(spriteVB,4,VertexXYZWDT1::declaration);

	CreateLocalFont();
	loadDirectXModel();
	loadTextures();
	checkExport();

	if ((curheight-curwidth/1.6f)==0)
		aspectfix=0;

	if (renderSystem->GetDevice()!=NULL)
		renderSystem->GetDevice()->Clear(0,NULL,D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,D3DCOLOR_XRGB(0,0,0),1.0f,0);	

	pl.x = -1.35f;
	pl.y = -0.64f;
	pl.z = 3.91f;
	plsize = 0.18f;

    return true;
}

void ViewPort(bool enable) {
	D3DVIEWPORT9 viewport;

	if (aspectfix) {
		viewport.X = 0;
		viewport.Y = (long)(curheight-curwidth/1.6f)/2;
		viewport.Width = curwidth;
		viewport.Height = (long)(curwidth/1.6f); 
	} else {
		viewport.X = 0;
		viewport.Y = 0;
		viewport.Width = curwidth;
		viewport.Height = curheight; 
	}

	viewport.MinZ = 0; 
	viewport.MaxZ = 1; 

	if (enable)
		viewport.Height = (long)(viewport.Height*0.795f); 

	renderSystem->GetDevice()->SetViewport( &viewport ); 
}


void CreateLocalFont() {
    HFONT hFont;

    int nWeight = FW_NORMAL;
	int nHeight = 16*curwidth/640;
    DWORD dwItalic = 0;
    DWORD dwUnderlined = 0;

    nWeight = FW_BOLD;
    //dwItalic = 1;
    //dwUnderlined = 1;

    hFont = CreateFont(nHeight, 0, 0, 0, nWeight, dwItalic, dwUnderlined, 
		0, ANSI_CHARSET, 0, 0, 0, 0, "Arial");
    
    D3DXCreateFont(renderSystem->GetDevice(), nHeight,0,nWeight,1,false,ANSI_CHARSET,0,0,0,"Arial", &m_pFont);

}

void DrawText(LPSTR pText, int x, int y, D3DCOLOR rgbFontColour)
{
	RECT Rect;

    Rect.left = x;
    Rect.top = y;
    Rect.right = 0;
    Rect.bottom = 0;

    //m_pFont->Begin();
    m_pFont->DrawTextA(textSprite,pText, -1, &Rect, DT_CALCRECT, 0); //Calculate the size of the rect needed
    m_pFont->DrawTextA(textSprite,pText, -1, &Rect, 0, rgbFontColour); //Draw the text
    //m_pFont->End();
}

D3DCOLORVALUE rgbaDiffuse0 = {0.0f, 0.0f, 0.0f, 0.0f,};
D3DCOLORVALUE rgbaDiffuse1 = {1.0f, 1.0f, 1.0f, 0.5f,};
D3DCOLORVALUE rgbaDiffuse2 = {0.75f, 0.75f, 0.75f, 0.5f,};
D3DCOLORVALUE rgbaDiffuse3 = {0.5f, 0.5f, 0.5f, 0.0f,};
D3DCOLORVALUE rgbaDiffuse4 = {0.25f, 0.25f, 0.25f, 0.25f,};
D3DCOLORVALUE rgbaDiffuse11 = {2.0f, 2.0f, 2.0f, 2.0f,};
D3DCOLORVALUE rgbaAmbient0 = {0.0f, 0.0f, 0.0f, 0.0f,};
D3DCOLORVALUE rgbaAmbient1 = {1.0f, 1.0f, 1.0f, 5.0f,};
D3DCOLORVALUE rgbaAmbient2 = {0.5f, 0.5f, 0.5f, 5.0f,};
D3DCOLORVALUE rgbaAmbient3 = {0.25f, 0.25f, 0.25f, 5.0f,};
D3DCOLORVALUE rgbaSpecular0 = {0.0f, 0.0f, 0.0f, 0.0f,};
D3DCOLORVALUE rgbaSpecular1 = {0.774597f, 0.774597f, 0.774597f, 5.0f,};
D3DCOLORVALUE rgbaSpecular2 = {1.0f, 1.0f, 1.0f, 0.0f,};
D3DCOLORVALUE rgbaSpecular3 = {0.5f, 0.5f, 0.5f, 0.0f,};
D3DCOLORVALUE rgbaEmissive0 = {0.0f, 0.0f, 0.0f, 0.0f,};
D3DCOLORVALUE rgbaEmissive1 = {1.0f, 1.0f, 1.0f, 0.0f,};

extern char ambientR;
extern char ambientG;
extern char ambientB;

void selectMaterial(int type, unsigned char cR, unsigned char cG, unsigned char cB) {
//D3DCOLORVALUE rgbaDiffuse;
D3DCOLORVALUE rgbaEmissive;
D3DCOLORVALUE rgbaDiffuse;
int c;

renderSystem->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
renderSystem->SetRenderState(D3DRS_SPECULARENABLE, true);
renderSystem->SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_MATERIAL);
renderSystem->SetRenderState(D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_COLOR1);
renderSystem->SetRenderState (D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
renderSystem->SetRenderState (D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA);

	m_matMaterial.Power = 30;
	switch(type) {
	case SUN:
		renderSystem->SetRenderState( D3DRS_ALPHABLENDENABLE, false);
		rgbaEmissive.r=1.0f/250*cR;
		rgbaEmissive.g=1.0f/250*cG;
		rgbaEmissive.b=1.0f/250*cB;
		rgbaEmissive.a=0.0f;
		//renderSystem->SetRenderState(D3DRS_AMBIENT, Color4c(128, 128, 128).RGBA());
		m_matMaterial.Diffuse = rgbaDiffuse0; 
		m_matMaterial.Ambient = rgbaAmbient1; 
		m_matMaterial.Specular = rgbaSpecular0; 
	    m_matMaterial.Emissive = rgbaEmissive0;
		break;
	case PLANET:
		renderSystem->SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1);
		renderSystem->SetRenderState(D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_MATERIAL);

		renderSystem->SetRenderState(D3DRS_SPECULARENABLE, false);
		renderSystem->SetRenderState( D3DRS_ALPHABLENDENABLE, false);
		c=(cR+cG+cB)/3;
		
		renderSystem->SetRenderState(D3DRS_AMBIENT, D3DCOLOR_XRGB(32, 32, 32));
		rgbaDiffuse.r=1.0f/255*c;
		rgbaDiffuse.g=1.0f/255*c;
		rgbaDiffuse.b=1.0f/255*c;
		rgbaDiffuse.a=0.0f;
		
		/*
		renderSystem->SetRenderState(D3DRS_AMBIENT, D3DCOLOR_XRGB(c/6, c/6, c/6));
		rgbaDiffuse.r=1.0f/255*c*0.75;
		rgbaDiffuse.g=1.0f/255*c*0.75;
		rgbaDiffuse.b=1.0f/255*c*0.75;
		rgbaDiffuse.a=0.0f;
		*/
		m_matMaterial.Diffuse = rgbaDiffuse0; 
		m_matMaterial.Ambient = rgbaAmbient1; 
		m_matMaterial.Specular = rgbaSpecular1; 
	    m_matMaterial.Emissive = rgbaEmissive0;
		break;
	case GALAXY:
		renderSystem->SetRenderState( D3DRS_ALPHABLENDENABLE, true);
		renderSystem->SetRenderState(D3DRS_AMBIENT, D3DCOLOR_XRGB(255, 255, 255));
		m_matMaterial.Diffuse = rgbaDiffuse0; 
		m_matMaterial.Ambient = rgbaAmbient1; 
		m_matMaterial.Specular = rgbaSpecular0; 
	    m_matMaterial.Emissive = rgbaEmissive0;
		break;
	case ATMO:
		renderSystem->SetRenderState( D3DRS_ALPHABLENDENABLE, true);
		renderSystem->SetRenderState(D3DRS_AMBIENT, D3DCOLOR_XRGB(0, 0, 0));
		//renderSystem->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
		//renderSystem->SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1);
		renderSystem->SetRenderState(D3DRS_SPECULARENABLE, false);
		//renderSystem->SetRenderState (D3DRS_SRCBLEND, D3DBLEND_ONE);
		//renderSystem->SetRenderState (D3DRS_DESTBLEND,D3DBLEND_SRCALPHA);
		m_matMaterial.Diffuse = rgbaDiffuse11; 
		m_matMaterial.Ambient = rgbaAmbient0; 
		m_matMaterial.Specular = rgbaSpecular0; 
	    m_matMaterial.Emissive = rgbaEmissive0;
		break;
	case ATMO2:
		renderSystem->SetRenderState( D3DRS_ALPHABLENDENABLE, false);
		renderSystem->SetRenderState(D3DRS_AMBIENT, D3DCOLOR_XRGB(0, 0, 0));
		//renderSystem->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
		//renderSystem->SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1);
		renderSystem->SetRenderState(D3DRS_SPECULARENABLE, false);
		renderSystem->SetRenderState (D3DRS_SRCBLEND, D3DBLEND_ONE);
		renderSystem->SetRenderState (D3DRS_DESTBLEND,D3DBLEND_SRCALPHA);
		m_matMaterial.Diffuse = rgbaDiffuse3; 
		m_matMaterial.Ambient = rgbaAmbient0; 
		m_matMaterial.Specular = rgbaSpecular0; 
	    m_matMaterial.Emissive = rgbaEmissive0;
		break;
	case TRANSP:
		//renderSystem->SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_MATERIAL);
		//renderSystem->SetRenderState(D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_COLOR1);
		renderSystem->SetRenderState( D3DRS_ALPHABLENDENABLE, true);
		renderSystem->SetRenderState(D3DRS_AMBIENT, D3DCOLOR_XRGB(64, 64, 64));
		//renderSystem->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
		//renderSystem->SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1);
		renderSystem->SetRenderState(D3DRS_SPECULARENABLE, false);
		//renderSystem->SetRenderState (D3DRS_SRCBLEND, D3DBLEND_SRCCOLOR  );
		//renderSystem->SetRenderState (D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA );
		m_matMaterial.Diffuse = rgbaDiffuse0; 
		m_matMaterial.Ambient = rgbaAmbient1; 
		m_matMaterial.Specular = rgbaSpecular0; 
	    m_matMaterial.Emissive = rgbaEmissive0;
		break;
	case ATMOSTAR:
		renderSystem->SetRenderState( D3DRS_ALPHABLENDENABLE, true);
		renderSystem->SetRenderState(D3DRS_SPECULARENABLE, false);
		renderSystem->SetRenderState (D3DRS_SRCBLEND, D3DBLEND_ONE);
		renderSystem->SetRenderState (D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA);
		renderSystem->SetRenderState(D3DRS_AMBIENT, D3DCOLOR_XRGB(128, 128, 128));
		rgbaEmissive.r=1.0f/255*cR/10;
		rgbaEmissive.g=1.0f/255*cG/10;
		rgbaEmissive.b=1.0f/255*cB/10;
		rgbaEmissive.a=0.0f;
		m_matMaterial.Diffuse = rgbaDiffuse0; 
		m_matMaterial.Ambient = rgbaAmbient0; 
		m_matMaterial.Specular = rgbaSpecular0; 
	    m_matMaterial.Emissive = rgbaEmissive;
		break;
	default:
		renderSystem->SetRenderState( D3DRS_ALPHABLENDENABLE, false);
		renderSystem->SetRenderState(D3DRS_AMBIENT, D3DCOLOR_XRGB(92, 92, 92));
		m_matMaterial.Diffuse = rgbaDiffuse1; 
		m_matMaterial.Ambient = rgbaAmbient1; 
		m_matMaterial.Specular = rgbaSpecular1; 
	    m_matMaterial.Emissive = rgbaEmissive0;
		break;
	}
}


extern float mPosX;
extern float mPosY;
extern float mPosZ;

float xr, xg, xb;

void setupLight(D3DXVECTOR3 lightPos, float r, float g, float b) {

xr=r;
xg=g;
xb=b;

ZeroMemory(&d3dLight, sizeof(D3DLIGHT9));

d3dLight.Type = D3DLIGHT_POINT;

d3dLight.Diffuse.r = r;
d3dLight.Diffuse.g = g;
d3dLight.Diffuse.b = b;

d3dLight.Ambient.r = 0.0f;
d3dLight.Ambient.g = 0.0f;
d3dLight.Ambient.b = 0.0f;

d3dLight.Specular.r = r;
d3dLight.Specular.g = g;
d3dLight.Specular.b = b;

d3dLight.Direction.x = 0;
d3dLight.Direction.y = 0;
d3dLight.Direction.z = 0;

d3dLight.Position.x = lightPos.x;
d3dLight.Position.y = lightPos.y;
d3dLight.Position.z = lightPos.z;

d3dLight.Attenuation0 = 1.0f; 
d3dLight.Attenuation1 = 0.0f; 
d3dLight.Attenuation2 = 0.0f; 
d3dLight.Range = 2147483647.0f;

renderSystem->GetDevice()->SetLight(0, &d3dLight);
renderSystem->GetDevice()->LightEnable(0, TRUE);
}

void setupModelLight(int num, D3DXVECTOR3 lightPos, float dist, float r, float g, float b, bool specular) {

ZeroMemory(&d3dLight, sizeof(D3DLIGHT9));

d3dLight.Type = D3DLIGHT_POINT;

d3dLight.Diffuse.r = r;
d3dLight.Diffuse.g = g;
d3dLight.Diffuse.b = b;

d3dLight.Ambient.r = 0.0f;
d3dLight.Ambient.g = 0.0f;
d3dLight.Ambient.b = 0.0f;

if (specular) {
	d3dLight.Specular.r = r;
	d3dLight.Specular.g = g;
	d3dLight.Specular.b = b;
} else {
	d3dLight.Specular.r = 0.0f;
	d3dLight.Specular.g = 0.0f;
	d3dLight.Specular.b = 0.0f;
}

d3dLight.Direction.x = 0;
d3dLight.Direction.y = 0;
d3dLight.Direction.z = 0;

d3dLight.Position.x = lightPos.x;
d3dLight.Position.y = lightPos.y;
d3dLight.Position.z = lightPos.z;

d3dLight.Attenuation0 = 1.0f; 
d3dLight.Attenuation1 = 0.0f; 
d3dLight.Attenuation2 = 0.0f; 
d3dLight.Range = dist;

renderSystem->GetDevice()->SetLight(num+1, &d3dLight);
renderSystem->GetDevice()->LightEnable(num+1, TRUE);
}

void doMatrixes(D3DXMATRIX world) {
	renderSystem->GetDevice()->SetTransform(D3DTS_WORLD, &world);
}

#define corrector 0.0f

void doPerspectiveNear()
{
	D3DXMATRIX matView;
	D3DXMatrixLookAtLH(&matView, &D3DXVECTOR3(0.0f, corrector, 0.0f), //Camera Position
		&D3DXVECTOR3(0.0f, corrector, 1.0f), //Look At Position
		&D3DXVECTOR3(0.0f, 1.0f, 0.0f)); //Up Direction
	renderSystem->GetDevice()->SetTransform(D3DTS_VIEW, &matView);

	D3DXMATRIX matProj;

	float ratio, fov;

	//ratio = 2.012f;
	//fov = 0.6f;
	ratio = 2.0253f;
	//fov = 0.5972f;
	fov = 0.5972f;
	
	// ������� ��������� ��������� ������� ������ ������, ����� �������� � 
	// ������� �������. ��������� �������� � �������, �� �������� ���������� �� 
	// ��������� ����������.
	D3DXMatrixPerspectiveFovLH(&matProj, fov, ratio, 0.1f, 300000);
	renderSystem->GetDevice()->SetTransform(D3DTS_PROJECTION, &matProj);
}

void doPerspectiveFar()
{
	D3DXMATRIX matView;
	D3DXMatrixLookAtLH(&matView, &D3DXVECTOR3(0.0f, corrector, 0.0f), //Camera Position
		&D3DXVECTOR3(0.0f, corrector, 1.0f), //Look At Position
		&D3DXVECTOR3(0.0f, 1.0f, 0.0f)); //Up Direction
	renderSystem->GetDevice()->SetTransform(D3DTS_VIEW, &matView);

	D3DXMATRIX matProj;

	float ratio, fov;

	//ratio = 2.012f;
	//fov = 0.6f;
	ratio = 2.0253f;
	//fov = 0.5972f;
	fov = 0.5972f;
	
	// ������� ��������� ��������� ������� ������ ������, ����� �������� � 
	// ������� �������. ��������� �������� � �������, �� �������� ���������� �� 
	// ��������� ����������.
	D3DXMatrixPerspectiveFovLH(&matProj, fov, ratio, 1.0f, 10e6);
	renderSystem->GetDevice()->SetTransform(D3DTS_PROJECTION, &matProj);
}

// Andis: � ���������� ������������� ������� �� �����������, ������� ����� ����������
// ������ ��������.
void doOrto() {
    D3DXMATRIX matOrtho; 
    D3DXMATRIX matIdentity;
    
    D3DXMatrixOrthoLH(&matOrtho, (float)curwidth, (float)curheight, 0.0f, 1.0f);
    D3DXMatrixIdentity(&matIdentity);

    renderSystem->GetDevice()->SetTransform(D3DTS_PROJECTION, &matOrtho);
    renderSystem->GetDevice()->SetTransform(D3DTS_WORLD, &matIdentity);
    renderSystem->GetDevice()->SetTransform(D3DTS_VIEW, &matIdentity);
}

void doOrtoRT(int w, int h) {
    D3DXMATRIX matOrtho; 
    D3DXMATRIX matIdentity;
    
    D3DXMatrixOrthoLH(&matOrtho, (float)w, (float)h, 0.0f, 1.0f);
    D3DXMatrixIdentity(&matIdentity);

    renderSystem->GetDevice()->SetTransform(D3DTS_PROJECTION, &matOrtho);
    renderSystem->GetDevice()->SetTransform(D3DTS_WORLD, &matIdentity);
    renderSystem->GetDevice()->SetTransform(D3DTS_VIEW, &matIdentity);
}

struct TRANSPBUF {
	int modelNum;
	int index;
};

TRANSPBUF transpBuf[20000];
int modelSort[19000];
int transpMax;
bool doTransp;
int currMod;

inline DWORD FtoDW(FLOAT f) {return *((DWORD*)&f);}

void drawModelPrimitives(int startVert, int endVert) 
{
	int zwritemem;
	//renderSystem->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	//renderSystem->GetDevice()->SetVertexShader(D3DFVF_CUSTOMVERTEX);
	renderSystem->SetVertexDeclaration(vertexBuffer);
	renderSystem->SetStreamSource(vertexBuffer);
	DWORD a;

	zwritemem = renderSystem->GetRenderState(D3DRS_ZWRITEENABLE);
	renderSystem->SetRenderState( D3DRS_ALPHATESTENABLE, false);

	for(int i=startVert;i<endVert;i++) {
		renderSystem->SetRenderState(D3DRS_ZWRITEENABLE, zwritemem);
		//if (vertexType[i].textNum>0 && textures[vertexType[i].textNum]==NULL)
		//	return;
		if (vertexType[i].textNum==43) return;

		//if (vertexType[i].textNum>0 && textures[vertexType[i].textNum]==NULL)
		//	vertexType[i].textNum=0;

/*
		if ((vertexType[i].textNum>=107 && vertexType[i].textNum<254) || (vertexType[i].textNum>261 && vertexType[i].textNum<=290)) {
			renderSystem->SetRenderState( D3DRS_ALPHATESTENABLE, true);
			renderSystem->SetRenderState(D3DRS_ALPHAREF,        220);
			renderSystem->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
		}
*/

		// matrixes
		if (vertexType[i].doLight==true || textures[vertexType[i].textNum] != NULL) {
			renderSystem->SetRenderState(D3DRS_LIGHTING, TRUE);
		} else {
			renderSystem->SetRenderState(D3DRS_LIGHTING, FALSE);
			//selectMaterial(GALAXY, 0, 0, 0);
			//renderSystem->GetDevice()->SetMaterial(&m_matMaterial);
		}
	
		// textures
		if (textures[vertexType[i].textNum] != NULL) {
			renderSystem->SetRenderState( D3DRS_ALPHABLENDENABLE, true);
			renderSystem->GetDevice()->SetTexture(0, textures[vertexType[i].textNum]);
		} else {
			renderSystem->SetRenderState( D3DRS_ALPHABLENDENABLE, false);
			renderSystem->GetDevice()->SetTexture(0, NULL);
		}

		// primitives
		if (vertexType[i].type==GL_LINES) {
			if (doTransp==false)
				renderSystem->GetDevice()->DrawPrimitive(D3DPT_LINELIST, i, vertexType[i].amount/2);
			i+=vertexType[i].amount-1;
			continue;
		}
		if (vertexType[i].type==GL_LINE_STRIP) {
			if (doTransp==false)
				renderSystem->GetDevice()->DrawPrimitive(D3DPT_LINESTRIP, i, vertexType[i].amount-1);
			i+=vertexType[i].amount-1;
			continue;
		}
		if (vertexType[i].type==GL_LINE_LOOP) {
			if (doTransp==false)
				renderSystem->GetDevice()->DrawPrimitive(D3DPT_LINESTRIP, i, vertexType[i].amount-1);
			i+=vertexType[i].amount-1;
			continue;
		}
		if (vertexType[i].type==GL_TRIANGLES) {
			renderSystem->SetRenderState(D3DRS_SPECULARENABLE, vertexType[i].specular);
			//renderSystem->SetRenderState(D3DRS_ZWRITEENABLE, vertexType[i].doLight);
			if (doTransp==false)
				renderSystem->GetDevice()->DrawPrimitive(D3DPT_TRIANGLELIST, i, vertexType[i].amount/3);
			i+=vertexType[i].amount-1;
			continue;
		}
		if (vertexType[i].type==GL_TRIANGLE_STRIP) {
			// ���������� �������� ����� �������� �����
			if (doTransp==false && vertexType[i].transparent==true) {
				transpBuf[transpMax].index=i;
				transpBuf[transpMax].modelNum=currMod;
				transpMax++;
				i+=vertexType[i].amount-1;
				continue;
			}
			//renderSystem->SetRenderState(D3DRS_ZWRITEENABLE, vertexType[i].doLight);
			renderSystem->GetDevice()->DrawPrimitive(D3DPT_TRIANGLESTRIP, i, vertexType[i].amount-2);
			i+=vertexType[i].amount-1;
			continue;
		}
		if (vertexType[i].type==GL_TRIANGLE_FAN) {
			
			// ���������� �������� ����� �������� �����
			if (doTransp==false && vertexType[i].transparent==true) {
				transpBuf[transpMax].index=i;
				transpBuf[transpMax].modelNum=currMod;
				transpMax++;
				i+=vertexType[i].amount-1;
				continue;
			}
			//renderSystem->SetRenderState(D3DRS_ZWRITEENABLE, vertexType[i].doLight);
			renderSystem->GetDevice()->DrawPrimitive(D3DPT_TRIANGLEFAN, i, vertexType[i].amount-2);
			i+=vertexType[i].amount-1;
			continue;
		}

		if (vertexType[i].type==GL_POINTS) { // ��������
			
			if (doTransp==false) {
				transpBuf[transpMax].index=i;
				transpBuf[transpMax].modelNum=currMod;
				transpMax++;
				continue;
			}

			//renderSystem->SetRenderState( D3DRS_ALPHABLENDENABLE, true);
			
			a = renderSystem->GetRenderState(D3DRS_AMBIENT);
			renderSystem->SetRenderState(D3DRS_AMBIENT, D3DCOLOR_XRGB(255, 255, 255));
			//renderSystem->SetRenderState(D3DRS_ZWRITEENABLE, false);
			renderSystem->SetRenderState(D3DRS_POINTSIZE, FtoDW(vertexType[i].radius));
			
			renderSystem->SetRenderState(D3DRS_POINTSIZE_MIN, FtoDW(15.0f));
			renderSystem->SetRenderState(D3DRS_POINTSIZE_MAX, FtoDW(15.0f));
			renderSystem->SetRenderState( D3DRS_POINTSCALE_A,  FtoDW(1.0f)); 
			renderSystem->SetRenderState( D3DRS_POINTSCALE_B,  FtoDW(0.0f)); 
			renderSystem->SetRenderState( D3DRS_POINTSCALE_C,  FtoDW(0.0f)); 
			renderSystem->GetDevice()->SetTexture(0, textures[602]);
			renderSystem->GetDevice()->DrawPrimitive(D3DPT_POINTLIST, i, 1);
			
			renderSystem->SetRenderState(D3DRS_POINTSIZE_MIN, FtoDW(1.0f));
			renderSystem->SetRenderState(D3DRS_POINTSIZE_MAX, FtoDW(50.0f));
			renderSystem->SetRenderState( D3DRS_POINTSCALE_A,  FtoDW(0.0f)); 
			renderSystem->SetRenderState( D3DRS_POINTSCALE_B,  FtoDW(1.0f)); 
			renderSystem->SetRenderState( D3DRS_POINTSCALE_C,  FtoDW(0.0f)); 
			renderSystem->GetDevice()->SetTexture(0, textures[603]);
			renderSystem->GetDevice()->DrawPrimitive(D3DPT_POINTLIST, i, 1);
			
			//renderSystem->SetRenderState(D3DRS_ZWRITEENABLE, true);
			renderSystem->SetRenderState(D3DRS_AMBIENT, a);
			//renderSystem->SetRenderState(D3DRS_ZBIAS,0);
			
			continue;
		}

		if (vertexType[i].type==GL_POINTS2) { // ������� �������/������
//			renderSystem->SetRenderState(D3DRS_ZENABLE, true);
			//renderSystem->SetRenderState( D3DRS_ALPHABLENDENABLE, true);
			
//			a = renderSystem->GetRenderState(D3DRS_AMBIENT);
//			renderSystem->SetRenderState(D3DRS_AMBIENT, D3DCOLOR_XRGB(255, 255, 255));
			//renderSystem->SetRenderState(D3DRS_ZWRITEENABLE, false);
			renderSystem->SetRenderState(D3DRS_POINTSIZE, FtoDW(vertexType[i].radius));
				
			renderSystem->SetRenderState(D3DRS_POINTSIZE_MIN, FtoDW(5.0f));
			renderSystem->SetRenderState(D3DRS_POINTSIZE_MAX, FtoDW(10.0f));
			renderSystem->SetRenderState( D3DRS_POINTSCALE_A,  FtoDW(1.0f)); 
			renderSystem->SetRenderState( D3DRS_POINTSCALE_B,  FtoDW(1.0f)); 
			renderSystem->SetRenderState( D3DRS_POINTSCALE_C,  FtoDW(0.0f)); 
			renderSystem->GetDevice()->SetTexture(0, textures[356]);
			renderSystem->GetDevice()->DrawPrimitive(D3DPT_POINTLIST, i, 1);
			
			//renderSystem->SetRenderState(D3DRS_ZWRITEENABLE, true);
//			renderSystem->SetRenderState(D3DRS_AMBIENT, a);
			//renderSystem->SetRenderState(D3DRS_ZBIAS,0);
			
			continue;
		}
	}
}


extern float dist;
extern float radius;
D3DXVECTOR3 playerLightPos;
bool playerLightPosEnable;
extern unsigned char currentAmbientR;
extern unsigned char currentAmbientG;
extern unsigned char currentAmbientB;

bool panellight[20];
extern int incabin;
extern bool clearBeforeRender;

void Render()
{
	D3DXMATRIX mdscale, mdworld, mdrotate;
	D3DXMATRIX mat;
	float scale, scale_x, scale_y, scale_z;
	float dtime;
	float lG;

    if(renderSystem->GetDevice() == NULL)
    {
        return;
    }

	if (clearBeforeRender)
		renderSystem->GetDevice()->Clear(0,NULL,D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,D3DCOLOR_XRGB(0,0,0),1.0f,0);

	doTransp=false;
	transpMax=0;

	int aspectfactor=(curheight-curwidth/1.6f);

	static int iFrames = 0;
	static double sttime = 0.0;
	iFrames++;
	if (iFrames == 100) 
	{
		QueryPerformanceCounter(&nowtime);
		float fps = float(iFrames/((nowtime.QuadPart - sttime)/ticks));
		char cBuff[32];
		sprintf(cBuff,"FFED3D FPS: %.1f", fps);
		//SetWindowText(hWnd,cBuff);
		sttime = (double)nowtime.QuadPart;
		iFrames = 0;
	}
	char cBuff[200];
	sprintf(cBuff,"FFED3D x: %f, y: %f, z: %f, size: %f", pl.x, pl.y, pl.z, plsize);
	//SetWindowText(hWnd,cBuff);

	//if (*(unsigned char*)DATA_008809==254)
	//	renderSystem->GetDevice()->Clear(0,NULL,D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,D3DCOLOR_XRGB(0,0,0),1.0f,0);	

	D3DRECT rect;
	rect.x1=0;
	if (aspectfix)
		rect.y1=(int)((float)curheight/200*157-aspectfactor/4);
	else
		rect.y1=(int)((float)curheight/200*157);
	rect.x2=curwidth;
	rect.y2=curheight;
	
	if (clearBeforeRender==false) {
		renderSystem->GetDevice()->Clear(1,(D3DRECT *)&rect,D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,D3DCOLOR_XRGB(0,0,0),1.0f,0);	
	}


		{
			renderSystem->SetRenderState(D3DRS_ZENABLE, false);

			renderSystem->GetDevice()->GetRenderTarget(0, &oldSurface);
			panelTex[0]->GetSurfaceLevel(0,&texSurface);
			renderSystem->GetDevice()->SetRenderTarget(0, texSurface);
			//doOrtoRT(1024, 1024);
			D3DVIEWPORT9 viewport;

			viewport.X = 0;
			viewport.Y = 0;
			viewport.Width = 1024;
			viewport.Height = 1024; 
			viewport.MinZ = 0; 
			viewport.MaxZ = 1; 
			renderSystem->GetDevice()->SetViewport( &viewport ); 	

			D3DXVECTOR3 spos;
			RECT rectsize;
			rectsize.left=0;
			rectsize.top=0;
			rectsize.right=49;
			rectsize.bottom=50;
			spos.x=549;
			spos.y=595;
			spos.z=0;
			textSprite->Begin(D3DXSPRITE_ALPHABLEND);
			D3DXMatrixIdentity(&mat);
			textSprite->SetTransform(&mat);
			textSprite->Draw(panelIcons[0],&rectsize,NULL,&spos,0xFFFFFFFF);
			textSprite->End();
			//rect.x1=512;
			//rect.y1=512;
			//rect.x2=750;
			//rect.y2=750;
			//renderSystem->GetDevice()->Clear(1,(D3DRECT *)&rect,D3DCLEAR_TARGET,D3DCOLOR_XRGB(0,0,0),1.0f,0);	


			renderSystem->GetDevice()->SetRenderTarget(0, oldSurface);

		}

    renderSystem->BeginScene();


    //renderSystem->GetDevice()->SetVertexShader(D3DFVF_CUSTOMVERTEX);
	//renderSystem->GetDevice()->SetStreamSource(0, d3d_vb, sizeof(CUSTOMVERTEX));

	//renderSystem->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESS); 
	if (wireframe==0) 
		renderSystem->SetRenderState(D3DRS_FILLMODE,D3DFILL_SOLID);
	else
		renderSystem->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME);
	
	renderSystem->SetRenderState(D3DRS_NORMALIZENORMALS, TRUE); 
	renderSystem->SetRenderState( D3DRS_COLORVERTEX, true );
	renderSystem->SetRenderState( D3DRS_SHADEMODE, D3DSHADE_GOURAUD );
	//renderSystem->SetRenderState( D3DRS_SHADEMODE, D3DSHADE_FLAT );
	//renderSystem->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, true);
	renderSystem->SetRenderState(D3DRS_ZENABLE, true);

	renderSystem->GetDevice()->SetSamplerState( 0, D3DSAMP_ADDRESSU,  D3DTADDRESS_WRAP ); 
	renderSystem->GetDevice()->SetSamplerState( 0, D3DSAMP_ADDRESSV,  D3DTADDRESS_WRAP ); 
	//renderSystem->GetDevice()->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE  );
	//renderSystem->GetDevice()->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TEXTURE );
	//renderSystem->GetDevice()->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE);

	renderSystem->GetDevice()->SetSamplerState(0, D3DSAMP_MAGFILTER, 3);
	renderSystem->GetDevice()->SetSamplerState(0, D3DSAMP_MINFILTER, 3 );
	renderSystem->GetDevice()->SetSamplerState(0, D3DSAMP_MIPFILTER, 3 );

	renderSystem->SetRenderState(D3DRS_POINTSPRITEENABLE, TRUE); 
	renderSystem->SetRenderState(D3DRS_POINTSCALEENABLE, TRUE); 

	renderSystem->SetRenderState( D3DRS_ALPHATESTENABLE, false);
	renderSystem->SetRenderState(D3DRS_LIGHTING, true);

	// draw galaxy background
	//renderSystem->GetDevice()->SetSamplerState(0, D3DSAMP_MIPFILTER, 0);
	ViewPort(true);
	doPerspectiveNear();
	renderSystem->SetRenderState(D3DRS_SPECULARENABLE, FALSE);
	selectMaterial(GALAXY, 0, 0, 0);
	renderSystem->GetDevice()->SetMaterial(&m_matMaterial);
	renderSystem->SetRenderState( D3DRS_ALPHABLENDENABLE, true);

	renderSystem->SetRenderState(D3DRS_ZWRITEENABLE, false);
	renderSystem->SetRenderState(D3DRS_ZENABLE, false);

	// nebula
	if (*(unsigned char*)(DATA_008804+0x20)!=0 || *(unsigned char*)(DATA_008804+0x21)!=0 || *(unsigned char*)(DATA_008804+0x22)!=0)
	for(int m=0;m<modelNum;m++) {
		if (modelList[m].index==315 || 
			modelList[m].index==444) {
			if (modelList[m].material!=NOTDRAW) {
				currMod=m;

				/*
				renderSystem->SetRenderState(D3DRS_CULLMODE, modelList[m].cull);
				doMatrixes(modelList[m].world);
				drawModelPrimitives(modelList[m].vertStart, modelList[m].vertEnd);
				*/		

				D3DXMATRIX mdworld;

				D3DXMatrixIdentity(&mdworld);
				D3DXMatrixRotationZ(&mdworld, 3.14f/2);

				D3DXMatrixMultiply(&mdworld, &mdworld, &modelList[m].world);
				
				mdworld[12]=0.0f;
				mdworld[13]=0.0f;
				mdworld[14]=0.0f;
				
				doMatrixes(mdworld);

				renderSystem->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
				objectList[315]->FrameMove(0, 0,&mdworld);

				renderSystem->GetDevice()->SetTexture(0, skyboxtex[0]);
				objectList[315]->Render(0,24,0,2);
				renderSystem->GetDevice()->SetTexture(0, skyboxtex[1]);
				objectList[315]->Render(0,24,2,2);
				renderSystem->GetDevice()->SetTexture(0, skyboxtex[2]);
				objectList[315]->Render(0,24,4,2);
				renderSystem->GetDevice()->SetTexture(0, skyboxtex[3]);
				objectList[315]->Render(0,24,6,2);
				renderSystem->GetDevice()->SetTexture(0, skyboxtex[4]);
				objectList[315]->Render(0,24,8,2);
				renderSystem->GetDevice()->SetTexture(0, skyboxtex[5]);
				objectList[315]->Render(0,24,10,2);
				break;
			}
		}
	}

	// stars
	/*
	for(m=0;m<modelNum;m++) {
		if (modelList[m].index==316) {
			if (modelList[m].material!=NOTDRAW) {
				currMod=m;
				renderSystem->SetRenderState(D3DRS_CULLMODE, modelList[m].cull);
				doMatrixes(modelList[m].world);
				drawModelPrimitives(modelList[m].vertStart, modelList[m].vertEnd);
			}
		}
	}
	*/
	for(int m=0;m<modelNum;m++) {
		if (modelList[m].index==315 || 
			modelList[m].index==444) {
			D3DXMATRIX mdscale, mdworld, mdrotate;
			float scale;
			
			int starMod=316;

			D3DXMatrixIdentity(&mdworld);
					
			D3DXMatrixMultiply(&mdworld, &mdworld, &modelList[m].world);

			//mdworld[12]*=1000;
			//mdworld[13]*=1000;
			//mdworld[14]*=1000;
			scale=0.1;
			D3DXMatrixScaling(&mdscale, scale, scale, scale);
			D3DXMatrixMultiply(&mdworld, &mdscale, &mdworld);
			doMatrixes(mdworld);
			

			renderSystem->GetDevice()->SetTexture(0, g_texture[starMod]);
			//renderSystem->GetDevice()->SetRenderState(D3DRS_CULLMODE, g_model[starMod].cullmode);
			renderSystem->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
			objectList[starMod]->FrameMove(0,0,&mdworld);
			objectList[starMod]->Render();
			//doMatrixes(modelList[m].world);
			break;
		}
	}
	
	// end draw galaxy background

	renderSystem->SetRenderState(D3DRS_ZENABLE, true);

		// panel


		if (*(unsigned char*)DATA_008835!=32 && *(unsigned char*)DATA_008835!=35) {

			renderSystem->GetDevice()->SetSamplerState(0, D3DSAMP_MIPFILTER, 0);
			renderSystem->SetRenderState(D3DRS_SPECULARENABLE, true);
			selectMaterial(-1, 0, 0, 0);
			renderSystem->GetDevice()->SetMaterial(&m_matMaterial);
			renderSystem->SetRenderState( D3DRS_ALPHABLENDENABLE, false);
			renderSystem->SetRenderState(D3DRS_ZWRITEENABLE, true);
			renderSystem->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
			for(int i=0;i<10;i++) {
				renderSystem->GetDevice()->LightEnable(i, false);
			}
			int c=0;
			float cr = 0.5f;
			float cg = 1.0f;
			float cb = 0.5f;
			D3DXVECTOR3 lpos;
			for(int i=1;i<=15;i++) {
				if (panellight[i]) {
					switch(i) {
						case 1: 
							lpos.x = -1.35f;
							lpos.y = -0.94f+corrector;
							lpos.z = 3.91f;
							setupModelLight(c++, lpos, 0.18f, cr, cg, cb, true);
							break;
						case 2: 
							lpos.x = -1.18f;
							lpos.y = -0.95f+corrector;
							lpos.z = 4.08f;
							setupModelLight(c++, lpos, 0.18f, cr, cg, cb, true);
							break;
						case 3: 
							lpos.x = -1.01f;
							lpos.y = -0.94f+corrector;
							lpos.z = 4.25f;
							setupModelLight(c++, lpos, 0.18f, cr, cg, cb, true);
							break;
						case 4: 
							lpos.x = -0.86f;
							lpos.y = -0.94f+corrector;
							lpos.z = 4.35f;
							setupModelLight(c++, lpos, 0.18f, cr, cg, cb, true);
							break;
						case 5: 
							lpos.x = -0.64f;
							lpos.y = -0.98f+corrector;
							lpos.z = 4.44f;
							setupModelLight(c++, lpos, 0.18f, cr, cg, cb, true);
							break;
						case 6: 
							lpos.x = -0.42f;
							lpos.y = -0.99f+corrector;
							lpos.z = 4.45f;
							setupModelLight(c++, lpos, 0.18f, cr, cg, cb, true);
							break;
						case 7: 
							lpos.x = -0.12f;
							lpos.y = -1.01f+corrector;
							lpos.z = 4.45f;
							setupModelLight(c++, lpos, 0.18f, cr, cg, cb, true);
							break;
						case 8: 
							lpos.x = 0.19f;
							lpos.y = -0.96f+corrector;
							lpos.z = 4.45f;
							setupModelLight(c++, lpos, 0.18f, cr, cg, cb, true);
							break;
						case 9: 
							lpos.x = 0.41f;
							lpos.y = -0.97f+corrector;
							lpos.z = 4.36f;
							setupModelLight(c++, lpos, 0.18f, cr, cg, cb, true);
							break;
						case 10: 
							lpos.x = 0.6f;
							lpos.y = -0.93f+corrector;
							lpos.z = 4.24f;
							setupModelLight(c++, lpos, 0.18f, cr, cg, cb, true);
							break;
						case 11: 
							lpos.x = 0.84f;
							lpos.y = -0.93f+corrector;
							lpos.z = 4.1f;
							setupModelLight(c++, lpos, 0.18f, cr, cg, cb, true);
							break;
						case 12: 
							lpos.x = 1.07f;
							lpos.y = -0.94f+corrector;
							lpos.z = 3.96f;
							setupModelLight(c++, lpos, 0.18f, cr, cg, cb, true);
							break;
						case 13: 
							lpos.x = 1.4f;
							lpos.y = -0.96f+corrector;
							lpos.z = 3.83f;
							setupModelLight(c++, lpos, 0.18f, cr, cg, cb, true);
							break;
						case 14: 
							lpos.x = 1.85f;
							lpos.y = -0.96f+corrector;
							lpos.z = 3.77f;
							setupModelLight(c++, lpos, 0.18f, cr, cg, cb, true);
							break;
						case 15: 
							lpos.x = 2.25f;
							lpos.y = -0.94f+corrector;
							lpos.z = 3.68f;
							setupModelLight(c++, lpos, 0.18f, cr, cg, cb, true);
							break;
						default: break;
					}
				}
			}
			//renderSystem->GetDevice()->LightEnable(0, TRUE);
			//if ((int)*(unsigned char*)DATA_008872>=2)
			//	setupLight(playerLightPos, xr, xg, xb);
			//else
			//D3DXVECTOR3 lp;
			//lp.x=*(int*)(DATA_008804+0x54+12);
			//lp.y=*(int*)(DATA_008804+0x54+16);
			////lp.z=*(int*)(DATA_008804+0x54+20);
			setupLight(playerLightPos, 1.0f/255*min(255,currentAmbientR*16+100), 1.0f/255*min(255,currentAmbientG*16+100), 1.0f/255*min(255,currentAmbientB*16+100));
			//setupLight(playerLightPos, 1.0f, 1.0f, 1.0f);

			ViewPort(false);

			D3DXMATRIX mdworld, mdobject;
			D3DXMatrixIdentity(&mdworld);

			mdworld[0]=1.4f;
			mdworld[1]=0.0f;
			mdworld[2]=0.0f;
			mdworld[3]=0.0f;

			mdworld[4]=0.0f;
			mdworld[5]=1.0f;
			mdworld[6]=0.0f;
			mdworld[7]=0.0f;

			mdworld[8]=0.0f;
			mdworld[9]=0.0f;
			mdworld[10]=1.0f;
			mdworld[11]=0.0f;

			mdworld[12]=-0.35f;
			mdworld[13]=-1.07f+corrector;
			mdworld[14]=4.0f;
			doMatrixes(mdworld);
			panelObj->FrameMove(0,0,&mdworld);
			renderSystem->GetDevice()->SetTexture(0, panelTex[panelnum]);
			panelObj->Render();
		}
		for(int i=0;i<20;i++) {
			panellight[i]=false;
		}

	// ���������� ����������
	renderSystem->GetDevice()->SetSamplerState(0, D3DSAMP_MIPFILTER, 0);
	renderSystem->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	renderSystem->SetRenderState(D3DRS_ZENABLE, false);
	renderSystem->SetRenderState(D3DRS_ZWRITEENABLE, false);
	selectMaterial(GALAXY, 0, 0, 0);
	renderSystem->GetDevice()->SetMaterial(&m_matMaterial);
	ViewPort(false);
	doOrto();

	for(int m=0;m<modelNum;m++) {
		if (modelList[m].backsprite==false)
			continue;
		currMod=m;
		drawModelPrimitives(modelList[m].vertStart, modelList[m].vertEnd);
	}

	// ���������� ������� ������

	renderSystem->GetDevice()->SetSamplerState(0, D3DSAMP_MIPFILTER, 2);
	renderSystem->SetRenderState(D3DRS_ZENABLE, true);

	ViewPort(true);
	doPerspectiveFar();

	for(int m=0;m<modelNum;m++) {
		if (modelList[m].index==315 || 
			modelList[m].index==444 || 
			modelList[m].index==316 ||
			modelList[m].backsprite==true) {
			continue;
		}
		if (modelList[m].doMatrix==1) {
			currMod=m;

			renderSystem->SetRenderState(D3DRS_ZWRITEENABLE, modelList[m].zwrite);
			renderSystem->SetRenderState(D3DRS_ZENABLE, modelList[m].zenable);
			renderSystem->SetRenderState(D3DRS_CULLMODE, modelList[m].cull);
			selectMaterial(modelList[m].material, modelList[m].ambientR, modelList[m].ambientG, modelList[m].ambientB);
			renderSystem->GetDevice()->SetMaterial(&m_matMaterial);

			// ����� ����
			setupLight(modelList[m].lightPos, 1.0f/255*modelList[m].ambientR, 1.0f/255*modelList[m].ambientG, 1.0f/255*modelList[m].ambientB);
			// ������������ �����
			for(int ii=0;ii<10 && !modelList[m].subObject;ii++) {
				if (modelList[m].light[ii].enable==true) {
					setupModelLight(ii, modelList[m].light[ii].pos, modelList[m].light[ii].dist, modelList[m].light[ii].r, modelList[m].light[ii].g, modelList[m].light[ii].b, true);
				}
			}

			// �������
			if (splineList[modelList[m].index]) {
				renderSystem->GetDevice()->SetTexture(0, 0);
				splineList[modelList[m].index]->FrameMove(0,0,&modelList[m].world);
				splineList[modelList[m].index]->Render();
			}

			// ������
			if (objectList[modelList[m].index]) {
				D3DXMatrixIdentity(&mdworld);				
				mdworld[12]+=modelconfig[modelList[m].index].offset_x;
				mdworld[13]+=modelconfig[modelList[m].index].offset_y;
				mdworld[14]+=modelconfig[modelList[m].index].offset_z;
				
				D3DXMatrixMultiply(&mdworld, &mdworld, &modelList[m].world);
				
				scale=modelconfig[modelList[m].index].scale;
				D3DXMatrixScaling(&mdscale, scale, scale, scale);
				D3DXMatrixMultiply(&mdworld, &mdscale, &mdworld);

				scale_x=modelconfig[modelList[m].index].scale_x;
				scale_y=modelconfig[modelList[m].index].scale_y;
				scale_z=modelconfig[modelList[m].index].scale_z;
				D3DXMatrixScaling(&mdscale, scale_x, scale_y, scale_z);
				D3DXMatrixMultiply(&mdworld, &mdscale, &mdworld);

				renderSystem->GetDevice()->SetTexture(0, g_texture[modelList[m].index]);

				//find the time difference
				QueryPerformanceCounter(&nowtime);
				dtime = ((nowtime.QuadPart - starttime)/ticks);
				if (modelList[m].index > 14 && modelList[m].index < 70) {
					if (modelList[m].landingGear>0 || objectList[modelList[m].index]->GetNumAnimationSets()==1) {
						objectList[modelList[m].index]->SetAnimationSet(0);
						lG = (float)modelList[m].landingGear/65536*objectList[modelList[m].index]->GetAnimationSetLength(0);
						objectList[modelList[m].index]->FrameMove(0, lG,&mdworld);
					} else {
						objectList[modelList[m].index]->SetAnimationSet(1);
						objectList[modelList[m].index]->FrameMove(1, dtime,&mdworld);
					}
				} else {
					objectList[modelList[m].index]->SetAnimationSet(0);
					objectList[modelList[m].index]->FrameMove(1, dtime,&mdworld);
				}
				starttime = nowtime.QuadPart;
				objectList[modelList[m].index]->Render();
			}

			// ������������ ���������� ������
			doMatrixes(modelList[m].world);
			drawModelPrimitives(modelList[m].vertStart, modelList[m].vertEnd);
			//if (modelList[m].zclear==true)
			//	renderSystem->GetDevice()->Clear(0,NULL,D3DCLEAR_ZBUFFER,D3DCOLOR_XRGB(0,0,0),1.0f,0);	

			//for(int ii=1;ii<=10 && !modelList[m].subObject;ii++) {
			///	renderSystem->GetDevice()->LightEnable(ii, false);
			//}
		}
	}

	// ���������� �������
			
	renderSystem->GetDevice()->SetSamplerState(0, D3DSAMP_MIPFILTER, 0);
	renderSystem->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	renderSystem->SetRenderState(D3DRS_ZENABLE, true);
	selectMaterial(GALAXY, 0, 0, 0);
	renderSystem->GetDevice()->SetMaterial(&m_matMaterial);
	ViewPort(false);
	doOrto();

	for(int m=0;m<modelNum;m++) {
		if (modelList[m].index==315 || 
			modelList[m].index==444 || 
			modelList[m].index==316 ||
			modelList[m].backsprite==true) {
			continue;
		}
		if (modelList[m].doMatrix==2) {
			currMod=m;

			renderSystem->SetRenderState(D3DRS_ZWRITEENABLE, modelList[m].zwrite);
			drawModelPrimitives(modelList[m].vertStart, modelList[m].vertEnd);
		}
	}

	// ������ ����� ���������� ���������� ��������

	renderSystem->SetRenderState(D3DRS_ZENABLE, true);
	renderSystem->SetRenderState(D3DRS_SPECULARENABLE, FALSE);
	selectMaterial(GALAXY, 0, 0, 0);
	renderSystem->GetDevice()->SetMaterial(&m_matMaterial);
	renderSystem->SetRenderState( D3DRS_ALPHABLENDENABLE, true);
	renderSystem->SetRenderState(D3DRS_ZWRITEENABLE, false);
	doTransp=true;
	renderSystem->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	for(int m=0;m<transpMax;m++) {
		currMod=transpBuf[m].modelNum;
		// ���� �� ������
		if (modelList[currMod].doMatrix==1) {
			renderSystem->GetDevice()->SetSamplerState(0, D3DSAMP_MIPFILTER, 2);
			//renderSystem->SetRenderState(D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_COLOR1 );
			//renderSystem->SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_MATERIAL);
			renderSystem->SetRenderState(D3DRS_ZENABLE, true);
			ViewPort(true);
			doPerspectiveFar();
			doMatrixes(modelList[currMod].world);
		} else { // ��� ������
			//renderSystem->SetRenderState(D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_MATERIAL );
			//renderSystem->SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1);
			renderSystem->SetRenderState(D3DRS_ZENABLE, false);
			renderSystem->GetDevice()->SetSamplerState(0, D3DSAMP_MIPFILTER, 0);
			ViewPort(false);
			doOrto();
		}
		drawModelPrimitives(transpBuf[m].index, transpBuf[m].index+1);
	}

	renderSystem->SetRenderState(D3DRS_ZWRITEENABLE, true );
	renderSystem->SetRenderState(D3DRS_ZENABLE, true);

	// panel sphere
	if (*(unsigned char*)DATA_008835!=32 && *(unsigned char*)DATA_008835!=35) {
		renderSystem->GetDevice()->SetSamplerState(0, D3DSAMP_MIPFILTER, 2);
		for(int i=0;i<10;i++) {
			renderSystem->GetDevice()->LightEnable(i, false);
		}
		setupLight(playerLightPos, 1.0f/255*min(255,currentAmbientR*28), 1.0f/255*min(255,currentAmbientG*28), 1.0f/255*min(255,currentAmbientB*28));

		ViewPort(false);
		doPerspectiveNear();
		D3DXMATRIX mdworld;
		D3DXMatrixIdentity(&mdworld);

		mdworld[0]=1.4f;
		mdworld[1]=0.0f;
		mdworld[2]=0.0f;
		mdworld[3]=0.0f;

		mdworld[4]=0.0f;
		mdworld[5]=1.0f;
		mdworld[6]=0.0f;
		mdworld[7]=0.0f;

		mdworld[8]=0.0f;
		mdworld[9]=0.0f;
		mdworld[10]=1.0f;
		mdworld[11]=0.0f;

		mdworld[12]=-0.35f;
		mdworld[13]=-1.07f+corrector;
		mdworld[14]=4.0f;
		doMatrixes(mdworld);

		selectMaterial(GALAXY, 0, 0, 0);
		renderSystem->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
		renderSystem->SetRenderState(D3DRS_ALPHATESTENABLE, false);
		renderSystem->SetRenderState(D3DRS_SPECULARENABLE, true);
		renderSystem->SetRenderState(D3DRS_LIGHTING, true);
		renderSystem->SetRenderState(D3DRS_AMBIENT, D3DCOLOR_XRGB(64, 64, 64));
		m_matMaterial.Diffuse = rgbaDiffuse1; 
		m_matMaterial.Ambient = rgbaAmbient1; 
		m_matMaterial.Specular = rgbaSpecular2; 
		m_matMaterial.Emissive = rgbaEmissive0;

		sphereObj->FrameMove(0,0,&mdworld);
		renderSystem->GetDevice()->SetMaterial(&m_matMaterial);
		renderSystem->GetDevice()->SetTexture(0, textures[0]);
		sphereObj->Render();
	}


	ViewPort(false);
	doOrto();

	// ��������� ���������
	textSprite->Begin(D3DXSPRITE_ALPHABLEND);
	D3DXMatrixIdentity(&mat);
	textSprite->SetTransform(&mat);
    for (int i=0;i<textNum;i++) {
		if (aspectfix) {
			ffText[i].x=ffText[i].x*curwidth/640;
			ffText[i].y=ffText[i].y*(curheight-aspectfactor)/400;
			//if (ffText[i].y>400/2) {
			//	factor*=-1;
			//}
			ffText[i].y+=aspectfactor/2;
		} else {
			ffText[i].x=ffText[i].x*curwidth/640;
			ffText[i].y=ffText[i].y*curheight/400;
		}
		DrawText(ffText[i].text, ffText[i].x, ffText[i].y, ffText[i].color);
		//DrawText(ffText[i].text, ffText[i].x*curwidth/640, ffText[i].y*curheight/400, ffText[i].color);
	}
/*
	renderSystem->SetRenderState( D3DRS_ALPHABLENDENABLE, true);
	//renderSystem->SetRenderState( D3DRS_ALPHATESTENABLE, true);
	//renderSystem->SetRenderState(D3DRS_ALPHAREF,        80);
	//renderSystem->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);

	
    //D3DTEXF_NONE            = 0,    // filtering disabled (valid for mip filter only)
    //D3DTEXF_POINT           = 1,    // nearest
    //D3DTEXF_LINEAR          = 2,    // linear interpolation
    //D3DTEXF_ANISOTROPIC     = 3,    // anisotropic
    //D3DTEXF_PYRAMIDALQUAD   = 6,    // 4-sample tent
    //D3DTEXF_GAUSSIANQUAD    = 7,    // 4-sample gaussian
*/	
	renderSystem->GetDevice()->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
	renderSystem->GetDevice()->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
	renderSystem->GetDevice()->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);

	D3DXVECTOR2 scaling(1.0f*2.5f,1.0f*2.5f);

	D3DXVECTOR3 spos;
	for (int i=0;i<maxsprite;i++) {
		D3DXMatrixTransformation2D(&mat,NULL,0.0,&scaling,NULL,NULL,NULL);
		textSprite->SetTransform(&mat);
		if (aspectfix) {
			spos.x=spriteList[i].pos.x;///320*curwidth;
			spos.y=spriteList[i].pos.y;///200*(curheight-aspectfactor)+aspectfactor/2;
		} else {
			spos.x=spriteList[i].pos.x/320*curwidth;
			spos.y=spriteList[i].pos.y/200*curheight;			
		}
		spos.z=0;
		textSprite->Draw(textures[spriteList[i].tex],NULL,NULL,&spos,0xFFFFFFFF);
	}
	


	textSprite->End();
	//renderSystem->SetRenderState( D3DRS_ALPHATESTENABLE, false);
	renderSystem->EndScene();    
    //renderSystem->GetDevice()->Present(NULL, NULL, NULL, NULL);

	vertexNum=0;
	textNum=0;
	modelNum=0;
	maxsprite=0;
	modelList[0].material=NOTDRAW;
	sprites.clear();
	playerLightPosEnable=false;
	clearBeforeRender=false;
	if (incabin>=2)
		incabin=0;
}
