#ifndef __STRANDED_PHYSICS_H__
#define __STRANDED_PHYSICS_H__

#include "vector.h"

/*
 * @brief: ��������
 */
struct stPointMass
{
	stPointMass() : m(0.0f) {}

	float		m;
	CVector3	pos;			// λ�� 
	CVector3	velocity;		// �ٶ�
	CVector3	force;			// ��
};

/*
 * @brief: ʩ��������
 * @param: ������, Ҫ���ݺ�ʩ��������������
 */
inline void ApplyForce(CVector3 &force, stPointMass *m)
{
	if (m != nullptr)
		m->force += force;
}

/*
 * @brief: ������������ʩ�Ӹ�������ٶȺ�λ��, ����ʩ�ӵ�������(�� ��������)�������һ��λ���ƶ�����һ��λ��
 * @param: ��ֵ(Scalar ʱ��)����ʱ���������Ⱦ֡���и��£�
 *			Ҫ���ݵ�����
 */
inline void SimulateMass(float dt, stPointMass *m)
{
	// Calculate new velocity and position.
	if (m != nullptr)
	{
		m->velocity += (m->force / m->m)*dt;
		m->pos += m->velocity*dt;
	}
}

#endif
