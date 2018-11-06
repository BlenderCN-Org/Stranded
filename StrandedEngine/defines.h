/*
* ��Ϸ�����ö�ٱ���
*/

#ifndef __DEFINES_H__
#define __DEFINES_H__

/*
 * Ĭ�ϵ�C��C++����Լ��__cdecl [�������� ��������ѹ��]
 * WINAPI/CALLBACK __stdcall(WIN API�ĵ���Լ��) [��������ѹջ]
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

// ͼԪ����
enum PrimType
{
	NULL_TYPE,
	POINT_LIST,		// D3DPT_POINTLIST ��
	TRIANGLE_LIST,	// D3DPT_TRIANGLELIST ��ɢ������
	TRIANGLE_STRIP,	// D3DPT_TRIANGLESTRIP ����������
	TRIANGLE_FAN,	// D3DPT_TRIANGLEFAN һ������صĶ��������
	LINE_LIST,		// D3DPT_LINE �߶�
	LINE_STRIP		// D3DPT_LINESTRIP ���ӵ��߶�
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

//�������úͽ���͸������Ⱦ״̬������͸������Ⱦ״̬
//ָ�������˲������˲������͵Ĺ���
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
	TRANS_SRCALPHA,			// ��������Ⱦ�Ķ����Դ��ϲ���(D3DRS_SRCBLEND)����ΪԴ�� Alpha ͨ��(ʹ��ͼ��򶥵���ɫ�� Alpha ֵ�����ƺ�̨�б���Ļ��)
	TRANS_INVSRCALPHA,
	TRANS_DSTALPHA,
	TRANS_INVDSTALPHA,		// ��ǰ��Ļ���ݣ���������ʹ��Դ������� alpha (Direct3D �ڽ����������ϵ�һ��ʱ�� Alpha �ɷ�����ȡһ��ֵ)
	TRANS_DSTCOLOR,
	TRANS_INVDSTCOLOR,
	TRANS_SRCALPHASAT,
	TRANS_BOTHSRCALPHA,
	TRANS_INVBOTHSRCALPHA,
	TRANS_BLENDFACTOR,
	TRANS_INVBLENDFACTOR
};

// �˲�ģʽ
enum TextureState
{
	MIN_FILTER,			// min(��С��)
	MAG_FILTER,			// mag(�Ŵ���)
	MIP_FILTER			// mip(mipmap)
};

enum FilterType
{
	POINT_TYPE,			// �����򵥣�ʹ�ö�Ӧ�ڶ����������������������ؽ�����ӳ�䵽������
	LINEAR_TYPE,		// ˫���������˲�, ����2*2�����о��������4��������������ؽ����˲�(�����ٶȲ��죬ͼ���������ã�������͹۲���֮���м���ĽǶȱ仯ʱ������ֿ��-����ͬ��ʧ��)
	ANISOTROPIC_TYPE	// ����ͬ���˲��ɱ�����ֿ�ߣ�������Ⱦ����ͼ��ʱ�ܻ�ȡ��ѵ�����(�����ٶ�����)
};

#define PERCENT_OF(a, b) (int)(a * b)

// Vertex formats.
// ��Ⱦ GUI ����ؼ�
#define GUI_FVF		2
#define MV_FVF		3

// Type of controls we support.
#define STRANDED_GUI_STATICTEXT	1		// ��̬�ı�
#define STRANDED_GUI_BUTTON		2		// ��ť
#define STRANDED_GUI_BACKDROP	3		// ����ͼ

// Button states.
#define STRANDED_BUTTON_UP			1
#define STRANDED_BUTTON_OVER		2
#define STRANDED_BUTTON_DOWN		3

#define STRANDED_LEFT_BUTTON		0
#define STRANDED_RIGHT_BUTTON		1

///////////////////////������Ч///////////////////////////////
/*
 * ȫ���������һ�ּ�������Ⱦ�����ʱ�������Ĳβ��Ч���ķ���. ���������Ҳ�����Ⱦ�������������������Ӱ��,
 * �������ڴ�����ʱ�������. ͨ��ʵ�ֶ������ϵ�ģ������������Ļ����ʾ��һ��ƽ�������ʵ��ȫ�������.
 */
// Multi-sampling.
enum STRANDED_MS_TYPE
{
	STRANDED_MS_NONE,				// �Ƕ����
	STRANDED_MS_SAMPLES_2,			// 2��
	STRANDED_MS_SAMPLES_4,			// 4�����ʲ��� 4x AA
	STRANDED_MS_SAMPLES_8,			// 8��
	STRANDED_MS_SAMPLES_16			// 16��
};

/*
 * ������Ӵ�����ʵ�У�����Ե��Ρ����ֵ����Լ������Ļ��ⳡ��.
 * ���ƹ۲��߶Ի����Ŀɼ��ȣ�ͬ�������˷��͸��ܵ��Ķ������Ŀ�������Ⱦʱ��
 * ����ͨ�����رȶ���Ҫ��ö࣬��ȣ����������Ⱦ�ٶȽ���. �������ṩ�ܺõ��Ӿ�Ч���������ڱ���������ڲ崦�������Ϣ.
 */
// Fog.
enum STRANDED_FOG_TYPE
{
	STRANDED_VERTEX_FOG,			// ������[����ÿ�������������������]
	STRANDED_PIXEL_FOG				// ������[�������ؼ�������������]
};
//////////////////////////////////////////////////////////////

#define KEYS_SIZE	256

#endif