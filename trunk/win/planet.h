#define DIVIDER 10000

int control_dist=30000000;
int max_divide_deep=8;
int smart_optimizer=1;
int smart_optimizer_c=3;

extern "C" void **model;
extern "C" ffeVertex *DATA_009200; // Vertex buffer?

extern "C" char *DATA_007837;
extern "C" char *DATA_007838;
extern "C" char *DATA_007839;
extern "C" char *DATA_007840;
extern "C" char *DATA_007841;
extern "C" char *DATA_007843;
extern "C" char *DATA_007844;
extern "C" char *DATA_007845;
extern "C" char *DATA_007846;
extern "C" char *DATA_007847;
extern "C" char *DATA_007848;
extern "C" char *DATA_007849;
extern "C" char *DATA_007852;
extern "C" char *DATA_007854;
extern "C" char *DATA_007855;
extern "C" char *DATA_007856;
extern "C" char *DATA_007857;
extern "C" char *DATA_007858;
extern "C" char *DATA_007859;
extern "C" char *DATA_007860;
extern "C" char *DATA_007861;
extern "C" char *DATA_007862;
extern "C" char *DATA_007863;
extern "C" char *DATA_007864;
extern "C" char *DATA_007865;
extern "C" char *DATA_007866;
extern "C" char *DATA_007867;
extern "C" char *DATA_007868;
extern "C" char *DATA_007869;
extern "C" char *DATA_007870;
extern "C" char *DATA_007871;
extern "C" char *DATA_007872;
extern "C" char *DATA_007873;
extern "C" char *DATA_007874;
extern "C" char *DATA_007875;
extern "C" char *DATA_007876;
extern "C" char *DATA_007877;
extern "C" char *DATA_007878;
extern "C" char *DATA_007879;
extern "C" char *DATA_007880;
extern "C" char *DATA_007881;
extern "C" char *DATA_007882;
extern "C" char *DATA_007883;
extern "C" char *DATA_007884;
extern "C" char *DATA_007885;
extern "C" char *DATA_007886;
extern "C" char *DATA_007887;
extern "C" char *DATA_007888;
extern "C" char *DATA_007889;
extern "C" char *DATA_007890;
extern "C" char *DATA_007891;
extern "C" char *DATA_007892_Icosahedron;
extern "C" char *DATA_007893;
extern "C" char *DATA_007894;
extern "C" char *DATA_008812_GraphicsDetailRelated;
extern "C" char *DATA_009259;
extern "C" char *DATA_009260;
extern "C" char *DATA_009261;
extern "C" char *DATA_009262;
extern "C" char *DATA_009263;
extern "C" char *DATA_009264;
extern "C" char *DATA_009265;
extern "C" char *DATA_009267;
extern "C" char *DATA_009269;
extern "C" char *DATA_009270;
extern "C" char *DATA_009271;
extern "C" char *DATA_009272;
extern "C" char *DATA_009273;
extern "C" char *DATA_009274;
extern "C" char *DATA_009275;
extern "C" char *DATA_009276_ArraySize;
extern "C" char *DATA_009277;
extern "C" char *DATA_009278;
extern "C" char *DATA_009279;
extern "C" char *DATA_009280;
extern "C" char *DATA_009281;
extern "C" char *DATA_009282;
extern "C" char *DATA_009283;
extern "C" char *DATA_009284;
extern "C" char *DATA_009285;
extern "C" char *DATA_009286;
extern "C" char *DATA_009258;
extern "C" char *DATA_007842;

struct icoFaces {
	unsigned int points[3];
	unsigned int edges[3];
};

struct icoEdges {
	unsigned int points[2];
};

struct cLoDTri
{
  ffeVertex* p[3]; //points
  //cLoDTri* e[3]; //edges
  //cLoDTri* pParent; //parent
  cLoDTri* child[4]; //children
  cLoDTri()
  {
	  p[0]=NULL;
	  p[1]=NULL;
	  p[2]=NULL;
	  child[0]=NULL;
	  child[1]=NULL;
	  child[2]=NULL;
	  child[3]=NULL;
  };
};

struct planetLod
{
	int uid;
	cLoDTri tri[20];
};

planetLod pLods[50];
int lastPlanetLod=-1;

extern "C" int *FUNC_001840_addr;

extern "C" int FUNC_001326_DPal32to16(int);

extern "C" int FUNC_001874_DrawPlanet(unsigned char *DrawMdl, char *cmd);

extern "C" int FUNC_001465(int *);
int C_FUNC_001465_GetDist(ffeVector* A8);
extern "C" ffeVertex* FUNC_001470_getVertex(void *a, int num);
extern "C" ffeVertex* FUNC_001471(void *a, int num);
extern "C" ffeVertex* FUNC_001472(void *a, int num);
extern "C" ModelInstance_t * FUNC_001532_GetModelInstancePtr(unsigned char, void *);
extern "C" Model_t * FUNC_001538_GetModelPtr(int);
extern "C" void FUNC_001674_MatBuildOdd(void *, int, int);
extern "C" void FUNC_001341_Int64ArithShift(__int64 *, int);
extern "C" void FUNC_000574 (int *, __int64 *, void *, void *);

extern "C" void FUNC_001692(int *, int);
extern "C" void FUNC_001691(char *, int *, int *, int *, int *); 
extern "C" int FUNC_001777(int *, ffeVector *);
int C_FUNC_001777(int *Data, ffeVector* A8);
extern "C" int FUNC_001778(int, int, int);
extern "C" void FUNC_001804(void);
extern "C" void FUNC_001805(void);
extern "C" void FUNC_001806(void);
extern "C" void FUNC_001804(void);
extern "C" void FUNC_001807(void);
extern "C" void FUNC_001808(void);
extern "C" void FUNC_001877(char *, int *, int *, int, int, int);
extern "C" int FUNC_001819(int);
extern "C" int FUNC_001821(int);
extern "C" int FUNC_001825(int *);
int C_FUNC_001825_MaxUsedBitsInXYZ(int *A8);
extern "C" void FUNC_001826(ffeVector *, ffeVector *);
void C_FUNC_001826_NormalizeTo13Bits(ffeVector* New, ffeVector* A8);
extern "C" void FUNC_001827(ffeVector*, ffeVector*, ffeVector*, ffeVector*);
void C_FUNC_001827_CalculateNewVertexXYZ(ffeVector* New, ffeVector* A8, ffeVector* Ac, ffeVector* A10);
extern "C" int FUNC_001837(int, int, int);
extern "C" int FUNC_001838(int, int, int, int);
extern "C" int FUNC_001839(int, int, int, int, int);
extern "C" int FUNC_001467(int);
extern "C" int FUNC_001521(int, int);
extern "C" int FUNC_001656_FindMSB(int);
extern "C" void FUNC_001479(int *, int *);
extern "C" int FUNC_001824(int);
extern "C" int FUNC_001842(void);
extern "C" int FUNC_001823(int *, short *);
extern "C" int SetJmp(char *);
extern "C" int LongJmp(int *, int);
extern "C" void FUNC_001817(void);
extern "C" void FUNC_001816(void);
extern "C" void C_FUNC_001816_ArrayInit(void);
extern "C" ffeVertex** FUNC_001820(void);
ffeVertex** C_FUNC_001820_ArrayCreateNewElement(void);
int C_FUNC_001821_ArrayRemoveElement(ffeVertex** A8);
extern "C" int FUNC_001833(char *, int, int, int);
int C_FUNC_001833(DrawMdl_t *, int, int, int);
extern "C" void FUNC_001835(ffeVertex *, ffeVertex *, ffeVertex *, ffeVertex **, ffeVertex **, ffeVertex **, int, int, int, int, int);
void C_FUNC_001835(ffeVertex *, ffeVertex *, ffeVertex *, ffeVertex **, ffeVertex **, ffeVertex **, int, int, int, int, int);

extern "C" void FUNC_001834(ffeVertex *, ffeVertex *, ffeVertex *, int, int, int);
void C_FUNC_001834(ffeVertex *, ffeVertex *, ffeVertex *, int, int, int);
extern "C" int FUNC_001868(int *, int *, int *, int, int, int, int, int, int);
extern "C" void FUNC_001869(ffeVertex *, ffeVertex *, ffeVertex *, ffeVertex **, ffeVertex **, ffeVertex **, int, int, int);
inline void C_FUNC_001869(ffeVertex *, ffeVertex *, ffeVertex *, ffeVertex **, ffeVertex **, ffeVertex **, int, int, int);
extern "C" int FUNC_001870(int *, int *, int *, int, int, int, int, int, int);
extern "C" int FUNC_001871(int *, int *, int *, int, int, int, int, int, int);
extern "C" int FUNC_001872(int *, int *, int *, int, int, int, int, int, int);
extern "C" int FUNC_001873(int *, int *, int *, int, int, int, int, int, int);

extern "C" void FUNC_001829(ffeVertex*);
extern "C" inline void C_FUNC_001829_GetTriangulateDepth(ffeVertex*);
extern "C" inline void C_FUNC_001829_GetTriangulateDepth_2(ffeVertex* AN, ffeVertex* A8, ffeVertex* Ac, ffeVertex* A10);

extern "C" int FUNC_001849(int* A8, ffeVertex* Ac, ffeVertex* A10, ffeVertex* A14, int A18);
extern "C" int FUNC_001818();

Vect3f GetTriangeNormal(Vect3f* vVertex1, Vect3f* vVertex2, Vect3f* vVertex3);

void inline DrawCustomTriangle2(CUSTOMVERTEX *p1, CUSTOMVERTEX *p2, CUSTOMVERTEX *p3);
void SphereMap2(double x,double y,double z,float &u,float &v);

extern D3DXMATRIX mainRotMatrixO;
extern int currentModel;
extern Model_t *mod;

#define MAX_PLANET_VERT 100000
int maxTessVerts;
CUSTOMVERTEX tessVerts[3];
CUSTOMVERTEX newTessVerts[100][4];
//customVertex tessVerts2[MAX_PLANET_VERT];
char C_DATA_009274[MAX_PLANET_VERT * 72];
char C_DATA_009277[MAX_PLANET_VERT * 16];

