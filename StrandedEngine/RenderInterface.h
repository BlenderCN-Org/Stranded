/*
* ��Ⱦϵͳ��������
*/

#ifndef __RENDER_INTERFACE_H__
#define __RENDER_INTERFACE_H__

#include "defines.h"
#include "material.h"
#include "light.h"
#include "GUI.h"
#include "vector.h"

class CRenderInterface
{
public:
	CRenderInterface() : m_screenWidth(0), m_screenHeight(0), m_fullscreen(false), m_mainHandle(NULL), m_near(0), m_far(0), m_guiList(nullptr), m_totalGUIs(0), m_totalFonts(0) {}
	virtual ~CRenderInterface(){}

	// ����Direct3D��Ⱦϵͳ
	virtual bool Initialize(int w, int h, WinHWND mainWin, bool fullscreen, STRANDED_MS_TYPE ms) = 0;

	virtual void OneTimeInit() = 0;
	// �����Ⱦϵͳ
	virtual void Shutdown() = 0;

	// ����̨������ɫ����Ϊָ������ɫ
	virtual void SetClearCol(float r, float g, float b) = 0;
	// �����³���
	virtual void StartRender(bool bColor, bool bDepth, bool bStencil) = 0;
	// ����Ⱦ����֮ǰ��֮���������������
	virtual void ClearBuffers(bool bColor, bool bDepth, bool bStencil) = 0;
	//��������
	virtual void EndRendering() = 0;

	//�����ʶ���ʩ�ӵ���Ⱦ�豸��
	virtual void SetMaterial(stMaterial *mat) = 0;
	
	//��Ӳ���д�����ע���Դ
	virtual void SetLight(stLight *light, int index) = 0;
	//�رյ�Դ[����]
	virtual void DisableLight(int index) = 0;

	// ��������ͶӰ����
	virtual void CalculateProjMatrix(float fov, float n, float f) = 0;
	// ��������ͶӰ����
	virtual void CalculateOrthoMatrix(float n, float f) = 0;
	// ���õ�ǰ�������/ģ�;���
	virtual void CalculateWorldMatrix(float x, float y, float z) = 0;
	// ��ת�������
	virtual void RotateWorldMatrix(float radian, E_ROTATION_DIR eRotation) = 0;
	// ������ͼ����(�����)
	virtual void CalculateViewMatrix(const CVector3& pos, const CVector3& view, const CVector3& up) = 0;

	// ����Ҫ���Ƶľ�̬���㻺��
	virtual int CreateStaticBuffer(VertexType, PrimType, unsigned int totalVerts, unsigned int totalIndices, int stride, void **data, unsigned int *indices, int *staticId) = 0;

	// ������������ʾ����Ļ��
	virtual int RenderStaticBuffer(unsigned int staticId) = 0;

	// ���� X ģ�������е� X ģ��
	virtual int RenderXModel(int xModelId) = 0;

	////////////////////////////////ģ��//////////////////////////////////////////
	// UMF ģ�ͺ� OBJ ģ��ֱ�Ӽ��ص���̬������
	// ���������ļ������洢������������λ�õ�����ID
	// UMF ģ�ͣ������ļ����ж�����񣬵ڶ���������ָ����Ҫ���ص���������
	virtual int LoadUMFAsStaticBuffer(const char* file, int meshIndex, int *staticID) = 0;
	// ���ؼ��غ�����ʹ���������滻����ģ��
	virtual int LoadUMFAsStaticBuffer(const char* file, int meshIndex, int staticID) = 0;
	virtual int LoadOBJAsStaticBuffer(const char* file, int *staticID) = 0;
	virtual int LoadOBJAsStaticBuffer(const char* file, int staticID) = 0;
	// X ģ�ͱ�����������
	virtual int LoadXModel(const char* file, int *xModelId) = 0;
	virtual int LoadXModel(const char* file, int xModelId) = 0;

	virtual void ReleaseAllXModels() = 0;
	virtual int ReleaseXModel(int xModelId) = 0;
	//////////////////////////////////////////////////////////////////////////
	virtual void ReleaseAllStaticBuffers() = 0;
	virtual int ReleaseStaticBuffer(unsigned int staticId) = 0;

	//����͸����
	virtual void SetTransparency(RenderState state, TransState src, TransState dst) = 0;

	//���2D����
	virtual int AddTexture2D(char *file, int *texId) = 0;

	virtual void SetTextureFilter(int index, int filter, int val) = 0;

	//���ö�������ͼ
	virtual void SetMultiTexture() = 0;

	virtual void ApplyTexture(int index, int texId) = 0;

	//������Ļ��ͼ
	virtual void SaveScreenShot(char *file) = 0;

	//���õ�״sprite
	virtual void EnablePointSprites(float size, float min, float a, float b, float c) = 0;

	//���õ�״sprite
	virtual void DisablePointSprites() = 0;

	///////////////////////////////////////GUI///////////////////////////////////
	virtual bool CreateText(char* font, int weight, bool italic, int size, int& id) = 0;
	virtual void DisplayText(int id, long x, long y, int r, int g, int b, char* text, ...) = 0;
	virtual void DisplayText(int id, long x, long y, unsigned long color, char* text, ...) = 0;

	// ����GUI��û�����Direct3D�Ĵ��룬��˻���CRenderInterface
	// �����ڲ���������ʵ�ִ���
	// If porting to OpenGL would probably stay the same.
	bool CreateGUI(int &id)
	{
		// �����µ�GUI���󣬲���ӵ�����
		if(!m_guiList)
		{
			m_guiList = new CGUISystem[1];
			if(!m_guiList)
				return STRANDED_FAIL;
		}
		else
		{
			CGUISystem* temp;
			temp = new CGUISystem[m_totalGUIs + 1];

			memcpy(temp, m_guiList, sizeof(CGUISystem) * m_totalGUIs);

			delete[] m_guiList;
			m_guiList = temp;
		}

		// �洢GUI����ID������ʹ��GUI
		id = m_totalGUIs;
		m_totalGUIs ++;

		return true;
	}

	void ReleaseAllGUIs()
	{
		for (int i = 0; i < m_totalGUIs; ++i)
		{
			m_guiList[i].Shutdown();
		}

		m_totalGUIs = 0;

		if (m_guiList)
		{
			delete[] m_guiList;
			m_guiList = nullptr;
		}
	}

	virtual void GetFPS() = 0;

	virtual bool AddGUIBackdrop(int guiId, char *fileName) = 0;
	virtual bool AddGUIStaticText(int guiId, int id, char *text,
		int x, int y, unsigned long color, int fontID) = 0;
	virtual bool AddGUIButton(int guiId, int id, int x, int y,
		char *up, char *over, char *down) = 0;
	virtual void ProcessGUI(int guiID, bool LMBDown,
		int mouseX, int mouseY,
		void (*funcPtr) (int id, int state)) = 0;
	///////////////////////////��//////////////////////////////////
	virtual void EnableFog(float start, float end, STRANDED_FOG_TYPE type, unsigned long color, bool rangeFog) = 0;
	virtual void DisableFog() = 0;
	//////////////////////////ϸ��ӳ��///////////////////////////////
	virtual void SetDetailMapping() = 0;

protected:
	// ��Ļ��ȡ��߶�
	int m_screenWidth, m_screenHeight;
	// �Ƿ���Ⱦ������Ļ��ʶ��
	bool m_fullscreen;

	// Direct3D��ʼ���������ھ��
	WinHWND m_mainHandle;
	
	// ͶӰ����Ľ�����ֵ��Զ����ֵ
	float m_near, m_far;

	//////////////////////////////////GUI////////////////////////////////////////
	// sGUI��������
	CGUISystem *m_guiList;
	// ȷ��ϵͳ��Ҫ��ʾ��GUI��������
	int m_totalGUIs;
	// ϵͳ��������
	int m_totalFonts;
	//////////////////////////////////////////////////////////////////////////
};

#endif