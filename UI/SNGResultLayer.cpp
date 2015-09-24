#include "SNGResultLayer.h"
#include "LayoutUtil.h"
#include "FontConstant.h"
#include "ResourceManager.h"
#include "Utils.h"
#include "TableLayer.h"
#include "CmdOutputs.h"
#include "SNG/SNGLayer.h"
#include "GameUtils.h"
#include "PlayerInfo.h"
#include "CmdSender.h"
#include "Screen.h"
#include "LocalizationManager.h"
#include "Node/MenuItemImage.h"

SNGResultLayer* SNGResultLayer::create(int rank,INT64 bonus)
{
	SNGResultLayer* pRet = new SNGResultLayer();
	if (pRet && pRet->init(rank,bonus))
	{
		pRet->autorelease();
		return pRet;
	}
	CC_SAFE_DELETE(pRet);
	return NULL;
}

SNGResultLayer::SNGResultLayer(){}

SNGResultLayer::~SNGResultLayer(){}

bool SNGResultLayer::init(int rank,INT64 bonus)
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

void SNGResultLayer::initBG()
{
	if (m_rank > 3)
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
		LayoutUtil::layoutParentTop(ptrTitle,11,-20);
	}
}

void SNGResultLayer::initContent(INT64 bonus)
{
	if (m_rank > 3)
	{
		//MSG
		CCLabelTTF* ptrMSG = CCLabelTTF::create(
			CCString::createWithFormat(LOCALIZED_STRING("info_finish"),
				GameUtils::getRank(m_rank).c_str())->getCString(),
			FontConstant::TTF_IMPACT,35);
		ptrMSG->setColor(ccc3(0,255,255));
		m_ptrBg->addChild(ptrMSG);
		LayoutUtil::layoutParentCenter(ptrMSG,-85,-15);
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
		string strRank;
		switch (m_rank)
		{
		case 1:
			strRank = "1st";
			break;
		case 2:
			strRank = "2nd";
			break;
		case 3:
			strRank = "3rd";
			break;
		}
		CCLabelTTF* ptrLbMSG = CCLabelTTF::create(
			CCString::createWithFormat(LOCALIZED_STRING("info_finish_1st_and_won"), 
			GameUtils::getRank(m_rank).c_str(),
			Utils::num2strF(bonus,11).c_str())->getCString(),
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
			CCString::createWithFormat(LOCALIZED_STRING("info_won"),
			Utils::num2strF(bonus,11).c_str())->getCString(),
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

void SNGResultLayer::initBtns()
{
	if (m_rank > 3)
	{
		//Play Btn
		CCMenuItem* ptrBtnPlay = GameUtils::createBtnBlue(CCSizeMake(265,70),
			this,menu_selector(SNGResultLayer::btnPlayCallback));
		m_ptrBg->addChild(ptrBtnPlay);
		LayoutUtil::layoutParentCenter(ptrBtnPlay,225,-14);

		CCLabelBMFont* ptrTextPlay = CCLabelBMFont::create(LOCALIZED_STRING("play_now"),
			FontConstant::BMFONT_GOTHIC725_BLK_BT_BLUE);
		ptrTextPlay->setScale(0.7f);
		ptrBtnPlay->addChild(ptrTextPlay);
		LayoutUtil::layoutParentLeft(ptrTextPlay,25,3);

		//Buy In Btn
		m_ptrBtnBuyIn = MenuItemImage::create("btn_select_table_small.png","btn_select_table_small.png", 
			this, menu_selector(SNGResultLayer::btnBuyInCallback));
		m_ptrBg->addChild(m_ptrBtnBuyIn);
		LayoutUtil::layoutRight(m_ptrBtnBuyIn,ptrBtnPlay,-68,-2);
		registerMenuItem(m_ptrBtnBuyIn);
		registerMenuItem(ptrBtnPlay);
	}
	else
	{
		//Close Btn
		CCMenuItem* ptrBtnClose = MenuItemImage::create("btn_close.png","btn_close_down.png",
			this,menu_selector(SNGResultLayer::btnCloseCallback));
		m_ptrBg->addChild(ptrBtnClose);
		LayoutUtil::layoutParentRightTop(ptrBtnClose,-26,-26);
		registerMenuItem(ptrBtnClose);

		//Play Btn
		CCMenuItem* ptrBtnPlay = GameUtils::createBtnBlue(CCSizeMake(292,89),
			this,menu_selector(SNGResultLayer::btnPlayCallback));
		m_ptrBg->addChild(ptrBtnPlay);
		LayoutUtil::layoutParentBottom(ptrBtnPlay,0,22);

		CCLabelBMFont* ptrLbPlay = CCLabelBMFont::create(LOCALIZED_STRING("play_now"),
			FontConstant::BMFONT_GOTHIC725_BLK_BT_BLUE);
		ptrLbPlay->setScale(0.65);
		ptrBtnPlay->addChild(ptrLbPlay);
		LayoutUtil::layoutParentLeft(ptrLbPlay,39,15);

		CCLabelTTF* ptrLbBuyIn = CCLabelTTF::create(
			CCString::createWithFormat(LOCALIZED_STRING("sng_result_buyin"),
			Utils::num2strF(TableLayer::s_tableInfo.buyIn,7).c_str(),
			Utils::num2strF(SNGLayer::getFeeByBuyIn(TableLayer::s_tableInfo.buyIn),7).c_str())->getCString(),
			FontConstant::TTF_IMPACT,21);
		ptrBtnPlay->addChild(ptrLbBuyIn);
		LayoutUtil::layoutCenter(ptrLbBuyIn,ptrLbPlay,0,-33);

		//Buy In Btn
		m_ptrBtnBuyIn = MenuItemImage::create("btn_select_table_small.png","btn_select_table_small.png", 
			this, menu_selector(SNGResultLayer::btnBuyInCallback));
		m_ptrBg->addChild(m_ptrBtnBuyIn);
		LayoutUtil::layoutRight(m_ptrBtnBuyIn,ptrBtnPlay,-72,0);
		registerMenuItem(m_ptrBtnBuyIn);
		registerMenuItem(ptrBtnPlay);
	}
}

void SNGResultLayer::btnCloseCallback( CCObject* pSender )
{
	setTouchEnabled(false);
	((TableLayer*)getParent())->m_isShowSNGOrSTOLayer = false;
	if (m_rank == 1)
	{
		((TableLayer*)getParent())->backToUI();
		return;
	}
	
	CCAction* ptrAct;
	if (m_rank > 3)
	{
		ptrAct = CCSequence::create(
			CCMoveBy::create(0.4f,ccp(0,-120)),
			CCCallFunc::create(this, callfunc_selector(SNGResultLayer::removeFromParent)),
			NULL);	
	}
	else
	{
		ptrAct = CCSequence::create(
			CCEaseBackIn::create(CCScaleTo::create(0.4f, 0.01f)),
			CCCallFunc::create(this, callfunc_selector(SNGResultLayer::removeFromParent)),
			NULL);
	}
	runAction(ptrAct);
}

void SNGResultLayer::btnPlayCallback( CCObject* pSender )
{
	if (TableLayer::s_tableInfo.buyIn > GameUtils::s_myInfo.getChip())
	{
		TableLayer* ptrTableLayer = dynamic_cast<TableLayer*>(getParent());
		if (ptrTableLayer)
		{
			ptrTableLayer->showQucikBuyLayer();
		}
		return;
	}

	TableLayer* ptrTableLayer = dynamic_cast<TableLayer*>(getParent());
	if (ptrTableLayer)
	{
		GameScreen* ptrGameScreen = (GameScreen*)ptrTableLayer->getParent();
		ptrGameScreen->unschedule(schedule_selector(GameScreen::update));
		ptrGameScreen->gameSocket->destroy();
	}

	EnterSitAndGoCmd* ptrEnterSitAndGoCmd = CmdSender::getInstance()->getEnterSitAndGoCmd(TableLayer::s_tableInfo.buyIn);
	CCNotificationCenter::sharedNotificationCenter()->addObserver(this,
		callfuncO_selector(SNGResultLayer::enterSitAndGoCmdNetworkCallback),
		ptrEnterSitAndGoCmd->m_cmdName.c_str(),
		NULL);
	CmdSender::getInstance()->sendCommands(1,ptrEnterSitAndGoCmd);
}

void SNGResultLayer::btnBuyInCallback( CCObject* pSender )
{
	((TableLayer*)getParent())->showSNGLayer();
	btnCloseCallback(NULL);
	((TableLayer*)getParent())->m_isShowSNGOrSTOLayer = true;
}

void SNGResultLayer::enterSitAndGoCmdNetworkCallback( CCObject* pSender )
{
	EnterSitAndGoCmd * ptrEnterSitAndGoCmd = (EnterSitAndGoCmd*)pSender;
	int errorNO = ptrEnterSitAndGoCmd->m_ptrOutput->errorInfo.errorNO;
	string errorMsg = ptrEnterSitAndGoCmd->m_ptrOutput->errorInfo.errorMsg;

	if(errorNO == RespHandler::OK)
	{
		GameScreen::phpSessionID = ptrEnterSitAndGoCmd->m_ptrOutput->session;
		GameScreen::host = ptrEnterSitAndGoCmd->m_ptrOutput->host;
		GameScreen::port = ptrEnterSitAndGoCmd->m_ptrOutput->port;
		TableLayer::s_tableInfo = ptrEnterSitAndGoCmd->m_ptrOutput->tableInfo;
		TableLayer::s_tableType = TABLE_TYPE_SNG;
		GameUtils::s_myInfo.setChip(ptrEnterSitAndGoCmd->m_ptrOutput->chip);
		GameUtils::s_myInfo.setDiamond(ptrEnterSitAndGoCmd->m_ptrOutput->diamond);

		CCNotificationCenter::sharedNotificationCenter()->removeObserver(this,ptrEnterSitAndGoCmd->m_cmdName.c_str());
		CC_SAFE_DELETE(ptrEnterSitAndGoCmd);
	}

	if(errorNO == RespHandler::OK)
	{
		CCScene* gameScene = GameScreen::scene();
		CCDirector::sharedDirector()->replaceScene(gameScene);
	}
}

void SNGResultLayer::animateShow()
{
	if (m_rank > 3)
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

bool SNGResultLayer::onKeyBackClicked()
{
	if (m_rank > 3)
	{
		return false;
	}
	else
	{
		btnCloseCallback(NULL);
		return true;
	}
}