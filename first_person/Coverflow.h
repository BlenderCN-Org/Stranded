#ifndef __COVER_FLOW_H__
#define __COVER_FLOW_H__

#include "cocos2d.h"
#include "extensions/cocos-ext.h"

/*
 * coverflow Ч��
 * @sample:
 	auto cover = CCoverflow::Create(cocos2d::Rect(0.0f, 0.0f, conSize.width, conSize.height), cocos2d::Size(conSize.width*3, conSize.width), disSpace, 0.2f);
	cover->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE_LEFT);
	container_polish->addChild(cover);
	for(int i = 0; i<12; i++)
	{
		auto buttonSp = createSp("a.png");
		cover->AddCard(bottomSp, 1, ++baseTAg);
	}
	cover->setIsHorizontal(true);
	cover->StartMiddleIndex(2);
	cover->m_selCallback = [&](cocos2d::Node* card, int selIndex)
	{
		cocos2d::long("card index:[%d]", selIndex);
	}
 */
class CCoverflow : public cocos2d::Node, public cocos2d::extension::ScrollViewDelegate
{
public:
	CCoverflow();
	virtual ~CCoverflow();

public:
	typedef std::function<void(cocos2d::Node* card, int selIndex)> SelectListener;
	typedef std::function<void()> StartListener;

public:
	/*
	 * @param: scrollView �� rect
	 * @param: scrollView �� size
	 * @param: card ֮��ľ���
	 * @param: card ֮�������
	 */
	static CCoverflow* Create(const cocos2d::Rect& swBox, const cocos2d::Size& slSize, float disDistance, float disScale);

protected:
	virtual bool Init(const cocos2d::Rect& swBox, const cocos2d::Size& slSize, float disDistance, float disScale);
	void InitData();

	bool onTouchBegan(cocos2d::Touch* touch, cocos2d::Event* event);
	void onTouchMoved(cocos2d::Touch* touch, cocos2d::Event* event);
	void onTouchEnded(cocos2d::Touch* touch, cocos2d::Event* event);
	void onTouchCancelled(cocos2d::Touch* touch, cocos2d::Event* event);

	// �ƶ���ɻص�����
	void CardViewEndCallback(cocos2d::Node* pSender);

public:
	virtual void onEnter();
	virtual void onExit();

	// inherit cocos2d::extension::ScrollViewDElegate
	void scrollViewDidScroll(cocos2d::extension::ScrollView* view);
	void scrollViewDidZoom(cocos2d::extension::ScrollView* view);

	void AdjustCardScale(const cocos2d::Point& adjustPoint);
	void AdjustScrollView(const cocos2d::Point& adjustPoint);
	void AdjustEndScrollView();

	// ��ǰ�м� card ����(��0��ʼ)
	int GetCurCardIndex();
	void AddCard(cocos2d::Node* card);
	void AddCard(cocos2d::Node* card, int zOrder);
	void AddCard(cocos2d::Node* card, int zOrder, int tag);
	cocos2d::Node* GetCardByIndex(int index);
	void StartMiddleIndex(int index);

	//  �Ƿ�����ӦTouch
	CC_SYNTHESIZE(bool, m_canResponseTouch, ResponseTouch);

	// scrollView λ��
	CC_SYNTHESIZE_PASS_BY_REF(cocos2d::Point, m_swPosition, SwPosition);
	// scrollView ��С
	CC_SYNTHESIZE_PASS_BY_REF(cocos2d::Size, m_swSize, SwSize);
	// scrolllayer ��С
	CC_SYNTHESIZE_PASS_BY_REF(cocos2d::Size, m_slSize, SlSize);
	// card ������
	CC_SYNTHESIZE(float, m_disDistance, DisDistance);
	// card ���ż��
	CC_SYNTHESIZE(float, m_disScale, DisScale);
	// scrollView �߿�
	CC_SYNTHESIZE_PASS_BY_REF(cocos2d::Rect, m_swBox, SwBox);
	// scrollView �� containLayer
	CC_SYNTHESIZE(cocos2d::Layer*, m_scrollLayer, ScrollLayer);
	// card ����
	CC_SYNTHESIZE(int, m_cardNum, CardNum);

	// card ��ʼλ��
	CC_SYNTHESIZE_PASS_BY_REF(cocos2d::Point, m_offsetPosition, OffsetPosition);

	// containLayer ��λ��
	CC_SYNTHESIZE_PASS_BY_REF(cocos2d::Point, m_slayerPosition, SlayerPosition);
	// �Ƿ������ƶ������ڼ��� Menu ��ʱ���ܻ�������ȷ��Ӧ�¼�
	CC_SYNTHESIZE(bool, m_isMove, IsMove);
	// ��ˮƽ���Ǵ�ֱ
	CC_SYNTHESIZE(bool, m_isHorizontal, IsHorizontal);
	CC_SYNTHESIZE(bool, m_isIgnoreScale, IsIgnoreScale);

	// ѡ�м����¼�
	CC_SYNTHESIZE_PASS_BY_REF(SelectListener, m_selCallback, SelCallback);
	// ��ʼ�����¼�
	CC_SYNTHESIZE_PASS_BY_REF(StartListener, m_startCallback, StartCallback);

private:
	std::vector<cocos2d::Node*> m_cardArray;
	cocos2d::extension::ScrollView* m_scrollView;

	// �����仯����
	float m_fDarkRate;
};

#endif