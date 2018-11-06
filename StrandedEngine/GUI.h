/*
 * 字体和静态几何图形缓存的创建、删除和管理
 */
#ifndef __GUI_H__
#define __GUI_H__

#include "defines.h"

/*
 * GUI控件属性:
 *		控件类型，控件ID，颜色，起始X和Y坐标位置，
 *		按钮的宽度和高度，静态文本标签的现实文本，
 *		背景图，按钮正常、按下以及鼠标在其上面时显示的不同图像，
 *		链表 ID
 */
struct stGUIControl
{
	// Control type, id, and color.
	int m_type, m_id;
	unsigned long m_color;

	// If text then this is the font id, else static buffer id.
	int m_listID;

	// Start pos of object, width and height of button.
	int m_xPos, m_yPos, m_width, m_height;

	// Text of static text.
	char *m_text;

	// Button's up, down, and over textures.
	// m_upTex also used for backdrop image.
	int m_upTex, m_downTex, m_overTex;
};

/*
 * GUI界面属性: 
 *		Direct3D 设备指针，Direct3D字体对象列表，存储的字体总数，
 *		控件链表，界面中存储的控件总数，顶点缓存链表，顶点缓存总数，
 *		背景控件，背景顶点控件，窗口的宽度和高度
 */
class CGUISystem
{
public:
	CGUISystem() : m_controls(0), m_totalControls(0), m_backDropID(-1){}
	~CGUISystem() {Shutdown();}

	int IncreaseControls();
	bool AddBackdrop(int texID, int staticID);
	bool AddStaticText(int id, char *text, int x, int y, unsigned long color, int fontID);
	bool AddButton(int id, int x, int y, int width, int height, int upID, int overID, int downID, unsigned int staticID);
	void Shutdown();

	stGUIControl *GetGUIControl(int id)
	{
		if(id < 0 || id >= m_totalControls)
			return NULL;

		return &m_controls[id];
	}

	int GetTotalControls()
	{
		return m_totalControls;
	}

	stGUIControl *GetBackDrop()
	{
		if(m_backDropID >= 0 && m_totalControls)
			return &m_controls[m_backDropID];

		return NULL;
	}
private:
	stGUIControl *m_controls;
	int m_totalControls;
	int m_backDropID;
};

#endif