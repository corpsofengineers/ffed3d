#ifndef __VERTEX_FORMAT_H_INCLUDED__
#define __VERTEX_FORMAT_H_INCLUDED__
#include <d3dx9math.h>
#include "xmath.h"

#pragma pack(push,1)
//renderSystem->GetDevice()->SetFVF(D3DFVF_XYZW|D3DFVF_DIFFUSE|D3DFVF_TEX0);
struct VertexXYZ
{
	Vect3f pos;
	static struct IDirect3DVertexDeclaration9* declaration;
};

struct VertexXYZN : public VertexXYZ
{
	Vect3f n;
	static struct IDirect3DVertexDeclaration9* declaration;
};

struct VertexXYZND : public VertexXYZN
{
	Color4c difuse;
	static struct IDirect3DVertexDeclaration9* declaration;
};

struct VertexXYZNDT1 : public VertexXYZND
{
	float uv[2];
	inline Vect2f& GetTextel() {return *(Vect2f*)&uv[0];}
	inline float& u1(){return uv[0];}
	inline float& v1(){return uv[1];}
	static struct IDirect3DVertexDeclaration9* declaration;
};
struct VertexXYZW
{
	float			x,y,z,w;
	inline int& xi()					{ return *((int*)&x); }
	inline int& yi()					{ return *((int*)&y); }
	inline int& zi()					{ return *((int*)&z); }
	Vect3f& GetVect3f()					{ return *(Vect3f*)&x; }
	static struct IDirect3DVertexDeclaration9* declaration;
};

struct VertexXYZWD:public VertexXYZW
{
	Color4c	diffuse;
	static struct IDirect3DVertexDeclaration9* declaration;
};

struct VertexXYZWDT1 : public VertexXYZWD
{
	float			uv[2];
	inline float& u1()					{ return uv[0]; }
	inline float& v1()					{ return uv[1]; }
	inline Vect2f& GetTexel()			{ return *((Vect2f*)&uv[0]); }
	static struct IDirect3DVertexDeclaration9* declaration;
};

#pragma pack(pop)

#endif //__VERTEX_FORMAT_H_INCLUDED__