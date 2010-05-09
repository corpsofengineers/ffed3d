#include "XfileEntity.h"
#include "Utility.h"
#include "MeshHierarchy.h"
#include "../Macros.h"

// The time to change from one animation set to another
// To see how the merging works - increase this time value to slow it down
const float kMoveTransitionTime=1.0f;

// Constructor
CXFileEntity::CXFileEntity(LPDIRECT3DDEVICE9 d3dDevice) : m_d3dDevice(d3dDevice),m_speedAdjust(1.0f),m_firstMesh(0),
	m_currentTrack(0),m_currentTime(0),m_numAnimationSets(0),m_currentAnimationSet(0),m_maxBones(0),m_sphereRadius(0),
	m_sphereCentre(0,0,0),m_boneMatrices(0)
{
	effect=NULL;
}

// Destructor
CXFileEntity::~CXFileEntity(void)
{
	if (m_animController)
	{
		m_animController->Release();
		m_animController=0;
	}

	if (m_frameRoot)
	{
		// Create a mesh heirarchy class to control the removal of memory for the frame heirarchy
		CMeshHierarchy memoryAllocator;
		D3DXFrameDestroy(m_frameRoot, &memoryAllocator);
		m_frameRoot=0;
	}

	if (m_boneMatrices)
	{
		delete []m_boneMatrices;
		m_boneMatrices=0;
	}
}

/*
	Load the x file
	The function D3DXLoadMeshHierarchyFromX requires a support object to handle memeory allocation etc.
	I have defined this in the class CMeshHierarchy
*/

bool CXFileEntity::LoadXFile(const std::string &filename,int startAnimation)
{
	if (!Load(filename))
	{
		return false;
	}

	SetAnimationSet(startAnimation);

	return true;
}

bool CXFileEntity::Load(const std::string &filename)
{
	// Create our mesh hierarchy class to control the allocation of memory - only used temporarily
	CMeshHierarchy *memoryAllocator=new CMeshHierarchy;

	// To make it easier to find the textures change the current directory to the one containing the .x file
	// First though remember the current one to put it back afterwards
	std::string currentDirectory=CUtility::GetTheCurrentDirectory();

	std::string xfilePath;
	CUtility::SplitPath(filename,&xfilePath,&m_filename);

	SetCurrentDirectory(xfilePath.c_str());

	// This is the function that does all the .x file loading. We provide a pointer to an instance of our 
	// memory allocator class to handle memory allocationm during the frame and mesh loading
	HRESULT hr = D3DXLoadMeshHierarchyFromX(filename.c_str(), D3DXMESH_MANAGED, m_d3dDevice, 
		memoryAllocator, NULL, &m_frameRoot, &m_animController);

	delete memoryAllocator;
	memoryAllocator=0;

	SetCurrentDirectory(currentDirectory.c_str());
	
	if (FAILED(hr))
		return false; 

	// if the x file contains any animation remember how many sets there are
	if(m_animController)
		m_numAnimationSets = m_animController->GetMaxNumAnimationSets();

	// Bones for skining
	if(m_frameRoot)
	{
		// Set the bones up
		SetupBoneMatrices((D3DXFRAME_EXTENDED*)m_frameRoot, NULL);

		// Create the bone matrices array for use during FrameMove to hold the final transform
		m_boneMatrices  = new D3DXMATRIX[m_maxBones];
		ZeroMemory(m_boneMatrices, sizeof(D3DXMATRIX)*m_maxBones);

		// Calculate the Bounding Sphere for this model (used in CalculateInitialViewMatrix to position camera correctly)
		D3DXFrameCalculateBoundingSphere(m_frameRoot, &m_sphereCentre, &m_sphereRadius);
	}

	m_firstMesh->MeshData.pMesh->GetVertexBuffer(&vb.vb);
	m_firstMesh->MeshData.pMesh->GetIndexBuffer(&ib.ib);

	D3DVERTEXELEMENT9 pDecl[MAX_FVF_DECL_SIZE];
	m_firstMesh->MeshData.pMesh->GetDeclaration(pDecl);
	renderSystem->CreateVertexDeclaration(&pDecl[0],&vb.declaration);
	// Получение данных о количестве вершин, индексов и полигонов
	dwNumVerticies	= m_firstMesh->MeshData.pMesh->GetNumVertices();
	dwNumIndecies	= m_firstMesh->MeshData.pMesh->GetNumFaces()*3;
	dwNumFaces		= m_firstMesh->MeshData.pMesh->GetNumFaces();
	vb.vertexSize = (short)m_firstMesh->MeshData.pMesh->GetNumBytesPerVertex();
	return true;
}

/*
	Since this demo can load many different sizes and shapes of 3d model setting the initial
	camera position so the model can be seen is a tricky task. This function uses the model's bounding sphere
	to come up with an initial position for the camera.
*/
D3DXVECTOR3 CXFileEntity::GetInitialCameraPosition() const
{
	D3DXVECTOR3 cameraPos(0.0f,m_sphereCentre.y,-(m_sphereRadius*3));
	return cameraPos;
}

/**
 * \brief we need to go through the hierarchy and set the combined matrices
 * calls itself recursively as it tareverses the hierarchy
 * \param device - the Direct3D device object
 * \param pFrame - current frame
 * \param pParentMatrix - the parent frame matrix
 * \author Keith Ditchburn \date 18 July 2005
*/
void CXFileEntity::SetupBoneMatrices(D3DXFRAME_EXTENDED *pFrame, LPD3DXMATRIX pParentMatrix)
{
	// Cast to our extended structure first
	D3DXMESHCONTAINER_EXTENDED* pMesh = (D3DXMESHCONTAINER_EXTENDED*)pFrame->pMeshContainer;

	// If this frame has a mesh
	if(pMesh)
	{
		// We need to remember which is the first mesh in the hierarchy for later when we 
		// update (FrameMove)
		if(!m_firstMesh)
			m_firstMesh = pMesh;
		
		// if there is skin info, then setup the bone matrices
		if(pMesh->pSkinInfo && pMesh->MeshData.pMesh)
		{
			// Create a copy of the mesh to skin into later
			D3DVERTEXELEMENT9 Declaration[MAX_FVF_DECL_SIZE];
			if (FAILED(pMesh->MeshData.pMesh->GetDeclaration(Declaration)))
				return;

			pMesh->MeshData.pMesh->CloneMesh(D3DXMESH_MANAGED, 
				Declaration, m_d3dDevice, 
				&pMesh->exSkinMesh);

			// Max bones is calculated for later use (to know how big to make the bone matrices array)
			m_maxBones=max(m_maxBones,(int)pMesh->pSkinInfo->GetNumBones());

			// For each bone work out its matrix
			for (unsigned int i = 0; i < pMesh->pSkinInfo->GetNumBones(); i++)
			{   
				// Find the frame containing the bone
				D3DXFRAME_EXTENDED* pTempFrame = (D3DXFRAME_EXTENDED*)D3DXFrameFind(m_frameRoot, 
						pMesh->pSkinInfo->GetBoneName(i));

				// set the bone part - point it at the transformation matrix
				pMesh->exFrameCombinedMatrixPointer[i] = &pTempFrame->exCombinedTransformationMatrix;
			}

		}
	}

	// Pass on to sibblings
	if(pFrame->pFrameSibling)
		SetupBoneMatrices((D3DXFRAME_EXTENDED*)pFrame->pFrameSibling, pParentMatrix);

	// Pass on to children
	if(pFrame->pFrameFirstChild)
		SetupBoneMatrices((D3DXFRAME_EXTENDED*)pFrame->pFrameFirstChild, &pFrame->exCombinedTransformationMatrix);
}


/**
 * \brief Called each frame update with the time and the current world matrix
 * \param elapsedTime - time passed
 * \param matWorld - current world matrix for the model
 * \author Keith Ditchburn \date 18 July 2005
*/

void CXFileEntity::FrameMove(bool mode, float elapsedTime,const D3DXMATRIX *matWorld)
{
	// Adjust animation speed
	//elapsedTime/=m_speedAdjust;

	// Advance the time and set in the controller
    //if (m_animController != NULL)
        //m_animController->AdvanceTime(elapsedTime, NULL);

	//m_animController->AdvanceTime(0.5f, NULL); 
	if (mode==0) {
		m_animController->SetTrackPosition(0, elapsedTime);
		m_animController->AdvanceTime(0, NULL); 
	} else {
		elapsedTime/=m_speedAdjust;
		m_animController->AdvanceTime(0.03f, NULL);
		m_currentTime+=0.01;
	}
	//m_currentTime+=0.01;//elapsedTime;
		//char cBuff[255];
		//sprintf(cBuff,"FFED3D track position: %.2f", elapsedTime);
		//SetWindowText(hWnd,cBuff);


	// Now update the model matrices in the hierarchy
    UpdateFrameMatrices(m_frameRoot, matWorld);

	// If the model contains a skinned mesh update the vertices
	D3DXMESHCONTAINER_EXTENDED* pMesh = m_firstMesh;
	if(pMesh && pMesh->pSkinInfo)
	{
		unsigned int Bones = pMesh->pSkinInfo->GetNumBones();

		// Create the bone matrices that transform each bone from bone space into character space
		// (via exFrameCombinedMatrixPointer) and also wraps the mesh around the bones using the bone offsets
		// in exBoneOffsetsArray
		for (unsigned int i = 0; i < Bones; ++i)
			D3DXMatrixMultiply(&m_boneMatrices[i],&pMesh->exBoneOffsets[i], pMesh->exFrameCombinedMatrixPointer[i]);

		// We need to modify the vertex positions based on the new bone matrices. This is achieved
		// by locking the vertex buffers and then calling UpdateSkinnedMesh. UpdateSkinnedMesh takes the
		// original vertex data (in pMesh->MeshData.pMesh), applies the matrices and writes the new vertices
		// out to skin mesh (pMesh->exSkinMesh). 

		// UpdateSkinnedMesh uses software skinning which is the slowest way of carrying out skinning 
		// but is easiest to describe and works on the majority of graphic devices. 
		// Other methods exist that use hardware to do this skinning - see the notes and the 
		// DirectX SDK skinned mesh sample for more details
		void *srcPtr=0;
		pMesh->MeshData.pMesh->LockVertexBuffer(D3DLOCK_READONLY, (void**)&srcPtr);

		void *destPtr=0;
		pMesh->exSkinMesh->LockVertexBuffer(0, (void**)&destPtr);

		// Update the skinned mesh 
		pMesh->pSkinInfo->UpdateSkinnedMesh(m_boneMatrices, NULL, srcPtr, destPtr);

		// Unlock the meshes vertex buffers
		pMesh->exSkinMesh->UnlockVertexBuffer();
		pMesh->MeshData.pMesh->UnlockVertexBuffer();
	}
}

D3DXMATRIX CXFileEntity::GetMatrix()
{
    D3DXFRAME_EXTENDED *currentFrame = (D3DXFRAME_EXTENDED*)m_frameRoot;

	//return currentFrame->TransformationMatrix;
	return currentFrame->exCombinedTransformationMatrix;
}
/**
 * \brief Called to update the frame matrices in the hierarchy to reflect current animation stage
 * \param frameBase - frame being looked at
 * \param parentMatrix - the matrix of our parent (if we have one)
 * \author Keith Ditchburn \date 18 July 2005
*/
void CXFileEntity::UpdateFrameMatrices(const D3DXFRAME *frameBase, const D3DXMATRIX *parentMatrix)
{
    D3DXFRAME_EXTENDED *currentFrame = (D3DXFRAME_EXTENDED*)frameBase;

	// If parent matrix exists multiply our frame matrix by it
    if (parentMatrix != NULL)
        D3DXMatrixMultiply(&currentFrame->exCombinedTransformationMatrix, &currentFrame->TransformationMatrix, parentMatrix);
    else
        currentFrame->exCombinedTransformationMatrix = currentFrame->TransformationMatrix;

	// If we have a sibling recurse 
    if (currentFrame->pFrameSibling != NULL)
        UpdateFrameMatrices(currentFrame->pFrameSibling, parentMatrix);

	// If we have a child recurse 
    if (currentFrame->pFrameFirstChild != NULL)
        UpdateFrameMatrices(currentFrame->pFrameFirstChild, &currentFrame->exCombinedTransformationMatrix);
}

/**
 * \brief Render our mesh.
 * Call the DrawFrame recursive fn on render with the root frame (see notes diagram)
 * \param device - the Direct3D device object
 * \author Keith Ditchburn \date 18 July 2005
*/
void CXFileEntity::Render()
{
	//D3DVERTEXELEMENT9 pDecl[MAX_FVF_DECL_SIZE];

	if (m_frameRoot) {
		//m_frameRoot->pMeshContainer->MeshData.pMesh->GetDeclaration(pDecl);
		//renderSystem->CreateVertexDeclaration(&pDecl[0],&vb.declaration);
		renderSystem->SetVertexDeclaration(vb.declaration);
		DrawFrame(m_frameRoot);
	}
}

void CXFileEntity::RenderM()
{
	renderSystem->DrawIndexedPrimitive(vb,0,dwNumVerticies,ib,0,dwNumFaces);
}

void CXFileEntity::Render(int vfrom, int vto, int ffrom, int fto)
{
	renderSystem->DrawIndexedPrimitive(vb, vfrom, vto, ib, ffrom, fto);
}
/**
 * \brief Called to render a frame in the hierarchy
 * \param device - the Direct3D device object
 * \param frame - frame to render
 * \author Keith Ditchburn \date 18 July 2005
*/
void CXFileEntity::DrawFrame(LPD3DXFRAME frame)
{
	// Draw all mesh containers in this frame
    LPD3DXMESHCONTAINER meshContainer = frame->pMeshContainer;
    while (meshContainer)
    {
        DrawMeshContainer(meshContainer, frame);
        meshContainer = meshContainer->pNextMeshContainer;
    }

	// Recurse for sibblings
    if (frame->pFrameSibling != NULL)
        DrawFrame(frame->pFrameSibling);

    // Recurse for children
	if (frame->pFrameFirstChild != NULL)
        DrawFrame(frame->pFrameFirstChild);
}

/**
 * \brief Called to render a mesh
 * \param device - the Direct3D device object
 * \param meshContainerBase - the mesh container
 * \param frameBase - frame containing the mesh
 * \author Keith Ditchburn \date 18 July 2005
*/
void CXFileEntity::DrawMeshContainer(LPD3DXMESHCONTAINER meshContainerBase, LPD3DXFRAME frameBase)
{
	DWORD attrSize = 0;
	// Cast to our extended frame type
	D3DXFRAME_EXTENDED *frame = (D3DXFRAME_EXTENDED*)frameBase;		

	// Cast to our extended mesh container
	D3DXMESHCONTAINER_EXTENDED *meshContainer = (D3DXMESHCONTAINER_EXTENDED*)meshContainerBase;
	
	// Set the world transform
    m_d3dDevice->SetTransform(D3DTS_WORLD, &frame->exCombinedTransformationMatrix);

	unsigned int pass;
	if (effect) {
		effect->SetMatrix("worldmat",&frame->exCombinedTransformationMatrix);

		effect->Begin(&pass,0);
		effect->BeginPass(0);
	}

	// Loop through all the materials in the mesh rendering each subset
    for (unsigned int iMaterial = 0; iMaterial < meshContainer->NumMaterials; iMaterial++)
    {
		// use the material in our extended data rather than the one in meshContainer->pMaterials[iMaterial].MatD3D
		//m_d3dDevice->SetMaterial( &meshContainer->exMaterials[iMaterial] );
		//m_d3dDevice->SetTexture( 0, meshContainer->exTextures[iMaterial] );

		// Select the mesh to draw, if there is skin then use the skinned mesh else the normal one
		LPD3DXMESH pDrawMesh = (meshContainer->pSkinInfo) ? meshContainer->exSkinMesh: meshContainer->MeshData.pMesh;

		// Finally Call the mesh draw function
        //pDrawMesh->DrawSubset(iMaterial);

		pDrawMesh->GetVertexBuffer(&vb.vb);
		pDrawMesh->GetIndexBuffer(&ib.ib);

		//D3DVERTEXELEMENT9 pDecl[MAX_FVF_DECL_SIZE];
		//pDrawMesh->GetDeclaration(pDecl);
		//renderSystem->CreateVertexDeclaration(&pDecl[0],&vb.declaration);
		// Получение данных о количестве вершин, индексов и полигонов
		dwNumVerticies	= pDrawMesh->GetNumVertices();
		dwNumIndecies	= pDrawMesh->GetNumFaces()*3;
		dwNumFaces		= pDrawMesh->GetNumFaces();
		vb.vertexSize = (short)pDrawMesh->GetNumBytesPerVertex();

		renderSystem->DrawIndexedPrimitive(vb,0,dwNumVerticies,ib,0,dwNumFaces);
    }

	if (effect) {
		effect->EndPass();
		effect->End();
	}
}

/**
 * \brief Change to a different animation set
 * Handles transitions between animations to make it smooth and not a sudden jerk to a new position
 * \param index - new animation set index
 * \author Keith Ditchburn \date 18 July 2005
*/
void CXFileEntity::SetAnimationSet(unsigned int index)
{
	if (index==m_currentAnimationSet)
		return;

	if (index>=m_numAnimationSets)
		index=0;

	// Remember current animation
	m_currentAnimationSet=index;

	// Get the animation set from the controller
	LPD3DXANIMATIONSET set;
	m_animController->GetAnimationSet(m_currentAnimationSet, &set );	

	// Note: for a smooth transition between animation sets we can use two tracks and assign the new set to the track
	// not currently playing then insert Keys into the KeyTrack to do the transition between the tracks
	// tracks can be mixed together so we can gradually change into the new animation

	// Alternate tracks
	DWORD newTrack = ( m_currentTrack == 0 ? 1 : 0 );

	// Assign to our track
	m_animController->SetTrackAnimationSet( newTrack, set );
    set->Release();	

	// Clear any track events currently assigned to our two tracks
	m_animController->UnkeyAllTrackEvents( m_currentTrack );
    m_animController->UnkeyAllTrackEvents( newTrack );

	// Add an event key to disable the currently playing track kMoveTransitionTime seconds in the future
    m_animController->KeyTrackEnable( m_currentTrack, FALSE, m_currentTime + kMoveTransitionTime );
	// Add an event key to change the speed right away so the animation completes in kMoveTransitionTime seconds
    m_animController->KeyTrackSpeed( m_currentTrack, 0.0f, m_currentTime, kMoveTransitionTime, D3DXTRANSITION_LINEAR );
	// Add an event to change the weighting of the current track (the effect it has blended with the secon track)
    m_animController->KeyTrackWeight( m_currentTrack, 0.0f, m_currentTime, kMoveTransitionTime, D3DXTRANSITION_LINEAR );

	// Enable the new track
    m_animController->SetTrackEnable( newTrack, TRUE );
	// Add an event key to set the speed of the track
    m_animController->KeyTrackSpeed( newTrack, 1.0f, m_currentTime, kMoveTransitionTime, D3DXTRANSITION_LINEAR );
	// Add an event to change the weighting of the current track (the effect it has blended with the first track)
	// As you can see this will go from 0 effect to total effect(1.0f) in kMoveTransitionTime seconds and the first track goes from 
	// total to 0.0f in the same time.
    m_animController->KeyTrackWeight( newTrack, 1.0f, m_currentTime, kMoveTransitionTime, D3DXTRANSITION_LINEAR );

	// Remember current track
    m_currentTrack = newTrack;
}

/**
 * \brief Go to the next animation
 * \author Keith Ditchburn \date 18 July 2005
*/
void CXFileEntity::NextAnimation()
{	
	unsigned int newAnimationSet=m_currentAnimationSet+1;
	if (newAnimationSet>=m_numAnimationSets)
		newAnimationSet=0;

	SetAnimationSet(newAnimationSet);
}


/**
 * \brief Get the name of the animation
 * Note: altered 24/09/07 to solve a D3DX memory leak caused because I was not releasing the set after getting it
 * \param index - the animation set index
 * \return the name
 * \author Keith Ditchburn \date 18 July 2005
*/
std::string CXFileEntity::GetAnimationSetName(unsigned int index)
{
	if (index>=m_numAnimationSets)
		return "Error: No set exists";

	// Get the animation set
	LPD3DXANIMATIONSET set;
	m_animController->GetAnimationSet(m_currentAnimationSet, &set );

	std::string nameString(set->GetName());

	set->Release();

	return nameString;
}

double CXFileEntity::GetAnimationSetLength(unsigned int index)
{
	if (index>=m_numAnimationSets)
		return 0;

	// Get the animation set
	LPD3DXANIMATIONSET set;
	m_animController->GetAnimationSet(m_currentAnimationSet, &set );

	double pr = set->GetPeriod();

	set->Release();

	return pr;
}

int CXFileEntity::GetNumAnimationSets()
{
	return m_numAnimationSets;
}

/**
 * \brief Slow down animation
 * \author Keith Ditchburn \date 18 July 2005
*/
void CXFileEntity::AnimateSlower()
{
	m_speedAdjust+=0.1f;
}

/**
 * \brief Speed up animation
 * \author Keith Ditchburn \date 18 July 2005
*/
void CXFileEntity::AnimateFaster()
{
	if (m_speedAdjust>0.1f)
		m_speedAdjust-=0.1f;
}

void CXFileEntity::ExportX(int exp)
{

	char buf[200];
	sprintf(buf,"models\\%i\\model_e.x",exp);
	int hr=D3DXSaveMeshToX(buf, m_firstMesh->MeshData.pMesh, NULL, NULL, NULL, 0, D3DXF_FILEFORMAT_TEXT);
}

void CXFileEntity::SetEffect(ID3DXEffect* effectF)
{
	effect = effectF;
}
