#include "XStructure.h"

STDMETHODIMP CD3DAllocate::CreateFrame(LPCSTR name, LPD3DXFRAME * outFrame)
{
	// 创建新帧
	stD3DFrameEx *frame = new stD3DFrameEx;
	ZeroMemory(frame, sizeof(stD3DFrameEx));

	*outFrame = NULL;
	frame->Name = NULL;

	if (name)
	{
		int len = strlen(name) + 1;

		frame->Name = new char[len];
		memcpy(frame->Name, name, len * sizeof(char));
	}

	// 初始化
	frame->pMeshContainer = NULL;
	frame->pFrameSibling = NULL;
	frame->pFrameFirstChild = NULL;

	D3DXMatrixIdentity(&frame->TransformationMatrix);
	D3DXMatrixIdentity(&frame->m_finalMatrix);

	*outFrame = frame;

	return S_OK;
}

STDMETHODIMP CD3DAllocate::CreateMeshContainer(LPCSTR name, CONST D3DXMESHDATA * mesh, CONST D3DXMATERIAL * mats, CONST D3DXEFFECTINSTANCE * effects, DWORD numMats, CONST DWORD * indices, LPD3DXSKININFO skin, LPD3DXMESHCONTAINER * outContainer)
{
	// 分配网格容器对象并清除
	stD3DContainerEx *meshCon = new stD3DContainerEx;
	ZeroMemory(meshCon, sizeof(stD3DContainerEx));

	*outContainer = NULL;
	meshCon->Name = NULL;

	// 将网格容器的名称复制到网格容器对象的 Name 成员变量
	if (name)
	{
		int len = strlen(len) + 1;

		meshCon->Name = new char[len];
		memcpy(meshCon->Name, name, len * sizeof(char));
	}

	// 当前版本，仅支持常用网格类型，不支持累进网格或碎片网格
	if (mesh->Type != D3DXMESHTYPE_MESH)
	{
		// D3DXMESHTYPE_PMESH are progressive meshes.
		// D3DXMESHTYPE_PATCHMESH are patch meshes.
		// In this version we only handle normal meshes.
		DestroyMeshContainer(meshCon);
		return E_FAIL;
	}

	meshCon->MeshData.Type = D3DXMESHTYPE_MESH;

	// Copy indices.
	unsigned long numFaces = mesh->pMesh->GetNumFaces();
	meshCon->pAdjacency = new unsigned long[numFaces * 3];
	memcpy(meshCon->pAdjacency, indices, sizeof(unsigned long) * numFaces * 3);

	// Get D3D Device.
	LPDIRECT3DDEVICE9 d3dDevice = NULL;
	mesh->pMesh->GetDevice(&d3dDevice);

	// Get mesh's declaration.
	D3DVERTEXELEMENT9 elements[MAX_FVF_DECL_SIZE];
	if (FAILED(mesh->pMesh->GetDeclaration(elements)))
		return E_FAIL;

	// Clone mesh to get copy of mesh.
	mesh->pMesh->CloneMesh(D3DXMESH_MANAGED, elements, d3dDevice, &meshCon->MeshData.pMesh);

	// Allocate materials and textures.
	meshCon->NumMaterials = max(numMats, 1);
	meshCon->m_pMat = new D3DMATERIAL9[meshCon->NumMaterials];
	meshCon->m_pTextures = new LPDIRECT3DTEXTURE9[meshCon->NumMaterials];

	ZeroMemory(meshCon->m_pMat, sizeof(D3DMATERIAL9) * meshCon->NumMaterials);
	ZeroMemory(meshCon->m_pTextures, sizeof(LPDIRECT3DTEXTURE9) * meshCon->NumMaterials);

	if (numMats > 0)
	{
		// If any materials get them and any textures.
		for (unsigned long i = 0; i < numMats; ++i)
		{
			meshCon->m_pTextures[i] = NULL;
			meshCon->m_pMat[i] = mats[i].MatD3D;

			if (mats[i].pTextureFilename)
			{
				if (FAILED(D3DXCreateTextureFromFile(d3dDevice, mats[i].pTextureFilename, &meshCon->m_pTextures[i])))
					meshCon->m_pTextures[i] = NULL;
			}
		}
	}
	else
	{
		// Set to default material and null texture.
		ZeroMemory(&meshCon->m_pMat[0], sizeof(D3DMATERIAL9));

		meshCon->m_pMat[0].Diffuse.r = 0.5f;
		meshCon->m_pMat[0].Diffuse.g = 0.5f;
		meshCon->m_pMat[0].Diffuse.b = 0.5f;
		meshCon->m_pMat[0].Specular = meshCon->m_pMat[0].Diffuse;
		meshCon->m_pTextures[0] = NULL;
	}

	// Nullify
	meshCon->pSkinInfo = NULL;
	meshCon->m_boneMatrices = NULL;

	if (skin)
	{
		// If skin mesh allocate and get bones.
		meshCon->pSkinInfo = skin;
		skin->AddRef();

		unsigned int numBones = skin->GetNumBones();
		meshCon->m_boneMatrices = new D3DXMATRIX *[numBones];
	}

	// Release reference count.
	if (d3dDevice)
	{
		d3dDevice->Release();
		d3dDevice = NULL;
	}

	*outContainer = meshCon;

	return S_OK;
}

STDMETHODIMP CD3DAllocate::DestroyFrame(LPD3DXFRAME inFrame)
{
	stD3DFrameEx *frame = (stD3DFrameEx*)inFrame;

	// 删除帧名
	if (frame->Name)
	{
		delete[] frame->Name;
		frame->Name = NULL;
	}

	// 删除帧
	if (frame)
	{
		delete frame;
		frame = NULL;
	}

	return S_OK;
}

STDMETHODIMP CD3DAllocate::DestroyMeshContainer(LPD3DXMESHCONTAINER container)
{
	stD3DContainerEx* meshCon = (stD3DContainerEx*)container;

	if (meshCon->Name)
	{
		delete[] meshCon->Name;
		meshCon->Name = NULL;
	}

	if (meshCon->m_pMat)
	{
		delete[] meshCon->m_pMat;
		meshCon->m_pMat = NULL;
	}

	if (meshCon->m_pTextures)
	{
		for (unsigned int i = 0; i < meshCon->NumMaterials; i++)
		{
			if (meshCon->m_pTextures[i])
			{
				meshCon->m_pTextures[i]->Release();
				meshCon->m_pTextures[i] = NULL;
			}
		}

		delete[] meshCon->m_pTextures;
		meshCon->m_pTextures = NULL;
	}

	if (meshCon->pAdjacency)
	{
		delete[] meshCon->pAdjacency;
		meshCon->pAdjacency = NULL;
	}

	if (meshCon->m_boneMatrices)
	{
		delete[] meshCon->m_boneMatrices;
		meshCon->m_boneMatrices = NULL;
	}

	if (meshCon->m_orignalMesh)
	{
		meshCon->m_orignalMesh->Release();
		meshCon->m_orignalMesh = NULL;
	}

	if (meshCon->MeshData.pMesh)
	{
		meshCon->MeshData.pMesh->Release();
		meshCon->MeshData.pMesh = NULL;
	}

	if (meshCon)
	{
		delete meshCon;
		meshCon = NULL;
	}

	return S_OK;
}
