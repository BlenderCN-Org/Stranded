#ifndef __BONE_H__
#define __BONE_H__

#include "matrix.h"

/*
 * 骨骼动画:
 * 通过父 ID 访问父骨骼. 绝对矩阵是骨骼的最终矩阵，通过相对矩阵和其父骨骼的绝对矩阵相乘得到.
 * 骨骼的相对矩阵是每个骨骼的位置. 例如，选择骨骼 30度，相对矩阵保存结果。但由于矩阵和层次结构相关联，因此在使用骨骼时，
 * 必须将子骨骼(当前骨骼)的相对矩阵施加给父骨骼的最终矩阵，这样父骨骼才会影响到子骨骼.
 */
class CBone
{
public:
	CBone() : m_parent(0), m_length(0.0f) {}

	~CBone() {}

	void SetBone(int P, float L)
	{
		// Set the bone data.
		m_parent = P;
		m_length = L;
	}

	void SetBone(int P, float L, CMatrix4x4& R, CMatrix4x4& A)
	{
		// Set the bone data.
		m_parent = P;
		m_length = L;
		m_relative = R;
		m_absolute = A;
	}

public:
	// 父 ID
	int m_parent;
	// 长度 - 骨骼的大小. (-1，代表根骨骼，无父骨骼)
	float m_length;

	// 相对矩阵
	CMatrix4x4 m_relative;
	// 绝对矩阵
	CMatrix4x4 m_absolute;
};

#endif
