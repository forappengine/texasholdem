#include "GameMoreMenuLayer.h"
#include "GameConstants.h"
#include "LayoutUtil.h"
#include "ResourceManager.h"
#include "GameSettingsLayer.h"
#include "FontConstant.h"
#include "TableLayer.h"
#include "TutorialLayer.h"
#include "Screen.h"
#include "SoundPlayer.h"
#include "CmdSender.h"
#include "GameUtils.h"
#include "PKMessageBox.h"
#include "PlayerInfo.h"
#include "LocalizationManager.h"
#include "Node/MenuItemImage.h"
#include "StatisticsManager.h"

GameMoreMenuLayer::GameMoreMenuLayer():
	m_ptrMenuBg(NULL),
	m_bIsLayerSwitching(false)
{}

GameMoreMenuLayer::~GameMoreMenuLayer(){}

bool GameMoreMenuLayer::init()
{
	if(!Layer::init())
	{
		return false;
	}

	initMenuButtons();

	return true;
}

void GameMoreMenuLayer::initMenuButtons()
{
	float itemGap = 0;
	// menu bg
	m_ptrMenuBg = GameUtils::createScale9("new_more_menu_bg.png",
		CCRectMake(0,0,100,100),CCRectMake(35,35,15,15));
	m_ptrMenuBg->setContentSize(CCSizeMake(291,300));
	setContentSize(m_ptrMenuBg->getContentSize());
	addChild(m_ptrMenuBg);
	LayoutUtil::layoutParentCenter(m_ptrMenuBg);

	//menu item hand ranks
	CCMenuItem* ptrBtnHandRanks = createMenuItem(menu_selector(GameMoreMenuLayer::onMenuItem),
		(void*)UD_HAND_RANKS);
	m_ptrMenuBg->addChild(ptrBtnHandRanks);
	registerMenuItem(ptrBtnHandRanks);
	LayoutUtil::layoutParentBottom(ptrBtnHandRanks, 0, 21);

	CCSprite* ptrIconHandRanks = CREATE_SPRITE(this,"new_icon_hand_ranks.png",false);
	ptrBtnHandRanks->addChild(ptrIconHandRanks);
	LayoutUtil::layoutParentCenter(ptrIconHandRanks,-89);

	CCLabelBMFont* ptrLbHandRanks = CCLabelBMFont::create(LOCALIZED_STRING("menu_hand_ranks"),
		FontConstant::BMFONT_GOTHIC725_BLK_BT_BLUE);
	ptrLbHandRanks->setScale(0.45f);
	ptrBtnHandRanks->addChild(ptrLbHandRanks);
	LayoutUtil::layoutParentCenter(ptrLbHandRanks,37);

	CCSprite* ptrLine0 = CREATE_SPRITE(this,"Images/separator_line_thin.png",false);
	ptrLine0->setScaleX(0.25f);
	m_ptrMenuBg->addChild(ptrLine0);
	LayoutUtil::layoutTop(ptrLine0,ptrBtnHandRanks,0,-18);

	//menu item settings
	CCMenuItem* ptrBtnSettings = createMenuItem(menu_selector(GameMoreMenuLayer::onMenuItem),
		(void*)UD_SETTINGS);
	m_ptrMenuBg->addChild(ptrBtnSettings);
	registerMenuItem(ptrBtnSettings);
	LayoutUtil::layoutTop(ptrBtnSettings,ptrBtnHandRanks,0,itemGap);

	CCSprite* ptrIconSettings = CREATE_SPRITE(this,"new_icon_settings.png",false);
	ptrBtnSettings->addChild(ptrIconSettings);
	LayoutUtil::layoutParentCenter(ptrIconSettings,-89);

	CCLabelBMFont* ptrLbSettings = CCLabelBMFont::create(LOCALIZED_STRING("menu_settings"),
		FontConstant::BMFONT_GOTHIC725_BLK_BT_BLUE);
	ptrLbSettings->setScale(0.45f);
	ptrBtnSettings->addChild(ptrLbSettings);
	LayoutUtil::layoutParentCenter(ptrLbSettings,37);

	CCSprite* ptrLine1 = CREATE_SPRITE(this,"Images/separator_line_thin.png",false);
	ptrLine1->setScaleX(0.25f);
	m_ptrMenuBg->addChild(ptrLine1);
	LayoutUtil::layoutTop(ptrLine1,ptrBtnSettings,0,-18);
	CCNode* ptrTargetNode = ptrBtnSettings;

	if (TableLayer::s_tableType == TABLE_TYPE_NORMAL)
	{
		//menu item change table
		CCMenuItem* ptrBtnChangeTable = createMenuItem(menu_selector(GameMoreMenuLayer::onMenuItem),
			(void*)UD_CHANGE_TABLE);
		m_ptrMenuBg->addChild(ptrBtnChangeTable);
		registerMenuItem(ptrBtnChangeTable);
		LayoutUtil::layoutTop(ptrBtnChangeTable,ptrBtnSettings,0,itemGap);

		CCSprite* ptrIconChangeRoom = CREATE_SPRITE(this,"new_icon_change_table.png",false);
		ptrBtnChangeTable->addChild(ptrIconChangeRoom);
		LayoutUtil::layoutParentCenter(ptrIconChangeRoom,-89);

		CCLabelBMFont* ptrLbChangeTable = CCLabelBMFont::create(LOCALIZED_STRING("menu_change_table"),
			FontConstant::BMFONT_GOTHIC725_BLK_BT_BLUE);
		ptrLbChangeTable->setScale(0.45f);
		ptrBtnChangeTable->addChild(ptrLbChangeTable);
		LayoutUtil::layoutParentCenter(ptrLbChangeTable,37);

		CCSprite* ptrLine = CREATE_SPRITE(this,"Images/separator_line_thin.png",false);
		ptrLine->setScaleX(0.25f);
		m_ptrMenuBg->addChild(ptrLine);
		LayoutUtil::layoutTop(ptrLine,ptrBtnChangeTable,0,-18);
		ptrTargetNode = ptrBtnChangeTable;
	}

	//menu item leave
	CCMenuItem* ptrBtnLeave = createMenuItem(menu_selector(GameMoreMenuLayer::onMenuItem),
		(void*)UD_LEAVE);
	m_ptrMenuBg->addChild(ptrBtnLeave);
	registerMenuItem(ptrBtnLeave);
	LayoutUtil::layoutTop(ptrBtnLeave,ptrTargetNode,0,itemGap);

	CCSprite* ptrIconLeave =  CREATE_SPRITE(this,"new_icon_exit.png",false);
	ptrBtnLeave->addChild(ptrIconLeave);
	LayoutUtil::layoutParentCenter(ptrIconLeave,-89);

	CCLabelBMFont* ptrLbLeave = CCLabelBMFont::create(LOCALIZED_STRING("menu_leave_room"),
		FontConstant::BMFONT_GOTHIC725_BLK_BT_BLUE);
	ptrLbLeave->setScale(0.45f);
	ptrBtnLeave->addChild(ptrLbLeave);
	LayoutUtil::layoutParentCenter(ptrLbLeave,37);

	CCSprite* ptrLine2 = CREATE_SPRITE(this,"Images/separator_line_thin.png",false);
	ptrLine2->setScaleX(0.25f);
	m_ptrMenuBg->addChild(ptrLine2);
	LayoutUtil::layoutTop(ptrLine2,ptrTargetNode,0,-18);
}

void GameMoreMenuLayer::onMenuItem(CCObject* pSender)
{
	SoundPlayer::play(SNDI_CLICK);
	if (m_bIsLayerSwitching)
	{
		return;
	}

	CCMenuItem* item = static_cast<CCMenuItem*>(pSender);
	switch ((int)item->getUserData())
	{
	case UD_LEAVE:
		{
			backToUI(NULL);
		}
		break;
	case UD_SETTINGS:
		{
			GameSettingsLayer* layer = GameSettingsLayer::create(false);
			getParent()->addChild(layer,TableLayer::Z_SETTING_LAYER);
			LayoutUtil::layoutParentCenter(layer);
			layer->animateShow();
			hideMenu();
		}
		break;
	case UD_HAND_RANKS:
		{
			TutorialLayer* ptrTutorialLayer = TutorialLayer::create(1);
			getParent()->addChild(ptrTutorialLayer, TableLayer::Z_TUTORIAL_LAYER);
			LayoutUtil::layoutParentLeft(ptrTutorialLayer,-14);
			ptrTutorialLayer->animateToShow();
			hideMenu();
		}
		break;
	case UD_CHANGE_TABLE:
		{
			TableLayer* ptrTableLayer = (TableLayer*)getParent();
			GameScreen* ptrGameScreen = (GameScreen*)ptrTableLayer->getParent();
			ptrGameScreen->unschedule(schedule_selector(GameScreen::update));
			ptrGameScreen->gameSocket->destroy();
			string speed;
			if (ptrTableLayer->thinkTime <10)
			{
				speed = "normal";
			}
			else
			{
				speed = "slow";
			}
			NewEnterTableCmd *ptrEnterTableCmd = CmdSender::getInstance()->getNewEnterTableCmd(
				TableLayer::s_tableInfo.buyIn,TableLayer::s_tableInfo.bigBlind,
				ptrTableLayer->getCapacity(),speed);
			CCNotificationCenter::sharedNotificationCenter()->addObserver(this,
				callfuncO_selector(GameMoreMenuLayer::enterTableNetworkCallback),
				ptrEnterTableCmd->m_cmdName.c_str(),
				NULL);
			CmdSender::getInstance()->sendCommands(1,ptrEnterTableCmd);
		}
		break;
	default:
		break;
	}
}

void GameMoreMenuLayer::enterTableNetworkCallback( CCObject* pSender )
{
	NewEnterTableCmd * ptrEnterTableCmd = (NewEnterTableCmd*)pSender;
	int errorNO = ptrEnterTableCmd->m_ptrOutput->errorInfo.errorNO;

	if(errorNO == RespHandler::OK)
	{
		GameScreen::phpSessionID = ptrEnterTableCmd->m_ptrOutput->session;
		GameScreen::host = ptrEnterTableCmd->m_ptrOutput->host;
		GameScreen::port = ptrEnterTableCmd->m_ptrOutput->port;
		TableLayer::s_tableInfo = ptrEnterTableCmd->m_ptrOutput->tableInfo;
		TableLayer::s_tableType = TABLE_TYPE_NORMAL;
		GameUtils::s_myInfo.setChip(ptrEnterTableCmd->m_ptrOutput->chip);
		GameUtils::s_myInfo.setDiamond(ptrEnterTableCmd->m_ptrOutput->diamond);

		StatisticsManager::getInstance()->onEnterTable(
			GameUtils::getDeviceID().c_str(),
			GameUtils::getReferrer().c_str(),
			GameUtils::getGiftVersion(),
			GameUtils::s_myInfo.getLevel(),
			GameUtils::s_myInfo.getChip(),
			GameUtils::s_myInfo.getDiamond(),
			"change_table",
			ptrEnterTableCmd->m_ptrOutput->tableInfo.bigBlind,
			ptrEnterTableCmd->m_ptrOutput->tableInfo.buyIn);
	}
	else if (errorNO == RespHandler::CHIP_NOT_ENOUGH)
	{
		string msg = LOCALIZED_STRING("reminder_chips_not_enough");
		showErrorMsg(msg);
	}
	else
	{
		string msg = LOCALIZED_STRING("reminder_bad_network");
		showErrorMsg(msg);
	}
	CCNotificationCenter::sharedNotificationCenter()->removeObserver(this,
		ptrEnterTableCmd->m_cmdName.c_str());
	CC_SAFE_DELETE(ptrEnterTableCmd);

	if(errorNO == RespHandler::OK)
	{
		CCScene* gameScene = GameScreen::scene();
		CCDirector::sharedDirector()->replaceScene(gameScene);
	}
	RespHandler::getInstance()->setLastErrorNO(RespHandler::OK);
}

void GameMoreMenuLayer::showMenu()
{
	m_bIsLayerSwitching = true;
	float deltaY = m_ptrMenuBg->getContentSize().height;
	m_ptrMenuBg->setPositionY(m_ptrMenuBg->getPositionY() + deltaY);
	m_ptrMenuBg->stopAllActions();
	m_ptrMenuBg->runAction(CCSequence::create(
		CCEaseIn::create(CCMoveBy::create(0.4f,ccp(0,-deltaY)),3),
		CCCallFunc::create(this,callfunc_selector(GameMoreMenuLayer::revertIsLayerSwitching)),
		NULL));
}

void GameMoreMenuLayer::hideMenu()
{
	m_bIsLayerSwitching = true;
	m_ptrMenuBg->runAction(CCSequence::create(
		CCEaseIn::create(CCMoveBy::create(0.4f,ccp(0,m_ptrMenuBg->getContentSize().height)),3),
		CCCallFunc::create(this,callfunc_selector(GameMoreMenuLayer::removeFromParent)),
		NULL));
	CCNotificationCenter::sharedNotificationCenter()->removeAllObservers(this);
}

void GameMoreMenuLayer::backToUI( CCObject *pSender )
{
	TableLayer* tableLayer = dynamic_cast<TableLayer*>(getParent());
	tableLayer->backToUI();
}

void GameMoreMenuLayer::showErrorMsg( string msg )
{
	PKMessageBox* ptrMessageBox = PKMessageBox::create(this,
		PKMsgTypeOk,msg,callfuncO_selector(GameMoreMenuLayer::backToUI),NULL);
	getParent()->addChild(ptrMessageBox,TableLayer::Z_MESSAGE_BOX);
	LayoutUtil::layoutParentCenter(ptrMessageBox);
}

bool GameMoreMenuLayer::ccTouchBegan(cocos2d::CCTouch* pTouch, cocos2d::CCEvent* pEvent)
{
	if (!Layer::ccTouchBegan(pTouch,pEvent))
	{
		hideMenu();
	}

	return true;
}

bool GameMoreMenuLayer::onKeyBackClicked()
{
	if(!m_bIsLayerSwitching)
	{
		hideMenu();
	}
	
	return true;
}

void GameMoreMenuLayer::revertIsLayerSwitching()
{
	m_bIsLayerSwitching = !m_bIsLayerSwitching;
}

CCMenuItemSprite* GameMoreMenuLayer::createMenuItem( SEL_MenuHandler selector,void* pUserData )
{
	CCSprite* ptrBtnSelected = CREATE_SPRITE(this,"more_menu_item_selected.png",false);
	CCNode* ptrBtnNormal = CCNode::create();
	ptrBtnNormal->setContentSize(ptrBtnSelected->getContentSize());
	CCMenuItemSprite* ptrBtn = CCMenuItemSprite::create(ptrBtnNormal,ptrBtnSelected,this,selector);
	registerMenuItem(ptrBtn);
	ptrBtn->setUserData(pUserData);

	return ptrBtn;
}
