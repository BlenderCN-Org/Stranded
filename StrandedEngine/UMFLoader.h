#ifndef __UMF_LOADER_H__
#define __UMF_LOADER_H__

#include <stdio.h>

static const int SC_I_UMF_VERSION = 9000;

// λ����Ϣ
struct stVector
{
	stVector() : x(0.0f), y(0.0f), z(0.0f) {}

	// Position.
	float x, y, z;
};

// ����������Ϣ
struct stTexCoord
{
	stTexCoord() : tu(0.0f), tv(0.0f) {}

	// tu, tv texture coordinates.
	float tu, tv;
};

// ָ����������������һ����۷���
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

// ����UMFģ��
bool SaveUMF(const char *file, stUMFModel *meshes, int numMeshes);
// ��UMFģ�ͼ��ص��ڴ���
bool LoadUMF(const char *file, stUMFModel **model, int *totalModels);
void FreeUMFModel(stUMFModel *mesh);
void FreeUMFModel(stUMFModel *meshes, int numMeshes);

#endif
