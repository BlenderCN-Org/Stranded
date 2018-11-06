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
 * @brief: 将 X 模型文件加载到内存
 * @param: 模型文件名
 */
bool CXModel::LoadXFile(const char *file)
{
	if (!m_device) return false;

	CD3DAllocate alh;

	/*
	 * Load X mesh from a file.
	 *
	 * 参数:
	 * X 文件的文件名、指定 X 文件加载方式的标识符、Direct3D 设备对象、用于分配帧和网格容器的类、
	 * 允许加载用户数据的接口(可NULL)、要为模型创建的根帧、要创建的动画控制器
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
	// 使用派生分配类 CD3DAllocate 销毁帧层次结构(派生结构：模型镇和网格容器)
	if (m_root)
	{
		CD3DAllocate alh;
		D3DXFrameDestroy(m_root, &alh);
		m_root = NULL;
	}

	// 释放动画控制器
	if (m_animControl)
	{
		m_animControl->Release();
		m_animControl = NULL;
	}

	// 释放骨骼矩阵链表
	if (m_boneMatrices)
	{
		delete[] m_boneMatrices;
		m_boneMatrices = NULL;
	}
}

/*
 * @brief: 更新模型的动画（整个网格数据）
 * @param:
	消逝的时间 - 用于基于时间的动画、
	确定模型位置的世界矩阵 - 用于将模型设定在具体的位置和方向
 */
void CXModel::Update(float time, const D3DXMATRIX *matWorld)
{
	time /= m_speed;
	m_currentTime += time;

	// 使用动画控制器推进时间
	if (m_animControl)
		m_animControl->AdvanceTime(time, NULL);

	// 更新模型中的所有矩阵
	UpdateMatrices(m_root, matWorld);

	// 循环所有骨骼，并得到模型中所有对象的最终骨骼矩阵
	stD3DContainerEx *pMesh = m_currentContainer;
	if (pMesh && pMesh->pSkinInfo)
	{
		unsigned int numBones = pMesh->pSkinInfo->GetNumBones();

		for (unsigned int i = 0; i < numBones; i++)
			D3DXMatrixMultiply(&m_boneMatrices[i], pMesh->pSkinInfo->GetBoneOffsetMatrix(i), pMesh->m_boneMatrices[i]);

		// 锁定模型的顶点缓存
		void *srcPtr;
		pMesh->MeshData.pMesh->LockVertexBuffer(D3DLOCK_READONLY, (void**)&srcPtr);

		void *destPtr;
		pMesh->m_orignalMesh->LockVertexBuffer(0, (void**)&destPtr);

		// 通知 Direct3D 更新顶点缓存中的模型. Direct3D 将使用该函数更新 CPU 上的模型，并用新的动画模型替换旧动画模型
		pMesh->pSkinInfo->UpdateSkinnedMesh(m_boneMatrices, NULL, srcPtr, destPtr);

		pMesh->m_orignalMesh->UnlockVertexBuffer();

		pMesh->MeshData.pMesh->UnlockVertexBuffer();
	}
}

/*
 * @brief: 在屏幕上渲染整个动画模型
 */
void CXModel::Render()
{
	// 递归渲染模型的每一帧，并绘制出属于该模型的每个网格
	if (m_root)
		DrawFrame(m_root);
}

/*
 * @brief: 设置想要播放的动画索引, 0代表第一个动画集合, 1代表第二个. 可以有多个动画集合以在文件中分开不同的动画.
 * @param:
 * @return:
 */
void CXModel::SetAnimation(unsigned int index)
{
	if (index >= m_numAnimations || index == m_currentAni)
		return;

	m_currentAni = index;

	// 获取当前正在处理的动画
	LPD3DXANIMATIONSET set;
	m_animControl->GetAnimationSet(m_currentAni, &set);

	unsigned long nextTrack = (m_currentTrack == 0 ? 1 : 0);

	// Set next track. 设置下一个动画路径
	m_animControl->SetTrackAnimationSet(nextTrack, set);
	set->Release();

	// Take way all tracks.
	// 解锁路径上的所有事件(即删除动画中的所有事件)，为了使用当前和下一个路径
	m_animControl->UnkeyAllTrackEvents(m_currentTrack);
	m_animControl->UnkeyAllTrackEvents(nextTrack);

	// Key current track.
	// 从当前动画集合到下一个动画集合的平滑转移

	// 为动画设置主要路径
	m_animControl->KeyTrackEnable(m_currentTrack, FALSE, m_currentTime + m_transition);
	// 设置主要路径事件的速度
	m_animControl->KeyTrackSpeed(m_currentTrack, 0.0f, m_currentTime, m_transition, D3DXTRANSITION_LINEAR);
	// 设置路径权重。这个权重将所有的路径归并在一起。
	m_animControl->KeyTrackWeight(m_currentTrack, 0.0f, m_currentTime, m_transition, D3DXTRANSITION_LINEAR);

	// Key next track.
	m_animControl->SetTrackEnable(nextTrack, TRUE);
	m_animControl->KeyTrackSpeed(nextTrack, 1.0f, m_currentTime, m_transition, D3DXTRANSITION_LINEAR);
	m_animControl->KeyTrackWeight(nextTrack, 1.0f, m_currentTime, m_transition, D3DXTRANSITION_LINEAR);

	m_currentTrack = nextTrack;
}

/*
 * @brief: 持续到下一个动画集合。如果已经到结尾，返回开始
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
 * @brief: 设置模型中的所有骨骼矩阵
 */
void CXModel::SetupMatrices(stD3DFrameEx *inFrame, LPD3DXMATRIX parentMatrix)
{
	if (!m_device) return;

	// 创建网格容器对象
	stD3DContainerEx *containerEx = (stD3DContainerEx*)inFrame->pMeshContainer;
	if (containerEx)
	{
		if (!m_currentContainer)
			m_currentContainer = containerEx;

		// 复制原始网格，使用皮肤信息获取总的骨骼数目。
		// 使用该数字循环所有的骨骼，并将骨骼复制到骨骼矩阵数组中
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

	// 递归处理兄弟指针
	if (inFrame->pFrameSibling)
		SetupMatrices((stD3DFrameEx*)inFrame->pFrameSibling, parentMatrix);

	// 递归处理子指针
	if (inFrame->pFrameFirstChild)
		SetupMatrices((stD3DFrameEx*)inFrame->pFrameFirstChild, &inFrame->m_finalMatrix);
}

/*
 * @brief:
 * @param: 帧、父矩阵
 * @return:
 */
void CXModel::UpdateMatrices(const D3DXFRAME *inFrame, const D3DXMATRIX *parentMatrix)
{
	stD3DFrameEx *frame = (stD3DFrameEx*)inFrame;

	// 有父矩阵，不是跟节点
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
 * @brief: 使用递归算法从帧层次结构中绘制单帧
 * @param:
 * @return:
 */
void CXModel::DrawFrame(LPD3DXFRAME frame)
{
	if (!m_device) return;

	LPD3DXMESHCONTAINER meshContainer = frame->pMeshContainer;

	// 对构成该帧的所有容器处理
	while (meshContainer)
	{
		stD3DFrameEx *frameEx = (stD3DFrameEx*)frame;

		// 根据帧的变换矩阵设置世界矩阵
		m_device->SetTransform(D3DTS_WORLD, &frameEx->m_finalMatrix);

		stD3DContainerEx *containerEx = (stD3DContainerEx*)meshContainer;

		// 循环所有素材
		for (unsigned int i = 0; i < containerEx->NumMaterials; i++)
		{
			// 施用素材
			m_device->SetMaterial(&containerEx->m_pMat[i]);
			// 施加纹理
			m_device->SetTexture(0, containerEx->m_pTextures[i]);

			// 在容器中绘制出网格
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
