#ifndef __QUATERNION_H__
#define __QUATERNION_H__

#include "vector.h"
#include "mathDefines.h"

/*
 * ��Ԫ��
 * q = w + xi + yj + zk [1ʵ�� 4����]
 * Direct3D ����Ԫ��ṹ
	typedef struct D3DXQUATERNION {
		FLOAT x;
		FLOAT y;
		FLOAT z;
		FLOAT w;
	} D3DXQUATERNION
	���: D3DXQuaternionDot()
 */

class CQuaternion
{
public:
	CQuaternion();
	CQuaternion(float xAxis, float yAxis, float zAxis, float wAxis);

	void operator=(const CQuaternion &q);
	// �˷� D3DXQuaternionMultiply()
	CQuaternion operator*(const CQuaternion &q);
	CQuaternion operator+(const CQuaternion &q);

	// ���ᴴ����Ԫ��
	void CreateQuatFromAxis(CVector3 &a, float radians);

	// ������Ԫ����ֵ(����) D3XQuaternionLength()
	float Length();
	// ��Ԫ���һ��
	void Normal();

	// ������Ԫ�鹲��(��Ԫ��Ĺ���ת��)
	CQuaternion Conjugate();
	// ���
	CQuaternion CrossProduct(const CQuaternion &q);

	// ��ת��Ԫ��
	void Rotatef(float amount, float xAxis, float yAxis, float zAxis);
	void RotationRadiansf(double X, double Y, double Z);

	// ����Ԫ�鴴������
	void CreateMatrix(float *pMatrix);

	// ��������Ԫ����������ڲ�
	void Slerp(const CQuaternion &q1, const CQuaternion &q2, float t);

public:
	float w, x, y, z;
};

#endif
