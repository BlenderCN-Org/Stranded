#ifndef __STRANDED_RAY_H__
#define __STRANDED_RAY_H__

/*
 * 射线
 * 包含原点(位置)和方向的结构
 * 用于测试射线是否和球、三角形或平面相交。常用于碰撞检测
 * 射线跟踪渲染技术:
 * 射线跟踪是在场景中发射射线，并确定哪一根射线击中了哪一个物体。然后可以使用该信息在场景中出现碰撞、或击中的像素位置添加颜色。
 */

#include "vector.h"
#include "plane.h"

class CRay
{
public:
	CRay() {}

	bool Intersect(CVector3 &pos, float radius, float *dist);
	bool Intersect(CVector3 &p1, CVector3 &p2, CVector3 &p3, bool cull, float *dist);
	bool Intersect(CVector3 & p1, CVector3 &p2, CVector3 &p3, bool cull, float length, float *dist);
	bool Intersect(CPlane &p1, bool cull, CVector3 *intersectPoint, float *dist);
	bool Intersect(CPlane &p1, bool cull, float length, CVector3 *intersectPoint, float *dist);

public:
	CVector3 m_origin;		// 原点(位置)
	CVector3 m_direction;	// 射线方向
};

#endif
