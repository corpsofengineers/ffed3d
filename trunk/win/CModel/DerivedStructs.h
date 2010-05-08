/////////////////////////////////////////////////
//
// File: "DerivedStructs.h"
//
// Author: Jason Jurecka
//
// Creation Date: June 9, 2003
//
// Purpose: This is my derived struct of the 
//		D3DXMESHCONTAINER
/////////////////////////////////////////////////

#ifndef _DERIVED_STRUCTS_H_
#define _DERIVED_STRUCTS_H_

#pragma comment(lib, "d3dx9")
#include <d3dx9.h>
#include <d3dx9anim.h>

//Derived from the default mesh container
typedef struct _D3DXMESHCONTAINER_DERIVED: public D3DXMESHCONTAINER
{
	//Mesh variables
    LPDIRECT3DTEXTURE9*  ppTextures;		// Textures of the mesh
	D3DMATERIAL9*		 pMaterials9;		// Use the DirectX 9 Material type
	
	//Skinned mesh variables
	LPD3DXMESH           pSkinMesh;			// The skin mesh
	LPD3DXMATRIX		 pBoneOffsets;		// The bone matrix Offsets
	LPD3DXMATRIX*		 ppFrameMatrices;	// Pointer to the Frame Matrix

	// Attribute table stuff
	LPD3DXATTRIBUTERANGE pAttributeTable;	// The attribute table
    DWORD                NumAttributeGroups;// The number of attribute groups

}MESHCONTAINER, *LPMESHCONTAINER;

//Derived frame struct so it looks like a heirarchy
typedef struct _D3DXFRAME_DERIVED: public D3DXFRAME
{
	D3DXMATRIX matCombined;	//Combined Transformation Matrix

}FRAME, *LPFRAME;

#endif