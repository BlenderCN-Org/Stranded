#ifndef __OBJ_LOADER_H__
#define __OBJ_LOADER_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "token.h"

/*
 * obj 模型
 */
struct stObjModel
{
	// 顶点
	float*	m_pVertices;
	// 法线
	float*	m_pNormals;
	// 纹理坐标
	float*	m_pTexCoords;
	// 在完整模型中用于计数外观总数的计数器
	int		m_numFaces;
};

// 将 OBJ 模型加载到模型结构中，并将数据返回给调用它的对象
stObjModel* LoadOBJModel(const char* fileName);
// 以适当的函数调用释放模型内存
void FreeOBJModel(stObjModel* model);

#endif
