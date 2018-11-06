#ifndef __STRANDED_BOUNDING_GEOMETRY_H__
#define __STRANDED_BOUNDING_GEOMETRY_H__

#include "mathLibrary.h"

/*
 * 边界几何图形基类
 */
class CBoundingBase
{
public:
	CBoundingBase() {}
	virtual ~CBoundingBase() {}

	// 创建边界几何图形
	virtual void CreateFromPoints(CVector3 *pointList, int numPoints) = 0;

	// 测试一个点是否和一个边界几何图形发生碰撞
	virtual bool IsPointInside(CVector3 &v) = 0;

	// 通过射线检测碰撞
	virtual bool Intersect(CRay& ray, float *dist) = 0;
	virtual bool Intersect(CRay& ray, float length, float *dist) = 0;

	// 计算边界框每个框边外的平面(6个平面)
	virtual void GetPlanes(CPlane *planes) = 0;
	// 查看射线是否真的在边界几何图形中
	virtual bool IsRayInside(CRay& ray, float length) = 0;
};

/*
 * 边界框
 */
class CBoundingBox : public CBoundingBase
{
public:
	CBoundingBox() {}
	~CBoundingBox() {}

	// 创建边界框
	void CreateFromPoints(CVector3 *pointList, int numPoints);

	// 测试一个点是否和一个边界框发生碰撞
	bool IsPointInside(CVector3 &v);

	// 通过射线检测碰撞
	bool Intersect(CRay& ray, float *dist);
	bool Intersect(CRay& ray, float length, float *dist);

	// 计算边界框每个框边外的平面(6个平面)
	void GetPlanes(CPlane *planes);
	// 查看射线是否真的在边界框中
	bool IsRayInside(CRay& ray, float length);

public:
	CVector3 m_min, m_max;
};

/*
 * 边界球
 */
class CBoundingSphere : public CBoundingBase
{
public:
	CBoundingSphere() : m_radius(0.0f) {}
	~CBoundingSphere() {};

	// 创建边界球
	void CreateFromPoints(CVector3 *pointList, int numPoints);

	// 测试一个点是否和一个边界球发生碰撞
	bool IsPointInside(CVector3 &v);

	// 通过射线检测碰撞
	bool Intersect(CRay& ray, float *dist);
	bool Intersect(CRay& ray, float length, float *dist);

	// 计算边界框每个框边外的平面(6个平面)
	void GetPlanes(CPlane *planes);
	// 查看射线是否真的在边界几何图形中
	bool IsRayInside(CRay& ray, float length);

public:
	CVector3 m_center;
	float m_radius;
};

// 检测两个方框之间的碰撞
bool BoxToBoxIntersect(CBoundingBox &bb1, CBoundingBox &bb2);
// 检测两个球体之间的碰撞
bool SphereToSphereIntersect(CBoundingSphere &bs1, CBoundingSphere &bs2);

#endif