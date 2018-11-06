#include "plane.h"

CPlane::CPlane()
{
	a = 0;
	b = 0;
	c = 0;
	d = 0;
}

CPlane::CPlane(float A, float B, float C, float D)
{
	a = A;
	b = B;
	c = C;
	d = D;
}

CPlane::CPlane(CVector3 &n, float D)
{
	a = n.x;
	b = n.y;
	c = n.z;
	d = D;
}

/*
 * @brief: 利用三角形信息计算平面法线和距离
 * @param: 构成三角形的三个顶点
 */
void CPlane::CreatePlaneFromTri(CVector3 &t1, CVector3 &t2, CVector3 &t3)
{
	// 将三角形归一化
	// Get triangle normal.
	CVector3 normal;

	normal.CrossProduct(t3 - t1, t2 - t1);
	normal.Normal();

	// Save normal and calculate d.
	a = normal.x;
	b = normal.y;
	c = normal.z;

	d = -(a * t1.x + b * t1.y + c * t1.z);
}

/*
 * @brief: 查找三个平面之间的交叉点
 * @return: 如果存在交叉点，返回true，并将交叉点返回给 intersectPoint
 */
bool CPlane::Intersect(CPlane &p2, CPlane &p3, CVector3 *intersectPoint)
{
	CVector3 temp;
	CVector3 normal(a, b, c);
	CVector3 p2Normal(p2.a, p2.b, p2.c);
	CVector3 p3Normal(p3.a, p3.b, p3.c);

	// Get cross product of p2 and p3.
	temp.CrossProduct(p2Normal, p3Normal);

	// Get the dot prod of this plane with cross of p2 and p3.
	float dp = normal.DotProduct3(temp);
	if (dp == 0.0f)
		return false;

	// Else we must find intersect point.
	if (intersectPoint)
	{
		*intersectPoint = temp * d;

		temp.CrossProduct(p3Normal, normal);
		*intersectPoint = *intersectPoint + (temp * p2.d);

		temp.CrossProduct(normal, p2Normal);
		*intersectPoint = *intersectPoint + (temp * p3.d);

		*intersectPoint = *intersectPoint / -dp;
	}

	return true;
}

/*
 * @brief: 交叉点函数, 测试两个平面之间是否相交
 * @param: 第二个平面对象、如果要和平面相交将要存储交叉点的矢量对象上的点
 * @return: 两个平面相交，返回true 及 相交的交叉点
 */
bool CPlane::Intersect(CPlane &p1, CVector3 *intersectPoint)
{
	CVector3 cross;
	CVector3 normal(a, b, c);
	CVector3 p1Normal(p1.a, p1.b, p1.c);
	float length = 0.0f;

	cross.CrossProduct(normal, p1Normal);

	length = cross.DotProduct3(cross);
	if (length < 1e-08f)
		return false;

	if (intersectPoint)
	{
		float l0 = normal.DotProduct3(normal);
		float l1 = normal.DotProduct3(p1Normal);
		float l2 = p1Normal.DotProduct3(p1Normal);

		float det = l0 * l2 - l1 * l1;
		if (fabs(det) < 1e-08f)
			return false;

		float invDet = 0;
		invDet = 1 / det;

		float d0 = (l2 * d - l1 * p1.d) * invDet;
		float d1 = (l0 * p1.d - l1 * d) * invDet;

		(*intersectPoint) = normal * d0 + p1Normal * d1;
	}

	return true;
}


bool CPlane::Intersect(CPlane &pl, CRay *intersect)
{
	CVector3 cross;
	CVector3 normal(a, b, c);
	CVector3 plNormal(pl.a, pl.b, pl.c);

	float length = 0.0f;

	cross.CrossProduct(normal, plNormal);
	length = cross.DotProduct3(cross);

	if (length < 1e-08f) return false;

	if (intersect)
	{
		float l0 = normal.DotProduct3(normal);
		float l1 = normal.DotProduct3(plNormal);
		float l2 = plNormal.DotProduct3(plNormal);

		float det = l0 * l2 - l1 * l1;
		float invDet = 0;

		if (fabs(det) < 1e-08f) return false;

		invDet = 1 / det;

		float d0 = (l2 * d - l1 * pl.d) * invDet;
		float d1 = (l0 * pl.d - l1 * d) * invDet;

		(*intersect).m_origin = normal * d0 + plNormal * d1;
		(*intersect).m_direction = cross;
	}

	return true;
}

/*
 * @brief: 确定点位于平面哪一侧方法: 分类点
 *		通过计算平面和位置的点积，加上平面距离而返回点和平面的相对位置。
 * @param: 3D 位置
 * @return: 距离<0，点在平面后面； >0 在平面前面；=0，在平面上
 */
int CPlane::ClassifyPoint(CVector3& v)
{
	float distance = a * v.x + b * v.y + c * v.z + d;

	if (distance > 0.001f)
		return STRANDED_FRONT;

	if (distance < -0.001f)
		return STRANDED_BACK;

	return STRANDED_ON_PLANE;
}

int CPlane::ClassifyPoint(float x, float y, float z)
{
	float distance = a * x + b * y + c * z + d;

	if (distance > 0.001f)
		return STRANDED_FRONT;

	if (distance < -0.001f)
		return STRANDED_BACK;

	return STRANDED_ON_PLANE;
}

/*
 * @param: 两个平面和百分比
 * 如果百分比=0，使用第一个平面。百分比=1，使用第二个平面。百分比为0~1之间任意值，
 * 生成一个介于两个平面之间新的内插平面.
 * amount=50%，在两个平面正中间生成一个新平面.
 * 内插结果存储在该函数所属的本地对象中
 */
void CPlane::Lerp(CPlane &p1, CPlane &p2, float amount)
{
	// First calculate a, b, c, d.
	a = p1.a * (1.0f - amount) + p2.a * amount;
	b = p1.b * (1.0f - amount) + p2.b * amount;
	c = p1.c * (1.0f - amount) + p2.c * amount;
	d = p1.d * (1.0f - amount) + p2.d * amount;

	// Re-normalize plane.
	float length = (float)sqrt(a * a + b * b + c * c);
	if (length == 0.0f)
		length = 1.0f;
	length = 1 / length;

	// Normalize all but d.
	a = a * length;
	b = b * length;
	c = c * length;
}

int CPlane::ClassifyPolygon(CPolygon &pol)
{
	int frontPolys = 0;
	int backPolys = 0;
	int planePolys = 0;
	int type = 0;

	for (int i = 0; i < 3; i++)
	{
		type = ClassifyPoint(pol.m_vertexList[i]);
		switch (type)
		{
		case STRANDED_FRONT:
			frontPolys++;
			break;
		case STRANDED_BACK:
			backPolys++;
			break;
		default:
			frontPolys++;
			backPolys++;
			planePolys++;
			break;
		}
	}

	if (planePolys == 3)
		return STRANDED_ON_PLANE;
	else if (frontPolys == 3)
		return STRANDED_FRONT;
	else if (backPolys == 3)
		return STRANDED_BACK;

	return STRANDED_CLIPPED;
}

float CPlane::GetDistance(CVector3& v)
{
	return a * v.x + b * v.y + c * v.z + d;
}

float CPlane::GetDistance(float x, float y, float z)
{
	return a * x + b * y + c * z + d;
}

/*
 * @brief: 测试平面是否和轴对称的边界框相交
 */
/*bool CPlane::Intersect(CAabb &aabb)
{
	CVector3 min, max;
	CVector3 normal(a, b, c);

	if (normal.x >= 0.0f)
	{
		min.x = aabb.m_min.x;
		max.y = aabb.m_max.x;
	}
	else
	{
		min.x = aabb.m_max.x;
		max.x = aabb.m_min.x;
	}

	if (normal.y >= 0.0f)
	{
		min.y = aabb.m_min.y;
		max.y = aabb.m_max.y;
	}
	else
	{
		min.y = aabb.m_max.y;
		max.y = aabb.m_min.y;
	}

	if (normal.z >= 0.0f)
	{
		min.z = aabb.m_min.z;
		max.z = aabb.m_max.z;
	}
	else
	{
		min.z = aabb.m_max.z;
		max.z = aabb.m_min.z;
	}

	if ((normal.DotProduct3(min) + d) > 0.0f)
		return false;

	if ((normal.DotProduct3(max) + d) >= 0.0f)
		return true;

	return false;
}*/
