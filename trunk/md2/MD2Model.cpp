
//#include <iostream>
#include <windows.h>
#include <commctrl.h>
#include <windowsx.h>

#include "MD2Model.h"

/* vector */
typedef float vec3_t[3];

static vec3_t anorms_table[162] = {
#include "anorms.h"
};


// Constructor
CMD2Model::CMD2Model () 
{
	m_index_list = NULL;
	m_frame_list = NULL;
	m_frames = m_vertices = m_triangles = 0;
}

// Destructor
CMD2Model::~CMD2Model () 
{
	
	if( m_frame_list != NULL ) 
	{
		for( int i = 0; i < m_frames; i++ )
			delete [] m_frame_list[i].vertex;
		delete [] m_frame_list;
	}
	
	if( m_index_list)
		delete [] m_index_list;
}


void CMD2Model::Destroy (void) 
{
	if( m_frame_list != NULL ) {
		
		for( int i = 0; i < m_frames; i++ )
			delete [] m_frame_list[i].vertex;
		
		delete [] m_frame_list;
		m_frame_list = NULL;
	}
	
	if( m_index_list != NULL ) {
		delete [] m_index_list;
		m_index_list = NULL;
	}
}

int CMD2Model::Load( char *filename ) 
{
	FILE		*modelfile = NULL;
	char		g_skins[MAX_MD2SKINS][64];
	dstvert_t	base_st[MAX_VERTS];
	byte		buffer[MAX_VERTS*4+128];
	SMD2Header	modelheader;
	
	dtriangle_t     tri;
	daliasframe_t	*out;
	
	if( (modelfile = fopen (filename, "rb")) == NULL )
		return 0;
	
	// Read the header
	fread( &modelheader, 1, sizeof(modelheader), modelfile );
	
	modelheader.framesize = (int)&((daliasframe_t *)0)->verts[modelheader.num_xyz];
	
	//copy some data
	m_frames     = modelheader.num_frames;
	m_vertices   = modelheader.num_xyz;
	m_triangles  = modelheader.num_tris;
	
	m_index_list = new make_index_list [modelheader.num_tris];
	m_frame_list = new make_frame_list [modelheader.num_frames];
	
	for( int i = 0; i < modelheader.num_frames; i++)
		m_frame_list[i].vertex = new make_vertex_list [modelheader.num_xyz];
	
	//read skin information
	fread( g_skins, 1, modelheader.num_skins * MAX_SKINNAME, modelfile );
	
	// read indice of the polygon meshes
	fread( base_st, 1, modelheader.num_st * sizeof(base_st[0]), modelfile );
	
	int	max_tex_u = 0, max_tex_v = 0;

	for( i = 0; i < modelheader.num_tris; i++ ) 
	{
		// read vertice
		fread( &tri, 1, sizeof(dtriangle_t), modelfile);
		
		(m_index_list)[i].a = tri.index_xyz[2];
		(m_index_list)[i].b = tri.index_xyz[1];
		(m_index_list)[i].c = tri.index_xyz[0];
	
		// read t&u
		(m_index_list)[i].a_s = base_st[tri.index_st[2]].s;
		(m_index_list)[i].a_t = base_st[tri.index_st[2]].t;
		(m_index_list)[i].b_s = base_st[tri.index_st[1]].s;
		(m_index_list)[i].b_t = base_st[tri.index_st[1]].t;
		(m_index_list)[i].c_s = base_st[tri.index_st[0]].s;
		(m_index_list)[i].c_t = base_st[tri.index_st[0]].t;
		max_tex_u = max( max_tex_u, base_st[tri.index_st[0]].s );
		max_tex_u = max( max_tex_u, base_st[tri.index_st[1]].s );
		max_tex_u = max( max_tex_u, base_st[tri.index_st[2]].s );
		max_tex_v = max( max_tex_v, base_st[tri.index_st[0]].t );
		max_tex_v = max( max_tex_v, base_st[tri.index_st[1]].t );
		max_tex_v = max( max_tex_v, base_st[tri.index_st[2]].t );
	}

	//convert t&u to be valid
	for ( i = 0; i < modelheader.num_tris; i++ ) 
	{
		m_index_list[ i ].a_s /= max_tex_u;
		m_index_list[ i ].b_s /= max_tex_u;
		m_index_list[ i ].c_s /= max_tex_u;
		m_index_list[ i ].a_t /= max_tex_v;
		m_index_list[ i ].b_t /= max_tex_v;
		m_index_list[ i ].c_t /= max_tex_v;
	}

	//g_D3D.m_toolz.FTrace ("Animation-names for : ");
	//g_D3D.m_toolz.FTrace (filename);
	//g_D3D.m_toolz.FTrace ("\n\n");

	// Read vertexdata from all animations
	for( i = 0; i < modelheader.num_frames; i++ ) 
	{
		out = (daliasframe_t *)buffer;
		fread( out, 1, modelheader.framesize, modelfile );

		//if (out->name)
		//	g_D3D.m_toolz.FTrace (out->name);
		//g_D3D.m_toolz.FTrace ("\n");
		
		for( int j = 0; j < modelheader.num_xyz; j++ ) 
		{
			(m_frame_list)[i].vertex[j].x = out->verts[j].v[0] * out->scale[0] + out->translate[0];
			(m_frame_list)[i].vertex[j].y = out->verts[j].v[1] * out->scale[1] + out->translate[1];
			(m_frame_list)[i].vertex[j].z = out->verts[j].v[2] * out->scale[2] + out->translate[2];
			(m_frame_list)[i].vertex[j].lightnormalindex = out->verts[j].lightnormalindex;
		}
	}
	
	fclose (modelfile);
	return Init();
}

int CMD2Model::Init() 
{
	D3DXVECTOR3 vNormal;
	
	// For each animation we use one SMesh
	for ( int i = 0; i < GetFrameCount(); i++ )
	{
		CUSTOMVERTEX pVertex;
		make_vertex_list *pvert;

		D3DXCOLOR	LightColor(1.0f, 1.0f, 1.0f, 1.0f );

		for(int j = 0; j < GetTriangleCount(); j++) 
		{
			vNormal = GetTriangeNormal(&D3DXVECTOR3(m_frame_list[i].vertex[m_index_list[j].a].x, m_frame_list[i].vertex[m_index_list[j].a].y, m_frame_list[i].vertex[m_index_list[j].a].z),
										&D3DXVECTOR3(m_frame_list[i].vertex[m_index_list[j].b].x, m_frame_list[i].vertex[m_index_list[j].b].y, m_frame_list[i].vertex[m_index_list[j].b].z),
										&D3DXVECTOR3(m_frame_list[i].vertex[m_index_list[j].c].x, m_frame_list[i].vertex[m_index_list[j].c].y, m_frame_list[i].vertex[m_index_list[j].c].z));

			pVertex.m_vecPos.x = m_frame_list[i].vertex[m_index_list[j].a].x;
			pVertex.m_vecPos.y = m_frame_list[i].vertex[m_index_list[j].a].z;
			pVertex.m_vecPos.z = m_frame_list[i].vertex[m_index_list[j].a].y;
			pvert = &m_frame_list[i].vertex[m_index_list[j].a];
			pVertex.m_vecNorm.x = anorms_table[pvert->lightnormalindex][0];
			pVertex.m_vecNorm.y = anorms_table[pvert->lightnormalindex][2];
			pVertex.m_vecNorm.z = anorms_table[pvert->lightnormalindex][1];
			pVertex.m_dwDiffuse = LightColor;
			pVertex.m_vecTex.x = m_index_list[j].a_s;
			pVertex.m_vecTex.y = m_index_list[j].a_t;
			m_data[i].vertex.push_back (pVertex);

			pVertex.m_vecPos.x = m_frame_list[i].vertex[m_index_list[j].b].x;
			pVertex.m_vecPos.y = m_frame_list[i].vertex[m_index_list[j].b].z;
			pVertex.m_vecPos.z = m_frame_list[i].vertex[m_index_list[j].b].y;
			pvert = &m_frame_list[i].vertex[m_index_list[j].b];
			pVertex.m_vecNorm.x = anorms_table[pvert->lightnormalindex][0];
			pVertex.m_vecNorm.y = anorms_table[pvert->lightnormalindex][2];
			pVertex.m_vecNorm.z = anorms_table[pvert->lightnormalindex][1];
			pVertex.m_dwDiffuse = LightColor;
			pVertex.m_vecTex.x = m_index_list[j].b_s;
			pVertex.m_vecTex.y = m_index_list[j].b_t;
			m_data[i].vertex.push_back (pVertex);

			pVertex.m_vecPos.x = m_frame_list[i].vertex[m_index_list[j].c].x;
			pVertex.m_vecPos.y = m_frame_list[i].vertex[m_index_list[j].c].z;
			pVertex.m_vecPos.z = m_frame_list[i].vertex[m_index_list[j].c].y;
			pvert = &m_frame_list[i].vertex[m_index_list[j].c];
			pVertex.m_vecNorm.x = anorms_table[pvert->lightnormalindex][0];
			pVertex.m_vecNorm.y = anorms_table[pvert->lightnormalindex][2];
			pVertex.m_vecNorm.z = anorms_table[pvert->lightnormalindex][1];
			pVertex.m_dwDiffuse = LightColor;
			pVertex.m_vecTex.x = m_index_list[j].c_s;
			pVertex.m_vecTex.y = m_index_list[j].c_t;
			m_data[i].vertex.push_back (pVertex);
		}

	}
	return 1;
}



//Draws me !
BOOL CMD2Model::Render( int frame ) 
{	
	CUSTOMVERTEX *Vertices;

	if( frame >= GetFrameCount()-1 )
		return 0;
	
	if(FAILED(d3d_vb->Lock(sizeof(CUSTOMVERTEX)*200000,sizeof(CUSTOMVERTEX)*GetTriangleCount()*3,(BYTE**)&Vertices,0))) return 0;

	memcpy(&Vertices->m_vecPos.x,&m_data[frame].vertex[0],sizeof(CUSTOMVERTEX)*GetTriangleCount()*3);

	d3d_vb->Unlock();

	d3d_device->DrawPrimitive(D3DPT_TRIANGLELIST, 200000, GetTriangleCount());


	return true;
}

BOOL CMD2Model::Render(char* name , BOOL b) 
{	
	CUSTOMVERTEX *Vertices;

	int play=-1;

	//Loop through all animation names
	for (int l=0;l<m_anim.size();l++)
	{
		//found it ???
		if (strcmp (m_anim[l].name.c_str(), name)==0)
		{
			if (b)
				m_anim[l].cur += m_anim[l].add;

			//restart animation
			if (m_anim[l].cur >= m_anim[l].end)
				m_anim[l].cur = m_anim[l].start;

			play=l;

			break;
		}
	}
	
	
	//d3d_device->SetVertexShader (D3DFVF_MODELVERTEX);

	if (play==-1) return FALSE;
	if (play>=GetFrameCount()) return FALSE;

	if(FAILED(d3d_vb->Lock(sizeof(CUSTOMVERTEX)*200000,sizeof(CUSTOMVERTEX)*GetTriangleCount()*3,(BYTE**)&Vertices,0))) return 0;

	memcpy(&Vertices->m_vecPos.x,&m_data[int(m_anim[l].cur)].vertex[0],sizeof(CUSTOMVERTEX)*GetTriangleCount()*3);

	d3d_device->DrawPrimitive(D3DPT_TRIANGLELIST, 200000, GetTriangleCount ());

	d3d_vb->Unlock();

	return true;
}
