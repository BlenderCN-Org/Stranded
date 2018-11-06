#ifndef __X_MESH_H__
#define __X_MESH_H__

#include "XStructure.h"

class CXModel
{
public:
	CXModel();
	~CXModel() { Shutdown(); }

	void SetDevice(LPDIRECT3DDEVICE9 device) { m_device = device; }

	bool LoadXFile(const char *file);
	void Shutdown();

	/*
	 * @brief: ��ģ�͵ı߽��򷵻ظ������еĵ�ַ
	 * @param: ��������λ��, ������뾶
	 * �ڹ����ǽо���ʱ��ʹ�û���ģ�ͱ߽������Ϣ�����������λ�ã����Լ��ض����Գߴ�ͱ���������ģ�ͣ����ص���������Ƿ��Զ�����
		// Define camera information.
		D3DXVECTOR3 cameraPos(0.0f, center.y - (radius * 3), 100.0f);
		D3DXVECTOR3 lookAtPos = center - cameraPos;
		D3DXVec3Normalize(&lookAtPos, &lookAtPos);
		D3DXVECTOR3 upDir(0.0f, 1.0f, 0.0f);

		// Build view matrix.
		D3DXMatrixLookAtLH(&g_ViewMatrix, &cameraPos, &lookAtPos, &upDir);
	 */
	void GetBoundingSphere(D3DXVECTOR3 *center, float *radius)
	{
		if (center)
			*center = m_center;
		if (radius)
			*radius = m_radius;
	}

	void Update(float time, const D3DXMATRIX *matWorld);

	void Render();

	void SetAnimation(unsigned int index);
	void NextAnimation();

	unsigned int GetCurrentAnimation() { return m_currentAni; }
	unsigned int GetTotalAnimation() { return m_numAnimations; }

	float GetAnimationSpeed() { return m_speed; }
	void IncreaseAnimationSpeed(float amt) { m_speed += amt; }
	void DecreaseAnimationSpeed(float amt)
	{
		if (m_speed > 0.1f)
			m_speed -= amt;
	}

	float GetTransitionSpeed() { return m_transition; }
	void DecreaseTransitionSpeed(float amt)
	{
		m_transition += amt;
	}
	void IncreaseTransitionSpeed(float amt)
	{
		if (m_transition > 0.1f)
			m_transition -= amt;
	}

private:
	void SetupMatrices(stD3DFrameEx *inFrame, LPD3DXMATRIX parentMatrix);
	void UpdateMatrices(const D3DXFRAME *inFrame, const D3DXMATRIX *parentMatrix);
	void DrawFrame(LPD3DXFRAME frame);

private:
	// D3D Device
	LPDIRECT3DDEVICE9 m_device;

	// Root frame and animation controller.
	LPD3DXFRAME m_root;
	LPD3DXANIMATIONCONTROLLER m_animControl;

	// Bones.
	D3DXMATRIX *m_boneMatrices;
	unsigned int m_maxBones;
	
	// Current mesh container.
	stD3DContainerEx *m_currentContainer;

	// Bounding sphere.
	D3DXVECTOR3 m_center;
	float m_radius;

	// Animation variables.
	unsigned int m_currentAni;
	unsigned int m_numAnimations;
	unsigned long m_currentTrack;
	float m_speed;
	float m_currentTime;
	float m_transition;
};

#endif
