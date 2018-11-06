#ifndef __IRREGULAR_BUTTON_H__
#define __IRREGULAR_BUTTON_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"

/*
 * CIrregularButton: ������ť
 * �� Image ��� initwithImageFile() ������ʼ�� Image ����
 * ֻ�ڿ�ʼ����һ�Σ������ļ�IO. ��һ�� bool ���鱣��ÿ�����ص��Ƿ�͸����Ϊ0����Ϣ��
 * ÿ�δ�������¼�ʱ������bool�������жϵ���Ƿ���Ч.
 *
 * ��֧�� png RGBA8888 ��ʽ��ͼƬ
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
	// ����ͼƬ�ļ���
	std::string m_normalFile;

	int m_iNormalImageWidth;
	int m_iNormalImageHeight;

	bool* m_pNormalTRansparent;

	// ��Ҫ���� alpha ֵ�� ��͸ touch
	bool m_bNeedSwallowByAlpha;
};

#endif