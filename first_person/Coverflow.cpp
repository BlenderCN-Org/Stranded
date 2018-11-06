#include "Coverflow.h"
#include "cocos2d/LuaScriptHandlerMgr.h"

/*
 * @Lua:
 *
	local coverflow = CCoverflow::Create(cc.rect(0, 0, width, height), cc.size(width*3, height), 160, 0.1);
	coverflow:setAnchorPoint(cc.p(0, 0));
	coverflow:setPosition(cc.p(0, 120));
	CardNode:addChild(coverflow);

	for i=1, #data do
		local card = CareateCard(i);
		coverflow:AddCard(card, 1, i);
	end

	coverflow:setIsHorizontal(true);
	coverflow:StartMiddleIndex(curIndex);
	coverflow:RegisterCoverflowEventHandler(function(card, selIndex)
		print("EVENT_COVERFLOW_SELECT (END). selIndex=[" .. selIndex .. "]");
		curIndex = selIndex;
		if card then
			print("card index = ", card:getTag());
		end
		......
	end, cc.Handler.EVENT_COVERFLOW_SELECT);
	coverflow:RegisterCoverflowEventHandler(function()
		print("EVENT_COVERFLOW_START");
		......
	end, cc.Handler.EVENT_COVERFLOW_START);

	coverflow:setResponseTouch(true);

	......

	coverflow:UnregisterCoverflowEventHandler(cc.Handler.EVENT_COVERFLOW_SELECT);
	coverflow:UnregisterCoverflowEventHandler(cc.Handler.EVENT_COVERFLOW_START);

	...................................................

	local function onTouchMoved(touch, event)
		local pt = touch:getLocation()
		pt = spr:getParent():convertToNodeSpace(pt)
		local rectOrigin = spr:getBoundingBox()
		if cc.rectContainsPoint(rectOrigin, pt) then
			local deltaPoint = touch.getDelta();
			if deltaPoint then
				if math.abs (deltaPoint.x) > 0 or math.abs(deltaPoint.y) > 0 then
					... move = true....
				end
			end
		end
	end
 */

// 每张卡牌 变暗 需要传入的灰度层 tag
static const int si_GRAY_LAY_TAG = 2000;

CCoverflow::CCoverflow() :
	m_selCallback(nullptr),
	m_startCallback(nullptr)
{

}

CCoverflow::~CCoverflow()
{
	if (m_selCallback)
	{
		cocos2d::ScriptHandlerMgr::HandlerType handlerType = cocos2d::ScriptHandlerMgr::HandlerType::EVENT_COVERFLOW_SELECT;
		cocos2d::ScriptHandlerMgr::getInstance()->removeObjectHandler((void*)this, handlerType);

		m_selCallback = nullptr;
	}

	if (m_startCallback)
	{
		cocos2d::ScriptHandlerMgr::HandlerType handlerType = cocos2d::ScriptHandlerMgr::HandlerType::EVENT_COVERFLOW_START;
		cocos2d::ScriptHandlerMgr::getInstance()->removeObjectHandler((void*)this, handlerType);

		m_startCallback = nullptr;
	}
}

CCoverflow* CCoverflow::Create(const cocos2d::Rect& swBox, const cocos2d::Size& slSize, float disDistance, float disScale)
{
	CCoverflow* cover = new CCoverflow();
	if (cover && cover->Init(swBox, slSize, disDistance, disScale))
	{
		cover->autorelease();
		return cover;
	}

	CC_SAFE_DELETE(cover);
	return nullptr;
}

bool CCoverflow::Init(const cocos2d::Rect& swBox, const cocos2d::Size& slSize, float disDistance, float disScale)
{
	if (!cocos2d::Node::init())
		return false;

	this->m_swBox = swBox;
	this->m_swPosition = swBox.origin;
	this->m_swSize = swBox.size;
	this->m_slSize = slSize;
	this->m_disDistance = disDistance;
	this->m_disSCale = disScale;

	this->m_canResponseTouch = true;

	InitData();

	return true;
}

void CCoverflow::InitData()
{
	m_cardNum = 0;
	m_selCallback = nullptr;
	m_startCallback = nullptr;
	// m_isIgnoreScale = abs(m_disScale) < 0.0001f; 浮点数安卓平台判断可能有问题
	m_isIgnoreScale = (abs(m_disScale) < 0);

	m_fDarkRate = 0.3f;

	m_offsetPosition = cocos2d::Vec2(m_swSize.width / 2, m_swSize.height / 2);

	m_scrollLayer = cocos2d::Layer::create();
	m_scrollLayer->setAnchorPoint(cocos2d::Vec2::ZERO);
	m_scrollLayer->setPosition(cocos2d::Vec2::ZERO);
	m_scrollLayer->setContentSize(m_slSize);;

	m_slayerPosition = cocos2d::Vec2::ZERO;
	m_isMove = true;
	m_isHorizontal = false;

	m_scrollView = cocos2d::extension::ScrollView::create(m_swSize, m_scrollLayer);
	m_scrollView->setAnchorPoint(cocos2d::Vec2::ZERO);
	m_scrollView->setContentOffset(cocos2d::Vec2::ZERO);
	m_scrollView->setTouchEnabled(false);
	m_scrollView->setDelegate(this);
	//m_scrollView->setClippingToBounds(false);
	m_scrollView->setDirection(cocos2d::extension::ScrollView::Direction::VERTICAl);
	addChild(m_scrollView, 1);
}

void CCoverflow::onEnter()
{
	cocos2d::Node::onEnter();

	auto dispatcher = cocos2d::Director::getInstance->getEventDispatcher();
	auto listener = cocos2d::EventListenerTouchOneByOne::create();
	listener->onTouchBegan = CC_CALLBACK_2(CCoverflow::onTouchBegan, this);
	listener->onTouchMoved = CC_CALLBACK_2(CCoverflow::onTouchMoved, this);
	listener->onTouchEnded = CC_CALLBACK_2(CCoverflow::onTouchEnded, this);
	listener->onTouchCancelled = CC_CALLBACK_2(CCoverflow::onTouchCancelled, this);
	listener->setSwallowTouches(true);
	listener->addEventListenerWithSceneGraphPriority(listener, this);
}

void CCoverflow::onExit()
{
	removeAllChildren();
	cocos2d::Node::onExit();
}

bool CCoverflow::onTouchBegan(cocos2d::Touch* touch, cocos2d::Event* event)
{
	if (!m_canResponseTouch)
		return false;

	if (touch && event)
	{
		auto target = static_cast<cocos2d::Node*>(event->getCurrentTarget());
		// 获取当前点击点所在相对按钮的位置坐标
		cocos2d::Point locationInNode = target->convertToNodeSpcae(touch->getLocation());
		if (m_swBox.containsPoint(locationInNode))
		{
			if (m_startCallback)
			{
				m_startCallback();
			}

			return true;
		}
	}

	return false;
}

void CCoverflow::onTouchMoved(cocos2d::Touch* touch, cocos2d::Event* event)
{
	if (!m_canResponseTouch)
		return;

	if (touch && event)
	{
		cocos2d::Vec2 scrollPrePoint = touch->getPreviousLocation();
		cocos2d::Vec2 scrollMovePoint = touch->getLocation();

		// 获取事件所绑定的 target
		auto target = static_cast<cocos2d::Node*>(event->getCurrentTarget());
		// 获取当前点击点所在相对按钮的位置坐标
		cocos2d::Point locationInNode = target->convertToNodeSpcae(touch->getLocation());
		if (m_swBox.containsPoint(locationInNode))
		{
			cocos2d::Vec2 adjustPoint = scrollMovePoint - scrollPrePoint;
			m_scrollLayer->stopAllActions();
			AdjustScrollView(adjustPoint);
			AdjustCardScale(adjustPoint);
		}
	}
}

void CCoverflow::onTouchEnded(cocos2d::Touch* touch, cocos2d::Event* event)
{
	if (!m_canResponseTouch)
		return;

	if (touch && event)
	{
		//cocos2d::Vec2 scrollPrePoint = touch->getPreviousLocation();
		//cocos2d::Vec2 scrollEndPoint = touch->getLocation();
		//float disX = scrollEndPoint.x - scrollPrePoint.x;
		m_scrollLayer->stopAllActions();
		AdjustEndScrollView();
		const cocos2d::Vec2& curPosition = m_scrollLayer->getPosition();
		float distance = m_slayerPosition.distance(curPosition);
		if (distance < 5.0f)
			m_isMove = false;
	}
}

void CCoverflow::onTouchCancelled(cocos2d::Touch* touch, cocos2d::Event* event)
{
	if (!m_canResponseTouch)
		return;

	onTouchEnded(touch, event);
}

void CCoverflow::AdjustCardScale(const cocos2d::Point& adjustPoint)
{
	if (!m_canResponseTouch)
		return;

	if (m_isHorizontal)
	{
		for (auto card : m_cardArray)
		{
			if (card)
			{
				float offset = m_scrollView->getContentOffset().x;
				float posX = card->getPositionX() + offset;
				float disMid = abs(m_swSize.width / 2 - posX);
				float scale = 1.0f - disMid / m_disDistance * m_disScale;
				card->setScale(scale);
				int zOr = (int)(1000.0f - disMid*0.1f);
				card->setLocalZOrder(zOr);

				auto layerColor = card->getChildByTag(si_GRAY_LAY_TAG);
				if (layerColor != nullptr)
				{
					float fDarkRate = m_fDarkRate * disMid / m_disDistance;
					unsigned char byAlpha = (unsigned char)(255 * fDarkRate);
					if (byAlpha > 255)
						byAlpha = 255;
					layerColor->setOpacity(byAlpha);
				}
			}
		}
	}
	else
	{
		for (auto card : m_cardArray)
		{
			if (card)
			{
				float offset = m_scrollView->getContentOffset().y;
				float posY = card->getPositionY() + offset;
				float disMid = abs(m_swSize.height / 2 - posY);
				float scale = 1.0f - disMid / m_disDistance * m_disScale;
				card->setScale(scale);
				int zOr = (int)(1000.0f - disMid*0.1f);
				card->setLocalZOrder(zOr);

				auto layerColor = card->getChildByTag(si_GRAY_LAY_TAG);
				if (layerColor != nullptr)
				{
					float fDarkRate = m_fDarkRate * disMid / m_disDistance;
					unsigned char byAlpha = (unsigned char)(255 * fDarkRate);
					if (byAlpha > 255)
						byAlpha = 255;
					layerColor->setOpacity(byAlpha);
				}
			}
		}
	}
}

void CCoverflow::AdjustScrollView(const cocos2d::Point& adjustPoint)
{
	cocos2d::Vec2 endPoint = m_scrollView->getContentOffset() + cocos2d::Vec2(0.0f, adjustPoint.y);
	if (m_isHorizontal)
		endPoint = m_scrollView->getContentOffset() + cocos2d::Vec2(adjustPoint.x, 0);
	m_scrollView->unscheduleAllCallbacks();
	m_scrollView->setContentOffset(endPoint, false);
}

void CCoverflow::AdjustEndScrollView()
{
	if (m_isHorizontal)
	{
		float minX = (float)LONG_MAX;
		float midX = m_swSize.width / 2;
		// 获取距离中间最小值得 card
		for (auto card : m_cardArray)
		{
			if (card)
			{
				float offset = m_scrollView->getContentOffset().x;
				// 转化父类坐标
				float posX = card->getPositionX() + offset;
				float disMid = midX - posX;
				if (abs(disMid) < abs(minX))
					minX = disMid;
			}
		}

		for (auto item : m_cardArray)
		{
			if (item)
			{
				// 转化父类坐标
				float offset = m_scrollView->getContentOffset().x;
				float posX = item->getPositionX() + offset;
				// 距离中间长度
				float disMid = abs(midX - posX - minX);
				if (m_isIgnoreScale == false)
				{
					// 目标scale
					float scale = 1.0f - disMid / m_disDistance * m_disScale;
					cocos2d::ScaleTo* scaleTo = cocos2d::ScaleTo::create(0.2f, scale);
					item->runAction(scaleTo);

					auto layerColor = item->getChildByTag(si_GRAY_LAY_TAG);
					if (layerColor != nullptr)
					{
						float fDarkRate = m_fDarkRate * disMid / m_disDistance;
						unsigned char byAlpha = (unsigned char)(255 * fDarkRate);
						if (byAlpha > 255)
							byAlpha = 255;
						cocos2d::FadeTo* fadeTo = cocos2d::FadeTo::create(0.2f, byAlpha);
						layerColor->runAction(fadeTo);
					}
				}

				int zOr = (int)(1000.0f - disMid*0.1f);
				item->setLocalZOrder(zOr);
			}
		}

		cocos2d::Layer* scrollLayer = (cocos2d::Layer*)m_scrollView->getContainer();
		cocos2d::MoveBy* moveBy = cocos2d::MoveBy::create(0.2f, cocos2d::Vec2(minX, 0.0f));
		cocos2d::CallFuncN* callFuncN = cocos2d::CallFuncN::create(std::bind(&CCoverflow::CardViewEndCallback, this, std::placeholders::_1));
		cocos2d::Sequence* seq = cocos2d::Sequence::create(moveBy, callFuncN, NULL);
		scrollLayer->runAction(seq);
	}
	else
	{
		float minY = (float)LONG_MAX;
		float midY = m_swSize.height / 2;
		// 获取距离中间最小值得 card
		for (auto card : m_cardArray)
		{
			if (card)
			{
				float offset = m_scrollView->getContentOffset().y;
				// 转化父类坐标
				float posY = card->getPositionY() + offset;
				float disMid = midY - posY;
				if (abs(disMid) < abs(minY))
					minY = disMid;
			}
		}

		for (auto item : m_cardArray)
		{
			if (item)
			{
				// 转化父类坐标
				float offset = m_scrollView->getContentOffset().y;
				float posY = item->getPositionY() + offset;
				// 距离中间长度
				float disMid = abs(midY - posY - minY);
				if (m_isIgnoreScale == false)
				{
					// 目标scale
					float scale = 1.0f - disMid / m_disDistance * m_disScale;
					cocos2d::ScaleTo* scaleTo = cocos2d::ScaleTo::create(0.2f, scale);
					item->runAction(scaleTo);

					auto layerColor = item->getChildByTag(si_GRAY_LAY_TAG);
					if (layerColor != nullptr)
					{
						float fDarkRate = m_fDarkRate * disMid / m_disDistance;
						unsigned char byAlpha = (unsigned char)(255 * fDarkRate);
						if (byAlpha > 255)
							byAlpha = 255;
						cocos2d::FadeTo* fadeTo = cocos2d::FadeTo::create(0.2f, byAlpha);
						layerColor->runAction(fadeTo);
					}
				}

				int zOr = (int)(1000.0f - disMid*0.1f);
				item->setLocalZOrder(zOr);
			}
		}

		cocos2d::Layer* scrollLayer = (cocos2d::Layer*)m_scrollView->getContainer();
		cocos2d::MoveBy* moveBy = cocos2d::MoveBy::create(0.2f, cocos2d::Vec2(0.0f, minY));
		cocos2d::CallFuncN* callFuncN = cocos2d::CallFuncN::create(std::bind(&CCoverflow::CardViewEndCallback, this, std::placeholders::_1));
		cocos2d::Sequence* seq = cocos2d::Sequence::create(moveBy, callFuncN, NULL);
		scrollLayer->runAction(seq);
	}
}

void CCoverflow::CardViewEndCallback(cocos2d::Node* pSender)
{
	m_slayerPosition = m_scrollView->getPosition();
	m_isMove = true;

	if (m_selCallback)
	{
		int index = GetCurCardIndex();
		m_selCallback(GetCardByIndex(index), index);
	}
}

void CCoverflow::scrollViewDidScroll(cocos2d::extension::ScrollView* view)
{

}

void CCoverflow::scrollViewDidZoom(cocos2d::extension::ScrollView* view)
{

}

void CCoverflow::AddCard(cocos2d::Node* card)
{
	int zOrder = 1000.0f - m_cardNum;
	this->AddCard(card, zOrder, 0);
}

void CCoverflow::AddCard(cocos2d::Node* card, int zOrder)
{
	this->AddCard(card, zOrder, 0);
}

void CCoverflow::AddCard(cocos2d::Node* card, int zOrder, int tag)
{
	if (card == nullptr)
		return;

	if (m_isHorizontal)
	{
		float positionX = m_offsetPosition.x + m_disDistance*m_cardNum;
		float scale = 1.0f - m_disScale*m_cardNum;
		card->setPosition(cocos2d::Vec2(positionX, m_offsetPosition.y));
		card->setScale(scale);

		auto layerColor = item->getChildByTag(si_GRAY_LAY_TAG);
		if (layerColor != nullptr)
		{
			float fDarkRate = m_fDarkRate * m_cardNum;
			unsigned char byAlpha = (unsigned char)(255 * fDarkRate);
			if (byAlpha > 255)
				byAlpha = 255;
			layerColor->setOpacity(byAlpha);
		}

		m_cardArray.push_back(card);
		m_scrollLayer->addChild(card, zOrder, tag);
		m_cardNum++;
	}
	else
	{
		float positionY = m_offsetPosition.y + m_disDistance*m_cardNum;
		float scale = 1.0f - m_disScale*m_cardNum;
		card->setPosition(cocos2d::Vec2(m_offsetPosition.x, positionY));
		card->setScale(scale);

		auto layerColor = item->getChildByTag(si_GRAY_LAY_TAG);
		if (layerColor != nullptr)
		{
			float fDarkRate = m_fDarkRate * m_cardNum;
			unsigned char byAlpha = (unsigned char)(255 * fDarkRate);
			if (byAlpha > 255)
				byAlpha = 255;
			layerColor->setOpacity(byAlpha);
		}

		m_cardArray.push_back(card);
		m_scrollLayer->addChild(card, zOrder, tag);
		m_cardNum++;
	}
}

cocos2d::Node* CCoverflow::GetCardByIndex(int index)
{
	if (index < 0 || index >(int)(m_cardArray.size() - 1))
		return nullptr;

	return m_cardArray[index];
}

int CCoverflow::GetCurCardIndex()
{
	if (m_isHorizontal)
	{
		float distance1 = -m_scrollLayer->getPositionX();
		float distance2 = m_swSize.width / 2 - m_offsetPosition.x;
		// +5 浮点数误差
		int index = (distance1 + distance2 + 5) / m_disDistance;
		index = std::min((int)m_cardArray.size() - 1, index);
		index = std::max(0, index);

		return index;
	}
	else
	{
		float distance1 = -m_scrollLayer->getPositionY();
		float distance2 = m_swSize.height / 2 - m_offsetPosition.y;
		// +5 浮点数误差
		int index = (distance1 + distance2 + 5) / m_disDistance;
		index = std::min((int)m_cardArray.size() - 1, index);
		index = std::max(0, index);

		return index;
	}

	return 0;
}

void CCoverflow::setOffsetPosition(const cocos2d::Point& var)
{
	if (m_isHorizontal)
	{
		m_offsetPosition = var;
		m_cardNum = 0;

		for (auto card : m_cardArray)
		{
			if (card)
			{
				float positionX = m_offsetPosition.x + m_disDistance * m_cardNum;
				card->setPosition(cocos2d::Vec2(positionX, m_offsetPosition.y);
				m_cardNum++;
			}
		}

		AdjustCardScale(cocos2d::Vec2::ZERO);
	}
	else
	{
		m_offsetPosition = var;
		m_cardNum = 0;

		for (auto card : m_cardArray)
		{
			if (card)
			{
				float positionY = m_offsetPosition.y + m_disDistance * m_cardNum;
				card->setPosition(cocos2d::Vec2(m_offsetPosition.x, positionY);
				m_cardNum++;
			}
		}

		AdjustCardScale(cocos2d::Vec2::ZERO);
	}
}

const cocos2d::Point& CCoverflow::getOffsetPosition() const
{
	return m_offsetPosition;
}

void CCoverflow::StartMiddleIndex(int index)
{
	int cardsCount = m_cardArray.size();
	if (index < 0 || index > cardsCount - 1)
		return;

	auto newOffsetPosition = m_offsetPosition + cocos2d::Vec2(0.0f, -m_disDistance*index);
	if(m_isHorizontal)
		newOffsetPosition = m_offsetPosition + cocos2d::Vec2(-m_disDistance*index, 0.0f);

	setOffsetPosition(newOffsetPosition);
}

int lua_CCoverflow_RegisterCoverflowEventHandler(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
	tolua_Error tolua_err;
	if (
		!tolua_isusertype(tolua_S, 1, "CCoverflow", 0, &tolua_err) ||
		!toluafix_isfunction(tolua_S, 2, "LUA_FUNCTION", 0, &tolua_err) ||
		!tolua_isnumber(tolua_S, 3, 0, &tolua_err) ||
		!tolua_isnoobj(tolua_S, 4, &tolua_err)
		)
		goto tolua_lerror;
	else
#endif
	{
		CCoverflow* self = (CCoverflow*)tolua_tousertype(tolua_S, 1, 0);
		if (self != nullptr)
		{
			int handler = toluafix_ref_function(tolua_S, 2, 0);

			cocos2d::ScriptHandlerMgr::HandlerType type = static_cast<cocos2d::ScriptHandlerMgr>((int)tolua_tonumber(tolua_S, 3, 0));
			switch (type)
			{
			case cocos2d::ScriptHandlerMgr::HandlerType::EVENT_COVERFLOW_SELECT:
			{
				self->setSelCallback([=](cocos2d::Node* card, int selIndex) {
					if (card == nullptr)
						return;

					int iSelHandle = cocos2d::ScriptHandlerMgr::getInstance()->getObjectHandler((void*)self, type);
					if (handler == 0)
						return;

					LuaStack* stack = LuaEngine::getInstance()->getLuaStack();
					if (nullptr == stack)
						return;

					stack->pushObject(card, "cc.Node");
					stack->pushInt(selIndex);

					int iRet = stack->executeFunctionByHandler(iSelHandle, 2);

					stack->clean();
				});

				cocos2d::ScriptHandlerMgr::getInstance()->addObjectHandler((void*)self, handler, type);
			}
				break;
			case cocos2d::ScriptHandlerMgr::HandlerType::EVENT_COVERFLOW_START:
			{
				self->setStartCallback([=]() {
					int iSelHandle = cocos2d::ScriptHandlerMgr::getInstance()->getObjectHandler((void*)self, type);
					if (handler == 0)
						return;

					LuaStack* stack = LuaEngine::getInstance()->getLuaStack();
					if (nullptr == stack)
						return;

					int iRet = stack->executeFunctionByHandler(iSelHandle, 2);

					stack->clean();
				});

				cocos2d::ScriptHandlerMgr::getInstance()->addObjectHandler((void*)self, handler, type);
			}
			break;
			default:
				break;
			}
		}
	}

	return 0;
#ifndef TOLUA_RELEASE
	tolua_lerror:
				tolua_error(tolua_S, "#ferror in function 'lua_CCoverflow_RegisterCoverflowEventHandler'.", &tolua_err);
				return 0;
#endif
}

int lua_CCoverflow_UnregisterCoverflowEventHandler(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
	tolua_Error tolua_err;
	if (
		!tolua_isusertype(tolua_S, 1, "CCoverflow", 0, &tolua_err) ||
		!tolua_isnumber(tolua_S, 2, 0, &tolua_err) ||
		!tolua_isnoobj(tolua_S, 3, &tolua_err)
		)
		goto tolua_lerror;
	else
#endif
	{
		CCoverflow* self = (CCoverflow*)tolua_tousertype(tolua_S, 1, 0);
		if (self != nullptr)
		{
			cocos2d::ScriptHandlerMgr::HandlerType type = static_cast<cocos2d::ScriptHandlerMgr>((int)tolua_tonumber(tolua_S, 2, 0));
			switch (type)
			{
			case cocos2d::ScriptHandlerMgr::HandlerType::EVENT_COVERFLOW_SELECT:
			{
				self->setSelCallback(nullptr);

				cocos2d::ScriptHandlerMgr::getInstance()->removeObjectHandler((void*)self, type);
			}
			break;
			case cocos2d::ScriptHandlerMgr::HandlerType::EVENT_COVERFLOW_START:
			{
				self->setStartCallback(nullptr);

				cocos2d::ScriptHandlerMgr::getInstance()->removeObjectHandler((void*)self, type);
			}
			break;
			default:
				break;
			}
		}
	}

	return 0;
#ifndef TOLUA_RELEASE
	tolua_lerror :
				 tolua_error(tolua_S, "#ferror in function 'lua_CCoverflow_UnregisterCoverflowEventHandler'.", &tolua_err);
				 return 0;
#endif
}

static void extendCCoverflow(lua_State* L)
{
	lua_pushstring(L, "CCoverflow");
	lua_rawget(L, LUA_REGISTRYINDEX);
	if (lua_istable(L, -1))
	{
		tolua_function(L, "RegisterCoverflowEventHandler", lua_CCoverflow_RegisterCoverflowEventHandler);
		tolua_function(L, "UnregisterCoverflowEventHandler", lua_CCoverflow_UnregisterCoverflowEventHandler);
	}
	lua_pop(L, 1);
}