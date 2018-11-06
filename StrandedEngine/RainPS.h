#ifndef __RAIN_PARTICLE_SYSTEM_H__
#define __RAIN_PARTICLE_SYSTEM_H__

/*
 * 粒子系统:
 * 保存操纵粒子，施加不同作用力。包括爆炸、烟、火、火花等
 * 粒子是一个小物体，包含多种属性，如位置、速度(方向)和质量、大小、能级、颜色、
 * 颜色增量(单位时间内颜色的变化量)等，游戏中通常是添加了纹理的小正方形，面向摄像机。
 * 可以使用点状 sprite.
 * 粒子相互之间独立作用，却限制在粒子系统范围内。通常不发生碰撞，会降低帧率.
 * 可使用脚本，方便为游戏创建、修改和添加粒子系统.
 *
 * 雨： 使用点状 sprite、图像为雨滴纹理的系统，粒子在场景中下落，满足条件被销毁(实际是重新初始化回到天空)
 * 粒子数动态变化，需要使用动态顶点缓存 D3DUSAGE_POINTS | D3DUSAGE_POINTS(渲染顶点)
	CRainPS g_rainPS(0, -4, 0, 0, 1, 0);

	bool InitializeObjects()
	{
		// Initialize particle system.
		g_rainPS.Initialize(0, -3, 0, 3000, 2, 3, 2);
		// 可在程序开始渲染是场景中出现一些雨滴
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
		// 确定存储当前场景中全部粒子(仅当前系统释放的粒子)所需要的空间
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
		
		// 启动点状 sprite
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

// 简单粒子
class CParticle
{
public:
	CParticle()
	{
		m_pos[0] = 0; m_pos[1] = 0; m_pos[2] = 0;
		m_vel[0] = 0; m_vel[1] = 0; m_vel[2] = 0;
	}

public:
	// 位置、速度
	float m_pos[3];
	float m_vel[3];
};

// 粒子系统
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
	// 根据效果的宽度、高度和深度得到的系统效果区域
	float m_width;
	float m_height;
	float m_depth;

	// 粒子系统位置，最大粒子数，当前正在使用的粒子总数，时间计数器
	float m_pos[3];
	CParticle* m_particles;
	int m_maxParticles;
	int m_particleCount;
	float m_totalTime;

	// 粒子系统速度和加速度
	float m_velocity[3];
	float m_velDelta[3];
};

#endif
