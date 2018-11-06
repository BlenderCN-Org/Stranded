#include "XMesh.h"

CXModel::CXModel() : m_root(NULL),
	m_animControl(NULL),
	m_boneMatrices(NULL),
	m_maxBones(0),
	m_currentContainer(NULL),
	m_center(D3DXVECTOR3(0.0f, 0.0f, 0.0f)),
	m_radius(0.0f),
	m_currentAni(0),
	m_numAnimations(0),
	m_currentTrack(0),
	m_speed(1.0f),
	m_currentTime(0.0f),
	m_transition(0.25f),
	m_device(NULL)
{

}

/*
 * @brief: �� X ģ���ļ����ص��ڴ�
 * @param: ģ���ļ���
 */
bool CXModel::LoadXFile(const char *file)
{
	if (!m_device) return false;

	CD3DAllocate alh;

	/*
	 * Load X mesh from a file.
	 *
	 * ����:
	 * X �ļ����ļ�����ָ�� X �ļ����ط�ʽ�ı�ʶ����Direct3D �豸�������ڷ���֡�������������ࡢ
	 * ��������û����ݵĽӿ�(��NULL)��ҪΪģ�ʹ����ĸ�֡��Ҫ�����Ķ���������
	 */
	if (FAILED(D3DXLoadMeshHierarchyFromX(file, D3DXMESH_MANAGED, m_device, &alh, NULL, &m_root, &m_animControl)))
		return false;

	// Record max number of animation sets in the X model.
	if (m_animControl)
		m_numAnimations = m_animControl->GetMaxNumAnimationSets();

	// Setup Bones.
	if (m_root)
	{
		SetupMatrices((stD3DFrameEx*)m_root, NULL);

		m_boneMatrices = new D3DXMATRIX[m_maxBones];
		ZeroMemory(m_boneMatrices, sizeof(D3DXMATRIX)*m_maxBones);

		D3DXFrameCalculateBoundingSphere(m_root, &m_center, &m_radius);
	}

	// Set initialize animation.
	SetAnimation(0);

	return true;
}

void CXModel::Shutdown()
{
	// ʹ������������ CD3DAllocate ����֡��νṹ(�����ṹ��ģ�������������)
	if (m_root)
	{
		CD3DAllocate alh;
		D3DXFrameDestroy(m_root, &alh);
		m_root = NULL;
	}

	// �ͷŶ���������
	if (m_animControl)
	{
		m_animControl->Release();
		m_animControl = NULL;
	}

	// �ͷŹ�����������
	if (m_boneMatrices)
	{
		delete[] m_boneMatrices;
		m_boneMatrices = NULL;
	}
}

/*
 * @brief: ����ģ�͵Ķ����������������ݣ�
 * @param:
	���ŵ�ʱ�� - ���ڻ���ʱ��Ķ�����
	ȷ��ģ��λ�õ�������� - ���ڽ�ģ���趨�ھ����λ�úͷ���
 */
void CXModel::Update(float time, const D3DXMATRIX *matWorld)
{
	time /= m_speed;
	m_currentTime += time;

	// ʹ�ö����������ƽ�ʱ��
	if (m_animControl)
		m_animControl->AdvanceTime(time, NULL);

	// ����ģ���е����о���
	UpdateMatrices(m_root, matWorld);

	// ѭ�����й��������õ�ģ�������ж�������չ�������
	stD3DContainerEx *pMesh = m_currentContainer;
	if (pMesh && pMesh->pSkinInfo)
	{
		unsigned int numBones = pMesh->pSkinInfo->GetNumBones();

		for (unsigned int i = 0; i < numBones; i++)
			D3DXMatrixMultiply(&m_boneMatrices[i], pMesh->pSkinInfo->GetBoneOffsetMatrix(i), pMesh->m_boneMatrices[i]);

		// ����ģ�͵Ķ��㻺��
		void *srcPtr;
		pMesh->MeshData.pMesh->LockVertexBuffer(D3DLOCK_READONLY, (void**)&srcPtr);

		void *destPtr;
		pMesh->m_orignalMesh->LockVertexBuffer(0, (void**)&destPtr);

		// ֪ͨ Direct3D ���¶��㻺���е�ģ��. Direct3D ��ʹ�øú������� CPU �ϵ�ģ�ͣ������µĶ���ģ���滻�ɶ���ģ��
		pMesh->pSkinInfo->UpdateSkinnedMesh(m_boneMatrices, NULL, srcPtr, destPtr);

		pMesh->m_orignalMesh->UnlockVertexBuffer();

		pMesh->MeshData.pMesh->UnlockVertexBuffer();
	}
}

/*
 * @brief: ����Ļ����Ⱦ��������ģ��
 */
void CXModel::Render()
{
	// �ݹ���Ⱦģ�͵�ÿһ֡�������Ƴ����ڸ�ģ�͵�ÿ������
	if (m_root)
		DrawFrame(m_root);
}

/*
 * @brief: ������Ҫ���ŵĶ�������, 0�����һ����������, 1����ڶ���. �����ж���������������ļ��зֿ���ͬ�Ķ���.
 * @param:
 * @return:
 */
void CXModel::SetAnimation(unsigned int index)
{
	if (index >= m_numAnimations || index == m_currentAni)
		return;

	m_currentAni = index;

	// ��ȡ��ǰ���ڴ���Ķ���
	LPD3DXANIMATIONSET set;
	m_animControl->GetAnimationSet(m_currentAni, &set);

	unsigned long nextTrack = (m_currentTrack == 0 ? 1 : 0);

	// Set next track. ������һ������·��
	m_animControl->SetTrackAnimationSet(nextTrack, set);
	set->Release();

	// Take way all tracks.
	// ����·���ϵ������¼�(��ɾ�������е������¼�)��Ϊ��ʹ�õ�ǰ����һ��·��
	m_animControl->UnkeyAllTrackEvents(m_currentTrack);
	m_animControl->UnkeyAllTrackEvents(nextTrack);

	// Key current track.
	// �ӵ�ǰ�������ϵ���һ���������ϵ�ƽ��ת��

	// Ϊ����������Ҫ·��
	m_animControl->KeyTrackEnable(m_currentTrack, FALSE, m_currentTime + m_transition);
	// ������Ҫ·���¼����ٶ�
	m_animControl->KeyTrackSpeed(m_currentTrack, 0.0f, m_currentTime, m_transition, D3DXTRANSITION_LINEAR);
	// ����·��Ȩ�ء����Ȩ�ؽ����е�·���鲢��һ��
	m_animControl->KeyTrackWeight(m_currentTrack, 0.0f, m_currentTime, m_transition, D3DXTRANSITION_LINEAR);

	// Key next track.
	m_animControl->SetTrackEnable(nextTrack, TRUE);
	m_animControl->KeyTrackSpeed(nextTrack, 1.0f, m_currentTime, m_transition, D3DXTRANSITION_LINEAR);
	m_animControl->KeyTrackWeight(nextTrack, 1.0f, m_currentTime, m_transition, D3DXTRANSITION_LINEAR);

	m_currentTrack = nextTrack;
}

/*
 * @brief: ��������һ���������ϡ�����Ѿ�����β�����ؿ�ʼ
 * @param:
 * @return:
 */
void CXModel::NextAnimation()
{
	unsigned int newAnimationSet = m_currentAni + 1;

	if (newAnimationSet >= m_numAnimations)
		m_numAnimations = 0;

	SetAnimation(newAnimationSet);
}

/*
 * @brief: ����ģ���е����й�������
 */
void CXModel::SetupMatrices(stD3DFrameEx *inFrame, LPD3DXMATRIX parentMatrix)
{
	if (!m_device) return;

	// ����������������
	stD3DContainerEx *containerEx = (stD3DContainerEx*)inFrame->pMeshContainer;
	if (containerEx)
	{
		if (!m_currentContainer)
			m_currentContainer = containerEx;

		// ����ԭʼ����ʹ��Ƥ����Ϣ��ȡ�ܵĹ�����Ŀ��
		// ʹ�ø�����ѭ�����еĹ����������������Ƶ���������������
		if (containerEx->pSkinInfo && containerEx->MeshData.pMesh)
		{
			D3DVERTEXELEMENT9 Declaration[MAX_FVF_DECL_SIZE];

			if (FAILED(containerEx->MeshData.pMesh->GetDeclaration(Declaration)))
				return;

			containerEx->MeshData.pMesh->CloneMesh(D3DXMESH_MANAGED, Declaration, m_device, &containerEx->m_orignalMesh);

			m_maxBones = max(m_maxBones, containerEx->pSkinInfo->GetNumBones());

			for (unsigned int i = 0; i < containerEx->pSkinInfo->GetNumBones(); i++)
			{
				stD3DFrameEx *temp = (stD3DFrameEx*)D3DXFrameFind(m_root, containerEx->pSkinInfo->GetBoneName(i));
				containerEx->m_boneMatrices[i] = &temp->m_finalMatrix;
			}
		}
	}

	// �ݹ鴦���ֵ�ָ��
	if (inFrame->pFrameSibling)
		SetupMatrices((stD3DFrameEx*)inFrame->pFrameSibling, parentMatrix);

	// �ݹ鴦����ָ��
	if (inFrame->pFrameFirstChild)
		SetupMatrices((stD3DFrameEx*)inFrame->pFrameFirstChild, &inFrame->m_finalMatrix);
}

/*
 * @brief:
 * @param: ֡��������
 * @return:
 */
void CXModel::UpdateMatrices(const D3DXFRAME *inFrame, const D3DXMATRIX *parentMatrix)
{
	stD3DFrameEx *frame = (stD3DFrameEx*)inFrame;

	// �и����󣬲��Ǹ��ڵ�
	if (parentMatrix != NULL)
		D3DXMatrixMultiply(&frame->m_finalMatrix, &frame->TransformationMatrix, parentMatrix);
	else
		frame->m_finalMatrix = frame->TransformationMatrix;

	if (frame->pFrameSibling != NULL)
		UpdateMatrices(frame->pFrameSibling, parentMatrix);

	if (frame->pFrameFirstChild != NULL)
		UpdateMatrices(frame->pFrameFirstChild, &frame->m_finalMatrix);
}

/*
 * @brief: ʹ�õݹ��㷨��֡��νṹ�л��Ƶ�֡
 * @param:
 * @return:
 */
void CXModel::DrawFrame(LPD3DXFRAME frame)
{
	if (!m_device) return;

	LPD3DXMESHCONTAINER meshContainer = frame->pMeshContainer;

	// �Թ��ɸ�֡��������������
	while (meshContainer)
	{
		stD3DFrameEx *frameEx = (stD3DFrameEx*)frame;

		// ����֡�ı任���������������
		m_device->SetTransform(D3DTS_WORLD, &frameEx->m_finalMatrix);

		stD3DContainerEx *containerEx = (stD3DContainerEx*)meshContainer;

		// ѭ�������ز�
		for (unsigned int i = 0; i < containerEx->NumMaterials; i++)
		{
			// ʩ���ز�
			m_device->SetMaterial(&containerEx->m_pMat[i]);
			// ʩ������
			m_device->SetTexture(0, containerEx->m_pTextures[i]);

			// �������л��Ƴ�����
			LPD3DXMESH pDrawMesh;

			if (containerEx->pSkinInfo)
				pDrawMesh = containerEx->m_orignalMesh;
			else
				pDrawMesh = containerEx->MeshData.pMesh;

			pDrawMesh->DrawSubset(i);
		}

		meshContainer = meshContainer->pNextMeshContainer;
	}

	if (frame->pFrameSibling != NULL)
		DrawFrame(frame->pFrameSibling);

	if (frame->pFrameFirstChild != NULL)
		DrawFrame(frame->pFrameFirstChild);
}
