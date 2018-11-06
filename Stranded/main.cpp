#include "main.h"
#include "../Common/Global.h"
#include "../Common/SException.h"

#ifdef _MSC_VER
#pragma comment(lib, "../lib/Common.lib")
#pragma comment(lib, "../lib/StrandedEngine.lib")
// timeSetEvent、timeKillEvent - Declared in Mmsystem.h; include Windows.h.
#pragma comment(lib, "Winmm.lib")
#endif

//////////////////////////////////////////////////////////////////////////

// Game Command
// 用于指定天空体网格的文件名X模型文件
static const char* SC_LOAD_SKY_BOX_AS_X = "LoadSkyBoxAsX";
// 将场景以X模型加载到引擎中
static const char* SC_LOAD_TERRAIN_AX_X = "LoadTerrainAsX";
static const char* SC_LOAD_STATIC_MODEL_AS_X = "LoadStaticModelAsX";
static const char* SC_LOAD_ANIMATED_MODEL_AS_X = "LoadAnimatedModelAsX";

//////////////////////////////////////////////////////////////////////////

// Globals...
HWND g_hwnd = NULL;
HINSTANCE g_hInstance;
CRenderInterface *g_Render = NULL;
CInputInterface *g_InputSystem = NULL;
// 声音系统基类指针
CSoundSystemInterface *g_SoundSystem = NULL;

// GUI ids.
int g_mainGui = -1;
int g_startGui = -1;
int g_creditsGui = -1;
int g_currentGUI = GUI_MAIN_SCREEN;

// Font id.
int g_arialID = -1;

// Temp Mouse state information.
bool g_LMBDown = false;
int g_mouseX = 0;
int g_mouseY = 0;

// Sound id.
int g_menuSound = -1;

stGameWorld g_gameWorld;
GAME_STATE g_gameState = GS_MENU;

// 系统Tick驱动事件
MMRESULT g_hEventTimer = NULL;
// Tick通知Event
HANDLE g_htickEvent = NULL;


LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		// 销毁消息
	case WM_DESTROY:
	{
		::ShowWindow(g_hwnd, SW_HIDE);

		::PostQuitMessage(0);
		return 0;
	}
	break;
	case WM_KEYUP:
	{
		if (wParam == VK_ESCAPE)
			::PostQuitMessage(0);
	}
	break;
	case WM_LBUTTONDOWN:
	{
		g_LMBDown = true;
	}
	break;
	case WM_LBUTTONUP:
	{
		g_LMBDown = false;
	}
	break;
	case WM_MOUSEMOVE:
	{
		g_mouseY = HIWORD(lParam);
		g_mouseX = LOWORD(lParam);
	}
	break;
	case WM_FLASH_WND:
	{
		// ::SendMessage(g_hwnd, WM_FLASH_WND, 0, 0); 主动发送消息
		if (::GetFocus() != g_hwnd)
		{
			FLASHWINFO fi;

			memset(&fi, 0, sizeof(FLASHWINFO));

			fi.cbSize = sizeof(FLASHWINFO);
			fi.hwnd = g_hwnd;

			::FlashWindowEx(&fi);
		}
	}
	break;
	case WM_SYSCOMMAND:
	{
		if (wParam == SC_CLOSE)
		{
			// 截获关闭按钮消息
		}
	}
	break;
	case WM_CLOSE:
	{
		::PostMessage(g_hwnd, WM_DESTROY, 0, 0);
	}
	break;
	default:
		break;
	}

	return ::DefWindowProc(hWnd, msg, wParam, lParam);
}

INT _tMainWithCPPException(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	// 保存全局句柄
	g_hInstance = hInstance;

	// Register the window class
	WNDCLASSEX wc =
	{
		sizeof(WNDCLASSEX),
		CS_CLASSDC,
		MsgProc,
		0,
		0,
		::GetModuleHandle(NULL),
		NULL,
		::LoadCursor(NULL, IDC_ARROW),
		(HBRUSH)(COLOR_WINDOW + 1),
		NULL,
		WINDOW_CLASS,
		NULL
	};

	::RegisterClassEx(&wc);

	// Create the application's window
	if (FULLSCREEN)
	{
		g_hwnd = ::CreateWindowEx(NULL, WINDOW_CLASS, WINDOW_NAME, WS_POPUP | WS_SYSMENU | WS_VISIBLE,
			0, 0, WIN_WIDTH, WIN_HEIGHT, NULL, NULL, hInstance, NULL);
	}
	else
	{
		g_hwnd = ::CreateWindowEx(NULL, WINDOW_CLASS, WINDOW_NAME, WS_OVERLAPPEDWINDOW | WS_VISIBLE,
			0, 0, WIN_WIDTH, WIN_HEIGHT, NULL, NULL, hInstance, NULL);
	}

	if (g_hwnd)
	{
		// Show the window
		::ShowWindow(g_hwnd, SW_SHOWDEFAULT);
		//强制窗口更新自己
		::UpdateWindow(g_hwnd);
	}

	// Initialize the Stranded Engine.
	if (InitializeEngine())
	{
		// Initialize Stranded game.
		if (GameInitialize())
		{
			MainLoop();
		}
	}

	// Release any and all resources.
	GameShutdown();
	ShutdownEngine();

	// 取消对窗口类的注册
	::UnregisterClass(WINDOW_CLASS, wc.hInstance);

	return 0;
}

void MainLoop()
{
	// Enter the message loop
	MSG msg;
	::ZeroMemory(&msg, sizeof(msg));

	::SetCursorPos(0, 0);

	while(TRUE)
	{
		// 读取出消息队列中的所有消息并处理
		while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
		{
			// 如果是退出消息，退出
			if (msg.message == WM_QUIT)
				return;
			
			// 转换成字符消息
			::TranslateMessage(&msg);
			// 发送给消息过程函数
			::DispatchMessage(&msg);
		};

		// 等待有新的消息进入消息队列或者到达渲染时间
		UINT dwResult = ::MsgWaitForMultipleObjects(1, &g_htickEvent, FALSE, INFINITE, QS_ALLINPUT);
		if (dwResult == WAIT_OBJECT_0)
		{
			GameLoop();
		}
		else
			continue;	// 有新消息进入消息循环, 处理消息
	}
}

/*
 * int WINAPI CALLBACK WinMain ( _In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd );
 * @param
 * 1、程序实例的句柄
 * 2、程序先前的实例
 * 3、传递给程序的命令行参数指针，不包括程序名
 * 4、指明窗口的显示方式
 */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	// 进入结构化异常块
	__try
	{
		return _tMainWithCPPException(hInstance, hPrevInstance, lpCmdLine, nShowCmd);
	}
	__except (CatchSEHException(GetExceptionInformation(), true), ::TerminateProcess(::GetCurrentProcess(), 0), EXCEPTION_EXECUTE_HANDLER){}

	return 0;
}

bool InitializeEngine()
{
	// 为渲染系统分配内存
	if (!CreateD3DRenderer(&g_Render))
		return false;

	// 初始化渲染系统
	if (!g_Render->Initialize(WIN_WIDTH, WIN_HEIGHT, g_hwnd, FULLSCREEN, STRANDED_MS_SAMPLES_4))
		return false;

	// 将清除色设为黑色
	g_Render->SetClearCol(0, 0, 0);

	// 为要显示的文本创建Arial字体
	if (!g_Render->CreateText("Arial", 0, true, 18, g_arialID))
		return false;

	if (!CreateDIInput(&g_InputSystem, g_hwnd, g_hInstance, false))
		return false;
	if (!g_InputSystem->Initialize())
		return false;

	// 创建声音系统
	if (!CreateDMSound(&g_SoundSystem))
		return false;

	return true;
}

//将所有系统使用的动态内存清除干净
void ShutdownEngine()
{
	if (g_Render)
	{
		g_Render->Shutdown();
		delete g_Render;
		g_Render = NULL;
	}

	if (g_InputSystem)
	{
		g_InputSystem->Shutdown();
		delete g_InputSystem;
		g_InputSystem = NULL;
	}

	// 关闭和删除引擎使用的声音系统
	if (g_SoundSystem)
	{
		g_SoundSystem->Shutdown();
		delete g_SoundSystem;
		g_SoundSystem = NULL;
	}
}

bool GameInitialize()
{
	// 初始化随机数种子
	__time32_t tNow;
	_time32(&tNow);
	srand(tNow);

	if (!InitializeMainMenu())
		return false;
	
	//////////////////////////////////////////////////////////////////////////
	// 创建Tick驱动
	INT nMaxFPS = 30;
	
	g_htickEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	::ResetEvent(g_htickEvent);
	g_hEventTimer = ::timeSetEvent((UINT)(1000.0f / nMaxFPS), (UINT)(1000.0f / nMaxFPS), EventTimerProc, 0, TIME_PERIODIC | TIME_CALLBACK_FUNCTION);

	return true;
}

void GameLoop()
{
	ProcessInput();

	if (g_gameState == GS_LEVEL)
		LevelRender();
	else if(g_gameState == GS_MENU)
		MainMenuRender();
	else if (g_gameState == GS_LEVEL_1_SWITCH)
	{
		GameReleaseAll();

		// Start first level.
		if (LoadLevel("maps/level1/level1.lvl"))
			g_gameState = GS_LEVEL;
		else
			g_gameState = GS_MENU;
	}
}

void GameShutdown()
{
	GameReleaseAll();

	// 关闭驱动函数
	if (g_hEventTimer)
	{
		::timeKillEvent(g_hEventTimer);
		g_hEventTimer = NULL;
	}
	::CloseHandle(g_htickEvent);
	g_htickEvent = NULL;
}

// 将整个GUI主菜单加载到内存中
bool InitializeMainMenu()
{
	// Create gui screens.
	if (!g_Render->CreateGUI(g_mainGui))
		return false;
	if (!g_Render->CreateGUI(g_startGui))
		return false;
	if (!g_Render->CreateGUI(g_creditsGui))
		return false;

	// Load backdrops.
	if (!g_Render->AddGUIBackdrop(g_mainGui, "menu/mainMenu.jpg"))
		return false;
	if (!g_Render->AddGUIBackdrop(g_startGui, "menu/startMenu.jpg"))
		return false;
	if (!g_Render->AddGUIBackdrop(g_creditsGui, "menu/creditsMenu.jpg"))
		return false;

	// Set main screen elements.
	if (!g_Render->AddGUIStaticText(g_mainGui, STATIC_TEXT_ID, "Version: 1.0", PERCENT_OF(WIN_WIDTH, 0.85), PERCENT_OF(WIN_WIDTH, 0.05), STRANDED_COLOR_ARGB(255, 255, 255, 255), g_arialID))
		return false;

	if (!g_Render->AddGUIButton(g_mainGui, BUTTON_START_ID, PERCENT_OF(WIN_WIDTH, 0.05), PERCENT_OF(WIN_HEIGHT, 0.40), "menu/startUp.png", "menu/startOver.png", "menu/startDown.png"))
		return false;

	if (!g_Render->AddGUIButton(g_mainGui, BUTTON_CREDITS_ID, PERCENT_OF(WIN_WIDTH, 0.05), PERCENT_OF(WIN_HEIGHT, 0.50), "menu/creditsUp.png", "menu/creditsOver.png", "menu/creditsDown.png"))
		return false;

	if (!g_Render->AddGUIButton(g_mainGui, BUTTON_QUIT_ID, PERCENT_OF(WIN_WIDTH, 0.05), PERCENT_OF(WIN_HEIGHT, 0.60), "menu/quitUp.png", "menu/quitOver.png", "menu/quitDown.png"))
		return false;

	// Set start screen elements.
	if (!g_Render->AddGUIButton(g_startGui, BUTTON_LEVEL_1_ID, PERCENT_OF(WIN_WIDTH, 0.1), PERCENT_OF(WIN_HEIGHT, 0.15), "menu/level1Up.png", "menu/level1Over.png", "menu/level1Down.png"))
		return false;

	if (!g_Render->AddGUIButton(g_startGui, BUTTON_BACK_ID, PERCENT_OF(WIN_WIDTH, 0.1), PERCENT_OF(WIN_HEIGHT, 0.80), "menu/backUp.png", "menu/backOver.png", "menu/backDown.png"))
		return false;

	// Set credits screen elements.
	if (!g_Render->AddGUIStaticText(g_creditsGui, STATIC_TEXT_ID, "Game Design -", PERCENT_OF(WIN_WIDTH, 0.1), PERCENT_OF(WIN_HEIGHT, 0.15), STRANDED_COLOR_ARGB(255, 255, 255, 255), g_arialID))
		return false;

	if (!g_Render->AddGUIStaticText(g_creditsGui, STATIC_TEXT_ID, "			Allen Sherrod", PERCENT_OF(WIN_WIDTH, 0.1), PERCENT_OF(WIN_HEIGHT, 02.0), STRANDED_COLOR_ARGB(255, 255, 255, 255), g_arialID))
		return false;

	if (!g_Render->AddGUIStaticText(g_creditsGui, STATIC_TEXT_ID, "Programming -", PERCENT_OF(WIN_WIDTH, 0.1), PERCENT_OF(WIN_HEIGHT, 0.25), STRANDED_COLOR_ARGB(255, 255, 255, 255), g_arialID))
		return false;

	if (!g_Render->AddGUIStaticText(g_creditsGui, STATIC_TEXT_ID, "			Allen Sherrod", PERCENT_OF(WIN_WIDTH, 0.1), PERCENT_OF(WIN_HEIGHT, 0.30), STRANDED_COLOR_ARGB(255, 255, 255, 255), g_arialID))
		return false;

	if (!g_Render->AddGUIStaticText(g_creditsGui, STATIC_TEXT_ID, "Sound -", PERCENT_OF(WIN_WIDTH, 0.1), PERCENT_OF(WIN_HEIGHT, 0.35), STRANDED_COLOR_ARGB(255, 255, 255, 255), g_arialID))
		return false;

	if (!g_Render->AddGUIStaticText(g_creditsGui, STATIC_TEXT_ID, "			Allen Sherrod", PERCENT_OF(WIN_WIDTH, 0.1), PERCENT_OF(WIN_HEIGHT, 0.40), STRANDED_COLOR_ARGB(255, 255, 255, 255), g_arialID))
		return false;

	if (!g_Render->AddGUIStaticText(g_creditsGui, STATIC_TEXT_ID, "Level Design -", PERCENT_OF(WIN_WIDTH, 0.1), PERCENT_OF(WIN_HEIGHT, 0.45), STRANDED_COLOR_ARGB(255, 255, 255, 255), g_arialID))
		return false;

	if (!g_Render->AddGUIStaticText(g_creditsGui, STATIC_TEXT_ID, "			Allen Sherrod", PERCENT_OF(WIN_WIDTH, 0.1), PERCENT_OF(WIN_HEIGHT, 0.50), STRANDED_COLOR_ARGB(255, 255, 255, 255), g_arialID))
		return false;

	if (!g_Render->AddGUIStaticText(g_creditsGui, STATIC_TEXT_ID, "Special Thanks -", PERCENT_OF(WIN_WIDTH, 0.1), PERCENT_OF(WIN_HEIGHT, 0.55), STRANDED_COLOR_ARGB(255, 255, 255, 255), g_arialID))
		return false;

	if (!g_Render->AddGUIStaticText(g_creditsGui, STATIC_TEXT_ID, "			Jenifer Niles", PERCENT_OF(WIN_WIDTH, 0.1), PERCENT_OF(WIN_HEIGHT, 0.60), STRANDED_COLOR_ARGB(255, 255, 255, 255), g_arialID))
		return false;

	if (!g_Render->AddGUIStaticText(g_creditsGui, STATIC_TEXT_ID, "			Bryan Davidson", PERCENT_OF(WIN_WIDTH, 0.1), PERCENT_OF(WIN_HEIGHT, 0.65), STRANDED_COLOR_ARGB(255, 255, 255, 255), g_arialID))
		return false;

	if (!g_Render->AddGUIStaticText(g_creditsGui, STATIC_TEXT_ID, "			Charles river Media", PERCENT_OF(WIN_WIDTH, 0.1), PERCENT_OF(WIN_HEIGHT, 0.70), STRANDED_COLOR_ARGB(255, 255, 255, 255), g_arialID))
		return false;

	if (!g_Render->AddGUIStaticText(g_creditsGui, STATIC_TEXT_ID, "			Readers of this book", PERCENT_OF(WIN_WIDTH, 0.1), PERCENT_OF(WIN_HEIGHT, 0.75), STRANDED_COLOR_ARGB(255, 255, 255, 255), g_arialID))
		return false;

	if (!g_Render->AddGUIButton(g_creditsGui, BUTTON_BACK_ID, PERCENT_OF(WIN_WIDTH, 0.1), PERCENT_OF(WIN_HEIGHT, 0.80), "menu/backUp.png", "menu/backOver.png", "menu/backDown.png"))
		return false;

	// Load menu sound.
	if (!g_SoundSystem->AddSound("sounds/menu.wav", STRANDED_INFINITE, &g_menuSound))
		return false;

	// Play the sound.
	g_SoundSystem->Play(g_menuSound);

	return true;
}

// GUI系统的回调函数，对按钮动作做出响应
void MainMenuCallback(int id, int state)
{
	switch (id)
	{
	case BUTTON_START_ID:
		// 确保按钮先是按下的，这样在从按钮上方经过时才不会响应
		if (state == STRANDED_BUTTON_DOWN)
			g_currentGUI = GUI_START_SCREEN;
		break;
	case BUTTON_CREDITS_ID:
		if (state == STRANDED_BUTTON_DOWN)
			g_currentGUI = GUI_CREDITS_SCREEN;
		break;
	case BUTTON_BACK_ID:
		if (state == STRANDED_BUTTON_DOWN)
			g_currentGUI = GUI_MAIN_SCREEN;
		break;
	case BUTTON_QUIT_ID:
		if (state == STRANDED_BUTTON_DOWN)
		{
			GameReleaseAll();
			PostQuitMessage(0);
		}
		break;
	case BUTTON_LEVEL_1_ID:
		// Start first level.
		if (state == STRANDED_BUTTON_DOWN)
			g_gameState = GS_LEVEL_1_SWITCH;
		break;
	}
}

// 借助渲染系统将主菜单显示在屏幕上
void MainMenuRender()
{
	if (!g_Render)
		return;
	
	g_Render->StartRender(1, 1, 0);

	if (g_currentGUI == GUI_MAIN_SCREEN)
		g_Render->ProcessGUI(g_mainGui, g_LMBDown, g_mouseX, g_mouseY, MainMenuCallback);
	else if (g_currentGUI == GUI_START_SCREEN)
		g_Render->ProcessGUI(g_startGui, g_LMBDown, g_mouseX, g_mouseY, MainMenuCallback);
	else
		g_Render->ProcessGUI(g_creditsGui, g_LMBDown, g_mouseX, g_mouseY, MainMenuCallback);

	g_Render->EndRendering();
}

VOID CALLBACK EventTimerProc(UINT uTimerID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
	::SetEvent(g_htickEvent);
}

void ProcessInput()
{
	if (!g_InputSystem) return;

	// Update all devices.
	g_InputSystem->UpdateDevices();

	// Keyboard Input.
	if (g_InputSystem->KeyUp(DIK_ESCAPE))
	{
		if(g_gameState == GS_MENU)
		{
			switch (g_currentGUI)
			{
			case GUI_MAIN_SCREEN:
				PostQuitMessage(0);
				break;
			default:
				g_currentGUI = GUI_MAIN_SCREEN;
			}
		}
		else if(g_gameState == GS_LEVEL)
		{
			GameReleaseAll();

			InitializeMainMenu();
			g_SoundSystem->Play(g_menuSound);
			g_gameState = GS_MENU;
			g_currentGUI = GUI_MAIN_SCREEN;
		}
	}

	// Mouse Input
	POINT pos = { 0, 0 };
	GetCursorPos(&pos);

	g_mouseX = pos.x;
	g_mouseY = pos.y;

	if (g_InputSystem->MouseButtonDown(STRANDED_LEFT_BUTTON))
		g_LMBDown = true;
	if (!g_InputSystem->MouseButtonDown(STRANDED_RIGHT_BUTTON))
		g_LMBDown = false;
}

void GameReleaseAll()
{
	if (!g_Render) return;

	g_Render->ReleaseAllStaticBuffers();
	g_Render->ReleaseAllXModels();
	g_Render->ReleaseAllGUIs();

	g_gameWorld.m_levelID = -1;
	g_gameWorld.m_skyBoxID = -1;

	g_mainGui = -1;
	g_startGui = -1;
	g_creditsGui = -1;
	g_currentGUI = GUI_MAIN_SCREEN;

	if (!g_SoundSystem) return;

	if (g_gameState != GS_MENU && g_gameState != GS_LEVEL)
		g_SoundSystem->Stop(g_menuSound);
}

/*
 * @brief: 等级加载
 * @param: 要加载的级别的文件名
 */
bool LoadLevel(const char *file)
{
	if (!g_Render || !file)
		return false;

	CCommandScript script;
	char command[MAX_COMMAND_SIZE];
	char param[MAX_PARAM_SIZE];

	if (!script.LoadScriptFile(file))
		return false;

	for (int i = 0; i < script.GetTotalLines(); i++)
	{
		script.ParseCommand(command);

		if (stricmp(command, SC_LOAD_SKY_BOX_AS_X) == 0)
		{

		}
		else if (stricmp(command, SC_LOAD_TERRAIN_AX_X) == 0)
		{

		}
		else if (stricmp(command, SC_LOAD_STATIC_MODEL_AS_X) == 0)
		{

		}
		else if (stricmp(command, SC_LOAD_ANIMATED_MODEL_AS_X) == 0)
		{

		}
		else if (stricmp(command, "#") == 0)
		{

		}
		else
		{
			char err[64];

			sprintf(err, "Error loading level on [%d].", script.GetCurrentLineNum() + 1);

			// TODO: Write to error log.

			script.Shutdown();
			return false;
		}

		script.MoveToNextLine();
	}

	script.MoveToStart();

	for (int i = 0; i < script.GetTotalLines(); i++)
	{
		script.ParseCommand(command);

		if (stricmp(command, SC_LOAD_SKY_BOX_AS_X) == 0)
		{
			script.ParseStringParam(param);

			if (g_gameWorld.m_skyBoxID == -1)
			{
				if (!g_Render->LoadXModel(param, &g_gameWorld.m_skyBoxID))
					return false;
			}
			else
			{
				if (!g_Render->LoadXModel(param, g_gameWorld.m_skyBoxID))
					return false;
			}
		}
		else if (stricmp(command, SC_LOAD_TERRAIN_AX_X) == 0)
		{
			script.ParseStringParam(param);

			if (g_gameWorld.m_levelID == -1)
			{
				if (!g_Render->LoadXModel(param, &g_gameWorld.m_levelID))
					return false;
			}
			else
			{
				if (!g_Render->LoadXModel(param, &g_gameWorld.m_levelID))
					return false;
			}
		}
		else if (stricmp(command, SC_LOAD_STATIC_MODEL_AS_X) == 0)
		{

		}
		else if (stricmp(command, SC_LOAD_ANIMATED_MODEL_AS_X) == 0)
		{

		}
		else if (stricmp(command, "#") == 0)
		{

		}

		script.MoveToNextLine();
	}

	script.Shutdown();

	return true;
}

void LevelRender()
{
	if (!g_Render || g_gameState != GS_LEVEL)
		return;

	g_Render->StartRender(1, 1, 0);

	// 渲染天空体
	g_Render->RenderXModel(g_gameWorld.m_skyBoxID);
	// 渲染地形
	g_Render->RenderXModel(g_gameWorld.m_levelID);

	g_Render->EndRendering();
}
