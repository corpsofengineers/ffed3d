#pragma once

/**
 * \struct D3DXMESHCONTAINER_EXTENDED
 * \brief Structure derived from D3DXMESHCONTAINER and extended with some app-specific 
 * info that will be stored with each mesh. To help detect which data is extended and which is
 * from the base each variable begins with ex
 * \author Keith Ditchburn
*/
struct D3DXMESHCONTAINER_EXTENDED: public D3DXMESHCONTAINER
{
	// The base D3DXMESHCONTAINER has a pMaterials member which is a D3DXMATERIAL structure 
	// that contains a texture filename and material data. It is easier to ignore this and 
	// instead store the data in arrays of textures and materials in this extended structure:
    IDirect3DTexture9**  exTextures;		// Array of texture pointers  
	D3DMATERIAL9*		 exMaterials;		// Array of materials
                                
	// Skinned mesh variables
	ID3DXMesh*           exSkinMesh;			// The skin mesh
	D3DXMATRIX*			 exBoneOffsets;			// The bone matrix Offsets, one per bone
	D3DXMATRIX**		 exFrameCombinedMatrixPointer;	// Array of frame matrix pointers
};

/**
 * \struct D3DXFRAME_EXTENDED
 * \brief Structure derived from D3DXFRAME and extended so we can add some app-specific
 * info that will be stored with each frame. To help detect which data is extended and which is
 * from the base each variable begins with ex
 * \author Keith Ditchburn
*/
struct D3DXFRAME_EXTENDED: public D3DXFRAME
{
    D3DXMATRIX exCombinedTransformationMatrix;
};