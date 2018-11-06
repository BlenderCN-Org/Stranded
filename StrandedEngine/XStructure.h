#ifndef __X_STRUCTURE_H__
#define __X_STRUCTURE_H__

#include <d3d9.h>
#include <d3dx9.h>

/*
 * X ģ�Ͷ���
 * �ɲ�νṹ��ɣ�ʹ�ù�������. ģ�Ͳ�νṹ�е�ÿ���������һ֡.
 * 
 */

/*
 * �洢�� X �ļ���ģ�Ͳ�νṹ�е�֡ D3DXFRAME. (��νṹ)
	typedef struct _D3DXFRAME{
		LPSTR Name;	// �ɱ����õ�����
		D3DXMATRIX TransformationMatrix;	// �任���� TransformationMatrix �����ԭʼ���������ı任
		LPD3DXMESHCONTAINER pMeshContainer;	// �����������洢��ʵ���񼯺�ͼ�εĶ���
		struct _D3DXFRAME* pFrameSibling;	// ��֡�ڵ�
		struct _D3DXFRAME* pFrameFirstChild;	// �ֵ�֡�ڵ�
	} D3DXFRAME, *LPD3DXFRAME;
 */
struct stD3DFrameEx : public D3DXFRAME
{
	D3DXMATRIX m_finalMatrix;	// �洢���յ�ת�ƾ���, ȷ������λ��
};

/*
 * �������������� D3DXMESHCONTAINER (��������)
	typedef struct _D3DXMESHCONTAINER {
		LPSTR Name;	// ������
		D3DXMESHDATA MeshData;	// ��������������
		LPD3DXMATERIAL pMaterials;	// �����ز�
		LPD3DXEFFECTINSTANCE pEffects;	// Ч��ʵ��
		DWORD NumMaterials;
		DWORD* pAdjacency;	// ����������
		LPD3DXSKININFO pSkinInfo;	// Ƥ����Ϣ
		struct _D3DXMESHCONTAINER* pNextMeshContainer;	// ָ����һ���������������ָ��
	} D3DXMESHCONTAINER, *LPD3DXMESHCONTAINER;
 */
struct stD3DContainerEx : public D3DXMESHCONTAINER
{
	LPDIRECT3DTEXTURE9	*m_pTextures;		// ��������(�ɶ�ģ��ʩ������)
	D3DMATERIAL9		*m_pMat;			// �ز�(��ʩ�ӹ���)
	LPD3DXMESH			m_orignalMesh;		// ԭʼ����(ģ��)
	D3DXMATRIX			**m_boneMatrices;	// ����(��������ģ��)
};

/*
 * ID3DXAllocateHierarchy: �� API �з���ģ��
 * ��������ڴ�ռ䣬Direct3D�ݴ˽� X ģ�ͼ��ص��ڴ棬ʹ���� X ģ�ͺ��ͷ�������ͬ�ڴ�
 * �����಻Ҫ�ֶ��������غ������� Direct3D ����
 */
class CD3DAllocate : public ID3DXAllocateHierarchy
{
public:
	// ����֡
	STDMETHOD(CreateFrame)(THIS_ LPCSTR name, LPD3DXFRAME *outFrame);

	// ������������
	STDMETHOD(CreateMeshContainer)(THIS_ LPCSTR name, CONST D3DXMESHDATA *mesh, CONST D3DXMATERIAL *mats, CONST D3DXEFFECTINSTANCE *effects, DWORD numMats, CONST DWORD *indices, LPD3DXSKININFO skin, LPD3DXMESHCONTAINER *outContainer);

	// ����֡
	STDMETHOD(DestroyFrame)(THIS_ LPD3DXFRAME inFrame);
	// ������������
	STDMETHOD(DestroyMeshContainer)(THIS_ LPD3DXMESHCONTAINER container);
};

#endif
