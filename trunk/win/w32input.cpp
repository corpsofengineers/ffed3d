#include "../ffeapi.h"
#include "win32api.h"
#include "../ffecfg.h"

#include <string.h>
#include <stdlib.h>
#include <math.h>

#define DIRECTINPUT_VERSION 0x500
#include <windows.h>
#define INITGUID
#include <objbase.h>
#include <dinput.h>

static LPDIRECTINPUT pDInput = NULL;
static LPDIRECTINPUTDEVICE pDIMouse = NULL;
static LPDIRECTINPUTDEVICE pDIKeyb = NULL;
static LPDIRECTINPUTDEVICE2 pDIJoy = NULL;

static long xmick = 0;
static long ymick = 0;
static int mflags = DISCL_FOREGROUND | DISCL_NONEXCLUSIVE;

static int sensitivity = 100;
static int repeatrate = 50;
static int repeatdelay = 300;

static int joyXRem = 0;
static int joyYRem = 0;
static int joyXVal = 0;
static int joyYVal = 0;
static ULONG joyLastTime = 0;
static int pJoyXTable[1024];
static int pJoyYTable[1024];


#include <stdio.h>
static FILE *pLog = NULL;

void Win32InputReacquire (void)
{
	if (pDIMouse) pDIMouse->Acquire();
	if (pDIKeyb) pDIKeyb->Acquire();
	if (pDIJoy) pDIJoy->Acquire();
}

void Win32SetMouseExclusive (void)
{
	mflags = DISCL_FOREGROUND | DISCL_EXCLUSIVE;
	if (pDIMouse) {
		pDIMouse->Unacquire();
		pDIMouse->SetCooperativeLevel (Win32GetWnd(), mflags);
		pDIMouse->Acquire();
	}
}

void Win32SetMouseNorm (void)
{
	mflags = DISCL_FOREGROUND | DISCL_NONEXCLUSIVE;
	if (pDIMouse) {
		pDIMouse->Unacquire();
		pDIMouse->SetCooperativeLevel (Win32GetWnd(), mflags);
		pDIMouse->Acquire();
	}
}

extern "C" long InputMouseReadButtons (void)
{
	DIMOUSESTATE mstate;
	long buttons = 0; int rval;

	if (pDIMouse == NULL) return 0;
	rval = pDIMouse->GetDeviceState (sizeof(mstate), (void *)&mstate);
	if (rval != DI_OK) return 0;

	xmick += mstate.lX;
	ymick += mstate.lY;
	if (mstate.rgbButtons[0]) buttons |= 1;
	if (mstate.rgbButtons[1]) buttons |= 2;
	
	return buttons;
}

extern "C" void InputMouseReadMickeys (long *pXMick, long *pYMick)
{
	DIMOUSESTATE mstate;
	if (pDIMouse == NULL) {
		*pXMick = 0; *pYMick = 0;
		return;
	}
	int rval = pDIMouse->GetDeviceState (sizeof(mstate), (void *)&mstate);
	if (rval != DI_OK) {
		*pXMick = 0; *pYMick = 0;
		return;
	}

	*pXMick = ((mstate.lX + xmick) * sensitivity) / 100;
	*pYMick = ((mstate.lY + ymick) * sensitivity) / 100;

	xmick = 0;
	ymick = 0;
}


unsigned char pKeybConvTable[256];
unsigned char pKeybConvTableDef[256] = {
	0, DIK_ESCAPE, DIK_1, DIK_2, DIK_3, DIK_4, DIK_5, DIK_6, 
	DIK_7, DIK_8, DIK_9, DIK_0, DIK_MINUS, DIK_EQUALS, DIK_BACK, DIK_TAB,
	DIK_Q, DIK_W, DIK_E, DIK_R, DIK_T, DIK_Y, DIK_U, DIK_I,
	DIK_O, DIK_P, DIK_LBRACKET, DIK_RBRACKET, DIK_RETURN, DIK_LCONTROL, DIK_A, DIK_S,
	DIK_D, DIK_F, DIK_G, DIK_H, DIK_J, DIK_K, DIK_L, DIK_SEMICOLON,
	DIK_APOSTROPHE, DIK_GRAVE, DIK_LSHIFT, DIK_BACKSLASH, DIK_Z, DIK_X, DIK_C, DIK_V,
	DIK_B, DIK_N, DIK_M, DIK_COMMA, DIK_PERIOD, DIK_SLASH, DIK_RSHIFT, DIK_MULTIPLY,
	DIK_LMENU, DIK_SPACE, DIK_CAPITAL, DIK_F1, DIK_F2, DIK_F3, DIK_F4, DIK_F5,
	DIK_F6, DIK_F7, DIK_F8, DIK_F9, DIK_F10, DIK_NUMLOCK, DIK_SCROLL, DIK_NUMPAD7,
	DIK_NUMPAD8, DIK_NUMPAD9, DIK_SUBTRACT, DIK_NUMPAD4, DIK_NUMPAD5, DIK_NUMPAD6, DIK_ADD, DIK_NUMPAD1,
	DIK_NUMPAD2, DIK_NUMPAD3, DIK_NUMPAD0, DIK_DECIMAL, 0, 0, 0, DIK_F11,
	DIK_F12, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,			// 0x60-0x67
	0, 0, 0, 0, 0, 0, 0, 0,			// 0x68-0x6f
	0, 0, 0, 0, 0, 0, 0, 0,			// 0x70-0x77
	0, 0, 0, 0, 0, 0, 0, 0,			// 0x78-0x7f
	0, 0, 0, 0, 0, 0, 0, 0,			// 0x80-0x87
	0, 0, 0, 0, 0, 0, 0, 0,			// 0x88-0x8f
	0, 0, 0, 0, 0, 0, 0, 0,			// 0x90-0x97
	0, 0, 0, 0, DIK_NUMPADENTER, DIK_RCONTROL, 0, 0,			// 0x98-0x9f
	0, 0, 0, 0, 0, 0, 0, 0,			// 0xa0-0xa7
	0, 0, 0, 0, 0, 0, 0, 0,			// 0xa8-0xaf
	0, 0, 0, 0, 0, DIK_DIVIDE, 0, DIK_SYSRQ,			// 0xb0-0xb7
	DIK_RMENU, 0, 0, 0, 0, 0, 0, 0,			// 0xb8-0xbf
	0, 0, 0, 0, 0, 0, 0, DIK_HOME,			// 0xc0-0xc7
	DIK_UP, DIK_PRIOR, 0, DIK_LEFT, 0, DIK_RIGHT, 0, DIK_END,			// 0xc8-0xcf
	DIK_DOWN, DIK_NEXT, DIK_INSERT, DIK_DELETE, 0, 0, 0, 0,			// 0xd0-0xd7
	0, 0, 0, DIK_LWIN, DIK_RWIN, DIK_APPS, 0, 0,			// 0xd8-0xdf
	0, 0, 0, 0, 0, 0, 0, 0,			// 0xe0-0xe7
	0, 0, 0, 0, 0, 0, 0, 0,			// 0xe8-0xef
	0, 0, 0, 0, 0, 0, 0, 0,			// 0xf0-0xf7
	0, 0, 0, 0, 0, 0, 0, 0,			// 0xf8-0xff
};

char *Commands[256] = {
	"", "Int_Pause", "KEY_1", "KEY_2", "KEY_3", "KEY_4", "KEY_5", "KEY_6", 
	"KEY_7", "KEY_8", "KEY_9", "KEY_0", "KEY_MINUS", "KEY_EQUALS", "KEY_BACK", "Ship_EngineControl",
	"KEY_Q", "KEY_W", "KEY_E", "Map_ShowSystemScheme", "Ship_Target", "KEY_Y","Ship_Gear", "KEY_I",
	"KEY_O", "KEY_P", "Ship_YawLeft", "Ship_YawRight", "Ship_Acceleration", "KEY_LCONTROL", "Ship_PitchDown", "KEY_S",
	"Ship_MiningCraft", "KEY_F", "KEY_G", "Ship_HyperJump", "KEY_J", "KEY_K", "Int_ShowMarkers", "KEY_SEMICOLON",
	"KEY_APOSTROPHE", "KEY_GRAVE", "KEY_LSHIFT", "KEY_BACKSLASH", "Ship_PitchUp", "Ship_Catapult", "Ship_Decoys", "KEY_V",
	"Ship_EnergyBomb", "Ship_LandTarget", "Ship_launchMissile", "Ship_RollLef", "Ship_RollRight", "Ship_HyperCloudAnalysis", "Ship_Inhibition", "KEY_MULTIPLY",
	"KEY_LMENU", "Ship_Fire", "KEY_CAPITAL", "Int_SwitchView", "Int_ShowShipInfo", "Int_StarMap", "Int_Comm", "Ship_SwitchTobMode",
	"KEY_F6", "KEY_F7", "KEY_F8", "KEY_F9", "KEY_F10", "KEY_NUMLOCK", "KEY_SCROLL", "Eng_StrafeLeft",
	"Eng_Main", "Eng_StrafeRight", "KEY_SUBTRACT", "KEY_NUMPAD4", "Eng_Retro", "KEY_NUMPAD6", "KEY_ADD", "KEY_NUMPAD1",
	"KEY_NUMPAD2", "KEY_NUMPAD3", "KEY_NUMPAD0", "KEY_DECIMAL", "", "", "", "KEY_F11",
	"KEY_F12", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "",			// ""x6""-""x67
	"", "", "", "", "", "", "", "",			// ""x68-""x6f
	"", "", "", "", "", "", "", "",			// ""x7""-""x77
	"", "", "", "", "", "", "", "",			// ""x78-""x7f
	"", "", "", "", "", "", "", "",			// ""x8""-""x87
	"", "", "", "", "", "", "", "",			// ""x88-""x8f
	"", "", "", "", "", "", "", "",			// ""x9""-""x97
	"", "", "", "", "KEY_NUMPADENTER", "KEY_RCONTROL", "", "",			// ""x98-""x9f
	"", "", "", "", "", "", "", "",			// ""xa""-""xa7
	"", "", "", "", "", "", "", "",			// ""xa8-""xaf
	"", "", "", "", "", "KEY_DIVIDE", "", "KEY_SYSRQ",			// ""xb""-""xb7
	"KEY_RMENU", "", "", "", "", "", "", "",			// ""xb8-""xbf
	"", "", "", "", "", "", "", "KEY_HOME",			// ""xc""-""xc7
	"KEY_UP", "Int_MessageUp", "", "KEY_LEFT", "", "KEY_RIGHT", "", "KEY_END",			// ""xc8-""xcf
	"KEY_DOWN", "Int_MessageDown", "KEY_INSERT", "KEY_DELETE", "", "", "", "",			// ""xd""-""xd7
	"", "", "", "KEY_LWIN", "KEY_RWIN", "KEY_APPS", "", "",			// ""xd8-""xdf
	"", "", "", "", "", "", "", "",			// ""xe""-""xe7
	"", "", "", "", "", "", "", "",			// ""xe8-""xef
	"", "", "", "", "", "", "", "",			// ""xf""-""xf7
	"", "", "", "", "", "", "", ""			// ""xf8-""xff
};

char *keyTOdig [256] = 
{
	"", "KEY_ESCAPE", "KEY_1", "KEY_2", "KEY_3", "KEY_4", "KEY_5", "KEY_6", 
	"KEY_7", "KEY_8", "KEY_9", "KEY_0", "KEY_MINUS", "KEY_EQUALS", "KEY_BACK", "KEY_TAB",
	"KEY_Q", "KEY_W", "KEY_E", "KEY_R", "KEY_T", "KEY_Y","KEY_U", "KEY_I",
	"KEY_O", "KEY_P", "KEY_LBRACKET", "KEY_RBRACKET", "KEY_RETURN", "KEY_LCONTROL", "KEY_A", "KEY_S",
	"KEY_D", "KEY_F", "KEY_G", "KEY_H", "KEY_J", "KEY_K", "KEY_L", "KEY_SEMICOLON",
	"KEY_APOSTROPHE", "KEY_GRAVE", "KEY_LSHIFT", "KEY_BACKSLASH", "KEY_Z", "KEY_X", "KEY_C", "KEY_V",
	"KEY_B", "KEY_N", "KEY_M", "KEY_COMMA", "KEY_PERIOD", "KEY_SLASH", "KEY_RSHIFT", "KEY_MULTIPLY",
	"KEY_LMENU", "KEY_SPACE", "KEY_CAPITAL", "KEY_F1", "KEY_F2", "KEY_F3", "KEY_F4", "KEY_F5",
	"KEY_F6", "KEY_F7", "KEY_F8", "KEY_F9", "KEY_F10", "KEY_NUMLOCK", "KEY_SCROLL", "KEY_NUMPAD7",
	"KEY_NUMPAD8", "KEY_NUMPAD9", "KEY_SUBTRACT", "KEY_NUMPAD4", "KEY_NUMPAD5", "KEY_NUMPAD6", "KEY_ADD", "KEY_NUMPAD1",
	"KEY_NUMPAD2", "KEY_NUMPAD3", "KEY_NUMPAD0", "KEY_DECIMAL", "", "", "", "KEY_F11",
	"KEY_F12", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "",			// ""x6""-""x67
	"", "", "", "", "", "", "", "",			// ""x68-""x6f
	"", "", "", "", "", "", "", "",			// ""x7""-""x77
	"", "", "", "", "", "", "", "",			// ""x78-""x7f
	"", "", "", "", "", "", "", "",			// ""x8""-""x87
	"", "", "", "", "", "", "", "",			// ""x88-""x8f
	"", "", "", "", "", "", "", "",			// ""x9""-""x97
	"", "", "", "", "KEY_NUMPADENTER", "KEY_RCONTROL", "", "",			// ""x98-""x9f
	"", "", "", "", "", "", "", "",			// ""xa""-""xa7
	"", "", "", "", "", "", "", "",			// ""xa8-""xaf
	"", "", "", "", "", "KEY_DIVIDE", "", "KEY_SYSRQ",			// ""xb""-""xb7
	"KEY_RMENU", "", "", "", "", "", "", "",			// ""xb8-""xbf
	"", "", "", "", "", "", "", "KEY_HOME",			// ""xc""-""xc7
	"KEY_UP", "KEY_PRIOR", "", "KEY_LEFT", "", "KEY_RIGHT", "", "KEY_END",			// ""xc8-""xcf
	"KEY_DOWN", "KEY_NEXT", "KEY_INSERT", "KEY_DELETE", "", "", "", "",			// ""xd""-""xd7
	"", "", "", "KEY_LWIN", "KEY_RWIN", "KEY_APPS", "", "",			// ""xd8-""xdf
	"", "", "", "", "", "", "", "",			// ""xe""-""xe7
	"", "", "", "", "", "", "", "",			// ""xe8-""xef
	"", "", "", "", "", "", "", "",			// ""xf""-""xf7
	"", "", "", "", "", "", "", ""			// ""xf8-""xff
};


static UCHAR pKeybStates[256];
static int keybLastKey = 0xff;
static ULONG pKeybTimes[256];


void InputKeybReadStates (UCHAR *pKeys)
{
	int i, rval;
	UCHAR pCur[256];

	memset (pKeys, 0x80, 256);
	if (pDIKeyb == NULL) return;
	rval = pDIKeyb->GetDeviceState (256, (void *)pCur);
	if (rval != DI_OK) return;

	for (i=0; i<256; i++)
	{
		if (pCur[pKeybConvTable[i]] & 0x80)
		{
			if (pKeybStates[i] == 0x80) {		// Initial press
				keybLastKey = i;
				pKeybStates[i] = 0;
				pKeybTimes[i] = TimerGetTimeStamp() + repeatdelay;
			}
			else if (pKeybStates[i] == 0) {			// hold
				if (pKeybTimes[i] < TimerGetTimeStamp()) {
					pKeybStates[i] = 0xff;
				}
			}
			if (pKeybStates[i] == 0xff) {		// Repeat
				if (pKeybTimes[i] < TimerGetTimeStamp()) {
					keybLastKey = i;
					pKeybTimes[i] += repeatrate;
				}
			}
		}
		else pKeybStates[i] = 0x80;
	}
	for (i=0; i<256; i++) {
		if (pKeybStates[i] == 0x80) pKeys[i] = 0x80;
		else pKeys[i] = 0x0;
	}
}

extern "C" long InputKeybGetLastKey (void)
{
	return keybLastKey;
}

extern "C" void InputKeybSetLastKey (long key)
{
	keybLastKey = key;
}

static BOOL CALLBACK JoyEnumFunc (LPCDIDEVICEINSTANCE lpddi, LPVOID pGuid)
{
	*(GUID *)pGuid = lpddi->guidInstance;
	return DIENUM_STOP;
}

void KeyBinds (CfgStruct *cfg)
{
	bool bind = false;
	char key[256];
	char *tok;

	CfgFindSection (cfg, "BINDS");

	for (int i=0;i<256;i++)
	{
		
		if (CfgGetKeyStr (cfg, Commands[i], key, 255) != 0)
		{
			tok = strtok (key, " \n\t");
			strcpy (key, tok);
			for (int kc=0;kc<256;kc++)
			{

				if (strcmp (keyTOdig[kc], key) == 0)
				{
					pKeybConvTable[i]=kc;
					bind=true;
					break;
				}

			}
		}

		if (!bind)
		{
			pKeybConvTable[i]=pKeybConvTableDef[i];
		}
		bind=false;
	
	}

}

void InputInit (void)
{
	int rval, i;
	CfgStruct cfg;
	GUID joyGuid;
	LPDIRECTINPUTDEVICE pDIJoy1;

	if (pDInput == NULL) {
		rval = DirectInputCreate (Win32GetInst(), DIRECTINPUT_VERSION,
			&pDInput, NULL);
		if (rval != DI_OK) return;
	}

	CfgOpen (&cfg, __CONFIGFILE__);
	CfgFindSection (&cfg, "MOUSE");
	CfgGetKeyVal (&cfg, "sensitivity", &sensitivity);
	CfgFindSection (&cfg, "KEYBOARD");
	CfgGetKeyVal (&cfg, "repeatrate", &repeatrate);
	CfgGetKeyVal (&cfg, "repeatdelay", &repeatdelay);

	//Reading Key Binds
	KeyBinds(&cfg);
	CfgClose (&cfg);

	if (pDIMouse == NULL) {
		rval = pDInput->CreateDevice (GUID_SysMouse, &pDIMouse, NULL);
		if (rval != DI_OK) return;
		rval = pDIMouse->SetCooperativeLevel (Win32GetWnd(), mflags);
		rval = pDIMouse->SetDataFormat (&c_dfDIMouse);
		rval = pDIMouse->Acquire();
	}


	if (pDIKeyb == NULL) {
		rval = pDInput->CreateDevice (GUID_SysKeyboard, &pDIKeyb, NULL);
		if (rval != DI_OK) return;
		rval = pDIKeyb->SetCooperativeLevel (Win32GetWnd(), 
			DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
		rval = pDIKeyb->SetDataFormat (&c_dfDIKeyboard);
		rval = pDIKeyb->Acquire();
		memset (pKeybStates, 0x80, 256);
	}

	int xexpi = 20, yexpi = 20;
	int xmax = 512, ymax = 512;
	int xdeadzone = 20, ydeadzone = 20;
	int xmaxzone = 20, ymaxzone = 20;
	int enabled = 0, pollfreq = 10;

	CfgOpen (&cfg, __CONFIGFILE__);
	CfgFindSection (&cfg, "JOYSTICK");
	CfgGetKeyVal (&cfg, "enabled", &enabled);
	CfgGetKeyVal (&cfg, "pollfreq", &pollfreq);
	CfgGetKeyVal (&cfg, "xexp", &xexpi);
	CfgGetKeyVal (&cfg, "yexp", &yexpi);
	CfgGetKeyVal (&cfg, "xmax", &xmax);
	CfgGetKeyVal (&cfg, "ymax", &ymax);
	CfgGetKeyVal (&cfg, "xdeadzone", &xdeadzone);
	CfgGetKeyVal (&cfg, "ydeadzone", &ydeadzone);
	CfgGetKeyVal (&cfg, "xmaxzone", &xmaxzone);
	CfgGetKeyVal (&cfg, "ymaxzone", &ymaxzone);
	CfgClose (&cfg);

	if (enabled == 0) return;
	if (pDIJoy != NULL) return;

	rval = pDInput->EnumDevices (DIDEVTYPE_JOYSTICK, JoyEnumFunc,
		&joyGuid, DIEDFL_ATTACHEDONLY);
	if (rval != DI_OK) return;
	rval = pDInput->CreateDevice (joyGuid, &pDIJoy1, NULL);
	if (rval != DI_OK) return;
	rval = pDIJoy1->QueryInterface (IID_IDirectInputDevice2, (void **)&pDIJoy);
	if (rval != DI_OK) return;
	pDIJoy1->Release();
	pDIJoy->SetCooperativeLevel (Win32GetWnd(), 
		DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	pDIJoy->SetDataFormat (&c_dfDIJoystick);
		
	DIPROPRANGE dirn;
	DIPROPDWORD didw;
		
	dirn.diph.dwSize = sizeof(DIPROPRANGE);
	dirn.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	dirn.diph.dwHow = DIPH_BYOFFSET;
	didw.diph.dwSize = sizeof(DIPROPDWORD);
	didw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	didw.diph.dwHow = DIPH_BYOFFSET;
	dirn.lMin = -512;
	dirn.lMax = +511;

	dirn.diph.dwObj = DIJOFS_X;
	pDIJoy->SetProperty (DIPROP_RANGE, &dirn.diph);
	didw.dwData = 0;
	pDIJoy->SetProperty (DIPROP_DEADZONE, &didw.diph);
	didw.dwData = 10000;
	pDIJoy->SetProperty (DIPROP_SATURATION, &didw.diph);

	dirn.diph.dwObj = DIJOFS_Y;
	pDIJoy->SetProperty (DIPROP_RANGE, &dirn.diph);
	didw.dwData = 0;
	pDIJoy->SetProperty (DIPROP_DEADZONE, &didw.diph);
	didw.dwData = 10000;
	pDIJoy->SetProperty (DIPROP_SATURATION, &didw.diph);

	pDIJoy->Acquire();
	joyLastTime = TimerGetTimeStamp();

	double xexp = (double)xexpi / 10.0;
	double yexp = (double)yexpi / 10.0;
	double xdiv = 512 - xdeadzone - xmaxzone;
	double ydiv = 512 - ydeadzone - ymaxzone;

	for (i=0; i<xdeadzone; i++) pJoyXTable[i+512] = 0;
	for (; i<512-xmaxzone; i++)
		pJoyXTable[i+512] = (int)(pow ((i-xdeadzone)/xdiv, xexp) * xmax + 0.999);
	for (; i<512; i++) pJoyXTable[i+512] = xmax;

	for (i=0; i<ydeadzone; i++) pJoyYTable[i+512] = 0;
	for (; i<512-ymaxzone; i++)
		pJoyYTable[i+512] = (int)(pow ((i-ydeadzone)/ydiv, yexp) * ymax + 0.999);
	for (; i<512; i++) pJoyYTable[i+512] = ymax;

	for (i=0; i<512; i++) pJoyXTable[i] = -pJoyXTable[1023-i];
	for (i=0; i<512; i++) pJoyYTable[i] = -pJoyYTable[1023-i];

}

void InputCleanup (void)
{
	if (pDIMouse != NULL) {
		pDIMouse->Unacquire();
		pDIMouse->Release();
		pDIMouse = NULL;
	}
	if (pDIKeyb != NULL) {
		pDIKeyb->Unacquire();
		pDIKeyb->Release();
		pDIKeyb = NULL;
	}
	if (pDIJoy != NULL) {
		pDIJoy->Unacquire();
		pDIJoy->Release();
		pDIJoy = NULL;
	}
	if (pDInput != NULL) { pDInput->Release(); pDInput = NULL; }
}

extern "C" void InputJoyReadPos (long *pXPos, long *pYPos)
{
	if (!pDIJoy) { 
		*pXPos = *pYPos = 0;
		return;
	}

	DIJOYSTATE state;
	int xval, yval, rval;
	ULONG joyCurTime = TimerGetTimeStamp();
	ULONG joyTickCount = (joyCurTime - joyLastTime) / 5;

	memset (&state, 0, sizeof(state));
	pDIJoy->Poll();
	rval = pDIJoy->GetDeviceState (sizeof(state), (void *)&state);
	if (rval != DI_OK) {
		*pXPos = *pYPos = 0;
		return;
	}

	xval = pJoyXTable[state.lX+512];
	yval = pJoyYTable[state.lY+512];

	if (xval == 0) joyXRem = 0;
	if (yval == 0) joyYRem = 0;

	xval = xval * joyTickCount + joyXRem;
	joyXRem = xval & 0x800000ff;
	if (joyXRem < 0) joyXRem |= 0xffffff00;
	xval = (xval - joyXRem) >> 8;

	yval = yval * joyTickCount + joyYRem;
	joyYRem = yval & 0x800000ff;
	if (joyYRem < 0) joyYRem |= 0xffffff00;
	yval = (yval - joyYRem) >> 8;

	joyLastTime = joyCurTime;
	*pXPos = xval; *pYPos = yval;

}

extern "C" long InputJoyReadButtons (void)
{
	if (!pDIJoy) return 0;
	DIJOYSTATE state;
	int rval, buttons = 0;

	memset (&state, 0, sizeof(state));
	rval = pDIJoy->GetDeviceState (sizeof(state), (void *)&state);
	if (rval != DI_OK) return 0;

	if (state.rgbButtons[0]) buttons |= 1;
	if (state.rgbButtons[1]) buttons |= 2;
	if (state.rgbButtons[2]) buttons |= 4;
	if (state.rgbButtons[3]) buttons |= 8;
	return buttons;
}
