#ifndef __STRANDED_PLANE_H__
#define __STRANDED_PLANE_H__

/*
 * 平面
 * typedef struct D3DXPLANE {
	FLOAT a;
	FLOAT b;
	FLOAT c;
	FLOAT d;
	} D3DXPLANE;
 * D3DXPlaneDot() - 计算平面和 4D 矢量点积
 * D3DXPlaneDotCoord() - 计算平面和 3D 矢量点积
 * D3DXPlaneDotCoord() - 使用假定 w 为 0 计算平面和 3D 矢量点积
 * D3DXPlaneFromPoints() - 由三角形计算平面
 * D3DXPlaneFromPointNormal() - 由点和法线计算平面
 * 常用于几何图形选择、碰撞检则
 * V*N+D=0
 * D: 平面到原点的距离，V: 位于平面某个位置上的点(该点可在平面的任意位置上)，N: 平面法线(平面法线与平面正交朝向平面前面的方向)
 */

#include "mathLibrary.h"

class CPlane
{
public:
	CPlane();
	CPlane(float A, float B, float C, float D);
	CPlane(CVector3 &n, float D);

	// 由三角形创建平面
	void CreatePlaneFromTri(CVector3 &t1, CVector3 &t2, CVector3 &t3);

	// 手动设置平面数据
	void SetPlaneNormal(float A, float B, float C)
	{
		a = A; b = B; c = C;
	}

	void SetPlaneIntercept(float D) { d = D; }

	// 测试三个平面的交叉点
	bool Intersect(CPlane &p2, CPlane &p3, CVector3 *intersectPoint);
	bool Intersect(CPlane &p1, CVector3 *intersectPoint);
	bool Intersect(CPlane &p1, CRay *intersect);

	//////////////////////////对物体进行分类，计算平面之间的内插////////////////////////////////////////////////
	// 根据平面测试点相对于平面的位置[点在平面上、平面后还是平面前]
	int ClassifyPoint(CVector3& v);
	// 根据平面测试多边形相对于平面的位置[位于哪一侧]
	int ClassifyPolygon(CPolygon &pol);
	int ClassifyPoint(float x, float y, float z);

	// 实现两个平面对象之间的线性内插
	void Lerp(CPlane &p1, CPlane &p2, float amount);

	// 计算平面到一个3D矢量的距离
	float GetDistance(CVector3& v);
	float GetDistance(float x, float y, float z);

public:
	// 平面法线(abc) 平面距离(d)
	float a, b, c, d;
};

#endif