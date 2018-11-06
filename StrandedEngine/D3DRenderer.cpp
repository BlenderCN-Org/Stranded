#include <stdio.h>
#include "D3DRenderer.h"
#include "structs.h"
#include "UMFLoader.h"
#include "objLoader.h"

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable: 4018)
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#endif

// Our custom FVF, which describes our custom vertex structure
#define D3DFVF_GUI		(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1)
#define D3DFVF_MV		(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX1)

//////////////////////////////////////////////////////////////////////////
//创建新渲染对象，存储在基类指针
bool CreateD3DRenderer(CRenderInterface **pObj)
{
	if(!*pObj)
		*pObj = new CD3DRenderer;
	else
		return false;

	return true;
}

/*
 * 检查是否支持所希望的多采样类型. 成功返回 D3DMULTISAMPLE_TYPE，否则返回 D3DMULTISAMPLE_NONE
 */
static D3DMULTISAMPLE_TYPE GetD3DMultiSampleType(LPDIRECT3D9 d3d, STRANDED_MS_TYPE ms, D3DDEVTYPE type, D3DFORMAT format, bool fullscreen)
{
	D3DMULTISAMPLE_TYPE ret = D3DMULTISAMPLE_NONE;

	if (d3d)
	{
		switch (ms)
		{
		case STRANDED_MS_NONE:
			ret = D3DMULTISAMPLE_NONE;
			break;
		case STRANDED_MS_SAMPLES_2:
			/*
			 * @brief: 确定硬件是否支持某种采样类型[使用该函数从最大采样了性知道最小采样类型循环，获取硬件支持采样类型]
			 * @return: 支持，返回D3D_OK; 否则，返回D3D_FAIL
			 * @param:
					正在使用的显卡、将要创建的设备类型、正在使用的渲染格式、确定是否为全屏模式的标识符、
					要查看的多采样类型、存储由MultiSampleType参数多采样类型支持的质量等级最大数的变量
			 * HRESULT CheckDeviceMultiSampleType(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SurfaceFormat, BOOL Windowed, D3DMULTISAMPLE_TYPE MultiSampleType, DWORD* pQualityLevels);
			 */
			if (d3d->CheckDeviceMultiSampleType(D3DADAPTER_DEFAULT, type, format, !fullscreen, D3DMULTISAMPLE_2_SAMPLES, nullptr) == D3D_OK)
				ret = D3DMULTISAMPLE_2_SAMPLES;
			break;
		case STRANDED_MS_SAMPLES_4:
			if (d3d->CheckDeviceMultiSampleType(D3DADAPTER_DEFAULT, type, format, !fullscreen, D3DMULTISAMPLE_4_SAMPLES, nullptr) == D3D_OK)
				ret = D3DMULTISAMPLE_4_SAMPLES;
			break;
		case STRANDED_MS_SAMPLES_8:
			if (d3d->CheckDeviceMultiSampleType(D3DADAPTER_DEFAULT, type, format, !fullscreen, D3DMULTISAMPLE_8_SAMPLES, nullptr) == D3D_OK)
				ret = D3DMULTISAMPLE_8_SAMPLES;
			break;
		case STRANDED_MS_SAMPLES_16:
			if (d3d->CheckDeviceMultiSampleType(D3DADAPTER_DEFAULT, type, format, !fullscreen, D3DMULTISAMPLE_16_SAMPLES, nullptr) == D3D_OK)
				ret = D3DMULTISAMPLE_16_SAMPLES;
			break;
		default:
			break;
		}
	}

	return ret;
}

/*
 * @brief: 将vType 标识符转换成 Direct3D FVF
 ******************************************************************************************************
 * 固定管线渲染，在定义定点格式的时候要用固定的格式，定义顶点结构体是有固定的顺序的
 * 这些是固定的顺序，可以不包含某一个，但是顺序不能颠倒
 * D3DFVF_XYZRHW和D3DFVF_XYZ、D3DFVF_NORMAL不能共存
 * 顶点格式标记 在处理的时候是直接从低位开始看每一位是否有值的
 *******************************************************************************************************

 * D3DFVF_XYZ - 坐标，默认的用户区中心坐标是（0，0）。默认的为非光照的, 没有经过转化的顶点，需要经过变换，再定义了之后要设置渲染状态才能看到
 * D3DFVF_XYZRHW - 坐标+位置元素(RHW或w元素 1.0f), 经过3D坐标变换，可以直接光栅化，[告知Direct3D使用的顶点已经在屏幕坐标系中]
 *		任何用SetTransform进行的转换都对其无效 
 *		原点就在客户区的左上角了，其中x向右为正，y向下为正，而z的意义已经变为z-buffer的象素深度。
 *		x、y必须在屏幕坐标系中，z必须是z-buffer中的象素深度，取值范围：0.0－1.0，离观察者最近的地方为0.0，
 *		观察范围内最远可见的地方为1.0。
 *		会被设置默认高洛德光照
 * D3DFVF_XYZW - 包含经过转换和裁剪的顶点坐标
 * D3DFVF_XYZB1………D3DFVF_XYZB5 - 包含用于骨骼动画的顶点和顶点对骨骼的权重信息
 * D3DFVF_NORMAL - 表示顶点数据中包含法线向量，这个法向量用来计算光照（照射角度不同渲染效果不同），所以定义了这个格式之后一定要加光照才可以显示
 * D3DFVF_PSIZE - 顶点信息指明绘制点的大小
 * D3DFVF_DIFFUSE - 颜色（包含漫反射的信息）
 * D3DFVF_SPECULAR - 包含镜面反射的信息
 * D3DFVF_TEX0………D3DFVF_TEX8 - 包含0-8个纹理坐标信息
 */
static unsigned long CreateD3DFVF(int flags)
{
	unsigned long fvf = 0;

	if (flags == GUI_FVF)
		fvf = D3DFVF_GUI;
	else if (flags == MV_FVF)
		fvf = D3DFVF_MV;

	return fvf;
}

// 将浮点值转换成无符号长整型
static unsigned long FtoW(float v)
{
	return *((unsigned long*)&v);
}

//////////////////////////////////////////////////////////////////////////

CD3DRenderer::CD3DRenderer() :
	m_Direct3D(NULL),
	m_Device(NULL),
	m_renderingScene(false),
	m_numStaticBuffers(0),
	m_activeStaticBuffer(STRANDED_INVALID),
	m_staticBufferList(nullptr),
	m_numTextures(0),
	m_textureList(nullptr),
	m_fonts(nullptr),
	m_xModels(nullptr)
{
	
}

CD3DRenderer::~CD3DRenderer()
{
	Shutdown();
}

/*
* 窗口宽度、高度、窗口句柄、是否全屏标识、
* 要使用的多采样类型(如果不想使用多采样，设为STRANDED_MS_NONE，不支持的参数采样类型会回到默认无多采样)
*/
bool CD3DRenderer::Initialize(int w, int h, WinHWND mainWin, bool fullscreen, STRANDED_MS_TYPE ms)
{
	Shutdown();

	m_mainHandle = mainWin;
	if(!m_mainHandle)
		return false;

	m_fullscreen = fullscreen;

	D3DDISPLAYMODE mode;
	D3DCAPS9 caps;
	// 定义Direct3D窗口的显示信息
	D3DPRESENT_PARAMETERS d3dpp;

	::ZeroMemory(&d3dpp, sizeof(d3dpp));

	// 创建D3D对象
	m_Direct3D = Direct3DCreate9(D3D_SDK_VERSION);
	if(!m_Direct3D)
		return false;

	// Get the desktop display mode.
	/*
	 * @param
	 * 1、正在查询的适配器 D3DADAPTER_DEFAULT-主显卡
	 * 2、保存信息的显示模式对象
	 */
	if(FAILED(m_Direct3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &mode)))
		return false;

	if(FAILED(m_Direct3D->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &caps)))
		return false;

	DWORD processing = 0;
	if(caps.VertexProcessingCaps != 0)
		processing = D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE;
	else
		processing = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	if(m_fullscreen)
	{
		// 存储显示器刷新率，窗口模式该值必须为0，全屏模式取决于显示器
		d3dpp.FullScreen_RefreshRateInHz = mode.RefreshRate;
		// 处理可以显示的交换链(桌面同时显示多个窗口)的后台缓存最大次数
		d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
	}
	else
		d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
	d3dpp.Windowed = !m_fullscreen;
	// 窗口宽、高
	d3dpp.BackBufferWidth = w;
	d3dpp.BackBufferHeight = h;
	d3dpp.hDeviceWindow = m_mainHandle;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	// 渲染后台缓存格式
	d3dpp.BackBufferFormat = mode.Format;
	// 渲染后台缓存总数
	d3dpp.BackBufferCount = 1;
	d3dpp.EnableAutoDepthStencil = TRUE;	// 是否开启深度缓存和模板缓存
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16;	// 深度值用16位二进制表示, 位数越多场景绘制的越精确，内存消耗更多
	/*
	 * D3DPRESENT_PARAMETERS：与多采样相关的变量 D3DMULTISAMPLE_TYPE MultiSampleType; DWORD MultiSampleQuality;
	 * MultiSampleType可设置 D3DMULTISAMPLE_2_SAMPLES、D3DMULTISAMPLE_4_SAMPLES等, 如果设置D3DMULTISAMPLE_NONMASKABLE，
	 * 必须设置 MultiSampleQuality
	 */
	d3dpp.MultiSampleType = GetD3DMultiSampleType(m_Direct3D, ms, D3DDEVTYPE_HAL, mode.Format, m_fullscreen);

	m_screenWidth = w;
	m_screenHeight = h;

	// Create the D3DDevice
	/*
	 * @param
	 * 1、指定正在使用的显卡
	 * 2、指定Direct3D中的渲染方式 D3DDVTYPE_HALL-采用硬件渲染, D3DEVTYPE_REF-采用软件渲染, D3DDETYPE_REF-不需要软硬件支持,
	 * D3DDEVTYPE_SW-采用要进行渲染工作的可插拔软件
	 * 3、窗口句柄
	 * 4、指定设备的运行方式的标识符组合
	 * 5、指向创建的D3DPRESENT_PARAMETERS结构的指针
	 * 6、指向新创建的Direct3D设备对象的指针
	 */
	if(FAILED(m_Direct3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, m_mainHandle, processing, &d3dpp, &m_Device)))
		return false;
	if(m_Device == NULL)
		return false;

	OneTimeInit();
	
	return true;
}

// 一次性初始化
void CD3DRenderer::OneTimeInit()
{
	if(!m_Device)
		return;
	
	m_Device->SetRenderState(D3DRS_LIGHTING, FALSE);
	m_Device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);	// 背面剔除

	// 开启深度缓存
	//m_Device->SetRenderState(D3DRS_ZENABLE, TRUE);

	// 设置默认投影矩阵
	CalculateProjMatrix(D3DX_PI/4, 0.1f, 1000);
}

void CD3DRenderer::Shutdown()
{
	ReleaseAllStaticBuffers();
	ReleaseAllXModels();

	for(int s = 0; s < m_numTextures; s++)
	{
		// 删除文件名
		if(m_textureList[s].fileName)
		{
			delete[] m_textureList[s].fileName;
			m_textureList[s].fileName = NULL;
		}

		// 释放LPDIRECT3DTEXTURE9对象
		if(m_textureList[s].image)
		{
			m_textureList[s].image->Release();
			m_textureList[s].image = NULL;
		}
	}

	// 清除纹理列表
	m_numTextures = 0;
	if(m_textureList)
	{
		delete[] m_textureList;
		m_textureList = NULL;
	}

	for (int i = 0; i < m_totalFonts; ++i)
	{
		if (m_fonts[i])
		{
			m_fonts[i]->Release();
			m_fonts[i] = nullptr;
		}
	}

	m_totalFonts = 0;
	if (m_fonts)
	{
		delete[] m_fonts;
		m_fonts = nullptr;
	}

	ReleaseAllGUIs();

	if(m_Device)
	{
		m_Device->Release();
		m_Device = NULL;
	}
		
	if(m_Direct3D)
	{
		m_Direct3D->Release();
		m_Direct3D = NULL;
	}	
}

// 清屏的红绿蓝三色的浮点值
void CD3DRenderer::SetClearCol(float r, float g, float b)
{
	m_Color = D3DCOLOR_COLORVALUE(r, g, b, 1.0f);
}

// 系统是否将清除颜色、深度和 stencil缓存(对渲染无用)
void CD3DRenderer::StartRender(bool bColor, bool bDepth, bool bStencil)
{
	if(!m_Device)
		return;

	unsigned int buffers = 0;
	if(bColor)
		buffers |= D3DCLEAR_TARGET;
	if(bDepth)
		buffers |= D3DCLEAR_ZBUFFER;	// 清除深度缓存
	if(bStencil)
		buffers |= D3DCLEAR_STENCIL;	// 清除模板缓存

	// Clear the backbuffer.
	/*
	 * @param
	 * 1、要清除的矩形数目(0代表清除整个屏幕)
	 * 2、定义想要清除的屏幕区域矩形链表(NULL代表整个屏幕)
	 * 3、标识要清除内容的标识符
	 * 4、清除后的颜色
	 * 5、要设定的深度值
	 * 6、设定模板缓存值
	 */
	if(FAILED(m_Device->Clear(0, NULL, buffers, m_Color, 1, 0)))
		return;

	// 启动一个新场景
	if(FAILED(m_Device->BeginScene()))
		return;

	m_renderingScene = true;
}

// 用于清理已经启动的场景
void CD3DRenderer::ClearBuffers(bool bColor, bool bDepth, bool bStencil)
{
	if(!m_Device)
		return;

	unsigned int buffers = 0;
	if(bColor)
		buffers |= D3DCLEAR_TARGET;
	if(bDepth)
		buffers |= D3DCLEAR_ZBUFFER;
	if(bStencil)
		buffers |= D3DCLEAR_STENCIL;

	if(m_renderingScene)
		m_Device->EndScene();
	if(FAILED(m_Device->Clear(0, NULL, buffers, m_Color, 1, 0)))
		return;

	if(m_renderingScene)
		if(FAILED(m_Device->BeginScene()))
			return;
}

// 停止场景渲染
void CD3DRenderer::EndRendering()
{
	if(!m_Device)
		return;

	// 结束渲染
	m_Device->EndScene();
	/*
	 * @param
	 * 1、正在显示的原始矩形，如果不使用交换链，该值必须为NULL
	 * 2、指向要渲染的最终矩形的指针
	 * 3、正在显示的窗口的窗口句柄(在Direct3D初始化时为D3DPSENT_PARAMETERS对象设置的窗口句柄)，如果不使用交换链，该值为NULL
	 * 4、需要更新的最小缓存区域
	 */
	m_Device->Present(NULL, NULL, NULL, NULL);

	m_renderingScene = false;
}

void CD3DRenderer::SetMaterial(stMaterial *mat)
{
	if(!mat || !m_Device)
		return;

	// 使用材质结构信息创建Direct3D材质对象
	D3DMATERIAL9 m =
	{
		mat->diffuseR, mat->diffuseG, mat->diffuseB, mat->diffuseA,			// r=0.6f, g=0.6f, b=0.7f
		mat->ambientR, mat->ambientG, mat->ambientB, mat->ambientA,			// r=0.6f, g=0.6f, b=0.7f
		mat->specularR, mat->specularG, mat->specularB, mat->specularA,		// r=0.4f, g=0.4f, b=0.4f
		mat->emissiveR, mat->emissiveG, mat->emissiveB, mat->emissiveA,
		mat->power															// 8.0f
	};

	// 启用材质，将该对象直接发送给Direct3D(渲染对象前调用，将材质应用到对象上)
	m_Device->SetMaterial(&m);
}

void CD3DRenderer::SetLight(stLight *light, int index)
{
	if(!light || !m_Device || index < 0)
		return;

	D3DLIGHT9 l;

	l.Ambient.a = light->ambientA;
	l.Ambient.r = light->ambientR;
	l.Ambient.g = light->ambientG;
	l.Ambient.b = light->ambientB;

	l.Attenuation0 = light->attenuation0;
	l.Attenuation1 = light->attenuation1;
	l.Attenuation2 = light->attenuation2;

	// 1.0f
	l.Diffuse.a = light->diffuseA;
	l.Diffuse.r = light->diffuseR;
	l.Diffuse.g = light->diffuseG;
	l.Diffuse.b = light->diffuseB;

	// D3DVECTOR3(0.0f, 0.0f, 1.0f)
	l.Direction.x = light->dirX;
	l.Direction.y = light->dirY;
	l.Direction.z = light->dirZ;

	l.Falloff = light->falloff;
	l.Phi = light->phi;

	l.Position.x = light->posX;
	l.Position.y = light->posY;
	l.Position.z = light->posZ;
	
	l.Range = light->range;

	// 1.0f
	l.Specular.a = light->specularA;
	l.Specular.r = light->specularR;
	l.Specular.g = light->specularG;
	l.Specular.b = light->specularB;

	l.Theta = light->theta;

	if(light->type == LIGHT_POINT)
		l.Type = D3DLIGHT_POINT;
	else if(light->type == LIGHT_SPOT)
		l.Type = D3DLIGHT_SPOT;
	else
		l.Type = D3DLIGHT_DIRECTIONAL;
	// 给Direct3D发送光源对象(基0的光源索引, 正在使用的第一盏灯, 一次可以使用最多8个光源[0~7])
	m_Device->SetLight(index, &l);
	// 开启光源
	m_Device->LightEnable(index, TRUE);
}

/*
 * @param: 想要关闭灯源索引
 */
void CD3DRenderer::DisableLight(int index)
{
	if(!m_Device)
		return;

	m_Device->LightEnable(index, FALSE);
}

/*
 * 创建平行投影矩阵
 *
 * {-0.3f, -0.3f, 1.0f, D3DCOLOR_XRGB(255,255,0)}
 * [单位，非像素坐标]
 * 添加
 * SetRenderState(D3DRS_LIGHTING, FALSE);
 * SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
 *
 * 视场(fv)
 * 近距离值(n)
 * 远距离值(f)
*/
void CD3DRenderer::CalculateProjMatrix(float fov, float n, float f)
{
	if(!m_Device)
		return;

	D3DXMATRIX projection;

	/*
	 * 设置左手立体投影
	 * @param: 将要创建的D3DXMATRIX对象
	 * @param: Y方向(以弧度指定)的可见区
	 * @param: 视角幅度比(分辨率宽度和高度之比)
	 * @param: 远近剪切平面值
	 */
	D3DXMatrixPerspectiveFovLH(&projection, fov, (float)m_screenWidth/(float)m_screenHeight, n, f);
	// Apply the view (camera).
	m_Device->SetTransform(D3DTS_PROJECTION, &projection);
}

/*
 * 创建正交投影矩阵
 *
 * 添加
 * SetRenderState(D3DRS_LIGHTING, FALSE);
 * SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
 * {-0.0ff, 150.0f, 0.1ff, D3DCOLOR_XRGB(255,255,0)}
 * 将(0,0)设在屏幕正中间
 * 禁用正交投影时，使用像素坐标
 */
void CD3DRenderer::CalculateOrthoMatrix(float n, float f)
{
	if(!m_Device)
		return;

	D3DXMATRIX ortho;

	/*
	 * 左手正交投影
	 * @param: 保存正交矩阵的D3DXMATRIX对象
	 * @param: 窗口分辨率的宽度、高度
	 * @param: 远近剪切平面值(0.1f、1000.0f)
	 */
	D3DXMatrixOrthoLH(&ortho, (float)m_screenWidth, (float)m_screenHeight, n, f);
	/*
	 * 在Direct3D设备中使用正交矩阵
	 * @param: 正在使用的矩阵类型、矩阵
	 */
	m_Device->SetTransform(D3DTS_PROJECTION, &ortho);
}

/*
 * 设置世界矩阵/模型矩阵
 * @brief: 世界矩阵控制绘制物体位置的参考点,
 *		开始起始点位于(0,0)
 * @sample: (0,0,0)->(0,0,-5)
 */
void CD3DRenderer::CalculateWorldMatrix(float x, float y, float z)
{
	if(!m_Device)
		return;

	D3DXMATRIX world;
	
	/*
	 * 改变世界矩阵
	 * @brief: 绘制对象，先创建矩阵，存储对象位置
	 * @param: 创建不同于D3DXMATRIX 类型的世界矩阵的矩阵(矩阵对象)
	 * @param: 要移动世界矩阵的X、Y和Z值(标识对象位置x,y,z)
	 */
	D3DXMatrixTranslation(&world, x, y, z);
	// 将矩阵应用到Direct3D中
	m_Device->SetTransform(D3DTS_WORLD, &world);
}

void CD3DRenderer::RotateWorldMatrix(float radian, E_ROTATION_DIR eRotation)
{
	if(!m_Device)
		return;

	D3DXMATRIX world;

	/*
	 * 围绕任意轴旋转
	 * D3DXMATRIX* WINAPI D3DXMatrixRotationAxis(D3DXMATRIX *pOut, CONST D3DXCECTOR3 *pV, FLOAT Angle);
	 * 按照横倾、纵倾、横摆值旋转
	 * D3DXMATRIX* __stdcall D3DXMatrixRotationYawPitchRoll(D3DXMATRIX *pOut, FLOAT Yaw, FLOAT Pitch, FLOAT Roll);
	 */
	switch(eRotation)
	{
	case ROTATION_X:
		D3DXMatrixRotationX(&world, radian);
		break;
	case ROTATION_Y:
		D3DXMatrixRotationY(&world, radian);
		break;
	case ROTATION_Z:
	default:
		D3DXMatrixRotationZ(&world, radian);
		break;
	}

	m_Device->SetTransform(D3DTS_WORLD, &world);
}

/*
 * 设置视图矩阵(摄像机)
 * D3D 左手坐标系 Y上X右Z里
 * @fun: 创建一个矩阵并将其应用到当前视图矩阵
 * @sample:
 *		D3DXVECTOR3 cameraPos(0.0f, 0.0f, -8.0f);
 *		D3DXVECTOR3 lookAtPos(0.0f, 0.0f, 0.0f);
 *		D3DXVECTOR3 upDir(0.0f, 1.0f, 0.0f);
 */
void CD3DRenderer::CalculateViewMatrix(const CVector3& pos, const CVector3& view, const CVector3& up)
{
	if(!m_Device) return;

	// Define camera information.
	D3DXVECTOR3 camePos(pos.x, pos.y, pos.z);
	D3DXVECTOR3 lookAtPos(view.x, view.y, view.z);
	D3DXVECTOR3 upDir(up.x, up.y, up.z);

	// Build view matrix.
	D3DXMATRIX mat;

	/*
	 * build view matrix.
	 * @param: 将要转变为视图矩阵的矩阵
	 * @param: 摄像机的位置
	 * @param: 观察的位置
	 * @param: 向上的方向
	 */
	D3DXMatrixLookAtLH(&mat, &camePos, &lookAtPos, &upDir);
	// Apply the view (camera).
	m_Device->SetTransform(D3DTS_VIEW, &mat);
}


/*
 * @define: 设置静态缓存对象中的一个对象内容，并将其添加到静态缓存列表
 * @param: 正在使用的顶点类型
 * @param: 渲染该几何图形的指令
 * @param: 几何图形中的顶点总数
 * @param: 一个对象的索引顶点总数
 * @param: 顶点尺寸
 * @param: 顶点数据
 * @param: 索引数据
 * @param: staticId 指向新创建的将要保存的静态缓存ID的变量指针
 */
int CD3DRenderer::CreateStaticBuffer(VertexType vType, PrimType primType, unsigned int totalVerts, unsigned int totalIndices, int stride, void **data, unsigned int *indices, int *staticId)
{
	void* ptr;
	int index = m_numStaticBuffers;

	// 设置一个静态缓存对象，并将其添加到静态缓存列表中
	if(!m_staticBufferList)
	{
		m_staticBufferList = new stD3DStaticBuffer[1];
		if(!m_staticBufferList)
			return STRANDED_FAIL;
	}
	else
	{
		stD3DStaticBuffer *temp;
		temp = new stD3DStaticBuffer[m_numStaticBuffers + 1];

		memcpy(temp, m_staticBufferList, sizeof(stD3DStaticBuffer) * m_numStaticBuffers);

		delete[] m_staticBufferList;
		m_staticBufferList = temp;
	}

	m_staticBufferList[index].numVerts = totalVerts;
	m_staticBufferList[index].numIndices = totalIndices;
	m_staticBufferList[index].primType = primType;
	m_staticBufferList[index].stride = stride;
	m_staticBufferList[index].fvf = CreateD3DFVF(vType);

	if(totalIndices > 0)
	{
		// 创建索引缓存
		if(FAILED(m_Device->CreateIndexBuffer(sizeof(unsigned int) * totalIndices, D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &m_staticBufferList[index].ibPtr, NULL)))
			return STRANDED_FAIL;

		if(FAILED(m_staticBufferList[index].ibPtr->Lock(0, 0, (void**)&ptr, 0)))
			return STRANDED_FAIL;

		memcpy(ptr, indices, sizeof(unsigned int) * totalIndices);

		m_staticBufferList[index].ibPtr->Unlock();
	}
	else
	{
		m_staticBufferList[index].ibPtr = NULL;
	}

	/*
	 * 绘制图元方式
	 * 1、使用指针将几何图形数据数组发送给某个Direct3D函数
	 * - (低效)需要创建临时顶点缓存 DrawPrimitive();
	 * - HRESULT DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride)
	 * - @param: 图元类型[D3DPT_POINTLIST, D3DPT_LINELIST, D3DPT_LINESTRIP, D3DPT_TRIANGLELIST, D3DPT_TRIANGLESTRIP, D3DPT_TRIANGLEFAN]
	 * - @param: 发送给函数的图元总数
	 * - @param: 发送给该函数的图元数据数组
	 * - @param: 每个顶点的大小(字节)
	 * 2、创建顶点缓存
	 * - HRESULT CreateVertexBuffer(UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool, IDirect3DVertexBuffer9** ppVertexBuffer, HANDLE* pSharedHandle);
	 * - @param: 顶点缓存所需的字节数
	 * - @param: 一个描述顶点缓存使用方法的标识符
	 * - @param: 一个描述使用顶点缓存的顶点结构的标识符或标识符集[顶点位置->顶点颜色等]
	 * - @param: 放置资源的正确内存类
	 * - @param: 创建的顶点缓存
	 * - @param: 保留参数 0
	 */
	if(FAILED(m_Device->CreateVertexBuffer(totalVerts * stride, D3DUSAGE_WRITEONLY, m_staticBufferList[index].fvf, D3DPOOL_DEFAULT, &m_staticBufferList[index].vbPtr, NULL)))
		return STRANDED_FAIL;

	/*
	 * 锁定顶点缓存[锁定一列数据，获取顶点缓存指针, 将数据复制给该顶点缓存]
	 * - HRESULT Lock(UINT OffsetToLock, UINT SizeToLock, void** ppbData, DWORD Flags);
	 * - @param: 为上锁的顶点缓存增加的偏移量(0 代表锁定全部缓存)
	 * - @param: 要锁定的数据量
	 * - @param: 指向锁定内存区的指针
	 * - @param: 一个指定所存数据方法的标识符
	 */
	if(FAILED(m_staticBufferList[index].vbPtr->Lock(0, 0, (void**)&ptr, 0)))
		return STRANDED_FAIL;

	memcpy(ptr, data, totalVerts * stride);
	m_staticBufferList[index].vbPtr->Unlock();

	// 保存静态缓存ID
	*staticId = m_numStaticBuffers;
	// 增加静态缓存总数
	m_numStaticBuffers++;

	return STRANDED_OK;
}

/*
 * @define: 将与静态缓存ID相关的几何图形渲染到屏幕上
 * @param: 静态缓存ID
 */
int CD3DRenderer::RenderStaticBuffer(unsigned int staticId)
{
	// 快速检查静态缓存ID，有效
	if(staticId >= m_numStaticBuffers)
		return STRANDED_FAIL;

	// 检查静态缓存ID是否可用
	if(m_activeStaticBuffer != staticId)	// 不可用
	{
		if(m_staticBufferList[staticId].ibPtr != NULL)
			m_Device->SetIndices(m_staticBufferList[staticId].ibPtr);

		/*
		 * @define: 将顶点缓存设为渲染流
		 * @param: 要设置的流索引(从0到最大流数-1)
		 * @param: 顶点缓存
		 * @param: 开始渲染缓存的偏移字节量
		 * @param: 顶点跨度(正在渲染的缓存中，在几何图形定义单个点时用到的顶点结构大小)
		 */
		m_Device->SetStreamSource(0, m_staticBufferList[staticId].vbPtr, 0, m_staticBufferList[staticId].stride);

		// 设置顶点格式 FVF
		m_Device->SetFVF(m_staticBufferList[staticId].fvf);

		m_activeStaticBuffer = staticId;
	}

	// 可用[渲染图形]
	if(m_staticBufferList[staticId].ibPtr != NULL)
	{
		// [绘制索引几何图形]
		switch(m_staticBufferList[staticId].primType)
		{
		case POINT_LIST:
			/*
			 * 绘制当前设置为流的顶点缓存内容
			 * @param: 要渲染的图元类型
			 * @param: 开始渲染的起始顶点索引(开始为0)
			 * @param: 渲染的图元数目
			 */
			if(FAILED(m_Device->DrawPrimitive(D3DPT_POINTLIST, 0, m_staticBufferList[staticId].numVerts)))
				return STRANDED_FAIL;
			break;
		case TRIANGLE_LIST:
			if(FAILED(m_Device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, m_staticBufferList[staticId].numVerts/3, 0, m_staticBufferList[staticId].numIndices)))
				return STRANDED_FAIL;
			break;
		case TRIANGLE_STRIP:
			if(FAILED(m_Device->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, 0, 0, m_staticBufferList[staticId].numVerts/2, 0, m_staticBufferList[staticId].numIndices)))
				return STRANDED_FAIL;
			break;
		case TRIANGLE_FAN:
			if(FAILED(m_Device->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, 0, 0, m_staticBufferList[staticId].numVerts/2, 0, m_staticBufferList[staticId].numIndices)))
				return STRANDED_FAIL;
			break;
		case LINE_LIST:
			if(FAILED(m_Device->DrawIndexedPrimitive(D3DPT_LINELIST, 0, 0, m_staticBufferList[staticId].numVerts/2, 0, m_staticBufferList[staticId].numIndices)))
				return STRANDED_FAIL;
			break;
		case LINE_STRIP:
			if(FAILED(m_Device->DrawIndexedPrimitive(D3DPT_LINESTRIP, 0, 0, m_staticBufferList[staticId].numVerts, 0, m_staticBufferList[staticId].numIndices)))
				return STRANDED_FAIL;
			break;
		default:
			return STRANDED_FAIL;
		}
	}
	else
	{
		// [绘制无索引几何图形]
		switch(m_staticBufferList[staticId].primType)
		{
		case POINT_LIST:
			if(FAILED(m_Device->DrawPrimitive(D3DPT_POINTLIST, 0, m_staticBufferList[staticId].numVerts)))
				return STRANDED_FAIL;
			break;
		case TRIANGLE_LIST:
			if(FAILED(m_Device->DrawPrimitive(D3DPT_TRIANGLELIST, 0, (int)(m_staticBufferList[staticId].numVerts/3))))
				return STRANDED_FAIL;
			break;
		case TRIANGLE_STRIP:
			if(FAILED(m_Device->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, (int)(m_staticBufferList[staticId].numVerts/2))))
				return STRANDED_FAIL;
			break;
		case TRIANGLE_FAN:
			if(FAILED(m_Device->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, (int)(m_staticBufferList[staticId].numVerts/2))))
				return STRANDED_FAIL;
			break;
		case LINE_LIST:
			if(FAILED(m_Device->DrawPrimitive(D3DPT_LINELIST, 0, m_staticBufferList[staticId].numVerts/2)))
				return STRANDED_FAIL;
			break;
		case LINE_STRIP:
			if(FAILED(m_Device->DrawPrimitive(D3DPT_LINESTRIP, 0, m_staticBufferList[staticId].numVerts)))
				return STRANDED_FAIL;
			break;
		default:
			return STRANDED_FAIL;
		}
	}

	return STRANDED_OK;
}

/*
 * @param: 渲染在屏幕上的 X 模型的 ID.
 */
int CD3DRenderer::RenderXModel(int xModelId)
{
	if (!m_xModels || xModelId >= m_numXModels || xModelId < 0)
		return STRANDED_FAIL;

	m_Device->SetIndices(NULL);
	m_Device->SetStreamSource(0, NULL, 0, 0);
	m_Device->SetFVF(0);

	//////////////////////////////////////////////////////////////////////////
	/*
	// 循环 X 模型的材质
	for (unsigned long i = 0; i < m_xModels[xModelId].numMaterials; i++)
	{
		// 设置模型材质
		m_Device->SetMaterial(&m_xModels[xModelId].matList[i]);
		// 纹理图像
		m_Device->SetTexture(0, m_xModels[xModelId].textureList[i]);
		//HRESULT DrawSubset(DWORD AttribId);
		m_xModels[xModelId].model->DrawSubset(i);
	}
	*/
	//////////////////////////////////////////////////////////////////////////
	// XMesh.h 中的模型结构代替 [上面是静态网格]
	m_xModels[xModelId].Render();

	return STRANDED_OK;
}

/*
 * @brief: 将 UMF 模型加载到静态缓存
 * @param: 模型文件名、网格的索引(由于 UMF 可以由多个模型构成，所以指明是从那个模型加载网格，即这个网格的索引)、保存静态缓存 ID 的位置地址
 */
int CD3DRenderer::LoadUMFAsStaticBuffer(const char* file, int meshIndex, int *staticID)
{
	void *ptr;
	int index = m_numStaticBuffers;

	stUMFModel *model;
	int numModels = 0;
	int stride = sizeof(stModelVertex);

	// 加载 UMF 模型
	if (!LoadUMF(file, &model, &numModels))
		return STRANDED_FAIL;

	if (!numModels || !model)
		return STRANDED_FAIL;

	if (meshIndex < 0 || meshIndex >= numModels)
		return STRANDED_FAIL;

	// 增加静态缓存链表，并未新增加的内容腾出空间
	if (!m_staticBufferList)
	{
		m_staticBufferList = new stD3DStaticBuffer[1];
		if (!m_staticBufferList) return STRANDED_FAIL;
	}
	else
	{
		stD3DStaticBuffer *temp;
		temp = new stD3DStaticBuffer[m_numStaticBuffers + 1];

		memcpy(temp, m_staticBufferList, sizeof(stD3DStaticBuffer)*m_numStaticBuffers);

		delete[] m_staticBufferList;
		m_staticBufferList = temp;
	}

	// 根据加载的网格信息设置缓存的成员变量 [顶点数目、图元类型]
	m_staticBufferList[index].numVerts = model[meshIndex].totalVertices;
	m_staticBufferList[index].numIndices = model[meshIndex].totalFaces;
	m_staticBufferList[index].primType = TRIANGLE_LIST;
	m_staticBufferList[index].stride = stride;
	m_staticBufferList[index].fvf = CreateD3DFVF(MV_FVF);

	//////////////////////////////////////////////////////////////////////////
	// 获取网格信息并将其复制给 Direct3D 索引和顶点缓存

	// 是否有外观存在
	if (model[meshIndex].totalFaces > 0)
	{
		// 复制给临时链表，在创建索引缓存时将用到该链表
		int size = model[meshIndex].totalFaces * 3;
		unsigned int *idx = new unsigned int[size];

		// 在 UMF 模型中，为每个外观保存一个索引。正在创建的索引缓存只能是无符号整形数据
		for (int i = 0, f = 0; i < size; i += 3)
		{
			idx[i + 0] = model[meshIndex].faces[f].indices[0];
			idx[i + 1] = model[meshIndex].faces[f].indices[1];
			idx[i + 2] = model[meshIndex].faces[f].indices[2];
			f++;
		}

		if (FAILED(m_Device->CreateIndexBuffer(sizeof(unsigned int)*size, D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &m_staticBufferList[index].ibPtr, NULL)))
			return STRANDED_FAIL;

		if (FAILED(m_staticBufferList[index].ibPtr->Lock(0, 0, (void**)&ptr, 0)))
			return STRANDED_FAIL;

		memcpy(ptr, idx, sizeof(unsigned int)*size);
		m_staticBufferList[index].ibPtr->Unlock();

		delete[] idx;
	}
	else
	{
		// 没有外观，将索引缓存设置为 NULL
		m_staticBufferList[index].ibPtr = NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	
	// 创建 stModelVertex 临时模型数据链表 [基于 GUI 顶点结构]
	int totalVerts = model[meshIndex].totalVertices;
	stModelVertex *data = new stModelVertex[totalVerts];

	// 循环每个模型顶点，并复制顶点位置、顶点法线(如果需要的话)、顶点颜色以及顶点纹理坐标
	for (int i = 0; i < totalVerts; i++)
	{
		data[i].x = model[meshIndex].vertices[i].x;
		data[i].y = model[meshIndex].vertices[i].y;
		data[i].z = model[meshIndex].vertices[i].z;

		if (model[meshIndex].normals)
		{
			data[i].nx = model[meshIndex].normals[i].x;
			data[i].ny = model[meshIndex].normals[i].y;
			data[i].nz = model[meshIndex].normals[i].z;
		}
		else
		{
			data[i].nx = data[i].ny = data[i].nz = 0.0f;
		}

		if (model[meshIndex].colors)
		{
			data[i].color = STRANDED_COLOR_ARGB(255,
				(int)model[meshIndex].colors[i].x,
				(int)model[meshIndex].colors[i].y,
				(int)model[meshIndex].colors[i].z);
		}
		else
			data[i].color = STRANDED_COLOR_ARGB(255, 255, 255, 255);

		if (model[meshIndex].texCoords)
		{
			data[i].tu = model[meshIndex].texCoords[i].tu;
			data[i].tv = model[meshIndex].texCoords[i].tv;
		}
		else
		{
			data[i].tu = data[i].tv = 0.0f;
		}
	}

	// 创建 Direct3D 顶点缓存
	if (FAILED(m_Device->CreateVertexBuffer(totalVerts*stride, D3DUSAGE_WRITEONLY, m_staticBufferList[index].fvf, D3DPOOL_DEFAULT, &m_staticBufferList[index].vbPtr, NULL)))
		return STRANDED_FAIL;

	// 复制临时链表的内容
	if (FAILED(m_staticBufferList[index].vbPtr->Lock(0, 0, (void**)&ptr, 0)))
		return STRANDED_FAIL;
	memcpy(ptr, data, totalVerts*stride);
	m_staticBufferList[index].vbPtr->Unlock();

	// 删除所有的临时数据
	if (data)
	{
		delete[] data;
		data = nullptr;
	}

	FreeUMFModel(model, numModels);

	if (model)
	{
		delete[] model;
		model = nullptr;
	}

	// 保存静态 ID
	*staticID = m_numStaticBuffers;
	m_numStaticBuffers++;

	return STRANDED_OK;
}

/*
 * @brief: 重载LoadUMFAsStaticBuffer，不增加静态缓存或静态缓存列表总数, 取消现有的静态缓存模型，用新数据代替.
 *			替换数据前：调用 ReleaseStaticBuffer() 函数清楚已经存在的内容.
 *			ReleaseStaticBuffer() 函数或ReleaseXModel() 函数在于释放单个对象的内容，并没有将对象从链表中删除，
 *			避免内容泄漏，不能在没有释放就数据前，使用新的动态内存替换数据.
 * @param: staticID - 指明由那个静态缓存重载新数据
 */
int CD3DRenderer::LoadUMFAsStaticBuffer(const char* file, int meshIndex, int staticID)
{
	void *ptr;
	
	if (staticID < 0 || staticID >= m_numStaticBuffers)
		return STRANDED_FAIL;

	stUMFModel *model;
	int numModels = 0;
	int stride = sizeof(stModelVertex);

	if (!LoadUMF(file, &model, &numModels))
		return STRANDED_FAIL;

	if (!numModels || !model)
		return STRANDED_FAIL;

	if (meshIndex < 0 || meshIndex >= numModels)
		return STRANDED_FAIL;

	ReleaseStaticBuffer(staticID);

	m_staticBufferList[staticID].numVerts = model[meshIndex].totalVertices;
	m_staticBufferList[staticID].numIndices = model[meshIndex].totalFaces;
	m_staticBufferList[staticID].primType = TRIANGLE_LIST;
	m_staticBufferList[staticID].stride = stride;
	m_staticBufferList[staticID].fvf = CreateD3DFVF(MV_FVF);

	if (model[meshIndex].totalFaces > 0)
	{
		int size = model[meshIndex].totalFaces * 3;
		unsigned int *idx = new unsigned int[size];

		for (int i = 0, f = 0; i < size; i += 3)
		{
			idx[i + 0] = model[meshIndex].faces[f].indices[0];
			idx[i + 1] = model[meshIndex].faces[f].indices[1];
			idx[i + 2] = model[meshIndex].faces[f].indices[2];
			f++;
		}

		if (FAILED(m_Device->CreateIndexBuffer(sizeof(unsigned int)*size, D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &m_staticBufferList[staticID].ibPtr, NULL)))
			return STRANDED_FAIL;

		if (FAILED(m_staticBufferList[staticID].ibPtr->Lock(0, 0, (void**)&ptr, 0)))
			return STRANDED_FAIL;

		memcpy(ptr, idx, sizeof(unsigned int)*size);
		m_staticBufferList[staticID].ibPtr->Unlock();

		delete[] idx;
	}
	else
	{
		m_staticBufferList[staticID].ibPtr = NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	// 记载 staticID 指定的数据，而不是原 LoadUMFAsStaticBuffer 中在链表末端加载数据

	int totalVerts = model[meshIndex].totalVertices;
	stModelVertex *data = new stModelVertex[totalVerts];

	for (int i = 0; i < totalVerts; i++)
	{
		data[i].x = model[meshIndex].vertices[i].x;
		data[i].y = model[meshIndex].vertices[i].y;
		data[i].z = model[meshIndex].vertices[i].z;

		if (model[meshIndex].normals)
		{
			data[i].nx = model[meshIndex].normals[i].x;
			data[i].ny = model[meshIndex].normals[i].y;
			data[i].nz = model[meshIndex].normals[i].z;
		}
		else
		{
			data[i].nx = data[i].ny = data[i].nz = 0.0f;
		}

		if (model[meshIndex].colors)
		{
			data[i].color = STRANDED_COLOR_ARGB(255,
				(int)model[meshIndex].colors[i].x,
				(int)model[meshIndex].colors[i].y,
				(int)model[meshIndex].colors[i].z);
		}
		else
			data[i].color = STRANDED_COLOR_ARGB(255, 255, 255, 255);

		if (model[meshIndex].texCoords)
		{
			data[i].tu = model[meshIndex].texCoords[i].tu;
			data[i].tv = model[meshIndex].texCoords[i].tv;
		}
		else
		{
			data[i].tu = data[i].tv = 0.0f;
		}
	}

	if (FAILED(m_Device->CreateVertexBuffer(totalVerts*stride, D3DUSAGE_WRITEONLY, m_staticBufferList[staticID].fvf, D3DPOOL_DEFAULT, &m_staticBufferList[staticID].vbPtr, NULL)))
		return STRANDED_FAIL;

	if (FAILED(m_staticBufferList[staticID].vbPtr->Lock(0, 0, (void**)&ptr, 0)))
		return STRANDED_FAIL;
	memcpy(ptr, data, totalVerts*stride);
	m_staticBufferList[staticID].vbPtr->Unlock();

	if (data)
	{
		delete[] data;
		data = nullptr;
	}

	FreeUMFModel(model, numModels);

	if (model)
	{
		delete[] model;
		model = nullptr;
	}

	return STRANDED_OK;
}

int CD3DRenderer::LoadOBJAsStaticBuffer(const char* file, int *staticID)
{
	void *ptr;
	int index = m_numStaticBuffers;

	stObjModel *model;
	int stride = sizeof(stObjModel);

	model = LoadOBJModel(file);

	if (!model)
		return STRANDED_FAIL;

	// 增加静态缓存链表，并未新增加的内容腾出空间
	if (!m_staticBufferList)
	{
		m_staticBufferList = new stD3DStaticBuffer[1];
		if (!m_staticBufferList) return STRANDED_FAIL;
	}
	else
	{
		stD3DStaticBuffer *temp;
		temp = new stD3DStaticBuffer[m_numStaticBuffers + 1];

		memcpy(temp, m_staticBufferList, sizeof(stD3DStaticBuffer)*m_numStaticBuffers);

		delete[] m_staticBufferList;
		m_staticBufferList = temp;
	}

	// 根据加载的网格信息设置缓存的成员变量 [顶点数目、图元类型]
	m_staticBufferList[index].numVerts = model->m_numFaces * 3;
	m_staticBufferList[index].numIndices = 0;
	m_staticBufferList[index].primType = TRIANGLE_LIST;
	m_staticBufferList[index].stride = stride;
	m_staticBufferList[index].fvf = CreateD3DFVF(MV_FVF);

	m_staticBufferList[index].ibPtr = NULL;

	//////////////////////////////////////////////////////////////////////////
	// 获取网格信息并将其复制给 Direct3D 顶点缓存

	// 创建 stModelVertex 临时模型数据链表 [基于 GUI 顶点结构]
	int totalVerts = model->m_numFaces * 3;
	stModelVertex *data = new stModelVertex[totalVerts];

	// 循环每个模型顶点，并复制顶点位置、顶点法线(如果需要的话)、顶点颜色以及顶点纹理坐标
	for (int i = 0, v = 0, t = 0; i < totalVerts; i++, v+=3, t+=2)
	{
		data[i].x = model->m_pVertices[v + 0];
		data[i].y = model->m_pVertices[v + 1];
		data[i].z = model->m_pVertices[v + 2];

		if (model->m_pNormals)
		{
			data[i].nx = model->m_pNormals[v + 0];
			data[i].ny = model->m_pNormals[v + 1];
			data[i].nz = model->m_pNormals[v + 2];
		}
		else
		{
			data[i].nx = data[i].ny = data[i].nz = 0.0f;
		}

		data[i].color = STRANDED_COLOR_ARGB(255, 255, 255, 255);

		if (model->m_pTexCoords)
		{
			data[i].tu = model->m_pTexCoords[t + 0];
			data[i].tv = model->m_pTexCoords[t + 1];
		}
		else
		{
			data[i].tu = data[i].tv = 0.0f;
		}
	}

	// 创建 Direct3D 顶点缓存
	if (FAILED(m_Device->CreateVertexBuffer(totalVerts*stride, D3DUSAGE_WRITEONLY, m_staticBufferList[index].fvf, D3DPOOL_DEFAULT, &m_staticBufferList[index].vbPtr, NULL)))
		return STRANDED_FAIL;

	// 复制临时链表的内容
	if (FAILED(m_staticBufferList[index].vbPtr->Lock(0, 0, (void**)&ptr, 0)))
		return STRANDED_FAIL;
	memcpy(ptr, data, totalVerts*stride);
	m_staticBufferList[index].vbPtr->Unlock();

	// 删除所有的临时数据
	if (data)
	{
		delete[] data;
		data = nullptr;
	}

	FreeOBJModel(model);

	if (model)
	{
		delete[] model;
		model = nullptr;
	}

	// 保存静态 ID
	*staticID = m_numStaticBuffers;
	m_numStaticBuffers++;

	return STRANDED_OK;
}

int CD3DRenderer::LoadOBJAsStaticBuffer(const char* file, int staticID)
{
	void *ptr;

	if (staticID < 0 || staticID >= m_numStaticBuffers)
		return STRANDED_FAIL;

	stObjModel *model;
	int stride = sizeof(stObjModel);

	model = LoadOBJModel(file);

	if (!model)
		return STRANDED_FAIL;

	ReleaseStaticBuffer(staticID);

	m_staticBufferList[staticID].numVerts = model->m_numFaces * 3;
	m_staticBufferList[staticID].numIndices = 0;
	m_staticBufferList[staticID].primType = TRIANGLE_LIST;
	m_staticBufferList[staticID].stride = stride;
	m_staticBufferList[staticID].fvf = CreateD3DFVF(MV_FVF);

	m_staticBufferList[staticID].ibPtr = NULL;

	//////////////////////////////////////////////////////////////////////////
	// 记载 staticID 指定的数据，而不是原 LoadOBJAsStaticBuffer 中在链表末端加载数据

	int totalVerts = model->m_numFaces * 3;
	stModelVertex *data = new stModelVertex[totalVerts];

	for (int i = 0, v = 0, t = 0; i < totalVerts; i++, v+=3, t+=2)
	{
		data[i].x = model->m_pVertices[v + 0];
		data[i].y = model->m_pVertices[v + 1];
		data[i].z = model->m_pVertices[v + 2];

		if (model->m_pNormals)
		{
			data[i].nx = model->m_pNormals[v + 0];
			data[i].ny = model->m_pNormals[v + 1];
			data[i].nz = model->m_pNormals[v + 2];
		}
		else
		{
			data[i].nx = data[i].ny = data[i].nz = 0.0f;
		}

		data[i].color = STRANDED_COLOR_ARGB(255, 255, 255, 255);

		if (model->m_pTexCoords)
		{
			data[i].tu = model->m_pTexCoords[t + 0];
			data[i].tv = model->m_pTexCoords[t + 1];
		}
		else
		{
			data[i].tu = data[i].tv = 0.0f;
		}
	}

	if (FAILED(m_Device->CreateVertexBuffer(totalVerts*stride, D3DUSAGE_WRITEONLY, m_staticBufferList[staticID].fvf, D3DPOOL_DEFAULT, &m_staticBufferList[staticID].vbPtr, NULL)))
		return STRANDED_FAIL;

	if (FAILED(m_staticBufferList[staticID].vbPtr->Lock(0, 0, (void**)&ptr, 0)))
		return STRANDED_FAIL;
	memcpy(ptr, data, totalVerts*stride);
	m_staticBufferList[staticID].vbPtr->Unlock();

	if (data)
	{
		delete[] data;
		data = nullptr;
	}

	FreeOBJModel(model);

	if (model)
	{
		delete[] model;
		model = nullptr;
	}

	return STRANDED_OK;
}

/*
 * @brief: 将 X 模型加载到渲染器中的 X 模型链表
 * @param: 加载的文件名、存储 X 模型链表 ID 的地址
 */
int CD3DRenderer::LoadXModel(const char* file, int *xModelId)
{
	if (!file) return STRANDED_FAIL;

	// 增加 X 模型链表
	if (!m_xModels)
	{
		m_xModels = new CXModel[1];
		if (!m_xModels) return STRANDED_FAIL;

		// 提供 Direct3D 设备
		m_xModels[0].SetDevice(m_Device);
	}
	else
	{
		CXModel *temp;
		temp = new CXModel[m_numXModels + 1];
		
		memcpy(temp, m_xModels, sizeof(CXModel)*m_numXModels);

		delete[] m_xModels;
		m_xModels = temp;

		m_xModels[m_numXModels].SetDevice(m_Device);
	}


	//////////////////////////////////////////////////////////////////////////
	// 将模型加载到内存
	//LPD3DXBUFFER matBuffer = NULL;
	//unsigned long numMats = 0;

	/*
	 * 将 X 模型加载到 LPD3DXMESH 对象中，并在 Direct3D 中使用该模型
	 * D3D_OK, 加载成功; 否则加载过程出现错误，模型没有被加载到内存中
	 *
		HRESULT D3DXLoadMeshFromX(
			LPCTSTR pFilename,					// X 文件名
			DWORD Options,						// 加载网格的选项标识符
			LPDIRECT3DDEVICE9 pD3DDevice,		// Direct3D 设备对象
			LPD3DXBUFFER* ppAdjacency,			// 存储临近数据(三角形索引)的 LPD3DXBUFFER
			LPD3DXBUFFER* ppMaterials,			// 存储定义在文件中的材质的缓存
			LPD3DXBUFFER* ppEffectInstances,	// 存储在文件中使用的效果(阴影器)实例的缓存
			DWORD* pNumMaterials,				// 指向材质总数的指针
			LPD3DXMESH* ppMesh					// 该函数调用创建的网格对象地址
		);
	 */
	/*
	if (FAILED(D3DXLoadMeshFromX(file, D3DXMESH_SYSTEMMEM, m_Device, NULL, &matBuffer, NULL, &numMats, &m_xModels[m_numXModels].model)))
		return STRANDED_FAIL;

	// Allocate the lists for materials and texture.
	m_xModels[m_numXModels].numMaterials = numMats;
	m_xModels[m_numXModels].matList = new D3DMATERIAL9[numMats];
	m_xModels[m_numXModels].textureList = new LPDIRECT3DTEXTURE9[numMats];

	// Get a pointer to the buffer
	D3DXMATERIAL* mat = (D3DXMATERIAL*)matBuffer->GetBufferPointer();

	// Loop and load each texture and get tach material.
	for (unsigned long i = 0; i < numMats; i++)
	{
		// Copy the materials from the buffer into our list.
		m_xModels[m_numXModels].matList[i] = mat[i].MatD3D;

		// Load the textures into the list.
		if (FAILED(D3DXCreateTextureFromFile(m_Device, mat[i].pTextureFilename, &m_xModels[m_numXModels].textureList[i])))
			m_xModels[m_numXModels].textureList[i] = NULL;
	}

	if (matBuffer != NULL)
	{
		matBuffer->Release();
		matBuffer = NULL;
	}
	*/
	//////////////////////////////////////////////////////////////////////////
	// XMesh.h 中的模型结构代替 [上面是静态网格。这里可以是静态网格，也可以是动态网格]
	if (!m_xModels[m_numXModels].LoadXFile(file))
		return STRANDED_FAIL;

	*xModelId = m_numXModels;
	m_numXModels++;
	
	return STRANDED_OK;
}

int CD3DRenderer::LoadXModel(const char* file, int xModelId)
{
	if (!file || xModelId < 0 || xModelId >= m_numXModels || !m_xModels)
		return STRANDED_FAIL;

	// 释放所有与 X 模型相关的数据
	m_xModels[xModelId].Shutdown();

	//////////////////////////////////////////////////////////////////////////
	/*
	LPD3DXBUFFER matBuffer = NULL;
	unsigned long numMats = 0;

	if (FAILED(D3DXLoadMeshFromX(file, D3DXMESH_SYSTEMMEM, m_Device, NULL, &matBuffer, NULL, &numMats, &m_xModels[xModelId].model)))
		return STRANDED_FAIL;

	// Allocate the lists for materials and texture.
	m_xModels[xModelId].numMaterials = numMats;
	m_xModels[xModelId].matList = new D3DMATERIAL9[numMats];
	m_xModels[xModelId].textureList = new LPDIRECT3DTEXTURE9[numMats];

	// Get a pointer to the buffer
	D3DXMATERIAL* mat = (D3DXMATERIAL*)matBuffer->GetBufferPointer();

	// Loop and load each texture and get tach material.
	for (unsigned long i = 0; i < numMats; i++)
	{
		// Copy the materials from the buffer into our list.
		m_xModels[xModelId].matList[i] = mat[i].MatD3D;

		// Load the textures into the list.
		if (FAILED(D3DXCreateTextureFromFile(m_Device, mat[i].pTextureFilename, &m_xModels[xModelId].textureList[i])))
			m_xModels[xModelId].textureList[i] = NULL;
	}

	if (matBuffer != NULL)
	{
		matBuffer->Release();
		matBuffer = NULL;
	}
	*/
	//////////////////////////////////////////////////////////////////////////
	// XMesh.h 中的模型结构代替 [上面是静态网格，这里支持静态网格，动画路径]
	if (!m_xModels[xModelId].LoadXFile(file))
		return STRANDED_FAIL;

	return STRANDED_OK;
}

/*
 * @brief: 释放 X 模型链表的所有内存
 */
void CD3DRenderer::ReleaseAllXModels()
{
	for (int s = 0; s < m_numXModels; s++)
	{
		m_xModels[s].Shutdown();
	}

	m_numXModels = 0;

	if (m_xModels)
	{
		delete[] m_xModels;
		m_xModels = nullptr;
	}
}

/*
 * @param: 要释放的单个模型
 */
int CD3DRenderer::ReleaseXModel(int xModelId)
{
	if (xModelId < 0 || xModelId >= m_numXModels || !m_xModels)
		return STRANDED_FAIL;

	m_xModels[xModelId].Shutdown();

	return STRANDED_OK;
}

void CD3DRenderer::ReleaseAllStaticBuffers()
{
	// 循环检查，释放所有的静态缓存数据，清理静态缓存列表
	for (unsigned int i = 0; i < m_numStaticBuffers; ++i)
	{
		ReleaseStaticBuffer(i);
	}

	m_numStaticBuffers = 0;

	if (m_staticBufferList)
	{
		delete[] m_staticBufferList;
		m_staticBufferList = NULL;
	}
}

/*
 * @brief: 删除与id关联的所有动态内存
 * @param: staticId - 正要释放的静态缓存 ID
 */
int CD3DRenderer::ReleaseStaticBuffer(unsigned int staticId)
{
	if (staticId < 0 || staticId >= m_numStaticBuffers || !m_staticBufferList)
		return STRANDED_FAIL;

	m_staticBufferList[staticId].fvf = 0;
	m_staticBufferList[staticId].numIndices = 0;
	m_staticBufferList[staticId].numVerts = 0;
	m_staticBufferList[staticId].stride = 0;

	if (m_staticBufferList[staticId].vbPtr)
	{
		m_staticBufferList[staticId].vbPtr->Release();
		m_staticBufferList[staticId].vbPtr = NULL;
	}

	if (m_staticBufferList[staticId].ibPtr)
	{
		m_staticBufferList[staticId].ibPtr->Release();
		m_staticBufferList[staticId].ibPtr = NULL;
	}

	return STRANDED_OK;
}

/*
 * @brief: 透明度 - 启用混合模式渲染对象
 *		要为纹理图加载图像必须有一个alpha通道，或是使用D3DCOLOR_ARGB宏而不是
 *		D3DCOLOR_XRGB，一边在顶点结构中使用alpha
 * @param: 1 正在设置的渲染状态
 * @param: 2 源混合操作
 * @param: 3 目的混合操作
 */
void CD3DRenderer::SetTransparency( RenderState state, TransState src, TransState dst )
{
	if(!m_Device) return;

	if(state == TRANSPARENCY_NONE)
	{
		m_Device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		return;
	}

	if(state == TRANSPARENCY_ENABLE)
	{
		// 启动 Direct3D Alpha 混合
		m_Device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);

		// 为要混合的源对象设置一个参数
		switch(src)
		{
		case TRANS_ZERO:
			m_Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
			break;
		case TRANS_ONE:
			m_Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
			break;
		case TRANS_SRCCOLOR:
			m_Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCCOLOR);
			break;
		case TRANS_INVSRCCOLOR:
			m_Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_INVSRCCOLOR);
			break;
		case TRANS_SRCALPHA:
			m_Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			break;
		case TRANS_INVSRCALPHA:
			m_Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_INVSRCALPHA);
			break;
		case TRANS_DSTALPHA:
			m_Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTALPHA);
			break;
		case TRANS_INVDSTALPHA:
			m_Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_INVDESTALPHA);
			break;
		case TRANS_DSTCOLOR:
			m_Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
			break;
		case TRANS_INVDSTCOLOR:
			m_Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_INVDESTCOLOR);
			break;
		case TRANS_SRCALPHASAT:
			m_Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHASAT);
			break;
		case TRANS_BOTHSRCALPHA:
			m_Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_BOTHSRCALPHA);
			break;
		case TRANS_INVBOTHSRCALPHA:
			m_Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_BOTHINVSRCALPHA);
			break;
		case TRANS_BLENDFACTOR:
			m_Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_BLENDFACTOR);
			break;
		case TRANS_INVBLENDFACTOR:
			m_Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_INVBLENDFACTOR);
			break;
		default:
			m_Device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
			return;
			break;
		}

		// 为正在渲染用的目的缓存设置一个参数
		switch(dst)
		{
		case TRANS_ZERO:
			m_Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
			break;
		case TRANS_ONE:
			m_Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
			break;
		case TRANS_SRCCOLOR:
			m_Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR);
			break;
		case TRANS_INVSRCCOLOR:
			m_Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCCOLOR);
			break;
		case TRANS_SRCALPHA:
			m_Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCALPHA);
			break;
		case TRANS_INVSRCALPHA:
			m_Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
			break;
		case TRANS_DSTALPHA:
			m_Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_DESTALPHA);
			break;
		case TRANS_INVDSTALPHA:
			m_Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVDESTALPHA);
			break;
		case TRANS_DSTCOLOR:
			m_Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_DESTCOLOR);
			break;
		case TRANS_INVDSTCOLOR:
			m_Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVDESTCOLOR);
			break;
		case TRANS_SRCALPHASAT:
			m_Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCALPHASAT);
			break;
		case TRANS_BOTHSRCALPHA:
			m_Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_BOTHSRCALPHA);
			break;
		case TRANS_INVBOTHSRCALPHA:
			m_Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_BOTHINVSRCALPHA);
			break;
		case TRANS_BLENDFACTOR:
			m_Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_BLENDFACTOR);
			break;
		case TRANS_INVBLENDFACTOR:
			m_Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVBLENDFACTOR);
			break;
		default:
			m_Device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
			break;
		}
	}
}

/*
 * @param: 从文件中加载的图像文件名
 * @param: 存储新创建的纹理对象的纹理ID
*/
int CD3DRenderer::AddTexture2D( char *file, int *texId )
{
	if(!file || !m_Device)
		return STRANDED_FAIL;

	int len = strlen(file);
	if(!len)
		return STRANDED_FAIL;

	// 检测纹理是否已创建
	for (int i = 0; i < m_numTextures; ++i)
	{
		if (strcmp(m_textureList[i].fileName, file) == 0)
		{
			*texId = i;
			return STRANDED_OK;
		}
	}

	int index = m_numTextures;

	if(!m_textureList)
	{
		m_textureList = new stD3DTexture[1];
		if(!m_textureList)
			return STRANDED_FAIL;
	}
	else
	{
		stD3DTexture *temp;
		temp = new stD3DTexture[m_numTextures + 1];

		memcpy(temp, m_textureList, sizeof(stD3DTexture) * m_numTextures);

		delete[] m_textureList;
		m_textureList = temp;
	}

	// 将文件名复制给纹理对象
	m_textureList[index].fileName = new char[len+1];
	memcpy(m_textureList[index].fileName, file, len);
	m_textureList[index].fileName[len] = '\0';

	D3DCOLOR colorkey = 0xff000000;
	D3DXIMAGE_INFO info;

	/*
	 * @brief: 通知Direct3D从指定文件加载纹理并将其加载到内存
	 * HRESULT WINAPI D3DXCreateTextureFromFile(LPDIRECT3DDEVICE9 pDevice, LPCTSTR, pSrcFile, LPDIRECT3DTEXTURE9 *ppTexture);
	 *
	 * @brief: 从文件加载纹理图像，生成mipmaps，在对象中保存纹理图像的宽度和高度，并将对象添加到纹理列表中. 然后保存纹理ID，并增加纹理总数.
	 * HRESULT WINAPI D3DXCreateTextureFromFileEx(LPDIRECT3DDEVICE9 pDevice, LPCTSTR pSrcFile, UINT Width, UINT Height, UINT MipLevels, DWORD Usage, D3DFORMAT Format,
	 *	D3DPOOL Pool, DWORD Filter, DWORD MipFilter, D3DCOLOR ColorKey, D3DXIMAGE_INFO *pSrcInfo, PALETTEENTRY *pPalette, LPDIRECT3DTEXTURE9 *ppTexture);
	 * @param: pSrcFile - 加载图像位置的字符串
	 * @param: Filter - 处理图像质量，并控制 Direct3D 填充图像数据的方法(在指明纹理图像为原始图像宽度和高度一半D3DX_FILTER_BOX[用2*2盒子将图像像素平均处理]，或是比原始图像要小有用)
	 * @param: MipFilter - 用于纹理图像的 mipmap 外同Filter. 滤镜目的为了控制要加载到图形应用程序中的纹理图像质量(在不放弃全部图像质量的情况下，使用低质量的滤波，获取应有纹理图像质量)
	 * @param: ColorKey - 指定图像中像素透明或不可见是的颜色值，可将与发送给该参数的颜色相配的每个像素设为透明，并且可以用纯黑色替代(对哪些有一部分内容不可见的图像，如菜单按钮和角色的二维图像有用)
	 * @param: pSrcInfo - 包含原始图像所有信息结构 [原始宽度，高度，格式，深度，Mip级别，资源类型，文件格式类型]
	 * @param: pPalette - 返回用该函数加载的所有经过调色处理的纹理图像的调色板.(一般图像是8位)
	 */
	if(D3DXCreateTextureFromFileEx(m_Device, file, 0, 0, 0, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, colorkey, &info, NULL, &m_textureList[index].image) != D3D_OK)
		return false;

	//在对象中保存纹理图像的宽度和高度
	m_textureList[index].width = info.Width;
	m_textureList[index].height = info.Height;

	//保存纹理ID
	*texId = m_numTextures;
	//增加纹理总数
	m_numTextures ++;

	return STRANDED_OK;
}

/*
 * @brief: 可与min、mag和mip滤波器一起使用，也可在采样器阶段使用点、线和各向异性滤波
 *
 * @param: 1、纹理单位或采样器阶段
 * @param: 2、滤波器模式
 * @param: 3、要使用的滤波器值
 *
 * @sample:
 * // Set the image states to get a good quality image.
 * m_Device->SetSampleState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
 * m_Device->SeetSampleState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
 */
void CD3DRenderer::SetTextureFilter( int index, int filter, int val )
{
	if (!m_Device || index < 0) return;

	D3DSAMPLERSTATETYPE fil = D3DSAMP_MINFILTER;
	int v = D3DTEXF_POINT;

	if(filter == MIN_FILTER)
		fil = D3DSAMP_MINFILTER;
	if(filter == MAG_FILTER)
		fil = D3DSAMP_MAGFILTER;
	if(filter == MIP_FILTER)
		fil = D3DSAMP_MIPFILTER;

	if(val == POINT_TYPE)
		v = D3DTEXF_POINT;			// 最近点采样模式
	if(val == LINEAR_TYPE)
		v = D3DTEXF_LINEAR;			// 线性滤波或三线滤波
	if(val == ANISOTROPIC_TYPE)
		v = D3DTEXF_ANISOTROPIC;	// 各项同性滤波

	m_Device->SetSamplerState(index, fil, v);
}

// 设置多纹理贴图
void CD3DRenderer::SetMultiTexture()
{
	if(!m_Device)
		return;

	/*
	 * HRESULT SetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value);
	 * @brief: 为纹理设置多纹理状态
	 * @param: 要处理的纹理索引
	 * @param: 所设置的纹理阶段
	 * @param: 设置阶段的标识符
	 */
	// Set the texture stages for the first texture unit.
	// Direct3D识别正在处理第一个索引
	m_Device->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
	// 通知 Direct3D 正在用另一个参数(参数2)乘以该纹理
	m_Device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	// 设置参数1：用 D3DTA_TEXTURE 将纹理发送给参数1
	m_Device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	// 设置参数2：用 D3DTA_DIFFUSE 将对象颜色发送给参数2
	m_Device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

	// Set the texture stages for the second texture unit.
	m_Device->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1);
	m_Device->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE);
	m_Device->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	// 设置参数2：用第二个纹理调整第一个纹理结果[用第一个纹理和顶点颜色混合第二个纹理]
	m_Device->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
}

/*
 * @brief:
 *		凸凹贴图
 * 提取纹理并使用该纹理的内部信息去照射表面，让表面显示出比自身实际拥有的更多细节.
 * 在照射几何图形是，通过保存在纹理图像中的法线值而不是表面的法线值完成的.
 * 通过改变图像中的法线值或像素，可得到物体的外观细节.
 * 纹理映射和凸凹映射之间的差别在于纹理用于阴影照射表面，而凸凹贴图用于灯光照射表面.
 * 凸凹贴图使用"法线贴图". 图像中每个像素的 R、G 和 B 成分实际上代表了法线的 X、Y 和 Z 轴.
 *
 * 凸凹映射技术使用的图像创建于灰度(从黑到白)2D纹理图像[高度图]. 法线映射技术使用的图像创建
 * 于高分辨率模型(角色、物体等)中的细节，并将其保存到纹理中，这样在凸凹映射中使用使，低分辨率
 * 模型看上去就像是或是类似于高分辨率模型. 法线映射是一种凸凹映射，它用于使低分辨率模型看上去
 * 像是高分辨率模型，而凸凹映射提取所有的2D图像，并从中创建法线.
 *
 * 从凸凹映射，可使用图像提取数据，并将其用于光照，而不是纹理渲染表面.
 * 单像素光照质量比单顶点光照质量高，因为图像中的每个像素都有自己的法线，可以使用单像素光照照射表面.
 * 改变这些法线值可以得到小而精细的细节，提高整个场景的感官效果.
 *
 * HRESULT WINAPI D3DXComputeNormalMap(LPDIRECT3DTEXTURE9 pTexture, LPDIRECT3DTEXTURE9 pSrcTexture, const PALETTEENTRY *pSrcPalette, DWORD Flags, DWORD Channel, FLOAT Amplitude);
 * @brief: 将 Direct3D 支持的所有图像格式转换成凸凹贴图
 * @extra: D3DXSaveTextureToFile (可将凸凹贴图保存到文件)
 * @param: pTexture - 保存法线贴图的Direct3D 纹理对象
 * @param: pSrcTexture - 想要转换成法线贴图的原始图像
 * @param: pSrcPalette - 原始资源纹理的调色板
 * @param: Flags - D3DX_NORMAL_MIRROR_U, D3DX_NORMAL_MIRROR_V, D3DX_NORMAL_MIRROR, D3DX_NORMAL_INVERTSIGN, D3DX_NORMAL_CUMPUTE_OCCLUSION
 * @param: Channel - D3DX_CHANNEL_RED: 计算发现贴图时使用红色通道
 *		D3DX_CHANNEL_GREEN: 计算发现贴图时使用绿色通道
 *		D3DX_CHANNEL_BLUE: 计算法线贴图时使用蓝色通道
 *		D3DX_CHANNEL_ALPHA: 计算法线贴图是使用 alpha 通道
 *		D3DX_CHANNEL_LUMINANCE: 计算发现贴图时使用红色、绿色和蓝色通道的亮度值
 * @param: Amplitude - Normal Map 的强度值(增加值或减少值)
 */
bool CD3DRenderer::CreateNormalMap(const char* file, LPDIRECT3DTEXTURE9 pNormalMap)
{
	if (!m_Device) return false;

	LPDIRECT3DTEXTURE9 pTexture = nullptr;

	// Load the original texture image from file.
	if (D3DXCreateTextureFromFile(m_Device, file, &pTexture) != D3D_OK)
		return false;

	if (!pTexture) return false;

	// Get the original image's information.
	D3DSURFACE_DESC desc;
	pTexture->GetLevelDesc(0, &desc);

	/*
	 * @brief: Create empty normal map texture the size of the original image.
	 * 创建空纹理图像
	 * @param: 纹理的宽度、高度
	 * @param: 纹理中的 Mipmap 的个数(MipLevels)
	 * @param: Usage - 要渲染纹理[D3DUSAGE_RENDERTARGET], 动态纹理[D3DUSAGE - DYNAMIC]
	 * @param: Format - 从文件加载图像可设为0，由Direct3D根据文件内容选择正确图像格式
	 * @param: Pool - 纹理对象驻留内存类别.D3DPOOL_DEFAULT[内存放置在视频内存，默认], D3DPOOL_SYSTEMMEM[将纹理对象保存在计算机RAM内存，
	 *	如果Direct3D设备丢失，无需重新创建资源.]
	 * @param: 正在创建的纹理对象 LPDIRECT3DTEXTIRE9
	 * HRESULT WINAPI D3DXCreateTexture(LPDIRECT3DDEVICE9 pDevice, UINT Width, UINT Height, UINT MipLevels, DWORD Usage,
	 *		D3DFORMAT Format, D3DPOOL Pool, LPDIRECT3DTEXTURE9 *ppPtexture);
	 *
	 * @breif: Set texture to offscreen surface.
	 * 创建幕外表面[从纹理对象中获取表面对象，即要渲染的表面，纹理对象将场景存储为一幅图像，不将其显示在屏幕上]
	 * @param: Level 是创建表面是的纹理资源级别
	 * @param: ppSurfaceLevel 是正在创建的 LPDIRECT3DSURFACE9
	 * HRESULT GetSurfaceLevel(UINT Level, IDirect3DSurface9 **ppSurfaceLevel);
	 *
	 * @brief: 设置渲染目标
	 * @param: 渲染目标索引
	 * @param: 要绘制的 LPDIRECTSURFACE9 对象
	 * HRESULT SetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9 *pRenderTarget);
	 *
	 * 幕外渲染：
	 * @prepare:
	 * 1、创建一个保存渲染结果的 LPDIRECT3DTEXTURE9 对象 [D3DXCreateTexture]
	 * 2、创建幕外表面 [GetSurfaceLevel]
	 * @render:
	 * 3、保存一份后台缓存表面的副本 [GetBackBuffer]
	 * 4、转化到渲染目标 [SetRenderTarget]
	 * 5、处理完幕外表面，返回正常渲染
	 * @sample:
		#include <d3d9.h>
		#include <d3dx9.h>

		#define WINDOW_CLASS "UGPDX"
		#define WINDOW_NAME "Off-Screen Rendering"
		#define WINDOW_WIDTH 640
		#define WINDOW_HEIGHT 480

		// Function Prototypes...
		bool InitializeD3D(HWND hWnd, bool fullscreen);
		bool InitializeObjects();
		void RenderScene();
		void Shutdown();

		// Direct#D object and device.
		LPDIRECT3D9 g_D#D = NULL;
		LPDIRECT3DDEVICE9 g_D3DDevice = NULL;

		// Matrices.
		D3DXMATRIX g_projection;
		D3DXMATRIX g_ViewMatrix;

		// stD3DVertex buffer to hold the square's geometry.
		LPDIRECT3DVERTEXBUFFER9 g_VertexBuffer = NULL;

		// g_Teapot object and its material.
		LPD3DXMESH g_Teapot = NULL;
		D3DMATERIAL9 g_Material;
		float g_RotationAngle = 0.0f;

		// Light object.
		D3DLIGHT9 g_Light;

		// Back buffer, offscreen texture, offscreen surface.
		LPDIRECT3DSURFACE9 g_BackSurface = NULL;
		LPDIRECT3DSURFACE9 g_SurfaceTexture = NULL;
		LPDIRECT3DSURFACE9 g_OffScreenSurface = NULL;

		// A structure for our custom vertex type
		sturct stD3DVertex
		{
			float x, y, z;
			float tu, tv;
		};

		// Our custom FVF, which describes our custom vertex structure.
		#define D3DFVF_VERTEX (D3DFVF_XYZ | D3DFVF_TEX1)

		bool InitializeObjects()
		{
			// Setup the g_Light source.
			g_Light.Type = D3DLIGHT_DIRECTIONAL;
			g_Light.Direction = D3DXVECTOR3(0.0f, 0.0f, 1.0f)
			g_Light.Diffuse.r = 1;
			g_Light.Diffuse.g = 1;
			g_Light.Diffuse.b = 1;
			g_Light.Diffuse.a = 1;
			g_Light.Specular.r = 1;
			g_Light.Specular.g = 1;
			g_Light.Specular.b = 1;
			g_Light.Specular.a = 1;

			g_D3DDevice->SetLight(0, &g_Light);
			g_D3DDevice->LightEnable(0, TRUE);

			// Setup the material properties for the teapot.
			ZeroMemory(&g_Material, sizeof(D3DMATERIAL9);
			g_Material.Diffuse.r = g_Material.Ambient.r = 0.6f;
			g_Material.Diffuse.g = g_Material.Ambient.g = 0.6f;
			g_Material.Diffuse.b = g_Material.Ambient.b = 0.7;
			g_Material.Specular.r = g_Material.Specular.g = g_Material.Specular.b = 0.4f;
			g_Material.Power = 8.0f;

			// Create the teapot.
			if(FAILED(D3DXCreateTeapot(g_D3DDevice, &g_Teapot, NULL)))
				return false;

			// Create the texture that will be rendered to.
			if(FAILED(D3DXCreateTexture(g_D3DDevice, WINDOW_WIDTH, WINDOW_HEIGHT, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &g_SurfaceTexture))) return false;

			// Set Texture to offscreen surface.
			g_SurfaceTexture->GetSurfaceLevel(0, &g_OffScreenSurface);

			// Square that will be textured with offscreen rendering data.
			stD3DVertex square[] =
			{
				{-1, 1, -1, 0, 0}, {1, 1, -1, 1, 0},
				{-1, -1, -1, 0, 1}, {1, -1, -1, 1, 1}
			};

			g_D3DDevice->CreateVertexBuffer(4*sizeof(stD3DVertex), 0, D3DFVF_VERTEX, D3DPOOL_DEFAULT, &g_VertexBuffer, NULL);

			void *pData = NULL;
			g_VertexBuffer->Lock(0, sizeof(square), (void**)&pData, 0);
			memcpy(pData, square, sizeof(square));
			g_VertexBuffer->Unlock();

			// Set the image states to get a good quality image.
			g_D3DDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
			g_D3DDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

			// Set default rendering states.
			g_D3DDevice->SetRenderState(D3DRS_LIGHTING, TRUE);
			g_D3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
			g_D3DDevice->SetRenderState(D3DRS_ZENABLE, TRUE);

			// Set the projection matrix.
			D3DXMatrixPerspectiveFovLH(&g_projection, 45.0f, WINDOW_WIDTH/WINDOW_HEIGHT, 0.1f, 1000.0f);
			g_D3DDevice->SetTransform(D3DTS_PROJECTION, &g_projection);

			// Define camera information.
			D3DXVECTOR3 cameraPos(0.0f, 0.0f, -4.0f);
			D3DXVECTOR3 lookAtPos(0.0f, 0.0f, 0.0f);
			D3DXVECTOR3 upDir(0.0f, 1.0f, 0.0f);

			// Build view matrix.
			D3DXMatrixLookAtLH(&g_ViewMatrix, &cameraPos, &lookAtPos, &upDir);

			return true;
		}

		void RenderScene()
		{
			// Get a copy of the back buffer.
			g_D3DDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &g_BackSurface);

			// Prepare to draw to the offscreen surface.
			g_D3DDevice->SetRenderTarget(0, g_OffScreenSurface);

			// Clear the offscreen surface.
			g_D3DDevice->Clear(0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(200, 200, 200), 1.0f, 0);

			// Begin the scene. Start rendering.
			g_D3DDevice->BeginScene();

				// Turn on lighting.
				g_D3DDevice->SetRenderState(D3DRS_LIGHTING, TRUE);

				// Set projection.
				g_D3DDevice->SetTransform(D3DTS_PROJECTION, &g_projection);

				// Create rotation matrix to rotate teapot.
				D3DXMATRIXA16 w;
				D3DXMatrixRotationY(&w, g_RotationAngle);
				g_D3DDevice->SetTransform(D3DTS_WORLD, &w);

				// Add to the rotation.
				g_RotationAngle += 0.02f;
				if(g_RotationAngle >= 360) g_RotationAngle = 0.0f;

				// Apply the view (camera).
				g_D3DDevice->SetTransform(D3DTS_VIEW, &g_ViewMatrix);

				// Set the material and draw the Teapot.
				g_D3DDevice->SetMaterial(&g_Material);
				g_D3DDevice->SetTexture(0, NULL);
				g_Teapot->DrawSubset(0);

			// End the scene. Stop rendering.
			g_D3DDevice->EndScene();

			// Switch back to our back buffer.
			g_D3DDevice->SetRenderTarget(0, g_BackSurface);

			// Clear the back buffer.
			g_D3DDevice->Clear(0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);

			// Begin the scene. Start Rendering.
			g_D3DDevice->BeginScene();
				
				// Turn off lighting. Don't need it.
				g_D3DDevice->SetRenderState(D3DRS_LIGHTING, FALSE);

				// Set projection.
				g_D3DDevice->SetTransform(D3DTS_PROJECTION, &g_projection);

				// Rotate just a little to see this is a flat surface.
				D3DXMatrixRotationY(&w, 120);
				g_D3DDevice->SetTransform(D3DTS_WORLD, &w);

				// Apply the view (camera).
				g_D3DDevice->SetTransform(D3DTS_VIEW, &g_ViewMatrix);

				g_D3DDevice->SetTexture(0, g_SurfaceTexture);
				g_D3DDevice->SetStreamSource(0, g_VertexBuffer, 0, sizeof(stD3DVertex));

				g_D3DDevice->SetFVF(D3DFVF_VERTEX);
				g_D3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);

			// End the scene. Stop rendering.
			g_D3DDevice->EndScene();

			// Display the scene.
			g_D3DDevice->Present(NULL, NULL, NULL, NULL);
		}

		void Shutdown()
		{
			if(g_D3DDevice != NULL)
			{
				g_D3DDevice->Release();
				g_D3DDevice = NULL;
			}

			if(g_D3D != NULL)
			{
				g_D3D->Release();
				g_D3D = NULL;
			}

			if(g_VertexBuffer != NULL)
			{
				g_VertexBuffer->Release();
				g_VertexBuffer = NULL;
			}

			if(g_Teapot != NULL)
			{
				g_Teapot->Release();
				g_Teapot = NULL;
			}

			if(g_BackSurface != NULL)
			{
				g_BackSurface->Release();
				g_BackSurface = NULL;
			}

			if(g_SurfaceTexture != NULL)
			{
				g_SurfaceTexture->Release();
				g_SurfaceTexture = NULL;
			}

			if(g_OffScreenSurface != NULL)
			{
				g_OffScreenSurface->Release();
				g_OffScreenSurface = NULL;
			}
		}
	 */
	if (D3DXCreateTexture(m_Device, desc.Width, desc.Height, 0, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &pNormalMap) != D3D_OK)
		return false;

	if (!pNormalMap) return false;

	// Compute the normal map.
	// 通常是灰度图
	if (D3DXComputeNormalMap(pNormalMap, pTexture, 0, D3DX_NORMALMAP_MIRROR, D3DX_CHANNEL_GREEN, 10) != D3D_OK)
		return false;

	return true;
}

/*
 * @brief: 为正在使用的渲染系统添加纹理
 * @param: 1、要使用的纹理单位
 * @param: 2、要使用的纹理纹理对象的纹理ID
*/
void CD3DRenderer::ApplyTexture( int index, int texId )
{
	if(!m_Device)
		return;

	// 如果所有参数都无效，给SetTexture函数发送一个NULL，清空限定在采样器阶段的所有纹理
	// 否则将撤销对纹理对象的绑定
	if(index < 0 || texId < 0)
		m_Device->SetTexture(0, NULL);
	else
		/*
		 * HRESULT SetTexture(DWORD Sampler, IDirect3DBaseTexture9* pTexture);
		 * @param: Sampler - 纹理要施加的采样器阶段[0~1].支持纹理的最大数量取决于硬件，Direct3D的GetDeviceCaps函数填充D3DCAP9对象:存储了采样器阶段总数(MaxSimulataneousTexture, MaxTextureBlendStages)
		 */
		m_Device->SetTexture(0, m_textureList[texId].image);
}

/*
 * @brief: 在渲染系统保存屏幕截图
 * @param: 1、屏幕截图要保存的文件名
 * @extra:  s键保存
 * if((GetKeyState('s')&0x80 || GetKeyState('S')&0x80))
 * { SaveScreenShot(); }
 */
void CD3DRenderer::SaveScreenShot( char *file )
{
	if(!file || !m_Device)
		return;

	LPDIRECT3DSURFACE9 surface = NULL;
	D3DDISPLAYMODE disp;

	/*
	 * 获取当前的显示模式[获取渲染场景/显示的宽度和高度等信息]
	 * 参数: 交换链索引、要填充对戏那个的显示模式
	 */
	m_Device->GetDisplayMode(0, &disp);
	/*
	 * 创建一个幕外表面
	 * 参数: 表面的宽度、高度...
	 */
	m_Device->CreateOffscreenPlainSurface(disp.Width, disp.Height,
		D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &surface, NULL);

	/*
	 * 将渲染后的场景复制到创建好的表面对象中
	 * 参数: 交换链索引、后台缓存索引、后台缓存类型(DirectX 9.0 仅支持 D3DBACKBUFFER_TYPE_MODE)、保存已渲染的场景的表面对象
	 */
	m_Device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &surface);
	/*
	 * 将表面保存到支持的文件格式中
	 * 参数: 保存图像时想要使用的文件名
	 * 保存的文件类型[D3DXIFF_BMP, D3DXIFF_JPG, D3DXIFF_TGA, D3DXIFF_PNG, D3DXIFF_DDS, D3DXIFF_PPM, D3DXIFF_DIB, D3DXIFF_HDR, D3DXIFF_PFM]
	 * 包含渲染数据的表面对象
	 * 保存文件是使用的调色板(可NULL)
	 * 定义要保存的表面区域的RECT结构(若为NULL，保存整幅图像)
	 */
	D3DXSaveSurfaceToFile(file, D3DXIFF_JPG, surface, NULL, NULL);

	if(surface != NULL)
	{
		surface->Release();
		surface = NULL;
	}
}

/*
 * @brief: sprite(子图形) 2D图像集合，使用 alpha 通道，分解图像中希望不可见的像素
 *		2D 总面对屏幕, 3D 对着摄像机
 *
 * 点状 sprite， 渲染 DrawPrimitive(D3DPT_POINTLIST)，可在渲染前绑定纹理
 * @param: 1、sprite的尺寸
 * @param: 2、要设置的最小尺寸
 * @param: 3、A、B和C的刻度值
 */
void CD3DRenderer::EnablePointSprites( float size, float min, float a, float b, float c )
{
	if(!m_Device)
		return;

	// 启用点状 sprite
	m_Device->SetRenderState(D3DRS_POINTSPRITEENABLE, TRUE);
	// 启用点状 sprite 的比例 (可色绘制比例值以及与距离调整单个 sprite 的尺寸)
	m_Device->SetRenderState(D3DRS_POINTSCALEENABLE, TRUE);
	// 设置 sprite 的尺寸
	m_Device->SetRenderState(D3DRS_POINTSIZE, FtoW(size));
	// 设置点状 sprite 的最小尺寸
	m_Device->SetRenderState(D3DRS_POINTSIZE_MIN, FtoW(min));
	// 设置点状 sprite 比例(默认值为1)，根据距离更改点状 sprite 的形状
	m_Device->SetRenderState(D3DRS_POINTSCALE_A, FtoW(a));
	m_Device->SetRenderState(D3DRS_POINTSCALE_B, FtoW(b));
	m_Device->SetRenderState(D3DRS_POINTSCALE_C, FtoW(c));
}

void CD3DRenderer::DisablePointSprites()
{
	// 禁用点状 sprite
	m_Device->SetRenderState(D3DRS_POINTSPRITEENABLE, FALSE);
	// 禁用点状 sprite 的比例
	m_Device->SetRenderState(D3DRS_POINTSCALEENABLE, FALSE);
}

/*
 * @brief: 创建一个 D3D 字体对象
 * @param:
 *		字体类型名称(Times New Roman)
 *		打印字体的权重(粗体)
 *		字体是否斜体
 *		字体对象大小(字符高度)
 *		ID指针(字体对象数组索引)
 */
bool CD3DRenderer::CreateText(char* font, int weight, bool italic, int size, int& id)
{
	if (!m_fonts)
	{
		m_fonts = new LPD3DXFONT[1];
		if (!m_fonts) return STRANDED_FAIL;
	}
	else
	{
		LPD3DXFONT* temp;
		temp = new LPD3DXFONT[m_totalFonts + 1];
		memcpy(temp, m_fonts, sizeof(LPD3DXFONT)*m_totalFonts);

		delete[] m_fonts;
		m_fonts = temp;
	}

	/*
	 * @brief: 创建 LPD3DXFONT 对象
	 * @param:
			Height: 字体字符的高度(通常字体的大小)
			Width: 字体字符的宽度(字体附加的宽度，通常取值0)
			Weight: 打印字体的权重(即: 粗体)
			MipLevels: 文本的 Mipmap 级别(文本不能像纹理图像那样可从屏幕更深入显示，设为1)
			Italic: 字体是否为斜体
			Charset: 用到的字符集
			OutputPrecision: 指定 Windows 将期望字体与实际字体匹配的方法[如果确保正在使用是TrueType字体，设为OUT_TT_ONLY_PRECIS]
			Quality: 控制文本特性，只影响到raster(光栅)字体，不会影响TrueType字体
			PitchAndFamily: 控制文本使用的 pitch(斜度)和 family 索引
			pFaceName: 要用的字体类型名称(例如: Arial、Times New Roman)
			ppFont: 要填充的 LPD3DFONT 对象
		HRESULT WINAPI D3DXCreateFont(LPDIRECT3DDEVICE9 pDevice, INT Height, UINT Width, UINT Weight, UINT MipLevels, BOOL Italic, DWORD CharSet, DWORD OutputPrecision, DWORD Quality, DWORD PitchAndFamily, LPCTSTR pFacename, LPD3DXFONT *ppFont);
	 */
	if (FAILED(D3DXCreateFont(m_Device, size, 0, weight, 1, italic, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, font, &m_fonts[m_totalFonts])))
		return false;

	id = m_totalFonts;
	m_totalFonts++;

	return true;
}

void CD3DRenderer::DisplayText(int id, long x, long y, int r, int g, int b, char* text, ...)
{
	RECT fontPosition = {x, y, m_screenWidth, m_screenHeight};

	char message[1024];
	va_list argList;

	if (id >= m_totalFonts) return;

	va_start(argList, text);
#pragma warning(push)
#pragma warning(disable:4996)
	vsprintf(message, text, argList);
#pragma warning(pop)
	va_end(argList);

	/*
	 * @brief: 渲染文本对象，LPD3DXFONT->DrawText()
	 * @param:
	 *		pSprite: 指向包含字符串的 LPD3DXSPRITE 对象的指针，如果需要 Direct3D 内部构建每个角色的小图形，该值可以设为 NULL.
	 *		pString: 屏幕上要显示的文本
	 *		Count: 字符串中字符的数目
	 *		pRect: RECT对象，定义要显示在屏幕上的第一个字符的起始位置
	 *		Format: 指定字符串的显示方式.(XOR)
				DT_BOTTOM - 将文本调整到定义文本的RECT区域的底部
				DT_CALCRECT - 计算RECT的高度和宽度. 使用pRect参数指定的区域根据需要来调整矩形
				DT_CENTER - 将文本水平地调整到矩形的中间
				DT_EXPANDTABS - 拓展字符串中的 tab 字符。默认情况下 tab 是8个字符
				DT_LEFT - 将文本调整到矩形的左侧
				DT_NOCLIP - 无剪切地挥之文本，这样渲染速度更快
				DT_RIGHT - 将文本调整到矩形的右侧
				DT_RTLREADING - 按照从右向左的顺序渲染文本. 通常在 Hebrew 或 Arabic 字体中使用该值
				DT_SINGLELINE - 只在一行上显示文本. 忽略所有的换行字符
				DT_TOP - 将文本显示在矩形的顶部
				DT_VCENTER - 将文本垂直地调整到矩形的中间. 只使用单独的一行
				DT_WORDBREAK - 单词通过定义好的矩形时，将被换行
		INT DrawText(LPD3DXSPRITE pSprite, LPCTSTR pString, INT Count, LPRECT pRect, DWORD Format, D3DCOLOR Color);
	 */
	m_fonts[id]->DrawText(NULL, message, -1, &fontPosition, DT_SINGLELINE, D3DCOLOR_ARGB(255, r, g, b));
}

void CD3DRenderer::DisplayText(int id, long x, long y, unsigned long color, char* text, ...)
{
	RECT fontPosition = { x, y, m_screenWidth, m_screenHeight };

	char message[1024];
	va_list argList;

	if (id >= m_totalFonts) return;

	va_start(argList, text);
#pragma warning(push)
#pragma warning(disable:4996)
	vsprintf(message, text, argList);
#pragma warning(pop)
	va_end(argList);

	m_fonts[id]->DrawText(NULL, message, -1, &fontPosition, DT_SINGLELINE, color);
}

/*
 * 计算FPS:
 *		对于每帧，计数器加1。每帧末获取当前时间并确认从上一次查看时间起是否已经过了
 *	一秒多。如果过了一秒，则计数器值就是FPS.
 */
void CD3DRenderer::GetFPS()
{
	// 静态变量
	static int s_fps = 0;
	static int s_counter = 0;
	static float s_currentTime = 0.0f;
	static float s_lastTime = 0.0f;

	// Get time in millisecond then convert to secs.
	s_currentTime = ::GetTickCount() * 0.001f;

	// If time - last time is > than 1, save fps.
	if (s_currentTime - s_lastTime > 1.0f)
	{
		// Record last time.
		s_lastTime = s_currentTime;

		// Save FPS.
		s_fps = s_counter;

		// Reset the FPS counter.
		s_counter = 0;
	}
	else
	{
		// Add to the counter.
		s_counter++;
	}
}

bool CD3DRenderer::AddGUIBackdrop(int guiId, char *fileName)
{
	if(guiId >= m_totalGUIs)
		return false;

	int texID = -1, staticID = -1;

	if(!AddTexture2D(fileName, &texID))
		return false;

	unsigned long col = D3DCOLOR_XRGB(255, 255, 255);

	stGUIVertex obj[] = 
	{
		{(float)m_screenWidth, 0, 0, 1, col, 1, 0},
		{(float)m_screenWidth, (float)m_screenHeight, 0, 1, col, 1, 1},
		{0, 0, 0, 1, col, 0, 0},
		{0, (float)m_screenHeight, 0, 1, col, 0, 1}
	};

	if(!CreateStaticBuffer(GUI_FVF, TRIANGLE_STRIP, 4, 0, sizeof(stGUIVertex), (void**)&obj, NULL, &staticID))
		return false;

	return m_guiList[guiId].AddBackdrop(texID, staticID);
}

bool CD3DRenderer::AddGUIStaticText(int guiId, int id, char *text, int x, int y, unsigned long color, int fontID)
{
	if (guiId >= m_totalGUIs) return false;

	return m_guiList[guiId].AddStaticText(id, text, x, y, color, fontID);
}

bool CD3DRenderer::AddGUIButton(int guiId, int id, int x, int y, char *up, char *over, char *down)
{
	if (guiId >= m_totalGUIs) return false;

	int upID = -1, overID = -1, downID = -1, staticID = -1;

	// 加载 GUI 元素的纹理
	if (!AddTexture2D(up, &upID)) return false;
	if (!AddTexture2D(over, &overID)) return false;
	if (!AddTexture2D(down, &downID)) return false;

	unsigned long col = D3DCOLOR_XRGB(255, 255, 255);

	int w = m_textureList[upID].width;
	int h = m_textureList[upID].height;

	// 创建在屏幕上渲染该元素要用的几何形状
	stGUIVertex obj[] =
	{
		{(float)(w+x), (float)(0+y), 0, 1, col, 1, 0},	// x=w,y=0 右上角
		{(float)(w+x), (float)(h+y), 0, 1, col, 1, 1},	// x=w,y=h 右下角
		{(float)(0+x), (float)(0+y), 0, 1, col, 0, 0},	// x=0,y=0 左上角
		{(float)(0+x), (float)(h+y), 0, 1, col, 0, 1},	// x=0,y=h 左下角
	};

	if (!CreateStaticBuffer(GUI_FVF, TRIANGLE_STRIP, 4, 0, sizeof(stGUIVertex), (void**)&obj, NULL, &staticID))
		return false;

	// 将上述信息发送给自己所属的 GUI 对象
	return m_guiList[guiId].AddButton(id, x, y, w, h, upID, overID, downID, staticID);
}

void CD3DRenderer::ProcessGUI(int guiID, bool LMBDown, int mouseX, int mouseY, void(*funcPtr) (int id, int state))
{
	if (guiID >= m_totalGUIs || !m_Device) return;

	CGUISystem* gui = &m_guiList[guiID];
	if (!gui) return;

	// Draw background.
	stGUIControl* backDrop = gui->GetBackDrop();

	// Draw backdrop first to control render order. Don't want to draw this after buttons to 2D.
	if (backDrop)
	{
		ApplyTexture(0, backDrop->m_upTex);
		RenderStaticBuffer(backDrop->m_listID);
		ApplyTexture(0, -1);
	}

	// Initial button state.
	int status = STRANDED_BUTTON_UP;

	// Loop through all controls and display them.
	for (int i = 0; i < gui->GetTotalControls(); ++i)
	{
		// Get the current control.
		stGUIControl* pCnt = gui->GetGUIControl(i);
		if(!pCnt) continue;

		// Take action depending on what type it is.
		switch (pCnt->m_type)
		{
		case STRANDED_GUI_STATICTEXT:
			DisplayText(pCnt->m_listID, pCnt->m_xPos, pCnt->m_yPos, pCnt->m_color, pCnt->m_text);
			break;
		case STRANDED_GUI_BUTTON:
			status = STRANDED_BUTTON_UP;

			// Set alpha on for the texture image.
			m_Device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
			m_Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			m_Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

			// Check if over the button or pressing it.
			if (mouseX > pCnt->m_xPos && mouseX < (pCnt->m_xPos + pCnt->m_width) && mouseY > pCnt->m_yPos && mouseY < (pCnt->m_yPos + pCnt->m_height))
			{
				if (LMBDown)
					status = STRANDED_BUTTON_DOWN;
				else
					status = STRANDED_BUTTON_OVER;
			}

			// State will depend on it's texture.
			if (status == STRANDED_BUTTON_UP)
				ApplyTexture(0, pCnt->m_upTex);
			else if (status == STRANDED_BUTTON_OVER)
				ApplyTexture(0, pCnt->m_overTex);
			else
				ApplyTexture(0, pCnt->m_downTex);

			// Render button.
			RenderStaticBuffer(pCnt->m_listID);
			// Turn off alpha.
			m_Device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);

			break;
		}

		// Process control by calling the callback.
		if (funcPtr)
			funcPtr(pCnt->m_id, status);
	}
}

/*
 * @param:
 *		雾起点、终点位置, 雾的颜色, 确认是否正在使用基于范围的雾的标识符
 */
void CD3DRenderer::EnableFog(float start, float end, STRANDED_FOG_TYPE type, unsigned long color, bool rangeFog)
{
	if (!m_Device) return;

	// 获取设备性能
	D3DCAPS9 caps;
	m_Device->GetDeviceCaps(&caps);

	// Set fog properties.
	// 在API中启用雾效果
	m_Device->SetRenderState(D3DRS_FOGENABLE, TRUE);
	// 设置要渲染的雾的期望颜色
	m_Device->SetRenderState(D3DRS_FOGCOLOR, color);

	// Start and end dist of fog.[告知Direct3D雾在观察者前面的开始距离以及最远处的重点位置]
	m_Device->SetRenderState(D3DRS_FOGSTART, *(DWORD*)(&start));
	m_Device->SetRenderState(D3DRS_FOGEND, *(DWORD*)(&end));

	// Set based on type.
	if (type == STRANDED_VERTEX_FOG)
		m_Device->SetRenderState(D3DRS_FOGVERTEXMODE, D3DFOG_LINEAR);	// 顶点雾
	else
		m_Device->SetRenderState(D3DRS_FOGTABLEMODE, D3DFOG_LINEAR);	// 像素雾

	// Can only use if hardware supports it.(硬件雾范围)
	if (caps.RasterCaps & D3DPRASTERCAPS_FOGRANGE)
	{
		if (rangeFog)	// 启用基于距离的雾，该值在变换和光照阶段由Direct3D计算得到
			m_Device->SetRenderState(D3DRS_RANGEFOGENABLE, TRUE);
		else
			m_Device->SetRenderState(D3DRS_RANGEFOGENABLE, FALSE);
	}
}

void CD3DRenderer::DisableFog()
{
	if (!m_Device) return;

	// Set fog properties.
	m_Device->SetRenderState(D3DRS_FOGENABLE, FALSE);
}

void CD3DRenderer::SetDetailMapping()
{
	if (!m_Device) return;

	// Set the texture stages for the first texture unit (image).
	m_Device->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
	m_Device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	m_Device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	m_Device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

	// Set the texture stages for the second texture unit (image).
	m_Device->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1);
	m_Device->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_ADDSIGNED);
	m_Device->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	m_Device->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
}

#ifdef DEBUG
#pragma warning (pop)
#endif // DEBUG
