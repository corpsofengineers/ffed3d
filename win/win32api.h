#ifndef __WIN32API_H__
#define __WIN32API_H__
#include <windows.h>
#include <d3dx9.h>

void VideoInit();
void VideoCleanup();
void TimerInit();
void TimerCleanup();
void InputInit();
void InputCleanup();
void SoundInit();
void SoundCleanup();
void UpVolume();
void DownVolume();

int InitDirect3D(void);
int InitDirect3DData(void);
void ReleaseDirect3D(void);
void RenderDirect3D(void);

extern "C" void DirInit();
extern "C" void DirCleanup();

HWND Win32GetWnd (void);
HINSTANCE Win32GetInst (void);
void Win32MsgHandler (void);
void Win32InputReacquire (void);
void Win32SetMouseExclusive (void);
void Win32SetMouseNorm (void);
void Win32SoundReacquire (void);
void Win32TimerUpdate (void);
void Win32StreamUpdate (void);

#endif // __WIN32API_H__