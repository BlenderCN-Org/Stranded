#ifndef __STRANDED_RAY_H__
#define __STRANDED_RAY_H__

/*
 * ����
 * ����ԭ��(λ��)�ͷ���Ľṹ
 * ���ڲ��������Ƿ���������λ�ƽ���ཻ����������ײ���
 * ���߸�����Ⱦ����:
 * ���߸������ڳ����з������ߣ���ȷ����һ�����߻�������һ�����塣Ȼ�����ʹ�ø���Ϣ�ڳ����г�����ײ������е�����λ�������ɫ��
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
	CVector3 m_origin;		// ԭ��(λ��)
	CVector3 m_direction;	// ���߷���
};

#endif
