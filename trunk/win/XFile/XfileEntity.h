#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include <string>

#include "MeshStructures.h"
#include "..\Render\RenderSystem.h"
/*
	This class represents an x file animation
	It loads the .x file and carries out the animation update and rendering
*/
extern HWND hWnd;
class CXFileEntity
{
private:
	sVertexBuffer				vb;
	sIndexBuffer				ib;

	DWORD						dwNumVerticies;					// Количество вершин
	DWORD						dwNumIndecies;					// Количество индексов
	DWORD						dwNumFaces;						// Количество полигонов

	LPDIRECT3DDEVICE9 m_d3dDevice; // note: a pointer copy (not a good idea but for simplicities sake)

	// Direct3D objects required for animation
	LPD3DXFRAME                 m_frameRoot;
    LPD3DXANIMATIONCONTROLLER   m_animController;
	D3DXMESHCONTAINER_EXTENDED* m_firstMesh;

	ID3DXEffect* effect;

	// Bone data
	D3DXMATRIX *m_boneMatrices;
	int m_maxBones;
	
	// Animation variables
	unsigned int m_currentAnimationSet;	
	unsigned int m_numAnimationSets;
	unsigned int m_currentTrack;
	float m_currentTime;
	float m_speedAdjust;

	// Bounding sphere (for camera placement)
	D3DXVECTOR3 m_sphereCentre;
	float m_sphereRadius;

	std::string m_filename;

	void UpdateFrameMatrices(const D3DXFRAME *frameBase, const D3DXMATRIX *parentMatrix);
	void DrawFrame(LPD3DXFRAME frame);
	void DrawMeshContainer(LPD3DXMESHCONTAINER meshContainerBase, LPD3DXFRAME frameBase);
	void SetupBoneMatrices(D3DXFRAME_EXTENDED *pFrame, LPD3DXMATRIX pParentMatrix);	
public:

	CXFileEntity(LPDIRECT3DDEVICE9 d3dDevice);
	~CXFileEntity(void);

	bool LoadXFile(const std::string &filename,int startAnimation);
	bool Load(const std::string &filename);
	void ExportX(int exp);
	void FrameMove(bool mode, float elapsedTime,const D3DXMATRIX *matWorld);
	
	void Render();
	void RenderM();
	void Render(int vfrom, int vto, int ffrom, int fto);
	void SetAnimationSet(unsigned int index);

	void NextAnimation();
	void AnimateFaster();
	void AnimateSlower();

	D3DXVECTOR3 GetInitialCameraPosition() const;
	unsigned int GetCurrentAnimationSet() const {return m_currentAnimationSet;}
	std::string GetAnimationSetName(unsigned int index);
	double GetAnimationSetLength(unsigned int index);
	int GetNumAnimationSets();
	D3DXMATRIX GetMatrix();
	void SetEffect(ID3DXEffect* effectF);
	std::string GetFilename() const {return m_filename;}
};
