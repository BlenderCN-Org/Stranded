#ifndef __ROUTE_H__
#define __ROUTE_H__

#include "vector.h"

// ��֧�ֵ�·��
#define ERROR_PATH			-1
#define STRAIGHT_PATH		0
#define CURVE_PATH			1
// Բ·����ʵ����������������·��ʵ��
#define CIRCLE_PATH			2

/*
 һ��ֱ��
 1�������ڲ幫ʽ
	Final = (B - A) * Scalar + A;
 2��3D ʸ��֮����ڲ�ʵ�ִ���
	diff.x = B.x - A.x;
	diff.y = B.y - A.y;
	diff.z = B.z - A.z;

	Final.x = diff.x * Scalar + A.x;
	Final.y = diff.y * Scalar + A.y;
	Final.z = diff.z * Scalar + A.z;
 3��
	float scalar = (float)timeGetTime();
	scalar = (scalar - startTime) * 0.003f;
	scalar = scalar / len;
 
 ��������
	���������ߣ�
 1�����Թ�ʽ:
	������P_0��P_1, ��ͬ�����Բ�ֵ.
	B(t)= P_0+ (P_1- P_0 )t=(1-t) P_0+ tP_1,t��[0,1]
 2�����η���ʽ:
	������P_0��P_1��P_2
	B(t)= ��(1-t)��^2 P_0+ 2t(1-t) P_1+ t^2 P_2,t��[0,1]
 3�����η���ʽ:
	P_0��P_1��P_2��P_3
	B(t)= P_0 ��(1-t)��^3+ 3P_1 t��(1-t)��^2+ 3P_2 t^2 (1-t)+ P_3 t^3,t��[0,1]
 4����ֵ
	float Time = (float)timeGetTime();
	Time = (Time - startTime)*0.001f;
	float scalar = (((float)sin(Time)) + 1.0f)*0.5f;
 
 ����Բ��
 1��Բ�ġ��뾶����ʼ�㡢�����ƶ���ƽ�淨�ߡ�ƽ���ϵ� u �� v ��������ʸ��.
	����ʸ�� u ��ƽ�淨�ߵ����(����)�ɵõ�ʸ�� v, ��ʸ�� u �ĳ��ȿɵõ��뾶.
 2����ֵ
	����Բ�ܵ�һ���Ƕ�
	float Time = (float)timeGetTime();
	Time = (Time - startTime)*0.06f;
*/
// ·��
class CPath
{
public:
	CPath();
	CPath(int Type, CVector3& start, CVector3& cnt1, CVector3& cnt2, CVector3& end);
	CPath(CVector3& center, CVector3& direction, CVector3& start);
	~CPath();

public:
	void Shutdown();

private:
	void SetPath(int Type, CVector3& start, CVector3& cnt1, CVector3& cnt2, CVector3& end);
	void SetPath(CVector3& center, CVector3& direction, CVector3& start);

public:
	// Type of path this is.
	int m_type;

	// Start location of this path.
	CVector3 m_startPos;
	// Control point one.
	CVector3 m_control1;
	// Control point two.
	CVector3 m_control2;
	// Destination location.
	CVector3 m_endPos;

	// Center of circle path.
	CVector3 m_centerPos;
	// the point in the plane (direction). (0.0f, 0.0f, 1.0f)
	CVector3 m_planeDirection;
	
	/////////////////////////// u, v, normal �������ݼ������ ///////////////////////////////////////////////
	// u and v are orthonormal vectors in the plane. u points from the center to the start and v is perpendicular to u.
	CVector3 m_circleU;
	CVector3 m_circleV;
	CVector3 m_planeNormal;
	float m_radius;
	
	// Start length for this path.
	float m_start;
	// Total length distance of path.
	float m_total;

	// Pointer to the next path.
	CPath *m_next;
};

//////////////////////////////////////////////////////////////////////////

// �洢·���������� - ·��
class CRoute
{
public:
	CRoute();
	~CRoute();

	bool AddPath(int Type, CVector3& start, CVector3& cnt1, CVector3& cnt2, CVector3& end);
	bool AddPath(CVector3& center, CVector3& direction, CVector3& start);

	void GetPosition(float time, CVector3 &objPos);

	CVector3 CalcBezierCurvePos(CVector3& start, CVector3& c1, CVector3& c2, CVector3& end, float Scalar);

	CVector3 CalcStriaghtPath(CVector3& start, CVector3& end, float Scalar);

	CVector3 CalcCirclePos(float angle, CVector3& center, CVector3& u, CVector3& v, float radius);

	void Shutdown();

private:
	CPath *m_path;
	float m_startTime;
};

#endif
