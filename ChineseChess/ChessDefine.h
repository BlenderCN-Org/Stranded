#pragma once

enum E_CHESS_TYPE
{
	CHESS_PAWN = 1,					// 兵，卒
	CHESS_CANNON = 2,				// 炮
	CHESS_CHARIOT = 3,				// 车
	CHESS_HORSE = 4,				// 马
	CHESS_ELEPHANT = 5,				// 象
	CHESS_MILITARY_COUNSELOR = 6,	// 士
	CHESS_KING = 7,					// 帅
	CHESS_INVALID = 8,				// 表示棋子无效
};

struct SChess
{
	int m_type;			// 1 兵; 2 炮; 3 车; 4 马; 5 象; 6 士; 7 帅; 大于7表示棋子无效
	bool m_color;		// false 上面(0号玩家); true 下面(1号)
	int m_x, m_y;
};