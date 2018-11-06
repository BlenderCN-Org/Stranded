#ifndef __IRREGULAR_BUTTON_H__
#define __IRREGULAR_BUTTON_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"

/*
 * CIrregularButton: 不规则按钮
 * 用 Image 类的 initwithImageFile() 方法初始化 Image 对象
 * 只在开始创建一次，减少文件IO. 用一个 bool 数组保存每个像素点是否透明度为0的信息。
 * 每次触发点击事件时，根据bool数组来判断点击是否有效.
 *
 * 仅支持 png RGBA8888 格式的图片
 */

class CIrregularButton : public cocos2d::ui::Button
{
public:
	CIrregularButton();
	virtual ~CIrregularButton();

	static CIrregularButton* Create();
	static CIrregularButton* Create(const std::string& normalImage, const std::string& selectedImage = "", const std::string& disableImage = "", cocos2d::ui::Widget::TextureResType texType = cocos2d::ui::Widget::TextureResType::LOCAL);

	virtual bool HitTest(const cocos2d::Vec2& pt);

	virtual void SetSwallowByAlpha(bool swallow);

protected:
	/*
	 * A callback which will be called when touch began event is issued.
	 * @param: touch The touch info.
	 * @param: unusedEvent The touch event info.
	 * @return: True if user want to handle touches, false otherwise.
	 */
	virtual bool onTouchBegan(cocos2d::Touch *touch, cocos2d::Event *unusedEvent) override;
	/*
	* A callback which will be called when touch ended event is issued.
	* @param: touch The touch info.
	* @param: unusedEvent The touch event info.
	*/
	virtual void onTouchEnded(cocos2d::Touch *touch, cocos2d::Event *unusedEvent) override;

	virtual bool init() override;
	virtual bool init(const std::string& normalImage, const std::string& selectedImage = "", const std::string& disableImage = "", cocos2d::ui::Widget::TextureResType texType = cocos2d::ui::Widget::TextureResType::LOCAL) override;

	void LoadNormalTransparentInfo();
	bool GetIsTransparentAtPoint(cocos2d::Vec2 point);

private:
	// 保存图片文件名
	std::string m_normalFile;

	int m_iNormalImageWidth;
	int m_iNormalImageHeight;

	bool* m_pNormalTRansparent;

	// 需要根据 alpha 值， 穿透 touch
	bool m_bNeedSwallowByAlpha;
};

#endif