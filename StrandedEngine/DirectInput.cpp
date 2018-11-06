#include "DirectInput.h"

#ifdef _MSC_VER
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")
#endif

CGameController *gThis = nullptr;

bool CreateDIInput(CInputInterface **pObj, HWND hwnd, HINSTANCE hInst, bool exclusive)
{
	if (!*pObj)
		*pObj = new CDirectInputSystem(hwnd, hInst, exclusive);
	else
		return false;

	return true;
}

BOOL gJSEnumDeviceCallBack(const DIDEVICEINSTANCE *inst, void* pData)
{
	return gThis->EnumDeviceCallback(inst, pData);
}

//////////////////////////////////////////////////////////////////////////
/*
 * @param: Direct3D 输入设备, 程序主窗口句柄
 */
CKeyboard::CKeyboard(LPDIRECTINPUT8 input, HWND hwnd) : m_device(NULL)
{
	memset(m_oldKeys, 0, sizeof(char) * KEYS_SIZE);

	/*
	* Initialize the keyboard.
	* 创建输入设备
		HRESULT CreateDevice(
		REFGUID rguid,
		LPDIRECTINPUTDEVICE *lplpDirectInputDevice,
		LPUNKNOWN pUnkOuter
		);
	* rguid - 正在试图创建的设备引用. 键盘(GUID_SysKeyboard); 鼠标(GUID_SysMouse)
	* lplpDirectInputDevice - 正在创建的设备对象
	* pUnkOuter - 对象的IUnKnown地址(可为NULL)
	*/
	if (input->CreateDevice(GUID_SysKeyboard, &m_device, NULL) == DI_OK)
	{
		/*
		* 设置数据格式
		* @param: 描述设备返回的数据格式的c_dfDIKeyboard、c_dfIDMouse、c_dfDIMouse2、c_dfDIJoystick 或 c_dfDIJoystick2 的地址
			HRESULT SetDataFormat(LPCDIDATAFORMAT lpdf);
		*/
		if (m_device->SetDataFormat(&c_dfDIKeyboard) == DI_OK)
		{
			/*
			* 设置设备的合作等级，指定设备与其他设备实例以及与系统自身之间的交互方式
				HRESULT SetCooperativeLevel(HWND hwnd, DWORD dwFlags);
			* hwnd - 程序窗口句柄
			* dwFlags - 指定设备合作等级的标识符. 获取无窗口设备(DISCL_BACKGROUND; 系统独占该设备(DISCL_EXCLUSIVE);
				程序在后台运行时不想获取该设备(DISCL_FOREGROUND); 不独占该设备(DISCL_NONEXCLUSIVE); 禁用Windows键(DISCL_NOWINKEY))
			*/
			if (m_device->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE) == DI_OK)
			{
				/*
				* 获取输入设备，成功，返回DI_OK; 否则返回其他任意值
					HRESULT Acquire();
				*/
				m_device->Acquire();
			}
		}
	}

	// Clear keys will clear out the array of keys we have.
	memset(m_keys, 0, sizeof(char) * KEYS_SIZE);
}

bool CKeyboard::UpdateDevice()
{
	if (m_device)
	{
		// Save old state for input comparing.
		memcpy(m_oldKeys, m_keys, sizeof(m_keys));

		// If error getting device state, re-aquire.
		/*
		* If error getting device state, re-aquire.
		* 为检测来自设备的输入，须获取设备的当前状态
			HRESULT GetDeviceState(DWORD cbData, LPVOID lpvData);
		* cbData - 存储设备状态的结构的大小
		* lpvData - 存储状态的对象
		*/
		if (FAILED(m_device->GetDeviceState(sizeof(m_keys), (LPVOID)m_keys)))
		{
			if (FAILED(m_device->Acquire()))
				return false;
			if (FAILED(m_device->GetDeviceState(sizeof(m_keys), (LPVOID)m_keys)))
				return false;
		}
	}

	return true;
}

int CKeyboard::ButtonUp(unsigned int key)
{
	// If the key is not pressed then return false.
	return (!(m_keys[key] & 0x80) && (m_keys[key] != m_oldKeys[key]));
}

int CKeyboard::ButtonDown(unsigned int key)
{
	return (m_keys[key] & 0x80);
}

POINT CKeyboard::GetPosition()
{
	// Doesn't have position.
	POINT p = { 0, 0 };
	return p;
}

POINT CKeyboard::GetZPosition()
{
	// Doesn't have position.
	POINT p = { 0, 0 };
	return p;
}

void CKeyboard::Shutdown()
{
	if (m_device)
	{
		m_device->Unacquire();
		m_device->Release();
		m_device = NULL;
	}
}

//////////////////////////////////////////////////////////////////////////
/*
* @param: Direct3D 输入设备, 程序主窗口句柄，鼠标是否独占
*/
CMouse::CMouse(LPDIRECTINPUT8 input, HWND hwnd, bool exclusive) : m_device(NULL),
	m_xMPos(0), m_yMPos(0), m_zMPos(0)
{
	// Initialize the Mouse.
	if (input->CreateDevice(GUID_SysMouse, &m_device, NULL) == DI_OK)
	{
		if (m_device->SetDataFormat(&c_dfDIMouse) == DI_OK)
		{
			DWORD flags;
			if (exclusive)
				flags = DISCL_FOREGROUND | DISCL_EXCLUSIVE | DISCL_NOWINKEY;
			else
				flags = DISCL_FOREGROUND | DISCL_NONEXCLUSIVE;
			if (m_device->SetCooperativeLevel(hwnd, flags) == DI_OK)
				m_device->Acquire();
		}
	}
}

bool CMouse::UpdateDevice()
{
	// Get the device state.
	if (m_device)
	{
		// Save old state for input comparing.
		memcpy(&m_oldMouseState, &m_mouseState, sizeof(m_mouseState));

		// If error getting device state, re-aquire.
		if (FAILED(m_device->GetDeviceState(sizeof(DIMOUSESTATE), &m_mouseState)))
		{
			if (FAILED(m_device->Acquire()))
				return false;
			if (FAILED(m_device->GetDeviceState(sizeof(DIMOUSESTATE), &m_mouseState)))
				return false;
		}

		m_xMPos += m_mouseState.lX;
		m_yMPos += m_mouseState.lY;
		m_zMPos = m_mouseState.lZ;
	}

	return true;
}

int CMouse::ButtonUp(unsigned int button)
{
	// If the button is not clicked we return false.
	return (!(m_mouseState.rgbButtons[button] & 0x80) && (m_mouseState.rgbButtons[button] != m_oldMouseState.rgbButtons[button]));
}

int CMouse::ButtonDown(unsigned int button)
{
	return (m_mouseState.rgbButtons[button] & 0x80);
}

POINT CMouse::GetPosition()
{
	POINT pos;

	pos.x = m_xMPos;
	pos.y = m_yMPos;

	return pos;
}

POINT CMouse::GetZPosition()
{
	POINT p = { m_zMPos, m_zMPos };
	return p;
}

void CMouse::Shutdown()
{
	if (m_device)
	{
		m_device->Unacquire();
		m_device->Release();
		m_device = NULL;
	}
}

//////////////////////////////////////////////////////////////////////////
CGameController::CGameController(LPDIRECTINPUT8 input, HWND hwnd) : m_device(NULL), m_numButtons(0),
	m_xGCPos(0), m_yGCPos(0), m_xGCPos2(0), m_yGCPos2(0)
{
	// Save copies.
	gThis = this;
	m_hwnd = hwnd;
	m_inputSystem = input;

	memset(m_name, 0, sizeof(m_name));

	// INitialize the game controller.
	/*
	* 创建游戏控制器，须使用回调函数，不能直接创建设备. 游戏控制器是枚举的，不是遍历所有与机器相连的可用设备手动创建的
	*/
	DIPROPRANGE range;
	DIDEVCAPS caps;

	/*
	* dwDevType - 限制搜索某一类型的设备筛选类型
	* lpCallback - 创建设备而被调用的回调函数的地址
	* pvRef - 传给该回调函数的数值
	* dwFlags - 指定设备枚举方式的标识符. DIEDFL_ALLDEVICES(枚举所有安装在机器上的设备)、DIEDFL_ATTACHEDONLY(只枚举已连接和安装的设备)、
		DIEDFL_FORCEFEEDBACK(枚举具有作用力反馈的设备)、DIEDFL_INCLUDEHIDDEN(枚举系统中隐藏的设备) 和 DIEDFL_INCLUDEPHANTOMS(枚举包含占位符的设备)
		HRESULT EnumDevices(
		DWORD dwDevType,
		LPDIENUMDEVICESCALLBACK lpCallback,
		LPVOID pvRef,
		DWORD dwFlags
	);
	*/
	m_inputSystem->EnumDevices(DI8DEVCLASS_GAMECTRL, (LPDIENUMDEVICESCALLBACK)gJSEnumDeviceCallBack, NULL, DIEDFL_ATTACHEDONLY);

	if (m_device)
	{
		range.diph.dwSize = sizeof(DIPROPRANGE);
		range.diph.dwHeaderSize = sizeof(DIPROPHEADER);
		range.diph.dwHow = DIPH_BYOFFSET;
		range.lMin = -1000;
		range.lMax = 1000;
		range.diph.dwObj = DIJOFS_X;
		m_device->SetProperty(DIPROP_RANGE, &range.diph);
		range.diph.dwObj = DIJOFS_Y;
		m_device->SetProperty(DIPROP_RANGE, &range.diph);

		/*
		* 获取设备上按钮的数目
			HRESULT GetCapabilities(LPDIDEVCAPS lpDIDevCaps);
		*/
		if (SUCCEEDED(m_device->GetCapabilities(&caps)))
			m_numButtons = caps.dwButtons;
		else
			m_numButtons = 4;
	}
}

bool CGameController::UpdateDevice()
{
	if (m_device)
	{
		m_device->Poll();

		// Save old state for input comparing.
		memcpy(&m_oldGCState, &m_gcState, sizeof(m_gcState));

		// If error getting device state, re-aquire.
		if (FAILED(m_device->GetDeviceState(sizeof(DIJOYSTATE2), &m_gcState)))
		{
			if (FAILED(m_device->Acquire()))
				return false;

			if (FAILED(m_device->GetDeviceState(sizeof(DIJOYSTATE2), &m_gcState)))
				return false;
		}

		m_xGCPos = m_gcState.lX;
		m_yGCPos = m_gcState.lY;

		m_xGCPos2 = m_gcState.lZ;
		m_yGCPos2 = m_gcState.lRz;
	}

	return true;
}

/*
* 创建实际的游戏控制器设备
* @return: DIENUM_STOP - 将通知 DirectInput 终止对设备的搜索(已经找到设备); DIENUM_CONTINUE (通知 DirectInput 继续搜索与 PC 相连的设备)
*/
BOOL CGameController::EnumDeviceCallback(const DIDEVICEINSTANCE *inst, void* pData)
{
	// Set to the first device found.
	if (SUCCEEDED(m_inputSystem->CreateDevice(inst->guidInstance, &m_device, NULL)))
	{
		if (SUCCEEDED(m_device->SetDataFormat(&c_dfDIJoystick2)))
		{
			if (SUCCEEDED(m_device->SetCooperativeLevel(m_hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE)))
			{
				if (SUCCEEDED(m_device->Acquire()))
				{
#pragma warning(push)
#pragma warning(disable:4996)
					strcpy(m_name, (char*)inst->tszProductName);
#pragma warning(pop)
					return DIENUM_STOP;
				}
			}
		}
	}

	// Return continue to try to init other connected devices.
	return DIENUM_CONTINUE;
}

int CGameController::ButtonUp(unsigned int button)
{
	if (button < 0 || button >= m_numButtons)
		return 0;

	return (!(m_gcState.rgbButtons[button] & 0x80) && (m_gcState.rgbButtons[button] != m_oldGCState.rgbButtons[button]));
}

int CGameController::ButtonDown(unsigned int button)
{
	if (button < 0 || button >= m_numButtons)
		return 0;

	return (m_gcState.rgbButtons[button] & 0x80);
}


// left stick
POINT CGameController::GetPosition()
{
	POINT pos;

	pos.x = m_xGCPos;
	pos.y = m_yGCPos;

	return pos;
}

POINT CGameController::GetZPosition()
{
	POINT pos;

	pos.x = m_xGCPos2;
	pos.y = m_yGCPos2;

	return pos;
}

void CGameController::Shutdown()
{
	if (m_device)
	{
		m_device->Unacquire();
		m_device->Release();
		m_device = NULL;
	}
}

//////////////////////////////////////////////////////////////////////////
CDirectInputSystem::CDirectInputSystem(HWND hwnd, HINSTANCE hInst, bool exclusive)
{
	// Initialize objects...
	m_keyboard = NULL;
	m_mouse = NULL;
	m_gameController = NULL;

	// Create input system.
	/*
	* Create input system.
		HRESULT WINAPI DirectInput8Create(
		HINSTANCE hinst,
		DWORD dwVersion,
		REFIID riidltf,
		LPVOID *ppvOut,
		LPUNKNOWN punkOuter
		);
	* hinst - 程序实例
	* dwVersion - 版本标识符
	* riidltf - 输入系统的唯一标识符(IID_IDirectInput8、IID_IDirectInput8A 或 IID_IDirectInput8W)
	* ppvOut - 指向正在创建的输入系统对象(类型为LPDIRECTINPUT8)的指针
	* punkOuter - 与COM+对象IUnKnown接口相关的指针
	*/
	if (DirectInput8Create(hInst, DIRECTINPUT_VERSION, IID_IDirectInput8, (void **)&m_system, NULL) == DI_OK)
	{
		m_keyboard = new CKeyboard(m_system, hwnd);
		m_mouse = new CMouse(m_system, hwnd, exclusive);
		m_gameController = new CGameController(m_system, hwnd);
	}
}

CDirectInputSystem::~CDirectInputSystem()
{
	// Shut everything down.
	Shutdown();
}

bool CDirectInputSystem::Initialize()
{
	// Everything took place in the constructor.
	// Force keyboard and mouse before considering the system initialized.
	// 确保系统安装了键盘和鼠标(游戏控制器作为备选项)
	return (m_keyboard && m_mouse);
}

bool CDirectInputSystem::UpdateDevices()
{
	int hr;

	// Get the device state.
	if (m_mouse)
		hr = m_mouse->UpdateDevice();
	if (m_keyboard)
		hr = m_keyboard->UpdateDevice();
	if (m_gameController)
		m_gameController->UpdateDevice();

	return true;
}

int CDirectInputSystem::KeyUp(unsigned int key)
{
	if (!m_keyboard)
		return 0;
	return m_keyboard->ButtonUp(key);
}

int CDirectInputSystem::KeyDown(unsigned int key)
{
	if (!m_keyboard)
		return 0;
	return m_keyboard->ButtonDown(key);
}

int CDirectInputSystem::MouseButtonUp(unsigned int button)
{
	if (!m_mouse)
		return 0;
	return m_mouse->ButtonUp(button);
}

int CDirectInputSystem::MouseButtonDown(unsigned int button)
{
	if (!m_mouse)
		return 0;
	return m_mouse->ButtonDown(button);
}

POINT CDirectInputSystem::GetMousePos()
{
	POINT null = { 0, 0 };
	if (!m_mouse)
		return null;
	return m_mouse->GetPosition();
}

long CDirectInputSystem::GetMouseWheelPos()
{
	if (!m_mouse)
		return 0;

	POINT wheel = m_mouse->GetZPosition();
	return wheel.y;
}

int CDirectInputSystem::ControllerButtonUp(unsigned int button)
{
	if (!m_gameController)
		return 0;
	return m_gameController->ButtonUp(button);
}

int CDirectInputSystem::ControllerButtonDown(unsigned int button)
{
	if (!m_gameController)
		return 0;
	return m_gameController->ButtonDown(button);
}

POINT CDirectInputSystem::GetLeftStickPos()
{
	POINT null = { 0, 0 };

	if (!m_gameController)
		return null;
	return m_gameController->GetPosition();
}

POINT CDirectInputSystem::GetRightStickPos()
{
	POINT null = { 0, 0 };

	if (!m_gameController)
		return null;
	return m_gameController->GetZPosition();
}

void CDirectInputSystem::Shutdown()
{
	// Delete each object...
	if (m_keyboard)
	{
		m_keyboard->Shutdown();
		delete m_keyboard;
		m_keyboard = NULL;
	}

	if (m_mouse)
	{
		m_mouse->Shutdown();
		delete m_mouse;
		m_mouse = NULL;
	}

	if (m_gameController)
	{
		m_gameController->Shutdown();
		delete m_gameController;
		m_gameController = NULL;
	}

	if (m_system)
	{
		m_system->Release();
		m_system = NULL;
	}
}
