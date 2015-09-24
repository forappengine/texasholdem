#include "TableLayer.h"
#include "LayoutUtil.h"
#include "AudioUtil.h"
#include "GameConstants.h"
#include "ResourceManager.h"
#include "JsonHelper.h"
#include "SimpleAudioEngine.h"
#include "PublicAreaLayer.h"
#include "BetControler.h"
#include "sstream"
#include "FontConstant.h"
#include "GameMoreMenuLayer.h"
#include "UI/InviteFriends/InviteFriendsLayer.h"
#include "UI/Talk/TalkLayer.h"
#include "LogTag.h"
#include "JSONCommand.h"
#include "CmdSender.h"
#include "PlayerInfo.h"
#include "UI/GameRank/GameRankLayer.h"
#include "UI/PlayerInfo/PlayerInfoLayer.h"
#include "GameUtils.h"
#include "PlayerInfo.h"
#include "SelectRoom/HoldemTablesLayer.h"
#include "Screen.h"
#include "PKMessageBox.h"
#include "CoverLayer.h"
#include "Utils.h"
#include "QuickBuy/QuickBuyLayer.h"
#include "MainMenuLayer.h"
#include "UI/SNGResultLayer.h"
#include "SNG/SNGLayer.h"
#include "UI/ShootOutResultLayer.h"
#include "UI/NotificationBar.h"
#include "UI/GameLotteryLayer.h"
#include "Guide/GuideBaseLayer.h"
#include "SoundPlayer.h"
#include "LocalizationManager.h"
#include "Node/MenuItemImage.h"
#include <algorithm>
#include "StatisticsManager.h"
#include "SpecialOffer/SpecialOfferLayer.h"
#include "SelectRoom/SelectRoomLayer.h"
#include "Vip/VipLayer.h"
#include "CardsTypeUtils.h"
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
#include "JniUtils.h"
#endif
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
#include "AdController.h"
#endif

#define MODULE_LOG_TAG "TableLayer.cpp"

using namespace std;

TableInfo TableLayer::s_tableInfo;
TABLE_TYPE TableLayer::s_tableType = TABLE_TYPE_NORMAL;
string TableLayer::s_tableId = "";
bool TableLayer::s_bIsJoinNearby = false;

TableLayer::TableLayer():
	mySelf(NULL),
	m_newBuyIn(0),
	myPosition(3),
	m_sitDownBtnIndex(-1),
	m_hasSetRemoveLoadingSchedule(false),
	m_isGameStarted(false),
	m_isShowSNGOrSTOLayer(false),
	m_ptrLbHint(NULL),
	m_isSocketClosedByServer(false),
	m_bHasSendSitDownCmd(false),
	m_tempChipsInGame(0),
	m_ptrPlayerInfoLayer(NULL),
	m_ptrBtnSpeicalOffer(NULL)
{}

TableLayer::~TableLayer(){}

void TableLayer::onMSGCommand(GameScreen* gameScreen,const JSONNode& node)
{
	int from = node["from"].as_int();//which one send the message, show the msg pop on his head
	string msg = node["msg"].as_string();

	if(players[from] == NULL)
	{
		return;
	}

	if(m_gagPlayerIdx.find(players[from]->m_playerInfo.getAccountID()) !=  m_gagPlayerIdx.end())
	{
		return;
	}

	string name = players[from]->m_playerInfo.getNickName();
	TalkLayer::pushNewMsg(name,msg);
	if(m_ptrTalkLayer)
	{
		m_ptrTalkLayer->updateTalkContent();
	}
	players[from]->talk(msg);
}

void TableLayer::onGiftCommand(GameScreen* gameScreen,const JSONNode& node)
{
	int from = node["from"].as_int();//which one send the message, show the msg pop on his head
	int to = node["to"].as_int();//0~8 ,the specified person; 9 : to all; for tips useless
	string giftCode = node["what"].as_string();
	
	if (giftCode.at(0) == '9')
	{
		showTipActions(from);
	}
	else
	{
		if (to >= 0 && to <= 8)
		{
			if (players[to] != NULL && players[from] != NULL)
			{
				players[to]->showSendGiftActions(players[from],giftCode);
			}
		}
		else if(to == 9)
		{
			for (int i = 0; i < capacity; i++)
			{
				if (players[i] != NULL && players[from] != NULL)
				{
					players[i]->showSendGiftActions(players[from],giftCode);
				}
			}
		}
	}
}

void TableLayer::onUserChangeCommand(GameScreen* gameScreen,const JSONNode& node)
{
	if (gameScreen->m_ptrLoadingLayer && !m_hasSetRemoveLoadingSchedule)
	{
		m_hasSetRemoveLoadingSchedule = true;
		gameScreen->scheduleOnce(schedule_selector(GameScreen::removeLoadingLayer), 2.0f);
	}
	if (s_tableType == TABLE_TYPE_NORMAL && !m_isGameStarted)
	{
		hideHint();
	}
	
	if(JsonHelper::optExist(node,"users"))
	{
		vector<string> needRefreshIDs;
		int len = node["users"].size();
		int currentPlayerNum = 0;
		bool tempFlag = false;
		for(int i = 0;i < len;i++)
		{
			string item = node["users"][i].as_string();
			if (item == "null")
			{
				removePlayer(i);
			}
			else
			{
				++currentPlayerNum;
				string id = node["users"][i]["id"].as_string();
				if (players[i] == NULL)//new player
				{
					needRefreshIDs.push_back(id);
					createAndAddPlayer(gameScreen,node["users"][i].as_node());
				}
				else if(id != players[i]->m_playerInfo.getAccountID())//player changed
				{
					needRefreshIDs.push_back(id);
					removePlayer(i);
					createAndAddPlayer(gameScreen,node["users"][i].as_node());
				}
				if (id == GameUtils::s_myInfo.getAccountID() && mySelf == NULL)
				{
					mySelf = players[i];
					tempFlag = true;
				}
			}
		}
		if (tempFlag && mySelf)
		{
			setMyPosition(mySelf->m_index);
			layoutAllPlayers();
			for (int i = 0; i <capacity; i++)
			{
				hideSitDownBtn(i);
			}
		}
		if(currentPlayerNum == capacity && s_tableType != TABLE_TYPE_NORMAL)
		{
			SoundPlayer::play(SNDI_TOURNAMENT_START);
		}
		getPlayerInfoByNetwork(needRefreshIDs);
	}
}

void TableLayer::onNotifyCommand(GameScreen* gameScreen,const JSONNode& node)
{
	if(node["reason"].as_int() == Player::REASON_KICKOUT)
	{
		m_isSocketClosedByServer = true;
	}
	else if(node["state"].as_int() == Player::GAME_STANDS_UP && node["reason"].as_int() == Player::REASON_RECHARGE)
	{
		setMyPosition(-3);
		scheduleOnce(schedule_selector(TableLayer::onRecharge),2.0f);
	}
	else if(node["state"].as_int() == Player::GAME_STANDS_UP && node["reason"].as_int() == Player::REASON_SIT_DOWN_FILURE)
	{
		m_sitDownBtnIndex = -1;
		m_bHasSendSitDownCmd = false;
		int errNum = node["errNum"].as_int(); //show the detail error reason, for user cant sit down from stand
		switch (errNum)
		{
		case Player::OP_DENIED:
			break;
		case Player::UNSUPPORTED_BUYIN:
			break;
		case Player::CHIP_NOT_ENOUGH:
			break;
		case Player::SEAT_FULLED:
			break;
		default:
			break;
		}
		if (m_newBuyIn > 0)
		{
			GameUtils::s_myInfo.setChip(GameUtils::s_myInfo.getChip() + m_newBuyIn);
			m_newBuyIn = 0;
		}
	}
	else if(node["state"].as_int() == Player::GAME_STANDS_UP && node["reason"].as_int() == Player::REASON_TOURNAMENT_LOST)
	{
		int rank = node["rank"].as_int();
		INT64 bonus;
		stringstream sval(node["bonus"].as_string());
		sval>>bonus;
		if (s_tableType == TABLE_TYPE_SNG)
		{
			showSNGResult(rank,bonus);
		}
		else if (s_tableType == TABLE_TYPE_SHOOT_OUT)
		{
			showShootOutResult(rank,bonus);
		}
	}
	else if (node["reason"].as_int() == Player::REASON_BLIND_BET_CHANGE)
	{
		INT64 newBlindBet;
		stringstream sval(node["blindBet"].as_string());
		sval>>newBlindBet;
		showNotificationBar(CCString::createWithFormat(
			LOCALIZED_STRING("reminder_blind_changed"),
			GameUtils::num2tableStr(newBlindBet/2,newBlindBet,11).c_str(),
			GameUtils::num2tableStr(newBlindBet,newBlindBet,11).c_str())->getCString(),4.0f);
	}
	else if (node["reason"].as_int() == Player::REASON_FRIEND_IN)
	{
		string friendId = node["friendId"].as_string();
		for (int i = 0; i < MainMenuLayer::s_friendInfosVec.size(); i++)
		{
			if (MainMenuLayer::s_friendInfosVec[i].account_id == friendId)
			{
				showNotificationBar(CCString::createWithFormat(
					LOCALIZED_STRING("reminder_friend_entered"),
					MainMenuLayer::s_friendInfosVec[i].nickName.c_str())->getCString(),2.0f);
				break;
			}
		}
	}
}

void TableLayer::onDecisionUpdateCommand(GameScreen* gameScreen,const JSONNode& node)
{
	stringstream sval(node["max_bet"].as_string());
	sval>>maxBet;

	int current = node["current"].as_int();
	if(players[current] == NULL)
	{
		return;
	}
	players[current]->updateState(node);
	players[current]->stopThinking(true);

	if(publicArea->updateArea(node))
	{
		CCSprite *ptrLilyDealCard = CREATE_SPRITE(this, 
			getLilyImgName("lily_deal_card.png").c_str(), false);
		m_ptrBtnLily->setNormalImage(ptrLilyDealCard);
		m_ptrBtnLily->setSelectedImage(ptrLilyDealCard);
		scheduleOnce(schedule_selector(TableLayer::resetLilyAfterDeal), 1.0f);

		maxBet = 0;//new round,reset the max bet
		for(int i = 0;i < capacity;i++)
		{
			if(players[i] != NULL)
			{
				players[i]->clearState();
			}			
		}
		betCtrl->clear();
		if (mySelf)
		{
			mySelf->updateHandIndicator();
		}
	}

	if(JsonHelper::optExist(node,"winners"))
	{
		for(int i = 0;i < capacity;i++)
		{
			if(players[i] != NULL)
			{
				players[i]->clearState();
			}			
		}

		betCtrl->hideAllBtns();
		int exp = JsonHelper::optInt(node,"exp");
		int level = JsonHelper::optInt(node,"level");
		updateExp(level,exp);

		const JSONNode& winnersNode = node["winners"].as_array();
		int len = winnersNode.size();
		m_winCards.clear();
		int maxWinCardsType = 0;
		bool amIWin = false;
		INT64 myWinAmount = 0;
		vector<pair<int,vector<INT64> > > winnerChipsVec;
		vector<pair<int,vector<int> > > winnerCardsVec;
		for(int i = 0; i < len; i++)
		{
			int index = winnersNode[i]["index"].as_int();
			players[index]->m_amIWin = true;
			players[index]->showWinTwinkleParticle();

			INT64 totalWinAmount = 0;
			JSONNode winListNode = winnersNode[i]["winList"].as_array();
			vector<INT64> winList;
			for (int j = 0; j < winListNode.size(); j++)
			{
				INT64 temp;
				stringstream sval(winListNode[j].as_string());
				sval>>temp;
				winList.push_back(temp);
				totalWinAmount += temp;
			}
			winnerChipsVec.push_back(pair<int,vector<INT64> >(index,winList));
			
			if(mySelf && mySelf->m_index == index)
			{
				amIWin = true;
				myWinAmount = totalWinAmount;
				if (s_tableType == TABLE_TYPE_NORMAL)
				{
					updateGameRankList(totalWinAmount);
				}
			}

			vector<int> winnerCards;
			if(JsonHelper::optExist(winnersNode[i],"winCardDescriptor"))
			{
				int card;
				for (int j = 0; j < 5; j++)
				{
					card = winnersNode[i]["winCardDescriptor"][j].as_int();
					m_winCards.insert(card);
					winnerCards.push_back(card);
				}

				card = winnersNode[i]["winCardDescriptor"][5].as_int();
				winnerCards.push_back(card);
				if(maxWinCardsType < card)
				{
					maxWinCardsType = card;
				}
			}
			winnerCardsVec.push_back(pair<int,vector<int> >(index,winnerCards));
		}
		GameUtils::setOkToShowRate(myWinAmount,blindBet);
		showWinChipsActions(winnerChipsVec,winnerCardsVec);
		
		if(JsonHelper::optExist(node,"bcs"))
		{
			int len = node["bcs"].size();
			for(int i = 0;i < len;i++)
			{
				Player* player = getPlayer(node["bcs"][i]["index"].as_int());
				if (player != NULL)
				{
					player->flipCardsInHands(node["bcs"][i]["bc"].as_array());
					player->runAction(CCSequence::create(
						CCDelayTime::create(1.0f),
						CCCallFuncND::create(player, callfuncND_selector(Player::highlightWinCards),(void*)&m_winCards),
						NULL));
				}
			}
			publicArea->runAction(CCSequence::create(
				CCDelayTime::create(1.0f),
				CCCallFuncND::create(publicArea, callfuncND_selector(PublicAreaLayer::highlightWinCards),(void*)&m_winCards),
				NULL));
		}
		else
		{
			for (int i = 0; i < capacity; i++)
			{
				if (players[i] && players[i]->m_amIWin)
				{
					players[i]->runAction(CCSequence::create(
						CCDelayTime::create(1.0f),
						CCCallFunc::create(players[i], callfunc_selector(Player::showWin)),
						NULL));
				}
			}
		}
		
		if(maxWinCardsType > 0)
		{
			publicArea->showWinCardsType((WIN_CARDS_TYPE)(maxWinCardsType - 1));
			if (amIWin)
			{
				showWinParticleChips((WIN_CARDS_TYPE)(maxWinCardsType - 1));
			}
		}	
		else
		{
			if (amIWin)
			{
				showWinParticleChips((WIN_CARDS_TYPE)(0));
			}
		}
		if (isSitDown && mySelf && mySelf->m_state != Player::GAME_WAIT)
		{
			GameUtils::addDailyTaskGetSomeCardType(mySelf->getMaxCardsType());
		}
	}
	else
	{
		m_thinker = players[node["next"].as_int()];
		if(m_thinker)
		{
			m_thinker->setThinkTime(thinkTime);
			m_thinker->startThinking();
		}
		betCtrl->updateButtons(gameScreen,node);
	}
}

void TableLayer::onStartGameCommand(GameScreen* gameScreen,const JSONNode& node)
{
	if (m_bHasSendSitDownCmd)
	{
		for (int i = 0; i < capacity; i++)
		{
			removePlayer(i);
		}
		m_bHasSendSitDownCmd = false;
	}
	m_isGameStarted = true;
	m_sitDownBtnIndex = -1;
	publicArea->clear();
	betCtrl->clear();
	m_startGameNode = node;
	m_isRunning = m_isRunning || node["isRunning"].as_bool();
	stringstream sval(node["blindBet"].as_string());
	sval>>blindBet;
	stringstream tempss(m_startGameNode["max_bet"].as_string());
	tempss>>maxBet;

	set<string> currentPlayerIDs;
	vector<string> needRefreshIDs;
	for(int i = 0;i < node["joinedPlayers"].size();i++)
	{
		int index = node["joinedPlayers"][i]["index"].as_int();
		string id = node["joinedPlayers"][i]["id"].as_string();
		currentPlayerIDs.insert(id);
		if(players[index] == NULL)
		{
			needRefreshIDs.push_back(id);
			createAndAddPlayer(gameScreen,m_startGameNode["joinedPlayers"][i].as_node());
		}
		else if (players[index]->m_playerInfo.getAccountID() != id)//该位置的玩家改变了
		{
			needRefreshIDs.push_back(id);
			removePlayer(index);
			createAndAddPlayer(gameScreen,m_startGameNode["joinedPlayers"][i].as_node());
		}
		else//该位置的玩家未改变，清除当前状态
		{
			players[index]->reInitState(m_startGameNode["joinedPlayers"][i].as_node());
		}
	}

	//该位置的玩家退出，且没有新玩家，将其清除
	for (int i = 0; i < capacity; i++)
	{
		if (players[i])
		{
			set<string>::iterator itr = currentPlayerIDs.find(players[i]->m_playerInfo.getAccountID());
			if(itr == currentPlayerIDs.end())
			{
				removePlayer(i);
			}
		}
		else
		{
			hideSitDownBtn(i);
			showSitDownBtn(i);
		}
	}

	if (needRefreshIDs.size() > 0)
	{
		getPlayerInfoByNetwork(needRefreshIDs);
	}

	//服务端会延迟deltaTime后再开始think，这段时间在客户端用来移除loading界面和发牌
	float deltaTime = node["deltaTime"].as_int() / 1000.0;
	//为了发完牌再开始think，对deltaTime进行处理
	deltaTime -= getDealCardsTime();
	if (deltaTime <= 0)
	{
		gameScreen->removeLoadingLayer(0);
		startGame(0);
		CCLOG("delta time for schedule startGame:0s");
	}
	else
	{
		gameScreen->scheduleOnce(schedule_selector(GameScreen::removeLoadingLayer), deltaTime);
		scheduleOnce(schedule_selector(TableLayer::startGame), deltaTime);
		CCLOG("delta time for schedule startGame:%fs",deltaTime);
	}
}

INT64 TableLayer::getMaxRaiseReference(){
	INT64 tmp=0;
	for(int i=0;i<capacity;i++){
		if(players[i]==NULL||players[i]==mySelf){
			continue;
		}
		if(players[i]->m_state!=Player::GAME_PLAYING){
			continue;
		}
		if(tmp<players[i]->m_betAmount+players[i]->m_chipsInGame){
			tmp=players[i]->m_betAmount+players[i]->m_chipsInGame;
		}
	}
	return tmp;
}
void TableLayer::setTable(int capacity,int myPosition,INT64 blindBet,int thinkTime){
	this->capacity=capacity;
	this->blindBet=blindBet;
	setMyPosition(myPosition);
	this->thinkTime = thinkTime;
}

void TableLayer::onEnter()
{
	Layer::onEnter();
	LayoutUtil::layoutParentCenter(this);
	string tablePath = "Images/beginner_table.png";
	if (s_tableType == TABLE_TYPE_NORMAL)
	{
		tablePath = "Images/" + SelectRoomLayer::getRoomType(blindBet) + "_table.png";
	}
	CCSprite* ptrTable = ResourceManager::getInstance()->createSprite(this,tablePath.c_str());
	this->addChild(ptrTable, Z_TABLE);
	LayoutUtil::layoutParentCenter(ptrTable);

	publicArea = PublicAreaLayer::create();
	this->addChild(publicArea, Z_PUBLIC_AREA);
	LayoutUtil::layoutParentCenter(publicArea,0,10);

	betCtrl = BetControler::create();
	this->addChild(betCtrl,Z_BET_CONTROL);

	CCSprite* ptrLily = ResourceManager::getInstance()->createSprite(this,
		getLilyImgName("lily.png").c_str());
	m_ptrBtnLily = CCMenuItemSprite::create(ptrLily,ptrLily,this,menu_selector(TableLayer::onLilyBtnCallback));
	registerMenuItem(m_ptrBtnLily);
	this->addChild(m_ptrBtnLily,Z_BTN_LILY);
	LayoutUtil::layoutCenter(m_ptrBtnLily,ptrTable,0,223);

	//Lily Blink
	CCSprite *ptrLilyBlink = CREATE_SPRITE(this, 
		getLilyImgName("lily_blink.png").c_str(), false);
	CCFiniteTimeAction *ptrActBlinkOnce = dynamic_cast<CCFiniteTimeAction *>(CCSequence::create(
		CCShow::create(), CCDelayTime::create(0.1f), 
		CCHide::create(), CCDelayTime::create(0.1f), NULL));
	ptrLilyBlink->runAction(CCRepeatForever::create(
		dynamic_cast<CCActionInterval *>(CCSequence::create(
		ptrActBlinkOnce, CCDelayTime::create(4.0f), 
		ptrActBlinkOnce, CCDelayTime::create(0.1f), 
		ptrActBlinkOnce, CCDelayTime::create(3.0f), 
		NULL))));
	m_ptrBtnLily->addChild(ptrLilyBlink, 1);
	LayoutUtil::layoutParentCenter(ptrLilyBlink);

	initMoreBtn();
	initShopBtn();
	initTalkBtn();
	initRankBtn();
	initLotteryBtn();
	initPlayerInfo();
	showHint();
	initSpecialOfferCountDown();
	CCNotificationCenter::sharedNotificationCenter()->addObserver(this,
		callfuncO_selector(TableLayer::enterBackground),"enterBackground",NULL);
	CCNotificationCenter::sharedNotificationCenter()->addObserver(this,
		callfuncO_selector(TableLayer::enterForeground),"enterForeground",NULL);
	TalkLayer::clearNewMsg();

	if (GameUtils::getGameGuide() != GuideBaseLayer::GUIDE_END)
	{
		showGuide();
	}
}

void TableLayer::onExit()
{
	Layer::onExit();
	delete[] players;
	delete[] m_prtBtnSitDown;
	CCNotificationCenter::sharedNotificationCenter()->removeAllObservers(this);
}

bool TableLayer::init()
{
	if ( !Layer::init() )
    {
        return false;
    }

    return true;
}

void TableLayer::initMoreBtn()
{
	CCSprite* ptrBtnMoreNormal = CREATE_SPRITE(this,"btn_round_bg.png",false);
	ptrBtnMoreNormal->setCascadeOpacityEnabled(true);
	CCSprite* ptrArrowDown = CREATE_SPRITE(this,"arrow_up.png",false);
	ptrArrowDown->setRotation(180.0f);
	ptrArrowDown->setCascadeOpacityEnabled(true);
	ptrBtnMoreNormal->addChild(ptrArrowDown);
	LayoutUtil::layoutParentCenter(ptrArrowDown,0,-4);

	CCMenuItem* ptrBtnMore = ScaleMenuItemSprite::create(ptrBtnMoreNormal,ptrBtnMoreNormal,
		this,menu_selector(TableLayer::onMoreBtnCallback),1.0f,1.1f,255 * 0.5,255);
	addChild(ptrBtnMore, Z_BTNS);
	LayoutUtil::layoutParentLeftTop(ptrBtnMore,2,-4);
	registerMenuItem(ptrBtnMore);
}

void TableLayer::initShopBtn()
{
	CCNode* ptrBtnShopNormal = GameUtils::createShopLightEffectNode(this, "btn_buy_light_%d.png");
	CCSprite* ptrBtnTempNormal = CREATE_SPRITE(this, "btn_buy.png", false);
	ptrBtnShopNormal->addChild(ptrBtnTempNormal);
	LayoutUtil::layoutParentCenter(ptrBtnTempNormal);

	CCNode* ptrBtnShopSelected = GameUtils::createShopLightEffectNode(this, "btn_buy_light_%d.png");
	CCSprite* ptrBtnTempSelected = ResourceManager::getInstance()->createSprite(this,"btn_buy_down.png");
	ptrBtnShopSelected->addChild(ptrBtnTempSelected);
	LayoutUtil::layoutParentCenter(ptrBtnTempSelected);

	m_ptrBtnShop = MenuItem4Shop::create(ptrBtnShopNormal,ptrBtnShopSelected,NULL,
		this,menu_selector(TableLayer::onShopBtnCallback));
	addChild(m_ptrBtnShop, Z_BTNS);
	LayoutUtil::layoutParentRightTop(m_ptrBtnShop,-10,-5);
	registerMenuItem(m_ptrBtnShop);
}

void TableLayer::initTalkBtn()
{
	CCSprite* ptrBtnTalkNormal = CREATE_SPRITE(this,"btn_round_bg.png",false);
	ptrBtnTalkNormal->setCascadeOpacityEnabled(true);
	CCSprite* ptrIconTalk = CREATE_SPRITE(this,"btn_talk.png",false);
	ptrIconTalk->setCascadeOpacityEnabled(true);
	ptrBtnTalkNormal->addChild(ptrIconTalk);
	LayoutUtil::layoutParentCenter(ptrIconTalk);
	m_ptrBtnTalk = ScaleMenuItemSprite::create(ptrBtnTalkNormal,ptrBtnTalkNormal,
		this,menu_selector(TableLayer::onTalkBtnCallback),1.0f,1.1f,255 * 0.5,255);
	addChild(m_ptrBtnTalk, Z_BTNS);
	LayoutUtil::layoutParentLeftBottom(m_ptrBtnTalk,2,-2);
	registerMenuItem(m_ptrBtnTalk);
}

void TableLayer::initPlayerInfo()
{
	// Node
	CCScale9Sprite* ptrPlayerInfoBg = GameUtils::createScale9("my_info_bg.png",
		CCRectMake(0,0,522,112),CCRectMake(30,30,450,50));
	ptrPlayerInfoBg->setContentSize(CCSizeMake(192,100));
	addChild(ptrPlayerInfoBg, Z_PLAYER_INFO_BG);
	LayoutUtil::layoutParentLeftTop(ptrPlayerInfoBg,85,-2);

	// Player Name
	string name = GameUtils::cutName(GameUtils::s_myInfo.getNickName(),13);
	CCLabelTTF* ptrLbPlayerName = CCLabelTTF::create(name.c_str(),FontConstant::TTF_IMPACT,22);
	ptrLbPlayerName->setDimensions(CCSizeMake(120,ptrLbPlayerName->getContentSize().height));
	ptrLbPlayerName->setHorizontalAlignment(kCCTextAlignmentLeft);
	ptrPlayerInfoBg->addChild(ptrLbPlayerName);
	LayoutUtil::layoutParentLeft(ptrLbPlayerName,16,28);

	// Level
	m_ptrPlayerLevel = CCLabelTTF::create(CCString::createWithFormat(
		LOCALIZED_STRING("level_with_format"),GameUtils::s_myInfo.getLevel())->getCString(),
		FontConstant::TTF_IMPACT,22);
	m_ptrPlayerLevel->setColor(ccc3(0,255,255));
	ptrPlayerInfoBg->addChild(m_ptrPlayerLevel);
	LayoutUtil::layoutParentLeft(m_ptrPlayerLevel,16,3);

	// Exp
	CCSprite* ptrExpProgressBg = CREATE_SPRITE(this,"new_exp_progress_bg.png",false);
	ptrPlayerInfoBg->addChild(ptrExpProgressBg);
	LayoutUtil::layoutParentLeftBottom(ptrExpProgressBg,16,15);

	CCSprite* ptrExpProgressFront = CREATE_SPRITE(this, "new_exp_progress_bar.png", false);
	m_ptrExpProgress = CCProgressTimer::create(ptrExpProgressFront);
	m_ptrExpProgress->setType(kCCProgressTimerTypeBar);
	m_ptrExpProgress->setMidpoint(ccp(0, 0.5f));
	m_ptrExpProgress->setBarChangeRate(ccp(1, 0));
	float percent = 100.0 * (float)(GameUtils::s_myInfo.getExp()) 
		/ GameUtils::getTargetExp(GameUtils::s_myInfo.getLevel());
	m_ptrExpProgress->setPercentage(percent);
	ptrExpProgressBg->addChild(m_ptrExpProgress);
	LayoutUtil::layoutParentCenter(m_ptrExpProgress);

	CCSprite* ptrExpProgressLight = CREATE_SPRITE(this,"new_exp_progress_light.png",false);
	ptrExpProgressBg->addChild(ptrExpProgressLight);
	LayoutUtil::layoutParentCenter(ptrExpProgressLight);
}

void TableLayer::onMoreBtnCallback(CCObject* pSender)
{
	SoundPlayer::play(SNDI_CLICK);
	GameMoreMenuLayer* ptrGameMoreMenuLayer = GameMoreMenuLayer::create();
	addChild(ptrGameMoreMenuLayer, Z_GAME_MORE_MENU_LAYER);
	float offsetY = 0;
	if (s_tableType != TABLE_TYPE_NORMAL)
	{
		offsetY = 62;
	}
	LayoutUtil::layoutParentLeftTop(ptrGameMoreMenuLayer,0,36 + offsetY);
	ptrGameMoreMenuLayer->showMenu();
}

void TableLayer::onShopBtnCallback(CCObject* pSender)
{
	SoundPlayer::play(SNDI_CLICK);
	showQucikBuyLayer();
}

void TableLayer::onSpecialOfferBtnCallback( CCObject* pSender )
{
	SoundPlayer::play(SNDI_CLICK);
	if (GameUtils::getSpecialOfferEndTime() - GameUtils::getCurrentTime() <= 0)
	{
		return;
	}

	SpecialOfferLayer* ptrSpecialOfferLayer = SpecialOfferLayer::create();
	addChild(ptrSpecialOfferLayer,Z_SPECIAL_OFFER_LAYER);
	LayoutUtil::layoutParentCenter(ptrSpecialOfferLayer);
}

void TableLayer::onPhotoBtnCallback(CCObject* pSender)
{
	CCLOG("TableLayer::onPhotoBtnCallback");
	/*CCSize size = CCDirector::sharedDirector()->getWinSize();
	CCRenderTexture* texture = CCRenderTexture::create((int)size.width, (int)size.height);
	texture->setPosition(ccp(size.width / 2, size.height / 2));
	texture->begin();
	CCDirector::sharedDirector()->getRunningScene()->visit();
	texture->end();
	string path = "poker_" ;
	path += GameUtils::getCurrentTimeFormated().substr(0, 10) + GameUtils::getCurrentTimeFormated().substr(11, 18);
	path += ".jpg";
	texture->saveToFile(path.c_str(), kCCImageFormatJPEG);
	CCLOG("save to %s", path.c_str());*/
	/*
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
	JniUtils::screenShoot();
#endif
	*/
}

void TableLayer::onTalkBtnCallback(CCObject* pSender)
{
	SoundPlayer::play(SNDI_CLICK);
	// stand-by player can't send message
	if(!isSitDown)  
	{
		PKMessageBox* ptrMessageBox = PKMessageBox::create(this, PKMsgTypeOk, 
			LocalizationManager::getInstance()->getLocalizedString("reminder_send_msg"), NULL, NULL);
		this->addChild(ptrMessageBox, Z_MESSAGE_BOX);
		LayoutUtil::layoutParentCenter(ptrMessageBox);
		return ;
	}
	
	m_ptrTalkLayer = TalkLayer::create();
	if(m_ptrTalkLayer)
	{
		addChild(m_ptrTalkLayer,Z_TALK_LAYER);
		LayoutUtil::layoutParentLeftBottom(m_ptrTalkLayer);
		m_ptrTalkLayer->animateShow();
		setBtnTalkVisible(false);
	}
}

int TableLayer::getCapacity()
{
	return capacity;
}

Player* TableLayer::getPlayer(int index)
{
	if(index < 0 || index >= capacity)
	{
		return NULL;
	}
	return players[index];
}

void TableLayer::setIsRunning(bool isRunning)
{
	this->m_isRunning = isRunning;
}

bool TableLayer::getIsRunning()
{
	return m_isRunning;
}

void TableLayer::talk(string content, int index)
{
	Player* player = getPlayer(index);
	if(player == NULL)
	{
		CCLOG("TableLayer::talk:: No Such Player");
		return;
	}
	player->talk(content);
}

void TableLayer::sendTalkMsg(string content)
{
	if(!isSitDown)
	{
		PKMessageBox* ptrMessageBox = PKMessageBox::create(this, PKMsgTypeOk, 
			LOCALIZED_STRING("reminder_send_msg"), NULL, NULL);
		this->addChild(ptrMessageBox, Z_MESSAGE_BOX);
		LayoutUtil::layoutParentCenter(ptrMessageBox);
		return;
	}
	char* msg=MSGRequestCommand::encode(content.c_str());
	GameScreen* gameScreen = dynamic_cast<GameScreen*>(getParent());
	gameScreen->gameSocket->sendMsg(msg);
	delete[] msg;
}

void TableLayer::getPlayerInfoByNetwork(vector<string>& ids)
{
	if (ids.size() <= 0)
	{
		return;
	}

	vector<Cmd*> cmds;
	for (int i = 0; i < ids.size(); i++)
	{
		PlayerInfoCmd *ptrPlayerInfoCmd = CmdSender::getInstance()->getPlayerInfoCmd(ids.at(i));
		ptrPlayerInfoCmd->m_needShowWaitingPanel = false;
		cmds.push_back(ptrPlayerInfoCmd);
		CCNotificationCenter::sharedNotificationCenter()->addObserver(this,
			callfuncO_selector(TableLayer::getPlayerInfoNetworkCallback),
			(ptrPlayerInfoCmd->m_cmdName + ids.at(i)).c_str(),NULL);
	}
	CmdSender::getInstance()->sendCommands(cmds);
}

void TableLayer::getPlayerInfoNetworkCallback(CCObject* pSender)
{
	PlayerInfoCmd * ptrPlayerInfoCmd = (PlayerInfoCmd*)pSender;
	int errorNO = ptrPlayerInfoCmd->m_ptrOutput->errorInfo.errorNO;

	if(errorNO == RespHandler::OK)
	{
		PlayerInfo playerInfo(ptrPlayerInfoCmd->m_ptrOutput);
		if(m_ptrPlayerInfoLayer &&
			m_ptrPlayerInfoLayer->m_playerInfo.getAccountID() == playerInfo.getAccountID())
		{
			m_ptrPlayerInfoLayer->refreshValue(playerInfo);
		}

		for (int i = 0; i < capacity; i++)
		{
			if (players[i] && players[i]->m_playerInfo.getAccountID() == playerInfo.getAccountID())
			{
				players[i]->m_playerInfo = PlayerInfo(ptrPlayerInfoCmd->m_ptrOutput);
				players[i]->refreshPhoto();

				if(playerInfo.getPhoto() < 0)
				{
					CCFileUtils* fileUtils = CCFileUtils::sharedFileUtils();
					string photoPath = GameUtils::WRITABLE_PHOTO_PATH + playerInfo.getAccountID() + ".jpg";
					if(!fileUtils->isFileExist(photoPath))
					{
						m_needGetPhotoIDs.push_back(playerInfo.getAccountID());
					}
				}
				break;
			}
		}
	}
	RespHandler::getInstance()->setLastErrorNO(RespHandler::OK);	
	CCNotificationCenter* notificationCenter = CCNotificationCenter::sharedNotificationCenter();
	notificationCenter->removeObserver(this,(ptrPlayerInfoCmd->m_cmdName + ptrPlayerInfoCmd->m_ptrOutput->accountID).c_str());

	vector<string> tempIDs;
	list<string>::iterator it;
	for (it = m_needGetPhotoIDs.begin(); it != m_needGetPhotoIDs.end(); it++)
	{
		if(notificationCenter->getObserverHandlerByName(it->c_str()) < 0)
		{
			notificationCenter->addObserver(this,callfuncO_selector(TableLayer::getPhotoNetworkCallback),
				it->c_str(),(CCObject*)&(*it));
			tempIDs.push_back(*it);
		}
	}
	CmdSender::getInstance()->getPhoto(tempIDs);
	CC_SAFE_DELETE(ptrPlayerInfoCmd);
}

void TableLayer::startGame(float dt)
{
	GameScreen* gameScreen = dynamic_cast<GameScreen*>(getParent());
	gameScreen->removeLoadingLayer(0);

	for(int i = 0;i < capacity;i++)
	{	
		Player* player = getPlayer(i);
		if (player == NULL)
		{
			continue;
		}

		if(i == myPosition && isSitDown)
		{
			mySelf = player;
			if(JsonHelper::optExist(m_startGameNode,"bc"))
			{
				player->addBaseCards(m_startGameNode["bc"].as_array(),m_isRunning);
			}
		}
		else
		{
			player->addBaseCards(JSONNode(JSON_ARRAY),m_isRunning);
		}
	}
	if (mySelf == NULL)
	{
		hideHint();
	}
	else if (mySelf && mySelf->m_state != Player::GAME_WAIT)
	{
		hideHint();
	}
	else if (mySelf && mySelf->m_state == Player::GAME_WAIT && m_ptrLbHint == NULL)
	{
		showHint();
	}

	//进入后台再返回前台后，如果之前是坐下而现在是站着的状态，就将之前的chipsInGame加上账户上
	if (!isSitDown && m_tempChipsInGame != 0)
	{
		GameUtils::s_myInfo.setChip(GameUtils::s_myInfo.getChip() + m_tempChipsInGame);
		m_tempChipsInGame = 0;
	}

	//用于同步Lily、发牌、时间条等动画，等发完牌后再播放时间条的Action
	float deltaTime = m_startGameNode["deltaTime"].as_int() / 1000.0;
	float dealCardsTime = getDealCardsTime();
	if (deltaTime > dealCardsTime)
	{
		deltaTime = dealCardsTime;
	}
	dealCards(dealCardsTime);

	m_thinker = getPlayer(m_startGameNode["next"].as_int());
	if (m_thinker)
	{
		m_thinker->setThinkTime(thinkTime);
		m_thinker->startThinking(deltaTime);
	}

	betCtrl->updateButtons(gameScreen,m_startGameNode,deltaTime);
}

void TableLayer::getPhotoNetworkCallback(CCObject* pSender)
{
	string* id = (string*)pSender;
	CCLOG("TableLayer::getPhotoNetworkCallback:id:%s",id->c_str());
	CCNotificationCenter::sharedNotificationCenter()->removeObserver(this,id->c_str());
	
	for (int i = 0; i < capacity; i++)
	{
		if(players[i] && players[i]->m_playerInfo.getAccountID() == *id)
		{
			players[i]->refreshPhoto();
			if(m_ptrPlayerInfoLayer && 
				m_ptrPlayerInfoLayer->m_playerInfo.getAccountID() == players[i]->m_playerInfo.getAccountID())
			{
				m_ptrPlayerInfoLayer->refreshPhoto();
			}
			break;
		}
	}

	list<string>::iterator it = m_needGetPhotoIDs.begin();
	while (it != m_needGetPhotoIDs.end())
	{
		if(*it == *id)
		{
			m_needGetPhotoIDs.erase(it);
			break;
		}
		it++;
	}
}

void TableLayer::onRecharge(float dlt/*0.0*/)
{
	if(s_tableInfo.minBuyIn <= GameUtils::s_myInfo.getChip())//chip is enough,buy in
	{
		HoldemTablesLayer* ptrHoldemTableLayer = HoldemTablesLayer::create();
		this->addChild(ptrHoldemTableLayer, Z_HOLDEM_TABLE_LAYER);
		ptrHoldemTableLayer->setSliderValue(s_tableInfo.minBuyIn,s_tableInfo.maxBuyIn,true);
		LayoutUtil::layoutParentCenter(ptrHoldemTableLayer);
		if (thinkTime < 10)
		{
			ptrHoldemTableLayer->setSpeed("normal",false);
		}
		else
		{
			ptrHoldemTableLayer->setSpeed("slow",false);
		}
	}
	else//chip is not enough,skip to shop
	{
		showQucikBuyLayer();
	}
}

void TableLayer::setMyPosition(int myPosition)
{
	if(myPosition<0)
	{
		isSitDown = false;
		mySelf = NULL;
	}
	else
	{
		isSitDown = true;
		this->myPosition = myPosition;
	}
}

void TableLayer::sendSitDownCmd(INT64 buyIn)
{
	CCLOG("TableLayer::sendSitDownCmd:buyIn:%lld",buyIn);
	int index = m_sitDownBtnIndex;
	if (index == -1)
	{
		index = myPosition;
	}
	GameScreen* gameScreen = dynamic_cast<GameScreen*>(getParent());
	if (gameScreen)
	{
		m_newBuyIn = buyIn;
		GameUtils::s_myInfo.setChip(GameUtils::s_myInfo.getChip() - buyIn);

		char* msg=SitDownCommand::encode(index,buyIn);
		gameScreen->gameSocket->sendMsg(msg);
		delete[] msg;
	}

	m_bHasSendSitDownCmd = true;
}

void TableLayer::backToUI()
{
	SoundPlayer::play(SNDI_EXIT_ROOM);
	if (mySelf != NULL)
	{
		if (s_tableType == TABLE_TYPE_NORMAL)
		{
			GameUtils::s_myInfo.setChip(GameUtils::s_myInfo.getChip() + mySelf->m_chipsInGame);
		}
		else if(!m_isGameStarted)
		{
			GameUtils::s_myInfo.setChip(GameUtils::s_myInfo.getChip() + s_tableInfo.buyIn);
		}	
	}
	else
	{
		if (s_tableType != TABLE_TYPE_NORMAL && !m_isGameStarted)
		{
			GameUtils::s_myInfo.setChip(GameUtils::s_myInfo.getChip() + s_tableInfo.buyIn);
		}
	}
	CCScene* uiScene = CoverLayer::scene(true);
	CCDirector::sharedDirector()->replaceScene(uiScene);
}

bool TableLayer::onKeyBackClicked()
{
	onMoreBtnCallback(NULL);
	return true;
}

void TableLayer::sendGiftCmd(string code)
{
	CCLOG("TableLayer::sendGiftCmd:code:%s",code.c_str());
	int toWhom,row,col,num;
	toWhom = Utils::atoi(code.substr(0,1),row);
	row = Utils::atoi(code.substr(2,1),row);
	col = Utils::atoi(code.substr(1,1),col);
	num = Utils::atoi(code.substr(3,1),num);
	INT64 giftPrice;
	if (col == 9)
	{
		giftPrice = blindBet;
		num = 1;
	}
	else
	{
		giftPrice = GameUtils::getGiftPrice(row,col);
	}
	
	if (col % 3 == 2)//use diamond
	{
		if (giftPrice * num > GameUtils::s_myInfo.getDiamond())
		{
			return;
		}
	}
	else//use chip
	{
		if (giftPrice * num > GameUtils::s_myInfo.getChip())
		{
			return;
		}
	}
	
	SendTableGiftCmd* ptrSendTableGiftCmd = CmdSender::getInstance()->getSendTableGiftCmd(code,GameScreen::phpSessionID);
	ptrSendTableGiftCmd->m_needShowWaitingPanel = false;
	CCNotificationCenter::sharedNotificationCenter()->addObserver(this,
		callfuncO_selector(TableLayer::sendGiftNetworkCallback),
		ptrSendTableGiftCmd->m_cmdName.c_str(),
		NULL);
	CmdSender::getInstance()->sendCommands(1,ptrSendTableGiftCmd);
}

void TableLayer::sendGiftNetworkCallback(CCObject* pSender)
{
	SendTableGiftCmd * ptrSendTableGiftCmd = (SendTableGiftCmd*)pSender;
	int errorNO = ptrSendTableGiftCmd->m_ptrOutput->errorInfo.errorNO;
	string errorMsg = ptrSendTableGiftCmd->m_ptrOutput->errorInfo.errorMsg;

	if(errorNO == RespHandler::OK)
	{
		GameUtils::s_myInfo.setChip(ptrSendTableGiftCmd->m_ptrOutput->chip);
		GameUtils::s_myInfo.setDiamond(ptrSendTableGiftCmd->m_ptrOutput->diamond);

		GameScreen* gameScreen = (GameScreen*)getParent();
		JSONCommand* lcmd=JSONCommand::createJSONCMD(JSONCommand::CMD_U_GIFT_REQUEST);
		char* msg=lcmd->encode(gameScreen);
		gameScreen->gameSocket->sendMsg(msg);
		delete lcmd;
	}
	RespHandler::getInstance()->setLastErrorNO(RespHandler::OK);
	CCNotificationCenter::sharedNotificationCenter()->removeObserver(this,ptrSendTableGiftCmd->m_cmdName.c_str());
	CC_SAFE_DELETE(ptrSendTableGiftCmd);
}

int TableLayer::getPlayerNum()
{
	int num = 0;
	for (int i = 0; i < capacity; i++)
	{
		if (players[i] != NULL)
		{
			num++;
		}
	}
	return num;
}

void TableLayer::showQucikBuyLayer()
{
	QuickBuyLayer* ptrQuickBuyLayer = QuickBuyLayer::create();
	addChild(ptrQuickBuyLayer, Z_QUICK_BUY_LAYER);
	LayoutUtil::layoutParentCenter(ptrQuickBuyLayer);
}

void TableLayer::onLilyBtnCallback(CCObject* pSender)
{
	if (!isSitDown)
	{
		return;
	}
	SoundPlayer::play(SNDI_CLICK);
	sendGiftCmd("9999");
}

void TableLayer::showTipActions(int index)
{
	if (players[index] == NULL)
	{
		return;
	}

	// Lily smile
	CCAnimation *ptrAnimationSmile = CCAnimation::create();
	ptrAnimationSmile->addSpriteFrame(ResourceManager::getInstance()->createFrame(this,
		getLilyImgName("lily_smile_1.png").c_str()));
	ptrAnimationSmile->addSpriteFrame(ResourceManager::getInstance()->createFrame(this,
		getLilyImgName("lily_smile_2.png").c_str()));
	ptrAnimationSmile->addSpriteFrame(ResourceManager::getInstance()->createFrame(this,
		getLilyImgName("lily_smile_1.png").c_str()));
	ptrAnimationSmile->addSpriteFrame(ResourceManager::getInstance()->createFrame(this,
		getLilyImgName("lily_smile_1.png").c_str()));
	ptrAnimationSmile->setDelayPerUnit(0.5f);
	ptrAnimationSmile->setRestoreOriginalFrame(false);
	ptrAnimationSmile->setLoops(1);

	CCAnimate *ptrAnimateSmile = CCAnimate::create(ptrAnimationSmile);
	CCSprite *ptrLilySmile = CCSprite::create();
	ptrLilySmile->runAction(CCSequence::create(CCDelayTime::create(0.5),ptrAnimateSmile,
							CCCallFunc::create(ptrLilySmile, callfunc_selector(CCSprite::removeFromParent)), NULL));
	m_ptrBtnLily->addChild(ptrLilySmile, 2);
	LayoutUtil::layoutParentCenter(ptrLilySmile);

	// Lily talk
	CCSprite* ptrPop = ResourceManager::getInstance()->createSprite(this,"pop.png");
	ptrPop->setAnchorPoint(ccp(0,0.5));
	CCLabelTTF* ptrLbThanks = CCLabelTTF::create(
		LocalizationManager::getInstance()->getLocalizedString("word_thanks"),FontConstant::TTF_IMPACT,20);
	ptrLbThanks->setColor(ccc3(0,0,0));
	ptrPop->addChild(ptrLbThanks);
	string name = GameUtils::cutName(players[index]->m_playerInfo.getNickName(),12);
	CCLabelTTF* ptrLbName = CCLabelTTF::create(name.c_str(),FontConstant::TTF_IMPACT,20);
	ptrLbName->setDimensions(CCSizeMake(115,ptrLbName->getContentSize().height));
	ptrLbName->setHorizontalAlignment(kCCTextAlignmentCenter);
	ptrLbName->setColor(ccc3(0,0,0));
	ptrPop->addChild(ptrLbName);
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
	LayoutUtil::layoutParentTop(ptrLbThanks,0,-13);
	LayoutUtil::layoutBottom(ptrLbName,ptrLbThanks,0,2);
#else
	LayoutUtil::layoutParentTop(ptrLbThanks,0,-18);
	LayoutUtil::layoutBottom(ptrLbName,ptrLbThanks);
#endif

	m_ptrBtnLily->addChild(ptrPop);
	LayoutUtil::layoutParentRight(ptrPop,115,40);
	ptrPop->setScale(0.1f);
	ptrPop->setVisible(false);
	CCAction* ptrAct = CCSequence::create(
		CCDelayTime::create(0.5),
		CCShow::create(),
		CCEaseSineIn::create(CCScaleTo::create(0.2f, 1.0f)),
		CCDelayTime::create(2.0f),
		CCEaseSineOut::create(CCScaleTo::create(0.2f, 0.1f)),
		CCCallFunc::create(ptrPop,callfunc_selector(CCNode::removeFromParent)),
		NULL);
	ptrPop->runAction(ptrAct);

	CCNode* ptrChipWithBG = createChipAmountWithBG(blindBet);
	addChild(ptrChipWithBG,Z_TIP_ACTION);
	ptrChipWithBG->setPosition(players[index]->getPosition());
	ptrChipWithBG->runAction(CCSequence::create(
		CCMoveTo::create(1.0f,m_ptrBtnLily->getPosition()),
		CCDelayTime::create(0.3f),
		CCCallFunc::create(ptrChipWithBG,callfunc_selector(CCNode::removeFromParent)),
		NULL));
}

void TableLayer::sendGotoTableCmd(string fID)
{
	for (int i = 0; i < capacity; i++)
	{
		if (players[i] && fID == players[i]->m_playerInfo.getAccountID())
		{
			PKMessageBox* ptrMsgBox = PKMessageBox::create(this,PKMsgTypeOk,
				LocalizationManager::getInstance()->getLocalizedString("reminder_play_with_friend"),NULL,NULL);
			addChild(ptrMsgBox, Z_MESSAGE_BOX);
			LayoutUtil::layoutParentCenter(ptrMsgBox);
			return;
		}
	}
	/*if (mySelf && isSitDown)
	{
		GameUtils::s_myInfo.setChip(GameUtils::s_myInfo.getChip() + mySelf->chipsInGame);
	}*/

	GameScreen* ptrGameScreen = (GameScreen*)getParent();
	ptrGameScreen->unschedule(schedule_selector(GameScreen::update));
	ptrGameScreen->gameSocket->destroy();

	NewGotoTableCmd *ptrGotoTableCmd = CmdSender::getInstance()->getNewGotoTableCmd(fID);
	CCNotificationCenter::sharedNotificationCenter()->addObserver(this,
		callfuncO_selector(TableLayer::gotoTableNetworkCallback),
		ptrGotoTableCmd->m_cmdName.c_str(),
		NULL);
	CmdSender::getInstance()->sendCommands(1,ptrGotoTableCmd);
}

void TableLayer::gotoTableNetworkCallback(CCObject* pSender)
{
	NewGotoTableCmd * ptrGotoTableCmd = (NewGotoTableCmd*)pSender;
	int errorNO = ptrGotoTableCmd->m_ptrOutput->errorInfo.errorNO;
	string errorMsg = ptrGotoTableCmd->m_ptrOutput->errorInfo.errorMsg;

	if(errorNO == RespHandler::OK)
	{
		GameScreen::phpSessionID = ptrGotoTableCmd->m_ptrOutput->session;
		GameScreen::host = ptrGotoTableCmd->m_ptrOutput->host;
		GameScreen::port = ptrGotoTableCmd->m_ptrOutput->port;
		TableLayer::s_tableInfo = ptrGotoTableCmd->m_ptrOutput->tableInfo;
		TableLayer::s_tableType = TABLE_TYPE_NORMAL;
		GameUtils::s_myInfo.setChip(ptrGotoTableCmd->m_ptrOutput->chip);
		GameUtils::s_myInfo.setDiamond(ptrGotoTableCmd->m_ptrOutput->diamond);

		StatisticsManager::getInstance()->onEnterTable(
			GameUtils::getDeviceID().c_str(),
			GameUtils::getReferrer().c_str(),
			GameUtils::getGiftVersion(),
			GameUtils::s_myInfo.getLevel(),
			GameUtils::s_myInfo.getChip(),
			GameUtils::s_myInfo.getDiamond(),
			"join_table_in_game",
			ptrGotoTableCmd->m_ptrOutput->tableInfo.bigBlind,
			ptrGotoTableCmd->m_ptrOutput->tableInfo.buyIn);

		CCNotificationCenter::sharedNotificationCenter()->removeObserver(this,ptrGotoTableCmd->m_cmdName.c_str());
		CC_SAFE_DELETE(ptrGotoTableCmd);
	}

	if(errorNO == RespHandler::OK)
	{
		CCScene* gameScene = GameScreen::scene();
		CCDirector::sharedDirector()->replaceScene(gameScene);
	}
}

void TableLayer::removePlayer(int index)
{
	if (index < 0 || index >= capacity)
	{
		return;
	}

	if(players[index] != NULL)
	{
		CCLOG("TableLayer::removePlayer:index:%d",index);
		if (m_ptrPlayerInfoLayer 
			&& m_ptrPlayerInfoLayer->m_playerInfo.getAccountID() == players[index]->m_playerInfo.getAccountID())
		{
			m_ptrPlayerInfoLayer->removeFromParent();
			m_ptrPlayerInfoLayer = NULL;
		}
		players[index]->removeFromParent();
		players[index] = NULL;
	}
	showSitDownBtn(index);
}

void TableLayer::hideSitDownBtn(int index)
{
	if (index < 0 || index >= capacity)
	{
		return;
	}

	if (m_prtBtnSitDown[index])
	{
		CCLOG("TableLayer::hideSitDownBtn:index:%d",index);
		m_prtBtnSitDown[index]->setVisible(false);
	}
}

void TableLayer::showSitDownBtn(int index)
{
	if (index < 0 || index >= capacity ||players[index] != NULL)
	{
		return;
	}

	if (isSitDown || s_tableType != TABLE_TYPE_NORMAL)
	{
		return;
	}

	CCLOG("TableLayer::showSitDownBtn:index:%d",index);
	if (m_prtBtnSitDown[index] == NULL)
	{
		createSitDownBtn(index);
	}
	layoutSitDownBtn(index);
	m_prtBtnSitDown[index]->setVisible(true);
}

void TableLayer::createSitDownBtn(int index)
{
	if (index < 0 || index >= capacity)
	{
		return;
	}

	CCSprite* ptrBtnNormal = ResourceManager::getInstance()->createSprite(this,"btn_sit.png");
	CCSprite* ptrBtnSelected = ResourceManager::getInstance()->createSprite(this,"btn_sit_down.png");
	m_prtBtnSitDown[index] = CCMenuItemSprite::create(ptrBtnNormal,ptrBtnSelected,this,menu_selector(TableLayer::onSitDownBtnCallback));
	m_prtBtnSitDown[index]->setTag(index);
	addChild(m_prtBtnSitDown[index], Z_BTN_SIT_DOWN);
	registerMenuItem(m_prtBtnSitDown[index]);
}

void TableLayer::onSitDownBtnCallback(CCObject* pSender)
{
	if (m_sitDownBtnIndex >= 0)
	{
		return;
	}

	unschedule(schedule_selector(TableLayer::onRecharge));
	SoundPlayer::play(SNDI_CLICK);
	int index = ((CCMenuItemSprite*)pSender)->getTag();
	int seat = (capacity + index - myPosition) % capacity;
	CCLOG("TableLayer::onSitDownBtnCallback:index:%d,seat:%d",index,seat);
	m_sitDownBtnIndex = index;
	onRecharge();
}

bool TableLayer::addGagByAccountId( string accountID,int index )
{
	if(m_gagPlayerIdx.find(accountID) != m_gagPlayerIdx.end())
	{
		return false;
	}
	if(accountID == GameUtils::s_myInfo.getAccountID())
	{
		return false;
	}
	m_gagPlayerIdx.insert(accountID);

	DoBanCmd* ptrDoBanCmd = CmdSender::getInstance()->getDoBanCmd(accountID);
	ptrDoBanCmd->m_needShowWaitingPanel = false;
	CCNotificationCenter::sharedNotificationCenter()->addObserver(this,
		callfuncO_selector(TableLayer::doBanCmdNetworkCallback),
		ptrDoBanCmd->m_cmdName.c_str(),NULL);
	CmdSender::getInstance()->sendCommands(1,ptrDoBanCmd);

	Player* player = getPlayer(index);
	if (player)
	{
		player->refreshPhoto();
	}

	return true;
}

bool TableLayer::isInGagSet(string accountID)
{
	if(m_gagPlayerIdx.find(accountID) != m_gagPlayerIdx.end())
	{
		return true;
	}
	return false;
}

bool TableLayer::deleteGagByAccountId( string accountID,int index )
{
	if(m_gagPlayerIdx.find(accountID) == m_gagPlayerIdx.end())
	{
		return false;
	}
	m_gagPlayerIdx.erase(accountID);
	Player* player = getPlayer(index);
	if (player)
	{
		player->refreshPhoto();
	}
	return true;
}

void TableLayer::updateExp(int level, int exp)
{
	if (!mySelf || (mySelf && mySelf->m_state == Player::GAME_WAIT))
	{
		return;
	}
	m_ptrPlayerLevel->setString(CCString::createWithFormat(
		LOCALIZED_STRING("level_with_format"),level)->getCString());

	float percent = 100.0 * (float)(exp) / GameUtils::getTargetExp(level);
	m_ptrExpProgress->setPercentage(percent);

	int deltaLevel = level - GameUtils::s_myInfo.getLevel();
	int deltaExp = exp - GameUtils::s_myInfo.getExp();
	while (deltaLevel > 0)
	{
		deltaLevel--;
		deltaExp += GameUtils::getTargetExp(GameUtils::s_myInfo.getLevel() + deltaLevel);
	}
	CC_ASSERT(deltaExp >= 0);
	if (level > GameUtils::s_myInfo.getLevel())
	{
		if (level >= 3 && GameUtils::s_myInfo.getType() != "facebook")
		{
			GameUtils::setIsOkToShowBindFB(true);
		}
		int newVipScore = GameUtils::s_myInfo.getVipScore() + level;
		if (VipLayer::getVipLevel(newVipScore) > GameUtils::getMyVipLevel())
		{
			if (GameUtils::s_bIsVipOn)
			{
				MainMenuLayer::s_bNeedShowVipUpgrade = true;
			}
		}
		GameUtils::s_myInfo.setVipScore(newVipScore);
	}
	GameUtils::s_myInfo.setLevel(level);
	GameUtils::s_myInfo.setExp(exp);

	//show actions
	CCParticleSystemQuad* ptrSuitParticle = CCParticleSystemQuad::create("Particle/suit_icon.plist");
	CCTexture2D* ptrTexture = CCTextureCache::sharedTextureCache()->addImage(
		CCString::createWithFormat("Particle/suit_%d.png",deltaExp % 4)->getCString());
	if (ptrTexture)
	{
		ptrSuitParticle->setTexture(ptrTexture);
	}
	
	float duration = ptrSuitParticle->getDuration();
	float delayTime = 1.0f;
	ptrSuitParticle->setDuration(duration + delayTime);
	ptrSuitParticle->setPosition(mySelf->getPosition());	
	addChild(ptrSuitParticle, Z_UPDATE_EXP_PARTICLE);
	ptrSuitParticle->setVisible(false);

	CCLabelTTF* ptrLbExp = CCLabelTTF::create(CCString::createWithFormat(
		LocalizationManager::getInstance()->getLocalizedString("add_exp"),deltaExp)->getCString(),
		FontConstant::TTF_IMPACT,20);
	ptrLbExp->setColor(ccc3(0,203,181));	

	CCSprite* ptrExpMark = ResourceManager::getInstance()->createSprite(this,"exp_mark.png");

	CCScale9Sprite* ptrExpBg = GameUtils::createScale9("bet_chip_bg.png",CCRectMake(0,0,10,10),CCRectMake(17,0,64,20));
	ptrExpBg->setContentSize(CCSizeMake(ptrExpMark->getContentSize().width + ptrLbExp->getContentSize().width + 10,20));
	ptrSuitParticle->addChild(ptrExpBg);
	LayoutUtil::layoutParentLeftTop(ptrExpBg,-12,12);	
	ptrExpBg->addChild(ptrExpMark);
	LayoutUtil::layoutParentLeft(ptrExpMark,3,0);
	ptrExpBg->addChild(ptrLbExp);
	LayoutUtil::layoutRight(ptrLbExp,ptrExpMark,5,0);
	
	CCPoint temp = m_ptrExpProgress->getParent()->convertToWorldSpace(m_ptrExpProgress->getPosition());
	CCPoint dest = convertToNodeSpace(temp);
	ptrSuitParticle->runAction(CCSequence::create(
		CCDelayTime::create(delayTime),
		CCShow::create(),
		CCCallFunc::create(this,callfunc_selector(TableLayer::playSoundGetExp)),
		CCMoveTo::create(duration,dest),
		CCCallFunc::create(ptrSuitParticle,callfunc_selector(CCNode::removeFromParent)),
		NULL));
}

float TableLayer::getDealCardsTime()
{
	//每张牌间隔0.15s，每张牌耗时0.4s
	int num = 0;
	for (int i = 0; i < capacity; i++)
	{
		Player* player = getPlayer(i);
		if (player != NULL && player->m_state != Player::GAME_WAIT)
		{
			num++;
		}
	}
	float dealCardsTime = 0.4 + (num * 2 - 1) * 0.15;
	CCLOG("deal cards time:%fs",dealCardsTime);
	return dealCardsTime;
}

void TableLayer::showSNGResult(int rank,INT64 bonus)
{
	if (rank <= 0)
	{
		return;
	}

	SNGResultLayer* ptrSNGResultLayer = SNGResultLayer::create(rank,bonus);
	addChild(ptrSNGResultLayer,Z_SNG_RESULT_LAYER);
	if (rank <= 3)
	{
		m_isShowSNGOrSTOLayer = true;
		LayoutUtil::layoutParentCenter(ptrSNGResultLayer);
	}
	else
	{
		LayoutUtil::layoutParentBottom(ptrSNGResultLayer);
	}
	ptrSNGResultLayer->animateShow();
}

void TableLayer::showSNGLayer()
{
	SNGLayer* ptrSNGLayer = SNGLayer::create(true);
	addChild(ptrSNGLayer,Z_SNG_LAYER);
	LayoutUtil::layoutParentCenter(ptrSNGLayer);
}

void TableLayer::showNotificationBar( string msg, float autoHideTime/*=0*/ )
{
	CCLOG("TableLayer::showNotificationBar:%s",msg.c_str());
	NotificationBar* ptrNotificationBar = NotificationBar::getInstance();
	ptrNotificationBar->setNotificationMsg(msg);
	if (autoHideTime > 0)
	{
		ptrNotificationBar->setAutoHide(autoHideTime);
	}
	addChild(ptrNotificationBar,Z_NOTIFICATION_BAR);
	LayoutUtil::layoutParentTop(ptrNotificationBar);
	ptrNotificationBar->animateShow();
}

void TableLayer::showShootOutResult( int rank, INT64 bonus )
{
	if (rank <= 0)
	{
		return;
	}

	ShootOutResultLayer* ptrShootOutResultLayer = ShootOutResultLayer::create(rank,bonus);
	addChild(ptrShootOutResultLayer,Z_SNG_RESULT_LAYER);
	if (rank == 1)
	{
		m_isShowSNGOrSTOLayer = true;
		int sotStage = GameUtils::s_myInfo.getSotStage() + 1;
		if (sotStage >= 4)
		{
			sotStage = 1;
		}
		GameUtils::s_myInfo.setSotStage(sotStage);
		LayoutUtil::layoutParentCenter(ptrShootOutResultLayer);
	}
	else
	{
		LayoutUtil::layoutParentBottom(ptrShootOutResultLayer);
	}
	ptrShootOutResultLayer->animateShow();
}

void TableLayer::showHint()
{
	hideHint();
	m_ptrLbHint = CCLabelTTF::create("",FontConstant::TTF_IMPACT,22);
	string strHint;
	switch (s_tableType)
	{
	case TABLE_TYPE_NORMAL:
		strHint = LocalizationManager::getInstance()->getLocalizedString("wait_for_next_hand");
		break;
	case TABLE_TYPE_SNG:
		strHint = CCString::createWithFormat(LocalizationManager::getInstance()->getLocalizedString("wait_for_sng"),
			GameUtils::num2tableStr(s_tableInfo.buyIn,blindBet,8).c_str())->getCString();
		break;
	case TABLE_TYPE_SHOOT_OUT:
		strHint = LocalizationManager::getInstance()->getLocalizedString("wait_for_shootout");
		break;
	default:
		break;
	}
	m_ptrLbHint->setString(strHint.c_str());
	addChild(m_ptrLbHint,Z_HINT);
	LayoutUtil::layoutParentBottom(m_ptrLbHint,0,13);
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
	LayoutUtil::layoutParentBottom(m_ptrLbHint,0,5);
#endif
	CCLOG("TableLayer::showHint:%s",strHint.c_str());
	if (m_ptrBtnSpeicalOffer)
	{
		m_ptrBtnSpeicalOffer->setVisible(false);
	}
}

void TableLayer::hideHint()
{
	if (m_ptrLbHint != NULL)
	{
		CCLOG("TableLayer::hideHint");
		m_ptrLbHint->removeFromParent();
		m_ptrLbHint = NULL;
	}
	if (m_ptrBtnSpeicalOffer)
	{
		m_ptrBtnSpeicalOffer->setVisible(true);
	}
}

void TableLayer::enterBackground( CCObject* pSender )
{
	GameScreen* gameScreen = dynamic_cast<GameScreen*>(getParent());
	if (gameScreen)
	{
		if (mySelf && isSitDown)
		{
			m_tempChipsInGame = mySelf->m_chipsInGame;
		}
		char* msg = PauseRequestCommand::encode();
		gameScreen->gameSocket->sendMsg(msg);
		delete[] msg;

		while(gameScreen->gameSocket->check())
		{
			JSONCommand* jsncmd = JSONCommand::decode(gameScreen->gameSocket);
			if (jsncmd == NULL)
			{
				break;
			}
			else
			{
				delete jsncmd;
			}
		}
	}
}

void TableLayer::enterForeground( CCObject* pSender )
{
	GameScreen* gameScreen = dynamic_cast<GameScreen*>(getParent());
	if (gameScreen)
	{
		/*if(!gameScreen->gameSocket->check())
		{
			backToUI();
			return;
		}*/
		char* msg = ResumeRequestCommand::encode();
		gameScreen->gameSocket->sendMsg(msg);
		delete[] msg;
	}
}

void TableLayer::showGuide()
{
	// show more menu
	onMoreBtnCallback(NULL);

	float offsetY = 0;
	if (TableLayer::s_tableType != TABLE_TYPE_NORMAL)
	{
		offsetY = 62;
	}

	// add guide layer
	GuideBaseLayer *ptrGuideLayer = GuideBaseLayer::create();
	addChild(ptrGuideLayer, Z_GUIDE);
	LayoutUtil::layoutParentCenter(ptrGuideLayer);

	ptrGuideLayer->setSelector(this, NULL, NULL);
	ptrGuideLayer->setStencil(CCSizeMake(240, 55), ccp(25, -185 + offsetY), GuideBaseLayer::P_LEFT_TOP);
 
 	CCAction *act = CCRepeatForever::create(static_cast<CCActionInterval *>(CCSequence::create(
		CCEaseIn::create(CCMoveBy::create(0.5f, ccp(0, 30)), 3.0f), 
		CCEaseOut::create(CCMoveBy::create(0.5f, ccp(0, -30)), 3.0f), NULL)));
 	ptrGuideLayer->addPointer(act, ccp(40, -60), GuideBaseLayer::P_LEFT_TOP);

	ptrGuideLayer->addGuideWord(LOCALIZED_STRING("guide_hand_ranks"), 
		CCSizeMake(300, 120), ccp(0, -260), GuideBaseLayer::P_LEFT_TOP,
		GuideBaseLayer::DIR_DEFAULT);

	GameUtils::setGameGuide(GuideBaseLayer::GUIDE_END);
}

void TableLayer::showWinParticleChips( WIN_CARDS_TYPE type )
{
	if(type < TYPE_HIGH_CARD)
	{
		return;
	}

	int count = 1;
	if (type > TYPE_HIGH_CARD && type <= TYPE_FLUSH)
	{
		count = 2;
	}
	else if( type > TYPE_FLUSH && type <= TYPE_ROYAL_FLUSH)
	{
		count = 3;
	}

	for (int i = 0; i < count; i++)
	{
		CCParticleSystemQuad* ptrChipParticle = CCParticleSystemQuad::create("Particle/win_chips.plist");
		ptrChipParticle->setTexture(CCTextureCache::sharedTextureCache()->addImage(
			CCString::createWithFormat("Particle/particle_chip_%d.png",i)->getCString()));
		ptrChipParticle->setVisible(false);
		addChild(ptrChipParticle,Z_WIN_CHIPS_PARTICLE);
		LayoutUtil::layoutParentBottom(ptrChipParticle,-10,-160);
		ptrChipParticle->runAction(CCSequence::create(
			CCDelayTime::create(1.0f),
			CCShow::create(),
			CCDelayTime::create(ptrChipParticle->getDuration() + 2.0f),
			CCCallFunc::create(ptrChipParticle,callfunc_selector(CCNode::removeFromParent)),
			NULL));
	}
	if (count >= 2)
	{
		SoundPlayer::play(SNDI_MY_BIG_WIN,1,1000);
	}
	else
	{
		SoundPlayer::play(SNDI_MY_SMALL_WIN,1,1000);
	}
}

void TableLayer::playSoundGetExp()
{
	SoundPlayer::play(SNDI_GET_EXP);
}

void TableLayer::resetLilyAfterDeal(float dt)
{
	CCSprite *ptrLily = CREATE_SPRITE(this, 
		getLilyImgName("lily.png").c_str(), false);
	m_ptrBtnLily->setNormalImage(ptrLily);
	m_ptrBtnLily->setSelectedImage(ptrLily);
}

void TableLayer::initLotteryBtn()
{
	CCSprite* ptrBtnLotteryNormal = CREATE_SPRITE(this,"game_lottery_arrow.png",false);
	ptrBtnLotteryNormal->setCascadeOpacityEnabled(true);
	m_ptrBtnLottery = ScaleMenuItemSprite::create(ptrBtnLotteryNormal, ptrBtnLotteryNormal, 
		this, menu_selector(TableLayer::onLotteryBtnCallback),1.0f,1.1f,0.5 * 255,255);
	m_ptrBtnLottery->setCascadeOpacityEnabled(true);
	registerMenuItem(m_ptrBtnLottery);
	addChild(m_ptrBtnLottery, Z_BTNS);
	LayoutUtil::layoutParentLeftBottom(m_ptrBtnLottery,126,-42);

	CCLabelBMFont* ptrLbSpin = CCLabelBMFont::create(LOCALIZED_STRING("menu_lottery"),
		FontConstant::BMFONT_GOTHIC725_BLK_BT_BLUE);
	ptrLbSpin->setCascadeOpacityEnabled(true);
	ptrLbSpin->setScale(0.375f);
	m_ptrBtnLottery->addChild(ptrLbSpin);
	LayoutUtil::layoutParentTop(ptrLbSpin,3,-18);
	m_ptrBtnLottery->setOpacity(0.5 * 255);
	m_bIsLotteryShowing = false;
}

void TableLayer::onLotteryBtnCallback(CCObject *pSender)
{
	SoundPlayer::play(SNDI_CLICK);
	GameLotteryLayer *ptrGameLotteryLayer = GameLotteryLayer::create();
	this->addChild(ptrGameLotteryLayer, Z_LOTTERY_LAYER);
	LayoutUtil::layoutParentLeftBottom(ptrGameLotteryLayer);
}

void TableLayer::setBtnLotteryVisible(bool isVisble)
{
	if (m_ptrBtnLottery != NULL)
	{
		m_ptrBtnLottery->setVisible(isVisble);
		m_ptrBtnLottery->setEnabled(isVisble);
	}
}

PublicAreaLayer* TableLayer::getPublicAreaLayer()
{
	return publicArea;
}

void TableLayer::initSpecialOfferCountDown()
{
	if (GameUtils::getSpecialOfferEndTime() - GameUtils::getCurrentTime() <= 0)
	{
		return;
	}

	m_ptrBtnSpeicalOffer = MenuItemImage::create("time_bg_small.png",
		"time_bg_small.png",this,menu_selector(TableLayer::onSpecialOfferBtnCallback));
	addChild(m_ptrBtnSpeicalOffer);
	LayoutUtil::layoutParentLeftBottom(m_ptrBtnSpeicalOffer,230,8);
	registerMenuItem(m_ptrBtnSpeicalOffer);
	if (m_ptrLbHint)
	{
		m_ptrBtnSpeicalOffer->setVisible(false);
	}

	m_ptrLbCountDown = CCLabelTTF::create("",FontConstant::TTF_IMPACT,28);
	m_ptrLbCountDown->setAnchorPoint(ccp(0,0.5));
	m_ptrBtnSpeicalOffer->addChild(m_ptrLbCountDown);
	LayoutUtil::layoutParentCenter(m_ptrLbCountDown,-66,4);

	schedule(schedule_selector(TableLayer::updateSpecialOfferCountDown),1.0f,kCCRepeatForever,0.1f);
}

void TableLayer::updateSpecialOfferCountDown(float dt)
{
	time_t timeLeft = GameUtils::getSpecialOfferEndTime() - GameUtils::getCurrentTime();
	if (timeLeft <= 0)
	{
		unschedule(schedule_selector(TableLayer::updateSpecialOfferCountDown));
		unRegisterMenuItem(m_ptrBtnSpeicalOffer);
		m_ptrBtnSpeicalOffer->removeFromParent();
		m_ptrBtnSpeicalOffer = NULL;
	}
	else
	{
		int hours = timeLeft / 3600;
		int minutes = (timeLeft - hours * 3600) / 60;
		int seconds = timeLeft - hours * 3600 - minutes * 60;
		m_ptrLbCountDown->setString(CCString::createWithFormat("%s:%02d:%02d",
			LocalizationManager::getInstance()->getLocalizedString("word_special"),minutes,seconds)->getCString());
	}
}

void TableLayer::initRankBtn()
{
	CCSprite* ptrNormalNode = CCSprite::create();
	ptrNormalNode->setCascadeOpacityEnabled(true);
	ptrNormalNode->setContentSize(CCSizeMake(70, 130));
	CCSprite* ptrNormal = CREATE_SPRITE(this, "sliding.png", false);
	ptrNormal->setCascadeOpacityEnabled(true);
	ptrNormalNode->addChild(ptrNormal);
	LayoutUtil::layoutParentLeft(ptrNormal);
	CCSprite* ptrArrow = CREATE_SPRITE(this, "arrow_up.png", false);
	ptrArrow->setScale(0.8f);
	ptrArrow->setCascadeOpacityEnabled(true);
	ptrArrow->setRotation(90);
	ptrNormal->addChild(ptrArrow);
	LayoutUtil::layoutParentCenter(ptrArrow, -3, 0);

	m_ptrBtnRank = ScaleMenuItemSprite::create(ptrNormalNode, ptrNormalNode, 
		this, menu_selector(TableLayer::onRankBtnCalback),1.0f,1.1f,255 * 0.5,255);
	addChild(m_ptrBtnRank, Z_BTNS);
	LayoutUtil::layoutParentLeft(m_ptrBtnRank,-10);
	registerMenuItem(m_ptrBtnRank);
}

void TableLayer::onRankBtnCalback(CCObject *pSender)
{
	GameRankLayer *ptrRankLayer = GameRankLayer::create();
	addChild(ptrRankLayer, Z_GAME_RANK_LAYER);
	LayoutUtil::layoutParentLeft(ptrRankLayer);
}

void TableLayer::setBtnRankVisible(bool isVisble)
{
	if (m_ptrBtnRank != NULL)
	{
		m_ptrBtnRank->setVisible(isVisble);
		m_ptrBtnRank->setEnabled(isVisble);
	}
}

void TableLayer::updateGameRankList( INT64 winAmount )
{
	// Update the Friend Rank List
	for(int i = 0; i < MainMenuLayer::s_friendInfosVec.size(); ++i)
	{
		if(GameUtils::s_myInfo.getAccountID() == MainMenuLayer::s_friendInfosVec.at(i).account_id)
		{
			MainMenuLayer::s_friendInfosVec.at(i).winCount += winAmount;
			sort(MainMenuLayer::s_friendInfosVec.begin(),MainMenuLayer::s_friendInfosVec.end(),greater<PokerPlayerRankInfo>());
			break;
		}
	}
	// Update the Global Rank List
	for(int i = 0; i < MainMenuLayer::s_globalInfosVec.size(); ++i)
	{
		if(GameUtils::s_myInfo.getAccountID() == MainMenuLayer::s_globalInfosVec.at(i).account_id)
		{
			MainMenuLayer::s_globalInfosVec.at(i).winCount += winAmount;
			sort(MainMenuLayer::s_globalInfosVec.begin(),MainMenuLayer::s_globalInfosVec.end(),greater<PokerPlayerRankInfo>());
			break;
		}
	}
}

void TableLayer::showWinChipsActions(vector<pair<int,vector<INT64> > > winnerChipsVec, 
									 vector<pair<int,vector<int> > > winnerCardsVec)
{
	m_tempWinnerChipsVec = winnerChipsVec;
	float delay = 2.0f;
	if (mySelf && mySelf->m_amIWin)
	{
		delay = 4.0f;
	}
	CCPoint sidePoolWorldPosition = publicArea->convertToWorldSpace(publicArea->getSidePool()->getPosition());
	CCPoint dest = convertToNodeSpace(sidePoolWorldPosition);

	float delta = 0.5f;
	for (int i = 0; i < m_tempWinnerChipsVec.size(); i++)
	{
		int cardsRank = CardsTypeUtils::getCardsRank(winnerCardsVec,winnerCardsVec[i].second);
		vector<INT64>& winAmountList = m_tempWinnerChipsVec[i].second;
		for (int j = 0; j < winAmountList.size(); j++)
		{
			CCNode* ptrChipWithBG = createChipAmountWithBG(winAmountList[j]);
			ptrChipWithBG->setUserData(&winAmountList[j]);
			addChild(ptrChipWithBG,Z_WIN_CHIPS_ACTION);
			ptrChipWithBG->setVisible(false);
			ptrChipWithBG->setPosition(dest);
			ptrChipWithBG->runAction(CCSequence::create(
				CCDelayTime::create(delay + j * delta + cardsRank * 1.0f),//cardsRank:后一名比前一名延迟1.0s
				CCCallFuncN::create(this,callfuncN_selector(TableLayer::updateSidePool)),
				CCShow::create(),
				CCMoveTo::create(1.0,players[m_tempWinnerChipsVec[i].first]->getPosition()),
				CCDelayTime::create(2.0f),
				CCCallFunc::create(ptrChipWithBG,callfunc_selector(CCNode::removeFromParent)),
				NULL));
		}
	}
}

void TableLayer::updateSidePool( CCNode* pSender )
{
	INT64 amount = *((INT64*)pSender->getUserData());
	if (publicArea->getSidePool() == NULL)
	{
		return;
	}
	

	//Test
	SidePool* ptrSidePool = publicArea->getSidePool();
	ptrSidePool->boundingBox();
	ptrSidePool->getBetChipAmount();
	ptrSidePool->getChildrenCount();
	ptrSidePool->getBetChipAmount();
	ptrSidePool->getAnchorPoint();
	//Test
	INT64 sidePoolChipAmount = publicArea->getSidePool()->getBetChipAmount();
	CCLOG("debug:updateSidePool:amount:%lld,sidePoolChipAmount:%lld",amount,sidePoolChipAmount);
	if (sidePoolChipAmount - amount > 0)
	{
		publicArea->getSidePool()->updatePool(sidePoolChipAmount - amount,false);
	}
	else
	{
		publicArea->getSidePool()->setVisible(false);
	}
}

CCNode* TableLayer::createChipAmountWithBG( INT64 chipAmount )
{
	string chipImgPath = "chip_100000.png";
	for (int i = 1; i < 17; i++)
	{
		if (chipAmount < Player::CHIP_NUM_FOR_IMAGES[i])
		{
			chipImgPath = CCString::createWithFormat("chip_%d.png",Player::CHIP_NUM_FOR_IMAGES[i - 1])->getCString();
			break;
		}
	}
	CCSprite* ptrChip = ResourceManager::getInstance()->createSprite(this,chipImgPath.c_str());
	CCLabelTTF* ptrLbChipAmount = CCLabelTTF::create(GameUtils::num2tableStr(chipAmount,blindBet,6).c_str(),FontConstant::TTF_IMPACT,20);
	CCSprite* ptrLabelBG = ResourceManager::getInstance()->createSprite(this,"bet_chip_bg.png");
	ptrLabelBG->addChild(ptrChip);
	ptrLabelBG->addChild(ptrLbChipAmount);
	LayoutUtil::layoutParentLeft(ptrChip);
	LayoutUtil::layoutParentCenter(ptrLbChipAmount,8,1);
	
	return ptrLabelBG;
}

void TableLayer::showPlayerInfoLayer( int index, CCPoint startPoint )
{
	if (players[index] == NULL)
	{
		return;
	}

	m_ptrPlayerInfoLayer = PlayerInfoLayer::create(this,players[index]->m_playerInfo,index);
	addChild(m_ptrPlayerInfoLayer, Z_PLAYER_INFO_LAYER);
	LayoutUtil::layoutParentCenter(m_ptrPlayerInfoLayer);
	m_ptrPlayerInfoLayer->animateToShow(startPoint);
}

string TableLayer::getLilyImgName( string imgName )
{
	if (s_tableType == TABLE_TYPE_NORMAL)
	{
		imgName = SelectRoomLayer::getRoomType(blindBet) + "_" + imgName;
	}
	else
	{
		imgName = "beginner_" + imgName;
	}

	return imgName;
}

void TableLayer::layoutAllPlayers()
{
	for (int i = 0; i < capacity; i++)
	{
		if (players[i])
		{
			layoutPlayer(i);
			players[i]->layoutChildren();
		}
	}
}

cocos2d::CCPoint TableLayer::getLilyPostion()
{
	return m_ptrBtnLily->getPosition();
}

void TableLayer::showShopLayer()
{
	ShopLayerEx* ptrShopLayer = ShopLayerEx::create();
	addChild(ptrShopLayer,Z_SHOP);
	LayoutUtil::layoutParentCenter(ptrShopLayer);
}

void TableLayer::doBanCmdNetworkCallback( CCObject* pSender )
{
	DoBanCmd * ptrDoBanCmd = (DoBanCmd*)pSender;
	int errorNO = ptrDoBanCmd->m_ptrOutput->errorInfo.errorNO;

	if(errorNO == RespHandler::OK)
	{
		CCNotificationCenter::sharedNotificationCenter()->removeObserver(this,
			ptrDoBanCmd->m_cmdName.c_str());
		CC_SAFE_DELETE(ptrDoBanCmd);
	}
}

void TableLayer::setBtnTalkVisible( bool val )
{
	if (m_ptrBtnTalk != NULL)
	{
		m_ptrBtnTalk->setVisible(val);
		m_ptrBtnTalk->setEnabled(val);
	}
}

void TableLayer::dealCards( float dealCardsTime )
{
	if (m_isRunning)
	{
		return;
	}
	
	CCSprite *ptrLilyDealCard = CREATE_SPRITE(this, 
		getLilyImgName("lily_deal_card.png").c_str(), false);
	m_ptrBtnLily->setNormalImage(ptrLilyDealCard);
	m_ptrBtnLily->setSelectedImage(ptrLilyDealCard);
	scheduleOnce(schedule_selector(TableLayer::resetLilyAfterDeal), dealCardsTime - 0.4f);

	int num = 0,dealerIndex = 0,order = 0;
	for (int i = 0; i < capacity; i++)
	{
		Player* player = getPlayer(i);
		if (player == NULL)
		{
			continue;
		}
		if (player->m_state != Player::GAME_WAIT)
		{
			num++;
		}
		if (player->m_isDealer)
		{
			dealerIndex = i;
			CCLOG("dealerIndex:%d",dealerIndex);
		}
	}
	for (int i = 0; i < capacity; i++)
	{
		Player* player = getPlayer(dealerIndex);
		if (player != NULL)
		{
			player->dealCard(true,mySelf == player,order,num);
			player->dealCard(false,mySelf == player,order,num);
			order++;
		}
		dealerIndex = (dealerIndex + 1) % capacity;
	}
}
