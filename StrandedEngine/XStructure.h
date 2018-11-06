#ifndef __X_STRUCTURE_H__
#define __X_STRUCTURE_H__

#include <d3d9.h>
#include <d3dx9.h>

/*
 * X 模型动画
 * 由层次结构组成，使用骨骼动画. 模型层次结构中的每个网格就是一帧.
 * 
 */

/*
 * 存储在 X 文件于模型层次结构中的帧 D3DXFRAME. (层次结构)
	typedef struct _D3DXFRAME{
		LPSTR Name;	// 可被引用的名称
		D3DXMATRIX TransformationMatrix;	// 变换矩阵 TransformationMatrix 保存从原始网格所作的变换
		LPD3DXMESHCONTAINER pMeshContainer;	// 网格容器，存储真实网格集合图形的对象
		struct _D3DXFRAME* pFrameSibling;	// 子帧节点
		struct _D3DXFRAME* pFrameFirstChild;	// 兄弟帧节点
	} D3DXFRAME, *LPD3DXFRAME;
 */
struct stD3DFrameEx : public D3DXFRAME
{
	D3DXMATRIX m_finalMatrix;	// 存储最终的转移矩阵, 确定对象位置
};

/*
 * 真正的网格数据 D3DXMESHCONTAINER (单个链表)
	typedef struct _D3DXMESHCONTAINER {
		LPSTR Name;	// 网格名
		D3DXMESHDATA MeshData;	// 真正的网格数据
		LPD3DXMATERIAL pMaterials;	// 网格素材
		LPD3DXEFFECTINSTANCE pEffects;	// 效果实例
		DWORD NumMaterials;
		DWORD* pAdjacency;	// 三角形索引
		LPD3DXSKININFO pSkinInfo;	// 皮肤信息
		struct _D3DXMESHCONTAINER* pNextMeshContainer;	// 指向另一个网格容器对象的指针
	} D3DXMESHCONTAINER, *LPD3DXMESHCONTAINER;
 */
struct stD3DContainerEx : public D3DXMESHCONTAINER
{
	LPDIRECT3DTEXTURE9	*m_pTextures;		// 纹理链表(可对模型施加纹理)
	D3DMATERIAL9		*m_pMat;			// 素材(可施加光照)
	LPD3DXMESH			m_orignalMesh;		// 原始网格(模型)
	D3DXMATRIX			**m_boneMatrices;	// 骨骼(动画驱动模型)
};

/*
 * ID3DXAllocateHierarchy: 在 API 中分配模型
 * 对类分配内存空间，Direct3D据此将 X 模型加载到内存，使用完 X 模型后释放所有相同内存
 * 派生类不要手动调用重载函数，由 Direct3D 调用
 */
class CD3DAllocate : public ID3DXAllocateHierarchy
{
public:
	// 创建帧
	STDMETHOD(CreateFrame)(THIS_ LPCSTR name, LPD3DXFRAME *outFrame);

	// 创建网格容器
	STDMETHOD(CreateMeshContainer)(THIS_ LPCSTR name, CONST D3DXMESHDATA *mesh, CONST D3DXMATERIAL *mats, CONST D3DXEFFECTINSTANCE *effects, DWORD numMats, CONST DWORD *indices, LPD3DXSKININFO skin, LPD3DXMESHCONTAINER *outContainer);

	// 销毁帧
	STDMETHOD(DestroyFrame)(THIS_ LPD3DXFRAME inFrame);
	// 销毁网格容器
	STDMETHOD(DestroyMeshContainer)(THIS_ LPD3DXMESHCONTAINER container);
};

#endif
