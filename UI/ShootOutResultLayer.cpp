#include "ShootOutResultLayer.h"
#include "LayoutUtil.h"
#include "FontConstant.h"
#include "ResourceManager.h"
#include "Utils.h"
#include "TableLayer.h"
#include "CmdOutputs.h"
#include "GameUtils.h"
#include "PlayerInfo.h"
#include "CmdSender.h"
#include "Screen.h"
#include "Shootout\ShootoutLayer.h"
#include "LocalizationManager.h"
#include "Node/MenuItemImage.h"

ShootOutResultLayer* ShootOutResultLayer::create( int rank,INT64 bonus )
{
	ShootOutResultLayer* pRet = new ShootOutResultLayer();
	if (pRet && pRet->init(rank,bonus))
	{
		pRet->autorelease();
		return pRet;
	}
	CC_SAFE_DELETE(pRet);
	return NULL;
}

ShootOutResultLayer::ShootOutResultLayer(){}

ShootOutResultLayer::~ShootOutResultLayer(){}

bool ShootOutResultLayer::init( int rank,INT64 bonus )
{
	if(!Layer::init())
	{
		return false;
	}

	m_rank = rank;
	initBG();
	initContent(bonus);
	initBtns();

	return true;
}

void ShootOutResultLayer::animateShow()
{
	if (m_rank > 1)
	{
		CCPoint temp = getPosition();
		temp.y -= 120;
		setPosition(temp);
		runAction(CCMoveBy::create(0.4f,ccp(0,120)));
	}
	else
	{
		setScale(0.2);
		CCFiniteTimeAction* action = CCEaseBackOut::create(CCScaleTo::create(0.4f,1.0f));
		runAction(action);
	}
}

void ShootOutResultLayer::initBG()
{
	if (m_rank > 1)
	{
		setContentSize(CCSizeMake(getContentSize().width,115));
		//BG 
		m_ptrBg = GameUtils::createScale9("msg_bg_bottom.png",
			CCRectMake(0,0,8,109),CCRectMake(2,0,4,109));
		m_ptrBg->setContentSize(CCSizeMake(getContentSize().width,109));
		addChild(m_ptrBg);
		LayoutUtil::layoutParentBottom(m_ptrBg);
	}
	else
	{
		//BG 
		m_ptrBg = GameUtils::createScale9("popup_bg.png", 
			CCRectMake(0, 0, 100, 100), CCRectMake(40, 40, 20, 20));
		m_ptrBg->setContentSize(CCSizeMake(704,520));
		addChild(m_ptrBg);
		LayoutUtil::layoutParentCenter(m_ptrBg);

		//Title
		CCLabelBMFont* ptrTitle = CCLabelBMFont::create(LOCALIZED_STRING("title_congratulations"),
			FontConstant::BMFONT_GOTHIC725_BLK_BT_BLUE);
		ptrTitle->setScale(0.84f);
		m_ptrBg->addChild(ptrTitle);
		LayoutUtil::layoutParentTop(ptrTitle,0,-20);
	}
}

void ShootOutResultLayer::initContent( INT64 bonus )
{
	if (m_rank > 1)
	{
		//MSG
		CCLabelTTF* ptrMSG = CCLabelTTF::create(
			CCString::createWithFormat(LOCALIZED_STRING("info_finish"), 
			GameUtils::getRank(m_rank).c_str())->getCString(),
			FontConstant::TTF_IMPACT,30);
		if (bonus > 0)
		{
			ptrMSG->setString(CCString::createWithFormat(
				LOCALIZED_STRING("info_finish_and_won"),
				GameUtils::getRank(m_rank).c_str(), 
				Utils::num2strF(bonus,8).c_str())->getCString());
		}
		ptrMSG->setColor(ccc3(0,255,255));
		m_ptrBg->addChild(ptrMSG);
		LayoutUtil::layoutParentCenter(ptrMSG,-130,-15);
	}
	else
	{
		//Content BG
		CCScale9Sprite* ptrContentBG = GameUtils::createScale9("blue_bg.png",
			CCRectMake(0,0,54,54),CCRectMake(20,20,15,15));
		ptrContentBG->setContentSize(CCSizeMake(646, 328));
		m_ptrBg->addChild(ptrContentBG);
		LayoutUtil::layoutParentCenter(ptrContentBG,0,15);

		//MSG
		CCLabelTTF* ptrLbMSG = CCLabelTTF::create(
			CCString::createWithFormat(LOCALIZED_STRING("info_finish_1st_and_won"),
			GameUtils::getRank(m_rank).c_str(), 
			Utils::num2strF(bonus,9).c_str())->getCString(),
			FontConstant::TTF_IMPACT,30);
		ptrLbMSG->setColor(ccc3(0,255,255));
		ptrContentBG->addChild(ptrLbMSG);
		LayoutUtil::layoutParentCenter(ptrLbMSG,0,120);

		//Chip Image
		CCSprite* ptrChip = CREATE_SPRITE(this,"shop_item_chip_5.png",true);
		ptrContentBG->addChild(ptrChip);
		LayoutUtil::layoutParentCenter(ptrChip);

		//Bonus
		CCLabelTTF* ptrLbBonus = CCLabelTTF::create(
			CCString::createWithFormat(LOCALIZED_STRING("info_won"),Utils::num2strF(bonus,11).c_str())->getCString(),
			FontConstant::TTF_IMPACT,30);
		ptrLbBonus->setColor(ccc3(0,255,255));
		ptrContentBG->addChild(ptrLbBonus);
		LayoutUtil::layoutParentCenter(ptrLbBonus,0,-90);

		//New Account Balance
		CCLabelTTF* ptrLbCurrentAmount = CCLabelTTF::create(
			CCString::createWithFormat(LOCALIZED_STRING("new_account_balance"),
			Utils::num2strF(GameUtils::s_myInfo.getChip(),11).c_str())->getCString(),
			FontConstant::TTF_IMPACT,30);
		ptrContentBG->addChild(ptrLbCurrentAmount);
		LayoutUtil::layoutParentCenter(ptrLbCurrentAmount,0,-130);
	}
}

void ShootOutResultLayer::initBtns()
{
	if (m_rank > 1)
	{
		//Play Btn
		CCMenuItem* ptrBtnPlay = GameUtils::createBtnBlue(CCSizeMake(220,70),
			this,menu_selector(ShootOutResultLayer::btnPlayCallback));
		m_ptrBg->addChild(ptrBtnPlay);
		LayoutUtil::layoutParentCenter(ptrBtnPlay,315,-17);
		registerMenuItem(ptrBtnPlay);

		CCLabelBMFont* ptrLbPlay = CCLabelBMFont::create(LOCALIZED_STRING("play_now"),
			FontConstant::BMFONT_GOTHIC725_BLK_BT_BLUE);
		ptrLbPlay->setScale(0.7f);
		ptrBtnPlay->addChild(ptrLbPlay);
		LayoutUtil::layoutParentCenter(ptrLbPlay,7,2);
	}
	else
	{
		//Close Btn
		CCMenuItem* ptrBtnClose = MenuItemImage::create("btn_close.png","btn_close_down.png",
			this,menu_selector(ShootOutResultLayer::btnCloseCallback));
		m_ptrBg->addChild(ptrBtnClose);
		LayoutUtil::layoutParentRightTop(ptrBtnClose,-26,-26);
		registerMenuItem(ptrBtnClose);

		//Play Btn
		CCMenuItem* ptrBtnPlay = GameUtils::createBtnBlue(CCSizeMake(221,84),
			this,menu_selector(ShootOutResultLayer::btnPlayCallback));
		m_ptrBg->addChild(ptrBtnPlay);
		LayoutUtil::layoutParentBottom(ptrBtnPlay,0,23);
		registerMenuItem(ptrBtnPlay);

		CCLabelBMFont* ptrLbPlay = CCLabelBMFont::create(LOCALIZED_STRING("play_now"),
			FontConstant::BMFONT_GOTHIC725_BLK_BT_BLUE);
		ptrLbPlay->setScale(0.65f);
		ptrBtnPlay->addChild(ptrLbPlay);
		LayoutUtil::layoutParentCenter(ptrLbPlay,6);
	}
}

void ShootOutResultLayer::btnCloseCallback( CCObject* pSender )
{
	setTouchEnabled(false);
	((TableLayer*)getParent())->m_isShowSNGOrSTOLayer = false;
	
	CCAction* ptrAct = CCSequence::create(
		CCEaseBackIn::create(CCScaleTo::create(0.4f, 0.01f)),
		CCCallFunc::create(this, callfunc_selector(ShootOutResultLayer::removeFromParent)),
		NULL);
	runAction(ptrAct);
}

void ShootOutResultLayer::btnPlayCallback( CCObject* pSender )
{
	if (ShootoutLayer::SHOOTOUT_ENTER_FEE > GameUtils::s_myInfo.getChip())
	{
		TableLayer* ptrTableLayer = dynamic_cast<TableLayer*>(getParent());
		if (ptrTableLayer)
		{
			ptrTableLayer->showQucikBuyLayer();
		}	

		return;
	}

	TableLayer* ptrTableLayer = (TableLayer*)getParent();
	GameScreen* ptrGameScreen = (GameScreen*)ptrTableLayer->getParent();
	ptrGameScreen->unschedule(schedule_selector(GameScreen::update));
	ptrGameScreen->gameSocket->destroy();

	EnterShootOutCmd* ptrEnterShootOutCmd = CmdSender::getInstance()->getEnterShootOutCmd();
	CCNotificationCenter::sharedNotificationCenter()->addObserver(this,
		callfuncO_selector(ShootOutResultLayer::enterShootOutCmdNetworkCallback),
		ptrEnterShootOutCmd->m_cmdName.c_str(),
		NULL);
	CmdSender::getInstance()->sendCommands(1,ptrEnterShootOutCmd);
}

void ShootOutResultLayer::enterShootOutCmdNetworkCallback( CCObject* pSender )
{
	EnterShootOutCmd* ptrEnterShootOutCmd = (EnterShootOutCmd*)pSender;
	int errorNO = ptrEnterShootOutCmd->m_ptrOutput->errorInfo.errorNO;
	string errorMsg = ptrEnterShootOutCmd->m_ptrOutput->errorInfo.errorMsg;

	if(errorNO == RespHandler::OK)
	{
		GameScreen::phpSessionID = ptrEnterShootOutCmd->m_ptrOutput->session;
		GameScreen::host = ptrEnterShootOutCmd->m_ptrOutput->host;
		GameScreen::port = ptrEnterShootOutCmd->m_ptrOutput->port;
		TableLayer::s_tableInfo = ptrEnterShootOutCmd->m_ptrOutput->tableInfo;
		TableLayer::s_tableType = TABLE_TYPE_SHOOT_OUT;
		GameUtils::s_myInfo.setChip(ptrEnterShootOutCmd->m_ptrOutput->chip);
		GameUtils::s_myInfo.setDiamond(ptrEnterShootOutCmd->m_ptrOutput->diamond);

		CCNotificationCenter::sharedNotificationCenter()->removeObserver(this,ptrEnterShootOutCmd->m_cmdName.c_str());
		CC_SAFE_DELETE(ptrEnterShootOutCmd);
	}

	if(errorNO == RespHandler::OK)
	{
		CCScene* gameScene = GameScreen::scene();
		CCDirector::sharedDirector()->replaceScene(gameScene);
	}
}

bool ShootOutResultLayer::onKeyBackClicked()
{
	if (m_rank > 1)
	{
		return false;
	}
	else
	{
		btnCloseCallback(NULL);
		return true;
	}
}
