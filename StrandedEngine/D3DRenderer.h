/*
* ������Ⱦ��-ʵ�־�����Ⱦ����
*/

#ifndef __D3D_RENDERER_H__
#define __D3D_RENDERER_H__

#include <d3d9.h>
#include <d3dx9.h>
#include "RenderInterface.h"
#include "XMesh.h"


// Direct3D��̬����
// �ö��㻺�潫��̬����ͼ��(�Ƕ�����)���Ƶ���Ļ��
struct stD3DStaticBuffer
{
	stD3DStaticBuffer() : vbPtr(0), ibPtr(0), numVerts(0), numIndices(0),
		stride(0), fvf(0), primType(NULL_TYPE) {}

	// ���㻺��
	LPDIRECT3DVERTEXBUFFER9 vbPtr;
	// ��������(����������)
	LPDIRECT3DINDEXBUFFER9 ibPtr;
	// �������
	int numVerts;
	// ��������
	int numIndices;
	// ��������ߴ�ķ���ֵ
	int stride;
	// Direct3D����FVF
	unsigned long fvf;
	// ��Ⱦ��̬�����ͼԪ����
	PrimType primType;
};

struct stD3DTexture
{
	char *fileName;
	int width, height;
	LPDIRECT3DTEXTURE9 image;
};

/*
// XMesh.h �е�ģ�ͽṹ���� [�����Ǿ�̬����]
class CXModel
{
public:
	CXModel() : model(NULL), numMaterials(0), matList(nullptr), textureList(nullptr) {}

	void Shutdown()
	{
		if (model != NULL)
		{
			model->Release();
			model = NULL;
		}

		if (textureList)
		{
			for (unsigned long i = 0; i < numMaterials; i++)
			{
				if (textureList[i] != nullptr)
				{
					textureList[i]->Release();
					textureList[i] = nullptr;
				}
			}

			delete[] textureList;
			textureList = nullptr;
		}

		if (matList != nullptr)
		{
			delete[] matList;
			matList = nullptr;
		}
	}

public:
	LPD3DXMESH model;
	unsigned long numMaterials;
	D3DMATERIAL9 *matList;
	LPDIRECT3DTEXTURE9 *textureList;
};
*/

class CD3DRenderer : public CRenderInterface
{
public:
	CD3DRenderer();
	~CD3DRenderer();

	bool Initialize(int w, int h, WinHWND mainWin, bool fullscreen, STRANDED_MS_TYPE ms);
	void Shutdown();

	void SetClearCol(float r, float g, float b);
	void StartRender(bool bColor, bool bDepth, bool bStencil);
	void ClearBuffers(bool bColor, bool bDepth, bool bStencil);
	void EndRendering();

	void SetMaterial(stMaterial *mat);

	void SetLight(stLight *light, int index);
	void DisableLight(int index);

	void CalculateProjMatrix(float fov, float n, float f);
	void CalculateOrthoMatrix(float n, float f);
	void CalculateWorldMatrix(float x, float y, float z);
	void RotateWorldMatrix(float radian, E_ROTATION_DIR eRotation);
	void CalculateViewMatrix(const CVector3& pos, const CVector3& view, const CVector3& up);

	int CreateStaticBuffer(VertexType, PrimType, unsigned int totalVerts, unsigned int totalIndices, int stride, void **data, unsigned int *indices, int *staticId);

	int RenderStaticBuffer(unsigned int staticId);
	// ���� X ģ�������е� X ģ��
	int RenderXModel(int xModelId);

	////////////////////////////////ģ��//////////////////////////////////////////
	// UMF ģ�ͺ� OBJ ģ��ֱ�Ӽ��ص���̬������
	// ���������ļ������洢������������λ�õ�����ID
	// UMF ģ�ͣ������ļ����ж�����񣬵ڶ���������ָ����Ҫ���ص���������
	int LoadUMFAsStaticBuffer(const char* file, int meshIndex, int *staticID);
	// ���ؼ��غ�����ʹ���������滻����ģ��
	int LoadUMFAsStaticBuffer(const char* file, int meshIndex, int staticID);
	int LoadOBJAsStaticBuffer(const char* file, int *staticID);
	int LoadOBJAsStaticBuffer(const char* file, int staticID);
	// X ģ�ͱ�����������
	int LoadXModel(const char* file, int *xModelId);
	int LoadXModel(const char* file, int xModelId);

	void ReleaseAllXModels();
	int ReleaseXModel(int xModelId);
	//////////////////////////////////////////////////////////////////////////

	void ReleaseAllStaticBuffers();
	int ReleaseStaticBuffer(unsigned int staticId);

	// ����͸����
	void SetTransparency(RenderState state, TransState src, TransState dst);

	// ��� 2D ����
	int AddTexture2D(char *file, int *texId);

	void SetTextureFilter(int index, int filter, int val);

	// ���ö�������ͼ
	void SetMultiTexture();

	// ����������ͼ
	bool CreateNormalMap(const char* file, LPDIRECT3DTEXTURE9 pNormalMap);

	void ApplyTexture(int index, int texId);

	// ������Ļ��ͼ
	void SaveScreenShot(char *file);

	// ���õ�״ sprite
	void EnablePointSprites(float size, float min, float a, float b, float c);

	// ���õ�״ sprite
	void DisablePointSprites();

	/////////////////////////////////GUI/////////////////////////////////////////
	bool CreateText(char* font, int weight, bool italic, int size, int& id);
	void DisplayText(int id, long x, long y, int r, int g, int b, char* text, ...);
	void DisplayText(int id, long x, long y, unsigned long color, char* text, ...);

	void GetFPS();

	bool AddGUIBackdrop(int guiId, char *fileName);
	bool AddGUIStaticText(int guiId, int id, char *text, int x, int y, unsigned long color, int fontID);
	bool AddGUIButton(int guiId, int id, int x, int y, char *up, char *over, char *down);
	void ProcessGUI(int guiID, bool LMBDown, int mouseX, int mouseY, void (*funcPtr) (int id, int state));
	/////////////////////////////////��//////////////////////////////////////
	// ������
	void EnableFog(float start, float end, STRANDED_FOG_TYPE type, unsigned long color, bool rangeFog);
	// ������
	void DisableFog();
	////////////////////////////ϸ��ӳ��////////////////////////////////////
	/*
	 * 3D������ʾ��ʾϸ��(�������ϵ����ƣ��ֲ�ǽ��͹����ƽ��)����һ��������ͼ�������������ϸ�ڡ�
	 * ͨ����һЩ��ɫͼ������Ȼ��Զ�����ܺã�����ʱ���ù۲��߾�����Щģ��.
	 * ϸ��ӳ�似����ϸ��ͼ��һ���Ҷ�ͼ���������ɫͼһ��ʩ�ӵ�����ʱ�����յ�ͼ����ȥ��ԭʼͼ������кܶ�ϸ�ڣ�
	 * ����ɫͼ���Ͽ�������ģ����ϸ��ͼΰ��������ϸ�ڣ���ͼ����С�������Լ������������ʵ��.
	 * ������������ͼ��ʵ�ֶ�����ӳ�䡣�ϲ���������ͼ��ʱ��ͨ��һ���з��Ų�����ϸ��ͼ��ӵ���ͼ�ϡ� ����ͼ���е�
	 * �з��ֵ��-127~128���޷���ֵ��0~255. ϸ��ͼ�н���ֵ��������ͼ�����Ϻڵ�ֵ����������Ч����ֻ��0��������յ�
	 * ͼ������κ�Ч������Ϊ���������ͼ������Ƕ�ͼ��������
	 */
	void SetDetailMapping();
	//////////////////////////////////////////////////////////////////////////
private:
	void OneTimeInit();
private:
	// ���ɫ
	D3DCOLOR m_Color;
	// Direct3D�豸����
	LPDIRECT3D9 m_Direct3D;
	LPDIRECT3DDEVICE9 m_Device;
	// ����ȷ��Direct3D9�����Ƿ�λ�ڵ�ǰ������Ⱦ�ĳ����еı�ʶ��
	bool m_renderingScene;

	// �������о�̬�������������
	stD3DStaticBuffer *m_staticBufferList;
	// ��̬������Ŀ�ļ�����
	int m_numStaticBuffers;
	// ����ʹ�õĵ�ǰ��̬�������(���������Ѿ����ù��ľ�̬����)
	int m_activeStaticBuffer;

	// ��������б�
	stD3DTexture *m_textureList;
	// ������������
	unsigned int m_numTextures;

	// �����������
	LPD3DXFONT* m_fonts;

	// X ģ������
	CXModel *m_xModels;
	int m_numXModels;
};

// Ϊ��Ⱦϵͳ�����ڴ�
bool CreateD3DRenderer(CRenderInterface **pObj);

#endif