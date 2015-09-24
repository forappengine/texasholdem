#include "NotificationBar.h"
#include "FontConstant.h"
#include "Screen.h"
#include "LayoutUtil.h"
#include "GameUtils.h"
#include "Node/MenuItemImage.h"

NotificationBar* NotificationBar::s_ptrInstance = NULL;

void NotificationBar::setNotificationMsg( string str )
{
	m_ptrLbNotificationMsg->setString(str.c_str());
}

bool NotificationBar::init()
{
	if (!Layer::init())
	{
		return false;
	}

	setContentSize(CCSizeMake(getContentSize().width,115));
	initBG();
	initNotificationMsg();
	initBtnClose();

	return true;
}

void NotificationBar::initNotificationMsg()
{
	m_ptrLbNotificationMsg = CCLabelTTF::create("",FontConstant::TTF_IMPACT,30);
	m_ptrLbNotificationMsg->setColor(ccc3(0,255,255));
	m_ptrBg->addChild(m_ptrLbNotificationMsg);
	LayoutUtil::layoutParentTop(m_ptrLbNotificationMsg,0,-33);
}

void NotificationBar::initBG()
{
	m_ptrBg = GameUtils::createScale9("msg_bg_top.png",
		CCRectMake(0,0,8,93),CCRectMake(2,0,4,93));
	m_ptrBg->setContentSize(CCSizeMake(getContentSize().width,93));
	addChild(m_ptrBg);
	LayoutUtil::layoutParentTop(m_ptrBg);
}

void NotificationBar::initBtnClose()
{
	CCMenuItem* ptrBtnClose = MenuItemImage::create("btn_close.png","btn_close_down.png",
		this,menu_selector(NotificationBar::btnCloseCallback));
	m_ptrBg->addChild(ptrBtnClose);
	LayoutUtil::layoutParentRight(ptrBtnClose,-6,14);
	registerMenuItem(ptrBtnClose);
}

void NotificationBar::btnCloseCallback( CCObject* pSender )
{
	setTouchEnabled(false);
	CCAction* ptrAct = CCSequence::create(
		CCMoveBy::create(0.4f,ccp(0,200)),	
		CCCallFunc::create(this, callfunc_selector(NotificationBar::removeFromParent)),
		NULL);
	this->runAction(ptrAct);
}

NotificationBar::NotificationBar(){}

NotificationBar::~NotificationBar()
{
	if (s_ptrInstance)
	{
		s_ptrInstance->removeFromParent();
		s_ptrInstance = NULL;
	}
}

NotificationBar* NotificationBar::getInstance()
{
	if (s_ptrInstance)
	{
		s_ptrInstance->removeFromParent();
		s_ptrInstance = NULL;
	}

	s_ptrInstance = new NotificationBar();
	if (s_ptrInstance && s_ptrInstance->init())
	{
		s_ptrInstance->autorelease();
		return s_ptrInstance;
	}

	CC_SAFE_DELETE(s_ptrInstance);
	return NULL;	
}

void NotificationBar::animateShow()
{
	CCPoint temp = getPosition();
	temp.y += 120;
	setPosition(temp);
	runAction(CCMoveBy::create(0.4f,ccp(0,-120)));
}

void NotificationBar::setAutoHide( float duration )
{
	if (duration <= 0)
	{
		return;
	}

	scheduleOnce(schedule_selector(NotificationBar::autoHide), duration);
}

void NotificationBar::autoHide( float dlt )
{
	btnCloseCallback(NULL);
}
