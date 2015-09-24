#include "GameScreen.h"
#include "LayoutUtil.h"
#include "AudioUtil.h"
#include "GameConstants.h"
#include "ResourceManager.h"
#include "SimpleAudioEngine.h"
#include "JSONCommand.h"
#include "JsonHelper.h"
#include "Utils.h"
#include "LogTag.h"
#include "CmdSender.h"
#include "GameUtils.h"
#include "PlayerInfo.h"
#include "CoverLayer.h"
#include "PKMessageBox.h"
#include "UI\LostConnectionLayer.h"
#include "LocalizationManager.h"
#include "SelectRoom\SelectRoomLayer.h"
#include "IAPManager.h"
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
#include "AdController.h"
#endif

#define MODULE_LOG_TAG "GameScreen.cpp"

string GameScreen::phpSessionID;
string GameScreen::host;
int GameScreen::port;

GameScreen::GameScreen():
	gameSocket(NULL),
	m_ptrLoadingLayer(NULL),
	table(NULL),
	isStarted(false)
{}

GameScreen::~GameScreen(){}

CCScene* GameScreen::scene()
{
    CCScene *scene = CCScene::create();
    GameScreen *layer = GameScreen::create();
    scene->addChild(layer);
	LayoutUtil::layoutParentCenter(layer);
	IAPManager::getInstance()->setIsInGame(true);
    return scene;
}

void GameScreen::onExit()
{
	Layer::onExit();
	CCNotificationCenter::sharedNotificationCenter()->removeAllObservers(this);
	gameSocket->destroy();
	delete gameSocket;
}

// on "init" you need to initialize your instance
bool GameScreen::init()
{
	if ( !Layer::init() )
    {
        return false;
    }
	//setContentSize(CCSizeMake(GameConstants::DESIGN_FRAME_WIDTH,GameConstants::DESIGN_FRAME_HEIGHT));

	m_ptrLoadingLayer = ParticleDemoLayer::create();
	this->addChild(m_ptrLoadingLayer,100);
	LayoutUtil::layoutParentCenter(m_ptrLoadingLayer);

	schedule(schedule_selector(GameScreen::update),0.01f);

    return true;
}

void GameScreen::update(float dt)
{
	if(!isStarted){
		if(gameSocket==NULL){
			gameSocket=new GameSocket();
		}
		
		//bool ret=gameSocket->create("localhost",48090,2,true);
		//bool ret=gameSocket->create("192.168.110.111",48090,2,true);
		bool ret=gameSocket->create(host.c_str(),port,2,true);
		if(ret){
			JSONCommand* lcmd=JSONCommand::createJSONCMD(JSONCommand::CMD_U_LOGIN);

			char* msg=lcmd->encode(this);

			gameSocket->sendMsg(msg);

			delete lcmd;
			isStarted=true;
		}else{
			unschedule(schedule_selector(GameScreen::update));
			showNetProblemMessageBox();
			return;
		}
	}

	if(isStarted&&gameSocket->check()){
		JSONCommand* jsncmd=JSONCommand::decode(gameSocket);
		if(jsncmd!=NULL){
			jsncmd->doAction(this);
			delete jsncmd;
		}
	}else{
		LOG_I("network has problem,we need to relogin,and catch the original context");
		if (table && !table->m_isShowSNGOrSTOLayer)
		{
			unschedule(schedule_selector(GameScreen::update));
		}
		lostConnection();
	}
}

void GameScreen::removeLoadingLayer(float dt)
{
	if(m_ptrLoadingLayer)
	{
		m_ptrLoadingLayer->removeFromParent();
		m_ptrLoadingLayer = NULL;
	}
}

void GameScreen::sendFailToEnterTableCmd(CCObject* pSender)
{
	FailedToEnterTableCmd* ptrCmd = CmdSender::getInstance()->getFailedToEnterTableCmd(phpSessionID);
	CCNotificationCenter::sharedNotificationCenter()->addObserver(this,
		callfuncO_selector(GameScreen::failToEnterTableCmdCallback),
		ptrCmd->m_cmdName.c_str(),NULL);
	CmdSender::getInstance()->sendCommands(1,ptrCmd);
}

void GameScreen::failToEnterTableCmdCallback(CCObject* pSender)
{
	FailedToEnterTableCmd * ptrFailedToEnterTableCmd = (FailedToEnterTableCmd*)pSender;
	int errorNO = ptrFailedToEnterTableCmd->m_ptrOutput->errorInfo.errorNO;
	string errorMsg = ptrFailedToEnterTableCmd->m_ptrOutput->errorInfo.errorMsg;

	if(errorNO == RespHandler::OK)
	{
		GameUtils::s_myInfo.setChip(ptrFailedToEnterTableCmd->m_ptrOutput->chip);
		GameUtils::s_myInfo.setDiamond(ptrFailedToEnterTableCmd->m_ptrOutput->diamond);

		CCNotificationCenter::sharedNotificationCenter()->removeObserver(this,ptrFailedToEnterTableCmd->m_cmdName.c_str());
		CC_SAFE_DELETE(ptrFailedToEnterTableCmd);

		backToUI();
	}
}

void GameScreen::lostConnection()
{
	CCLOG("GameScreen::lostConnection");
	if (table && table->m_isShowSNGOrSTOLayer)
	{
		//do nothing
	}
	else if (table && !table->m_isSocketClosedByServer)
	{
		table->stopAllActions();
		table->m_isSocketClosedByServer = true;
		LostConnectionLayer* ptrLayer = LostConnectionLayer::create();
		addChild(ptrLayer,102);
		LayoutUtil::layoutParentCenter(ptrLayer);
	}
	else
	{
		backToUI();
	}
}

void GameScreen::showNetProblemMessageBox()
{
	PKMessageBox* ptrMessageBox = PKMessageBox::create(this,PKMsgTypeOk,
		LOCALIZED_STRING("reminder_net_problem"),
		callfuncO_selector(GameScreen::sendFailToEnterTableCmd),NULL);
	addChild(ptrMessageBox,101);
	LayoutUtil::layoutParentCenter(ptrMessageBox);
}

void GameScreen::backToUI()
{
	CCScene* uiScene = CoverLayer::scene(true);
	CCDirector::sharedDirector()->replaceScene(uiScene);
}

void GameScreen::initBg()
{
	string bgPath = "Images/beginner_game_bg.jpg";
	if (TableLayer::s_tableType == TABLE_TYPE_NORMAL)
	{
		bgPath = "Images/" + SelectRoomLayer::getRoomType(table->blindBet) + "_game_bg.jpg";
	}
	CCSprite*  ptrBackGround = ResourceManager::getInstance()->createSprite(this,bgPath.c_str());
	GameUtils::fitToScreen(ptrBackGround);
	this->addChild(ptrBackGround);
	LayoutUtil::layoutParentCenter(ptrBackGround);
}
