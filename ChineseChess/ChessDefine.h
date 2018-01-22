#pragma once

enum E_CHESS_TYPE
{
	CHESS_PAWN = 1,					// ������
	CHESS_CANNON = 2,				// ��
	CHESS_CHARIOT = 3,				// ��
	CHESS_HORSE = 4,				// ��
	CHESS_ELEPHANT = 5,				// ��
	CHESS_MILITARY_COUNSELOR = 6,	// ʿ
	CHESS_KING = 7,					// ˧
	CHESS_INVALID = 8,				// ��ʾ������Ч
};

struct SChess
{
	int m_type;			// 1 ��; 2 ��; 3 ��; 4 ��; 5 ��; 6 ʿ; 7 ˧; ����7��ʾ������Ч
	bool m_color;		// false ����(0�����); true ����(1��)
	int m_x, m_y;
};