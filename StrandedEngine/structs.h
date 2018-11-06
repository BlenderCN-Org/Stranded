/*
* 指定创建背景图和按钮所需的全部数据
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
	 * 纹理坐标, 0.0~1.0，
	 * 设置顶点纹理信息起始位置的宽度比和高度比[在多边形表面上不同点之间对纹理图像做内插处理]
	 * 纹理坐标的开始位置在图像的左上角(tu=0, tv=0),
	 * 结束位置在图像的右下角(tu=1, tv=1)
	 * 左下角的位置(tu=0, tv=1)
	 * 右上角的位置(tu=1, tv=0)
	 * 借助纹理坐标可以使用纹理图像的任意部分内容,
	 * 如果只想使用一半的纹理，需将纹理坐标的水平方向和垂直方向各设为0.5
	 *
	 * Multitexture 贴图[多通道渲染、硬件多纹理贴图]
	 * #define D3DFVF_VERTEX (D3DFVF_XYZW | D3DFVF_DIFFUSE | D3DFVF_TEX2)
	 * struct stD3DVertex
	 * {
	 *		float x, y, z;
	 *		unsigned long color;
	 *		float tu, tv;
	 *		float tu2, tv2;
	 * }
	 *
	 * 纹理贴图：1D、2D、3D、球体和立方体
	 * 3D、立方体图或球状图纹理，使用3D纹理坐标U、V、W(tu、tv和tw)
	 */
	float tu, tv;
};

// A general structure for our models.
struct stModelVertex
{
	// x、y、z坐标
	float x, y, z;
	// 顶点法线
	float nx, ny, nz;
	// 顶点颜色值
	unsigned long color;
	// 一对纹理坐标
	float tu, tv;
};

#endif