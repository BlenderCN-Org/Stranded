#ifndef __STRANDED_BOUNDING_GEOMETRY_H__
#define __STRANDED_BOUNDING_GEOMETRY_H__

#include "mathLibrary.h"

/*
 * �߽缸��ͼ�λ���
 */
class CBoundingBase
{
public:
	CBoundingBase() {}
	virtual ~CBoundingBase() {}

	// �����߽缸��ͼ��
	virtual void CreateFromPoints(CVector3 *pointList, int numPoints) = 0;

	// ����һ�����Ƿ��һ���߽缸��ͼ�η�����ײ
	virtual bool IsPointInside(CVector3 &v) = 0;

	// ͨ�����߼����ײ
	virtual bool Intersect(CRay& ray, float *dist) = 0;
	virtual bool Intersect(CRay& ray, float length, float *dist) = 0;

	// ����߽��ÿ��������ƽ��(6��ƽ��)
	virtual void GetPlanes(CPlane *planes) = 0;
	// �鿴�����Ƿ�����ڱ߽缸��ͼ����
	virtual bool IsRayInside(CRay& ray, float length) = 0;
};

/*
 * �߽��
 */
class CBoundingBox : public CBoundingBase
{
public:
	CBoundingBox() {}
	~CBoundingBox() {}

	// �����߽��
	void CreateFromPoints(CVector3 *pointList, int numPoints);

	// ����һ�����Ƿ��һ���߽������ײ
	bool IsPointInside(CVector3 &v);

	// ͨ�����߼����ײ
	bool Intersect(CRay& ray, float *dist);
	bool Intersect(CRay& ray, float length, float *dist);

	// ����߽��ÿ��������ƽ��(6��ƽ��)
	void GetPlanes(CPlane *planes);
	// �鿴�����Ƿ�����ڱ߽����
	bool IsRayInside(CRay& ray, float length);

public:
	CVector3 m_min, m_max;
};

/*
 * �߽���
 */
class CBoundingSphere : public CBoundingBase
{
public:
	CBoundingSphere() : m_radius(0.0f) {}
	~CBoundingSphere() {};

	// �����߽���
	void CreateFromPoints(CVector3 *pointList, int numPoints);

	// ����һ�����Ƿ��һ���߽�������ײ
	bool IsPointInside(CVector3 &v);

	// ͨ�����߼����ײ
	bool Intersect(CRay& ray, float *dist);
	bool Intersect(CRay& ray, float length, float *dist);

	// ����߽��ÿ��������ƽ��(6��ƽ��)
	void GetPlanes(CPlane *planes);
	// �鿴�����Ƿ�����ڱ߽缸��ͼ����
	bool IsRayInside(CRay& ray, float length);

public:
	CVector3 m_center;
	float m_radius;
};

// �����������֮�����ײ
bool BoxToBoxIntersect(CBoundingBox &bb1, CBoundingBox &bb2);
// �����������֮�����ײ
bool SphereToSphereIntersect(CBoundingSphere &bs1, CBoundingSphere &bs2);

#endif