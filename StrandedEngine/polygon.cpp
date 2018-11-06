#include "polygon.h"

/*
 * @brief: 以多边形对象为参数，将其数据复制到当前多边形中
 */
void CPolygon::Copy(CPolygon &pol)
{
	m_vertexList[0] = pol.m_vertexList[0];
	m_vertexList[1] = pol.m_vertexList[1];
	m_vertexList[2] = pol.m_vertexList[2];
}

/*
 * @brief: 将信息传递给射线类，并测试多边形与射线是否存在重叠的点。
 * @return: 如果存在，返回true
 */
bool CPolygon::Intersect(CRay &ray, bool cull, float *dist)
{
	if (!ray.Intersect(m_vertexList[0], m_vertexList[1], m_vertexList[2], false, dist))
		return false;

	if (cull)
	{
		if (!ray.Intersect(m_vertexList[2], m_vertexList[1], m_vertexList[0], false, dist))
			return false;
	}

	return true;
}
