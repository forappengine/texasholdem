#include "GameRankLayer.h"
#include "ResourceManager.h"
#include "LayoutUtil.h"
#include "FontConstant.h"
#include "Utils.h"
#include "Game/UI/Talk/TalkLayer.h"
#include "Game/UI/PlayerInfo/PlayerInfoLayer.h"
#include "Screen.h"
#include "MainMenuRank/MainMenuRankListLayer.h"
#include "CmdOutputs.h"
#include "TableLayer.h"
#include "SoundPlayer.h"
#include "LocalizationManager.h"
#include "Node/MenuItemImage.h"
#include "GameUtils.h"

GameRankLayer::GameRankLayer():
	m_bIsLayerSwitching(false),
	m_ptrGameRankList(NULL),
	m_bNeedRemoveFromParent(false)
{}

bool GameRankLayer::init()
{
	if(!Layer::init())
	{
		return false;
	}
	
	initBG();
	initTabs();
	initBtnBack();

	return true;
}

void GameRankLayer::initBG()
{
	//initWithColor(ccc4(155,0,0,200));
	setContentSize(CCSizeMake(365,getContentSize().height));
	m_ptrBg = GameUtils::createScale9("popup_bg_1.png",CCRectMake(0,0,100,100),CCRectMake(40,40,20,20));
	m_ptrBg->setContentSize(CCSizeMake(350,getContentSize().height + 100));
	addChild(m_ptrBg);
	LayoutUtil::layoutParentLeft(m_ptrBg,-17);
}

void GameRankLayer::initTabs()
{
	// Friend Tab
	CCScale9Sprite* ptrTabFriendsSelected = GameUtils::createScale9("new_blue_bg.png",
		CCRectMake(0,0,200,82),CCRectMake(35,35,125,10));
	ptrTabFriendsSelected->setContentSize(CCSizeMake(204,82));
	CCScale9Sprite* ptrTabFriendsDisabled = GameUtils::createScale9("new_blue_bg.png",
		CCRectMake(0,0,200,82),CCRectMake(35,35,125,10));
	ptrTabFriendsDisabled->setContentSize(CCSizeMake(204,82));
	CCNode* ptrTabFriendsNormal = CCNode::create();
	ptrTabFriendsNormal->setContentSize(ptrTabFriendsSelected->getContentSize());

	m_ptrTabFriends = CCMenuItemSprite::create(ptrTabFriendsNormal, ptrTabFriendsSelected, 
		ptrTabFriendsDisabled, this, menu_selector(GameRankLayer::tabFriendsCallback));
	m_ptrBg->addChild(m_ptrTabFriends);
	LayoutUtil::layoutParentLeftTop(m_ptrTabFriends, -5, -27);
	registerMenuItem(m_ptrTabFriends);

	m_ptrLbFriends = CCLabelBMFont::create(LOCALIZED_STRING("word_friends"), 
		FontConstant::BMFONT_GOTHIC725_BLK_BT_BLUE);
	m_ptrLbFriends->setScale(0.6f);
	m_ptrTabFriends->addChild(m_ptrLbFriends);
	LayoutUtil::layoutParentBottom(m_ptrLbFriends,6,22);

	// Global Tab
	CCScale9Sprite* ptrTabGlobalSelected = GameUtils::createScale9("new_blue_bg.png",
		CCRectMake(0,0,200,82),CCRectMake(35,35,125,10));
	ptrTabGlobalSelected->setContentSize(CCSizeMake(204,82));
	CCScale9Sprite* ptrTabGlobalDisabled = GameUtils::createScale9("new_blue_bg.png",
		CCRectMake(0,0,200,82),CCRectMake(35,35,125,10));
	ptrTabGlobalDisabled->setContentSize(CCSizeMake(204,82));
	CCNode* ptrTabGlobalNormal = CCNode::create();
	ptrTabGlobalNormal->setContentSize(ptrTabGlobalSelected->getContentSize());
	m_ptrTabGolbal = CCMenuItemSprite::create(ptrTabGlobalNormal, ptrTabGlobalSelected,
		ptrTabGlobalDisabled,this, menu_selector(GameRankLayer::tabGlobalCallback));
	m_ptrBg->addChild(m_ptrTabGolbal);
	LayoutUtil::layoutRight(m_ptrTabGolbal,m_ptrTabFriends,-44);
	registerMenuItem(m_ptrTabGolbal);

	m_ptrLbGlobal = CCLabelBMFont::create(LOCALIZED_STRING("word_global"), 
		FontConstant::BMFONT_GOTHIC725_BLK_BT_BLUE);
	m_ptrLbGlobal->setScale(0.6f);
	m_ptrTabGolbal->addChild(m_ptrLbGlobal);
	LayoutUtil::layoutParentBottom(m_ptrLbGlobal,5,22);

	tabFriendsCallback(NULL);
}

void GameRankLayer::initBtnBack()
{
	CCSprite* ptrBtnBackNormal = CREATE_SPRITE(this, "sliding.png", false);
	ptrBtnBackNormal->setCascadeOpacityEnabled(true);
	CCSprite* ptrArrow = CREATE_SPRITE(this, "arrow_up.png", false);
	ptrArrow->setScale(0.8f);
	ptrArrow->setRotation(-90);
	ptrArrow->setCascadeOpacityEnabled(true);
	ptrBtnBackNormal->addChild(ptrArrow);
	LayoutUtil::layoutParentCenter(ptrArrow, -7, 0);

	m_ptrBtnBack = ScaleMenuItemSprite::create(ptrBtnBackNormal, ptrBtnBackNormal, 
		this, menu_selector(GameRankLayer::btnBackCallback),1.0f,1.0f);
	m_ptrBg->addChild(m_ptrBtnBack);
	LayoutUtil::layoutParentRight(m_ptrBtnBack, 40, 0);
	registerMenuItem(m_ptrBtnBack);
}

void GameRankLayer::btnBackCallback(CCObject* pSender)
{
	SoundPlayer::play(SNDI_CLICK);
	if(m_bIsLayerSwitching)
	{
		return;
	}
	animateToHide();
}

void GameRankLayer::btnJoinCallback(CCObject* pSender)
{
	SoundPlayer::play(SNDI_CLICK);
	CCNode* sender = dynamic_cast<CCNode*>(pSender);
	if(sender)
	{
		PokerPlayerRankInfo* ptrInfo = static_cast<PokerPlayerRankInfo*>(sender->getUserData());
		if(ptrInfo)
		{
			TableLayer* tableLayer = (TableLayer*)getParent();
			tableLayer->sendGotoTableCmd(ptrInfo->account_id);
		}
	}
}

void GameRankLayer::tabFriendsCallback(CCObject* pSender)
{
	if (m_bIsLayerSwitching)
	{
		return;
	}
	if(m_ptrGameRankList)
	{
		SoundPlayer::play(SNDI_CLICK);
	}
	m_ptrTabFriends->setEnabled(false);
	m_ptrLbFriends->setOpacity(255.0f);
	m_ptrTabGolbal->setEnabled(true);
	m_ptrLbGlobal->setOpacity(255.0f * 0.5f);

	refreshGameRankList(true);
}

void GameRankLayer::tabGlobalCallback(CCObject* pSender)
{
	if (m_bIsLayerSwitching)
	{
		return;
	}
	if(m_ptrGameRankList)
	{
		SoundPlayer::play(SNDI_CLICK);
	}
	m_ptrTabFriends->setEnabled(true);
	m_ptrLbFriends->setOpacity(255.0f * 0.5f);
	m_ptrTabGolbal->setEnabled(false);
	m_ptrLbGlobal->setOpacity(255.0f);

	refreshGameRankList(false);
}

void GameRankLayer::refreshGameRankList( bool isToShowFriends )
{
	if(m_ptrGameRankList)
	{
		m_ptrGameRankList->removeFromParent();
		m_ptrGameRankList = NULL;
	}

	m_ptrGameRankList = GameRankListLayer::create();
	m_ptrGameRankList->setSelectors(this, menu_selector(GameRankLayer::btnJoinCallback),isToShowFriends);
	m_ptrBg->addChild(m_ptrGameRankList);
	LayoutUtil::layoutParentRight(m_ptrGameRankList, -14, -19);
}

void GameRankLayer::animateToShow()
{
	m_bIsLayerSwitching = true;
	TableLayer *ptrTableLayer = static_cast<TableLayer*>(getParent());
	if(ptrTableLayer)
	{
		ptrTableLayer->setBtnRankVisible(false);
	}
	m_ptrBg->setPositionX(m_ptrBg->getPositionX() - m_ptrBg->getContentSize().width);
	m_ptrBg->runAction(CCSequence::create(
		CCEaseSineIn::create(CCMoveBy::create(0.5f, ccp(m_ptrBg->getContentSize().width, 0))),
		CCCallFunc::create(this, callfunc_selector(GameRankLayer::onAnimateEnd)),
		NULL));
}

void GameRankLayer::animateToHide()
{
	m_bIsLayerSwitching = true;
	m_bNeedRemoveFromParent = true;
	CCPoint route = ccp(- m_ptrBg->getContentSize().width, 0);
	CCAction* ptrAct = CCSequence::create(
		CCEaseSineIn::create(CCMoveBy::create(0.5f, route)),
		CCCallFunc::create(this, callfunc_selector(GameRankLayer::onAnimateEnd)), 
		NULL);
	m_ptrBg->runAction(ptrAct);
}

void GameRankLayer::onAnimateEnd()
{
	m_bIsLayerSwitching = false;
	TableLayer *ptrTableLayer = static_cast<TableLayer*>(getParent());
	if(ptrTableLayer)
	{
		if (m_bNeedRemoveFromParent)
		{
			m_bNeedRemoveFromParent = false;
			ptrTableLayer->setBtnRankVisible(true);
			removeFromParent();
		}
		else
		{
			ptrTableLayer->setBtnRankVisible(false);
		}
	}
}

void GameRankLayer::onEnter()
{
	Layer::onEnter();
	animateToShow();
}

bool GameRankLayer::onKeyBackClicked()
{
	SoundPlayer::play(SNDI_CLICK);
	CCLOG("GameRankLayer::onKeyBackClicked");
	btnBackCallback(NULL);
	return true;
}

bool GameRankLayer::ccTouchBegan(cocos2d::CCTouch* pTouch, cocos2d::CCEvent* pEvent)
{
	if (m_bIsLayerSwitching)
	{
		return true;
	}
	if (!Layer::ccTouchBegan(pTouch, pEvent))
	{
		animateToHide();
	}

	return true;
}