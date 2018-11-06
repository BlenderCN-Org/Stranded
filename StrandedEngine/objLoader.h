#ifndef __OBJ_LOADER_H__
#define __OBJ_LOADER_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "token.h"

/*
 * obj ģ��
 */
struct stObjModel
{
	// ����
	float*	m_pVertices;
	// ����
	float*	m_pNormals;
	// ��������
	float*	m_pTexCoords;
	// ������ģ�������ڼ�����������ļ�����
	int		m_numFaces;
};

// �� OBJ ģ�ͼ��ص�ģ�ͽṹ�У��������ݷ��ظ��������Ķ���
stObjModel* LoadOBJModel(const char* fileName);
// ���ʵ��ĺ��������ͷ�ģ���ڴ�
void FreeOBJModel(stObjModel* model);

#endif
