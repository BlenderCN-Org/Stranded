#ifndef __UMF_LOADER_H__
#define __UMF_LOADER_H__

#include <stdio.h>

static const int SC_I_UMF_VERSION = 9000;

// 位置信息
struct stVector
{
	stVector() : x(0.0f), y(0.0f), z(0.0f) {}

	// Position.
	float x, y, z;
};

// 纹理坐标信息
struct stTexCoord
{
	stTexCoord() : tu(0.0f), tv(0.0f) {}

	// tu, tv texture coordinates.
	float tu, tv;
};

// 指定三个顶点索引和一个外观法线
struct stFace
{
	stFace()
	{
		indices[0] = indices[1] = indices[2] = 0;
	}

	// Vertex indexes and a surface normal.
	unsigned int indices[3];
	stVector normal;
};

struct stUMFModel
{
	// Material data.
	stVector diffuse;
	stVector specular;
	int power;

	// Model data
	stVector *vertices;
	stTexCoord *texCoords;
	stVector *normals;
	stVector *colors;
	stFace *faces;

	// Array counters;
	int totalVertices;
	int totalFaces;

	// Bounding box data.
	stVector bbMin, bbMax;
};

// 保存UMF模型
bool SaveUMF(const char *file, stUMFModel *meshes, int numMeshes);
// 将UMF模型加载到内存中
bool LoadUMF(const char *file, stUMFModel **model, int *totalModels);
void FreeUMFModel(stUMFModel *mesh);
void FreeUMFModel(stUMFModel *meshes, int numMeshes);

#endif
