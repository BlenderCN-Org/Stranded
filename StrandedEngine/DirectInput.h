#ifndef __DIRECTINPUT_H__
#define __DIRECTINPUT_H__

#include <dinput.h>
#include "InputInterface.h"
#include "defines.h"

class CKeyboard : public CDeviceInterface
{
public:
	CKeyboard(LPDIRECTINPUT8 input, HWND hwnd);
	~CKeyboard() { Shutdown(); }

	bool UpdateDevice();

	int ButtonUp(unsigned int key);
	int ButtonDown(unsigned int key);

	POINT GetPosition();
	POINT GetZPosition();

	void Shutdown();

	// Keyboard device.
	LPDIRECTINPUTDEVICE8 m_device;
	char m_keys[KEYS_SIZE];
	char m_oldKeys[KEYS_SIZE];
};

class CMouse : public CDeviceInterface
{
public:
	CMouse(LPDIRECTINPUT8 input, HWND hwnd, bool exclusive);
	~CMouse() { Shutdown(); }

	bool UpdateDevice();

	int ButtonUp(unsigned int button);
	int ButtonDown(unsigned int button);

	POINT GetPosition();
	POINT GetZPosition();

	void Shutdown();

	// Mouse device.
	LPDIRECTINPUTDEVICE8 m_device;
	DIMOUSESTATE m_mouseState;
	DIMOUSESTATE m_oldMouseState;

	// Mouse x, y, and wheel position.
	long m_xMPos;
	long m_yMPos;
	long m_zMPos;
};

class CGameController : public CDeviceInterface
{
public:
	CGameController(LPDIRECTINPUT8 input, HWND hwnd);
	~CGameController() { Shutdown(); }

	bool UpdateDevice();

	// Used to create the game controllers.
	BOOL EnumDeviceCallback(const DIDEVICEINSTANCE *inst, void* pData);

	int ButtonUp(unsigned int button);
	int ButtonDown(unsigned int button);

	POINT GetPosition();
	POINT GetZPosition();

	void Shutdown();

	// Game controller device.
	LPDIRECTINPUTDEVICE8 m_device;
	DIJOYSTATE2 m_gcState;
	DIJOYSTATE2 m_oldGCState;
	char m_name[256];
	unsigned long m_numButtons;

	// Left and right stick x and y positions.
	long m_xGCPos;
	long m_yGCPos;
	long m_xGCPos2;
	long m_yGCPos2;

	// Window handled (needed for game controllers).
	HWND m_hwnd;

	// Copy of input system.
	LPDIRECTINPUT8 m_inputSystem;
};

class CDirectInputSystem : public CInputInterface
{
public:
	CDirectInputSystem(HWND hwnd, HINSTANCE hInst, bool exclusive);
	~CDirectInputSystem();

	// Init devices, update devices states.
	bool Initialize();
	bool UpdateDevices();

	// Keyboard functions.
	int KeyUp(unsigned int key);
	int KeyDown(unsigned int key);

	// Mouse functions.
	int MouseButtonUp(unsigned int button);
	int MouseButtonDown(unsigned int button);

	// Get mouse position (x, y) and mouse wheel data (z).
	POINT GetMousePos();
	long GetMouseWheelPos();

	// Game controller functions.
	int ControllerButtonUp(unsigned int button);
	int ControllerButtonDown(unsigned int button);

	// Get controller main (left) and right stick position.
	POINT GetLeftStickPos();
	POINT GetRightStickPos();

	void Shutdown();

protected:
	LPDIRECTINPUT8 m_system;

	// Keyboard device.
	CKeyboard *m_keyboard;
	// Mouse device.
	CMouse *m_mouse;
	// Game controller device.
	CGameController *m_gameController;
};

bool CreateDIInput(CInputInterface **pObj, HWND hwnd, HINSTANCE hInst, bool exclusive);

#endif
