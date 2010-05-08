/////////////////////////////////////////////////
//
// File: "CAllocateHierarchy.h"
//
// Author: Jason Jurecka
//
// Creation Date: June 9, 2003
//
// Purpose: This is an Allocation class that is
//		used with the D3DXLoadMeshHierarchyFromX
//		function. It handles the Creation and Deletion
//		of Frames and Mesh Containers. The overloaded
//		functions are callbacks so there is no need
//		to call any of the functions in written code
//		just pass an Object of this class to the function
/////////////////////////////////////////////////

#ifndef _ALLOCATE_HIERARCHY_H_
#define _ALLOCATE_HIERARCHY_H_

#include "DerivedStructs.h"
#include "Macros.h"

class CAllocateHierarchy: public ID3DXAllocateHierarchy
{
public:

	// Create a frame
    //1. The name of the frame
	//2. The output new frame
	STDMETHOD(CreateFrame)(THIS_ LPCTSTR Name, LPD3DXFRAME *ppNewFrame);
    
	// Create a Mesh Container
	//1. Name of the Mesh
	//2. The mesh Data
	//3. that materials of the mesh
	//4. the effects on the mesh
	//5. the number of meterials in the mesh
	//6. the adjacency array for the mesh
	//7. the skin information for the mesh
	//8. the output mesh container
	/*
	STDMETHOD(CreateMeshContainer)(THIS_ LPCTSTR Name, 
		LPD3DXMESHDATA pMeshData, LPD3DXMATERIAL pMaterials, 
		LPD3DXEFFECTINSTANCE pEffectInstances, DWORD NumMaterials, 
		DWORD *pAdjacency, LPD3DXSKININFO pSkinInfo, LPD3DXMESHCONTAINER *ppNewMeshContainer);
    */
    STDMETHOD(CreateMeshContainer)(THIS_ 
        LPCSTR Name, 
        CONST D3DXMESHDATA *pMeshData, 
        CONST D3DXMATERIAL *pMaterials, 
        CONST D3DXEFFECTINSTANCE *pEffectInstances, 
        DWORD NumMaterials, 
        CONST DWORD *pAdjacency, 
        LPD3DXSKININFO pSkinInfo, 
        LPD3DXMESHCONTAINER *ppNewMeshContainer);

	// Destroy a frame
	//1. The frame to delete
	STDMETHOD(DestroyFrame)(THIS_ LPD3DXFRAME pFrameToFree);
    
	// Destroy a mesh container
	//1. The container to destroy
	STDMETHOD(DestroyMeshContainer)(THIS_ LPD3DXMESHCONTAINER pMeshContainerBase);
};

#endif