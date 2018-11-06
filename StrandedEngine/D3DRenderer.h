/*
* 派生渲染类-实现具体渲染功能
*/

#ifndef __D3D_RENDERER_H__
#define __D3D_RENDERER_H__

#include <d3d9.h>
#include <d3dx9.h>
#include "RenderInterface.h"
#include "XMesh.h"


// Direct3D静态缓存
// 用顶点缓存将静态几何图形(非动画的)绘制到屏幕上
struct stD3DStaticBuffer
{
	stD3DStaticBuffer() : vbPtr(0), ibPtr(0), numVerts(0), numIndices(0),
		stride(0), fvf(0), primType(NULL_TYPE) {}

	// 顶点缓存
	LPDIRECT3DVERTEXBUFFER9 vbPtr;
	// 索引缓存(三角形索引)
	LPDIRECT3DINDEXBUFFER9 ibPtr;
	// 顶点计数
	int numVerts;
	// 索引计数
	int numIndices;
	// 单个顶点尺寸的幅度值
	int stride;
	// Direct3D顶点FVF
	unsigned long fvf;
	// 渲染静态缓存的图元类型
	PrimType primType;
};

struct stD3DTexture
{
	char *fileName;
	int width, height;
	LPDIRECT3DTEXTURE9 image;
};

/*
// XMesh.h 中的模型结构代替 [这里是静态网格]
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
	// 绘制 X 模型链表中的 X 模型
	int RenderXModel(int xModelId);

	////////////////////////////////模型//////////////////////////////////////////
	// UMF 模型和 OBJ 模型直接加载到静态缓存中
	// 参数包括文件名、存储网格在链表中位置的索引ID
	// UMF 模型，可在文件中有多个网格，第二个参数：指定将要加载的网格索引
	int LoadUMFAsStaticBuffer(const char* file, int meshIndex, int *staticID);
	// 重载加载函数，使用新数据替换现有模型
	int LoadUMFAsStaticBuffer(const char* file, int meshIndex, int staticID);
	int LoadOBJAsStaticBuffer(const char* file, int *staticID);
	int LoadOBJAsStaticBuffer(const char* file, int staticID);
	// X 模型保存在链表中
	int LoadXModel(const char* file, int *xModelId);
	int LoadXModel(const char* file, int xModelId);

	void ReleaseAllXModels();
	int ReleaseXModel(int xModelId);
	//////////////////////////////////////////////////////////////////////////

	void ReleaseAllStaticBuffers();
	int ReleaseStaticBuffer(unsigned int staticId);

	// 设置透明度
	void SetTransparency(RenderState state, TransState src, TransState dst);

	// 添加 2D 纹理
	int AddTexture2D(char *file, int *texId);

	void SetTextureFilter(int index, int filter, int val);

	// 设置多纹理贴图
	void SetMultiTexture();

	// 创建法线贴图
	bool CreateNormalMap(const char* file, LPDIRECT3DTEXTURE9 pNormalMap);

	void ApplyTexture(int index, int texId);

	// 保存屏幕截图
	void SaveScreenShot(char *file);

	// 启用点状 sprite
	void EnablePointSprites(float size, float min, float a, float b, float c);

	// 禁用点状 sprite
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
	/////////////////////////////////雾//////////////////////////////////////
	// 启用雾
	void EnableFog(float start, float end, STRANDED_FOG_TYPE type, unsigned long color, bool rangeFog);
	// 禁用雾
	void DisableFog();
	////////////////////////////细节映射////////////////////////////////////
	/*
	 * 3D场景显示显示细节(混凝土上的裂纹，粗糙墙面凸凹不平处)，不一定在纹理图像自身包含这种细节。
	 * 通常，一些彩色图纹理虽然从远处看很好，靠近时会让观察者觉得有些模糊.
	 * 细节映射技术：细节图是一个灰度图，当其和颜色图一起施加到表面时，最终的图像看上去比原始图像好像有很多细节，
	 * 且颜色图从上靠近看不模糊。细节图伟表面增加细节，给图增加小的裂纹以及线条，提高真实感.
	 * 加载两个文理图像，实现多纹理映射。合并两幅纹理图像时，通过一个有符号操作将细节图添加到基图上。 纹理图像中的
	 * 有符皓值从-127~128，无符号值从0~255. 细节图中较亮值会增亮基图，而较黑的值则产生反面的效果。只有0不会对最终的
	 * 图像产生任何效果，因为是正在添加图像而不是对图像做调制
	 */
	void SetDetailMapping();
	//////////////////////////////////////////////////////////////////////////
private:
	void OneTimeInit();
private:
	// 清除色
	D3DCOLOR m_Color;
	// Direct3D设备对象
	LPDIRECT3D9 m_Direct3D;
	LPDIRECT3DDEVICE9 m_Device;
	// 用于确认Direct3D9对象是否位于当前正在渲染的场景中的标识符
	bool m_renderingScene;

	// 保存所有静态缓存的数组链表
	stD3DStaticBuffer *m_staticBufferList;
	// 静态缓存数目的计数器
	int m_numStaticBuffers;
	// 正在使用的当前静态缓存变量(避免设置已经设置过的静态缓存)
	int m_activeStaticBuffer;

	// 纹理对象列表
	stD3DTexture *m_textureList;
	// 纹理对象计数器
	unsigned int m_numTextures;

	// 字体对象链表
	LPD3DXFONT* m_fonts;

	// X 模型链表
	CXModel *m_xModels;
	int m_numXModels;
};

// 为渲染系统分配内存
bool CreateD3DRenderer(CRenderInterface **pObj);

#endif