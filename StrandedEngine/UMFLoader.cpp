#include "UMFLoader.h"

/*
 * @brief: �����ص�stUMFModel �ṹ�е�ģ�ͱ���ΪUMF �ļ�
 *
 * @param: Ҫ������ļ���������ģ�͵����������ַ������ģ���е���������
 */
bool SaveUMF(const char *file, stUMFModel *meshes, int numMeshes)
{
	FILE *fp;
	int id = SC_I_UMF_VERSION;
	int f = 0;

	// Make sure we have valid data.
	if (!file) return false;
	if (!meshes || numMeshes <= 0)
		return false;

	// Open file. ��ģ���ļ���Ϊ���
#pragma warning(push)
#pragma warning(disable:4996)
	fp = fopen(file, "wb");
#pragma warning(pop)
	if (!fp) return false;

	// Wirte ID (9000). �ļ��汾
	fwrite(&id, 4, 1, fp);

	// Write total mesh. �����ļ���������
	fwrite(&numMeshes, 4, 1, fp);

	// Save each mesh to the file.
	for (int m = 0; m < numMeshes; m++)
	{
		// дһ������

		// ��д������Ͱ������淴�������ľ��淴����Ϣ��
		// Write material data.
		fwrite(&meshes[m].diffuse, 4 * 3, 1, fp);
		fwrite(&meshes[m].specular, 4 * 3, 1, fp);
		fwrite(&meshes[m].power, 4, 1, fp);

		// Ȼ��д���������Ͷ�������
		// Write number of verts then vertices.
		fwrite(&meshes[m].totalVertices, 4, 1, fp);
		fwrite(meshes[m].vertices, sizeof(stVector) * meshes[m].totalVertices, 1, fp);

		// Start with the total normals then read any normals.
		if (!meshes[m].normals)
			fwrite(&f, 4, 1, fp);
		else
		{
			fwrite(&meshes[m].totalVertices, 4, 1, fp);
			fwrite(meshes[m].normals, sizeof(stVector) * meshes[m].totalVertices, 1, fp);
		}

		// Write colors.
		if (!meshes[m].colors)
			fwrite(&f, 4, 1, fp);
		else
		{
			fwrite(&meshes[m].totalVertices, 4, 1, fp);
			fwrite(meshes[m].colors, sizeof(stVector) * meshes[m].totalVertices, 1, fp);
		}

		// Write texture coords.
		if (!meshes[m].texCoords)
			fwrite(&f, 4, 1, fp);
		else
		{
			fwrite(&meshes[m].totalVertices, 4, 1, fp);
			fwrite(meshes[m].texCoords, sizeof(stTexCoord) * meshes[m].totalVertices, 1, fp);
		}

		// Write total faces, and face data.
		fwrite(&meshes[m].totalFaces, 4, 1, fp);
		for (int i = 0; i < meshes[m].totalFaces; i++)
		{
			fwrite(meshes[m].faces[i].indices, 4 * 3, 1, fp);
			fwrite(&meshes[m].faces[i].normal, sizeof(stVector), 1, fp);
		}
	}

	fclose(fp);

	return true;
}

/*
 * @brief: ���� UMF ģ��
 * @param: �ļ���������ָ���Ѽ���ģ�͵�ָ���ַ������������������ַ
 */
bool LoadUMF(const char *file, stUMFModel **model, int *totalModels)
{
	FILE *fp = NULL;
	int id = 0, temp = 0, numVerts = 0, numMeshes = 0;
	stVector *tVec = nullptr;
	stTexCoord *pTexC = nullptr;
	stFace *pFaces = nullptr;
	stUMFModel *meshes = nullptr;

	// Error checking.
	if (!file || !model)
		return false;

	// Open file.
#pragma warning(push)
#pragma warning(disable:4996)
	fp = fopen(file, "rb");
#pragma warning(pop)
	if (!fp) return false;

	// All UMF files have an id of 9000.
	fread(&id, 4, 1, fp);
	if (id != SC_I_UMF_VERSION)
		return false;

	fread(&numMeshes, 4, 1, fp);
	if (!numMeshes)
	{
		fclose(fp);
		return false;
	}

	meshes = new stUMFModel[numMeshes];
	if (!meshes)
	{
		fclose(fp);
		return false;
	}

	for (int i = 0; i < numMeshes; i++)
	{
		// Read material data.
		fread(&meshes[i].diffuse, 4 * 3, 1, fp);
		fread(&meshes[i].specular, 4 * 3, 1, fp);
		fread(&meshes[i].power, 4, 1, fp);

		// Read vertices. Start with total then read vertices.
		fread(&numVerts, 4, 1, fp);
		meshes[i].totalVertices = numVerts;

		// Read vertex data.
		tVec = new stVector[numVerts];
		fread(tVec, sizeof(stVector)*numVerts, 1, fp);
		meshes[i].vertices = tVec;

		// Read total. If any read normals.
		fread(&temp, 4, 1, fp);
		if (temp > 0)
		{
			tVec = new stVector[numVerts];
			fread(tVec, sizeof(stVector)*numVerts, 1, fp);
			meshes[i].normals = tVec;
		}

		// Read colors.
		fread(&temp, 4, 1, fp);
		if (temp > 0)
		{
			tVec = new stVector[numVerts];
			fread(tVec, sizeof(stVector)*numVerts, 1, fp);
			meshes[i].colors = tVec;
		}

		// Read texture coords.
		fread(&temp, 4, 1, fp);
		if (temp > 0)
		{
			pTexC = new stTexCoord[numVerts];
			fread(pTexC, sizeof(stTexCoord)*numVerts, 1, fp);
			meshes[i].texCoords = pTexC;
		}

		// Read face data.
		fread(&temp, 4, 1, fp);
		if (temp > 0)
		{
			// Get number, allocate space, then read face data.
			meshes[i].totalFaces = temp;

			pFaces = new stFace[meshes[i].totalFaces];

			for (int f = 0; f < meshes[i].totalFaces; f++)
			{
				fread(pFaces[f].indices, 4 * 3, 1, fp);
				fread(&pFaces[f].normal, sizeof(stVector), 1, fp);
			}

			meshes[i].faces = pFaces;
		}

		// Calculate model's bounding box.
		tVec = meshes[i].vertices;
		for (int k = 0; k < numVerts; k++)
		{
			if (tVec[k].x < meshes[i].bbMin.x)
				meshes[i].bbMin.x = tVec[k].x;

			if (tVec[k].y < meshes[i].bbMin.y)
				meshes[i].bbMin.y = tVec[k].y;

			if (tVec[k].z < meshes[i].bbMin.z)
				meshes[i].bbMin.z = tVec[k].z;

			if (tVec[k].x > meshes[i].bbMax.x)
				meshes[i].bbMax.x = tVec[k].x;

			if (tVec[k].y > meshes[i].bbMax.y)
				meshes[i].bbMax.y = tVec[k].y;

			if (tVec[k].z > meshes[i].bbMax.z)
				meshes[i].bbMax.z = tVec[k].z;
		}
	}

	// Save information to pointer parameters.
	*model = meshes;
	if (totalModels)
		*totalModels = numMeshes;

	fclose(fp);

	return true;
}

/*
 * @brief: �ͷŵ�������, ɾ�����㡢���ߡ���ɫ�������������
 * @param: Ҫ�ͷŵ�����
 */
void FreeUMFModel(stUMFModel *mesh)
{
	if (!mesh) return;

	// Rlease all resources.
	if (mesh->faces)
	{
		delete[] mesh->faces;
		mesh->faces = nullptr;
	}

	if (mesh->vertices)
	{
		delete[] mesh->vertices;
		mesh->vertices = nullptr;
	}

	if (mesh->texCoords)
	{
		delete[] mesh->texCoords;
		mesh->texCoords = nullptr;
	}

	if (mesh->colors)
	{
		delete[] mesh->colors;
		mesh->colors = nullptr;
	}

	if (mesh->normals)
	{
		delete[] mesh->normals;
		mesh->normals = nullptr;
	}
}

/*
 * @param: �������顢ģ���е���������
 */
void FreeUMFModel(stUMFModel *meshes, int numMeshes)
{
	if (!meshes || numMeshes <= 0)
		return;

	for (int i = 0; i < numMeshes; i++)
		FreeUMFModel(&meshes[i]);
}
