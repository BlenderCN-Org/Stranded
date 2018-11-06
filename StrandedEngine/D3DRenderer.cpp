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
//��������Ⱦ���󣬴洢�ڻ���ָ��
bool CreateD3DRenderer(CRenderInterface **pObj)
{
	if(!*pObj)
		*pObj = new CD3DRenderer;
	else
		return false;

	return true;
}

/*
 * ����Ƿ�֧����ϣ���Ķ��������. �ɹ����� D3DMULTISAMPLE_TYPE�����򷵻� D3DMULTISAMPLE_NONE
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
			 * @brief: ȷ��Ӳ���Ƿ�֧��ĳ�ֲ�������[ʹ�øú���������������֪����С��������ѭ������ȡӲ��֧�ֲ�������]
			 * @return: ֧�֣�����D3D_OK; ���򣬷���D3D_FAIL
			 * @param:
					����ʹ�õ��Կ�����Ҫ�������豸���͡�����ʹ�õ���Ⱦ��ʽ��ȷ���Ƿ�Ϊȫ��ģʽ�ı�ʶ����
					Ҫ�鿴�Ķ�������͡��洢��MultiSampleType�������������֧�ֵ������ȼ�������ı���
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
 * @brief: ��vType ��ʶ��ת���� Direct3D FVF
 ******************************************************************************************************
 * �̶�������Ⱦ���ڶ��嶨���ʽ��ʱ��Ҫ�ù̶��ĸ�ʽ�����嶥��ṹ�����й̶���˳���
 * ��Щ�ǹ̶���˳�򣬿��Բ�����ĳһ��������˳���ܵߵ�
 * D3DFVF_XYZRHW��D3DFVF_XYZ��D3DFVF_NORMAL���ܹ���
 * �����ʽ��� �ڴ����ʱ����ֱ�Ӵӵ�λ��ʼ��ÿһλ�Ƿ���ֵ��
 *******************************************************************************************************

 * D3DFVF_XYZ - ���꣬Ĭ�ϵ��û������������ǣ�0��0����Ĭ�ϵ�Ϊ�ǹ��յ�, û�о���ת���Ķ��㣬��Ҫ�����任���ٶ�����֮��Ҫ������Ⱦ״̬���ܿ���
 * D3DFVF_XYZRHW - ����+λ��Ԫ��(RHW��wԪ�� 1.0f), ����3D����任������ֱ�ӹ�դ����[��֪Direct3Dʹ�õĶ����Ѿ�����Ļ����ϵ��]
 *		�κ���SetTransform���е�ת����������Ч 
 *		ԭ����ڿͻ��������Ͻ��ˣ�����x����Ϊ����y����Ϊ������z�������Ѿ���Ϊz-buffer��������ȡ�
 *		x��y��������Ļ����ϵ�У�z������z-buffer�е�������ȣ�ȡֵ��Χ��0.0��1.0����۲�������ĵط�Ϊ0.0��
 *		�۲췶Χ����Զ�ɼ��ĵط�Ϊ1.0��
 *		�ᱻ����Ĭ�ϸ���¹���
 * D3DFVF_XYZW - ��������ת���Ͳü��Ķ�������
 * D3DFVF_XYZB1������D3DFVF_XYZB5 - �������ڹ��������Ķ���Ͷ���Թ�����Ȩ����Ϣ
 * D3DFVF_NORMAL - ��ʾ���������а��������������������������������գ�����ǶȲ�ͬ��ȾЧ����ͬ�������Զ����������ʽ֮��һ��Ҫ�ӹ��ղſ�����ʾ
 * D3DFVF_PSIZE - ������Ϣָ�����Ƶ�Ĵ�С
 * D3DFVF_DIFFUSE - ��ɫ���������������Ϣ��
 * D3DFVF_SPECULAR - �������淴�����Ϣ
 * D3DFVF_TEX0������D3DFVF_TEX8 - ����0-8������������Ϣ
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

// ������ֵת�����޷��ų�����
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
* ���ڿ�ȡ��߶ȡ����ھ�����Ƿ�ȫ����ʶ��
* Ҫʹ�õĶ��������(�������ʹ�ö��������ΪSTRANDED_MS_NONE����֧�ֵĲ����������ͻ�ص�Ĭ���޶����)
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
	// ����Direct3D���ڵ���ʾ��Ϣ
	D3DPRESENT_PARAMETERS d3dpp;

	::ZeroMemory(&d3dpp, sizeof(d3dpp));

	// ����D3D����
	m_Direct3D = Direct3DCreate9(D3D_SDK_VERSION);
	if(!m_Direct3D)
		return false;

	// Get the desktop display mode.
	/*
	 * @param
	 * 1�����ڲ�ѯ�������� D3DADAPTER_DEFAULT-���Կ�
	 * 2��������Ϣ����ʾģʽ����
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
		// �洢��ʾ��ˢ���ʣ�����ģʽ��ֵ����Ϊ0��ȫ��ģʽȡ������ʾ��
		d3dpp.FullScreen_RefreshRateInHz = mode.RefreshRate;
		// ���������ʾ�Ľ�����(����ͬʱ��ʾ�������)�ĺ�̨����������
		d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
	}
	else
		d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
	d3dpp.Windowed = !m_fullscreen;
	// ���ڿ���
	d3dpp.BackBufferWidth = w;
	d3dpp.BackBufferHeight = h;
	d3dpp.hDeviceWindow = m_mainHandle;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	// ��Ⱦ��̨�����ʽ
	d3dpp.BackBufferFormat = mode.Format;
	// ��Ⱦ��̨��������
	d3dpp.BackBufferCount = 1;
	d3dpp.EnableAutoDepthStencil = TRUE;	// �Ƿ�����Ȼ����ģ�建��
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16;	// ���ֵ��16λ�����Ʊ�ʾ, λ��Խ�ೡ�����Ƶ�Խ��ȷ���ڴ����ĸ���
	/*
	 * D3DPRESENT_PARAMETERS����������صı��� D3DMULTISAMPLE_TYPE MultiSampleType; DWORD MultiSampleQuality;
	 * MultiSampleType������ D3DMULTISAMPLE_2_SAMPLES��D3DMULTISAMPLE_4_SAMPLES��, �������D3DMULTISAMPLE_NONMASKABLE��
	 * �������� MultiSampleQuality
	 */
	d3dpp.MultiSampleType = GetD3DMultiSampleType(m_Direct3D, ms, D3DDEVTYPE_HAL, mode.Format, m_fullscreen);

	m_screenWidth = w;
	m_screenHeight = h;

	// Create the D3DDevice
	/*
	 * @param
	 * 1��ָ������ʹ�õ��Կ�
	 * 2��ָ��Direct3D�е���Ⱦ��ʽ D3DDVTYPE_HALL-����Ӳ����Ⱦ, D3DEVTYPE_REF-���������Ⱦ, D3DDETYPE_REF-����Ҫ��Ӳ��֧��,
	 * D3DDEVTYPE_SW-����Ҫ������Ⱦ�����Ŀɲ�����
	 * 3�����ھ��
	 * 4��ָ���豸�����з�ʽ�ı�ʶ�����
	 * 5��ָ�򴴽���D3DPRESENT_PARAMETERS�ṹ��ָ��
	 * 6��ָ���´�����Direct3D�豸�����ָ��
	 */
	if(FAILED(m_Direct3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, m_mainHandle, processing, &d3dpp, &m_Device)))
		return false;
	if(m_Device == NULL)
		return false;

	OneTimeInit();
	
	return true;
}

// һ���Գ�ʼ��
void CD3DRenderer::OneTimeInit()
{
	if(!m_Device)
		return;
	
	m_Device->SetRenderState(D3DRS_LIGHTING, FALSE);
	m_Device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);	// �����޳�

	// ������Ȼ���
	//m_Device->SetRenderState(D3DRS_ZENABLE, TRUE);

	// ����Ĭ��ͶӰ����
	CalculateProjMatrix(D3DX_PI/4, 0.1f, 1000);
}

void CD3DRenderer::Shutdown()
{
	ReleaseAllStaticBuffers();
	ReleaseAllXModels();

	for(int s = 0; s < m_numTextures; s++)
	{
		// ɾ���ļ���
		if(m_textureList[s].fileName)
		{
			delete[] m_textureList[s].fileName;
			m_textureList[s].fileName = NULL;
		}

		// �ͷ�LPDIRECT3DTEXTURE9����
		if(m_textureList[s].image)
		{
			m_textureList[s].image->Release();
			m_textureList[s].image = NULL;
		}
	}

	// ��������б�
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

// �����ĺ�������ɫ�ĸ���ֵ
void CD3DRenderer::SetClearCol(float r, float g, float b)
{
	m_Color = D3DCOLOR_COLORVALUE(r, g, b, 1.0f);
}

// ϵͳ�Ƿ������ɫ����Ⱥ� stencil����(����Ⱦ����)
void CD3DRenderer::StartRender(bool bColor, bool bDepth, bool bStencil)
{
	if(!m_Device)
		return;

	unsigned int buffers = 0;
	if(bColor)
		buffers |= D3DCLEAR_TARGET;
	if(bDepth)
		buffers |= D3DCLEAR_ZBUFFER;	// �����Ȼ���
	if(bStencil)
		buffers |= D3DCLEAR_STENCIL;	// ���ģ�建��

	// Clear the backbuffer.
	/*
	 * @param
	 * 1��Ҫ����ľ�����Ŀ(0�������������Ļ)
	 * 2��������Ҫ�������Ļ�����������(NULL����������Ļ)
	 * 3����ʶҪ������ݵı�ʶ��
	 * 4����������ɫ
	 * 5��Ҫ�趨�����ֵ
	 * 6���趨ģ�建��ֵ
	 */
	if(FAILED(m_Device->Clear(0, NULL, buffers, m_Color, 1, 0)))
		return;

	// ����һ���³���
	if(FAILED(m_Device->BeginScene()))
		return;

	m_renderingScene = true;
}

// ���������Ѿ������ĳ���
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

// ֹͣ������Ⱦ
void CD3DRenderer::EndRendering()
{
	if(!m_Device)
		return;

	// ������Ⱦ
	m_Device->EndScene();
	/*
	 * @param
	 * 1��������ʾ��ԭʼ���Σ������ʹ�ý���������ֵ����ΪNULL
	 * 2��ָ��Ҫ��Ⱦ�����վ��ε�ָ��
	 * 3��������ʾ�Ĵ��ڵĴ��ھ��(��Direct3D��ʼ��ʱΪD3DPSENT_PARAMETERS�������õĴ��ھ��)�������ʹ�ý���������ֵΪNULL
	 * 4����Ҫ���µ���С��������
	 */
	m_Device->Present(NULL, NULL, NULL, NULL);

	m_renderingScene = false;
}

void CD3DRenderer::SetMaterial(stMaterial *mat)
{
	if(!mat || !m_Device)
		return;

	// ʹ�ò��ʽṹ��Ϣ����Direct3D���ʶ���
	D3DMATERIAL9 m =
	{
		mat->diffuseR, mat->diffuseG, mat->diffuseB, mat->diffuseA,			// r=0.6f, g=0.6f, b=0.7f
		mat->ambientR, mat->ambientG, mat->ambientB, mat->ambientA,			// r=0.6f, g=0.6f, b=0.7f
		mat->specularR, mat->specularG, mat->specularB, mat->specularA,		// r=0.4f, g=0.4f, b=0.4f
		mat->emissiveR, mat->emissiveG, mat->emissiveB, mat->emissiveA,
		mat->power															// 8.0f
	};

	// ���ò��ʣ����ö���ֱ�ӷ��͸�Direct3D(��Ⱦ����ǰ���ã�������Ӧ�õ�������)
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
	// ��Direct3D���͹�Դ����(��0�Ĺ�Դ����, ����ʹ�õĵ�һյ��, һ�ο���ʹ�����8����Դ[0~7])
	m_Device->SetLight(index, &l);
	// ������Դ
	m_Device->LightEnable(index, TRUE);
}

/*
 * @param: ��Ҫ�رյ�Դ����
 */
void CD3DRenderer::DisableLight(int index)
{
	if(!m_Device)
		return;

	m_Device->LightEnable(index, FALSE);
}

/*
 * ����ƽ��ͶӰ����
 *
 * {-0.3f, -0.3f, 1.0f, D3DCOLOR_XRGB(255,255,0)}
 * [��λ������������]
 * ���
 * SetRenderState(D3DRS_LIGHTING, FALSE);
 * SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
 *
 * �ӳ�(fv)
 * ������ֵ(n)
 * Զ����ֵ(f)
*/
void CD3DRenderer::CalculateProjMatrix(float fov, float n, float f)
{
	if(!m_Device)
		return;

	D3DXMATRIX projection;

	/*
	 * ������������ͶӰ
	 * @param: ��Ҫ������D3DXMATRIX����
	 * @param: Y����(�Ի���ָ��)�Ŀɼ���
	 * @param: �ӽǷ��ȱ�(�ֱ��ʿ�Ⱥ͸߶�֮��)
	 * @param: Զ������ƽ��ֵ
	 */
	D3DXMatrixPerspectiveFovLH(&projection, fov, (float)m_screenWidth/(float)m_screenHeight, n, f);
	// Apply the view (camera).
	m_Device->SetTransform(D3DTS_PROJECTION, &projection);
}

/*
 * ��������ͶӰ����
 *
 * ���
 * SetRenderState(D3DRS_LIGHTING, FALSE);
 * SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
 * {-0.0ff, 150.0f, 0.1ff, D3DCOLOR_XRGB(255,255,0)}
 * ��(0,0)������Ļ���м�
 * ��������ͶӰʱ��ʹ����������
 */
void CD3DRenderer::CalculateOrthoMatrix(float n, float f)
{
	if(!m_Device)
		return;

	D3DXMATRIX ortho;

	/*
	 * ��������ͶӰ
	 * @param: �������������D3DXMATRIX����
	 * @param: ���ڷֱ��ʵĿ�ȡ��߶�
	 * @param: Զ������ƽ��ֵ(0.1f��1000.0f)
	 */
	D3DXMatrixOrthoLH(&ortho, (float)m_screenWidth, (float)m_screenHeight, n, f);
	/*
	 * ��Direct3D�豸��ʹ����������
	 * @param: ����ʹ�õľ������͡�����
	 */
	m_Device->SetTransform(D3DTS_PROJECTION, &ortho);
}

/*
 * �����������/ģ�;���
 * @brief: ���������ƻ�������λ�õĲο���,
 *		��ʼ��ʼ��λ��(0,0)
 * @sample: (0,0,0)->(0,0,-5)
 */
void CD3DRenderer::CalculateWorldMatrix(float x, float y, float z)
{
	if(!m_Device)
		return;

	D3DXMATRIX world;
	
	/*
	 * �ı��������
	 * @brief: ���ƶ����ȴ������󣬴洢����λ��
	 * @param: ������ͬ��D3DXMATRIX ���͵��������ľ���(�������)
	 * @param: Ҫ�ƶ���������X��Y��Zֵ(��ʶ����λ��x,y,z)
	 */
	D3DXMatrixTranslation(&world, x, y, z);
	// ������Ӧ�õ�Direct3D��
	m_Device->SetTransform(D3DTS_WORLD, &world);
}

void CD3DRenderer::RotateWorldMatrix(float radian, E_ROTATION_DIR eRotation)
{
	if(!m_Device)
		return;

	D3DXMATRIX world;

	/*
	 * Χ����������ת
	 * D3DXMATRIX* WINAPI D3DXMatrixRotationAxis(D3DXMATRIX *pOut, CONST D3DXCECTOR3 *pV, FLOAT Angle);
	 * ���պ��㡢���㡢���ֵ��ת
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
 * ������ͼ����(�����)
 * D3D ��������ϵ Y��X��Z��
 * @fun: ����һ�����󲢽���Ӧ�õ���ǰ��ͼ����
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
	 * @param: ��Ҫת��Ϊ��ͼ����ľ���
	 * @param: �������λ��
	 * @param: �۲��λ��
	 * @param: ���ϵķ���
	 */
	D3DXMatrixLookAtLH(&mat, &camePos, &lookAtPos, &upDir);
	// Apply the view (camera).
	m_Device->SetTransform(D3DTS_VIEW, &mat);
}


/*
 * @define: ���þ�̬��������е�һ���������ݣ���������ӵ���̬�����б�
 * @param: ����ʹ�õĶ�������
 * @param: ��Ⱦ�ü���ͼ�ε�ָ��
 * @param: ����ͼ���еĶ�������
 * @param: һ�������������������
 * @param: ����ߴ�
 * @param: ��������
 * @param: ��������
 * @param: staticId ָ���´����Ľ�Ҫ����ľ�̬����ID�ı���ָ��
 */
int CD3DRenderer::CreateStaticBuffer(VertexType vType, PrimType primType, unsigned int totalVerts, unsigned int totalIndices, int stride, void **data, unsigned int *indices, int *staticId)
{
	void* ptr;
	int index = m_numStaticBuffers;

	// ����һ����̬������󣬲�������ӵ���̬�����б���
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
		// ������������
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
	 * ����ͼԪ��ʽ
	 * 1��ʹ��ָ�뽫����ͼ���������鷢�͸�ĳ��Direct3D����
	 * - (��Ч)��Ҫ������ʱ���㻺�� DrawPrimitive();
	 * - HRESULT DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride)
	 * - @param: ͼԪ����[D3DPT_POINTLIST, D3DPT_LINELIST, D3DPT_LINESTRIP, D3DPT_TRIANGLELIST, D3DPT_TRIANGLESTRIP, D3DPT_TRIANGLEFAN]
	 * - @param: ���͸�������ͼԪ����
	 * - @param: ���͸��ú�����ͼԪ��������
	 * - @param: ÿ������Ĵ�С(�ֽ�)
	 * 2���������㻺��
	 * - HRESULT CreateVertexBuffer(UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool, IDirect3DVertexBuffer9** ppVertexBuffer, HANDLE* pSharedHandle);
	 * - @param: ���㻺��������ֽ���
	 * - @param: һ���������㻺��ʹ�÷����ı�ʶ��
	 * - @param: һ������ʹ�ö��㻺��Ķ���ṹ�ı�ʶ�����ʶ����[����λ��->������ɫ��]
	 * - @param: ������Դ����ȷ�ڴ���
	 * - @param: �����Ķ��㻺��
	 * - @param: �������� 0
	 */
	if(FAILED(m_Device->CreateVertexBuffer(totalVerts * stride, D3DUSAGE_WRITEONLY, m_staticBufferList[index].fvf, D3DPOOL_DEFAULT, &m_staticBufferList[index].vbPtr, NULL)))
		return STRANDED_FAIL;

	/*
	 * �������㻺��[����һ�����ݣ���ȡ���㻺��ָ��, �����ݸ��Ƹ��ö��㻺��]
	 * - HRESULT Lock(UINT OffsetToLock, UINT SizeToLock, void** ppbData, DWORD Flags);
	 * - @param: Ϊ�����Ķ��㻺�����ӵ�ƫ����(0 ��������ȫ������)
	 * - @param: Ҫ������������
	 * - @param: ָ�������ڴ�����ָ��
	 * - @param: һ��ָ���������ݷ����ı�ʶ��
	 */
	if(FAILED(m_staticBufferList[index].vbPtr->Lock(0, 0, (void**)&ptr, 0)))
		return STRANDED_FAIL;

	memcpy(ptr, data, totalVerts * stride);
	m_staticBufferList[index].vbPtr->Unlock();

	// ���澲̬����ID
	*staticId = m_numStaticBuffers;
	// ���Ӿ�̬��������
	m_numStaticBuffers++;

	return STRANDED_OK;
}

/*
 * @define: ���뾲̬����ID��صļ���ͼ����Ⱦ����Ļ��
 * @param: ��̬����ID
 */
int CD3DRenderer::RenderStaticBuffer(unsigned int staticId)
{
	// ���ټ�龲̬����ID����Ч
	if(staticId >= m_numStaticBuffers)
		return STRANDED_FAIL;

	// ��龲̬����ID�Ƿ����
	if(m_activeStaticBuffer != staticId)	// ������
	{
		if(m_staticBufferList[staticId].ibPtr != NULL)
			m_Device->SetIndices(m_staticBufferList[staticId].ibPtr);

		/*
		 * @define: �����㻺����Ϊ��Ⱦ��
		 * @param: Ҫ���õ�������(��0���������-1)
		 * @param: ���㻺��
		 * @param: ��ʼ��Ⱦ�����ƫ���ֽ���
		 * @param: ������(������Ⱦ�Ļ����У��ڼ���ͼ�ζ��嵥����ʱ�õ��Ķ���ṹ��С)
		 */
		m_Device->SetStreamSource(0, m_staticBufferList[staticId].vbPtr, 0, m_staticBufferList[staticId].stride);

		// ���ö����ʽ FVF
		m_Device->SetFVF(m_staticBufferList[staticId].fvf);

		m_activeStaticBuffer = staticId;
	}

	// ����[��Ⱦͼ��]
	if(m_staticBufferList[staticId].ibPtr != NULL)
	{
		// [������������ͼ��]
		switch(m_staticBufferList[staticId].primType)
		{
		case POINT_LIST:
			/*
			 * ���Ƶ�ǰ����Ϊ���Ķ��㻺������
			 * @param: Ҫ��Ⱦ��ͼԪ����
			 * @param: ��ʼ��Ⱦ����ʼ��������(��ʼΪ0)
			 * @param: ��Ⱦ��ͼԪ��Ŀ
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
		// [��������������ͼ��]
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
 * @param: ��Ⱦ����Ļ�ϵ� X ģ�͵� ID.
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
	// ѭ�� X ģ�͵Ĳ���
	for (unsigned long i = 0; i < m_xModels[xModelId].numMaterials; i++)
	{
		// ����ģ�Ͳ���
		m_Device->SetMaterial(&m_xModels[xModelId].matList[i]);
		// ����ͼ��
		m_Device->SetTexture(0, m_xModels[xModelId].textureList[i]);
		//HRESULT DrawSubset(DWORD AttribId);
		m_xModels[xModelId].model->DrawSubset(i);
	}
	*/
	//////////////////////////////////////////////////////////////////////////
	// XMesh.h �е�ģ�ͽṹ���� [�����Ǿ�̬����]
	m_xModels[xModelId].Render();

	return STRANDED_OK;
}

/*
 * @brief: �� UMF ģ�ͼ��ص���̬����
 * @param: ģ���ļ��������������(���� UMF �����ɶ��ģ�͹��ɣ�����ָ���Ǵ��Ǹ�ģ�ͼ������񣬼�������������)�����澲̬���� ID ��λ�õ�ַ
 */
int CD3DRenderer::LoadUMFAsStaticBuffer(const char* file, int meshIndex, int *staticID)
{
	void *ptr;
	int index = m_numStaticBuffers;

	stUMFModel *model;
	int numModels = 0;
	int stride = sizeof(stModelVertex);

	// ���� UMF ģ��
	if (!LoadUMF(file, &model, &numModels))
		return STRANDED_FAIL;

	if (!numModels || !model)
		return STRANDED_FAIL;

	if (meshIndex < 0 || meshIndex >= numModels)
		return STRANDED_FAIL;

	// ���Ӿ�̬����������δ�����ӵ������ڳ��ռ�
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

	// ���ݼ��ص�������Ϣ���û���ĳ�Ա���� [������Ŀ��ͼԪ����]
	m_staticBufferList[index].numVerts = model[meshIndex].totalVertices;
	m_staticBufferList[index].numIndices = model[meshIndex].totalFaces;
	m_staticBufferList[index].primType = TRIANGLE_LIST;
	m_staticBufferList[index].stride = stride;
	m_staticBufferList[index].fvf = CreateD3DFVF(MV_FVF);

	//////////////////////////////////////////////////////////////////////////
	// ��ȡ������Ϣ�����临�Ƹ� Direct3D �����Ͷ��㻺��

	// �Ƿ�����۴���
	if (model[meshIndex].totalFaces > 0)
	{
		// ���Ƹ���ʱ�����ڴ�����������ʱ���õ�������
		int size = model[meshIndex].totalFaces * 3;
		unsigned int *idx = new unsigned int[size];

		// �� UMF ģ���У�Ϊÿ����۱���һ�����������ڴ�������������ֻ�����޷�����������
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
		// û����ۣ���������������Ϊ NULL
		m_staticBufferList[index].ibPtr = NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	
	// ���� stModelVertex ��ʱģ���������� [���� GUI ����ṹ]
	int totalVerts = model[meshIndex].totalVertices;
	stModelVertex *data = new stModelVertex[totalVerts];

	// ѭ��ÿ��ģ�Ͷ��㣬�����ƶ���λ�á����㷨��(�����Ҫ�Ļ�)��������ɫ�Լ�������������
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

	// ���� Direct3D ���㻺��
	if (FAILED(m_Device->CreateVertexBuffer(totalVerts*stride, D3DUSAGE_WRITEONLY, m_staticBufferList[index].fvf, D3DPOOL_DEFAULT, &m_staticBufferList[index].vbPtr, NULL)))
		return STRANDED_FAIL;

	// ������ʱ���������
	if (FAILED(m_staticBufferList[index].vbPtr->Lock(0, 0, (void**)&ptr, 0)))
		return STRANDED_FAIL;
	memcpy(ptr, data, totalVerts*stride);
	m_staticBufferList[index].vbPtr->Unlock();

	// ɾ�����е���ʱ����
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

	// ���澲̬ ID
	*staticID = m_numStaticBuffers;
	m_numStaticBuffers++;

	return STRANDED_OK;
}

/*
 * @brief: ����LoadUMFAsStaticBuffer�������Ӿ�̬�����̬�����б�����, ȡ�����еľ�̬����ģ�ͣ��������ݴ���.
 *			�滻����ǰ������ ReleaseStaticBuffer() ��������Ѿ����ڵ�����.
 *			ReleaseStaticBuffer() ������ReleaseXModel() ���������ͷŵ�����������ݣ���û�н������������ɾ����
 *			��������й©��������û���ͷž�����ǰ��ʹ���µĶ�̬�ڴ��滻����.
 * @param: staticID - ָ�����Ǹ���̬��������������
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
	// ���� staticID ָ�������ݣ�������ԭ LoadUMFAsStaticBuffer ��������ĩ�˼�������

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

	// ���Ӿ�̬����������δ�����ӵ������ڳ��ռ�
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

	// ���ݼ��ص�������Ϣ���û���ĳ�Ա���� [������Ŀ��ͼԪ����]
	m_staticBufferList[index].numVerts = model->m_numFaces * 3;
	m_staticBufferList[index].numIndices = 0;
	m_staticBufferList[index].primType = TRIANGLE_LIST;
	m_staticBufferList[index].stride = stride;
	m_staticBufferList[index].fvf = CreateD3DFVF(MV_FVF);

	m_staticBufferList[index].ibPtr = NULL;

	//////////////////////////////////////////////////////////////////////////
	// ��ȡ������Ϣ�����临�Ƹ� Direct3D ���㻺��

	// ���� stModelVertex ��ʱģ���������� [���� GUI ����ṹ]
	int totalVerts = model->m_numFaces * 3;
	stModelVertex *data = new stModelVertex[totalVerts];

	// ѭ��ÿ��ģ�Ͷ��㣬�����ƶ���λ�á����㷨��(�����Ҫ�Ļ�)��������ɫ�Լ�������������
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

	// ���� Direct3D ���㻺��
	if (FAILED(m_Device->CreateVertexBuffer(totalVerts*stride, D3DUSAGE_WRITEONLY, m_staticBufferList[index].fvf, D3DPOOL_DEFAULT, &m_staticBufferList[index].vbPtr, NULL)))
		return STRANDED_FAIL;

	// ������ʱ���������
	if (FAILED(m_staticBufferList[index].vbPtr->Lock(0, 0, (void**)&ptr, 0)))
		return STRANDED_FAIL;
	memcpy(ptr, data, totalVerts*stride);
	m_staticBufferList[index].vbPtr->Unlock();

	// ɾ�����е���ʱ����
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

	// ���澲̬ ID
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
	// ���� staticID ָ�������ݣ�������ԭ LoadOBJAsStaticBuffer ��������ĩ�˼�������

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
 * @brief: �� X ģ�ͼ��ص���Ⱦ���е� X ģ������
 * @param: ���ص��ļ������洢 X ģ������ ID �ĵ�ַ
 */
int CD3DRenderer::LoadXModel(const char* file, int *xModelId)
{
	if (!file) return STRANDED_FAIL;

	// ���� X ģ������
	if (!m_xModels)
	{
		m_xModels = new CXModel[1];
		if (!m_xModels) return STRANDED_FAIL;

		// �ṩ Direct3D �豸
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
	// ��ģ�ͼ��ص��ڴ�
	//LPD3DXBUFFER matBuffer = NULL;
	//unsigned long numMats = 0;

	/*
	 * �� X ģ�ͼ��ص� LPD3DXMESH �����У����� Direct3D ��ʹ�ø�ģ��
	 * D3D_OK, ���سɹ�; ������ع��̳��ִ���ģ��û�б����ص��ڴ���
	 *
		HRESULT D3DXLoadMeshFromX(
			LPCTSTR pFilename,					// X �ļ���
			DWORD Options,						// ���������ѡ���ʶ��
			LPDIRECT3DDEVICE9 pD3DDevice,		// Direct3D �豸����
			LPD3DXBUFFER* ppAdjacency,			// �洢�ٽ�����(����������)�� LPD3DXBUFFER
			LPD3DXBUFFER* ppMaterials,			// �洢�������ļ��еĲ��ʵĻ���
			LPD3DXBUFFER* ppEffectInstances,	// �洢���ļ���ʹ�õ�Ч��(��Ӱ��)ʵ���Ļ���
			DWORD* pNumMaterials,				// ָ�����������ָ��
			LPD3DXMESH* ppMesh					// �ú������ô�������������ַ
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
	// XMesh.h �е�ģ�ͽṹ���� [�����Ǿ�̬������������Ǿ�̬����Ҳ�����Ƕ�̬����]
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

	// �ͷ������� X ģ����ص�����
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
	// XMesh.h �е�ģ�ͽṹ���� [�����Ǿ�̬��������֧�־�̬���񣬶���·��]
	if (!m_xModels[xModelId].LoadXFile(file))
		return STRANDED_FAIL;

	return STRANDED_OK;
}

/*
 * @brief: �ͷ� X ģ������������ڴ�
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
 * @param: Ҫ�ͷŵĵ���ģ��
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
	// ѭ����飬�ͷ����еľ�̬�������ݣ�����̬�����б�
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
 * @brief: ɾ����id���������ж�̬�ڴ�
 * @param: staticId - ��Ҫ�ͷŵľ�̬���� ID
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
 * @brief: ͸���� - ���û��ģʽ��Ⱦ����
 *		ҪΪ����ͼ����ͼ�������һ��alphaͨ��������ʹ��D3DCOLOR_ARGB�������
 *		D3DCOLOR_XRGB��һ���ڶ���ṹ��ʹ��alpha
 * @param: 1 �������õ���Ⱦ״̬
 * @param: 2 Դ��ϲ���
 * @param: 3 Ŀ�Ļ�ϲ���
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
		// ���� Direct3D Alpha ���
		m_Device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);

		// ΪҪ��ϵ�Դ��������һ������
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

		// Ϊ������Ⱦ�õ�Ŀ�Ļ�������һ������
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
 * @param: ���ļ��м��ص�ͼ���ļ���
 * @param: �洢�´�����������������ID
*/
int CD3DRenderer::AddTexture2D( char *file, int *texId )
{
	if(!file || !m_Device)
		return STRANDED_FAIL;

	int len = strlen(file);
	if(!len)
		return STRANDED_FAIL;

	// ��������Ƿ��Ѵ���
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

	// ���ļ������Ƹ��������
	m_textureList[index].fileName = new char[len+1];
	memcpy(m_textureList[index].fileName, file, len);
	m_textureList[index].fileName[len] = '\0';

	D3DCOLOR colorkey = 0xff000000;
	D3DXIMAGE_INFO info;

	/*
	 * @brief: ֪ͨDirect3D��ָ���ļ���������������ص��ڴ�
	 * HRESULT WINAPI D3DXCreateTextureFromFile(LPDIRECT3DDEVICE9 pDevice, LPCTSTR, pSrcFile, LPDIRECT3DTEXTURE9 *ppTexture);
	 *
	 * @brief: ���ļ���������ͼ������mipmaps���ڶ����б�������ͼ��Ŀ�Ⱥ͸߶ȣ�����������ӵ������б���. Ȼ�󱣴�����ID����������������.
	 * HRESULT WINAPI D3DXCreateTextureFromFileEx(LPDIRECT3DDEVICE9 pDevice, LPCTSTR pSrcFile, UINT Width, UINT Height, UINT MipLevels, DWORD Usage, D3DFORMAT Format,
	 *	D3DPOOL Pool, DWORD Filter, DWORD MipFilter, D3DCOLOR ColorKey, D3DXIMAGE_INFO *pSrcInfo, PALETTEENTRY *pPalette, LPDIRECT3DTEXTURE9 *ppTexture);
	 * @param: pSrcFile - ����ͼ��λ�õ��ַ���
	 * @param: Filter - ����ͼ�������������� Direct3D ���ͼ�����ݵķ���(��ָ������ͼ��Ϊԭʼͼ���Ⱥ͸߶�һ��D3DX_FILTER_BOX[��2*2���ӽ�ͼ������ƽ������]�����Ǳ�ԭʼͼ��ҪС����)
	 * @param: MipFilter - ��������ͼ��� mipmap ��ͬFilter. �˾�Ŀ��Ϊ�˿���Ҫ���ص�ͼ��Ӧ�ó����е�����ͼ������(�ڲ�����ȫ��ͼ������������£�ʹ�õ��������˲�����ȡӦ������ͼ������)
	 * @param: ColorKey - ָ��ͼ��������͸���򲻿ɼ��ǵ���ɫֵ���ɽ��뷢�͸��ò�������ɫ�����ÿ��������Ϊ͸�������ҿ����ô���ɫ���(����Щ��һ�������ݲ��ɼ���ͼ����˵���ť�ͽ�ɫ�Ķ�άͼ������)
	 * @param: pSrcInfo - ����ԭʼͼ��������Ϣ�ṹ [ԭʼ��ȣ��߶ȣ���ʽ����ȣ�Mip������Դ���ͣ��ļ���ʽ����]
	 * @param: pPalette - �����øú������ص����о�����ɫ���������ͼ��ĵ�ɫ��.(һ��ͼ����8λ)
	 */
	if(D3DXCreateTextureFromFileEx(m_Device, file, 0, 0, 0, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, colorkey, &info, NULL, &m_textureList[index].image) != D3D_OK)
		return false;

	//�ڶ����б�������ͼ��Ŀ�Ⱥ͸߶�
	m_textureList[index].width = info.Width;
	m_textureList[index].height = info.Height;

	//��������ID
	*texId = m_numTextures;
	//������������
	m_numTextures ++;

	return STRANDED_OK;
}

/*
 * @brief: ����min��mag��mip�˲���һ��ʹ�ã�Ҳ���ڲ������׶�ʹ�õ㡢�ߺ͸��������˲�
 *
 * @param: 1������λ��������׶�
 * @param: 2���˲���ģʽ
 * @param: 3��Ҫʹ�õ��˲���ֵ
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
		v = D3DTEXF_POINT;			// ��������ģʽ
	if(val == LINEAR_TYPE)
		v = D3DTEXF_LINEAR;			// �����˲��������˲�
	if(val == ANISOTROPIC_TYPE)
		v = D3DTEXF_ANISOTROPIC;	// ����ͬ���˲�

	m_Device->SetSamplerState(index, fil, v);
}

// ���ö�������ͼ
void CD3DRenderer::SetMultiTexture()
{
	if(!m_Device)
		return;

	/*
	 * HRESULT SetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value);
	 * @brief: Ϊ�������ö�����״̬
	 * @param: Ҫ�������������
	 * @param: �����õ�����׶�
	 * @param: ���ý׶εı�ʶ��
	 */
	// Set the texture stages for the first texture unit.
	// Direct3Dʶ�����ڴ����һ������
	m_Device->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
	// ֪ͨ Direct3D ��������һ������(����2)���Ը�����
	m_Device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	// ���ò���1���� D3DTA_TEXTURE �������͸�����1
	m_Device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	// ���ò���2���� D3DTA_DIFFUSE ��������ɫ���͸�����2
	m_Device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

	// Set the texture stages for the second texture unit.
	m_Device->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1);
	m_Device->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE);
	m_Device->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	// ���ò���2���õڶ������������һ��������[�õ�һ������Ͷ�����ɫ��ϵڶ�������]
	m_Device->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
}

/*
 * @brief:
 *		͹����ͼ
 * ��ȡ����ʹ�ø�������ڲ���Ϣȥ������棬�ñ�����ʾ��������ʵ��ӵ�еĸ���ϸ��.
 * �����伸��ͼ���ǣ�ͨ������������ͼ���еķ���ֵ�����Ǳ���ķ���ֵ��ɵ�.
 * ͨ���ı�ͼ���еķ���ֵ�����أ��ɵõ���������ϸ��.
 * ����ӳ���͹��ӳ��֮��Ĳ����������������Ӱ������棬��͹����ͼ���ڵƹ��������.
 * ͹����ͼʹ��"������ͼ". ͼ����ÿ�����ص� R��G �� B �ɷ�ʵ���ϴ����˷��ߵ� X��Y �� Z ��.
 *
 * ͹��ӳ�似��ʹ�õ�ͼ�񴴽��ڻҶ�(�Ӻڵ���)2D����ͼ��[�߶�ͼ]. ����ӳ�似��ʹ�õ�ͼ�񴴽�
 * �ڸ߷ֱ���ģ��(��ɫ�������)�е�ϸ�ڣ������䱣�浽�����У�������͹��ӳ����ʹ��ʹ���ͷֱ���
 * ģ�Ϳ���ȥ�����ǻ��������ڸ߷ֱ���ģ��. ����ӳ����һ��͹��ӳ�䣬������ʹ�ͷֱ���ģ�Ϳ���ȥ
 * ���Ǹ߷ֱ���ģ�ͣ���͹��ӳ����ȡ���е�2Dͼ�񣬲����д�������.
 *
 * ��͹��ӳ�䣬��ʹ��ͼ����ȡ���ݣ����������ڹ��գ�������������Ⱦ����.
 * �����ع��������ȵ�������������ߣ���Ϊͼ���е�ÿ�����ض����Լ��ķ��ߣ�����ʹ�õ����ع����������.
 * �ı���Щ����ֵ���Եõ�С����ϸ��ϸ�ڣ�������������ĸй�Ч��.
 *
 * HRESULT WINAPI D3DXComputeNormalMap(LPDIRECT3DTEXTURE9 pTexture, LPDIRECT3DTEXTURE9 pSrcTexture, const PALETTEENTRY *pSrcPalette, DWORD Flags, DWORD Channel, FLOAT Amplitude);
 * @brief: �� Direct3D ֧�ֵ�����ͼ���ʽת����͹����ͼ
 * @extra: D3DXSaveTextureToFile (�ɽ�͹����ͼ���浽�ļ�)
 * @param: pTexture - ���淨����ͼ��Direct3D �������
 * @param: pSrcTexture - ��Ҫת���ɷ�����ͼ��ԭʼͼ��
 * @param: pSrcPalette - ԭʼ��Դ����ĵ�ɫ��
 * @param: Flags - D3DX_NORMAL_MIRROR_U, D3DX_NORMAL_MIRROR_V, D3DX_NORMAL_MIRROR, D3DX_NORMAL_INVERTSIGN, D3DX_NORMAL_CUMPUTE_OCCLUSION
 * @param: Channel - D3DX_CHANNEL_RED: ���㷢����ͼʱʹ�ú�ɫͨ��
 *		D3DX_CHANNEL_GREEN: ���㷢����ͼʱʹ����ɫͨ��
 *		D3DX_CHANNEL_BLUE: ���㷨����ͼʱʹ����ɫͨ��
 *		D3DX_CHANNEL_ALPHA: ���㷨����ͼ��ʹ�� alpha ͨ��
 *		D3DX_CHANNEL_LUMINANCE: ���㷢����ͼʱʹ�ú�ɫ����ɫ����ɫͨ��������ֵ
 * @param: Amplitude - Normal Map ��ǿ��ֵ(����ֵ�����ֵ)
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
	 * ����������ͼ��
	 * @param: ����Ŀ�ȡ��߶�
	 * @param: �����е� Mipmap �ĸ���(MipLevels)
	 * @param: Usage - Ҫ��Ⱦ����[D3DUSAGE_RENDERTARGET], ��̬����[D3DUSAGE - DYNAMIC]
	 * @param: Format - ���ļ�����ͼ�����Ϊ0����Direct3D�����ļ�����ѡ����ȷͼ���ʽ
	 * @param: Pool - �������פ���ڴ����.D3DPOOL_DEFAULT[�ڴ��������Ƶ�ڴ棬Ĭ��], D3DPOOL_SYSTEMMEM[��������󱣴��ڼ����RAM�ڴ棬
	 *	���Direct3D�豸��ʧ���������´�����Դ.]
	 * @param: ���ڴ������������ LPDIRECT3DTEXTIRE9
	 * HRESULT WINAPI D3DXCreateTexture(LPDIRECT3DDEVICE9 pDevice, UINT Width, UINT Height, UINT MipLevels, DWORD Usage,
	 *		D3DFORMAT Format, D3DPOOL Pool, LPDIRECT3DTEXTURE9 *ppPtexture);
	 *
	 * @breif: Set texture to offscreen surface.
	 * ����Ļ�����[����������л�ȡ������󣬼�Ҫ��Ⱦ�ı��棬������󽫳����洢Ϊһ��ͼ�񣬲�������ʾ����Ļ��]
	 * @param: Level �Ǵ��������ǵ�������Դ����
	 * @param: ppSurfaceLevel �����ڴ����� LPDIRECT3DSURFACE9
	 * HRESULT GetSurfaceLevel(UINT Level, IDirect3DSurface9 **ppSurfaceLevel);
	 *
	 * @brief: ������ȾĿ��
	 * @param: ��ȾĿ������
	 * @param: Ҫ���Ƶ� LPDIRECTSURFACE9 ����
	 * HRESULT SetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9 *pRenderTarget);
	 *
	 * Ļ����Ⱦ��
	 * @prepare:
	 * 1������һ��������Ⱦ����� LPDIRECT3DTEXTURE9 ���� [D3DXCreateTexture]
	 * 2������Ļ����� [GetSurfaceLevel]
	 * @render:
	 * 3������һ�ݺ�̨�������ĸ��� [GetBackBuffer]
	 * 4��ת������ȾĿ�� [SetRenderTarget]
	 * 5��������Ļ����棬����������Ⱦ
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
	// ͨ���ǻҶ�ͼ
	if (D3DXComputeNormalMap(pNormalMap, pTexture, 0, D3DX_NORMALMAP_MIRROR, D3DX_CHANNEL_GREEN, 10) != D3D_OK)
		return false;

	return true;
}

/*
 * @brief: Ϊ����ʹ�õ���Ⱦϵͳ�������
 * @param: 1��Ҫʹ�õ�����λ
 * @param: 2��Ҫʹ�õ�����������������ID
*/
void CD3DRenderer::ApplyTexture( int index, int texId )
{
	if(!m_Device)
		return;

	// ������в�������Ч����SetTexture��������һ��NULL������޶��ڲ������׶ε���������
	// ���򽫳������������İ�
	if(index < 0 || texId < 0)
		m_Device->SetTexture(0, NULL);
	else
		/*
		 * HRESULT SetTexture(DWORD Sampler, IDirect3DBaseTexture9* pTexture);
		 * @param: Sampler - ����Ҫʩ�ӵĲ������׶�[0~1].֧��������������ȡ����Ӳ����Direct3D��GetDeviceCaps�������D3DCAP9����:�洢�˲������׶�����(MaxSimulataneousTexture, MaxTextureBlendStages)
		 */
		m_Device->SetTexture(0, m_textureList[texId].image);
}

/*
 * @brief: ����Ⱦϵͳ������Ļ��ͼ
 * @param: 1����Ļ��ͼҪ������ļ���
 * @extra:  s������
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
	 * ��ȡ��ǰ����ʾģʽ[��ȡ��Ⱦ����/��ʾ�Ŀ�Ⱥ͸߶ȵ���Ϣ]
	 * ����: ������������Ҫ����Ϸ�Ǹ�����ʾģʽ
	 */
	m_Device->GetDisplayMode(0, &disp);
	/*
	 * ����һ��Ļ�����
	 * ����: ����Ŀ�ȡ��߶�...
	 */
	m_Device->CreateOffscreenPlainSurface(disp.Width, disp.Height,
		D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &surface, NULL);

	/*
	 * ����Ⱦ��ĳ������Ƶ������õı��������
	 * ����: ��������������̨������������̨��������(DirectX 9.0 ��֧�� D3DBACKBUFFER_TYPE_MODE)����������Ⱦ�ĳ����ı������
	 */
	m_Device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &surface);
	/*
	 * �����汣�浽֧�ֵ��ļ���ʽ��
	 * ����: ����ͼ��ʱ��Ҫʹ�õ��ļ���
	 * ������ļ�����[D3DXIFF_BMP, D3DXIFF_JPG, D3DXIFF_TGA, D3DXIFF_PNG, D3DXIFF_DDS, D3DXIFF_PPM, D3DXIFF_DIB, D3DXIFF_HDR, D3DXIFF_PFM]
	 * ������Ⱦ���ݵı������
	 * �����ļ���ʹ�õĵ�ɫ��(��NULL)
	 * ����Ҫ����ı��������RECT�ṹ(��ΪNULL����������ͼ��)
	 */
	D3DXSaveSurfaceToFile(file, D3DXIFF_JPG, surface, NULL, NULL);

	if(surface != NULL)
	{
		surface->Release();
		surface = NULL;
	}
}

/*
 * @brief: sprite(��ͼ��) 2Dͼ�񼯺ϣ�ʹ�� alpha ͨ�����ֽ�ͼ����ϣ�����ɼ�������
 *		2D �������Ļ, 3D ���������
 *
 * ��״ sprite�� ��Ⱦ DrawPrimitive(D3DPT_POINTLIST)��������Ⱦǰ������
 * @param: 1��sprite�ĳߴ�
 * @param: 2��Ҫ���õ���С�ߴ�
 * @param: 3��A��B��C�Ŀ̶�ֵ
 */
void CD3DRenderer::EnablePointSprites( float size, float min, float a, float b, float c )
{
	if(!m_Device)
		return;

	// ���õ�״ sprite
	m_Device->SetRenderState(D3DRS_POINTSPRITEENABLE, TRUE);
	// ���õ�״ sprite �ı��� (��ɫ���Ʊ���ֵ�Լ������������� sprite �ĳߴ�)
	m_Device->SetRenderState(D3DRS_POINTSCALEENABLE, TRUE);
	// ���� sprite �ĳߴ�
	m_Device->SetRenderState(D3DRS_POINTSIZE, FtoW(size));
	// ���õ�״ sprite ����С�ߴ�
	m_Device->SetRenderState(D3DRS_POINTSIZE_MIN, FtoW(min));
	// ���õ�״ sprite ����(Ĭ��ֵΪ1)�����ݾ�����ĵ�״ sprite ����״
	m_Device->SetRenderState(D3DRS_POINTSCALE_A, FtoW(a));
	m_Device->SetRenderState(D3DRS_POINTSCALE_B, FtoW(b));
	m_Device->SetRenderState(D3DRS_POINTSCALE_C, FtoW(c));
}

void CD3DRenderer::DisablePointSprites()
{
	// ���õ�״ sprite
	m_Device->SetRenderState(D3DRS_POINTSPRITEENABLE, FALSE);
	// ���õ�״ sprite �ı���
	m_Device->SetRenderState(D3DRS_POINTSCALEENABLE, FALSE);
}

/*
 * @brief: ����һ�� D3D �������
 * @param:
 *		������������(Times New Roman)
 *		��ӡ�����Ȩ��(����)
 *		�����Ƿ�б��
 *		��������С(�ַ��߶�)
 *		IDָ��(���������������)
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
	 * @brief: ���� LPD3DXFONT ����
	 * @param:
			Height: �����ַ��ĸ߶�(ͨ������Ĵ�С)
			Width: �����ַ��Ŀ��(���帽�ӵĿ�ȣ�ͨ��ȡֵ0)
			Weight: ��ӡ�����Ȩ��(��: ����)
			MipLevels: �ı��� Mipmap ����(�ı�����������ͼ�������ɴ���Ļ��������ʾ����Ϊ1)
			Italic: �����Ƿ�Ϊб��
			Charset: �õ����ַ���
			OutputPrecision: ָ�� Windows ������������ʵ������ƥ��ķ���[���ȷ������ʹ����TrueType���壬��ΪOUT_TT_ONLY_PRECIS]
			Quality: �����ı����ԣ�ֻӰ�쵽raster(��դ)���壬����Ӱ��TrueType����
			PitchAndFamily: �����ı�ʹ�õ� pitch(б��)�� family ����
			pFaceName: Ҫ�õ�������������(����: Arial��Times New Roman)
			ppFont: Ҫ���� LPD3DFONT ����
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
	 * @brief: ��Ⱦ�ı�����LPD3DXFONT->DrawText()
	 * @param:
	 *		pSprite: ָ������ַ����� LPD3DXSPRITE �����ָ�룬�����Ҫ Direct3D �ڲ�����ÿ����ɫ��Сͼ�Σ���ֵ������Ϊ NULL.
	 *		pString: ��Ļ��Ҫ��ʾ���ı�
	 *		Count: �ַ������ַ�����Ŀ
	 *		pRect: RECT���󣬶���Ҫ��ʾ����Ļ�ϵĵ�һ���ַ�����ʼλ��
	 *		Format: ָ���ַ�������ʾ��ʽ.(XOR)
				DT_BOTTOM - ���ı������������ı���RECT����ĵײ�
				DT_CALCRECT - ����RECT�ĸ߶ȺͿ��. ʹ��pRect����ָ�������������Ҫ����������
				DT_CENTER - ���ı�ˮƽ�ص��������ε��м�
				DT_EXPANDTABS - ��չ�ַ����е� tab �ַ���Ĭ������� tab ��8���ַ�
				DT_LEFT - ���ı����������ε����
				DT_NOCLIP - �޼��еػ�֮�ı���������Ⱦ�ٶȸ���
				DT_RIGHT - ���ı����������ε��Ҳ�
				DT_RTLREADING - ���մ��������˳����Ⱦ�ı�. ͨ���� Hebrew �� Arabic ������ʹ�ø�ֵ
				DT_SINGLELINE - ֻ��һ������ʾ�ı�. �������еĻ����ַ�
				DT_TOP - ���ı���ʾ�ھ��εĶ���
				DT_VCENTER - ���ı���ֱ�ص��������ε��м�. ֻʹ�õ�����һ��
				DT_WORDBREAK - ����ͨ������õľ���ʱ����������
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
 * ����FPS:
 *		����ÿ֡����������1��ÿ֡ĩ��ȡ��ǰʱ�䲢ȷ�ϴ���һ�β鿴ʱ�����Ƿ��Ѿ�����
 *	һ��ࡣ�������һ�룬�������ֵ����FPS.
 */
void CD3DRenderer::GetFPS()
{
	// ��̬����
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

	// ���� GUI Ԫ�ص�����
	if (!AddTexture2D(up, &upID)) return false;
	if (!AddTexture2D(over, &overID)) return false;
	if (!AddTexture2D(down, &downID)) return false;

	unsigned long col = D3DCOLOR_XRGB(255, 255, 255);

	int w = m_textureList[upID].width;
	int h = m_textureList[upID].height;

	// ��������Ļ����Ⱦ��Ԫ��Ҫ�õļ�����״
	stGUIVertex obj[] =
	{
		{(float)(w+x), (float)(0+y), 0, 1, col, 1, 0},	// x=w,y=0 ���Ͻ�
		{(float)(w+x), (float)(h+y), 0, 1, col, 1, 1},	// x=w,y=h ���½�
		{(float)(0+x), (float)(0+y), 0, 1, col, 0, 0},	// x=0,y=0 ���Ͻ�
		{(float)(0+x), (float)(h+y), 0, 1, col, 0, 1},	// x=0,y=h ���½�
	};

	if (!CreateStaticBuffer(GUI_FVF, TRIANGLE_STRIP, 4, 0, sizeof(stGUIVertex), (void**)&obj, NULL, &staticID))
		return false;

	// ��������Ϣ���͸��Լ������� GUI ����
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
 *		����㡢�յ�λ��, �����ɫ, ȷ���Ƿ�����ʹ�û��ڷ�Χ����ı�ʶ��
 */
void CD3DRenderer::EnableFog(float start, float end, STRANDED_FOG_TYPE type, unsigned long color, bool rangeFog)
{
	if (!m_Device) return;

	// ��ȡ�豸����
	D3DCAPS9 caps;
	m_Device->GetDeviceCaps(&caps);

	// Set fog properties.
	// ��API��������Ч��
	m_Device->SetRenderState(D3DRS_FOGENABLE, TRUE);
	// ����Ҫ��Ⱦ�����������ɫ
	m_Device->SetRenderState(D3DRS_FOGCOLOR, color);

	// Start and end dist of fog.[��֪Direct3D���ڹ۲���ǰ��Ŀ�ʼ�����Լ���Զ�����ص�λ��]
	m_Device->SetRenderState(D3DRS_FOGSTART, *(DWORD*)(&start));
	m_Device->SetRenderState(D3DRS_FOGEND, *(DWORD*)(&end));

	// Set based on type.
	if (type == STRANDED_VERTEX_FOG)
		m_Device->SetRenderState(D3DRS_FOGVERTEXMODE, D3DFOG_LINEAR);	// ������
	else
		m_Device->SetRenderState(D3DRS_FOGTABLEMODE, D3DFOG_LINEAR);	// ������

	// Can only use if hardware supports it.(Ӳ����Χ)
	if (caps.RasterCaps & D3DPRASTERCAPS_FOGRANGE)
	{
		if (rangeFog)	// ���û��ھ��������ֵ�ڱ任�͹��ս׶���Direct3D����õ�
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
