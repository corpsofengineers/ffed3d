#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include "meshstructures.h"

/**
 * \class CMeshHierarchy
 * \brief This class defines a number of callbacks required by the D3DXLoadMeshHierarchyFromX function.
 * The required functions are defined by the abstract interface class: ID3DXAllocateHierarchy
 * \author Keith Ditchburn \date 17 July 2005
*/
class CMeshHierarchy : public ID3DXAllocateHierarchy
{
public:	
    // callback to create a D3DXFRAME extended object and initialize it
    STDMETHOD( CreateFrame )(LPCSTR Name, LPD3DXFRAME *retNewFrame );

    // callback to create a D3DXMESHCONTAINER extended object and initialize it
    STDMETHOD( CreateMeshContainer )(LPCSTR Name, CONST D3DXMESHDATA * meshData, 
                            CONST D3DXMATERIAL * materials, CONST D3DXEFFECTINSTANCE * effectInstances,
                            DWORD numMaterials, CONST DWORD * adjacency, LPD3DXSKININFO skinInfo, 
                            LPD3DXMESHCONTAINER * retNewMeshContainer );

    // callback to release a D3DXFRAME extended object
    STDMETHOD( DestroyFrame )(LPD3DXFRAME frameToFree );

    // callback to release a D3DXMESHCONTAINER extended object
    STDMETHOD( DestroyMeshContainer )(LPD3DXMESHCONTAINER meshContainerToFree );
};