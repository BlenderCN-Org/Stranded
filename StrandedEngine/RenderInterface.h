/*
* 渲染系统基类声明
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

	// 设置Direct3D渲染系统
	virtual bool Initialize(int w, int h, WinHWND mainWin, bool fullscreen, STRANDED_MS_TYPE ms) = 0;

	virtual void OneTimeInit() = 0;
	// 清除渲染系统
	virtual void Shutdown() = 0;

	// 将后台缓存颜色设置为指定的颜色
	virtual void SetClearCol(float r, float g, float b) = 0;
	// 启动新场景
	virtual void StartRender(bool bColor, bool bDepth, bool bStencil) = 0;
	// 在渲染场景之前、之后或过程中清除场景
	virtual void ClearBuffers(bool bColor, bool bDepth, bool bStencil) = 0;
	//结束场景
	virtual void EndRendering() = 0;

	//将材质对象施加到渲染设备上
	virtual void SetMaterial(stMaterial *mat) = 0;
	
	//在硬件中创建和注册光源
	virtual void SetLight(stLight *light, int index) = 0;
	//关闭灯源[索引]
	virtual void DisableLight(int index) = 0;

	// 设置立体投影矩阵
	virtual void CalculateProjMatrix(float fov, float n, float f) = 0;
	// 设置正交投影矩阵
	virtual void CalculateOrthoMatrix(float n, float f) = 0;
	// 设置当前世界矩阵/模型矩阵
	virtual void CalculateWorldMatrix(float x, float y, float z) = 0;
	// 旋转世界矩阵
	virtual void RotateWorldMatrix(float radian, E_ROTATION_DIR eRotation) = 0;
	// 设置视图矩阵(摄像机)
	virtual void CalculateViewMatrix(const CVector3& pos, const CVector3& view, const CVector3& up) = 0;

	// 创建要绘制的静态顶点缓存
	virtual int CreateStaticBuffer(VertexType, PrimType, unsigned int totalVerts, unsigned int totalIndices, int stride, void **data, unsigned int *indices, int *staticId) = 0;

	// 将缓存内容显示在屏幕上
	virtual int RenderStaticBuffer(unsigned int staticId) = 0;

	// 绘制 X 模型链表中的 X 模型
	virtual int RenderXModel(int xModelId) = 0;

	////////////////////////////////模型//////////////////////////////////////////
	// UMF 模型和 OBJ 模型直接加载到静态缓存中
	// 参数包括文件名、存储网格在链表中位置的索引ID
	// UMF 模型，可在文件中有多个网格，第二个参数：指定将要加载的网格索引
	virtual int LoadUMFAsStaticBuffer(const char* file, int meshIndex, int *staticID) = 0;
	// 重载加载函数，使用新数据替换现有模型
	virtual int LoadUMFAsStaticBuffer(const char* file, int meshIndex, int staticID) = 0;
	virtual int LoadOBJAsStaticBuffer(const char* file, int *staticID) = 0;
	virtual int LoadOBJAsStaticBuffer(const char* file, int staticID) = 0;
	// X 模型保存在链表中
	virtual int LoadXModel(const char* file, int *xModelId) = 0;
	virtual int LoadXModel(const char* file, int xModelId) = 0;

	virtual void ReleaseAllXModels() = 0;
	virtual int ReleaseXModel(int xModelId) = 0;
	//////////////////////////////////////////////////////////////////////////
	virtual void ReleaseAllStaticBuffers() = 0;
	virtual int ReleaseStaticBuffer(unsigned int staticId) = 0;

	//设置透明度
	virtual void SetTransparency(RenderState state, TransState src, TransState dst) = 0;

	//添加2D纹理
	virtual int AddTexture2D(char *file, int *texId) = 0;

	virtual void SetTextureFilter(int index, int filter, int val) = 0;

	//设置多纹理贴图
	virtual void SetMultiTexture() = 0;

	virtual void ApplyTexture(int index, int texId) = 0;

	//保存屏幕截图
	virtual void SaveScreenShot(char *file) = 0;

	//启用点状sprite
	virtual void EnablePointSprites(float size, float min, float a, float b, float c) = 0;

	//禁用点状sprite
	virtual void DisablePointSprites() = 0;

	///////////////////////////////////////GUI///////////////////////////////////
	virtual bool CreateText(char* font, int weight, bool italic, int size, int& id) = 0;
	virtual void DisplayText(int id, long x, long y, int r, int g, int b, char* text, ...) = 0;
	virtual void DisplayText(int id, long x, long y, unsigned long color, char* text, ...) = 0;

	// 由于GUI类没有针对Direct3D的代码，因此基类CRenderInterface
	// 在其内部有完整的实现代码
	// If porting to OpenGL would probably stay the same.
	bool CreateGUI(int &id)
	{
		// 创建新的GUI对象，并添加到链表
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

		// 存储GUI对象ID，可以使用GUI
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
	///////////////////////////雾//////////////////////////////////
	virtual void EnableFog(float start, float end, STRANDED_FOG_TYPE type, unsigned long color, bool rangeFog) = 0;
	virtual void DisableFog() = 0;
	//////////////////////////细节映射///////////////////////////////
	virtual void SetDetailMapping() = 0;

protected:
	// 屏幕宽度、高度
	int m_screenWidth, m_screenHeight;
	// 是否渲染整个屏幕标识符
	bool m_fullscreen;

	// Direct3D初始化函数窗口句柄
	WinHWND m_mainHandle;
	
	// 投影矩阵的近距离值、远距离值
	float m_near, m_far;

	//////////////////////////////////GUI////////////////////////////////////////
	// sGUI界面链表
	CGUISystem *m_guiList;
	// 确定系统中要显示的GUI界面数量
	int m_totalGUIs;
	// 系统字体总数
	int m_totalFonts;
	//////////////////////////////////////////////////////////////////////////
};

#endif