/*
* ָ����������ͼ�Ͱ�ť�����ȫ������
*/
#ifndef __STRUCTS_H__
#define __STRUCTS_H__

// A structure for our custom vertex type
// #define D3DFVF_VERTEX (D3DFVF_XYZW | D3DFVF_DIFFUSE | D3DFVF_TEX1)
struct stGUIVertex
{
	float x, y, z, rhw;
	unsigned long color;
	/*
	 * ��������, 0.0~1.0��
	 * ���ö���������Ϣ��ʼλ�õĿ�ȱȺ͸߶ȱ�[�ڶ���α����ϲ�ͬ��֮�������ͼ�����ڲ崦��]
	 * ��������Ŀ�ʼλ����ͼ������Ͻ�(tu=0, tv=0),
	 * ����λ����ͼ������½�(tu=1, tv=1)
	 * ���½ǵ�λ��(tu=0, tv=1)
	 * ���Ͻǵ�λ��(tu=1, tv=0)
	 * ���������������ʹ������ͼ������ⲿ������,
	 * ���ֻ��ʹ��һ��������轫���������ˮƽ����ʹ�ֱ�������Ϊ0.5
	 *
	 * Multitexture ��ͼ[��ͨ����Ⱦ��Ӳ����������ͼ]
	 * #define D3DFVF_VERTEX (D3DFVF_XYZW | D3DFVF_DIFFUSE | D3DFVF_TEX2)
	 * struct stD3DVertex
	 * {
	 *		float x, y, z;
	 *		unsigned long color;
	 *		float tu, tv;
	 *		float tu2, tv2;
	 * }
	 *
	 * ������ͼ��1D��2D��3D�������������
	 * 3D��������ͼ����״ͼ����ʹ��3D��������U��V��W(tu��tv��tw)
	 */
	float tu, tv;
};

// A general structure for our models.
struct stModelVertex
{
	// x��y��z����
	float x, y, z;
	// ���㷨��
	float nx, ny, nz;
	// ������ɫֵ
	unsigned long color;
	// һ����������
	float tu, tv;
};

#endif