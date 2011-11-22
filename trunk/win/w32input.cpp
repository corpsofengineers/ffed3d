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
#include "ffe3d.h"

extern int console;
char* con_text= new char[255];

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
	"NULL", "time_pause", "cheat1", "cheat2", "cheat3", "cheat_4", "game_load", "time_x1", 
	"time_x2", "time_x4", "time_x8", "time_x16", "cam_zoomin", "cam_zoomout", "NULL", "eng_switch",
	"NULL", "NULL", "env_ecm", "map_orbscheme", "pl_target", "NULL", "pl_gear", "NULL",
	"NULL", "NULL", "pl_rolll", "pl_rollr", "pl_accel", "NULL", "pl_pitchd", "NULL",
	"env_mdeploy", "fps", "NULL", "pl_hjump", "NULL", "NULL", "int_shownames", "NULL",
	"NULL", "com_wing", "NULL", "NULL", "pl_pitchu", "env_evac", "env_decoy", "NULL",
	"env_bomb", "pl_ltarget", "env_missile", "pl_yawl", "pl_yawr", "env_hanalis", "pl_inhibition", "UNKNOWN",
	"NULL", "pl_fire", "NULL", "cam_switch", "pl_info", "pl_map", "pl_comm", "pl_bmode",
	"cam_ahead", "cam_back", "cam_turret", "cam_orbit", "cam_missile", "NULL", "NULL", "pl_strafel",
	"pl_trustm", "pl_strafer", "NULL", "pl_strafeu", "pl_trustr", "NULL", "NULL", "pl_strafed",
	"NULL", "NULL", "NULL", "NULL", "NULL", "NULL", "NULL", "env_battlec",
	"cam_capsule", "NULL", "NULL", "NULL", "NULL", "NULL", "NULL", "NULL",
	"NULL", "NULL", "NULL", "NULL", "NULL", "NULL", "NULL", "NULL",			// "NULL"x6"NULL"-"NULL"x67
	"NULL", "NULL", "NULL", "NULL", "NULL", "NULL", "NULL", "NULL",			// "NULL"x68-"NULL"x6f
	"NULL", "NULL", "NULL", "NULL", "NULL", "NULL", "NULL", "NULL",			// "NULL"x7"NULL"-"NULL"x77
	"NULL", "NULL", "NULL", "NULL", "NULL", "NULL", "NULL", "NULL",			// "NULL"x78-"NULL"x7f
	"NULL", "NULL", "NULL", "NULL", "NULL", "NULL", "NULL", "NULL",			// "NULL"x8"NULL"-"NULL"x87
	"NULL", "NULL", "NULL", "NULL", "NULL", "NULL", "NULL", "NULL",			// "NULL"x88-"NULL"x8f
	"NULL", "NULL", "NULL", "NULL", "NULL", "NULL", "NULL", "NULL",			// "NULL"x9"NULL"-"NULL"x97
	"NULL", "NULL", "NULL", "NULL", "NULL", "NULL", "NULL", "NULL",			// "NULL"x98-"NULL"x9f
	"NULL", "NULL", "NULL", "NULL", "NULL", "NULL", "NULL", "NULL",			// "NULL"xa"NULL"-"NULL"xa7
	"NULL", "NULL", "NULL", "NULL", "NULL", "NULL", "NULL", "NULL",			// "NULL"xa8-"NULL"xaf
	"NULL", "NULL", "NULL", "NULL", "NULL", "NULL", "NULL", "NULL",			// 0xb0-0xb7
	"NULL", "NULL", "NULL", "NULL", "NULL", "NULL", "NULL", "NULL",			// "NULL"xb8-"NULL"xbf
	"NULL", "NULL", "NULL", "NULL", "NULL", "NULL", "NULL", "NULL",			// 0xc0-0xc7
	"cam_up", "msg_down", "NULL", "cam_left", "NULL", "cam_right", "NULL", "NULL",			// 0xc8-0xcf
	"cam_down", "msg_up", "NULL", "NULL", "NULL", "NULL", "NULL", "NULL",			// "NULL"xd"NULL"-"NULL"xd7
	"NULL", "NULL", "NULL", "NULL", "NULL", "NULL", "NULL", "NULL",			// "NULL"xd8-"NULL"xdf
	"NULL", "NULL", "NULL", "NULL", "NULL", "NULL", "NULL", "NULL",			// "NULL"xe"NULL"-"NULL"xe7
	"NULL", "NULL", "NULL", "NULL", "NULL", "NULL", "NULL", "NULL",			// "NULL"xe8-"NULL"xef
	"NULL", "NULL", "NULL", "NULL", "NULL", "NULL", "NULL", "NULL",			// "NULL"xf"NULL"-"NULL"xf7
	"NULL", "NULL", "NULL", "NULL", "NULL", "NULL", "NULL", "NULL",			// "NULL"xf8-"NULL"xff
};

char *keyTOdig [256] = 
{
	"", "KEY_ESCAPE", "KEY_1", "KEY_2", "KEY_3", "KEY_4", "KEY_5", "KEY_6", 
	"KEY_7", "KEY_8", "KEY_9", "KEY_0", "KEY_MINUS", "KEY_EQUALS", "KEY_BACK", "KEY_TAB",
	"KEY_Q", "KEY_W", "KEY_E", "KEY_R", "KEY_T", "KEY_Y", "KEY_U", "KEY_I",
	"KEY_O", "KEY_P", "KEY_LBRACKET", "KEY_RBRACKET", "KEY_RETURN", "KEY_LCONTROL", "KEY_A", "KEY_S",
	"KEY_D", "KEY_F", "KEY_G", "KEY_H", "KEY_J", "KEY_K", "KEY_L", "KEY_SEMICOLON",
	"KEY_APOSTROPHE", "KEY_GRAVE", "KEY_LSHIFT", "KEY_BACKSLASH", "KEY_Z", "KEY_X", "KEY_C", "KEY_V",
	"KEY_B", "KEY_N", "KEY_M", "KEY_COMMA", "KEY_PERIOD", "KEY_SLASH", "KEY_RSHIFT", "KEY_MULTIPLY",
	"KEY_LMENU", "KEY_SPACE", "KEY_CAPITAL", "KEY_F1", "KEY_F2", "KEY_F3", "KEY_F4", "KEY_F5",
	"KEY_F6", "KEY_F7", "KEY_F8", "KEY_F9", "KEY_F10", "KEY_NUMLOCK", "KEY_SCROLL", "KEY_NUMPAD7",
	"KEY_NUMPAD8", "KEY_NUMPAD9", "KEY_SUBTRACT", "KEY_NUMPAD4", "KEY_NUMPAD5", "KEY_NUMPAD6", "KEY_ADD", "KEY_NUMPAD1",
	"KEY_NUMPAD2", "KEY_NUMPAD3", "KEY_NUMPAD0", "KEY_DECIMAL","","","", "KEY_F11",
	"KEY_F12","","","","","","","",
	"","","","","","","","",			// 0x60-0x67
	"","","","","","","","",			// 0x68-0x6f
	"","","","","","","","",			// 0x70-0x77
	"","","","","","","","",			// 0x78-0x7f
	"","","","","","","","",			// 0x80-0x87
	"","","","","","","","",			// 0x88-0x8f
	"","","","","","","","",			// 0x90-0x97
	"","","","", "KEY_NUMPADENTER", "KEY_RCONTROL","","",			// 0x98-0x9f
	"","","","","","","","",			// 0xa0-0xa7
	"","","","","","","","",			// 0xa8-0xaf
	"","","","","", "KEY_DIVIDE","", "KEY_SYSRQ",			// 0xb0-0xb7
	"KEY_RMENU","","","","","","","",			// 0xb8-0xbf
	"","","","","","","", "KEY_HOME",			// 0xc0-0xc7
	"KEY_UP", "KEY_PRIOR","", "KEY_LEFT","", "KEY_RIGHT","", "KEY_END",			// 0xc8-0xcf
	"KEY_DOWN", "KEY_NEXT", "KEY_INSERT", "KEY_DELETE","","","","",			// 0xd0-0xd7
	"","","", "KEY_LWIN", "KEY_RWIN", "KEY_APPS","","",			// 0xd8-0xdf
	"","","","","","","","",			// 0xe0-0xe7
	"","","","","","","","",			// 0xe8-0xef
	"","","","","","","","",			// 0xf0-0xf7
	"","","","","","","","",			// 0xf8-0xff
};

static UCHAR pKeybStates[256];
static int keybLastKey = 0xff;
static ULONG pKeybTimes[256];


extern WingmanList_t WingmanArray;

extern u8 console_enable;
char* console_comand;
char* console_history[20];
int console_historyC = 0;
int console_historyPos = -1;
char* console_visual;
int console_carPos;
int console_LogP = 1;

void keyToConsole (WPARAM key) //преобразуем нажатые клавиши в текст в консольки с карреткой
{
	int len = strlen (console_comand);

	//история
	if (key == VK_UP)
	{
		if (console_historyPos > 0)
		{
			console_historyPos--;
			memcpy (console_comand, console_history[console_historyPos], 65);
			console_carPos = strlen (console_comand)-1;
		}
	}

	if (key == VK_DOWN)
	{
		if (console_historyPos < console_historyC-1)
		{
			console_historyPos++;
			memcpy (console_comand, console_history[console_historyPos], 65);
			console_carPos = strlen (console_comand)-1;
		}
	}

	if (key == VK_PRIOR)
	{
		if (console_LogP < scriptSystem::getSingleton()->getLogCount())
			console_LogP++;
	}

	if (key == VK_NEXT)
	{
		if (console_LogP > 1)
			console_LogP--;
	}

	//обработчики карректи
	if (key == VK_LEFT)
	{
		if (console_carPos > -1)
			console_carPos--;
		return;
	}

	if (key == VK_RIGHT)
	{
		if (console_carPos+1 < len)
			console_carPos++;
		return;
	}

	if (key == VK_HOME)
	{
		console_carPos = -1;
	}

	if (key == VK_END)
	{
		console_carPos = strlen (console_comand)-1;
	}

	if (key == VK_BACK)
	{
		if (console_carPos > -1)
		{
			memcpy (console_comand+console_carPos, console_comand+console_carPos+1, len-console_carPos);
			*(console_comand+len)=0;
			console_carPos--;
		}
		return;
	}

	if (key == VK_DELETE)
	{
		if (console_carPos+1 < len)
		{
			memcpy (console_comand+console_carPos+1, console_comand+console_carPos+2, len-console_carPos+1);
			*(console_comand+len)=0;
		}
		return;
	}

	if (key == VK_RETURN)
	{
		scriptSystem::getSingleton()->doString (console_comand);

		memcpy (console_history[console_historyC], console_comand, 65);
		
		if (console_historyC < 19)
			console_historyC++;

		console_historyPos = console_historyC;

		memset (console_comand, 0, 65);
		console_carPos=-1;
	
	}

	//преобразуем клавиши в строку текста
	BYTE* keyboardState = new BYTE[256];

	if (GetKeyboardState (keyboardState))
	{
		WORD wChar;
		int ra = ToAscii (key, 0, keyboardState, &wChar, 1);
		
		if(ra > 0)
		{
			if (len == 64)
				return;

			if (console_carPos+1 == len) //пишем в конец строки
			{
				*(console_comand+len) = wChar;
			} else if(console_carPos+1 < len) //пишем в середину строки
			{
				memcpy (console_comand+console_carPos+2, console_comand+console_carPos+1, len-console_carPos+1);
				*(console_comand+console_carPos+1) = wChar;
			}

			*(console_comand+len+1) = 0;
			console_carPos++;
		}

	}

}

void InputKeybReadStates (UCHAR *pKeys)
{
	int i, rval;
	UCHAR pCur[256];

	memset (pKeys, 0x80, 256);
	if (pDIKeyb == NULL) return;
	rval = pDIKeyb->GetDeviceState (256, (void *)pCur);
	if (rval != DI_OK) return;

	if (pCur[DIK_LCONTROL] == 0x80 && pCur[DIK_GRAVE] == 0x80 && pKeybTimes[41] < TimerGetTimeStamp())
	{
		if (!console_enable)
			console_enable++;
		else
			console_enable--;
		pKeybTimes[41] = TimerGetTimeStamp() + repeatdelay;
		keybLastKey = 255;
		return;
	}

	if (console_enable)
	{
		//тут будет скроллинг текста и управление буфером ввода/вывода консоли
		return;
	}

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
	//WINGMAN MENU
	if (keybLastKey == 41 && !WingmanArray.showMenu)
	{
		WingmanArray.showMenu++;
		WingmanArray.menuState = 0;
		WingmanArray.wingmanId = -1;
	}
	else
		if (keybLastKey == 41 && WingmanArray.showMenu)
		{
			WingmanArray.showMenu--;
			WingmanArray.menuState = 0;
		}

	if (WingmanArray.showMenu)
	{
		switch (WingmanArray.menuState)
		{
		case 0: //main screen
			{
				if (keybLastKey == 2) {WingmanArray.menuState = 1; WingmanArray.wingmanId = -1;}
				if (keybLastKey == 3) WingmanArray.menuState = 2;
				if (keybLastKey == 4) WingmanArray.menuState = 3;
				if (keybLastKey == 5) WingmanArray.menuState = 4;
				if (keybLastKey == 6) WingmanArray.menuState = 5;
				//keybLastKey = 255
				break;
			}
		case 1: //select wingman
			{
				if (keybLastKey == 2) WingmanArray.wingmanId = 0;
				if (keybLastKey == 3) WingmanArray.wingmanId = 1;
				if (keybLastKey == 4) WingmanArray.wingmanId = 2;
				if (keybLastKey == 5) WingmanArray.wingmanId = 3;
				if (keybLastKey == 6) WingmanArray.wingmanId = 4;
				if (keybLastKey == 7) WingmanArray.wingmanId = 5;
				if (keybLastKey == 8) WingmanArray.wingmanId = 6;
				if (keybLastKey == 9) WingmanArray.wingmanId = 7;
				if (keybLastKey == 10) WingmanArray.wingmanId = 8;

				if (WingmanArray.wingmanId > -1)
				{
					if (WingmanArray.wingmanId >= WingmanArray.Count)
						WingmanArray.wingmanId = -1;
					WingmanArray.menuState = 0;
				}

				break;
			}
		case 2: //goto
			{
				if (WingmanArray.wingmanId > -1)
				{
					WingmanArray.instances[WingmanArray.wingmanId].command = WINGCOM_GOTO;
					WingmanArray.instances[WingmanArray.wingmanId].targetId = DATA_ObjectArray->instances[DATA_DestIndex].globalvars.unique_Id;
				} else
				{
					for (int i = 0; i < WingmanArray.Count; i++)
					{
						WingmanArray.instances[i].command = WINGCOM_GOTO;
						WingmanArray.instances[i].targetId = DATA_ObjectArray->instances[DATA_DestIndex].globalvars.unique_Id;
					}
				}
				
				WingmanArray.showMenu--;
				WingmanArray.menuState = 0;
				break;
			}
		case 3: //hold position
			{
				if (WingmanArray.wingmanId > -1)
				{
					WingmanArray.instances[WingmanArray.wingmanId].command = WINGCOM_HALT;
				} else
				{
					for (int i = 0; i < WingmanArray.Count; i++)
					{
						WingmanArray.instances[i].command = WINGCOM_HALT;
					}
				}

				WingmanArray.showMenu--;
				WingmanArray.menuState = 0;
				break;
			}
		case 4: //attack
			{
				if (WingmanArray.wingmanId > -1)
				{
					WingmanArray.instances[WingmanArray.wingmanId].command = WINGCOM_ATTACK;
					WingmanArray.instances[WingmanArray.wingmanId].targetId = DATA_ObjectArray->instances[DATA_TargIndex].globalvars.unique_Id;

				} else
				{
					for (int i = 0; i < WingmanArray.Count; i++)
					{
						WingmanArray.instances[i].command = WINGCOM_ATTACK;
						WingmanArray.instances[i].targetId = DATA_ObjectArray->instances[DATA_TargIndex].globalvars.unique_Id;
					}
				}

				WingmanArray.showMenu--;
				WingmanArray.menuState = 0;
				break;
			}
		case 5: //auto
			{
				if (WingmanArray.wingmanId > -1)
				{
					WingmanArray.instances[WingmanArray.wingmanId].command = WINGCOM_AUTO;
				} else
				{
					for (int i = 0; i < WingmanArray.Count; i++)
					{
						WingmanArray.instances[i].command = WINGCOM_AUTO;
					}
				}

				WingmanArray.showMenu--;
				WingmanArray.menuState = 0;
				break;
			}
		}
	}

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
	bool notuse = false;
	char key[256];
	char *tok;

	CfgFindSection (cfg, "BINDS");

	for (int i=0;i<256;i++)
	{
		//Если комманда по индексу задана, то проверяем клавиши, в противном случае биндин NULL и едем дальше
		if (strcmp (Commands[i], "NULL") == 0) {notuse = true;}
		if (!notuse && CfgGetKeyStr (cfg, Commands[i], key, 255) != 0)
		{
			tok = strtok (key, " \n\t");
			strcpy (key, tok);

			for (int kc=0; kc<256;kc++)
			{
				if (strlen (key) > 0 && strcmp (keyTOdig[kc], key) == 0)
				{
					pKeybConvTable[i] = kc;
					bind = true;
				}
			}
		}

		if (notuse)
		{
			pKeybConvTable[i] = 0;
		}

		if (!notuse && !bind)
		{
			pKeybConvTable[i] = pKeybConvTableDef[i];
		}
			
		bind = false;
		notuse = false;
	}
}

void InputInit (void)
{
	int rval, i;
	CfgStruct cfg;
	GUID joyGuid;
	LPDIRECTINPUTDEVICE pDIJoy1;
	con_text[0] = '\0';

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
