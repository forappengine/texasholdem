#include "GameLotteryLayer.h"
#include "ResourceManager.h"
#include "LayoutUtil.h"
#include "FontConstant.h"
#include "Utils.h"
#include "TableLayer.h"
#include "Cmds.h"
#include "CmdSender.h"
#include "GameUtils.h"
#include "Screen.h"
#include "PlayerInfo.h"
#include "SoundPlayer.h"
#include "LocalizationManager.h"
#include "Node/MenuItemImage.h"

#define LIGHT_TAG 10

const int GameLotteryLayer::DEFAULT_SELECTED_INDEX = 5;
const int GameLotteryLayer::MUlTIPLE_COUNT[DIAMOND_BTN_COUNT + 1] = { 100, 20, 10, 5, 2, 1, 1 };
const int GameLotteryLayer::CircleContent[CIRCLE_CONTENT_COUNT] = { 1000, 2000, 3000, 4000, 5000, 30000, 40000, 50000 };
const int GameLotteryLayer::ChipAmountPositionOffsetX[CIRCLE_CONTENT_COUNT] = { 173, 249, 260, 206, 107, 34, 22, 78 };
const int GameLotteryLayer::ChipAmountPositionOffsetY[CIRCLE_CONTENT_COUNT] = { 260, 205, 107, 35, 22, 80, 174, 248 };
const int GameLotteryLayer::CircleHeightLightRotation[CIRCLE_CONTENT_COUNT] = { 45, 90, 135, 180, 225, 268, 315, 0 };
const int GameLotteryLayer::CircleHeightLightPositionX[CIRCLE_CONTENT_COUNT] = { 182, 249, 252, 189, 98, 30, 29, 93 };
const int GameLotteryLayer::CircleHeightLightPositionY[CIRCLE_CONTENT_COUNT] = { 253, 191, 99,  34,  31, 94, 184, 249 };
const int GameLotteryLayer::LotteryProbability[CIRCLE_CONTENT_COUNT] = { 1, 1, 1, 1, 1, 1, 1, 1 };

GameLotteryLayer::GameLotteryLayer(){}

GameLotteryLayer::~GameLotteryLayer(){}

bool GameLotteryLayer::init()
{
	CCLOG("GameLotteryLayer::init");
	if (!Layer::init())
	{
		return false;
	}

	//initWithColor(ccc4(0,0,0,128));
	m_bIsRunning = false;
	m_bIsLotteryNormally = false;
	m_bIsFree = true;
	m_bIsShowing = false;
	m_ptrMenuGo = NULL;
	m_ptrMenuHint = NULL;
	m_iCurLottery = 0;
	m_iChoiceDiamond = decideDefaultSelect();
	m_bTempSoundOn = true;
	memset(m_ptrLbBtnGoDiamond, 0, sizeof(m_ptrLbBtnGoDiamond[0]) * 3);

	setContentSize(CCSizeMake(320,380));
	m_ptrBg = CREATE_SPRITE(this, "Images/bg_desk_game.png", true);
	addChild(m_ptrBg);
	LayoutUtil::layoutParentLeftBottom(m_ptrBg);

	initUI();
	initCircleContent();
	initBackBtn();
	chargeNewDay();
	
	return true;
}

void GameLotteryLayer::onEnter()
{
	Layer::onEnter();
	animateToShow();
}

void GameLotteryLayer::initUI()
{
	// Circle
	m_ptrCircle = CREATE_SPRITE(this, "Images/circle_game.png", true);
	m_ptrBg->addChild(m_ptrCircle);
	LayoutUtil::layoutParentTop(m_ptrCircle, 0, 78);

	CCSprite* ptrCircleFront = CREATE_SPRITE(this, "Images/circle_front.png", true);
	m_ptrBg->addChild(ptrCircleFront);
	LayoutUtil::layoutCenter(ptrCircleFront,m_ptrCircle);

	// Plus Button
	m_ptrBtnPlus = MenuItemImage::create("btn_blue_round.png","btn_blue_round.png",
		this, menu_selector(GameLotteryLayer::onBtnPlusCallback));
	CCSprite* ptrBtnPlusCover = CREATE_SPRITE(this,"btn_blue_round_selected.png",true);
	m_ptrBtnPlus->getSelectedImage()->addChild(ptrBtnPlusCover);
	LayoutUtil::layoutParentCenter(ptrBtnPlusCover);
	m_ptrBg->addChild(m_ptrBtnPlus);
	LayoutUtil::layoutParentLeftBottom(m_ptrBtnPlus, 38);
	registerMenuItem(m_ptrBtnPlus);

	CCSprite* ptrPlus = CREATE_SPRITE(this, "btn_plus.png", true);
	m_ptrBtnPlus->addChild(ptrPlus);
	LayoutUtil::layoutParentCenter(ptrPlus);

	// Minus Button
	m_ptrBtnMinus = MenuItemImage::create("btn_blue_round.png","btn_blue_round.png",
		this, menu_selector(GameLotteryLayer::onBtnMinusCallback));
	CCSprite* ptrBtnMinusCover = CREATE_SPRITE(this,"btn_blue_round_selected.png",true);
	m_ptrBtnMinus->getSelectedImage()->addChild(ptrBtnMinusCover);
	LayoutUtil::layoutParentCenter(ptrBtnMinusCover);
	m_ptrBg->addChild(m_ptrBtnMinus);
	LayoutUtil::layoutParentRightBottom(m_ptrBtnMinus, -40);
	registerMenuItem(m_ptrBtnMinus);

	CCSprite* ptrMinus = CREATE_SPRITE(this, "btn_minus.png", true);
	m_ptrBtnMinus->addChild(ptrMinus);
	LayoutUtil::layoutParentCenter(ptrMinus);
}

void GameLotteryLayer::initCircleContent()
{
	for(int i = 0; i < CIRCLE_CONTENT_COUNT; i++)
	{
		CCLabelTTF* ptrLbReward = CCLabelTTF::create(Utils::itoa(CircleContent[i]).c_str(), FontConstant::TTF_IMPACT, 18);
		ptrLbReward->setRotation(20 + 45 * i);
		m_ptrCircle->addChild(ptrLbReward, 1);
		ptrLbReward->setPosition(ccp(ChipAmountPositionOffsetX[i], ChipAmountPositionOffsetY[i]));
		m_ptrLbMutilply[i] = CCLabelTTF::create("", FontConstant::TTF_IMPACT, 18);
		//m_ptrLbMutilply[i]->setAnchorPoint(ccp(0, 0.5f));
		m_ptrLbMutilply[i]->setColor(ccc3(0,255,255));
		m_ptrLbMutilply[i]->setRotation(15.0f);
		ptrLbReward->addChild(m_ptrLbMutilply[i]);
		LayoutUtil::layoutParentRight(m_ptrLbMutilply[i], 20, -3);
	}
}

void GameLotteryLayer::initBackBtn()
{
	CCSprite* ptrBtnMenuBackNormal = CREATE_SPRITE(this,"game_lottery_arrow.png",false);
	m_ptrMenuBack = ScaleMenuItemSprite::create(ptrBtnMenuBackNormal, ptrBtnMenuBackNormal, 
		this, menu_selector(GameLotteryLayer::onBtnBackCallback),1.0f,1.1f);
	m_ptrBg->addChild(m_ptrMenuBack);
	LayoutUtil::layoutParentTop(m_ptrMenuBack, 0, 132);
	registerMenuItem(m_ptrMenuBack);

	CCLabelBMFont* ptrLbSpin = CCLabelBMFont::create(LOCALIZED_STRING("menu_lottery"),
		FontConstant::BMFONT_GOTHIC725_BLK_BT_BLUE);
	ptrLbSpin->setScale(0.375f);
	m_ptrMenuBack->addChild(ptrLbSpin);
	LayoutUtil::layoutParentTop(ptrLbSpin,3,-18);
}

void GameLotteryLayer::chargeNewDay()
{
	// disable button according to lastLottery date
	if (!GameUtils::s_myInfo.getHasFreeLottery())
	{
		if (m_bIsFree)
		{
			m_iChoiceDiamond = decideDefaultSelect();
		}
		initBtnGo();
		selectBtnByIndex(m_iChoiceDiamond);
	}
	else
	{
		m_bIsFree = true;
		m_iChoiceDiamond = decideDefaultSelect();

		for (int i=0; i<CIRCLE_CONTENT_COUNT; i++)
		{
			m_ptrLbMutilply[i]->setString((LOCALIZED_STRING("multiplication_sign") 
				+ Utils::itoa(MUlTIPLE_COUNT[m_iChoiceDiamond])).c_str());
		}

		// init the free button
		// Normal Sprite
		CCSprite* ptrBtnFreeNormal = CREATE_SPRITE(this, "btn_go.png", true);
		CCSprite* ptrBtnFreeLight = CREATE_SPRITE(this,"btn_go_light.png",false);
		ptrBtnFreeLight->runAction(CCRepeatForever::create(
			CCSequence::create(CCFadeOut::create(1.0f),CCFadeIn::create(1.0f),NULL)));
		ptrBtnFreeNormal->addChild(ptrBtnFreeLight);
		LayoutUtil::layoutParentCenter(ptrBtnFreeLight, 0, 1);
		CCSprite* ptrBtnTxt = CREATE_SPRITE(this, "btn_txt_free.png", true);
		ptrBtnFreeNormal->addChild(ptrBtnTxt);
		LayoutUtil::layoutParentCenter(ptrBtnTxt);
		// Selected Sprite
		CCSprite* ptrBtnFreeSelected = CREATE_SPRITE(this, "btn_go_down.png", true);
		ptrBtnTxt = CREATE_SPRITE(this, "btn_txt_free_down.png", true);
		ptrBtnFreeSelected->addChild(ptrBtnTxt);
		LayoutUtil::layoutParentCenter(ptrBtnTxt);
		// Disabled Sprite
		CCSprite* ptrBtnFreeDisable = CREATE_SPRITE(this, "btn_go_down.png", true);
		ptrBtnTxt = CREATE_SPRITE(this, "btn_txt_free_down.png", true);
		ptrBtnFreeDisable->addChild(ptrBtnTxt);
		LayoutUtil::layoutParentCenter(ptrBtnTxt);
		// Menu
		m_ptrMenuGo = CCMenuItemSprite::create(ptrBtnFreeNormal, ptrBtnFreeSelected, ptrBtnFreeDisable, 
			this, menu_selector(GameLotteryLayer::onBtnGoCallback));
		m_ptrBg->addChild(m_ptrMenuGo);
		LayoutUtil::layoutCenter(m_ptrMenuGo, m_ptrCircle);
		registerMenuItem(m_ptrMenuGo);

		selectBtnByIndex(m_iChoiceDiamond);
		m_ptrBtnMinus->setEnabled(false);
		m_ptrBtnPlus->setEnabled(false);
	}
}

void GameLotteryLayer::initBtnGo()
{
	m_bIsFree = false;
	if (m_ptrMenuGo)
	{
		unRegisterMenuItem(m_ptrMenuGo);
		m_ptrMenuGo->removeFromParent();
		m_ptrMenuGo = NULL;
	}
	// Normal Sprite
	CCSprite* ptrBtnGoNormal = CREATE_SPRITE(this, "btn_go.png", true);
	CCSprite* ptrBtnGoLight = CREATE_SPRITE(this,"btn_go_light.png",false);
	ptrBtnGoLight->runAction(CCRepeatForever::create(
		CCSequence::create(CCFadeOut::create(1.0f),CCFadeIn::create(1.0f),NULL)));
	ptrBtnGoNormal->addChild(ptrBtnGoLight);
	LayoutUtil::layoutParentCenter(ptrBtnGoLight, 0, 1);
	CCSprite* ptrDiamond = CREATE_SPRITE(this,"diamond.png",true);
	ptrBtnGoNormal->addChild(ptrDiamond);
	LayoutUtil::layoutParentCenter(ptrDiamond,0,25);
	m_ptrLbBtnGoDiamond[0] = CCLabelTTF::create("",FontConstant::TTF_IMPACT,30);
	ptrBtnGoNormal->addChild(m_ptrLbBtnGoDiamond[0]);
	LayoutUtil::layoutParentCenter(m_ptrLbBtnGoDiamond[0],0,-15);

	// Selected Sprite
	CCSprite* ptrBtnGoSelected = CREATE_SPRITE(this, "btn_go_down.png", true);
	ptrDiamond = CREATE_SPRITE(this,"diamond.png",true);
	ptrBtnGoSelected->addChild(ptrDiamond);
	LayoutUtil::layoutParentCenter(ptrDiamond, 0, 25);	
	m_ptrLbBtnGoDiamond[1] = CCLabelTTF::create("",FontConstant::TTF_IMPACT,30);
	ptrBtnGoSelected->addChild(m_ptrLbBtnGoDiamond[1]);
	LayoutUtil::layoutParentCenter(m_ptrLbBtnGoDiamond[1],0,-15);

	// Disabled Sprite
	CCSprite* ptrBtnGoDisabled = CREATE_SPRITE(this, "btn_go_down.png", true);
	ptrDiamond = CREATE_SPRITE(this,"diamond.png",true);
	ptrBtnGoDisabled->addChild(ptrDiamond);
	LayoutUtil::layoutParentCenter(ptrDiamond, 0, 25);
	m_ptrLbBtnGoDiamond[2] = CCLabelTTF::create("",FontConstant::TTF_IMPACT,30);
	ptrBtnGoDisabled->addChild(m_ptrLbBtnGoDiamond[2]);
	LayoutUtil::layoutParentCenter(m_ptrLbBtnGoDiamond[2],0,-15);

	// Go
	m_ptrMenuGo = CCMenuItemSprite::create(ptrBtnGoNormal, ptrBtnGoSelected, ptrBtnGoDisabled, 
		this, menu_selector(GameLotteryLayer::onBtnGoCallback));
	m_ptrBg->addChild(m_ptrMenuGo);
	LayoutUtil::layoutCenter(m_ptrMenuGo, m_ptrCircle);
	registerMenuItem(m_ptrMenuGo);
}

bool GameLotteryLayer::ccTouchBegan(cocos2d::CCTouch* pTouch, cocos2d::CCEvent* pEvent)
{
	if (!isTouchEnabled())
	{
		return true;
	}
	if (!Layer::ccTouchBegan(pTouch, pEvent))
	{
		if (m_bIsShowing && !m_bIsRunning)
		{
			CCLOG("GameLottery Showing NotRunning");
			m_bTempSoundOn = false;
			onBtnBackCallback(NULL);
			m_bTempSoundOn = true;
			return true;
		}
		return false;
	}
	CCLOG("GameLottery %s %s", m_bIsShowing?"true":"false", m_bIsRunning?"true":"false");
	return true;
}

bool GameLotteryLayer::onKeyBackClicked()
{
	if (m_bIsShowing && !m_bIsRunning)
	{
		m_bTempSoundOn = false;
		onBtnBackCallback(NULL);
		m_bTempSoundOn = true;
		return true;
	}
	return true;
}

void GameLotteryLayer::onBtnMinusCallback(CCObject* pSender)
{
	if (m_bTempSoundOn)
	{
		SoundPlayer::play(SNDI_CLICK);
	}
	int index = m_iChoiceDiamond;
	if (index >= DIAMOND_BTN_COUNT-1)
	{
		return;
	}
	if (index == DIAMOND_BTN_COUNT-1-1)
	{
		m_ptrBtnMinus->setEnabled(false);
		m_ptrBtnMinus->setOpacity(255.0f * 0.8f);
	}
	index++;
	selectBtnByIndex(index);
}

void GameLotteryLayer::onBtnPlusCallback(CCObject* pSender)
{
	if (m_bTempSoundOn)
	{
		SoundPlayer::play(SNDI_CLICK);
	}
	int index = m_iChoiceDiamond;
	if (index <= 0)
	{
		return;
	}
	if (index == 1)
	{
		m_ptrBtnPlus->setEnabled(false);
		m_ptrBtnPlus->setOpacity(255.0f * 0.8f);
	}
	index--;
	selectBtnByIndex(index);
}

void GameLotteryLayer::onBtnGoCallback(CCObject* pSender)
{
	if (m_bTempSoundOn)
	{
		SoundPlayer::play(SNDI_CLICK);
	}
	if ((!GameUtils::s_myInfo.getHasFreeLottery()) && (GameUtils::s_myInfo.getDiamond() < MUlTIPLE_COUNT[m_iChoiceDiamond]))
	{
		CCLOG("== GameLottery not enough money ==");
		disableBtns();
		setGoResult(DIAMOND_NOT_ENOUGH);
		animateToShowHint();
		this->scheduleOnce(schedule_selector(GameLotteryLayer::removeHint), 2.0f);
		return;
	}

	disableBtns();
	SoundPlayer::play(SNDI_SLOT_ROLL);

	PlayLotteryCmd* ptrPlayerLotteryCmd = CmdSender::getInstance()->getPlayLotteryCmd(m_bIsFree ? 0 : MUlTIPLE_COUNT[m_iChoiceDiamond]);
	ptrPlayerLotteryCmd->m_needShowWaitingPanel = false;
	CCNotificationCenter::sharedNotificationCenter()->addObserver(this, callfuncO_selector(GameLotteryLayer::onGetLotteryNetworkCallback), ptrPlayerLotteryCmd->m_cmdName.c_str(), NULL);
	CmdSender::getInstance()->sendCommands(1, ptrPlayerLotteryCmd);

	m_ptrCircle->runAction(CCRepeatForever::create(CCRotateBy::create(2.0f, 360 * 4)));
}

void GameLotteryLayer::onBtnBackCallback(CCObject* pSender)
{
	if (m_bTempSoundOn)
	{
		SoundPlayer::play(SNDI_CLICK);
	}
	SoundPlayer::play(SNDI_SLOT_SHOW_HIDE);
	
	animateToExit();
}

void GameLotteryLayer::onBtnRewardCallback(CCObject* pSender)
{
	if (m_bTempSoundOn)
	{
		SoundPlayer::play(SNDI_CLICK);
	}
	unschedule(schedule_selector(GameLotteryLayer::removeHint));
	enableBtns();

	// delete hint
	CCMenuItemSprite* item = static_cast<CCMenuItemSprite*> (pSender);
	unRegisterMenuItem(item);
	item->runAction(CCSequence::create(
		CCCallFunc::create(item, callfunc_selector(CCMenuItemSprite::removeFromParent)), 
		NULL));
	m_ptrMenuHint = NULL;

	// reset circle
	m_ptrCircle->setRotation(0.0f);
	// delete the highLight
	CCNode* ptrLight = m_ptrCircle->getChildByTag(LIGHT_TAG);
	if(ptrLight)
	{
		ptrLight->removeFromParent();
	}

	chargeNewDay();
}

void GameLotteryLayer::onCircleStop()
{
	SoundPlayer::stopAllSounds();
	enableBtns();

	// result Go button
	if (m_bIsLotteryNormally)
	{
		m_bIsLotteryNormally = false;
		SoundPlayer::play(SNDI_SLOT_RESULT);

		// remove old highlight
		CCNode* ptrLight = m_ptrCircle->getChildByTag(LIGHT_TAG);
		if(ptrLight)
		{
			ptrLight->removeFromParent();
			ptrLight = NULL;
		}

		// create highlight
		ptrLight = CREATE_SPRITE(this, "lottery_selected_game.png", true);
		ptrLight->setTag(LIGHT_TAG);
		ptrLight->setRotation(CircleHeightLightRotation[m_iCurLottery]);
		m_ptrCircle->addChild(ptrLight);
		ptrLight->setPosition(CircleHeightLightPositionX[m_iCurLottery], CircleHeightLightPositionY[m_iCurLottery]);

		disableBtns();
		setGoResult(WIN_CHIPS);
		animateToShowHint();
		scheduleOnce(schedule_selector(GameLotteryLayer::removeHint), 5.0f);
	}
	// reset buttons
	//initBtnGo();
	//chargeNewDay();
}

void GameLotteryLayer::selectBtnByIndex(int index)
{
	// The bigger the index is, the smaller the multiply number is. 
	m_iChoiceDiamond = index;
	if (m_iChoiceDiamond <= 0)
	{
		// Disable plus button
		m_iChoiceDiamond = 0;
		m_ptrBtnPlus->setEnabled(false);
		m_ptrBtnPlus->setOpacity(255.0f * 0.8f);
	}
	else
	{
		m_ptrBtnPlus->setEnabled(true);
		m_ptrBtnPlus->setOpacity(255.0f);
	}
	if (m_iChoiceDiamond >= DIAMOND_BTN_COUNT-1)
	{
		// Disable minus button
		m_iChoiceDiamond = DIAMOND_BTN_COUNT-1;
		m_ptrBtnMinus->setEnabled(false);
		m_ptrBtnMinus->setOpacity(255.0f * 0.8f);
	}
	else
	{
		m_ptrBtnMinus->setEnabled(true);
		m_ptrBtnMinus->setOpacity(255.0f);
	}

	// set multiply number as selected diamonds number
	for (int i = 0; i < CIRCLE_CONTENT_COUNT; i++)
	{
		m_ptrLbMutilply[i]->setString((LOCALIZED_STRING("multiplication_sign") 
			+ Utils::itoa(MUlTIPLE_COUNT[m_iChoiceDiamond])).c_str());
	}
	for (int i = 0; i < 3; i++)
	{
		if (m_ptrLbBtnGoDiamond[i])
		{
			m_ptrLbBtnGoDiamond[i]->setString((" X " + Utils::itoa(MUlTIPLE_COUNT[m_iChoiceDiamond])).c_str());
		}
	}
}

void GameLotteryLayer::animateToShow()
{
	SoundPlayer::play(SNDI_SLOT_SHOW_HIDE);
	setIsShowing(true);

	m_ptrBg->setPositionY(m_ptrBg->getPositionY() - BACKGROUND_MOVE_HEIGHT);
	CCPoint deltaPoint = ccp(0, BACKGROUND_MOVE_HEIGHT);
	m_ptrBg->runAction(CCSequence::create(
		CCMoveBy::create(0.5f, deltaPoint), 
		CCCallFunc::create(this, callfunc_selector(GameLotteryLayer::onAnimateEnd)), 
		NULL));
}

void GameLotteryLayer::animateToExit()
{
	SoundPlayer::play(SNDI_SLOT_SHOW_HIDE);
	m_bIsShowing = false;

	CCPoint deltaPoint = ccp(0, -BACKGROUND_MOVE_HEIGHT);
	m_ptrBg->runAction(CCSequence::create(
		CCMoveBy::create(0.5f, deltaPoint), 
		CCCallFunc::create(this, callfunc_selector(GameLotteryLayer::onAnimateEnd)), 
		NULL));
}

void GameLotteryLayer::onAnimateEnd()
{
	if (!m_bIsShowing)
	{
		setIsShowing(false);
		removeFromParent();
	}
}

void GameLotteryLayer::onGetLotteryNetworkCallback(CCObject* pSender)
{
	PlayLotteryCmd* ptrPlayLotteryCmd = (PlayLotteryCmd*)pSender;
	int errorNo = ptrPlayLotteryCmd->m_ptrOutput->errorInfo.errorNO;
	if(errorNo == RespHandler::OK)
	{
		m_bIsLotteryNormally = true;
		m_ptrCircle->stopAllActions();
		m_iCurLottery = ptrPlayLotteryCmd->m_ptrOutput->index;
		m_ptrCircle->runAction(CCSequence::create(
			CCRotateBy::create(2.0f, 360 * 4),
			CCEaseSineOut::create(CCRotateTo::create(3.0f, 360.0f * 3 + (22.5f) + 45 * (7 - m_iCurLottery))),
			CCCallFunc::create(this, callfunc_selector(GameLotteryLayer::onCircleStop)),
			NULL));
		GameUtils::s_myInfo.setHasFreeLottery(false);
		GameUtils::s_myInfo.setChip(GameUtils::s_myInfo.getChip() + ptrPlayLotteryCmd->m_ptrOutput->reward);
		GameUtils::s_myInfo.setDiamond(ptrPlayLotteryCmd->m_ptrOutput->diamond); // set diamond
	}
	else
	{
		m_bIsLotteryNormally = false;
		RespHandler::getInstance()->setLastErrorNO(RespHandler::OK);// mark had dealt with the error
		m_ptrCircle->stopAllActions();
		m_iCurLottery = 0;
		m_ptrCircle->runAction(CCSequence::create(
			CCRotateBy::create(2.0f, 360 * 4),
			CCEaseSineOut::create(CCRotateTo::create(3.0f, 360.0f * 3)),
			CCCallFunc::create(this, callfunc_selector(GameLotteryLayer::onCircleStop)),
			NULL));
	}
	CCNotificationCenter::sharedNotificationCenter()->removeObserver(this, ptrPlayLotteryCmd->m_cmdName.c_str());
	CC_SAFE_DELETE(ptrPlayLotteryCmd);
}

void GameLotteryLayer::setGoResult(int type)
{
	CCSprite* ptrBtnGoLarge = NULL;
	CCLabelTTF* ptrLbUp = NULL;
	CCLabelTTF* ptrLbDown1 = NULL;
	CCLabelTTF* ptrLbDown2 = NULL;
	CCLabelTTF* ptrLbDown3 = NULL;
	vector<string> reminder;
	int iReward = 0;
	switch (type)
	{
	case DIAMOND_NOT_ENOUGH:
		ptrBtnGoLarge = CREATE_SPRITE(this, "btn_go_large.png", false);
		Utils::SeperateString(LOCALIZED_STRING("reminder_diamond_not_enough"), ' ', reminder);
		ptrLbUp = CCLabelTTF::create(reminder.at(0).c_str(), FontConstant::TTF_IMPACT, 35);
		ptrLbDown1 = CCLabelTTF::create(reminder.at(1).c_str(), FontConstant::TTF_IMPACT, 25);		// to adjust the line height
		ptrLbDown2 = CCLabelTTF::create(reminder.at(2).c_str(), FontConstant::TTF_IMPACT, 25);
		ptrLbDown3 = CCLabelTTF::create(reminder.at(3).c_str(), FontConstant::TTF_IMPACT, 25);

		ptrBtnGoLarge->addChild(ptrLbUp);
		ptrBtnGoLarge->addChild(ptrLbDown1);
		ptrBtnGoLarge->addChild(ptrLbDown2);
		ptrBtnGoLarge->addChild(ptrLbDown3);
#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
		LayoutUtil::layoutParentTop(ptrLbUp, 0, -10);
		LayoutUtil::layoutBottom(ptrLbDown1, ptrLbUp, 0, 13);
		LayoutUtil::layoutBottom(ptrLbDown2, ptrLbDown1, 0, 13);
		LayoutUtil::layoutBottom(ptrLbDown3, ptrLbDown2, 0, 13);
#else
		LayoutUtil::layoutParentTop(ptrLbUp, 0, -18);
		LayoutUtil::layoutBottom(ptrLbDown1, ptrLbUp, 0, 5);
		LayoutUtil::layoutBottom(ptrLbDown2, ptrLbDown1, 0, 7);
		LayoutUtil::layoutBottom(ptrLbDown3, ptrLbDown2, 0, 7);
#endif

		m_ptrMenuHint = CCMenuItemSprite::create(ptrBtnGoLarge, ptrBtnGoLarge,  this, menu_selector(GameLotteryLayer::onBtnRewardCallback));
		m_ptrBg->addChild(m_ptrMenuHint, 1);
		LayoutUtil::layoutCenter(m_ptrMenuHint, m_ptrCircle);
		registerMenuItem(m_ptrMenuHint);
		break;
	case WIN_CHIPS:
		iReward = CircleContent[m_iCurLottery] * MUlTIPLE_COUNT[m_iChoiceDiamond];
		ptrBtnGoLarge = CREATE_SPRITE(this, "btn_go_large.png", false);
		Utils::SeperateString(LOCALIZED_STRING("reminder_win_chips"), ' ', reminder);
		ptrLbUp = CCLabelTTF::create(reminder.at(0).c_str(), FontConstant::TTF_IMPACT, 40);
		ptrLbDown1 = CCLabelTTF::create(Utils::num2strF(iReward, 9).c_str(), FontConstant::TTF_IMPACT, 25);
		ptrLbDown2 = CCLabelTTF::create(reminder.at(2).c_str(), FontConstant::TTF_IMPACT, 25);

		ptrBtnGoLarge->addChild(ptrLbUp);
		ptrBtnGoLarge->addChild(ptrLbDown1);
		ptrBtnGoLarge->addChild(ptrLbDown2);
#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
		LayoutUtil::layoutParentTop(ptrLbUp, 0, -20);
		LayoutUtil::layoutBottom(ptrLbDown1, ptrLbUp, 0, 10);
		LayoutUtil::layoutBottom(ptrLbDown2, ptrLbDown1, 0, 10);
#else
		LayoutUtil::layoutParentTop(ptrLbUp, 0, -25);
		LayoutUtil::layoutBottom(ptrLbDown1, ptrLbUp, 0, 5);
		LayoutUtil::layoutBottom(ptrLbDown2, ptrLbDown1, 0, 5);
#endif

		m_ptrMenuHint = CCMenuItemSprite::create(ptrBtnGoLarge, ptrBtnGoLarge,  this, menu_selector(GameLotteryLayer::onBtnRewardCallback));
		m_ptrBg->addChild(m_ptrMenuHint, 1);
		LayoutUtil::layoutCenter(m_ptrMenuHint, m_ptrCircle);
		registerMenuItem(m_ptrMenuHint);
		break;
	default:
		break;
	}
}

void GameLotteryLayer::setIsShowing(bool isLotteryShowing)
{
	m_bIsShowing = isLotteryShowing;
	TableLayer *parent = static_cast<TableLayer *>(getParent());
	if (parent != NULL)
	{
		parent->setBtnLotteryVisible(!isLotteryShowing);
	}
}

void GameLotteryLayer::animateToShowHint()
{
	m_ptrMenuHint->setScale(0.8f);
	m_ptrMenuHint->runAction(CCEaseBackOut::create(CCScaleTo::create(0.4f, 1.0f, 1.0f)));
}

void GameLotteryLayer::removeHint(float dt)
{
	if (m_ptrMenuHint)
	{
		m_bTempSoundOn = false;
		onBtnRewardCallback(m_ptrMenuHint);
		m_bTempSoundOn = true;
	}
}

void GameLotteryLayer::enableBtns()
{
	m_bIsRunning = false;
	m_ptrMenuGo->setEnabled(true);
	m_ptrMenuBack->setEnabled(true);
	selectBtnByIndex(m_iChoiceDiamond);
}

void GameLotteryLayer::disableBtns()
{
	m_bIsRunning = true;
	m_ptrBtnMinus->setEnabled(false);
	m_ptrBtnPlus->setEnabled(false);
	m_ptrMenuGo->setEnabled(false);	
	m_ptrMenuBack->setEnabled(false);
}

int GameLotteryLayer::decideDefaultSelect()
{
	int ret = DEFAULT_SELECTED_INDEX;
	if (GameUtils::s_myInfo.getHasFreeLottery())
	{
		ret = DIAMOND_BTN_COUNT - 1 - GameUtils::getMyVipLevel();
		return ret;
	}
	INT64 diamond = GameUtils::s_myInfo.getDiamond();
	if (diamond < 0)
	{
	} else if (diamond <= 10)
	{
		ret = 5;
	} else if (diamond <= 20)
	{
		ret = 4;
	} else if (diamond <= 50)
	{
		ret = 3;
	} else if (diamond <= 100)
	{
		ret = 2;
	} else if (diamond <= 299)
	{
		ret = 1;
	} else
	{
		ret = 0;
	}
	return ret;
}