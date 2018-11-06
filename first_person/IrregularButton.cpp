#include "IrregularButton.h"

CIrregularButton::CIrregularButton() :
	cocos2d::ui::Button(),
	m_pNormalTRansparent(nullptr),
	m_bNeedSwallowByAlpha(false)
{

}

CIrregularButton::~CIrregularButton()
{
	if (m_pNormalTRansparent)
	{
		delete[] m_pNormalTRansparent;
		m_pNormalTRansparent = nullptr;
	}
}

CIrregularButton* CIrregularButton::Create()
{
	CIrregularButton* pBtn = new CIrregularButton;
	if (pBtn && pBtn->init())
	{
		pBtn->autorelease();
		return pBtn;
	}

	CC_SAFE_DELETE(pBtn);
	return nullptr;
}

CIrregularButton* CIrregularButton::Create(const std::string& normalImage, const std::string& selectedImage /*= ""*/, const std::string& disableImage /*= ""*/, cocos2d::ui::Widget::TextureResType texType /*= cocos2d::ui::Widget::TextureResType::LOCAL*/)
{
	CIrregularButton* pBtn = new CIrregularButton;
	if(pBtn && pBtn->init(normalImage, selectedImage, disableImage, texType))
	{
		pBtn->autorelease();
		return pBtn;
	}

	CC_SAFE_DELETE(pBtn);
	return nullptr;
}

bool CIrregularButton::init()
{
	if (cocos2d::ui::Button::init())
		return true;

	return false;
}

bool CIrregularButton::init(const std::string& normalImage, const std::string& selectedImage /*= ""*/, const std::string& disableImage /*= ""*/, cocos2d::ui::Widget::TextureResType texType /*= cocos2d::ui::Widget::TextureResType::LOCAL*/)
{
	bool bRet = true;
	do 
	{
		m_normalFile = normalImage;

		if (!cocos2d::ui::Button::init(normalImage, selectedImage, disableImage, texType))
		{
			bRet = false;
			break;
		}
	} while (0);

	LoadNormalTransparentInfo();

	return bRet;
}

void CIrregularButton::LoadNormalTransparentInfo()
{
	cocos2d::Image* pNormalImage = new cocos2d::Image();
	pNormalImage->initWithImageFile(m_normalFile);

	m_iNormalImageWidth = pNormalImage->getWidth();
	m_iNormalImageHeight = pNormalImage->getHeight();

	auto dataLen = pNormalImage->getDataLen();
	if (m_pNormalTRansparent != nullptr)
		delete[] m_pNormalTRansparent;

	auto normalPixels = pNormalImage->getData();
	m_pNormalTRansparent = new bool[dataLen / (sizeof(unsigned char) * 4)];
	for (auto i = 0; i < m_iNormalImageHeight; ++i)
	{
		for (auto j = 0; j < m_iNormalImageWidth; ++j)
		{
			// ���� cocos2dx 
			// m_pNormalTRansparent[i*m_iNormalImageWidth + j] = (normalPixels[(i*m_iNormalImageWidth + j) * 4] == 0);
			// �޸Ĺ��� cocos2dx ���棬ȥ���� CC_RGB_PREMULTIPLY_ALPHA ���������� alpha ��Ȼ��ԭʼλ��
			// r,g,b,alpha
			//printf("normalPixels=[%d]\n", normalPixels[(i*m_iNormalImageWidth+j)*4+3]);
			m_pNormalTRansparent[i*m_iNormalImageWidth + j] = (normalPixels[(i*m_iNormalImageWidth + j) * 4 + 3] == 0);
		}
	}

	delete pNormalImage;
}

bool CIrregularButton::GetIsTransparentAtPoint(cocos2d::Vec2 point)
{
	// cocos2dx ʹ�õ��� openGL ����ϵ��y �����Ǵ��µ��ϵ����ġ�
	// libpng ��ȡ png ͼƬʱ �Ǵ��ϵ������ж�ȡ�ġ�
	point.y = _buttonNormalRenderer->getContentSize().height - point.;
	// ת���� int
	int x = (int)point.x - 1;
	if (x < 0)
		x = 0;
	else if (x >= m_iNormalImageWidth)
		x = m_iNormalImageWidth - 1;

	int y = (int)point.y - 1;
	if (y < 0)
		y = 0;
	else if (y >= m_iNormalImageHeight)
		y = m_iNormalImageHeight - 1;

	return m_pNormalTRansparent[m_iNormalImageWidth*y + x];
}

bool CIrregularButton::HitTest(const cocos2d::Vec2& pt)
{
	// _buttonNormalRenderer - normal sprite
	cocos2d::Vec2 localLocation = _buttonNormalRenderer->convertToNodeSpace(pt);

	cocos2d::Rect validTouchedRect;
	validTouchedRect.size = _buttonNormalRenderer->getContentSize();
	if (validTouchedRect.containsPoint(localLocation) && GetIsTransparentAtPoint(localLocation) == false)
		return true;

	return false;
}

void CIrregularButton::SetSwallowByAlpha(bool swallow)
{
	m_bNeedSwallowByAlpha = swallow;
}

bool CIrregularButton::onTouchBegan(cocos2d::Touch *touch, cocos2d::Event *unusedEvent)
{
	if (m_bNeedSwallowByAlpha)
	{
		_hitted = false;
		if (isVisible() && isEnabled() && isAncestorsEnbaled() && isAncestorsVisible(this))
		{
			_touchBeganPosition = touch->getLocation();
			auto camera = cocos2d::Camera::getVisitingCamera();
			if (HitTest(_touchBeganPosition, camera, nullptr))
			{
				if (isClippingParentContainsPoint(_touchBeganPosition))
				{
					_hittedByCamera = camera;
					_hitted = true;
				}
			}
		}

		if (!_hitted)
		{
			return false;
		}

		// ���� alpha ֵ����
		if (!(this->HitTest(_touchBeganPosition)))
			return false;

		setHighlighted(true);

		/*
		 * Propagate touch events to its parents
		 */
		if (_propagateTouchEvents)
		{
			this->propagateTouchEvent(TouchEventType::BEGAN, this, touch);
		}

		pushDownEvent();

		return true;
	}
	else
		return cocos2d::ui::Widget::onTouchBegan(touch, unusedEvent);

	return false;
}

void CIrregularButton::onTouchEnded(cocos2d::Touch *touch, cocos2d::Event *unusedEvent)
{
	if (m_bNeedSwallowByAlpha)
	{
		_touchEndPosition = touch->getLocation();

		// ���� alpha ֵ����
		if (this->HitTest(_touchEndPosition))
		{
			/*
			* Propagate touch events to its parents
			*/
			if (_propagateTouchEvents)
			{
				this->propagateTouchEvent(TouchEventType::ENDED, this, touch);
			}

			bool highlight = _highlight;
			setHighlighted(false);

			if (highlight)
			{
				releaseUpEvent();
			}
			else
			{
				cancelUpEvent();
			}
		}
	}
	else
		cocos2d::ui::Widght::onTouchEnded(touch, unusedEvent);
}



/*
 * @Lua
 *
	local everySpanBtn = CIrregularButton:Create("res/mainui/skill/" .. tmpIndex .. ".png");
	everySpanBtn:SetSwallowByAlpha(true);
	everySpanBtn:setPosition(cc.p(tmpSpanBtnX[tmpIndex], tmpSpanBtnY[tmpIndex]));
	everySpanBtn:setRotation(tmpSpanRotate[tmpIndex]);
	local function everySpanBtnCallback(sender, eventType)
		print("everySpanBtnCallback");
		if eventType == ccui.TouchEventType.began then
			print("ccui.TouchEventType.began");
		elseif eventType == ccui.TouchEventType.moved then
			print("ccui.TouchEventType.moved");
		elseif eventType == ccui.TouchEventType.ended then
			print("ccui.TouchEventType.ended");
			if menu_item and eventSpanBtn then
				func(menu_item)
			end
		elseif eventType == ccui.TouchEventType.canceled then
			print("ccui.TouchEventType.canceled");
		end
	end
	everySpanBtn:addTouchEventListener(everySpanBtnCallback);
	everySpanBtn:setOpacity(0);
	menu_item:addChild(everySpanBtn);
 */