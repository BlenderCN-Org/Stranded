#include <math.h>
#include <stdlib.h>
#include "RainPS.h"

#define GET_RANDOM (((float)rand() - (float)rand()) / RAND_MAX)

CRainPS::CRainPS(float vX, float vY, float vZ, float dX, float dY, float dZ)
{
	m_width = 0; m_height = 0; m_depth = 0;
	m_particles = nullptr;
	m_maxParticles = 0;
	m_particleCount = 0;
	m_totalTime = 0;

	m_pos[0] = m_pos[1] = m_pos[2] = 0;

	m_velocity[0] = vX;
	m_velocity[1] = vY;
	m_velocity[2] = vZ;

	m_velDelta[0] = dX;
	m_velDelta[1] = dY;
	m_velDelta[2] = dZ;
}

/*
 * @brief: 设置指定的粒子系统信息，分配粒子链表
 * @param:
		系统的X、Y、Z位置，粒子最大数目，粒子系统效果区域宽、高和深度
 */
bool CRainPS::Initialize(float x, float y, float z, int maxP, float w, float h, float d)
{
	// Release all previous data.
	Shutdown();

	m_width = w;
	m_height = h;
	m_depth = d;
	m_particles = nullptr;
	m_maxParticles = maxP;
	m_particleCount = 0;
	m_totalTime = 0;
	m_pos[0] = x;
	m_pos[1] = y;
	m_pos[2] = z;

	// Error checking.
	if (m_maxParticles <= 0)
		m_maxParticles = 1;

	// Allocate space for the particles.
	m_particles = new CParticle[m_maxParticles];
	if (!m_particles)
		return false;

	return true;
}

/*
 * @brief: 更新粒子系统粒子
 * @param: 时间值
 */
void CRainPS::Update(float scalar)
{
	int numParticles = 0;

	// Loop through and update.
	for (int i = 0; i < m_particleCount;)
	{
		m_particles[i].m_pos[0] += m_particles[i].m_vel[0] * scalar;
		m_particles[i].m_pos[1] += m_particles[i].m_vel[1] * scalar;
		m_particles[i].m_pos[2] += m_particles[i].m_vel[2] * scalar;

		// If particle goes past min height, destroy.
		if (m_particles[i].m_pos[1] <= m_pos[1])
		{
			m_particleCount--;
			m_particles[i] = m_particles[m_particleCount];
		}
		else
			i++;
	}

	// Calculate how many new particles we should create.
	m_totalTime += scalar;
	numParticles = (int)(UPDATE_AMT * m_totalTime);
	if (m_totalTime > 1)
		m_totalTime = 0;

	// Create them.
	CreateParticle(numParticles);
}

/*
 * @param: 准备创建的粒子总数
 */
void CRainPS::CreateParticle(int amount)
{
	// Simply loop through and create the amount of particles.
	for (int i = 0; i < amount; ++i)
	{
		// 如果到链表末端，跳出循环
		if(m_particleCount >= m_maxParticles)
			break;
		
		/*
		 * Create a particle at the next free slot.
		 * 用0~1之间随机数乘上面积可却表新粒子在该区域中
		 */
		m_particles[m_particleCount].m_pos[0] = m_pos[0] + GET_RANDOM * m_width;
		m_particles[m_particleCount].m_pos[1] = m_pos[1] + GET_RANDOM * m_height;
		m_particles[m_particleCount].m_pos[2] = m_pos[2] + GET_RANDOM * m_depth;
		m_particles[m_particleCount].m_vel[0] = m_velocity[0] + GET_RANDOM * m_velDelta[0];
		m_particles[m_particleCount].m_vel[1] = m_velocity[1] + GET_RANDOM * m_velDelta[1];
		m_particles[m_particleCount].m_vel[2] = m_velocity[2] + GET_RANDOM * m_velDelta[2];

		m_particleCount++;
	}
}

void CRainPS::Shutdown()
{
	if (m_particles)
	{
		delete[] m_particles;
		m_particles = nullptr;
	}

	m_particleCount = 0;
	m_totalTime = 0;
}
