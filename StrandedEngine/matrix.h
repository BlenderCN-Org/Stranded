#ifndef __MATRIX_H__
#define __MATRIX_H__

/*
 * ������
 * �����ת������ gimble lock��������ת����
 * Direct3D
 * D3DXMATRIX16
 * D3DXMatrixIdentity() - ������λ����
 * D3DXMatrixTranslation() - ʵ��ƽ��
 * D3DXMatrixRotateAxis() - ����ƽ��
 * D3DXMatrixRotationYawPitchRoll() - Χ����������ת[����ƫ������б�͹�������ת]
 * D3DXMatrixRotationX()��D3DXMatrixRotationY()��D3DXMatrixRotationZ()ʵ��ʸ�����ո��Ե�����ת
 * ͶӰ����: ��ʸ��λ��ת������Ļ�ϵ�����λ��
 * �ӽǾ���: ��ȡ����ͼ��λ�ã����������������Ϣʩ�Ӹ������������Ը��ݵ�ǰ�ӽ���Ϣ�ı䳡�����
 * �������: Ҳ��ģ�;��󣬻�ȡ���ؼ���ͼ�����ݣ�������ת��������ռ�
 * �ϲ�ͶӰ�����ӽǾ����������������һ�����վ��� - ģ���ӽ�ͶӰ����(Model-View-Projection��MVP)
 * �ϲ��ӽǾ����ͶӰ�������� - �ӽ�ͶӰ����
 */

#include "vector.h"

class CMatrix4x4
{
public:
	CMatrix4x4() { Identity(); }
	CMatrix4x4(const CMatrix4x4 &m);
	CMatrix4x4(float r11, float r12, float r13, float r14,
		float r21, float r22, float r23, float r24,
		float r31, float r32, float r33, float r34,
		float r41, float r42, float r43, float r44);
	~CMatrix4x4() {};

	//��λ������
	void Identity();

	void operator=(CMatrix4x4 &m);
	CMatrix4x4 operator-(CMatrix4x4 &m);
	CMatrix4x4 operator+(CMatrix4x4 &m);
	CMatrix4x4 operator*(CMatrix4x4 &m);

	CMatrix4x4 operator*(float f);
	CMatrix4x4 operator/(float f);

	void operator+=(CMatrix4x4 &m);
	void operator-=(CMatrix4x4 &m);
	void operator*=(CMatrix4x4 &m);

	void operator*=(float f);
	void operator/=(float f);

	// ƽ�ƺ���
	void Translate(CVector3 &Translate);
	void Translate(float x, float y, float z);
	void inverseTranslate();

	void Rotate(double angle, int x, int y, int z);

	CVector3 VectorMatrixMultiply(CVector3 &v);
	CVector3 VectorMatrixMultiply3x3(CVector3 &v);

private:
	// 4��4�� 2D����
	float matrix[16];
};

#endif