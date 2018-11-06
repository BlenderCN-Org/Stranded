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
 * @brief: ������������Ϣ����ƽ�淨�ߺ;���
 * @param: ���������ε���������
 */
void CPlane::CreatePlaneFromTri(CVector3 &t1, CVector3 &t2, CVector3 &t3)
{
	// �������ι�һ��
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
 * @brief: ��������ƽ��֮��Ľ����
 * @return: ������ڽ���㣬����true����������㷵�ظ� intersectPoint
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
 * @brief: ����㺯��, ��������ƽ��֮���Ƿ��ཻ
 * @param: �ڶ���ƽ��������Ҫ��ƽ���ཻ��Ҫ�洢������ʸ�������ϵĵ�
 * @return: ����ƽ���ཻ������true �� �ཻ�Ľ����
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
 * @brief: ȷ����λ��ƽ����һ�෽��: �����
 *		ͨ������ƽ���λ�õĵ��������ƽ���������ص��ƽ������λ�á�
 * @param: 3D λ��
 * @return: ����<0������ƽ����棻 >0 ��ƽ��ǰ�棻=0����ƽ����
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
 * @param: ����ƽ��Ͱٷֱ�
 * ����ٷֱ�=0��ʹ�õ�һ��ƽ�档�ٷֱ�=1��ʹ�õڶ���ƽ�档�ٷֱ�Ϊ0~1֮������ֵ��
 * ����һ����������ƽ��֮���µ��ڲ�ƽ��.
 * amount=50%��������ƽ�����м�����һ����ƽ��.
 * �ڲ����洢�ڸú��������ı��ض�����
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
 * @brief: ����ƽ���Ƿ����ԳƵı߽���ཻ
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
