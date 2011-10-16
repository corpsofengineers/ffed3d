#include <windows.h>
#include <assert.h>
#include <ddraw.h>
#include <d3dx9.h>
#include <sys/stat.h>

#include "../ffeapi.h"
#include "../ffecfg.h"
#include "win32api.h"
#include "ffe3d.h"
#include "Macros.h"
#include "ini/ini.h"
#include "Render/RenderSystem.h"
#include "ffescr.h"

D3DXVECTOR3 pl;
float plsize;

static int exclusive = 0;
static int fullscreen = 0;
int aspectfix = 1;
static int wireframe = 0;
int console = 0;
extern char* con_text;

static int fswidth = 640;
static int fsheight = 400;
static int fsbpp = 32;

static int winwidth = 640;
static int winheight = 400;

static int vidInit = 0;
static int active = 1;
static int ptrEnabled = 1;
static int error = 0;
static int bpp = 32;

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
LPDIRECT3DTEXTURE9 starstex;
D3DMATERIAL9 m_matMaterial;
D3DLIGHT9 d3dLight;
LPD3DXFONT m_pFont, m_tFont;

VERTEXTYPE vertexType[MAXVERT];
int vertexNum=0;
MODEL modelList[6000];
int modelNum=0;
FFTEXT ffText[2000];
int textNum=0;
FFEsprite spriteList[400];
int maxsprite;

bool renderAvi=false;

//Set up the time stuff
double starttime;
LARGE_INTEGER nowtime;
LONGLONG ticks;

LARGE_INTEGER time;

DWORD CurrentTime;
DWORD LastTime;
int FPS = 0;
float DeltaTime;
float tt = 0.0f;

bool InitD3D(HWND hWnd);
void Render();
void setupLight();
void CreateLocalFont();
void ViewPort();
void loadTextures();
void loadDirectXModel();
void checkExport();
void DrawText(LPSTR pText, int x, int y, D3DCOLOR rgbFontColour);
extern bool Planet(int modelNum);
extern bool Rings(int modelNum);

CXFileEntity* LoadModel(const std::string &filename, CXFileEntity* obj);

static FILE *pLog = NULL;

extern "C" int asmmain (int argc, char **argv);

extern "C" unsigned char *DATA_008604;
extern "C" unsigned char *DATA_008605; // avi frame buffer
extern "C" char *DATA_008804;
extern "C" ModelInstance_t** DATA_008861;
extern "C" char *DATA_008872;
extern "C" char *DATA_008870;
extern "C" char *DATA_008835;
extern "C" char *DATA_008809;
extern "C" char *DATA_008810;
extern "C" char *DATA_009054;

xModel* objectList[500];

unsigned int __stdcall Thread1(void* p)
{
	while(1)
	{
		for(int i=3;i<500;i++) {
			if (objectList[i]) {
				if (objectList[i]->haveFile && objectList[i]->needLoad) {
					objectList[i]->object = LoadModel(objectList[i]->filename, objectList[i]->object);
					objectList[i]->loadSkins();
					objectList[i]->needLoad = false;
					objectList[i]->counter = 500;
				} else if (objectList[i]->Exist()) {
					if (objectList[i]->counter > 0)
						objectList[i]->counter--;
					else if (objectList[i]->counter <= 0)
						objectList[i]->DropModel();
				}
			}
			Sleep(1);
		} 
		Sleep(100);
	}
	return 0;
}

extern void scrInit();

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow)
{
	hInst = hInstance;

	LARGE_INTEGER time;
	QueryPerformanceCounter(&time);
	starttime = (double)time.QuadPart;

	QueryPerformanceFrequency(&time);
	ticks = time.QuadPart;

	_beginthreadex(NULL, 0, &Thread1, NULL, 0, NULL);

	scrInit();

	int res;

	res = asmmain(0, NULL);

	scriptSystem::getSingleton()->FreeInst();

	return res;
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

void inline UpdateDeltaTime()
{
	CurrentTime = timeGetTime();
	DeltaTime = ((float)CurrentTime - (float)LastTime) * 0.001f;
	LastTime = timeGetTime();
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
	if (fullscreen) {
		Win32SetMouseNorm();
		InitFullscreen();
	} else {
		Win32SetMouseNorm();
		InitWindowed();
	}

	if (InitD3D(hWnd)==false) {
		PostQuitMessage(0);
		return;
	}

}

static void RestoreSurfaces()
{
	VideoReset();
}

extern void printTyConsole (WPARAM shift);

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
				if (console)
				{
					printTyConsole (wparam);
				}

			}
		case WM_SYSKEYDOWN:
			return 0L;
	}
	return DefWindowProc (wnd, msg, wparam, lparam);
}


static UCHAR pPalTable[3][64];

char* lTip = NULL;
int lpX, lpY;
int TipColor;
bool showTip = false;

int showLoadingTip (void)
{
	lTip = scriptSystem::getSingleton()->getAsString (2);
	lpX = scriptSystem::getSingleton()->getAsInteger (3);
	lpY = scriptSystem::getSingleton()->getAsInteger (4);
	TipColor = scriptSystem::getSingleton()->getAsInteger (5);
	showTip = true;

	return 0;
}

void VideoInit()
{
	scriptSystem* scr = scriptSystem::getSingleton();

	scr->newClass();
	scr->newChildFunction ("showTip", showLoadingTip);
	scr->registerClass ("lTip");

	scr->doPreLaunchScripts();

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

    ShowWindow(hWnd, SW_SHOWNORMAL);
    UpdateWindow(hWnd);

	VideoReset();

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
CXFileEntity* panelObj = NULL;
CXFileEntity* sphereObj = NULL;
CXFileEntity* atmoObj = NULL;
CXFileEntity* nebulaObj = NULL;
CXFileEntity* starsObj = NULL;
LPDIRECT3DTEXTURE9 aviTex;
LPDIRECT3DTEXTURE9 panelTex, battlepanel, navigatepanel, voidbutton;
LPDIRECT3DTEXTURE9 loadingTex, modelTex, texTex, effectTex;
LPDIRECT3DTEXTURE9 panelIcons[4];
LPDIRECT3DTEXTURE9 permTexture;
LPDIRECT3DTEXTURE9 permTexture2d;
int panelnum=0;

ID3DXEffect* effectList[500];
ID3DXEffect* effectPlanet;
ID3DXEffect* effectCurrent;


// permutation table
static int perm[] = { 151,160,137,91,90,15,
131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
};

int perm2d(int i) 
{ 
	return perm[i % 256]; 
} 

void GeneratePermTexture() 
{ 
	HRESULT res;
	res = D3DXCreateTexture(renderSystem->GetDevice(), 256, 1, 1, 0, D3DFMT_L8, D3DPOOL_MANAGED, &permTexture);
	assert(res == D3D_OK);

	byte data[256];

	 for (int x = 0; x < 256; x++) 
			data[x] = (byte)(perm[x]);

	D3DLOCKED_RECT sLockedRect;
	res = permTexture->LockRect(0, &sLockedRect, 0, 0);
	assert(res == D3D_OK);

	memcpy(sLockedRect.pBits, data, 256);

	permTexture->UnlockRect(0);
}

void GeneratePermTexture2d() 
{ 
	HRESULT res;
	res = D3DXCreateTexture(renderSystem->GetDevice(), 256, 256, 16, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &permTexture2d);
	assert(res == D3D_OK);

	unsigned int data[256*256];

	D3DLOCKED_RECT sLockedRect; 

	for (int l = 0;; l++)
	{
		res = permTexture2d->LockRect(l, &sLockedRect, 0, 0);
		if (res != D3D_OK) break;

		int w = 0;
		int h = 0;
		for (int x = 0; x < 256; x+= 1 << l) 
		{ 
			w = 0;
			for (int y = 0; y < 256; y+= 1 << l) 
			{ 
				int A = perm2d(x) + y; 
				int AA = perm2d(A); 
				int AB = perm2d(A + 1); 
				int B = perm2d(x + 1) + y; 
				int BA = perm2d(B); 
				int BB = perm2d(B + 1); 
				data[w + (h * (256 >> l))] = D3DCOLOR_ARGB((byte)(AA)>>(l>>1), 
														   (byte)(AB)>>(l>>1), 
														   (byte)(BA)>>(l>>1), 
														   (byte)(BB)>>(l>>1));
				w++;
			}
			h++;
		} 

		memcpy(sLockedRect.pBits, data, (256 >> l) * (256 >> l) * 4);
		permTexture2d->UnlockRect(l);
	}
}

extern bool exportb[500];

void loadTextures() {
	char buf[1000];

	GeneratePermTexture();
	GeneratePermTexture2d();

	sprintf_s(buf,"textures/panelicons/panel.png");
	if (FAILED(FFED3DCreateTextureFromFileRT(renderSystem->GetDevice(), buf, &panelTex))) {
		panelTex=NULL;
	}	
	sprintf_s(buf,"textures/panelicons/battlepanel.png");
	if (FAILED(FFED3DCreateTextureFromFileRT(renderSystem->GetDevice(), buf, &battlepanel))) {
		battlepanel=NULL;
	}	
	sprintf_s(buf,"textures/panelicons/navigatepanel.png");
	if (FAILED(FFED3DCreateTextureFromFileRT(renderSystem->GetDevice(), buf, &navigatepanel))) {
		navigatepanel=NULL;
	}	
	sprintf_s(buf,"textures/panelicons/voidbutton.png");
	if (FAILED(FFED3DCreateTextureFromFileRT(renderSystem->GetDevice(), buf, &voidbutton))) {
		voidbutton=NULL;
	}	

	sprintf_s(buf,"textures/panelicons/icons0.png");
	if (FAILED(FFED3DCreateTextureFromFile(renderSystem->GetDevice(), buf, &panelIcons[0])))
		panelIcons[0]=NULL;
	sprintf_s(buf,"textures/panelicons/icons1.png");
	if (FAILED(FFED3DCreateTextureFromFile(renderSystem->GetDevice(), buf, &panelIcons[1])))
		panelIcons[1]=NULL;
	sprintf_s(buf,"textures/panelicons/icons2.png");
	if (FAILED(FFED3DCreateTextureFromFile(renderSystem->GetDevice(), buf, &panelIcons[2])))
		panelIcons[2]=NULL;
	sprintf_s(buf,"textures/panelicons/icons3.png");
	if (FAILED(FFED3DCreateTextureFromFile(renderSystem->GetDevice(), buf, &panelIcons[3])))
		panelIcons[3]=NULL;


	for (int i=0;i<1000;i++) {
		sprintf_s(buf,"textures/tex%i.png",i);
		if (FAILED(FFED3DCreateTextureFromFile(renderSystem->GetDevice(), buf, &textures[i]))) {
			sprintf_s(buf,"textures/tex%i.dds",i);
			if (FAILED(FFED3DCreateTextureFromFile(renderSystem->GetDevice(), buf, &textures[i]))) {
				sprintf_s(buf,"textures/tex%i.tga",i);
				if (FAILED(D3DXCreateTextureFromFile(renderSystem->GetDevice(), buf, &textures[i]))) {
					textures[i]=NULL;
				}
			}
		}
	}
	for (int i=0;i<6;i++) {
		sprintf_s(buf,"textures/skybox%i.tga",i);
		if (FAILED(D3DXCreateTextureFromFile(renderSystem->GetDevice(), buf, &skyboxtex[i]))) {
			skyboxtex[i]=NULL;
		}
	}
	sprintf_s(buf,"textures/stars.tga");
	if (FAILED(D3DXCreateTextureFromFile(renderSystem->GetDevice(), buf, &starstex))) {
		starstex=NULL;
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

void loadEffects() {
	char buf[1000];
	LPD3DXBUFFER err=NULL;
	char* data=NULL;

	sprintf_s(buf,"models/planet.fx");
	if ( FAILED(D3DXCreateEffectFromFileA(renderSystem->GetDevice(), (LPCSTR)buf, 0, 0, 0, 0, &effectPlanet, &err))) {
		effectPlanet=NULL;
		if (err) {
			char *tempString = (char*)err->GetBufferPointer();
			if (tempString != NULL) {
				printf("Modifier shader: %s",tempString);
				MessageBox(0, tempString, 0, 0);
			}
		}
	}

	for(int i=3;i<500;i++) {
		if (Planet(i)) {
			effectList[i] = effectPlanet;
		} else {
			sprintf_s(buf,"models/%i/effect.fx",i);
			if ( FAILED(D3DXCreateEffectFromFileA(renderSystem->GetDevice(), (LPCSTR)buf, 0, 0, 0, 0, &effectList[i], &err))) {
				effectList[i]=NULL;
				if (err) {
					char *tempString = (char*)err->GetBufferPointer();
					if (tempString != NULL) {
						printf("Modifier shader: %s",tempString);
						MessageBox(0, tempString, 0, 0);
					}
				}
			}
		}
	}
}

xModel* initXModel(const std::string &filename, int id)
{
	xModel* model = new xModel(id);
	if (CUtility::DoesFileExist(CUtility::GetTheCurrentDirectory()+filename)) {
		//model->object = new CXFileEntity();
		model->filename = filename;
		model->haveFile = true;
	}
	return model;
}

CXFileEntity* LoadModel(const std::string &filename, CXFileEntity* obj)
{
	obj = NULL;
	if (CUtility::DoesFileExist(CUtility::GetTheCurrentDirectory()+filename)) {
		if (obj == NULL)
			obj = new CXFileEntity();
		if (!obj->LoadXFile(CUtility::GetTheCurrentDirectory()+filename,0)) {
			SAFE_DELETE(obj);
		}
	}
	return obj;
}

void loadModels() 
{
	char buf[1000];
	CIniFile file;

	panelObj = LoadModel("/models/panel.x", panelObj);
	sphereObj = LoadModel("/models/sphere.x", sphereObj);
	atmoObj = LoadModel("/models/atmo.x", atmoObj);
	starsObj = LoadModel("/models/stars.x", starsObj);
	nebulaObj = LoadModel("/models/nebula.x", nebulaObj);

	for(int i=0;i<500;i++) {
		sprintf_s(buf,"/models/%i/model.x",i);
		objectList[i] = initXModel(buf,i);
		if (objectList[i]) {			
			sprintf_s(buf,"models\\%i\\tris.ini",i);
			file.SetFileName (buf);
			file.GetIntValue (objectList[i]->config.skip,"MODEL", "skip");
			file.GetIntValue (objectList[i]->config.notdrawtext,"MODEL", "notdrawtext");
			file.GetIntValue (objectList[i]->config.notdrawsubmodels,"MODEL", "notdrawsubmodels");
			file.GetFloatValue (objectList[i]->config.scale,"MODEL", "scale");
			file.GetFloatValue (objectList[i]->config.scale_x,"MODEL", "scale_x");
			file.GetFloatValue (objectList[i]->config.scale_y,"MODEL", "scale_y");
			file.GetFloatValue (objectList[i]->config.scale_z,"MODEL", "scale_z");
			objectList[i]->config.scale = objectList[i]->config.scale > 0 ? objectList[i]->config.scale : 1.0f;
			objectList[i]->config.scale_x = objectList[i]->config.scale_x > 0 ? objectList[i]->config.scale_x : 1.0f;
			objectList[i]->config.scale_y = objectList[i]->config.scale_y > 0 ? objectList[i]->config.scale_y : 1.0f;
			objectList[i]->config.scale_z = objectList[i]->config.scale_z > 0 ? objectList[i]->config.scale_z : 1.0f;
			file.GetFloatValue (objectList[i]->config.offset_x,"MODEL", "offset_x");
			file.GetFloatValue (objectList[i]->config.offset_y,"MODEL", "offset_y");
			file.GetFloatValue (objectList[i]->config.offset_z,"MODEL", "offset_z");
			file.GetIntValue (objectList[i]->config.cullmode,"MODEL", "cullmode");
		} else {
			sprintf_s(buf,"models\\%i\\tris.ini",i);
			file.SetFileName (buf);
			file.GetIntValue (objectList[i]->config.skip,"MODEL", "skip");
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

	//Loading Screen Pack
	D3DXMATRIX *lScale = new D3DXMATRIX;
	float sx = curwidth/1024.0f;
	float sy = curheight/1024.0f;
	D3DXMatrixScaling (lScale, sx, sy, 1);
	
	D3DXVECTOR3 *elPos = new D3DXVECTOR3 (curwidth/2/sx-128, curheight/2/sy-32, 0);
	D3DXVECTOR3 *tipPos = new D3DXVECTOR3 (0, 0, 0);
	
	char buf[1000];
	sprintf_s(buf,"textures/loading/Loadscreen.png");
	if (FAILED(FFED3DCreateTextureFromFileRT(renderSystem->GetDevice(), buf, &loadingTex))) {
		loadingTex=NULL;
	}

	sprintf_s(buf,"textures/loading/loading_models.png");
	if (FAILED(FFED3DCreateTextureFromFileRT(renderSystem->GetDevice(), buf, &modelTex))) {
		modelTex=NULL;
	}

	sprintf_s(buf,"textures/loading/loading_textures.png");
	if (FAILED(FFED3DCreateTextureFromFileRT(renderSystem->GetDevice(), buf, &texTex))) {
		texTex=NULL;
	}

	sprintf_s(buf,"textures/loading/loading_effects.png");
	if (FAILED(FFED3DCreateTextureFromFileRT(renderSystem->GetDevice(), buf, &effectTex))) {
		effectTex=NULL;
	}

	CreateLocalFont();
	D3DXCreateSprite(renderSystem->GetDevice(),&textSprite);
	RECT tRect;
	tRect.left = lpX;
	tRect.top = lpY;
	tRect.right = 0;
	tRect.bottom = 0;

	if (renderSystem->GetDevice()!=NULL)
		renderSystem->GetDevice()->Clear(0,NULL,D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,D3DCOLOR_XRGB(0,0,0),1.0f,0);	

	renderSystem->BeginScene();    
	textSprite->Begin(D3DXSPRITE_ALPHABLEND);
	textSprite->SetTransform (lScale);
	textSprite->Draw (loadingTex, NULL, NULL, NULL, 0xffffffff);
	textSprite->Draw (modelTex, NULL, NULL, elPos, 0xffffffff);
	if (showTip) {m_tFont->DrawTextA(textSprite,lTip, -1, &tRect, DT_NOCLIP, TipColor);}
	//DrawText("Loading models...", curwidth/2-100, curheight/2, D3DCOLOR_XRGB(255,255,255));
	textSprite->End();
	renderSystem->EndScene();

	loadModels();

	if (renderSystem->GetDevice()!=NULL)
		renderSystem->GetDevice()->Clear(0,NULL,D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,D3DCOLOR_XRGB(0,0,0),1.0f,0);	

	renderSystem->BeginScene();    
	textSprite->Begin(D3DXSPRITE_ALPHABLEND);
	textSprite->SetTransform (lScale);
	textSprite->Draw (loadingTex, NULL, NULL, NULL, 0xffffffff);
	textSprite->Draw (texTex, NULL, NULL, elPos, 0xffffffff);
	if (showTip) {m_tFont->DrawTextA(textSprite,lTip, -1, &tRect, DT_NOCLIP, TipColor);}
	//DrawText("Loading textures...", curwidth/2-100, curheight/2, D3DCOLOR_XRGB(255,255,255));
	textSprite->End();
	renderSystem->EndScene();  

	loadTextures();

	if (renderSystem->GetDevice()!=NULL)
		renderSystem->GetDevice()->Clear(0,NULL,D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,D3DCOLOR_XRGB(0,0,0),1.0f,0);	

	renderSystem->BeginScene();    
	textSprite->Begin(D3DXSPRITE_ALPHABLEND);
	textSprite->SetTransform (lScale);
	textSprite->Draw (loadingTex, NULL, NULL, NULL, 0xffffffff);
	textSprite->Draw (effectTex, NULL, NULL, elPos, 0xffffffff);
	if (showTip) {m_tFont->DrawTextA(textSprite,lTip, -1, &tRect, DT_NOCLIP, TipColor);}
	//DrawText("Loading effects...", curwidth/2-100, curheight/2, D3DCOLOR_XRGB(255,255,255));
	textSprite->End();
	renderSystem->EndScene();  

	loadEffects();

	checkExport();

	if ((curheight-curwidth/1.6f)==0)
		aspectfix=0;

	pl.x = -1.35f;
	pl.y = -0.64f;
	pl.z = 3.91f;
	plsize = 0.18f;

	loadingTex = NULL;
	modelTex = NULL;
	texTex = NULL;
	effectTex = NULL;
	//m_tFont = NULL;

	if (renderSystem->GetDevice()!=NULL)
		renderSystem->GetDevice()->Clear(0,NULL,D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,D3DCOLOR_XRGB(0,0,0),1.0f,0);

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
    
    D3DXCreateFont(renderSystem->GetDevice(), nHeight,0,nWeight,1,false,DEFAULT_CHARSET,0,0,0,"Arial", &m_pFont);
	D3DXCreateFont (renderSystem->GetDevice(), 22, 0, FW_NORMAL, 1, false, DEFAULT_CHARSET, 0,0,0,"Arial", &m_tFont);

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

void DrawConText(LPSTR pText, int x, int y, D3DCOLOR rgbFontColour)
{
	RECT Rect;

    Rect.left = x;
    Rect.top = y;
    Rect.right = 0;
    Rect.bottom = 0;

    //m_pFont->Begin();
    m_tFont->DrawTextA(textSprite,pText, -1, &Rect, DT_CALCRECT, 0); //Calculate the size of the rect needed
    m_tFont->DrawTextA(textSprite,pText, -1, &Rect, 0, rgbFontColour); //Draw the text
    //m_pFont->End();
}

D3DCOLORVALUE rgbaDiffuse0 = {0.0f, 0.0f, 0.0f, 0.0f,};
D3DCOLORVALUE rgbaDiffuse1 = {1.0f, 1.0f, 1.0f, 1.0f,};
D3DCOLORVALUE rgbaDiffuse2 = {0.75f, 0.75f, 0.75f, 0.5f,};
D3DCOLORVALUE rgbaDiffuse3 = {0.5f, 0.5f, 0.5f, 0.0f,};
D3DCOLORVALUE rgbaDiffuse4 = {0.25f, 0.25f, 0.25f, 0.25f,};
D3DCOLORVALUE rgbaDiffuse11 = {2.0f, 2.0f, 2.0f, 1.0f,};
D3DCOLORVALUE rgbaAmbient0 = {0.0f, 0.0f, 0.0f, 0.0f,};
D3DCOLORVALUE rgbaAmbient1 = {1.0f, 1.0f, 1.0f, 1.0f,};
D3DCOLORVALUE rgbaAmbient2 = {1.0f, 1.0f, 1.0f, 0.5f,};
D3DCOLORVALUE rgbaAmbient3 = {0.25f, 0.25f, 0.25f, 0.5f,};
D3DCOLORVALUE rgbaSpecular0 = {0.0f, 0.0f, 0.0f, 0.0f,};
D3DCOLORVALUE rgbaSpecular1 = {0.774597f, 0.774597f, 0.774597f, 1.0f,};
D3DCOLORVALUE rgbaSpecular2 = {1.0f, 1.0f, 1.0f, 0.5f,};
D3DCOLORVALUE rgbaSpecular3 = {0.5f, 0.5f, 0.5f, 0.0f,};
D3DCOLORVALUE rgbaEmissive0 = {0.0f, 0.0f, 0.0f, 0.0f,};
D3DCOLORVALUE rgbaEmissive1 = {1.0f, 1.0f, 1.0f, 0.0f,};

extern char ambientR;
extern char ambientG;
extern char ambientB;

void selectMaterial(int type, float cR, float cG, float cB) {
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
		//renderSystem->SetRenderState( D3DRS_ALPHABLENDENABLE, false);
		rgbaEmissive.r=cR;
		rgbaEmissive.g=cG;
		rgbaEmissive.b=cB;
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
		rgbaDiffuse.r=c;
		rgbaDiffuse.g=c;
		rgbaDiffuse.b=c;
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
		//renderSystem->SetRenderState( D3DRS_ALPHABLENDENABLE, true);
		renderSystem->SetRenderState(D3DRS_AMBIENT, D3DCOLOR_XRGB(255, 255, 255));
		m_matMaterial.Diffuse = rgbaDiffuse0; 
		m_matMaterial.Ambient = rgbaAmbient1; 
		m_matMaterial.Specular = rgbaSpecular0; 
	    m_matMaterial.Emissive = rgbaEmissive0;
		break;
	case ATMO:
		//renderSystem->SetRenderState( D3DRS_ALPHABLENDENABLE, true);
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
		//renderSystem->SetRenderState( D3DRS_ALPHABLENDENABLE, false);
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
		renderSystem->SetRenderState(D3DRS_AMBIENT, D3DCOLOR_XRGB(64, 64, 64));
		m_matMaterial.Diffuse = rgbaDiffuse2; 
		m_matMaterial.Ambient = rgbaAmbient2; 
		m_matMaterial.Specular = rgbaSpecular2; 
		m_matMaterial.Emissive = rgbaEmissive0;
		break;
	case ATMOSTAR:
		//renderSystem->SetRenderState( D3DRS_ALPHABLENDENABLE, true);
		renderSystem->SetRenderState(D3DRS_SPECULARENABLE, false);
		renderSystem->SetRenderState (D3DRS_SRCBLEND, D3DBLEND_ONE);
		renderSystem->SetRenderState (D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA);
		renderSystem->SetRenderState(D3DRS_AMBIENT, D3DCOLOR_XRGB(128, 128, 128));
		rgbaEmissive.r=cR/10;
		rgbaEmissive.g=cG/10;
		rgbaEmissive.b=cB/10;
		rgbaEmissive.a=0.0f;
		m_matMaterial.Diffuse = rgbaDiffuse0; 
		m_matMaterial.Ambient = rgbaAmbient0; 
		m_matMaterial.Specular = rgbaSpecular0; 
	    m_matMaterial.Emissive = rgbaEmissive;
		break;
	case SPLINE:
		rgbaDiffuse.r=cR;
		rgbaDiffuse.g=cG;
		rgbaDiffuse.b=cB;
		rgbaDiffuse.a=1.0f;

		m_matMaterial.Diffuse = rgbaDiffuse; 
		m_matMaterial.Ambient = rgbaDiffuse; 
		m_matMaterial.Specular = rgbaDiffuse; 
	    m_matMaterial.Emissive = rgbaEmissive0;
		break;
	default:
		//renderSystem->SetRenderState( D3DRS_ALPHABLENDENABLE, false);
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

void setupLight(D3DXVECTOR3 lightPos, float r, float g, float b) 
{

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

D3DXMATRIX matProj;
D3DXMATRIX matView;

void doPerspectiveNear()
{
	D3DXMatrixLookAtLH(&matView, &D3DXVECTOR3(0.0f, corrector, 0.0f), //Camera Position
		&D3DXVECTOR3(0.0f, corrector, 1.0f), //Look At Position
		&D3DXVECTOR3(0.0f, 1.0f, 0.0f)); //Up Direction
	renderSystem->GetDevice()->SetTransform(D3DTS_VIEW, &matView);

	float ratio, fov;

	//ratio = 2.012f;
	//fov = 0.6f;
	ratio = 2.0253f;
	//fov = 0.5972f;
	fov = 0.5972f;
	//fov = 0.8726f;
	
	// ближнюю плоскость отсечени€ ставить меньше нельз€, будут проблемы с 
	// буфером глубины. ¬озникает проблема с лазером, он начинает рисоватьс€ на 
	// некотором рассто€нии.
	D3DXMatrixPerspectiveFovLH(&matProj, fov, ratio, 0.1f, 300000);
	renderSystem->GetDevice()->SetTransform(D3DTS_PROJECTION, &matProj);
}

void doPerspectiveFar()
{
	D3DXMatrixLookAtLH(&matView, &D3DXVECTOR3(0.0f, corrector, 0.0f), //Camera Position
		&D3DXVECTOR3(0.0f, corrector, 1.0f), //Look At Position
		&D3DXVECTOR3(0.0f, 1.0f, 0.0f)); //Up Direction
	renderSystem->GetDevice()->SetTransform(D3DTS_VIEW, &matView);

	float ratio, fov;

	//ratio = 2.012f;
	//fov = 0.6f;
	ratio = 2.0253f;
	//fov = 0.5972f;
	fov = 0.5972f;
	//fov = 0.8726f;
	
	// ближнюю плоскость отсечени€ ставить меньше нельз€, будут проблемы с 
	// буфером глубины. ¬озникает проблема с лазером, он начинает рисоватьс€ на 
	// некотором рассто€нии.
	D3DXMatrixPerspectiveFovLH(&matProj, fov, ratio, 1.0f, 10e6);
	renderSystem->GetDevice()->SetTransform(D3DTS_PROJECTION, &matProj);
}
extern DWORD fullAmbientColor;
extern bool inAtmo;
extern float atmoW;
extern float atmoR;

//void SetupPixelFog(DWORD Color, DWORD Mode) 
//{ 
//    float Start  = 1.0f;    // For linear mode 
//    float End    = 400000.0f; 
//    float Density = 0.66f;  // For exponential modes
//
//	if (!inAtmo) {
//		renderSystem->GetDevice()->SetRenderState(D3DRS_FOGENABLE, FALSE);
//		return;
//	}
//
//	End = atmoR*30*atmoW;
//	//Density = atmoW;
//    // Enable fog blending. 
//    renderSystem->GetDevice()->SetRenderState(D3DRS_FOGENABLE, TRUE);
//
//    // Set the fog color. 
//    renderSystem->GetDevice()->SetRenderState(D3DRS_FOGCOLOR, (Color+fullAmbientColor)/2);
//
//    // Set fog parameters. 
//    if( Mode == D3DFOG_LINEAR ) 
//    { 
//        renderSystem->GetDevice()->SetRenderState(D3DRS_FOGTABLEMODE, Mode); 
//        renderSystem->GetDevice()->SetRenderState(D3DRS_FOGSTART, *(DWORD *)(&Start)); 
//        renderSystem->GetDevice()->SetRenderState(D3DRS_FOGEND,  *(DWORD *)(&End)); 
//    } 
//    else 
//    { 
//        renderSystem->GetDevice()->SetRenderState(D3DRS_FOGTABLEMODE, Mode); 
//        renderSystem->GetDevice()->SetRenderState(D3DRS_FOGDENSITY, *(DWORD *)(&Density)); 
//    } 
//}

// Andis: ¬ дальнейшем ортогональна€ матрица не понадобитс€, спрайты будут выводитьс€
// другим способом.
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
int currModIndex;

inline DWORD FtoDW(FLOAT f) {return *((DWORD*)&f);}

float tn;

void drawModelPrimitives(int startVert, int endVert) 
{
	int zwritemem;
	//renderSystem->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	//renderSystem->GetDevice()->SetVertexShader(D3DFVF_CUSTOMVERTEX);
	//renderSystem->GetDevice()->SetFVF( D3DFVF_CUSTOMVERTEX );
	renderSystem->SetVertexDeclaration(vertexBuffer);
	renderSystem->SetStreamSource(vertexBuffer);
	DWORD a;
	unsigned int pass;
//	zwritemem = renderSystem->GetRenderState(D3DRS_ZWRITEENABLE);
//	renderSystem->SetRenderState( D3DRS_ALPHATESTENABLE, false);

	for(int i=startVert;i<endVert;i++) {
//		renderSystem->SetRenderState(D3DRS_ZWRITEENABLE, zwritemem);
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
		if (currModIndex==166) {
			int a=0;
		}
		// textures
		if (textures[vertexType[i].textNum] != NULL) {
			//renderSystem->SetRenderState( D3DRS_ALPHABLENDENABLE, true);
			renderSystem->GetDevice()->SetTexture(0, textures[vertexType[i].textNum]);
		} else {
			//renderSystem->SetRenderState( D3DRS_ALPHABLENDENABLE, false);
			if (renderAvi && vertexType[i].textNum==-100) {
				renderSystem->GetDevice()->SetTexture(0, aviTex);
			} else
				renderSystem->GetDevice()->SetTexture(0, NULL);
		}

		if (vertexType[i].doLight==true || (textures[vertexType[i].textNum] != NULL && currModIndex!=169 && currModIndex!=170)) {
			renderSystem->SetRenderState(D3DRS_LIGHTING, TRUE);
		} else {
			renderSystem->SetRenderState(D3DRS_LIGHTING, FALSE);
			//selectMaterial(GALAXY, 0, 0, 0);
			//renderSystem->GetDevice()->SetMaterial(&m_matMaterial);
		}

		if (doTransp==false && vertexType[i].transparent==true) {

		} else if (effectList[currModIndex]) {
			tn = vertexType[i].textNum;
			effectList[currModIndex]->SetTexture("tex",textures[vertexType[i].textNum]);
			effectList[currModIndex]->SetValue("tangent", &vertexType[i].tangent, D3DX_DEFAULT);
			if (Planet(currModIndex)) {
				effectList[currModIndex]->SetTexture("wave1",textures[722]);
				effectList[currModIndex]->SetTexture("wave2",textures[723]);
				effectList[currModIndex]->SetTexture("permTexture",permTexture);
				effectList[currModIndex]->SetTexture("permTexture2d",permTexture2d);
			}
			if (vertexType[i].textNum==720) // water
				effectList[currModIndex]->BeginPass(1);
			else if (vertexType[i].textNum==699 || vertexType[i].textNum==700) // atmo
				effectList[currModIndex]->BeginPass(3);
			else if (vertexType[i].textNum == 94) // noise land
				effectList[currModIndex]->BeginPass(2);
			else {
				effectList[currModIndex]->BeginPass(0);
			}
		}

		// primitives
		if (vertexType[i].type==GL_LINES) {
			if (doTransp==false)
				renderSystem->GetDevice()->DrawPrimitive(D3DPT_LINELIST, i, vertexType[i].amount/2);
			i+=vertexType[i].amount-1;
		}
		else if (vertexType[i].type==GL_LINE_STRIP) {
			if (doTransp==false)
				renderSystem->GetDevice()->DrawPrimitive(D3DPT_LINESTRIP, i, vertexType[i].amount-1);
			i+=vertexType[i].amount-1;
		}
		else if (vertexType[i].type==GL_LINE_LOOP) {
			if (doTransp==false)
				renderSystem->GetDevice()->DrawPrimitive(D3DPT_LINESTRIP, i, vertexType[i].amount-1);
			i+=vertexType[i].amount-1;
		}
		else if (vertexType[i].type==GL_TRIANGLES) {
			if ((currModIndex>=121 && currModIndex<=131) || currModIndex==449)
				renderSystem->SetRenderState(D3DRS_SPECULARENABLE, vertexType[i].specular);
			//renderSystem->SetRenderState(D3DRS_ZWRITEENABLE, vertexType[i].doLight);
			if (doTransp==false)
				renderSystem->GetDevice()->DrawPrimitive(D3DPT_TRIANGLELIST, i, vertexType[i].amount/3);
			i+=vertexType[i].amount-1;
		}
		else if (vertexType[i].type==GL_TRIANGLE_STRIP) {
			// прозрачные полигоны будем рисовать потом
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
		}
		else if (vertexType[i].type==GL_TRIANGLE_FAN) {
			
			// прозрачные полигоны будем рисовать потом
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
		}
		else if (vertexType[i].type==GL_POINTS) { // лампочки
			
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
			
		}
		else if (vertexType[i].type==GL_POINTS2) { // далекие планеты/звезды
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
		}
		if (effectList[currModIndex]) {
			effectList[currModIndex]->EndPass();
		}
	}
}

void BattleMode(bool mode)
{
	D3DXVECTOR3 spos;
	RECT rectsize;
	rectsize.left=0;
	rectsize.top=0;
	rectsize.right=512;
	rectsize.bottom=256;
	spos.x=73;
	spos.y=239;
	spos.z=0;
	if (mode)
		textSprite->Draw(battlepanel,&rectsize,NULL,&spos,0xFFFFFFFF);
	else
		textSprite->Draw(navigatepanel,&rectsize,NULL,&spos,0xFFFFFFFF);
}

void VoidButton(int x, int y)
{
	D3DXVECTOR3 spos;
	RECT rectsize;
	rectsize.left=0;
	rectsize.top=0;
	rectsize.right=50;
	rectsize.bottom=50;
	spos.x=x;
	spos.y=y;
	spos.z=0;
	textSprite->Draw(voidbutton,&rectsize,NULL,&spos,0xFFFFFFFF);
}

extern void DrawClipSprite(int index, int x, int y, int z, int h, int w, float NPx1, float NPx2, float NPy1, float NPy2);

extern "C" void ResetAviFrame(void)
{
	aviTex=NULL;
	renderAvi=false;
}

extern "C" void FillAviFrame(void)
{
	D3DLOCKED_RECT lockrect_out;

	if (aviTex==NULL) {
		D3DXCreateTexture(renderSystem->GetDevice(),  120,  120,  0,  0,  D3DFMT_A8R8G8B8,  D3DPOOL_MANAGED,  &aviTex);
	}
	aviTex->LockRect(0, &lockrect_out, 0, 0);
	ULONG *Pixel = (ULONG *)lockrect_out.pBits;
	int addr=14400;

	ULONG al;
	int dJump = (lockrect_out.Pitch>>2) - 120;
	Pixel+=dJump;
	for(int i=0;i<120;i++){
		for(int j=0;j<120;j++){
			*Pixel=0;
			al = DATA_008604[DATA_008605[addr]];
			if (al==0 /*|| i & 1*/)
				*Pixel=0x00000000;
			else {
				*Pixel=pWinPal32[al] | 0xFF000000;
			}
			addr--;
			Pixel++;
		}
		Pixel+=dJump;
	}

	aviTex->UnlockRect(0);
	renderAvi=true;
	int asp=120*(curwidth/320);
	int x=(-curwidth/2);
	int y=(curheight/2)-asp;
	int x2=(-curwidth/2)+asp;
	int y2=(curheight/2);
	int xadd=1*(curwidth/320);
	int yadd=33*(curwidth/320);
	DrawClipSprite(-100,x+xadd,y-yadd,0,y2-yadd,x2+xadd,0,1,0,1);
}

bool panellight[20];

void PreparePanel(void)
{
	LPDIRECT3DSURFACE9 texSurface, oldSurface, zSurface;
	D3DXMATRIX mat;
	ModelInstance_t *instance;
	Model_t *model;

	instance = *(ModelInstance_t**)((int)DATA_008804+0xc8);
	if (instance==NULL)
		return;

	model = GetModel(instance->model_num);

	if (model->Shipdef_ptr==NULL)
		return;

	renderSystem->SetRenderState(D3DRS_ZENABLE, false);
	renderSystem->SetRenderState(D3DRS_ZWRITEENABLE, false);

	renderSystem->GetDevice()->GetDepthStencilSurface(&zSurface);
	renderSystem->GetDevice()->GetRenderTarget(0, &oldSurface);
	panelTex->GetSurfaceLevel(0,&texSurface);
	renderSystem->GetDevice()->SetRenderTarget(0, texSurface);
	renderSystem->GetDevice()->SetDepthStencilSurface(0);

	doOrtoRT(1024, 1024);
	D3DVIEWPORT9 viewport;

	viewport.X = 0;
	viewport.Y = 0;
	viewport.Width = 1024-1;
	viewport.Height = 1024-1; 
	viewport.MinZ = 0; 
	viewport.MaxZ = 1; 
	renderSystem->GetDevice()->SetViewport( &viewport ); 	

	D3DXVECTOR3 spos;
	RECT rectsize;
	rectsize.left=0;
	rectsize.top=0;
	rectsize.right=773;
	rectsize.bottom=51;
	spos.x=34;
	spos.y=595;
	spos.z=0;
	textSprite->Begin(D3DXSPRITE_ALPHABLEND);
	D3DXMatrixIdentity(&mat);
	textSprite->SetTransform(&mat);
	textSprite->Draw(panelIcons[panelnum],&rectsize,NULL,&spos,0xFFFFFFFF);

	if (panelnum == 0) {
		if (panellight[5]==true)
			BattleMode(true);
		else
			BattleMode(false);

		// turret view
		if (model->Shipdef_ptr->Lasers < 3)
			VoidButton(34+(7*52), 595);
		// missile viewer
		if ((instance->globalvars.equip & EQUIP_MISSILE_VIEWER) == 0)
			VoidButton(34+(9*52), 595);
		// combat computer
		if ((instance->globalvars.equip & EQUIP_COMBAT_COMP) == 0)
			VoidButton(34+(10*52), 595);
		// escape capsule
		if ((instance->globalvars.equip & EQUIP_ESCAPE_CAPSULE) == 0 && (instance->globalvars.equip & EQUIP_STOWMASTER) == 0)
			VoidButton(34+(11*52), 595);
		// navigation computer
		if ((instance->globalvars.equip & EQUIP_NAV_COMP) == 0)
			VoidButton(34+(12*52), 595);
	} else if (panelnum == 1) {
		// crew roster
		if (model->Shipdef_ptr->Crew == 1)
			VoidButton(34+(7*52), 595);	
		// passenger roster
		if (*(int*)((int)DATA_008804+0x0124) == 0) // cabins used
			VoidButton(34+(8*52), 595);	
		// missions roster
		if (*(int*)((int)DATA_008804+0x4ef0) == 0) // missions in list
			VoidButton(34+(10*52), 595);	
		// MB4 roster
		if (*(int*)((int)DATA_008804+0x511a) == 0) // MB4 count
			VoidButton(34+(11*52), 595);	
	}

	textSprite->End();

	renderSystem->GetDevice()->SetRenderTarget(0, oldSurface);
	renderSystem->GetDevice()->SetDepthStencilSurface(zSurface);
}

bool t_work = false;

extern float dist;
extern float radius;
D3DXVECTOR3 playerLightPos;

extern int incabin;
extern bool clearBeforeRender;

//extern int lastPlanetLod;

extern D3DXVECTOR4 GetStarLightColor();

void Render()
{
	D3DXMATRIX mdscale, mdworld, mdrotate;
	D3DXMATRIX mat;
	float scale, scale_x, scale_y, scale_z;
	float dtime;
	float lG;

	UpdateDeltaTime();

	if(renderSystem->GetDevice()==NULL) {
		vertexNum=0;
		textNum=0;
		modelNum=0;
		maxsprite=0;
		sprites.clear();
		clearBeforeRender=false;
		return;
	}

	int hr = renderSystem->GetDevice()->TestCooperativeLevel();

	if (hr == D3DERR_DEVICENOTRESET)
	{
		RestoreSurfaces();
		vertexNum=0;
		textNum=0;
		modelNum=0;
		maxsprite=0;
		sprites.clear();
		clearBeforeRender=false;
		return;
	}

    if(hr != 0)
    {
		vertexNum=0;
		textNum=0;
		modelNum=0;
		maxsprite=0;
		sprites.clear();
		clearBeforeRender=false;
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
	//sprintf(cBuff,"FFED3D %i", lastPlanetLod);
	SetWindowText(hWnd,cBuff);

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

    renderSystem->BeginScene();

	PreparePanel();

	//SetupPixelFog(0xffffff, D3DFOG_LINEAR);

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

	renderSystem->GetDevice()->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	renderSystem->GetDevice()->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
	renderSystem->GetDevice()->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR );

	renderSystem->SetRenderState(D3DRS_POINTSPRITEENABLE, TRUE); 
	renderSystem->SetRenderState(D3DRS_POINTSCALEENABLE, TRUE); 

	renderSystem->SetRenderState( D3DRS_ALPHATESTENABLE, false);
    //renderSystem->GetDevice()->SetRenderState(D3DRS_ALPHAREF, 255);
    //renderSystem->GetDevice()->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE); 
    //renderSystem->GetDevice()->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_EQUAL);

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

	D3DXVECTOR4 starColor = GetStarLightColor();

	//renderSystem->SetRenderState(D3DRS_LIGHTING, FALSE);
	// nebula
	if (nebulaObj && (*(unsigned char*)(DATA_008804+0x20)!=0 || *(unsigned char*)(DATA_008804+0x21)!=0 || *(unsigned char*)(DATA_008804+0x22)!=0))
	for(int m=0;m<modelNum;m++) {
		if (modelList[m].index==315 || 
			modelList[m].index==444) {
			if (modelList[m].material!=NOTDRAW) {

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
				nebulaObj->FrameMove(0, 0,&mdworld);

				renderSystem->GetDevice()->SetTexture(0, skyboxtex[0]);
				nebulaObj->Render(0,24,0,2);
				renderSystem->GetDevice()->SetTexture(0, skyboxtex[1]);
				nebulaObj->Render(0,24,2,2);
				renderSystem->GetDevice()->SetTexture(0, skyboxtex[2]);
				nebulaObj->Render(0,24,4,2);
				renderSystem->GetDevice()->SetTexture(0, skyboxtex[3]);
				nebulaObj->Render(0,24,6,2);
				renderSystem->GetDevice()->SetTexture(0, skyboxtex[4]);
				nebulaObj->Render(0,24,8,2);
				renderSystem->GetDevice()->SetTexture(0, skyboxtex[5]);
				nebulaObj->Render(0,24,10,2);
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
	for(int m=0;starsObj && m<modelNum;m++) {
		if (modelList[m].index==315 || 
			modelList[m].index==444) {
			D3DXMATRIX mdscale, mdworld, mdrotate;
			float scale;
			
			D3DXMatrixIdentity(&mdworld);
					
			D3DXMatrixMultiply(&mdworld, &mdworld, &modelList[m].world);

			//mdworld[12]*=1000;
			//mdworld[13]*=1000;
			//mdworld[14]*=1000;
			scale=10.0;
			D3DXMatrixScaling(&mdscale, scale, scale, scale);
			D3DXMatrixMultiply(&mdworld, &mdscale, &mdworld);
			doMatrixes(mdworld);
			

			renderSystem->GetDevice()->SetTexture(0, starstex);
			//renderSystem->GetDevice()->SetRenderState(D3DRS_CULLMODE, g_model[starMod].cullmode);
			renderSystem->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
			starsObj->FrameMove(0,0,&mdworld);
			starsObj->Render();
			//doMatrixes(modelList[m].world);
			break;
		}
	}
	
	// end draw galaxy background

	renderSystem->SetRenderState(D3DRS_ZENABLE, true);

		// panel


		if (panelObj && *(unsigned char*)DATA_008835!=32 && *(unsigned char*)DATA_008835!=35) {

			renderSystem->GetDevice()->SetSamplerState(0, D3DSAMP_MIPFILTER, 0);
			renderSystem->SetRenderState(D3DRS_SPECULARENABLE, true);
			selectMaterial(-1, 0, 0, 0);
			renderSystem->GetDevice()->SetMaterial(&m_matMaterial);
			//renderSystem->SetRenderState( D3DRS_ALPHABLENDENABLE, false);
			renderSystem->SetRenderState(D3DRS_ZWRITEENABLE, true);
			renderSystem->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
			for(int i=0;i<10;i++) {
				renderSystem->GetDevice()->LightEnable(i, false);
			}
			int c=0;
			float cr = 0.0f;
			float cg = 1.0f;
			float cb = 0.0f;
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

			setupLight(playerLightPos, starColor.x, starColor.y, starColor.z);


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
			renderSystem->GetDevice()->SetTexture(0, panelTex);
			panelObj->Render();
		}
		for(int i=0;i<20;i++) {
			panellight[i]=false;
		}

	// ќтображаем бэкспрайты
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

	// ќтображаем модели

	renderSystem->GetDevice()->SetSamplerState(0, D3DSAMP_MIPFILTER, 2);
	renderSystem->SetRenderState(D3DRS_ZENABLE, true);

	ViewPort(true);

	unsigned int pass;
	D3DXMATRIX Full, WorldInverse;

	for(int m=0;m<modelNum;m++) {
		if (modelList[m].index==315 || 
			modelList[m].index==444 || 
			modelList[m].index==316 ||
			modelList[m].backsprite==true) {
			continue;
		}
		if (modelList[m].doMatrix==1) {
			currMod=m;
			currModIndex=modelList[m].index;
			D3DXMatrixInverse(&WorldInverse,NULL, &modelList[m].world);

			if (Planet(currModIndex)==true || Rings(currModIndex)==true) {
				doPerspectiveFar();
			} else {
				doPerspectiveNear();
			}
			for(int ii=0;ii<=10 && !modelList[m].subObject;ii++) {
				renderSystem->GetDevice()->LightEnable(ii, false);
			}

			// ќбщий свет
			if (!modelList[m].subObject)
				setupLight(modelList[m].lightPos, starColor.x, starColor.y, starColor.z);
			// —убисточники света
			for(int ii=0;ii<10 && !modelList[m].subObject;ii++) {
				if (modelList[m].light[ii].enable==true) {
					setupModelLight(ii, modelList[m].light[ii].pos, modelList[m].light[ii].dist, modelList[m].light[ii].r, modelList[m].light[ii].g, modelList[m].light[ii].b, true);
				}
			}

			if (effectList[currModIndex]) {
				effectList[currModIndex]->SetTechnique("Main");
				effectList[currModIndex]->SetValue("light", new D3DXVECTOR4(modelList[m].lightPos.x,modelList[m].lightPos.y,modelList[m].lightPos.z,0.0f), D3DX_DEFAULT);
				effectList[currModIndex]->SetValue("lightcol", starColor, D3DX_DEFAULT);
				effectList[currModIndex]->SetValue("ambient", new D3DXVECTOR4(1.0f/255*32,1.0f/255*32,1.0f/255*32, 1), D3DX_DEFAULT);
				tt +=DeltaTime*0.3f;
				effectList[currModIndex]->SetValue("time",&tt, D3DX_DEFAULT);

				D3DXMatrixMultiply(&Full, &matProj, &matView);
				effectList[currModIndex]->SetMatrix("viewprojmat",&Full);
				effectList[currModIndex]->SetMatrix("worldInverse",&WorldInverse);
			}

			renderSystem->SetRenderState(D3DRS_LIGHTING, TRUE);
			renderSystem->SetRenderState(D3DRS_ZWRITEENABLE, modelList[m].zwrite);
			renderSystem->SetRenderState(D3DRS_ZENABLE, modelList[m].zenable);
			renderSystem->SetRenderState(D3DRS_CULLMODE, modelList[m].cull);
			selectMaterial(modelList[m].material, starColor.x, starColor.y, starColor.z);
			renderSystem->GetDevice()->SetMaterial(&m_matMaterial);

			// ¬нешние модели
			if (objectList[currModIndex]->Exist()) {

				D3DXMatrixIdentity(&mdworld);				
				mdworld[12]+=objectList[currModIndex]->config.offset_x;
				mdworld[13]+=objectList[currModIndex]->config.offset_y;
				mdworld[14]+=objectList[currModIndex]->config.offset_z;
				
				D3DXMatrixMultiply(&mdworld, &mdworld, &modelList[m].world);
				
				scale=objectList[currModIndex]->config.scale;
				D3DXMatrixScaling(&mdscale, scale, scale, scale);
				D3DXMatrixMultiply(&mdworld, &mdscale, &mdworld);

				scale_x=objectList[currModIndex]->config.scale_x;
				scale_y=objectList[currModIndex]->config.scale_y;
				scale_z=objectList[currModIndex]->config.scale_z;
				D3DXMatrixScaling(&mdscale, scale_x, scale_y, scale_z);
				D3DXMatrixMultiply(&mdworld, &mdscale, &mdworld);

				renderSystem->GetDevice()->SetTexture(0, objectList[currModIndex]->skin[0]);

				//find the time difference
				QueryPerformanceCounter(&nowtime);
				dtime = ((nowtime.QuadPart - starttime)/ticks);
				if ((currModIndex > 14 && currModIndex < 70) || currModIndex==227 || currModIndex==234 || currModIndex==236) {
					if (modelList[m].landingGear>0 || objectList[currModIndex]->GetNumAnimationSets()==1) {
						objectList[currModIndex]->SetAnimationSet(0);
						if (modelList[m].index==227) {
							char cBuff[32];
							sprintf(cBuff,"FFED3D: %i", modelList[m].landingGear);
							SetWindowText(hWnd,cBuff);
							lG = ((float)modelList[m].landingGear/65535)*objectList[currModIndex]->GetAnimationSetLength(0);
						}
						else
							lG = (float)modelList[m].landingGear/65536*objectList[currModIndex]->GetAnimationSetLength(0);
						objectList[currModIndex]->FrameMove(0, lG,&mdworld);
					} else {
						objectList[currModIndex]->SetAnimationSet(1);
						objectList[currModIndex]->FrameMove(1, dtime,&mdworld);
					}
				} else {
					objectList[currModIndex]->SetAnimationSet(0);
					objectList[currModIndex]->FrameMove(1, dtime,&mdworld);
				}
				starttime = nowtime.QuadPart;
		
				if (effectList[currModIndex]) {
					effectList[currModIndex]->SetTexture("skin",objectList[currModIndex]->skin[0]);
					char buf[100];
					for(int i=1;i<10;i++) {
						if (objectList[currModIndex]->skin[i]) {
							sprintf(buf,"skin%i",i);
							effectList[currModIndex]->SetTexture(buf,objectList[currModIndex]->skin[i]);
						}
					}
				}

				objectList[currModIndex]->SetEffect(effectList[currModIndex]);
				objectList[currModIndex]->Render();
				objectList[currModIndex]->counter++;
			} else {
				objectList[currModIndex]->needLoad = true;
			}

			{

				// ќтрисовываем внутренние модели
				doMatrixes(modelList[m].world);

				if (effectList[currModIndex]) {
					effectList[currModIndex]->SetMatrix("worldmat",&modelList[m].world);
					effectList[currModIndex]->Begin(&pass,0);
				}
				drawModelPrimitives(modelList[m].vertStart, modelList[m].vertEnd);
				if (effectList[currModIndex]) {
					effectList[currModIndex]->End();
				}
			}

			if (modelList[m].zclear==true)
				renderSystem->GetDevice()->Clear(0,NULL,D3DCLEAR_ZBUFFER,D3DCOLOR_XRGB(0,0,0),1.0f,0);	
			for(int ii=0;ii<10 && !modelList[m].subObject;ii++) {
				modelList[m].light[ii].enable=false;
			}
		}
	}

	// ќтображаем спрайты

	renderSystem->GetDevice()->SetSamplerState( 0, D3DSAMP_ADDRESSU,  D3DTADDRESS_CLAMP ); 
	renderSystem->GetDevice()->SetSamplerState( 0, D3DSAMP_ADDRESSV,  D3DTADDRESS_CLAMP ); 

	renderSystem->SetRenderState( D3DRS_ALPHABLENDENABLE, true);
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

	// теперь нужно отрисовать прозрачные полигоны

	renderSystem->SetRenderState(D3DRS_LIGHTING, TRUE);
	renderSystem->SetRenderState(D3DRS_ZENABLE, true);
	renderSystem->SetRenderState(D3DRS_SPECULARENABLE, FALSE);
	selectMaterial(GALAXY, 0, 0, 0);
	renderSystem->GetDevice()->SetMaterial(&m_matMaterial);
	doTransp=true;
	renderSystem->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	for(int m=0;m<transpMax;m++) {
		currMod=transpBuf[m].modelNum;
		// ≈сли не спрайт
		if (modelList[currMod].doMatrix==1) {
			renderSystem->GetDevice()->SetSamplerState(0, D3DSAMP_MIPFILTER, 2);
			//renderSystem->SetRenderState(D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_COLOR1 );
			//renderSystem->SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_MATERIAL);
			renderSystem->SetRenderState(D3DRS_ZENABLE, true);
			ViewPort(true);
			doPerspectiveNear();
			doMatrixes(modelList[currMod].world);
		} else { // это спрайт
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
	renderSystem->SetRenderState(D3DRS_LIGHTING, TRUE);

	// panel sphere
	if (sphereObj && *(unsigned char*)DATA_008835!=32 && *(unsigned char*)DATA_008835!=35) {
		renderSystem->GetDevice()->SetSamplerState(0, D3DSAMP_MIPFILTER, 2);
		for(int i=0;i<10;i++) {
			renderSystem->GetDevice()->LightEnable(i, false);
		}
		setupLight(playerLightPos, starColor.x, starColor.y, starColor.z);

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

		selectMaterial(TRANSP, 0, 0, 0);

		sphereObj->FrameMove(0,0,&mdworld);
		renderSystem->GetDevice()->SetMaterial(&m_matMaterial);
		renderSystem->GetDevice()->SetTexture(0, NULL);
		sphereObj->Render();
	}


	ViewPort(false);
	doOrto();

	// текстовые сообщени€
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

	// онсоль рисуем.
	
	if (console)
	{
		float sx = curwidth/640.0f;
		D3DXMATRIX *scale = new D3DXMATRIX;;
		D3DXMatrixScaling (scale, sx, 1, 1);
		textSprite->Begin (D3DXSPRITE_ALPHABLEND);
		//textSprite->SetTransform (scale);
		textSprite->Draw (textures[96], NULL, NULL, NULL, 0xffffffff);
		//рисуем бффер
		//рисуем строку ввода
		DrawConText (">", 2, 480, 0xff00ff00);
		DrawConText (con_text, 20, 480, 0xffffffff);
		
		//выводим лог консоли

		scriptSystem* scr = scriptSystem::getSingleton();
		int conPosY = 460;

		for (int i=scr->getLogCount()-1;i>=0;i--)
		{
			DrawConText (scr->getLogString(i), 2, conPosY, D3DCOLOR_XRGB (220, 240, 255));
			conPosY -= 20;
		}

		textSprite->End();
	}

	renderSystem->EndScene();    
    //renderSystem->GetDevice()->Present(NULL, NULL, NULL, NULL);

	vertexNum=0;
	textNum=0;
	modelNum=0;
	maxsprite=0;
	modelList[0].material=NOTDRAW;
	sprites.clear();
	clearBeforeRender=false;
	if (incabin>=2)
		incabin=0;
}
