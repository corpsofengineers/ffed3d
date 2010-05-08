 /////////////////////////////////////////////////
//
// File: "CAllocateHierarchy.cpp"
//
// Author: Jason Jurecka
//
// Creation Date: June 9, 2003
//
// Purpose: This is an Allocation class that is
//		used with the D3DXLoadMeshHierarchyFromX
//		function. It handles the Creation and Deletion
//		of Frames and Mesh Containers. The overloaded
//		functions are callbacks, so there is no need
//		to call any of the functions in written code
//		just pass an Object of this class to the function
/////////////////////////////////////////////////
#include "..\CModel\CAllocateHierarchy.h"

HRESULT CAllocateHierarchy::CreateFrame(LPCTSTR Name, LPD3DXFRAME *ppNewFrame)
{    
	// Create a frame
	// Using my drived struct here
	LPFRAME pFrame = new FRAME;
	ZeroMemory(pFrame, sizeof(FRAME));

	// Inicilize the passed in frame
	*ppNewFrame = NULL;

	// Put the name in the frame
	if(Name)
	{
		int nNameSize = strlen(Name)+1;
		pFrame->Name = new char[nNameSize];
		memcpy(pFrame->Name, Name, nNameSize*sizeof(char));
	}
	else
		pFrame->Name = NULL;
	
	// Inicilize the rest of the frame
	pFrame->pFrameFirstChild = NULL;
	pFrame->pFrameSibling = NULL;
	pFrame->pMeshContainer = NULL;
	D3DXMatrixIdentity(&pFrame->matCombined);
	D3DXMatrixIdentity(&pFrame->TransformationMatrix);

	// Set the output frame to the one that we have
	*ppNewFrame = (LPD3DXFRAME)pFrame;

	// It no longer points to the frame
	pFrame = NULL;

	// Returns an HRESULT so give it the AOk result
    return S_OK;
}
/*
HRESULT CAllocateHierarchy::CreateMeshContainer(LPCTSTR Name, 
	LPD3DXMESHDATA pMeshData, LPD3DXMATERIAL pMaterials, 
	LPD3DXEFFECTINSTANCE pEffectInstances, DWORD NumMaterials, 
	DWORD *pAdjacency, LPD3DXSKININFO pSkinInfo, LPD3DXMESHCONTAINER *ppNewMeshContainer) 
*/
HRESULT CAllocateHierarchy::CreateMeshContainer(LPCTSTR Name, 
        CONST D3DXMESHDATA *pMeshData, 
        CONST D3DXMATERIAL *pMaterials, 
        CONST D3DXEFFECTINSTANCE *pEffectInstances, 
        DWORD NumMaterials, 
        CONST DWORD *pAdjacency, 
        LPD3DXSKININFO pSkinInfo, 
        LPD3DXMESHCONTAINER *ppNewMeshContainer)
{

    // Create a Temp mesh contianer
	// Using my drived struct here
	LPMESHCONTAINER pMeshContainer = new MESHCONTAINER;
	ZeroMemory(pMeshContainer, sizeof(MESHCONTAINER));

	// Inicialize passed in Container
	*ppNewMeshContainer = NULL;

	if(Name)
	{
		// Put in the name
		int nNameSize = strlen(Name)+1;
		pMeshContainer->Name = new char[nNameSize];
		memcpy(pMeshContainer->Name, Name, nNameSize*sizeof(char));
	}
	else
		pMeshContainer->Name = NULL;

	pMeshContainer->MeshData.Type = D3DXMESHTYPE_MESH;
	
	// Get the number of Faces for adjacency
	DWORD dwFaces = pMeshData->pMesh->GetNumFaces();

	//Get Initcilize all the other data
	pMeshContainer->NumMaterials = NumMaterials;

	//Create the arrays for the materials and the textures
	pMeshContainer->pMaterials9 = new D3DMATERIAL9[pMeshContainer->NumMaterials];

	// Multiply by 3 because there are three adjacent triangles
	pMeshContainer->pAdjacency = new DWORD[dwFaces*3];
	memcpy(pMeshContainer->pAdjacency, pAdjacency, sizeof(DWORD) * dwFaces*3);

	
	//Get the device to use
	LPDIRECT3DDEVICE9 pd3dDevice = NULL;// Direct3D Rendering device
	pMeshData->pMesh->GetDevice(&pd3dDevice);

	pMeshData->pMesh->CloneMeshFVF(D3DXMESH_MANAGED, 
		pMeshData->pMesh->GetFVF(), pd3dDevice, 
		&pMeshContainer->MeshData.pMesh);
	
	pMeshContainer->ppTextures  = new LPDIRECT3DTEXTURE9[NumMaterials];
	for(DWORD dw = 0; dw < NumMaterials; ++dw)
	{
		pMeshContainer->ppTextures [dw] = NULL;

		if(pMaterials[dw].pTextureFilename && strlen(pMaterials[dw].pTextureFilename) > 0)
		{
			if(FAILED(D3DXCreateTextureFromFile(pd3dDevice, 
				pMaterials[dw].pTextureFilename, &pMeshContainer->ppTextures[dw])))
					pMeshContainer->ppTextures [dw] = NULL;
		}
	}

	//Release the device
	SAFE_RELEASE(pd3dDevice);

	if(pSkinInfo)
	{
		// first save off the SkinInfo and original mesh data
	    pMeshContainer->pSkinInfo = pSkinInfo;
	    pSkinInfo->AddRef();

	    // Will need an array of offset matrices to move the vertices from 
		//	the figure space to the bone's space
	    UINT uBones = pSkinInfo->GetNumBones();
	    pMeshContainer->pBoneOffsets = new D3DXMATRIX[uBones];

		//Create the arrays for the bones and the frame matrices
		pMeshContainer->ppFrameMatrices = new D3DXMATRIX*[uBones];

	    // get each of the bone offset matrices so that we don't need to 
		//	get them later
	    for (UINT i = 0; i < uBones; i++)
	        pMeshContainer->pBoneOffsets[i] = *(pMeshContainer->pSkinInfo->GetBoneOffsetMatrix(i));
	}
	else
	
	{
		pMeshContainer->pSkinInfo = NULL;
		pMeshContainer->pBoneOffsets = NULL;
		pMeshContainer->pSkinMesh = NULL;
		pMeshContainer->ppFrameMatrices = NULL;
	}

	pMeshContainer->pMaterials = NULL;
	pMeshContainer->pEffects = NULL;

	//pMeshContainer->MeshData.pMesh->OptimizeInplace(
	//	D3DXMESHOPT_VERTEXCACHE|D3DXMESHOPT_COMPACT|D3DXMESHOPT_ATTRSORT,
	//	pMeshContainer->pAdjacency,NULL,NULL,NULL);

	// Set the output mesh container to the temp one
	*ppNewMeshContainer = pMeshContainer;
    pMeshContainer = NULL;

	// Returns an HRESULT so give it the AOk result
    return S_OK;
}

HRESULT CAllocateHierarchy::DestroyFrame(LPD3DXFRAME pFrameToFree) 
{
	//Convert the frame
	LPFRAME pFrame = (LPFRAME)pFrameToFree;

	// Delete the name
	SAFE_DELETE_ARRAY(pFrame->Name)
	
	// Delete the frame
    SAFE_DELETE(pFrame)

	// Returns an HRESULT so give it the AOk result
    return S_OK; 
}

HRESULT CAllocateHierarchy::DestroyMeshContainer(LPD3DXMESHCONTAINER pMeshContainerBase)
{
	//Convert to my derived struct type
    LPMESHCONTAINER pMeshContainer = (LPMESHCONTAINER)pMeshContainerBase;
	
	// if there is a name
	SAFE_DELETE_ARRAY(pMeshContainer->Name)

	//if there are materials
	SAFE_DELETE_ARRAY(pMeshContainer->pMaterials9)

	//Release the textures
	if(pMeshContainer->ppTextures)
		for(UINT i = 0; i < pMeshContainer->NumMaterials; ++i)
				SAFE_RELEASE(pMeshContainer->ppTextures[i]);

	//if there are textures
	SAFE_DELETE_ARRAY(pMeshContainer->ppTextures)

	// if there is adjacency data
	SAFE_DELETE_ARRAY(pMeshContainer->pAdjacency) 
	
	// if there are bone parts
	SAFE_DELETE_ARRAY(pMeshContainer->pBoneOffsets)
	
	//if there are frame matrices
	SAFE_DELETE_ARRAY(pMeshContainer->ppFrameMatrices)
	
	SAFE_DELETE_ARRAY(pMeshContainer->pAttributeTable)
	
	//if there is a copy of the mesh here
	SAFE_RELEASE(pMeshContainer->pSkinMesh)
	
	//if there is a mesh
	SAFE_RELEASE(pMeshContainer->MeshData.pMesh)
	
	// if there is skin information
	SAFE_RELEASE(pMeshContainer->pSkinInfo)
	
	//Delete the mesh container
	SAFE_DELETE(pMeshContainer)
	
	// Returns an HRESULT so give it the AOk result
    return S_OK;
}
