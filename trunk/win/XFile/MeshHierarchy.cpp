#include "MeshHierarchy.h"
#include "Utility.h"

/**
 * \brief callback called when a new frame is encountered during the .x file load
 * \param Name - name of the frame
 * \param ppNewFrame - output pointer assign our newly created frame
 * \return success code
 * \author Keith Ditchburn \date 17 July 2005
*/
HRESULT CMeshHierarchy::CreateFrame(LPCSTR Name, LPD3DXFRAME *retNewFrame)
{
	// Always a good idea to initialise a return pointer before proceeding
	*retNewFrame = 0;

	// Create a new frame using the derived version of the structure
    D3DXFRAME_EXTENDED *newFrame = new D3DXFRAME_EXTENDED;
	ZeroMemory(newFrame,sizeof(D3DXFRAME_EXTENDED));

	// Now fill in the data members in the frame structure
	
    // Now initialize other data members of the frame to defaults
    D3DXMatrixIdentity(&newFrame->TransformationMatrix);
    D3DXMatrixIdentity(&newFrame->exCombinedTransformationMatrix);

	newFrame->pMeshContainer = 0;
    newFrame->pFrameSibling = 0;
    newFrame->pFrameFirstChild = 0;

	// Assign the return pointer to our newly created frame
    *retNewFrame = newFrame;
	
	// The frame name (note: may be 0 or zero length)
	if (Name && strlen(Name))
	{
		newFrame->Name=CUtility::DuplicateCharString(Name);	
		CUtility::DebugString("Added frame: "+ToString(Name)+"\n");
	}
	else
	{
		CUtility::DebugString("Added frame: no name given\n");
	}
    return S_OK;
}

/**
 * \brief callback called when a mesh data is encountered during the .x file load
 * \param Name - name of the Mesh (const char*)
 * \param meshData - the mesh data
 * \param materials - material array
 * \param effectInstances - effect files / settings for the mesh
 * \param numMaterials - number of materials in the mesh
 * \param adjacency - adjacency array 
 * \param pSkinInfo - skin info.
 * \param retNewMeshContainer - output pointer to assign our newly created mesh container
 * \return success code
 * \author Keith Ditchburn \date 17 July 2005
*/
HRESULT CMeshHierarchy::CreateMeshContainer(
    LPCSTR Name,
    CONST D3DXMESHDATA *meshData,
    CONST D3DXMATERIAL *materials,
    CONST D3DXEFFECTINSTANCE *effectInstances,
    DWORD numMaterials,
    CONST DWORD *adjacency,
    LPD3DXSKININFO pSkinInfo,
    LPD3DXMESHCONTAINER* retNewMeshContainer)
{    
	// Create a mesh container structure to fill and initilaise to zero values
	// Note: I use my extended version of the structure (D3DXMESHCONTAINER_EXTENDED) defined in MeshStructures.h
	D3DXMESHCONTAINER_EXTENDED *newMeshContainer=new D3DXMESHCONTAINER_EXTENDED;
	ZeroMemory(newMeshContainer, sizeof(D3DXMESHCONTAINER_EXTENDED));

	// Always a good idea to initialise return pointer before proceeding
	*retNewMeshContainer = 0;

	// The mesh name (may be 0) needs copying over
	if (Name && strlen(Name))
	{
		newMeshContainer->Name=CUtility::DuplicateCharString(Name);
		CUtility::DebugString("Added mesh: "+ToString(Name)+"\n");
	}
	else
	{
		CUtility::DebugString("Added Mesh: no name given\n");
	}

	// The mesh type (D3DXMESHTYPE_MESH, D3DXMESHTYPE_PMESH or D3DXMESHTYPE_PATCHMESH)
	if (meshData->Type!=D3DXMESHTYPE_MESH)
	{
		// This demo does not handle mesh types other than the standard
		// Other types are D3DXMESHTYPE_PMESH (progressive mesh) and D3DXMESHTYPE_PATCHMESH (patch mesh)
		DestroyMeshContainer(newMeshContainer);
		return E_FAIL;
	}

	newMeshContainer->MeshData.Type = D3DXMESHTYPE_MESH;
	
	// Adjacency data - holds information about triangle adjacency, required by the ID3DMESH object
	DWORD dwFaces = meshData->pMesh->GetNumFaces();
	newMeshContainer->pAdjacency = new DWORD[dwFaces*3];
	memcpy(newMeshContainer->pAdjacency, adjacency, sizeof(DWORD) * dwFaces*3);
	
	// Get the Direct3D device, luckily this is held in the mesh itself (Note: must release it when done with it)
	LPDIRECT3DDEVICE9 pd3dDevice = 0;
	meshData->pMesh->GetDevice(&pd3dDevice);

	// Changed 24/09/07 - can just assign pointer and add a ref rather than need to clone
	newMeshContainer->MeshData.pMesh=meshData->pMesh;
	newMeshContainer->MeshData.pMesh->AddRef();

	// Create material and texture arrays. Note that I always want to have at least one
	newMeshContainer->NumMaterials = max(numMaterials,1);
	newMeshContainer->exMaterials = new D3DMATERIAL9[newMeshContainer->NumMaterials];
	newMeshContainer->exTextures  = new LPDIRECT3DTEXTURE9[newMeshContainer->NumMaterials];

	ZeroMemory(newMeshContainer->exTextures, sizeof(LPDIRECT3DTEXTURE9) * newMeshContainer->NumMaterials);

	if (numMaterials>0)
	{
		// Load all the textures and copy the materials over		
		for(DWORD i = 0; i < numMaterials; ++i)
		{
			newMeshContainer->exTextures[i] = 0;	
			newMeshContainer->exMaterials[i]=materials[i].MatD3D;

			if(materials[i].pTextureFilename)
			{
				std::string texturePath(materials[i].pTextureFilename);
				if (CUtility::FindFile(&texturePath))
				{
					// Use the D3DX function to load the texture
					if(FAILED(D3DXCreateTextureFromFile(pd3dDevice, texturePath.c_str(),
						&newMeshContainer->exTextures[i])))
					{
						CUtility::DebugString("Could not load texture: "+texturePath+"\n");					
					}
				}
				else
				{
					CUtility::DebugString("Could not find texture: "+ToString(materials[i].pTextureFilename)+"\n");					
				}
			}
		}
	}
	else    
	// make a default material in the case where the mesh did not provide one
    {
		ZeroMemory(&newMeshContainer->exMaterials[0], sizeof( D3DMATERIAL9 ) );
        newMeshContainer->exMaterials[0].Diffuse.r = 0.5f;
        newMeshContainer->exMaterials[0].Diffuse.g = 0.5f;
        newMeshContainer->exMaterials[0].Diffuse.b = 0.5f;
        newMeshContainer->exMaterials[0].Specular = newMeshContainer->exMaterials[0].Diffuse;
		newMeshContainer->exTextures[0]=0;
    }

	// If there is skin data associated with the mesh copy it over
	if (pSkinInfo)
	{
		// save off the SkinInfo
	    newMeshContainer->pSkinInfo = pSkinInfo;
	    pSkinInfo->AddRef();

	    // Need an array of offset matrices to move the vertices from the figure space to the bone's space
	    UINT numBones = pSkinInfo->GetNumBones();
	    newMeshContainer->exBoneOffsets = new D3DXMATRIX[numBones];

		// Create the arrays for the bones and the frame matrices
		newMeshContainer->exFrameCombinedMatrixPointer = new D3DXMATRIX*[numBones];

	    // get each of the bone offset matrices so that we don't need to get them later
	    for (UINT i = 0; i < numBones; i++)
	        newMeshContainer->exBoneOffsets[i] = *(newMeshContainer->pSkinInfo->GetBoneOffsetMatrix(i));

		CUtility::DebugString("Mesh has skinning info.\n Number of bones is: "+ToString(numBones)+"\n");
        // Note: in the Microsoft samples a GenerateSkinnedMesh function is called here in order to prepare
		// the skinned mesh data for optimial hardware acceleration. As mentioned in the notes this sample
		// does not do hardware skinning but instead uses software skinning.
	}
	else	
	{
		// No skin info so 0 all the pointers
		newMeshContainer->pSkinInfo = 0;
		newMeshContainer->exBoneOffsets = 0;
		newMeshContainer->exSkinMesh = 0;
		newMeshContainer->exFrameCombinedMatrixPointer = 0;
	}

	// When we got the device we caused an internal reference count to be incremented
	// So we now need to release it
	pd3dDevice->Release();

	// The mesh may contain a reference to an effect file
	if (effectInstances)
	{
		if (effectInstances->pEffectFilename)
			CUtility::DebugString("This .x file references an effect file. Effect files are not handled by this demo\n");
	}
	
	// Set the output mesh container pointer to our newly created one
	*retNewMeshContainer = newMeshContainer;    

	return S_OK;
}

/**
 * \brief callback called to deallocate the frame data
 * \param the frame to free
 * \return success result
 * \author Keith Ditchburn \date 17 July 2005
*/
HRESULT CMeshHierarchy::DestroyFrame(LPD3DXFRAME frameToFree) 
{
	// Convert to our extended type. OK to do this as we know for sure it is:
	D3DXFRAME_EXTENDED *frame = (D3DXFRAME_EXTENDED*)frameToFree;

	if (frame->Name)
		delete []frame->Name;
	delete frame;

    return S_OK; 
}

/**
 * \brief callback called to deallocate the mesh container data
 * \param the mesh data to free
 * \return success result
 * \author Keith Ditchburn \date 17 July 2005
*/
HRESULT CMeshHierarchy::DestroyMeshContainer(LPD3DXMESHCONTAINER meshContainerBase)
{
	// Convert to our extended type. OK as we know for sure it is:
    D3DXMESHCONTAINER_EXTENDED* meshContainer = (D3DXMESHCONTAINER_EXTENDED*)meshContainerBase;
	if (!meshContainer)
		return S_OK;

	// name
	if (meshContainer->Name)
	{
		delete []meshContainer->Name;
		meshContainer->Name=0;
	}

	// material array
	if (meshContainer->exMaterials)
	{
		delete []meshContainer->exMaterials;
		meshContainer->exMaterials=0;
	}

	// release the textures before deleting the array
	if(meshContainer->exTextures)
	{
		for(UINT i = 0; i < meshContainer->NumMaterials; ++i)
		{
			if (meshContainer->exTextures[i])
				meshContainer->exTextures[i]->Release();
		}
	}

	// texture array
	if (meshContainer->exTextures)
		delete []meshContainer->exTextures;

	// adjacency data
	if (meshContainer->pAdjacency)
		delete []meshContainer->pAdjacency;
	
	// bone parts
	if (meshContainer->exBoneOffsets)
	{
		delete []meshContainer->exBoneOffsets;
		meshContainer->exBoneOffsets=0;
	}
	
	// frame matrices
	if (meshContainer->exFrameCombinedMatrixPointer)
	{
		delete []meshContainer->exFrameCombinedMatrixPointer;
		meshContainer->exFrameCombinedMatrixPointer=0;
	}
	
	// release skin mesh
	if (meshContainer->exSkinMesh)
	{
		meshContainer->exSkinMesh->Release();
		meshContainer->exSkinMesh=0;
	}
	
	// release the main mesh
	if (meshContainer->MeshData.pMesh)
	{
		meshContainer->MeshData.pMesh->Release();
		meshContainer->MeshData.pMesh=0;
	}
		
	// release skin information
	if (meshContainer->pSkinInfo)
	{
		meshContainer->pSkinInfo->Release();
		meshContainer->pSkinInfo=0;
	}
	
	// finally delete the mesh container itself
	delete meshContainer;
	meshContainer=0;

    return S_OK;
}
