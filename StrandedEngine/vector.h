/*
 * ʸ��
 */

#ifndef __VECTOR_H__
#define __VECTOR_H__

#include <math.h>

class CVector3
{
public:
	CVector3();
	CVector3(float X, float Y, float Z);

	void operator=(CVector3 &v);
	CVector3 operator-(CVector3 &v);
	CVector3 operator+(CVector3 &v);
	CVector3 operator*(CVector3 &v);
	CVector3 operator/(CVector3 &v);

	CVector3 operator+(float f);
	CVector3 operator-(float f);
	CVector3 operator*(float f);
	CVector3 operator/(float f);

	void operator +=(CVector3 &v);
	void operator -=(CVector3 &v);
	void operator *=(CVector3 &v);
	void operator /=(CVector3 &v);
	void operator +=(float f);
	void operator -=(float f);
	void operator *=(float f);
	void operator /=(float f);

	// ʸ�����
	void CrossProduct(CVector3 &v1, CVector3 &v2);
	// ʸ�����
	float DotProduct3(CVector3 &v1);
	// ����ʸ������
	float GetLength();
	// ʸ����һ��
	void Normal();
	// �����ι�һ��
	void Normalize(CVector3 &v1, CVector3 &v2, CVector3 &v3);
	
public:
	float x, y, z;
};

#endif