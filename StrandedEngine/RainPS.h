#ifndef __RAIN_PARTICLE_SYSTEM_H__
#define __RAIN_PARTICLE_SYSTEM_H__

/*
 * ����ϵͳ:
 * ����������ӣ�ʩ�Ӳ�ͬ��������������ը���̡��𡢻𻨵�
 * ������һ��С���壬�����������ԣ���λ�á��ٶ�(����)����������С���ܼ�����ɫ��
 * ��ɫ����(��λʱ������ɫ�ı仯��)�ȣ���Ϸ��ͨ��������������С�����Σ������������
 * ����ʹ�õ�״ sprite.
 * �����໥֮��������ã�ȴ����������ϵͳ��Χ�ڡ�ͨ����������ײ���ή��֡��.
 * ��ʹ�ýű�������Ϊ��Ϸ�������޸ĺ��������ϵͳ.
 *
 * �꣺ ʹ�õ�״ sprite��ͼ��Ϊ��������ϵͳ�������ڳ��������䣬��������������(ʵ�������³�ʼ���ص����)
 * ��������̬�仯����Ҫʹ�ö�̬���㻺�� D3DUSAGE_POINTS | D3DUSAGE_POINTS(��Ⱦ����)
	CRainPS g_rainPS(0, -4, 0, 0, 1, 0);

	bool InitializeObjects()
	{
		// Initialize particle system.
		g_rainPS.Initialize(0, -3, 0, 3000, 2, 3, 2);
		// ���ڳ���ʼ��Ⱦ�ǳ����г���һЩ���
		g_rainPS.Update(100);

		// Create the vertex buffer.
		if(FAILED(g_D3DDevice->CreateVertexBuffer(sizeof(CParticle)*3000, D3DUSAGE_DYNAMIC|D3DUSAGE_WRITEONLY|D3DUSAGE_POINTS, D3DFVF_XYZ, D3DPOOL_DEFAULT, &g_VertexBuffer, NULL)))
			return false;

		// Load the texture image from file.
		if(D3DXCreateTextureFromFile(g_D3DDevice, "sprite.tga", &g_Texture) != D3D_OK)
			return false;

		// Set the image states to get a good quality image.
		g_D3DDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		g_D3DDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

		// Set default rendering states.
		g_D3DDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
		g_D3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

		// Set the projection matrix.
		D3DXMatrixPerspectiveFovLH(&g_projection, 45.0f, WINDOW_WIDTH/WINDOW_HEIGHT, 0.1f, 1000.0f);
		g_D3DDevice->SetTransform(D3DRS_PROJECTION, &g_projection);

		return true;
	}

	void RenderScene()
	{
		// Update the particle system.
		g_rainPS.Update(0.2f);

		// Buffer pointer and size of particles we are copying.
		void *p;
		// ȷ���洢��ǰ������ȫ������(����ǰϵͳ�ͷŵ�����)����Ҫ�Ŀռ�
		int num = sizeof(CParticle) * g_rainPS.m_particleCount;

		// Copy particles in the vertex buffer.
		if(FAILED(g_VertexBuffer->Lock(0, num, (void**)&p, 0)))
			return;
		memcpy(p, g_rainPS.m_particles, num);

		// Clear the backbuffer.
		g_D3DDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);

		// Begin the scene. Start rendering.
		g_D3DDevice->BeginScene();

		// Draw square.
		g_D3DDevice->SetTexture(0, g_Texture);
		
		// ������״ sprite
		g_D3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		g_D3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_NONE);

		g_D3DDevice->SetRenderState(D3DRS_POINTSPRITEENABLE, TRUE);
		g_D3DDevice->SetRenderState(D3DRS_POINTSCALEENABLE, TRUE);
		g_D3DDevice->SetRenderState(D3DRS_POINTSIZE, FtoDW(0.02f));
		g_D3DDevice->SetRenderState(D3DRS_POINTSIZE_MIN, FtoDW(0.02f));
		g_D3DDevice->SetRenderState(D3DRS_POINTSCALE_A, FtoDW(0.0f));
		g_D3DDevice->SetRenderState(D3DRS_POINTSCALE_B, FtoDW(0.0f));
		g_D3DDevice->SetRenderState(D3DRS_POINTSCALE_C, FtoDW(1.0f));

		g_D3DDevice->SetFVF(D3DFVF_XYZ);
		g_D3DDevice->SetStreamSource(0, g_VertexBuffer, 0, sizeof(CParticle));
		g_D3DDevice->DrawPrimitive(D3DPT_POINTLIST, 0, g_rainPS.m_particleCount);

		g_D3DDevice->SetRenderState(D3DRS_POINTSPRITEENABLE, FALSE);
		g_D3DDevice->SetRenderState(D3DRS_POINTSCALEENABLE, FALSE);
		g_D3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);

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

		if(g_Texture != NULL)
		{
			g_Texture->Release();
			g_Texture = NULL;
		}

		g_rainPS.Shutdown();
	}
 */

// Max number of new particles.
#define UPDATE_AMT 1000

// ������
class CParticle
{
public:
	CParticle()
	{
		m_pos[0] = 0; m_pos[1] = 0; m_pos[2] = 0;
		m_vel[0] = 0; m_vel[1] = 0; m_vel[2] = 0;
	}

public:
	// λ�á��ٶ�
	float m_pos[3];
	float m_vel[3];
};

// ����ϵͳ
class CRainPS
{
public:
	CRainPS(float vX, float vY, float vZ, float dX, float dY, float dZ);

	~CRainPS() { Shutdown(); }

	// Init system.
	bool Initialize(float x, float y, float z, int maxP, float w, float h, float d);

	// Update all particles based on time.
	void Update(float scalar);

	// Create a new particle(s).
	void CreateParticle(int amount);

	// Release all resources.
	void Shutdown();
public:
	// ����Ч���Ŀ�ȡ��߶Ⱥ���ȵõ���ϵͳЧ������
	float m_width;
	float m_height;
	float m_depth;

	// ����ϵͳλ�ã��������������ǰ����ʹ�õ�����������ʱ�������
	float m_pos[3];
	CParticle* m_particles;
	int m_maxParticles;
	int m_particleCount;
	float m_totalTime;

	// ����ϵͳ�ٶȺͼ��ٶ�
	float m_velocity[3];
	float m_velDelta[3];
};

#endif
