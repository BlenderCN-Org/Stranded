#ifndef __STRANDED_PHYSICS_H__
#define __STRANDED_PHYSICS_H__

#include "vector.h"

/*
 * @brief: 定义物体
 */
struct stPointMass
{
	stPointMass() : m(0.0f) {}

	float		m;
	CVector3	pos;			// 位置 
	CVector3	velocity;		// 速度
	CVector3	force;			// 力
};

/*
 * @brief: 施加作用力
 * @param: 作用力, 要操纵和施加作用力的物体
 */
inline void ApplyForce(CVector3 &force, stPointMass *m)
{
	if (m != nullptr)
		m->force += force;
}

/*
 * @brief: 将物体作用力施加给物体的速度和位置, 根据施加的作用力(如 环境重力)将物体从一个位置移动到另一个位置
 * @param: 比值(Scalar 时间)根据时间而不是渲染帧进行更新；
 *			要操纵的物体
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
