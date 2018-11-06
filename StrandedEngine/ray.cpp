#include "ray.h"

/*
 * @brief: 相交测试
 * @return: true-相交, false，表示射线并未击中该对象
 */
bool CRay::Intersect(CVector3 &pos, float radius, float *dist)
{
	CVector3 RSDir;

	float RayToSphereLength = 0.0f;
	float IntersectPoint = 0.0f;
	float SquaredPoint = 0.0f;

	// Get the ray/sphere direction.
	RSDir = pos - m_origin;

	RayToSphereLength = RSDir.DotProduct3(RSDir);
	IntersectPoint = RSDir.DotProduct3(m_direction);

	// If true no collision.
	if (IntersectPoint < 0)
		return false;

	SquaredPoint = (radius * radius) - RayToSphereLength + (IntersectPoint * IntersectPoint);

	// If this is less then zero then the ray does not hit.
	if (SquaredPoint < 0)
		return false;

	// Else it does hit the sphere and we record the results.
	if (dist)
		*dist = IntersectPoint - (float)sqrt(SquaredPoint);

	return true;
}

bool CRay::Intersect(CVector3 &p1, CVector3 &p2, CVector3 &p3, bool cull, float *dist)
{
	// Store these vectors between the points for future use.
	CVector3 vecAB = p2 - p1;
	CVector3 vecAC = p3 - p1;

	// Get the cross between the p3 - p1 vec and the ray dir.
	CVector3 cross;
	cross.CrossProduct(m_direction, vecAC);

	// Calculate the determinate.
	float det = vecAB.DotProduct3(cross);

	// If either true, no collision.
	if (cull && det < 0.0001f)
		return false;
	else if (det < 0.0001f && det > -0.0001f)
		return false;

	CVector3 rayPointVec = m_origin - p1;
	float test1 = rayPointVec.DotProduct3(cross);

	// Ray behind, no collision.
	if (test1 < 0.0f || test1 > det)
		return false;

	CVector3 cross2;
	cross2.CrossProduct(rayPointVec, vecAB);

	float test2 = m_direction.DotProduct3(cross2);

	// If true then no collision.
	if (test2 < 0.0f || test1 + test2 > det)
		return false;

	// Get collision dist.
	float inverseDet = 1.0f / det;

	// Record the distance of the intersection.
	if (dist)
	{
		*dist = vecAC.DotProduct3(cross2);
		*dist *= inverseDet;
	}

	return true;
}

bool CRay::Intersect(CVector3 & p1, CVector3 &p2, CVector3 &p3, bool cull, float length, float *dist)
{
	// Store these vectors between the points for future use.
	CVector3 vecAB = p2 - p1;
	CVector3 vecAC = p3 - p1;

	// Get the cross between the p3 - p1 vec and the ray dir.
	CVector3 cross;
	cross.CrossProduct(m_direction, vecAC);

	// Calculate the determinate.
	float det = vecAB.DotProduct3(cross);

	// If either true, no collision.
	if (cull && det < 0.0001f)
		return false;
	else if (det < 0.0001f && det > -0.0001f)
		return false;

	CVector3 rayPointVec = m_origin - p1;

	float test1 = rayPointVec.DotProduct3(cross);

	// Ray behind, no collision.
	if (test1 < 0.0f || test1 > det)
		return false;

	CVector3 cross2;
	cross2.CrossProduct(rayPointVec, vecAB);

	float test2 = m_direction.DotProduct3(cross2);

	// If true then no collision.
	if (test2 < 0.0f || test1 + test2 > det)
		return false;

	// Get distance. If smaller then ray length then collision.
	float inverseDet = 1.0f / det;

	// Test ray's length and see if it still intersects.
	// Record distance.
	if (dist)
	{
		*dist = vecAC.DotProduct3(cross2);
		*dist *= inverseDet;

		if (*dist > length)
			return false;
	}
	else
	{
		float l = vecAC.DotProduct3(cross2) * inverseDet;

		if (l > length)
			return false;
	}

	return true;
}
