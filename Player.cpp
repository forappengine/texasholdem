#include "Player.h"
#include "LayoutUtil.h"
#include "AudioUtil.h"
#include "GameConstants.h"
#include "ResourceManager.h"
#include "sstream"
#include "SimpleAudioEngine.h"
#include "FontConstant.h"
#include "LogTag.h"
#include "TableLayer.h"
#include "Utils.h"
#include "GameUtils.h"
#include "UI/PlayerInfo/PlayerInfoLayer.h"
#include "PlayerInfo.h"
#include "Icon.h"
#include "Game/UI/GameRank/GameRankLayer.h"
#include "Game/UI/Talk/TalkLayer.h"
#include "BetControler.h"
#include "BetSlider.h"
#include "JSONCommand.h"
#include "SoundPlayer.h"
#include "CmdOutputs.h"
#include "LocalizationManager.h"
#include "CardsTypeUtils.h"
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
#include "AdController.h"
#endif
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
#include "JniUtils.h"
#endif

#define MODULE_LOG_TAG "Player.cpp"

const int Player::CHIP_NUM_FOR_IMAGES[17] = {1,2,5,10,20,25,50,100,250,500,1000,2000,5000,10000,20000,50000,100000};

Player::Player():
	m_baseImg0(NULL),
	m_baseImg1(NULL),
	m_baseImgIndex0(0),
	m_baseImgIndex1(0),
	m_ptrDealer(NULL),
	m_ptrGiftSprite(NULL),
	m_amIWin(false),
	m_ptrBlackCover(NULL),
	m_ptrWinActions(NULL),
	m_ptrWinLabel(NULL),
	m_ptrAllInEffect(NULL),
	m_ptrHandIndicatorBg(NULL),
	m_ptrHandIndicator(NULL),
	m_ptrLbHandCardsType(NULL),
	m_thinkTime(8)
{}

Player::~Player(){}

// on "init" you need to initialize your instance
bool Player::init( TableLayer* ptrTableLayer, int seat, const JSONNode& node )
{
	if (!Layer::init())
	{
		return false;
	}

	//initWithColor(ccc4(255,0,0,100));
	setContentSize(CCSizeMake(PAD_WIDTH, PAD_HEIGHT));

	m_ptrTableLayer = ptrTableLayer;
	m_seatPosition = seat;
	m_role = node["role"].as_int();
	m_isDealer = node["isDealer"].as_bool();
	m_state = node["state"].as_int();
	m_index = node["index"].as_int();
	stringstream sval(node["chipsInGame"].as_string());
	sval>>m_chipsInGame;
	m_playerInfo.setAccountID(node["id"].as_string());

	if(m_role == ROLE_BIG_BLIND)
	{
		m_betAmount = ptrTableLayer->blindBet;
	}
	else if(m_role == ROLE_SMALL_BLIND)
	{
		m_betAmount = ptrTableLayer->blindBet / 2;
	}
	else
	{
		m_betAmount = 0;
	}
	m_preBetAmount = 0;

	m_ptrWinBg = ResourceManager::getInstance()->createSprite(this,"player_frame_yellow.png");
	addChild(m_ptrWinBg,Z_WIN_BG);
	m_ptrWinBg->setVisible(false);

	m_ptrFailBg = ResourceManager::getInstance()->createSprite(this,"player_frame_black.png");
	addChild(m_ptrFailBg,Z_FAIL_BG);
	m_ptrFailBg->setVisible(false);

	m_ptrHalo=ResourceManager::getInstance()->createSprite(this,"halo.png");
	addChild (m_ptrHalo,Z_HALO);
	m_ptrHalo->setVisible(false);

	m_ptrNormalBg=ResourceManager::getInstance()->createSprite(this,"player_frame.png");
	addChild (m_ptrNormalBg,Z_NORMAL_BG);

	m_progressTimerBg = ResourceManager::getInstance()->createSprite(this,"timer_bg.png");
	addChild(m_progressTimerBg,Z_PROGRESS_TIMER_BG);

	m_progressTimer=CCProgressTimer::create(ResourceManager::getInstance()->createSprite(this, "timer.png"));
	m_progressTimer->setType(kCCProgressTimerTypeRadial);
	m_progressTimer->setReverseProgress(true);
	m_progressTimer->setPercentage(0);//100 for DEBUG
	addChild(m_progressTimer,Z_PROGRESS_TIMER);

	m_ptrPhoto = Icon::create(86);
	m_ptrPhoto->setAnchorPoint(ccp(0.5,0.5));
	m_ptrPhoto->setStencil(CREATE_SPRITE(this,"photo_mask_clip_game.png",false));
	m_ptrPhoto->setCover(CREATE_SPRITE(this,"photo_mask_game.png",false));
	string photoPath = GameUtils::LOCAL_PHOTO_PATH + "0.jpg";
	if (m_ptrTableLayer->isInGagSet(m_playerInfo.getAccountID()))
	{
		photoPath = "ban.png";
	}
	else
	{
		m_playerInfo.getPhotoPath(photoPath);
	}
	CCSprite* p_img = ResourceManager::getInstance()->createSprite(this,photoPath.c_str());
	m_ptrPhoto->setContent(p_img);
	addChild (m_ptrPhoto,Z_PHOTO);

	m_ptrBetLabelBG = ResourceManager::getInstance()->createSprite(this,"bet_chip_bg.png");
	m_ptrBetLabelBG->setVisible(false);
	addChild (m_ptrBetLabelBG,Z_BET_LABEL_BG);

	for (int i = 0; i < 17; i++)
	{
		m_ptrChipImages[i] = ResourceManager::getInstance()->createFrame(this,
			CCString::createWithFormat("chip_%d.png",CHIP_NUM_FOR_IMAGES[i])->getCString());
	}
	m_ptrChipImg = CCSprite::createWithSpriteFrame(m_ptrChipImages[0]);
	m_ptrBetLabelBG->addChild(m_ptrChipImg);
	LayoutUtil::layoutParentLeft(m_ptrChipImg);

	m_ptrLbBet = CCLabelTTF::create("", FontConstant::TTF_IMPACT, 20);
	m_ptrLbBet->setHorizontalAlignment(kCCTextAlignmentLeft);
	m_ptrLbBet->setVerticalAlignment(kCCVerticalTextAlignmentCenter);
	m_ptrBetLabelBG->addChild(m_ptrLbBet);
	LayoutUtil::layoutParentCenter(m_ptrLbBet,8,1);

	m_ptrLbState = CCLabelTTF::create("", FontConstant::TTF_IMPACT, 16);
	m_ptrLbState->setContentSize(CCSizeMake(m_ptrPhoto->getContentSize().width, m_ptrChipImg->getContentSize().height));
	m_ptrLbState->setHorizontalAlignment(kCCTextAlignmentCenter);
	m_ptrLbState->setVerticalAlignment(kCCVerticalTextAlignmentCenter);
	m_ptrLbState->setDimensions(CCSizeMake(90,m_ptrLbState->getContentSize().height));
	addChild(m_ptrLbState,Z_STATE_LABEL);

	CCSprite* ptrChipsInGameLabelBg = ResourceManager::getInstance()->createSprite(this,"bet_chip_bg.png");
	addChild(ptrChipsInGameLabelBg,Z_CHIPS_IN_GAME_BG);

	CCSprite* chipIcon = ResourceManager::getInstance()->createSprite(this,"assets_chip_icon.png");
	chipIcon->setScale(0.6f);
	ptrChipsInGameLabelBg->addChild(chipIcon);
	LayoutUtil::layoutParentLeft(chipIcon);

	m_ptrLbChipsInGame = CCLabelTTF::create("", FontConstant::TTF_IMPACT, 20);
	ptrChipsInGameLabelBg->addChild(m_ptrLbChipsInGame);
	LayoutUtil::layoutParentCenter(m_ptrLbChipsInGame,7,1);

	layoutChildren();
	initState();

	return true;
}

void Player::startThinking(float delayTime)
{
	m_progressTimer->setColor(ccc3(0,255,0));
	m_ptrHalo->setVisible(true);
	CCFiniteTimeAction *expire = CCCallFuncN::create(this, callfuncN_selector(Player::progressTimerDone));
	CCProgressFromTo *from2 = CCProgressFromTo::create(m_thinkTime, 100,0);
	CCTintTo* tintAction=CCTintTo::create(m_thinkTime,255,0,0);
	CCSpawn* actionSpawn = CCSpawn::create(tintAction,from2,NULL); 
	CCDelayTime* actionDelay = CCDelayTime::create(delayTime);
	
	CCAction* actSeq = CCSequence::create(actionDelay,actionSpawn,expire,NULL);
	if (m_ptrTableLayer->mySelf && m_ptrTableLayer->mySelf == this)
	{
		actionSpawn = CCSpawn::create(tintAction,from2,
			CCSequence::create(
				CCCallFunc::create(this,callfunc_selector(Player::playSoundStartThinking)),
				CCCallFunc::create(this,callfunc_selector(Player::vibrate)),
				CCDelayTime::create(m_thinkTime / 4.0 * 3),
				CCCallFunc::create(this,callfunc_selector(Player::playSoundTickTock)),
				CCCallFunc::create(this,callfunc_selector(Player::vibrate)),
				NULL),
			NULL); 
		actSeq = CCSequence::create(actionDelay,actionSpawn,expire,NULL);
	}
	m_progressTimer->runAction(actSeq);
}

void Player::stopThinking(bool calledByServer)
{
	m_ptrHalo->setVisible(false);
	m_progressTimer->stopAllActions();
	m_progressTimer->setPercentage(0);

	if (calledByServer)
	{
		return;
	}

	TableLayer* ptrTableLayer = (TableLayer*)getParent();
	BetControler* ptrBetCtrl = ptrTableLayer->betCtrl;
	if (ptrBetCtrl->actionID == DECISION_RAISE)
	{
		INT64 minRaise = ptrTableLayer->maxBet + ptrTableLayer->blindBet;
		ptrBetCtrl->actionID = Player::DECISION_RAISE;
		ptrBetCtrl->raiseData = minRaise;
		JSONCommand* lcmd = JSONCommand::createJSONCMD(JSONCommand::CMD_U_COMMIT);
		GameScreen* gameScreen = (GameScreen*)ptrTableLayer->getParent();
		char* msg = lcmd->encode(gameScreen);
		gameScreen->gameSocket->sendMsg(msg);
		delete lcmd;
	}
}

void Player::progressTimerDone(CCNode* node)
{
	if (m_ptrTableLayer->mySelf && m_ptrTableLayer->mySelf == this)
	{
		stopThinking(false);
	}
}

void Player::addBaseCards(const JSONNode& cardArray, bool isGameRunning)
{
	if(m_state == GAME_WAIT || m_state == GAME_FOLDED)
	{
		return;// no base card for wait and folded player
	}
	CCLOG("Player::addBaseCards");

	if(cardArray.size() > 0)
	{
		m_baseImgIndex0 = cardArray[0].as_int();
		m_baseImgIndex1 = cardArray[1].as_int();
	}
	else
	{
		m_baseImgIndex0 = 0;
		m_baseImgIndex1 = 0;
	}

	if (m_baseImgIndex0 == 0 && m_baseImgIndex1 == 0)
	{
		m_baseImg0 = ResourceManager::getInstance()->createSprite(this, "card_00.png");
		m_baseImg0->setScale(0.5);
		m_baseImg1 = ResourceManager::getInstance()->createSprite(this, "card_00.png");
		m_baseImg1->setScale(0.5);

		if(m_isSeatRight)
		{
			LayoutUtil::layoutLeftBottom(m_baseImg0,m_ptrPhoto,-6,-4);
		}
		else
		{
			LayoutUtil::layoutRightBottom(m_baseImg0,m_ptrPhoto,-6,-4);
		}
		LayoutUtil::layoutCenter(m_baseImg1,m_baseImg0,12,-3);
	}
	else
	{
		char img[128];
		sprintf(img, "card_%02d.png", m_baseImgIndex0);
		m_baseImg0=ResourceManager::getInstance()->createSprite(this, img);
		m_baseImg0->setScale(0.7f);
		sprintf(img, "card_%02d.png", m_baseImgIndex1);
		m_baseImg1=ResourceManager::getInstance()->createSprite(this, img);
		m_baseImg1->setScale(0.7f);

		LayoutUtil::layoutRight(m_baseImg0,m_ptrPhoto,-12,-4);
		LayoutUtil::layoutCenter(m_baseImg1,m_baseImg0,19,-6);
	}

	m_baseImg0->setRotation(-15);
	m_baseImg1->setRotation(15);
	addChild(m_baseImg0,Z_BASE_IMG_0);
	addChild(m_baseImg1,Z_BASE_IMG_1);

	if(!isGameRunning)
	{
		m_baseImg0->setVisible(false);
		m_baseImg1->setVisible(false);
	}
}

void Player::flipCardsInHands( const JSONNode& cardArray )
{
	if(m_state == GAME_WAIT || m_state == GAME_FOLDED
		|| m_baseImg0 == NULL || m_baseImg1 == NULL)
	{
		return;// no base card for wait and folded player
	}
	removeHandIndicator(true);
	CCLOG("Player::flipCardsInHands");
	CCPoint photoPos = m_ptrPhoto->getPosition();
	if (m_baseImgIndex0 == 0 && m_baseImgIndex1 == 0)
	{
		if(cardArray.size() > 0)
		{
			m_baseImgIndex0 = cardArray[0].as_int();
			m_baseImgIndex1 = cardArray[1].as_int();
		}
		CCSprite *tempBaseImg_0 = m_baseImg0;
		CCSprite *tempBaseImg_1 = m_baseImg1;
		m_baseImg0 = ResourceManager::getInstance()->createSprite(this, 
			CCString::createWithFormat("card_%02d.png",m_baseImgIndex0)->getCString());
		addChild(m_baseImg0,Z_BASE_IMG_0);
		m_baseImg0->setScale(0.8f);
		m_baseImg0->setPosition(ccpAdd(photoPos,ccp(-9,-12)));
		m_baseImg0->setRotation(-15);
		m_baseImg0->setVisible(false);

		tempBaseImg_0->runAction(CCSequence::create(
			CCSpawn::create(CCMoveTo::create(0.3f,m_baseImg0->getPosition()),
			CCScaleTo::create(0.3f,0.8),NULL),
			CCOrbitCamera::create(0.2f,1,0,0,90,0,0),
			CCCallFunc::create(tempBaseImg_0, callfunc_selector(CCNode::removeFromParent)),
			NULL));
		CCAction* ptrAction = CCSequence::create(
			CCDelayTime::create(0.5f),
			CCShow::create(),
			CCOrbitCamera::create(0.2f,1,0,270,90,0,0),
			NULL);
		m_baseImg0->runAction(ptrAction);

		m_baseImg1 = ResourceManager::getInstance()->createSprite(this, 
			CCString::createWithFormat("card_%02d.png",m_baseImgIndex1)->getCString());
		addChild(m_baseImg1,Z_BASE_IMG_1);
		m_baseImg1->setScale(0.8f);
		m_baseImg1->setPosition(ccpAdd(photoPos,ccp(9,-18)));
		m_baseImg1->setRotation(15);
		m_baseImg1->setVisible(false);

		tempBaseImg_1->runAction(CCSequence::create(
			CCSpawn::create(CCMoveTo::create(0.3f,m_baseImg1->getPosition()),
			CCScaleTo::create(0.3f,0.8),NULL),
			CCOrbitCamera::create(0.2f,1,0,0,90,0,0),
			CCCallFunc::create(tempBaseImg_1, callfunc_selector(CCNode::removeFromParent)),
			NULL));
		m_baseImg1->runAction((CCAction*)ptrAction->copy());
	}
	else
	{
		m_baseImg0->runAction(CCSpawn::create(CCMoveTo::create(0.3f,ccpAdd(photoPos,ccp(-9,-12))),
			CCScaleTo::create(0.3f,0.8),NULL));
		m_baseImg1->runAction(CCSpawn::create(CCMoveTo::create(0.3f,ccpAdd(photoPos,ccp(9,-18))),
			CCScaleTo::create(0.3f,0.8),NULL));
	}
}

void Player::updateState(const JSONNode& node)
{
	m_decision=node["decision"].as_int();
	stringstream bval(node["betamount"].as_string());
	bval>>m_betAmount;
	stringstream sval(node["chipsInGame"].as_string());
	sval>>m_chipsInGame;

	switch(m_decision)
	{
	case DECISION_CHECK:
		SoundPlayer::play(SNDI_PASS);
		m_ptrLbState->setString(LOCALIZED_STRING("word_check"));
		m_state = GAME_PLAYING;
		break;
	case DECISION_CALL:
		playSoundBet(m_betAmount - m_preBetAmount);
		m_ptrLbState->setString(LOCALIZED_STRING("word_call"));
		m_state = GAME_PLAYING;
		break;
	case DECISION_RAISE:
		playSoundBet(m_betAmount - m_preBetAmount);
		m_ptrLbState->setString(LOCALIZED_STRING("word_raise"));
		m_state = GAME_PLAYING;
		break;
	case DECISION_ALLIN:
		SoundPlayer::play(SNDI_ALL_IN);
		m_ptrLbState->setString("");
		m_state = GAME_ALLIN;
		showAllIn();
		break;
	case DECISION_FOLD:
		//SoundPlayer::play(SNDI_TIME_UP);
		SoundPlayer::play(SNDI_FOLD);
		m_ptrLbState->setString(LOCALIZED_STRING("word_fold"));
		m_state = GAME_FOLDED;
		if (m_baseImg0 && m_baseImg1 && m_baseImgIndex0 != 0 && m_baseImgIndex1 != 0)
		{
			GameUtils::setMask(m_baseImg0,0,0,0,255*0.5);
			GameUtils::setMask(m_baseImg1,0,0,0,255*0.5);
		}
		m_amIWin = false;
		showFail(true);
		break;
	default:
		m_ptrLbState->setString(LOCALIZED_STRING("word_unknown"));//debug use
	}

	if(m_decision == DECISION_CALL || m_decision == DECISION_RAISE || m_decision == DECISION_ALLIN)
	{
		showBetAction(m_betAmount - m_preBetAmount);
		m_preBetAmount = m_betAmount;
		m_ptrLbBet->setString(GameUtils::num2tableStr(m_betAmount,m_ptrTableLayer->blindBet,6).c_str());
		m_ptrBetLabelBG->setVisible(true);
		updateChipImg(m_betAmount);
		m_ptrLbChipsInGame->setString(GameUtils::num2tableStr(m_chipsInGame,m_ptrTableLayer->blindBet,6).c_str());
	}
}

void Player::clearState()
{
	m_ptrBetLabelBG->setVisible(false);
	if(m_state == GAME_WAIT || m_state == GAME_FOLDED || m_state == GAME_ALLIN 
		|| m_decision == DECISION_FOLD || m_decision == DECISION_ALLIN)
	{
		return;
	}
	CCLOG("Player::clearState():seatPosition:%d",m_seatPosition);
	m_ptrLbState->setString("");
	m_ptrLbBet->setString("");
	m_betAmount = 0;
	m_preBetAmount = 0;
	m_ptrLbChipsInGame->setString(GameUtils::num2tableStr(m_chipsInGame,m_ptrTableLayer->blindBet,6).c_str());
}

void Player::initState()
{
	m_ptrLbState->setColor(ccc3(0,0,0));
	if(m_state == GAME_WAIT)
	{
		m_ptrLbState->setString(LOCALIZED_STRING("word_wait"));
		showFail(false);
	}
	else if (m_state == GAME_FOLDED)
	{
		m_ptrLbState->setString(LOCALIZED_STRING("word_fold"));
		//GameUtils::setMask(baseImg_0,0,0,0,255*0.5);
		//GameUtils::setMask(baseImg_1,0,0,0,255*0.5);
		removeBaseCards();
		showFail(false);
	}
	else if (m_state == GAME_ALLIN)
	{
		m_ptrLbState->setString(LOCALIZED_STRING("word_allin"));
		showAllIn();
	}
	else
	{
		m_state = GAME_PLAYING;
		if(m_role == ROLE_SMALL_BLIND)
		{
			m_ptrBetLabelBG->setVisible(true);
			updateChipImg(m_ptrTableLayer->blindBet / 2);
			m_ptrLbBet->setString(GameUtils::num2tableStr(m_ptrTableLayer->blindBet / 2,m_ptrTableLayer->blindBet,6).c_str());
			m_ptrLbState->setString(LOCALIZED_STRING("word_smallblind"));
		}
		else if(m_role == ROLE_BIG_BLIND)
		{
			m_ptrBetLabelBG->setVisible(true);
			updateChipImg(m_ptrTableLayer->blindBet);
			m_ptrLbBet->setString(GameUtils::num2tableStr(m_ptrTableLayer->blindBet,m_ptrTableLayer->blindBet,6).c_str());
			m_ptrLbState->setString(LOCALIZED_STRING("word_bigblind"));
		}
		else
		{
			m_ptrLbState->setString(GameUtils::cutName(m_playerInfo.getNickName(),12).c_str());
		}
		if(m_isDealer)
		{
			m_ptrDealer = ResourceManager::getInstance()->createSprite(this,"dealer_token.png");
			float f_img_w = m_ptrNormalBg->getContentSize().width;
			float f_img_h = m_ptrNormalBg->getContentSize().height;
			float d_img_w = m_ptrDealer->getContentSize().width;
			float d_img_h = m_ptrDealer->getContentSize().height;

			m_ptrDealer->setPosition(CCPointMake(m_ptrNormalBg->getPositionX()-f_img_w / 2 + d_img_w / 2,
				m_ptrNormalBg->getPositionY() + f_img_h / 2 - d_img_h / 2));
			addChild(m_ptrDealer,Z_DEALER);
		}
	}

	m_ptrLbChipsInGame->setString(GameUtils::num2tableStr(m_chipsInGame,m_ptrTableLayer->blindBet,6).c_str());
	m_amIWin = false;
}

void Player::onExit()
{
	Layer::onExit();
	stopAllActions();
	if (m_ptrGiftSprite != NULL)
	{
		m_ptrGiftSprite->removeFromParent();
		m_ptrGiftSprite = NULL;
	}
}

bool Player::ccTouchBegan(cocos2d::CCTouch* pTouch, cocos2d::CCEvent* pEvent) 
{	
	if(Layer::ccTouchBegan(pTouch, pEvent) && 
		m_ptrNormalBg->boundingBox().containsPoint(convertToNodeSpace(pTouch->getLocation())))
	{
		SoundPlayer::play(SNDI_CLICK);
		m_ptrTableLayer->showPlayerInfoLayer(m_index,pTouch->getLocation());
		return true;
	}
	return false;
}

void Player::showBetAction( INT64 currentBetAmount )
{
	CCLOG("showBetAction");
	if(currentBetAmount <= 0)
	{
		return;
	}

	int i = 16;
	vector<CCSprite*> chips;
	while (currentBetAmount > 0 && i >= 0 && chips.size() <= 10)//the max count is 10
	{	
		if (currentBetAmount >= CHIP_NUM_FOR_IMAGES[i])
		{
			CCSprite* ptrTemp = CCSprite::createWithSpriteFrame(m_ptrChipImages[i]);
			chips.push_back(ptrTemp);
			currentBetAmount -= CHIP_NUM_FOR_IMAGES[i];
			if(currentBetAmount < CHIP_NUM_FOR_IMAGES[i])
			{
				i--;
			}
		}	
		else
		{
			i--;
		}
	}
	float r = 30.0f;
	for (i = 0; i < chips.size(); i++)
	{
		CCSprite* ptrTemp = chips.at(i);
		addChild(ptrTemp,Z_BET_ACTION);
		float offsetX = 0.0f,offsetY = 0.0f;
		if(chips.size() > 1)
		{
			offsetX = r * CCRANDOM_0_1() * (CCRANDOM_0_1() > 0.5 ? 1 : -1);
			offsetY = r * CCRANDOM_0_1() * (CCRANDOM_0_1() > 0.5 ? 1 : -1);
		}		
		ptrTemp->setPosition(ccp(m_ptrLbChipsInGame->getParent()->getPosition().x - 35 + offsetX,
			m_ptrLbChipsInGame->getParent()->getPosition().y + offsetY));
		CCPoint dest = ccp(m_ptrBetLabelBG->getPosition().x - 34, m_ptrBetLabelBG->getPosition().y);
		ptrTemp->runAction(CCSequence::create(
			CCMoveTo::create(0.6f,dest),
			CCCallFunc::create(ptrTemp, callfunc_selector(CCNode::removeFromParent)),
			NULL));
	}
}

CCSprite* Player::getBetLabelBG()
{
	return m_ptrBetLabelBG;
}

void Player::dealCard(bool isFirstOne, bool needFlipCard, int order, int all)
{
	if(m_state == GAME_WAIT || m_baseImg0 == NULL || m_baseImg1 == NULL || getParent() == NULL)
	{
		return;// no base card for wait player
	}
	CCLOG("Player::dealCard:isFirstOne:%d,needFlipCard:%d,order:%d,all:%d",isFirstOne,needFlipCard,order,all);

	CCSprite* dealingCard = ResourceManager::getInstance()->createSprite(this,"card_00.png");
	dealingCard->setScale(0.1f);
	dealingCard->setVisible(false);
	getParent()->addChild(dealingCard,TableLayer::Z_BTN_LILY + 1);
	dealingCard->setPosition(ccpAdd(m_ptrTableLayer->getLilyPostion(),ccp(0,-60)));
	CCSprite* destBaseImg = NULL;
	float delta = 0.15f;//相邻两个玩家发牌的间隔
	float delay = order * delta;
	if (isFirstOne)
	{	
		dealingCard->setRotation(-15);
		destBaseImg = m_baseImg0;
	}
	else
	{
		dealingCard->setRotation(15);
		destBaseImg = m_baseImg1;
		delay += (delta * all);
	}
	CCPoint endPoint = getParent()->convertToNodeSpace(convertToWorldSpace(destBaseImg->getPosition()));
	if (needFlipCard)
	{
		dealingCard->runAction(CCSpawn::create(
			CCSequence::create(
				CCDelayTime::create(delay),
				CCCallFunc::create(this,callfunc_selector(Player::playSoundDealCard)),NULL),
			CCSequence::create(
				CCDelayTime::create(delay),
				CCShow::create(),
				CCSpawn::create(CCMoveTo::create(0.4f,endPoint),CCScaleTo::create(0.4f,0.5),NULL),
				CCOrbitCamera::create(0.2f,1,0,0,90,0,0),
				CCCallFunc::create(dealingCard, callfunc_selector(CCNode::removeFromParent)),
				NULL),
			NULL));
		if (isFirstOne)
		{
			destBaseImg->runAction(CCSequence::create(
				CCDelayTime::create(0.55f + delay),
				CCShow::create(),
				CCOrbitCamera::create(0.2f,1,0,270,90,0,0),
				NULL));
		}
		else
		{
			destBaseImg->runAction(CCSequence::create(
				CCDelayTime::create(0.55f + delay),
				CCShow::create(),
				CCOrbitCamera::create(0.2f,1,0,270,90,0,0),
				CCCallFunc::create(this,callfunc_selector(Player::updateHandIndicator)),
				NULL));
		}
	}
	else
	{
		dealingCard->runAction(CCSpawn::create(
			CCSequence::create(
				CCDelayTime::create(delay),
				CCCallFunc::create(this,callfunc_selector(Player::playSoundDealCard)),NULL),
			CCSequence::create(
				CCDelayTime::create(delay),
				CCShow::create(),
				CCSpawn::create(CCMoveTo::create(0.4f,endPoint),CCScaleTo::create(0.4f,0.5),NULL),
				CCCallFunc::create(dealingCard, callfunc_selector(CCNode::removeFromParent)),
				NULL),
			NULL));
		destBaseImg->runAction(CCSequence::create(
			CCDelayTime::create(0.4f + delay),
			CCShow::create(),
			NULL));
	}
}

int Player::getRole()
{
	return m_role;
}

void Player::highlightWinCards(CCNode* node,void *ptrVoid)
{
	if(m_state == GAME_WAIT)
	{
		return;// no base card for wait player
	}

	set<int> *winCards = (set<int> *)ptrVoid;
	CCLOG("Player::highlightWinCards");
	set<int>::iterator itr = winCards->find(m_baseImgIndex0);
	if(itr == winCards->end())
	{
		GameUtils::setMask(m_baseImg0,0,0,0,255*0.5);
	}
	else
	{
		winCards->erase(itr);
		reorderChild(m_baseImg0,Z_BASE_IMG_1);
	}

	itr = winCards->find(m_baseImgIndex1);
	if(itr == winCards->end())
	{
		GameUtils::setMask(m_baseImg1,0,0,0,255*0.5);
	}
	else
	{
		winCards->erase(itr);
		reorderChild(m_baseImg1,Z_BASE_IMG_1 + 1);
	}
	m_baseImgIndex0 = 0;
	m_baseImgIndex1 = 0;

	if (m_amIWin)
	{
		showWin();	
	}
	else
	{
		showFail(false);
	}
}

void Player::talk(string content)
{
	CCSprite* ptrPop = CREATE_SPRITE(this, "pop.png", false);
	addChild(ptrPop, Z_POP);
	CCPoint endPoint = ccpAdd(m_ptrPhoto->getPosition(),ccp(20,60));
	CCPoint beginPoint = ccpAdd(endPoint,ccp(-47,-42));
	ptrPop->setPosition(beginPoint);
	
	CCLabelTTF* ptrLbContent = CCLabelTTF::create(content.c_str(), FontConstant::TTF_IMPACT, 20);
	ptrLbContent->setColor(ccc3(0, 0, 0));
	ptrLbContent->setHorizontalAlignment(kCCTextAlignmentLeft);
	ptrLbContent->setAnchorPoint(ccp(0.5,0.5));
	ptrPop->addChild(ptrLbContent);
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
	ptrLbContent->setDimensions(CCSizeMake(122,70));
	LayoutUtil::layoutParentTop(ptrLbContent,0,-12);
#else
	ptrLbContent->setDimensions(CCSizeMake(122,55));
	LayoutUtil::layoutParentTop(ptrLbContent,0,-15);
#endif

	ptrPop->setScale(0.1f);
	CCAction* ptrAct = CCSequence::create(
		CCSpawn::create(CCScaleTo::create(0.2f, 1.0f),CCMoveTo::create(0.2f,endPoint),NULL),
		CCDelayTime::create(2.0f),
		CCSpawn::create(CCScaleTo::create(0.2f, 0.1f),CCMoveTo::create(0.2f,beginPoint),NULL),
		CCCallFunc::create(ptrPop,callfunc_selector(CCNode::removeFromParent)),
		NULL);
	ptrPop->runAction(ptrAct);
}

void Player::updateChipImg(INT64 amount)
{
	int i;
	for (i = 1; i < 17; i++)
	{
		if (amount <= CHIP_NUM_FOR_IMAGES[i])
		{
			m_ptrChipImg->setDisplayFrame(m_ptrChipImages[i - 1]);
			break;
		}
	}
	if(i == 17)
	{
		m_ptrChipImg->setDisplayFrame(m_ptrChipImages[i - 1]);
	}
}

void Player::refreshPhoto()
{
	CCLOG("Player::refreshPhoto:id:%s",m_playerInfo.getAccountID().c_str());
	string photoPath;
	if (m_ptrTableLayer->isInGagSet(m_playerInfo.getAccountID()))
	{
		photoPath = "ban.png";
	}
	else
	{
		m_playerInfo.getPhotoPath(photoPath);
	}
	CCSprite* p_img = ResourceManager::getInstance()->createSprite(this,photoPath.c_str());
	m_ptrPhoto->setContent(p_img);
}

void Player::removeBaseCards()
{
	if(m_baseImg0 && m_baseImg1)
	{
		CCLOG("Player::removeBaseCards:seatPostion:%d",m_seatPosition);
		m_baseImg0->removeFromParent();
		m_baseImg0 = NULL;
		m_baseImgIndex0 = 0;
		m_baseImg1->removeFromParent();
		m_baseImg1 = NULL;
		m_baseImgIndex1 = 0;
	}
}

void Player::setDecision(int decision)
{
	m_decision = decision;
}

void Player::setThinkTime(int thinkTime)
{
	if (m_thinkTime != thinkTime && thinkTime > 0)
	{
		m_thinkTime = thinkTime;
	}
}

void Player::reInitState(const JSONNode& node)
{
	CCLOG("Player::reInitState:index:%d,seatPosition:%d",m_index,m_seatPosition);
	m_state = GAME_PLAYING;
	m_decision = DECISION_NONE;
	if (m_ptrDealer != NULL)
	{
		m_ptrDealer->removeFromParent();
		m_ptrDealer = NULL;
	}
	stringstream chips(node["chipsInGame"].as_string());
	chips>>m_chipsInGame;
	m_role = node["role"].as_int();
	m_isDealer = node["isDealer"].as_bool();
	clearState();
	m_state = node["state"].as_int();
	if (m_role == ROLE_SMALL_BLIND)
	{
		m_betAmount = m_ptrTableLayer->blindBet/2;
	}
	else if(m_role == ROLE_BIG_BLIND)
	{
		m_betAmount = m_ptrTableLayer->blindBet;
	}
	removeBaseCards();
	setScale(1.0f);
	removeBlackCover();
	m_ptrWinBg->setVisible(false);
	m_ptrFailBg->setVisible(false);
	m_ptrNormalBg->setVisible(true);
	m_progressTimerBg->setVisible(true);
	removeWinActions();
	removeAllInEffect();
	if (m_ptrWinLabel)
	{
		m_ptrWinLabel->removeFromParent();
		m_ptrWinLabel = NULL;
	}
	removeHandIndicator(true);
	initState();
}

void Player::showSendGiftActions(Player* pSender,string giftCode)
{
	CCLOG("Player::showSendGiftActions:%d to %d",pSender->m_index,m_index);
	CCPoint beginPosition = pSender->getPosition();
	CCPoint endPosition = m_ptrPhoto->getPosition();
	endPosition.x += m_isSeatRight ? 45 : -45;
	CCPoint tempPos = endPosition;
	endPosition = getParent()->convertToNodeSpace(convertToWorldSpace(endPosition));
	string prefix = "gift_" + giftCode.substr(0,2);
	set<string> fileNames = GameUtils::getFileNamesByPrefix(GameUtils::WRITABLE_GIFTS_PATH,prefix,true);
	if (fileNames.size() == 0)
	{
		return;
	}
	string filePath = *fileNames.begin();
	if (m_ptrGiftSprite != NULL)
	{
		m_ptrGiftSprite->removeFromParent();
		m_ptrGiftSprite = NULL;
	}
	m_ptrGiftSprite = ResourceManager::getInstance()->createSprite(this,filePath.c_str());
	m_ptrGiftSprite->setVisible(false);
	CCSprite* tempGiftSprite = ResourceManager::getInstance()->createSprite(this,filePath.c_str());
	if (m_ptrGiftSprite == NULL)
	{
		return;
	}
	SoundPlayer::play(SNDI_RECEIVE_GIFT);
	getParent()->addChild(tempGiftSprite,TableLayer::Z_GIFT);
	tempGiftSprite->setPosition(beginPosition);
	tempGiftSprite->runAction(CCSequence::create(
		CCMoveTo::create(1.0f,endPosition),
		CCCallFunc::create(tempGiftSprite,callfunc_selector(CCNode::removeFromParent)),
		NULL));
	addChild(m_ptrGiftSprite,Z_GIFT);
	m_ptrGiftSprite->setPosition(tempPos);
	m_ptrGiftSprite->runAction(CCSequence::create(
		CCDelayTime::create(1.0f),
		CCShow::create(),NULL));
}

void Player::showWinTwinkleParticle()
{
	CCParticleSystemQuad* ptrTwinkleParticle = CCParticleSystemQuad::create("Particle/twinkle.plist");
	ptrTwinkleParticle->setVisible(false);
	addChild(ptrTwinkleParticle,Z_TWINKLE_PARTICLE);
	ptrTwinkleParticle->setPosition(m_ptrNormalBg->getPosition());
	ptrTwinkleParticle->runAction(CCSequence::create(
		CCDelayTime::create(1.0f),
		CCShow::create(),
		CCDelayTime::create(ptrTwinkleParticle->getDuration() + 1.0f),
		CCCallFunc::create(ptrTwinkleParticle,callfunc_selector(CCNode::removeFromParent)),
		NULL));
}

void Player::showFail(bool throwCards)
{
	setScale(0.8f);
	if (m_baseImg0 && m_baseImg1 && throwCards && m_baseImgIndex0 == 0 && m_baseImgIndex1 == 0)
	{
		CCPoint dest = convertToNodeSpace(ccp(515,460));
		m_baseImg0->runAction(CCSpawn::create(CCMoveTo::create(0.4f,dest),CCScaleTo::create(0.4f,0.2),NULL));
		m_baseImg1->runAction(CCSequence::create(
			CCSpawn::create(CCMoveTo::create(0.4f,dest),CCScaleTo::create(0.4f,0.2),NULL),
			CCCallFunc::create(this,callfunc_selector(Player::removeBaseCards)),NULL));
	}
	m_ptrFailBg->setVisible(true);
	m_ptrNormalBg->setVisible(false);
	removeBlackCover();
	m_ptrBlackCover = ResourceManager::getInstance()->createSprite(this,"player_frame_black.png");
	addChild(m_ptrBlackCover,Z_BLACK_COVER);
	m_ptrBlackCover->setPosition(m_ptrNormalBg->getPosition());
	m_ptrLbState->setColor(ccc3(0,203,181));
	removeAllInEffect();
}

void Player::showWinActions()
{
	removeWinActions();

	m_ptrWinActions = CCNode::create();
	for (int i = 0; i < 4; i++)
	{
		CCSprite* ptrFrame = ResourceManager::getInstance()->createSprite(this,
			CCString::createWithFormat("win_frame_%d.png",i)->getCString());
		if (i == 3)
		{
			ptrFrame->runAction(CCRepeatForever::create(CCSequence::create(
				CCFadeOut::create(0.5f),
				CCDelayTime::create(1.0f),
				CCFadeIn::create(0.5f),
				NULL)));
		}
		else
		{
			float delay = i * 0.5f;
			ptrFrame->setOpacity(0);
			ptrFrame->runAction(CCRepeatForever::create(CCSequence::create(
				CCDelayTime::create(delay),
				CCFadeIn::create(0.5f),
				CCFadeOut::create(0.5f),
				CCDelayTime::create(1.0 - delay),
				NULL)));
		}	
		m_ptrWinActions->setContentSize(ptrFrame->getContentSize());
		m_ptrWinActions->addChild(ptrFrame);
		LayoutUtil::layoutParentCenter(ptrFrame);
	}
	m_ptrWinActions->setAnchorPoint(ccp(0.5,0.5));
	addChild(m_ptrWinActions,Z_WIN_ACTIONS);
	m_ptrWinActions->setPosition(m_ptrNormalBg->getPosition());

	m_ptrWinLabel = ResourceManager::getInstance()->createSprite(this,"win_label.png");
	addChild(m_ptrWinLabel,Z_WIN_LABEL);
	LayoutUtil::layoutBottom(m_ptrWinLabel,m_ptrPhoto,0,20);
}

void Player::showWin()
{
	SoundPlayer::play(SNDI_I_AM_WINNER);
	removeHandIndicator(true);
	m_progressTimerBg->setVisible(false);
	m_ptrWinBg->setVisible(true);
	m_ptrNormalBg->setVisible(false);
	showWinActions();
	removeAllInEffect();
}

void Player::showAllIn()
{
	removeAllInEffect();
	m_ptrAllInEffect = ResourceManager::getInstance()->createSprite(this,"text_all_in.png");
	m_ptrAllInEffect->setScale(0.1f);
	CCSprite* ptrLight = ResourceManager::getInstance()->createSprite(this,"text_all_in_light.png");
	ptrLight->runAction(CCRepeatForever::create(
		CCSequence::create(CCFadeOut::create(1.0f),CCFadeIn::create(1.0f),NULL)));
	m_ptrAllInEffect->addChild(ptrLight);
	LayoutUtil::layoutParentCenter(ptrLight);

	CCParticleSystemQuad* ptrAllInParticle = CCParticleSystemQuad::create("Particle/all_in.plist");
	m_ptrAllInEffect->addChild(ptrAllInParticle);
	LayoutUtil::layoutParentCenter(ptrAllInParticle);	

	addChild(m_ptrAllInEffect,Z_ALL_IN_EFFECT);
	m_ptrAllInEffect->setPosition(ccp(m_ptrPhoto->getPosition().x,m_ptrPhoto->getPosition().y - 50));
	m_ptrAllInEffect->runAction(CCScaleTo::create(0.2f,1.0f));
}

void Player::vibrate()
{
	if (GameUtils::isVibrateOn())
	{
		CCLOG("vibrate");
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
		JniUtils::vibrate(500);
#endif
	}
}

void Player::playSoundStartThinking()
{
	SoundPlayer::play(SNDI_START_THINKING);
}

void Player::playSoundTickTock()
{
	SoundPlayer::play(SNDI_TICK_TOCK);
}

void Player::playSoundDealCard()
{
	TableLayer* ptrTableLayer = (TableLayer*)getParent();
	if (ptrTableLayer == NULL)
	{
		return;
	}
	if (ptrTableLayer->getCapacity() == 5)
	{
		if (m_seatPosition == 0)
		{
			SoundPlayer::play(SNDI_DEAL_CARD_2);
		}
		else if (m_seatPosition == 1 || m_seatPosition == 4)
		{
			SoundPlayer::play(SNDI_DEAL_CARD_4);
		}
		else//seatPosition == 2 || seatPosition == 3
		{
			SoundPlayer::play(SNDI_DEAL_CARD_0);
		}
	}
	else
	{
		if (m_seatPosition == 0)
		{
			SoundPlayer::play(SNDI_DEAL_CARD_2);
		}
		else if (m_seatPosition == 2 || m_seatPosition == 7)
		{
			SoundPlayer::play(SNDI_DEAL_CARD_4);
		}
		else if (m_seatPosition == 1 || m_seatPosition == 8)
		{
			SoundPlayer::play(SNDI_DEAL_CARD_3);
		}
		else if (m_seatPosition == 3 || m_seatPosition == 6)
		{
			SoundPlayer::play(SNDI_DEAL_CARD_1);
		}
		else//seatPosition == 4 || seatPosition == 5
		{
			SoundPlayer::play(SNDI_DEAL_CARD_0);
		}
	}
}

void Player::playSoundBet( INT64 betChipAmount )
{
	if (betChipAmount  >= 90 * m_ptrTableLayer->blindBet)
	{
		SoundPlayer::play(SNDI_CHIPS_ON_TABLE_BIG);
	}
	else
	{
		SoundPlayer::play(SNDI_CHIPS_ON_TABLE_SMALL);
	}
}

void Player::removeAllInEffect()
{
	if (m_ptrAllInEffect)
	{
		m_ptrAllInEffect->removeFromParent();
		m_ptrAllInEffect = NULL;
	}
}

void Player::removeBlackCover()
{
	if (m_ptrBlackCover)
	{
		m_ptrBlackCover->removeFromParent();
		m_ptrBlackCover = NULL;
	}
}

void Player::removeWinActions()
{
	if (m_ptrWinActions)
	{
		m_ptrWinActions->removeFromParent();
		m_ptrWinActions = NULL;
	}
}

void Player::removeHandIndicator( bool removeBg )
{
	if(m_ptrHandIndicator)
	{
		m_ptrHandIndicator->removeFromParent();
		m_ptrHandIndicator= NULL;
	}
	if (m_ptrLbHandCardsType)
	{
		m_ptrLbHandCardsType->removeFromParent();
		m_ptrLbHandCardsType = NULL;
	}
	if (m_ptrHandIndicatorBg && removeBg)
	{
		m_ptrHandIndicatorBg->removeFromParent();
		m_ptrHandIndicatorBg = NULL;
	}
}

void Player::updateHandIndicator()
{
	if (!GameUtils::isHandIndicatorOn())
	{
		removeHandIndicator(true);
		return;
	}
	if (m_state == GAME_WAIT)
	{
		return;
	}

	WIN_CARDS_TYPE cardsType = getMaxCardsType();
	if (cardsType <= TYPE_HIGH_CARD || cardsType >= WIN_CARDS_TYPE_COUNT)
	{
		return;
	}

	if (!m_ptrHandIndicatorBg)
	{
		m_ptrHandIndicatorBg = ResourceManager::getInstance()->createSprite(this,"hand_indicator_bg.png");
		addChild(m_ptrHandIndicatorBg,Z_HAND_INDICATOR);
		LayoutUtil::layoutParentCenter(m_ptrHandIndicatorBg,71,-15);
	}

	if (!m_ptrHandIndicator || (m_ptrHandIndicator && m_ptrHandIndicator->getTag() != cardsType))
	{
		removeHandIndicator(false);
		createHandIndicator(cardsType);
	}
}

void Player::createHandIndicator( WIN_CARDS_TYPE cardsType )
{
	float fontScale = 0.0,widthPercent = 0.18;
	int index = 0;
	switch (cardsType)
	{
	case TYPE_ONE_PAIR:
		fontScale = 0.9;
		widthPercent = 0.18;
		index = 0;
		break;
	case TYPE_TWO_PAIRS:
		fontScale = 0.9;
		widthPercent = 0.3;
		index = 0;
		break;
	case TYPE_THREE_OF_A_KIND:
		fontScale = 0.85;
		widthPercent = 0.4;
		index = 1;
		break;
	case TYPE_STRAIGHT:
		fontScale = 0.9;
		widthPercent = 0.55;
		index = 1;
		break;
	case TYPE_FLUSH:
		fontScale = 0.95;
		widthPercent = 0.65;
		index = 1;
		break;
	case TYPE_FULL_HOUSE:
		fontScale = 0.8;
		widthPercent = 0.75;
		index = 2;
		break;
	case TYPE_FOUR_OF_A_KIND:
		fontScale = 0.8;
		widthPercent = 0.85;
		index = 2;
		break;
	case TYPE_STRAIGHT_FLUSH:
		fontScale = 0.65;
		widthPercent = 0.94;
		index = 2;
		break;
	case TYPE_ROYAL_FLUSH:
		fontScale = 0.8;
		widthPercent = 1.0;
		index = 2;
		break;
	default:
		break;
	}
	m_ptrHandIndicator = GameUtils::createScale9(CCString::createWithFormat("card_type_%d.png",index)->getCString(),
		CCRectMake(0,0,16,30),CCRectMake(6,7,3,13));
	m_ptrHandIndicator->setContentSize(
		CCSizeMake((m_ptrHandIndicatorBg->getContentSize().width - 2) * widthPercent,30));
	m_ptrHandIndicator->setTag(cardsType);
	m_ptrHandIndicatorBg->addChild(m_ptrHandIndicator);
	LayoutUtil::layoutParentLeft(m_ptrHandIndicator,1,1);
	m_ptrLbHandCardsType = CCLabelBMFont::create(LocalizationManager::getInstance()->getLocalizedString(
		CCString::createWithFormat("hand_indicator_%d",9 - cardsType)->getCString()),FontConstant::BMFONT_IMPACT_BLACK_EDGE);
	m_ptrLbHandCardsType->setScale(fontScale);
	m_ptrHandIndicatorBg->addChild(m_ptrLbHandCardsType,1);
	LayoutUtil::layoutParentCenter(m_ptrLbHandCardsType,3);
}

WIN_CARDS_TYPE Player::getMaxCardsType()
{
	WIN_CARDS_TYPE cardsType = TYPE_HIGH_CARD;
	TableLayer* ptrTableLayer = dynamic_cast<TableLayer*>(getParent());
	vector<int> cards = ptrTableLayer->getPublicAreaLayer()->getPublicCardsVec();
	if (m_baseImgIndex0 != 0)
	{
		cards.push_back(m_baseImgIndex0);
	}
	if (m_baseImgIndex1 != 0)
	{
		cards.push_back(m_baseImgIndex1);
	}
	if (cards.size() > 0)
	{
		cardsType = CardsTypeUtils::getMaxCardsType(cards);
	}

	return cardsType;
}
