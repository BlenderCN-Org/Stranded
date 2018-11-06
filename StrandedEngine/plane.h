#ifndef __STRANDED_PLANE_H__
#define __STRANDED_PLANE_H__

/*
 * ƽ��
 * typedef struct D3DXPLANE {
	FLOAT a;
	FLOAT b;
	FLOAT c;
	FLOAT d;
	} D3DXPLANE;
 * D3DXPlaneDot() - ����ƽ��� 4D ʸ�����
 * D3DXPlaneDotCoord() - ����ƽ��� 3D ʸ�����
 * D3DXPlaneDotCoord() - ʹ�üٶ� w Ϊ 0 ����ƽ��� 3D ʸ�����
 * D3DXPlaneFromPoints() - �������μ���ƽ��
 * D3DXPlaneFromPointNormal() - �ɵ�ͷ��߼���ƽ��
 * �����ڼ���ͼ��ѡ����ײ����
 * V*N+D=0
 * D: ƽ�浽ԭ��ľ��룬V: λ��ƽ��ĳ��λ���ϵĵ�(�õ����ƽ�������λ����)��N: ƽ�淨��(ƽ�淨����ƽ����������ƽ��ǰ��ķ���)
 */

#include "mathLibrary.h"

class CPlane
{
public:
	CPlane();
	CPlane(float A, float B, float C, float D);
	CPlane(CVector3 &n, float D);

	// �������δ���ƽ��
	void CreatePlaneFromTri(CVector3 &t1, CVector3 &t2, CVector3 &t3);

	// �ֶ�����ƽ������
	void SetPlaneNormal(float A, float B, float C)
	{
		a = A; b = B; c = C;
	}

	void SetPlaneIntercept(float D) { d = D; }

	// ��������ƽ��Ľ����
	bool Intersect(CPlane &p2, CPlane &p3, CVector3 *intersectPoint);
	bool Intersect(CPlane &p1, CVector3 *intersectPoint);
	bool Intersect(CPlane &p1, CRay *intersect);

	//////////////////////////��������з��࣬����ƽ��֮����ڲ�////////////////////////////////////////////////
	// ����ƽ����Ե������ƽ���λ��[����ƽ���ϡ�ƽ�����ƽ��ǰ]
	int ClassifyPoint(CVector3& v);
	// ����ƽ����Զ���������ƽ���λ��[λ����һ��]
	int ClassifyPolygon(CPolygon &pol);
	int ClassifyPoint(float x, float y, float z);

	// ʵ������ƽ�����֮��������ڲ�
	void Lerp(CPlane &p1, CPlane &p2, float amount);

	// ����ƽ�浽һ��3Dʸ���ľ���
	float GetDistance(CVector3& v);
	float GetDistance(float x, float y, float z);

public:
	// ƽ�淨��(abc) ƽ�����(d)
	float a, b, c, d;
};

#endif