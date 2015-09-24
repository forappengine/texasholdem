#include "LostConnectionLayer.h"
#include "LayoutUtil.h"
#include "FontConstant.h"
#include "CmdSender.h"
#include "GameUtils.h"
#include "ResourceManager.h"
#include "TableLayer.h"
#include "PlayerInfo.h"
#include "PKMessageBox.h"
#include "LocalizationManager.h"
#include "StatisticsManager.h"
#include "SelectRoom\SelectRoomLayer.h"

bool LostConnectionLayer::init()
{
	if (!Layer::init())
	{
		return false;
	}

	initBG();
	initTitle();
	initContent();
	initBtns();

	return true;
}

void LostConnectionLayer::initBG()
{
	CCSprite* ptrBg = CREATE_SPRITE(this,"Images/CommonBackGround_2.jpg",false);
	GameUtils::fitToScreen(ptrBg);
	addChild(ptrBg);
	LayoutUtil::layoutParentCenter(ptrBg);
	CCSprite* ptrBgCover = CREATE_SPRITE(this,"Images/cover.png",false);
	GameUtils::fitToScreen(ptrBgCover);
	addChild(ptrBgCover);
	LayoutUtil::layoutParentCenter(ptrBgCover);
}

void LostConnectionLayer::initTitle()
{
	CCLabelBMFont* ptrLbTitle = CCLabelBMFont::create(LOCALIZED_STRING("title_lost_connection"),
		FontConstant::BMFONT_GOTHIC725_BLK_BT_BLUE);
	ptrLbTitle->setScale(1.28f);
	addChild(ptrLbTitle);
	LayoutUtil::layoutParentTop(ptrLbTitle,12,-100);
}

void LostConnectionLayer::initContent()
{
	CCLabelTTF* ptrContent = CCLabelTTF::create(LOCALIZED_STRING("info_lost_connection"),
		FontConstant::TTF_IMPACT,30);
	addChild(ptrContent);
	LayoutUtil::layoutParentCenter(ptrContent,0,35);
}

void LostConnectionLayer::initBtns()
{
	//Cancel Btn
	CCMenuItem* ptrBtnCancel = GameUtils::createBtnBlue(CCSizeMake(170,80),
		this, menu_selector(LostConnectionLayer::btnCancelCallback));
	addChild(ptrBtnCancel);
	registerMenuItem(ptrBtnCancel);
	LayoutUtil::layoutParentCenter(ptrBtnCancel,-215,-155);

	CCLabelBMFont* ptrLbCancel = CCLabelBMFont::create(LOCALIZED_STRING("word_cancel"),
		FontConstant::BMFONT_GOTHIC725_BLK_BT_BLUE);
	ptrLbCancel->setScale(0.65);
	ptrBtnCancel->addChild(ptrLbCancel);
	LayoutUtil::layoutParentCenter(ptrLbCancel,6);

	//Retry Btn
	CCMenuItem* ptrBtnRetry = GameUtils::createBtnOrange(CCSizeMake(170,80),
		this,menu_selector(LostConnectionLayer::btnRetryCallback));
	addChild(ptrBtnRetry);
	registerMenuItem(ptrBtnRetry);
	LayoutUtil::layoutParentCenter(ptrBtnRetry,215,-155);

	CCLabelBMFont* ptrLbRetry = CCLabelBMFont::create(LOCALIZED_STRING("word_retry"),
		FontConstant::BMFONT_GOTHIC725_BLK_BT_ORANGE);
	ptrLbRetry->setScale(0.65);
	ptrBtnRetry->addChild(ptrLbRetry);
	LayoutUtil::layoutParentCenter(ptrLbRetry,6);
}

void LostConnectionLayer::btnCancelCallback( CCObject* pSender )
{
	((GameScreen*)getParent())->backToUI();
}

void LostConnectionLayer::btnRetryCallback( CCObject* pSender )
{
	NewEnterTableCmd *ptrEnterTableCmd;
	if (TableLayer::s_tableType == TABLE_TYPE_NORMAL)
	{
		ptrEnterTableCmd = CmdSender::getInstance()->getNewEnterTableCmd(0,0,SelectRoomLayer::DEFAULT_ROOM_CAPACITY,"normal");
	}
	else if (TableLayer::s_tableType == TABLE_TYPE_SHOOT_OUT)
	{
		ptrEnterTableCmd = CmdSender::getInstance()->getEnterShootOutCmd();
	}
	else if (TableLayer::s_tableType == TABLE_TYPE_SNG)
	{
		ptrEnterTableCmd = CmdSender::getInstance()->getEnterSitAndGoCmd(TableLayer::s_tableInfo.buyIn);
	}
	CCNotificationCenter::sharedNotificationCenter()->addObserver(this,
		callfuncO_selector(LostConnectionLayer::enterTableNetworkCallback),
		ptrEnterTableCmd->m_cmdName.c_str(),NULL);
	CmdSender::getInstance()->sendCommands(1,ptrEnterTableCmd);
}

bool LostConnectionLayer::onKeyBackClicked()
{
	return true;
}

void LostConnectionLayer::enterTableNetworkCallback( CCObject* pSender )
{
	NewEnterTableCmd * ptrEnterTableCmd = (NewEnterTableCmd*)pSender;
	int errorNO = ptrEnterTableCmd->m_ptrOutput->errorInfo.errorNO;
	string errorMsg = ptrEnterTableCmd->m_ptrOutput->errorInfo.errorMsg;

	if(errorNO == RespHandler::OK)
	{
		GameScreen::phpSessionID = ptrEnterTableCmd->m_ptrOutput->session;
		GameScreen::host = ptrEnterTableCmd->m_ptrOutput->host;
		GameScreen::port = ptrEnterTableCmd->m_ptrOutput->port;
		TableLayer::s_tableInfo = ptrEnterTableCmd->m_ptrOutput->tableInfo;
		GameUtils::s_myInfo.setChip(ptrEnterTableCmd->m_ptrOutput->chip);
		GameUtils::s_myInfo.setDiamond(ptrEnterTableCmd->m_ptrOutput->diamond);

		if (TableLayer::s_tableType == TABLE_TYPE_NORMAL)
		{
			StatisticsManager::getInstance()->onEnterTable(
				GameUtils::getDeviceID().c_str(),
				GameUtils::getReferrer().c_str(),
				GameUtils::getGiftVersion(),
				GameUtils::s_myInfo.getLevel(),
				GameUtils::s_myInfo.getChip(),
				GameUtils::s_myInfo.getDiamond(),
				"retry_when_lost_connection",
				ptrEnterTableCmd->m_ptrOutput->tableInfo.bigBlind,
				ptrEnterTableCmd->m_ptrOutput->tableInfo.buyIn);
		}

		CCNotificationCenter::sharedNotificationCenter()->removeObserver(this,ptrEnterTableCmd->m_cmdName.c_str());
		CC_SAFE_DELETE(ptrEnterTableCmd);
	}
	else if (errorNO == RespHandler::CHIP_NOT_ENOUGH)
	{
		CCNotificationCenter::sharedNotificationCenter()->removeObserver(this,ptrEnterTableCmd->m_cmdName.c_str());
		CC_SAFE_DELETE(ptrEnterTableCmd);
		RespHandler::getInstance()->setLastErrorNO(RespHandler::OK);

		PKMessageBox* ptrMessageBox = PKMessageBox::create(this, PKMsgTypeOk, 
			LOCALIZED_STRING("reminder_chips_not_enough"), 
			callfuncO_selector(LostConnectionLayer::btnCancelCallback), NULL);
		addChild(ptrMessageBox);
		LayoutUtil::layoutParentCenter(ptrMessageBox);
	}
	if(errorNO == RespHandler::OK)
	{
		CCScene* gameScene = GameScreen::scene();
		CCDirector::sharedDirector()->replaceScene(gameScene);
	}
}
