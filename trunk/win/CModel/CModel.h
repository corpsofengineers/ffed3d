/////////////////////////////////////////////////
//
// File: "CModel.h"
//
// Author: Jason Jurecka
//
// Creation Date: June 5, 2003
//
// Purpose: This is a wrapper for the DirectX functions
//	that work with models. This class when completed
//	will allow for loading, drawing, texturing, and
//	model animation.
//
// Note: This was written for DirectX version 9
//	Each instance of this class = one model
//
//		In the future could add functionality to 
//		load files in other formats for DirectX
//		and if I feel up too it OpenGL
/////////////////////////////////////////////////

#ifndef _MODEL_H_
#define _MODEL_H_

#include <d3dx9.h>
#include <d3dx9anim.h>
#include "..\Render\RenderSystem.h"

#include "..\CModel\CAllocateHierarchy.h"
#include "..\CModel\DerivedStructs.h"
#include "..\CModel\Macros.h"

#define SHADOWFVF D3DFVF_XYZRHW | D3DFVF_DIFFUSE //The FVF for the shadow vertex

class CModel  
{
private:


	LPDIRECT3DDEVICE9	m_pd3dDevice;			//The d3d device to use
	LPD3DXFRAME			m_pFrameRoot;			// Frame hierarchy of the model
	//Model
	LPD3DXMATRIX		m_pBoneMatrices;		// Used when calculating the bone position
	D3DXVECTOR3			m_vecCenter;			// Center of bounding sphere of object
    float               m_fRadius;				// Radius of bounding sphere of object
	UINT				m_uMaxBones;			// The Max number of bones for the model
	//Animation
	DWORD				m_dwCurrentAnimation;	// The current animation
	DWORD				m_dwAnimationSetCount;	// Number of animation sets
	LPD3DXANIMATIONCONTROLLER   m_pAnimController;// Controller for the animations

	////////////////////////////////////////////////
	//
	// Purpose: go through each frame and draw the
	//	ones that have a mesh container that is
	//	not NULL
	//
	// Parameters:
	//	LPFRAME pFrame				The frame root
	//
	// Notes: Called in the Draw Function
	//
	// Last Modified: July 7, 2003
	//
	////////////////////////////////////////////////
	void DrawFrame(LPFRAME pFrame);

	////////////////////////////////////////////////
	//
	// Purpose:
	//		set up the bone matrices
	//
	// Parameters:
	//	LPFRAME pFrame The frame root 
	//	LPD3DXMATRIX pParentMatrix	parent matrix
	//
	// Last Modified: July 7, 2003
	//
	////////////////////////////////////////////////
	void SetupBoneMatrices(LPFRAME pFrame, LPD3DXMATRIX pParentMatrix);

	////////////////////////////////////////////////
	//
	// Purpose:
	//		Update the frame matrices after animation
	//
	// Parameters:
	//		LPFRAME pFrame				Frame to use
	//		LPD3DXMATRIX pParentMatrix	Matrix passed in
	//
	// Notes:
	//		This function does not need to have a parent
	//			matrix passed when you are calling this function
	//			just pass the frame root
	//
	// Last Modified:
	//		July 7, 2003
	//
	////////////////////////////////////////////////
	void UpdateFrameMatrices(LPFRAME pFrame, LPD3DXMATRIX pParentMatrix);

public:
	sVertexBuffer				vb;
	sIndexBuffer				ib;
	LPMESHCONTAINER		m_pFirstMesh;			// The first mesh in the hierarchy

	//////////////////////////////////////////////////////////////////////////
	// Construction/Destruction
	//////////////////////////////////////////////////////////////////////////
	CModel(LPDIRECT3DDEVICE9 pD3DDevice);
	virtual ~CModel();
	CModel &operator=(const CModel&){}
	CModel(const CModel&){}

	//////////////////////////////////////////////////////////////////////////
	// Inline Functions
	//////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////
	//
	// Purpose:
	//		Return the center of the bounding sphere
	//
	// Returns:
	//		LPD3DXVECTOR3  Pointer to the center vector
	//
	// Last Modified:
	//		July 7, 2003
	//
	////////////////////////////////////////////////
	inline LPD3DXVECTOR3 GetBoundingSphereCenter()
	{ return &m_vecCenter; }

	////////////////////////////////////////////////
	//
	// Purpose:
	//		Return the Radius of the bounding sphere
	//
	// Returns:
	//		float	the radius of the bounding sphere
	//
	// Last Modified:
	//		July 7, 2003
	//
	////////////////////////////////////////////////
	inline float GetBoundingSphereRadius()
	{ return m_fRadius; }

	////////////////////////////////////////////////
	//
	// Purpose:
	//		Return the animation being used
	//
	// Returns:
	//		DWORD	the current animation
	//
	// Last Modified:
	//		Aug 6, 2003
	//
	////////////////////////////////////////////////
	inline DWORD GetCurrentAnimation()
	{ return m_dwCurrentAnimation; }

	//////////////////////////////////////////////////////////////////////////
	// Public Functions
	//////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////
	//
	// Purpose:
	//		Set the current animation to the passed in
	//			flag
	//
	// Parameters:
	//		DWORD dwAnimationFlag	The animation set to use
	//
	// Last Modified: Aug 6, 2003
	//
	////////////////////////////////////////////////
	void SetCurrentAnimation(DWORD dwAnimationFlag);
	
	////////////////////////////////////////////////
	//
	// Purpose:
	//		draw the model with the device given
	//
	// Last Modified: July 7, 2003
	//
	////////////////////////////////////////////////
	void Draw();
	void Draw(int vfrom, int vto, int ffrom, int fto);
	////////////////////////////////////////////////
	//
	// Purpose:
	//		Load the model from the .X file given
	//
	// Parameters:
	//	char*	strFileName		the file to load from
	//
	// Last Modified: Aug 6, 2003
	//
	////////////////////////////////////////////////
	bool LoadXFile(char* strFileName);

	////////////////////////////////////////////////
	//
	// Purpose:
	//		Update the model for animation
	//
	// Parameters:
	//	double dElapsedTime	the time
	//
	// Last Modified: Aug 6, 2003
	//
	////////////////////////////////////////////////
	void Update(double dElapsedTime);
};

#endif