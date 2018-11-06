#ifndef __COVER_FLOW_H__
#define __COVER_FLOW_H__

#include "cocos2d.h"
#include "extensions/cocos-ext.h"

/*
 * coverflow 效果
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
	 * @param: scrollView 的 rect
	 * @param: scrollView 的 size
	 * @param: card 之间的距离
	 * @param: card 之间的缩放
	 */
	static CCoverflow* Create(const cocos2d::Rect& swBox, const cocos2d::Size& slSize, float disDistance, float disScale);

protected:
	virtual bool Init(const cocos2d::Rect& swBox, const cocos2d::Size& slSize, float disDistance, float disScale);
	void InitData();

	bool onTouchBegan(cocos2d::Touch* touch, cocos2d::Event* event);
	void onTouchMoved(cocos2d::Touch* touch, cocos2d::Event* event);
	void onTouchEnded(cocos2d::Touch* touch, cocos2d::Event* event);
	void onTouchCancelled(cocos2d::Touch* touch, cocos2d::Event* event);

	// 移动完成回调函数
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

	// 当前中间 card 索引(从0开始)
	int GetCurCardIndex();
	void AddCard(cocos2d::Node* card);
	void AddCard(cocos2d::Node* card, int zOrder);
	void AddCard(cocos2d::Node* card, int zOrder, int tag);
	cocos2d::Node* GetCardByIndex(int index);
	void StartMiddleIndex(int index);

	//  是否能响应Touch
	CC_SYNTHESIZE(bool, m_canResponseTouch, ResponseTouch);

	// scrollView 位置
	CC_SYNTHESIZE_PASS_BY_REF(cocos2d::Point, m_swPosition, SwPosition);
	// scrollView 大小
	CC_SYNTHESIZE_PASS_BY_REF(cocos2d::Size, m_swSize, SwSize);
	// scrolllayer 大小
	CC_SYNTHESIZE_PASS_BY_REF(cocos2d::Size, m_slSize, SlSize);
	// card 距离间隔
	CC_SYNTHESIZE(float, m_disDistance, DisDistance);
	// card 缩放间隔
	CC_SYNTHESIZE(float, m_disScale, DisScale);
	// scrollView 边框
	CC_SYNTHESIZE_PASS_BY_REF(cocos2d::Rect, m_swBox, SwBox);
	// scrollView 的 containLayer
	CC_SYNTHESIZE(cocos2d::Layer*, m_scrollLayer, ScrollLayer);
	// card 索引
	CC_SYNTHESIZE(int, m_cardNum, CardNum);

	// card 起始位置
	CC_SYNTHESIZE_PASS_BY_REF(cocos2d::Point, m_offsetPosition, OffsetPosition);

	// containLayer 的位置
	CC_SYNTHESIZE_PASS_BY_REF(cocos2d::Point, m_slayerPosition, SlayerPosition);
	// 是否发生了移动，用于加入 Menu 的时候能滑动并正确响应事件
	CC_SYNTHESIZE(bool, m_isMove, IsMove);
	// 是水平还是垂直
	CC_SYNTHESIZE(bool, m_isHorizontal, IsHorizontal);
	CC_SYNTHESIZE(bool, m_isIgnoreScale, IsIgnoreScale);

	// 选中监听事件
	CC_SYNTHESIZE_PASS_BY_REF(SelectListener, m_selCallback, SelCallback);
	// 开始监听事件
	CC_SYNTHESIZE_PASS_BY_REF(StartListener, m_startCallback, StartCallback);

private:
	std::vector<cocos2d::Node*> m_cardArray;
	cocos2d::extension::ScrollView* m_scrollView;

	// 明暗变化比率
	float m_fDarkRate;
};

#endif