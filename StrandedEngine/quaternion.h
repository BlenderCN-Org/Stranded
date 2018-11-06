#ifndef __QUATERNION_H__
#define __QUATERNION_H__

#include "vector.h"
#include "mathDefines.h"

/*
 * 四元组
 * q = w + xi + yj + zk [1实数 4虚数]
 * Direct3D 的四元组结构
	typedef struct D3DXQUATERNION {
		FLOAT x;
		FLOAT y;
		FLOAT z;
		FLOAT w;
	} D3DXQUATERNION
	点积: D3DXQuaternionDot()
 */

class CQuaternion
{
public:
	CQuaternion();
	CQuaternion(float xAxis, float yAxis, float zAxis, float wAxis);

	void operator=(const CQuaternion &q);
	// 乘法 D3DXQuaternionMultiply()
	CQuaternion operator*(const CQuaternion &q);
	CQuaternion operator+(const CQuaternion &q);

	// 由轴创建四元组
	void CreateQuatFromAxis(CVector3 &a, float radians);

	// 计算四元组量值(长度) D3XQuaternionLength()
	float Length();
	// 四元组归一化
	void Normal();

	// 查找四元组共轭(四元组的共轭转置)
	CQuaternion Conjugate();
	// 叉积
	CQuaternion CrossProduct(const CQuaternion &q);

	// 旋转四元组
	void Rotatef(float amount, float xAxis, float yAxis, float zAxis);
	void RotationRadiansf(double X, double Y, double Z);

	// 由四元组创建矩阵
	void CreateMatrix(float *pMatrix);

	// 对两个四元组进行球形内插
	void Slerp(const CQuaternion &q1, const CQuaternion &q2, float t);

public:
	float w, x, y, z;
};

#endif
