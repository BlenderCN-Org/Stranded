#ifndef __BONE_H__
#define __BONE_H__

#include "matrix.h"

/*
 * ��������:
 * ͨ���� ID ���ʸ�����. ���Ծ����ǹ��������վ���ͨ����Ծ�����丸�����ľ��Ծ�����˵õ�.
 * ��������Ծ�����ÿ��������λ��. ���磬ѡ����� 30�ȣ���Ծ��󱣴����������ھ���Ͳ�νṹ������������ʹ�ù���ʱ��
 * ���뽫�ӹ���(��ǰ����)����Ծ���ʩ�Ӹ������������վ��������������Ż�Ӱ�쵽�ӹ���.
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
	// �� ID
	int m_parent;
	// ���� - �����Ĵ�С. (-1��������������޸�����)
	float m_length;

	// ��Ծ���
	CMatrix4x4 m_relative;
	// ���Ծ���
	CMatrix4x4 m_absolute;
};

#endif
