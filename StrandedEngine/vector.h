/*
 * 矢量
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

	// 矢量叉积
	void CrossProduct(CVector3 &v1, CVector3 &v2);
	// 矢量点积
	float DotProduct3(CVector3 &v1);
	// 计算矢量长度
	float GetLength();
	// 矢量归一化
	void Normal();
	// 三角形归一化
	void Normalize(CVector3 &v1, CVector3 &v2, CVector3 &v3);
	
public:
	float x, y, z;
};

#endif