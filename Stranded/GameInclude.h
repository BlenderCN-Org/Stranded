#ifndef __GAME_INCLUDE_H__
#define __GAME_INCLUDE_H__

#include "../Common/GlobalMsg.h"
#include "../Common/SException.h"
#include "../StrandedEngine/engine.h"

// Main menu defines.
#define GUI_MAIN_SCREEN		1
#define GUI_START_SCREEN	2
#define GUI_CREDITS_SCREEN	3

// ids for our GUI controls.
#define STATIC_TEXT_ID		1
#define BUTTON_START_ID		2
#define BUTTON_CREDITS_ID	3
#define BUTTON_QUIT_ID		4
#define BUTTON_BACK_ID		5
#define BUTTON_LEVEL_1_ID	6


//////////////////////////////////////////////////////////////////////////

// 存储整个等级(对象、环境等)
struct stGameWorld
{
	stGameWorld() : m_levelID(-1), m_skyBoxID(-1) {}

	// Level id and skydome.
	int m_levelID;
	int m_skyBoxID;
};

// Game states.
enum GAME_STATE
{
	GS_MENU = 1,
	GS_LEVEL,
	GS_LEVEL_1_SWITCH
};


#endif
