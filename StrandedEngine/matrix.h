#ifndef __MATRIX_H__
#define __MATRIX_H__

/*
 * 矩阵类
 * 多次旋转后会出现 gimble lock，导致旋转错误
 * Direct3D
 * D3DXMATRIX16
 * D3DXMatrixIdentity() - 创建单位矩阵
 * D3DXMatrixTranslation() - 实现平移
 * D3DXMatrixRotateAxis() - 矩阵平移
 * D3DXMatrixRotationYawPitchRoll() - 围绕任意轴旋转[根据偏航、倾斜和滚动量旋转]
 * D3DXMatrixRotationX()、D3DXMatrixRotationY()和D3DXMatrixRotationZ()实现矢量按照各自的轴旋转
 * 投影矩阵: 将矢量位置转换到屏幕上的像素位置
 * 视角矩阵: 获取几何图形位置，并将“摄像机”信息施加给它，这样可以根据当前视角信息改变场景外观
 * 世界矩阵: 也叫模型矩阵，获取本地集合图形数据，并将其转换到世界空间
 * 合并投影矩阵、视角矩阵和世界矩阵后将生成一个最终矩阵 - 模型视角投影矩阵(Model-View-Projection，MVP)
 * 合并视角矩阵和投影矩阵生成 - 视角投影矩阵
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

	//单位矩阵函数
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

	// 平移函数
	void Translate(CVector3 &Translate);
	void Translate(float x, float y, float z);
	void inverseTranslate();

	void Rotate(double angle, int x, int y, int z);

	CVector3 VectorMatrixMultiply(CVector3 &v);
	CVector3 VectorMatrixMultiply3x3(CVector3 &v);

private:
	// 4行4列 2D数组
	float matrix[16];
};

#endif