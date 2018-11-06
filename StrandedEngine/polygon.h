#ifndef __STRANDED_POLYGON_H__
#define __STRANDED_POLYGON_H__


#include "mathLibrary.h"

class CPolygon
{
public:
	CPolygon() {}

	void Copy(CPolygon &pol);
	bool Intersect(CRay &ray, bool cull, float *dist);

public:
	CVector3 m_vertexList[3];
};

#endif
