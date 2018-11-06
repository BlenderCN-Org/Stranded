

#ifndef __GUI16 
#define __GUI16 
 
#include <windows.h> 
#include <windowsx.h> 
#include <ddraw.h> 
#include <math.h> 
 
#pragma pack(1)  
struct BMPHEAD 
{	char identifier[2]; 
	DWORD fileSize; 
	DWORD reserved; 
	DWORD dataOffset; 
	DWORD headerSize; 
	DWORD width; 
	DWORD height; 
	WORD planes; 
	WORD bitsPerPixel; 
	DWORD compression; 
	DWORD dataSize; 
	DWORD hresolution; 
	DWORD vresolution; 
	DWORD colors; 
	DWORD importantColors; 
}; 
#pragma pack  
 
extern LPDIRECTDRAW            lpDD;           // DirectDraw object 
 
extern WORD RGBto16bit(DDPIXELFORMAT* pixFormat,unsigned char r,unsigned char g,unsigned char b); 
extern void writePixel(DDSURFACEDESC* ddsd,int x,int y,WORD color); 
extern void line(DDSURFACEDESC* ddsd,int x1,int y1,int x2,int y2,WORD color); 
extern void lineS(DDSURFACEDESC* ddsd,int x1,int y1,int x2,int y2,WORD color); 
extern void lineS2(DDSURFACEDESC* ddsd,int x1,int y1,int x2,int y2,WORD color); 
extern void circle(DDSURFACEDESC* ddsd,int x,int y,int r,WORD color); 
extern void circleFill(DDSURFACEDESC* ddsd,int x,int y,int r,WORD color); 
extern void ellipse(DDSURFACEDESC* ddsd,int xx1,int yy1,int xx2,int yy2,WORD color); 
extern void ellipseFill(DDSURFACEDESC* ddsd,int a1,int b1,int a2,int b2,WORD color); 
extern void drawRect(DDSURFACEDESC* ddsd,int x1,int y1,int x2,int y2,WORD color); 
extern void rectFill(DDSURFACEDESC* ddsd,int x1,int y1,int x2,int y2,WORD color); 
extern void drawPolygon(DDSURFACEDESC* ddsd,POINT* points,int num,WORD color); 
extern void polygonFill(DDSURFACEDESC* ddsd,POINT* points,int num,WORD color); 
extern bool loadBitmap(LPDIRECTDRAWSURFACE FAR * lpsurface,LPCSTR fileName); 
 
typedef struct { float x,y,z;}POINT3D; 
typedef struct { long x,y,z;}POINT3D_INT1; 
typedef struct { long x,y,z,u,v;}POINT3D_INT2; 
typedef struct { long x,y; 
				 float z,u,v;}POINT3D_INT3; 
typedef struct { float x,y,z;}VECTOR3D; 
typedef struct  
{	UINT p1,p2,p3,map,light; 
	float u1,v1,u2,v2,u3,v3; 
	float a,b,c,d; 
	BOOL maped; 
}TRIANGLE3D; 
typedef struct  
{	UINT num,light; 
	float a,b,c,d; 
	UINT* points; 
}POLYGON3D; 
typedef struct 
{	float e11,e12,e13,e14; 
	float e21,e22,e23,e24; 
	float e31,e32,e33,e34; 
	float e41,e42,e43,e44; 
}CONVERT_MATRIX3D; 
 
enum CPLCFLAG{CONVERTALL,CONVERTSIGNED}; 
 
class CPointList 
{	 
//private: 
public: 
	UINT topPos; 
	POINT3D* points; 
	CHAR* shouldConvert; 
	UINT size; 
public: 
	CPointList(UINT s); 
	~CPointList(); 
	BOOL operator!(); 
	BOOL copy(CPointList* l); 
	int addPoint(POINT3D* p); 
	int addPointAt(UINT pos,POINT3D* p); 
	void clearConvert(); 
	void convertPoints(CONVERT_MATRIX3D* m,CPLCFLAG f); 
	void scalePoints(CONVERT_MATRIX3D* m,CPLCFLAG f); 
	void movePoints(CONVERT_MATRIX3D* m,CPLCFLAG f); 
	void rotatePoints(CONVERT_MATRIX3D* m,CPLCFLAG f); 
}; 
 
class CFaceList 
{ 
//private: 
public: 
	UINT size; 
	UINT topPos; 
	TRIANGLE3D* faces; 
	CHAR* shouldConvert; 
public: 
	CFaceList(UINT s); 
	~CFaceList(); 
	BOOL operator!(); 
	void clearConvert(); 
	int addTriangle(TRIANGLE3D* p); 
	int addAsPolygon(POLYGON3D* p); 
	int addTriangleAt(UINT pos,TRIANGLE3D* p); 
	void updataParams(CPointList* ppl,CPLCFLAG f); 
	void judgeSide(CPointList* pl,POINT3D* pt); 
	void calLight(VECTOR3D* v,UINT groundLight,UINT spotLight,CPLCFLAG f); 
	void calLightSpot(POINT3D* pot,CPointList* pt,UINT groundLight,UINT spotLight,CPLCFLAG f); 
	void calLightMirror(POINT3D* pot,CPointList* pt,UINT groundLight,UINT spotLight,CPLCFLAG f); 
}; 
 
class CPolygonList 
{ 
//private: 
public: 
	UINT size; 
	UINT topPos; 
	POLYGON3D* faces; 
	CHAR* shouldConvert; 
public: 
	CPolygonList(UINT s); 
	~CPolygonList(); 
	BOOL operator!(); 
	void clearConvert(); 
	int addPolygon(POLYGON3D* p); 
	int addPolygonAt(UINT pos,POLYGON3D* p); 
	void updataParams(CPointList* ppl,CPLCFLAG f); 
	void judgeSide(CPointList* pl,POINT3D* pt); 
	void calLight(VECTOR3D* v,UINT groundLight,UINT spotLight,CPLCFLAG f); 
}; 
 
class CTexture 
{ 
//private: 
public: 
	UINT width,height,sectLenth; 
	BOOL level; 
	BOOL badf; 
	WORD* image; 
public: 
	CTexture(LPCSTR fileName); 
	CTexture(LPCSTR fileName,BOOL level); 
	~CTexture(); 
	BOOL bad(); 
}; 
/* 
class CObject3D 
{ 
//private: 
public: 
	long vertex_num; 
	long face_num; 
	long *vertexes; 
	long *faces; 
public: 
	CObject(long vNum,long fNum); 
} 
*/ 
class CTextureList 
{	//private: 
public: 
	UINT size; 
	UINT topPos; 
	CTexture** textures; 
public: 
	CTextureList(UINT s); 
	~CTextureList(); 
	BOOL operator!(); 
	int addTexture(LPCSTR fileName); 
	int addTextureAt(UINT pos,LPCSTR fileName); 
	int addLevelTexture(LPCSTR fileName); 
	int addLevelTextureAt(UINT pos,LPCSTR fileName); 
}; 
 
extern void vector3dToSTD(VECTOR3D* v); 
extern void MKProjectionMatrix(CONVERT_MATRIX3D* m,float eyePoint); 
extern void MKWCtoVcMatrix(CONVERT_MATRIX3D* m,VECTOR3D* n,POINT3D* vp); 
extern float vectorDotMul(VECTOR3D* v1,VECTOR3D* v2); 
extern inline void vectorForkMul(VECTOR3D* v1,VECTOR3D* v2,VECTOR3D* v); 
extern float getVectorMod(VECTOR3D* v); 
extern void matrixTranspose(CONVERT_MATRIX3D* s); 
extern inline void dotConvert(POINT3D* dot,CONVERT_MATRIX3D* m); 
extern void matrixMul(CONVERT_MATRIX3D* m1,CONVERT_MATRIX3D* m2); 
extern void MKMoveMatrix(CONVERT_MATRIX3D* m,VECTOR3D* v); 
extern void MKRotateMatrix(CONVERT_MATRIX3D* m,VECTOR3D* u1,float th); 
extern void MKRotateMatrix(CONVERT_MATRIX3D* m,POINT3D* p,VECTOR3D* u1,float th); 
extern void MKRotateMatrix(CONVERT_MATRIX3D* m,CONVERT_MATRIX3D* m2,POINT3D* p); 
extern void MKProjectionMatrix(CONVERT_MATRIX3D* m,float eyePoint,float w,float h); 
extern inline void drawTriangle3d(DDSURFACEDESC* ddsd,CPointList* p,TRIANGLE3D* t,WORD color); 
extern BOOL readDxfFile(LPCSTR fileName,CPointList* p,CFaceList* f); 
extern BOOL readDxfFile2(LPCSTR fileName,CPointList* p,CPolygonList* f); 
extern BOOL readAscFile(LPCSTR fileName,CPointList* p,CFaceList* f); 
extern void MKScaleMatrix(CONVERT_MATRIX3D* m,POINT3D* p,float rate); 
extern inline void dotScale(POINT3D* dot,CONVERT_MATRIX3D* m); 
extern inline void dotRotate(POINT3D* dot,CONVERT_MATRIX3D* m); 
extern void dotMove(POINT3D* dot,CONVERT_MATRIX3D* m); 
extern inline void fillTriangle3d(DDSURFACEDESC* ddsd,CPointList* ppl,TRIANGLE3D* t,WORD color); 
extern inline void fillTriangle3dWithZ(DDSURFACEDESC* ddsd,DDSURFACEDESC* ddsdZBuf,CPointList* ppl,TRIANGLE3D* t,WORD color); 
extern inline void fillTriangle3dWithZT(DDSURFACEDESC* ddsd,DDSURFACEDESC* ddsdZBuf,CPointList* ppl,TRIANGLE3D* t,WORD color); 
extern inline void fillTriangle3dWithZT1(DDSURFACEDESC* ddsd,DDSURFACEDESC* ddsdZBuf,CPointList* ppl,TRIANGLE3D* t,WORD color); 
extern inline void fillTriangle3dWithZLT(DDSURFACEDESC* ddsd,DDSURFACEDESC* ddsdZBuf,CPointList* ppl,TRIANGLE3D* t,UINT lightLevel); 
extern inline void fillTriangle3dWithZLT1(DDSURFACEDESC* ddsd,DDSURFACEDESC* ddsdZBuf,CPointList* ppl,TRIANGLE3D* t,UINT lightLevel); 
extern inline void fillTriangle3dWithT1(DDSURFACEDESC* ddsd,CPointList* ppl,TRIANGLE3D* t,WORD color); 
extern inline void fillTriangleParam(CPointList* p,TRIANGLE3D* t); 
extern inline void fillPolygonParam(CPointList* p,POLYGON3D* t); 
extern inline long fixedMul(long a,long b); 
extern inline long fixedDiv(long a,long b); 
 
extern UINT groundLight; 
extern UINT spotLight; 
extern WORD lightLevel[33]; 
extern VECTOR3D vLight; 
extern UINT errorLine; 
extern CTextureList tex; 
 
#define MAX_BORDER_NUM 20 
#define MAX_ZDEPTH 20000 
 
#define FIXED_1 0x00010000 
#define FIXED_half 0x00008000 
#define LONG_TO_FIXED(l)	(l*FIXED_1) 
#define FLOAT_TO_FIXED(f)	((long)(f*FIXED_1)) 
#define FIXED_TO_SHORT(f)	((short)(f>>16)) 
#define FIXED_TO_LONG(f)	(f>>16) 
//#define FIXED_TO_SHORT(f)	((short)((f+FIXED_half)>>16)) 
#define Bit16ToRGB(c)	(RGB( ((c)&0xf800)>>8, ((c)&0x07e0)>>3, ((c)&0x001f)<<3 )) 
 
#endif

