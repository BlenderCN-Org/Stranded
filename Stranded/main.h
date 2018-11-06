#ifndef __MAIN_H__
#define __MAIN_H__

#include "GameInclude.h"


#define WINDOW_CLASS	"StrandedGame"
#define WINDOW_NAME		"Stranded"
#define WIN_WIDTH		800
#define WIN_HEIGHT		600
#define FULLSCREEN		0


// Function Prototypes...
bool InitializeEngine();
void ShutdownEngine();

// Menu functions.
bool InitializeMainMenu();
void MainMenuCallback(int id, int state);
void MainMenuRender();

// Main game functions.
bool GameInitialize();
void GameLoop();
void MainLoop();
void GameShutdown();
void ProcessInput();
void GameReleaseAll();

// Level loading and rendering.
bool LoadLevel(const char *file);
void LevelRender();

// extern Globals...
extern HWND g_hwnd;
extern HINSTANCE g_hInstance;
extern CRenderInterface *g_Render;
extern CInputInterface *g_InputSystem;
extern CSoundSystemInterface *g_SoundSystem;

// GUI ids.
extern int g_mainGui;
extern int g_startGui;
extern int g_creditsGui;
extern int g_currentGUI;

// Font id.
extern int g_arialID;

// Temp Mouse state information.
extern bool g_LMBDown;
extern int g_mouseX;
extern int g_mouseY;

// 存储声音系统中主菜单音乐的 ID
extern int g_menuSound;

// Game level.
extern stGameWorld g_gameWorld;
extern GAME_STATE g_gameState;

// 系统Tick驱动事件
extern MMRESULT g_hEventTimer;
// Tick通知Event
extern HANDLE g_htickEvent;
// 系统Tick驱动函数
VOID CALLBACK EventTimerProc(UINT uTimerID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2);

#endif