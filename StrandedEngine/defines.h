/*
* 游戏定义和枚举变量
*/

#ifndef __DEFINES_H__
#define __DEFINES_H__

/*
 * 默认的C、C++调用约定__cdecl [函数参数 从右往左压入]
 * WINAPI/CALLBACK __stdcall(WIN API的调用约定) [从左向右压栈]
 */
#include <Windows.h>
#include <time.h>

#define STRANDED_INVALID	-1
#define STRANDED_OK			1
#define STRANDED_FAIL		0

// Light type defines.
#define LIGHT_POINT			1	// D3DLIGHT_POINT
#define LIGHT_DIRECTIONAL	2	// D3DLIGHT_DIRECTION
#define LIGHT_SPOT			3	// D3DLIGHT_SPOT

// Window handle (need new way if porting to Mac and OpenGL).
#define WinHWND HWND

// Typedefs and enumerations.
typedef long VertexType;

// 图元类型
enum PrimType
{
	NULL_TYPE,
	POINT_LIST,		// D3DPT_POINTLIST 点
	TRIANGLE_LIST,	// D3DPT_TRIANGLELIST 离散三角形
	TRIANGLE_STRIP,	// D3DPT_TRIANGLESTRIP 相连三角形
	TRIANGLE_FAN,	// D3DPT_TRIANGLEFAN 一个点相关的多个三角形
	LINE_LIST,		// D3DPT_LINE 线段
	LINE_STRIP		// D3DPT_LINESTRIP 连接的线段
};

enum E_ROTATION_DIR
{
	ROTATION_X,
	ROTATION_Y,
	ROTATION_Z
};

// Color defines.
#define STRANDED_COLOR_ARGB(a, r, g, b)	((unsigned long)((((a)&0xff)<<24)|\
									(((r)&0xff<<16)|((g)&0xff)<<8)|\
									((b)&0xff)))

//增加启用和禁用透明度渲染状态、设置透明度渲染状态
//指明纹理滤波器和滤波器类型的功能
enum RenderState
{
	CULL_NONE,
	CULL_CW,
	CULL_CCW,
	DEPTH_NONE,
	DEPTH_READONLY,
	DEPTH_READWRITE,
	SHADE_POINTS,
	SHADE_SOLIDTRI,
	SHADE_WIRETRI,
	SHADE_WIREPOLY,
	TRANSPARENCY_NONE,
	TRANSPARENCY_ENABLE
};

enum TransState
{
	TRANS_ZERO = 1,
	TRANS_ONE,
	TRANS_SRCCOLOR,
	TRANS_INVSRCCOLOR,
	TRANS_SRCALPHA,			// 将正在渲染的对象的源混合操作(D3DRS_SRCBLEND)设置为源的 Alpha 通道(使用图像或顶点颜色的 Alpha 值来控制后台中表面的混合)
	TRANS_INVSRCALPHA,
	TRANS_DSTALPHA,
	TRANS_INVDSTALPHA,		// 当前屏幕内容，设置用来使用源对象的逆 alpha (Direct3D 在将两个结果混合到一起时从 Alpha 成分中提取一个值)
	TRANS_DSTCOLOR,
	TRANS_INVDSTCOLOR,
	TRANS_SRCALPHASAT,
	TRANS_BOTHSRCALPHA,
	TRANS_INVBOTHSRCALPHA,
	TRANS_BLENDFACTOR,
	TRANS_INVBLENDFACTOR
};

// 滤波模式
enum TextureState
{
	MIN_FILTER,			// min(缩小率)
	MAG_FILTER,			// mag(放大率)
	MIP_FILTER			// mip(mipmap)
};

enum FilterType
{
	POINT_TYPE,			// 最快最简单，使用对应于顶点纹理坐标的最近纹理像素将纹理映射到表面上
	LINEAR_TYPE,		// 双线性纹理滤波, 基于2*2区域中距离采样点4个最近的纹理像素进行滤波(计算速度不快，图像质量更好，当表面和观察者之间有急剧的角度变化时，会出现块斑-各向同性失真)
	ANISOTROPIC_TYPE	// 各向同性滤波可避免出现块斑，且在渲染纹理图像时能获取最佳的质量(计算速度最慢)
};

#define PERCENT_OF(a, b) (int)(a * b)

// Vertex formats.
// 渲染 GUI 顶点控件
#define GUI_FVF		2
#define MV_FVF		3

// Type of controls we support.
#define STRANDED_GUI_STATICTEXT	1		// 静态文本
#define STRANDED_GUI_BUTTON		2		// 按钮
#define STRANDED_GUI_BACKDROP	3		// 背景图

// Button states.
#define STRANDED_BUTTON_UP			1
#define STRANDED_BUTTON_OVER		2
#define STRANDED_BUTTON_DOWN		3

#define STRANDED_LEFT_BUTTON		0
#define STRANDED_RIGHT_BUTTON		1

///////////////////////增加特效///////////////////////////////
/*
 * 全景抗混叠是一种减少在渲染多边形时所产生的参差不齐效果的方法. 抗混叠物体也会对渲染后的整个场景产生负面影响,
 * 尤其是在处理动画时更是如此. 通过实现对像素上的模糊处理，并在屏幕上显示出一个平均结果而实现全景抗混叠.
 */
// Multi-sampling.
enum STRANDED_MS_TYPE
{
	STRANDED_MS_NONE,				// 非多采样
	STRANDED_MS_SAMPLES_2,			// 2倍
	STRANDED_MS_SAMPLES_4,			// 4倍速率采样 4x AA
	STRANDED_MS_SAMPLES_8,			// 8倍
	STRANDED_MS_SAMPLES_16			// 16倍
};

/*
 * 雾可增加大量真实感，尤其对地形、树林地区以及幽灵般的户外场景.
 * 限制观察者对环境的可见度，同样限制了发送给管道的多边形数目，提高渲染时间
 * 由于通常像素比顶点要多得多，相比，像素雾的渲染速度较慢. 顶点雾提供很好的视觉效果，可以在表面上逐点内插处理雾的信息.
 */
// Fog.
enum STRANDED_FOG_TYPE
{
	STRANDED_VERTEX_FOG,			// 顶点雾[按照每个顶点来计算雾的数据]
	STRANDED_PIXEL_FOG				// 像素雾[根据像素级别计算雾的数据]
};
//////////////////////////////////////////////////////////////

#define KEYS_SIZE	256

#endif