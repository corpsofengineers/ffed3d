///////////////////////////////////////////////////////////
//                                                       //
// PC MAGAZIN - PC Underground                           //
//                                                       //
// Direct3DX Basisprogramm und CMD2Model Viewer           //
//                                                       //
///////////////////////////////////////////////////////////
#include <stdio.h>
//#include <iostream>
#include <windows.h>
#include <commctrl.h>
#include <windowsx.h>
#include <math.h>
#include <d3dx9.h>
//include "d3d!.h"
#include <vector>
#include <string>

//extern CDirect3D g_D3D;

#ifndef _CMD2Model_H
#define _CMD2Model_H

extern LPDIRECT3DDEVICE9 d3d_device;
extern LPDIRECT3DVERTEXBUFFER9 d3d_vb;

extern D3DVECTOR GetTriangeNormal(D3DXVECTOR3* vVertex1, D3DXVECTOR3* vVertex2, D3DXVECTOR3* vVertex3);

// The definitions are from ID sources !!!
const int MAX_TRIANGLES = 32768;
const int MAX_VERTS		= 16384;
const int MAX_FRAMES	= 512;
const int MAX_MD2SKINS	= 32;
const int MAX_SKINNAME	= 64;

//used to identify an animation
struct SAnimation
{
	//start and end of an animation
	int start, end;
	//Name
	std::string name;
	//current frame of the animation
	float cur;
	//increase =faster, decrease =slower animation
	float add;
};

class CMD2Model
{
private :
	struct make_index_list
	{
		int a, b, c;
		float	a_s, a_t,
				b_s, b_t,
				c_s, c_t;
	};

	struct make_vertex_list
	{
		float x, y, z;
		int   lightnormalindex;
	};

	struct make_frame_list
	{
		make_vertex_list *vertex;
	};

	typedef unsigned char byte;

	struct vec3_t
	{
		float v[3];
	};


	struct dstvert_t
	{
		short s;
		short t;
	};

	struct dtriangle_t
	{
		  short index_xyz[3];
		  short index_st[3];
	};


	struct dtrivertx_t
	{
		byte v[3];
		byte lightnormalindex;
	};

	//Identify frames
	struct daliasframe_t
	{
		float scale[3];
		float translate[3];
		char name[16];
		dtrivertx_t verts[1];
	};

	struct SMD2Header
	{
	  int ident;
	  int version;

	  int skinwidth;			//Width and height of frame
	  int skinheight;			//Should be dividable by 8

	  int framesize;

	  int num_skins;			//Number of skintextures
	  int num_xyz;			
	  int num_st;
	  int num_tris;				//Count of triangles
	  int num_glcmds;
	  int num_frames;			//Count of fames

	//Some file offsets
	  int ofs_skins;
	  int ofs_st;
	  int ofs_tris;
	  int ofs_frames;
	  int ofs_glcmds; 
	  int ofs_end;

	} ;

	typedef struct
	{
			vec3_t          v;
			int   lightnormalindex;
	} trivert_t;

	typedef struct
	{
			vec3_t          mins, maxs;
			char            name[16];
			trivert_t       v[MAX_VERTS];
	} frame_t;

	
	// The modellvertex for D3D
	/*
	struct MODELVERTEX 
	{
	  D3DXVECTOR3   m_vecPos;
	  D3DCOLOR      m_dwDiffuse;
	  D3DXVECTOR2   m_vecTex;
	} ;
	#define D3DFVF_MODELVERTEX ( D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE2(0) )
*/
	struct CUSTOMVERTEX {
		D3DXVECTOR3 m_vecPos;
		D3DXVECTOR3 m_vecNorm;
		D3DCOLOR      m_dwDiffuse;
		D3DXVECTOR2   m_vecTex;

	};
	#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_DIFFUSE|D3DFVF_TEX1)

	struct SMesh
	{
		std::vector <CUSTOMVERTEX> vertex;
	};


	make_index_list* m_index_list;
	make_frame_list* m_frame_list;
	long m_frames, m_vertices, m_triangles;
	//Save vertice
	SMesh			 m_data [MAX_FRAMES];
	//Called by Load
	int				 Init();
public:

	//Look @comments for SAnimation
	//Must be filled externally (see GameInit in main.cpp)
	std::vector <SAnimation> m_anim;

	float scale;
	int cullmode;

	CMD2Model();
	~CMD2Model();
	
	//Loads the file 
	BOOL				Load (char* );
	//Frees memory
	void				Destroy ();
	//Draws frame nr frame :-)
	BOOL				Render (int frame);
	//Plays the animation by name and "advances" it but only
	//if advance==TRUE
	//if advance==FALSE the animation stops !
	BOOL				Render (char* name, BOOL advance=TRUE);

	inline int			GetFrameCount()		{ return    m_frames;	}
	inline int			GetVertexCount()	{ return	m_vertices; }
	inline int			GetTriangleCount()	{ return	m_triangles;} 
};

#endif