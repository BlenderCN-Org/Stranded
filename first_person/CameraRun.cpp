#include "CameraRun.h"
#include "Camera.h"

namespace CameraRun
{
	Camera camera;
	
	void initData()
	{
		gCamera = &camera;
		gCamera->pos() = D3DXVECTOR3(-20.0f, 2.0f, 0.0f);
	}

	void updateAll(float dt)
	{
		gCamera->update(dt);
	}
}

