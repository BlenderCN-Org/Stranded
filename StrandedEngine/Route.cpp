#include "Route.h"
#include <windows.h>
#include <mmsystem.h>

#ifdef _MSC_VER
#pragma comment(lib, "winmm.lib")
#endif

#ifndef M_PI
#define M_PI       3.14159265358979323846   // pi
#endif

CPath::CPath() : m_type(ERROR_PATH), m_start(0), m_total(0), m_next(nullptr)
{

}

CPath::CPath(int Type, CVector3& start, CVector3& cnt1, CVector3& cnt2, CVector3& end)
{
	m_start = 0.0f;
	m_total = 0.0f;
	m_next = nullptr;

	SetPath(m_type, start, cnt1, cnt2, end);
}

CPath::CPath(CVector3& center, CVector3& direction, CVector3& start)
{
	m_start = 0.0f;
	m_total = 0.0f;
	m_next = nullptr;

	m_radius = 0.0f;

	SetPath(center, direction, start);

	// First we make sure that the center, start pos, and point in the plane are colinear.
	m_planeNormal.CrossProduct(m_startPos - m_centerPos, m_planeDirection - m_centerPos);

	// Check if they are colinear.
	if (m_planeNormal.GetLength() < 0.01f)
	{
		MessageBox(NULL, "Circle path must be colinear!", "Error ...", MB_OK);
		m_type = ERROR_PATH;
	}

	// If they are then we normalize and continue the calculation.
	m_planeNormal.Normal();

	// The radius will be determined by where the center of the circle is and the start position of the animation.
	// The u and v are orthonormal vectors. u goes from the start to cetner while v is perpendicular to the u.
	m_radius = (m_startPos - m_centerPos).GetLength();

	m_circleU = m_startPos - m_centerPos;
	m_circleU.Normal();
	m_circleV.CrossProduct(m_circleU, m_planeNormal);
}

CPath::~CPath()
{
	// Release all resources.
	Shutdown();
}

void CPath::SetPath(int Type, CVector3& start, CVector3& cnt1, CVector3& cnt2, CVector3& end)
{
	// Simple initialize.
	m_type = Type;
	m_startPos = start;
	m_control1 = cnt1;
	m_control2 = cnt2;
	m_endPos = end;
}

void CPath::SetPath(CVector3& center, CVector3& direction, CVector3& start)
{
	m_type = CIRCLE_PATH;
	m_startPos = start;
	
	m_centerPos = center;
	m_planeDirection = direction;
}

void CPath::Shutdown()
{
	// Release resources.
	if (m_next)
	{
		// By calling the m_next pointer's Shutdown, deletion of list of nodes will not start until you get to
		// the end of the list. This way there are absolutely no memory leaks and is quite safe.
		m_next->Shutdown();
		delete m_next;
		m_next = nullptr;
	}
}

//////////////////////////////////////////////////////////////////////////

CRoute::CRoute() : m_path(nullptr), m_startTime(0.0f)
{

}

CRoute::~CRoute()
{
	Shutdown();
}

/*
 * @param: 设置的路径类型、起始位置、两个控制点、终止位置
		Type: 0-代表直线; 1-代表曲线; 2-代表圆
 * @return: 如果增加了路径，返回true; 否则返回false.
 */
bool CRoute::AddPath(int Type, CVector3& start, CVector3& cnt1, CVector3& cnt2, CVector3& end)
{
	// If we don't have a root node for the path.
	if (m_path == nullptr)
	{
		// Allocate data for the root node.
		m_path = new CPath(Type, start, cnt1, cnt2, end);
		// Make sure all went well.
		if (m_path == nullptr) return false;

		// Since this is the start node, its start m_total is 0.
		m_path->m_start = 0;

		// Set based on type.
		if (Type == STRAIGHT_PATH)
		{
			m_path->m_total = CVector3(start - end).GetLength();
		}
		else if (Type == CURVE_PATH)
		{
			float length01 = CVector3(cnt1 - start).GetLength();
			float length12 = CVector3(cnt2 - cnt1).GetLength();
			float length23 = CVector3(end - cnt2).GetLength();
			float length03 = CVector3(end - start).GetLength();
			m_path->m_total = (length01 + length12 + length23)*0.5f + length03*0.5f;
		}
		else
			return false;
	}
	else
	{
		// Point to root node.
		CPath *ptr = m_path;

		// Loop until we find last node.
		while (ptr->m_next != nullptr)
		{
			ptr = ptr->m_next;
		}

		// Create the m_next path in our list.
		ptr->m_next = new CPath(Type, start, cnt1, cnt2, end);
		// Error checking.
		if (ptr->m_next == nullptr)
			return false;

		// Total is last path total + its start.
		ptr->m_next->m_start = ptr->m_total + ptr->m_start;

		// Set based on type.
		if (Type == STRAIGHT_PATH)
		{
			ptr->m_next->m_total = CVector3(start - end).GetLength();
		}
		else if (Type == CURVE_PATH)
		{
			float length01 = CVector3(cnt1 - start).GetLength();
			float length12 = CVector3(cnt2 - cnt1).GetLength();
			float length23 = CVector3(end - cnt2).GetLength();
			float length03 = CVector3(end - start).GetLength();
			ptr->m_next->m_total = (length01 + length12 + length23)*0.5f + length03*0.5f;
		}
		else
			return false;
	}

	return true;
}

/*
 * 增加圆形路径
 */
bool CRoute::AddPath(CVector3& center, CVector3& direction, CVector3& start)
{
	// If we don't have a root node for the path.
	if (m_path == nullptr)
	{
		// Allocate data for the root node.
		m_path = new CPath(center, direction, start);
		// Make sure all went well.
		if (m_path == nullptr) return false;
		if (m_path->m_type == ERROR_PATH) return false;

		// Since this is the start node, its start m_total is 0.
		m_path->m_start = 0;

		m_path->m_total = 2 * M_PI * m_path->m_radius;
	}
	else
	{
		// Point to root node.
		CPath *ptr = m_path;

		// Loop until we find last node.
		while (ptr->m_next != nullptr)
		{
			ptr = ptr->m_next;
		}

		// Create the m_next path in our list.
		ptr->m_next = new CPath(center, direction, start);
		// Error checking.
		if (ptr->m_next == nullptr)
			return false;
		if (ptr->m_next->m_type == ERROR_PATH) return false;

		// Total is last path total + its start.
		ptr->m_next->m_start = ptr->m_total + ptr->m_start;

		ptr->m_next->m_total = 2 * M_PI * ptr->m_next->m_radius;
	}

	return true;
}

/*
 * @param: 当前时间比值、保存计算得到的位置地址
 */
void CRoute::GetPosition(float time, CVector3 &objPos)
{
	// Error checking.
	if (m_path == nullptr) return;

	CPath *ptr = m_path;

	// Initialize the start time if it has not been already.
	if (m_startTime == 0)
		m_startTime = (float)timeGetTime();	// 毫秒,为从系统开启算起所经过的时间。

	// Loop through all paths to see where we are at.
	do 
	{
		// Check if we fall within this path.
		if (time >= ptr->m_start && time < ptr->m_start + ptr->m_total)
		{
			// Subtract from this start to get place in this path.
			time -= ptr->m_start;

			// Divide by the path's total length to get scalar.
			float Scalar = time / ptr->m_total;

			// Set depending on type.
			if (ptr->m_type == STRAIGHT_PATH)
			{
				objPos = CalcStriaghtPath(ptr->m_startPos, ptr->m_endPos, Scalar);
			}
			else if (ptr->m_type == CURVE_PATH)
			{
				objPos = CalcBezierCurvePos(ptr->m_startPos, ptr->m_control1, ptr->m_control2, ptr->m_endPos, Scalar);
			}
			else if (ptr->m_type == CIRCLE_PATH)
			{
				objPos = CalcCirclePos(Scalar, ptr->m_centerPos, ptr->m_circleU, ptr->m_circleV, ptr->m_radius);
			}

			// We are down since we found the path.
			break;
		}
		else
		{
			// If we hit the last node, restart.
			if (ptr->m_next == nullptr)
				m_startTime = (float)timeGetTime();
		}

		// Move to the next path, we didn't find the right one.
		ptr = ptr->m_next;
	} while (ptr != nullptr);
}


CVector3 CRoute::CalcBezierCurvePos(CVector3& start, CVector3& c1, CVector3& c2, CVector3& end, float Scalar)
{
	// the curve path.
	CVector3 out;

	out = start * (1.0f - Scalar) * (1.0f - Scalar) * (1.0f - Scalar) +
		c1 * 3.0f * Scalar * (1.0f - Scalar) * (1.0f - Scalar) +
		c2 * 3.0f * Scalar * Scalar * (1.0f - Scalar) +
		end * Scalar * Scalar * Scalar;

	return out;
}

CVector3 CRoute::CalcStriaghtPath(CVector3& start, CVector3& end, float Scalar)
{
	// the Striaght line path.
	CVector3 out = (end - start)*Scalar + start;

	return out;
}

CVector3 CRoute::CalcCirclePos(float angle, CVector3& center, CVector3& u, CVector3& v, float radius)
{
	float newAngle = angle * (float)(M_PI / 180.0f);

	return center + u * radius * float(cos(newAngle)) + v * radius * float(sin(newAngle));
}

void CRoute::Shutdown()
{
	// Release all resources.
	if (m_path)
	{
		// Call the root node's Shutdown(), clear the whole list.
		m_path->Shutdown();

		delete m_path;
		m_path = nullptr;
	}
}
