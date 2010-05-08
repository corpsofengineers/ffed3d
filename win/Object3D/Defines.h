#pragma once
#include <d3dx9.h>
struct VERTEX_3DPNT1 { D3DXVECTOR3 position, normal; D3DXVECTOR2 texture; };
#define D3DFVF_3DPNT1 ( D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1)

struct VERTEX_2DPNT1 { float x, y, z, rhw, tx, ty; };
#define D3DFVF_2DPNT1 ( D3DFVF_XYZRHW | D3DFVF_TEX1 )

//#define KEYDOWN(key)	(dinput->keyboard.IsButD(key))		    
//#define AXISDELTA(axis) (directinput8.mouse.GetDeltaPos(axis))	   	
//#define JOYARROW(axis)	(directinput8.joystick.IsAxisD(axis)) 	   

