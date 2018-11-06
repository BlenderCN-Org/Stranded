#include "vector.h"
#include "mathDefines.h"


CVector3::CVector3()
{
	x = 0.0f;
	y = 0.0f;
	z = 0.0f;
}

CVector3::CVector3(float X, float Y, float Z)
{
	x = X;
	y = Y;
	z = Z;
}

void CVector3::operator=(CVector3 &v)
{
	x = v.x;
	y = v.y;
	z = v.z;
}

// D3DXVec3Substract(&result, &v1, &v2);
CVector3 CVector3::operator-(CVector3 &v)
{
	return CVector3(x - v.x, y - v.y, z - v.z);
}

// D3DXVec3Add(&result, &v1, &v2);
CVector3 CVector3::operator+(CVector3 &v)
{
	return CVector3(x + v.x, y + v.y, z + v.z);
}

CVector3 CVector3::operator+(float f)
{
	return CVector3(x + f, y + f, z + f);
}

CVector3 CVector3::operator*(CVector3 &v)
{
	return CVector3(x * v.x, y * v.y, z * v.z);
}

CVector3 CVector3::operator*(float f)
{
	return CVector3(x * f, y * f, z * f);
}

CVector3 CVector3::operator/(CVector3 &v)
{
	float new_x = 0.0, new_y = 0.0, new_z = 0.0;
	if (v.x)
		new_x = x / v.x;

	if (v.y)
		new_y = y / v.y;

	if (v.z)
		new_z = z / v.z;

	return CVector3(new_x, new_y, new_z);
}

CVector3 CVector3::operator/(float f)
{
	if (f)
		f = 1 / f;

	return CVector3(x * f, y * f, z * f);
}

CVector3 CVector3::operator-(float f)
{
	return CVector3(x - f, y - f, z - f);
}

void CVector3::operator+=(CVector3 &v)
{
	x += v.x;
	y += v.y;
	z += v.z;
}

void CVector3::operator+=(float f)
{
	x += f;
	y += f;
	z += f;
}

void CVector3::operator-=(CVector3 &v)
{
	x -= v.x;
	y -= v.y;
	z -= v.z;
}

void CVector3::operator*=(CVector3 &v)
{
	x *= v.x;
	y *= v.y;
	z *= v.z;
}

void CVector3::operator*=(float f)
{
	x *= f;
	y *= f;
	z *= f;
}

void CVector3::operator/=(CVector3 &v)
{
	if (v.x)
		x /= v.x;
	else
		x = 0.0;

	if (v.y)
		y /= v.y;
	else
		x = 0.0;

	if (v.z)
		z /= v.z;
	else
		z = 0.0;
}

void CVector3::operator/=(float f)
{
	if (f)
		f = 1 / f;

	x *= f;
	y *= f;
	z *= f;
}

void CVector3::operator-=(float f)
{
	x -= f;
	y -= f;
	z -= f;
}

/*
 * 矢量叉积[矢量积，外积]
 * 通过查找与两个矢量正交的矢量而计算得到该新矢量
 * 常用于计算多边形的法线(多边形面朝的方向)
 * D3DXVec3Cross(&result, &v1, &v2);
 */
void CVector3::CrossProduct(CVector3 &v1, CVector3 &v2)
{
	x = ((v1.y * v2.z) - (v1.z * v2.y));
	y = ((v1.z * v2.x) - (v1.x * v2.z));
	z = ((v1.x * v2.y) - (v1.y * v2.x));
}

/*
 * 矢量点积[标量积]
 * 度量两个方向的差
 * 常用与手动光照算法
 * val = D3DXVec3Dot(&v1, &v2);
 */
float CVector3::DotProduct3(CVector3 &v1)
{
	return x * v1.x + y * v1.y + z * v1.z;
}

// val = D3DXVec3Length(&v1);
float CVector3::GetLength()
{
	// 平方根函数时CPU上执行速度最慢的一个函数
	return (float)sqrt(x*x + y*y + z*z);
}

/*
 * 矢量归一化
 * 常用于光照
 * D3DXVec3Normalize(&result, &v1);
 */
void CVector3::Normal()
{
	float lengh = GetLength();

	if (lengh == 0.0f)
		lengh = 1.0f;

	x /= lengh;
	y /= lengh;
	z /= lengh;
}

void CVector3::Normalize(CVector3 &v1, CVector3 &v2, CVector3 &v3)
{
	CVector3 e1, e2;

	e1.x = v2.x - v1.x;
	e1.y = v2.y - v1.y;
	e1.z = v2.z - v1.z;
	e1.Normal();

	e2.x = v3.x - v1.x;
	e2.y = v3.y - v1.y;
	e2.z = v3.z - v2.z;
	e2.Normal();

	CrossProduct(e1, e2);
	Normal();
}
