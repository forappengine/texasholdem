#include "BetSlider.h"
#include "ResourceManager.h"
#include "LayoutUtil.h"
#include "BetControler.h"
#include "Player.h"
#include "TableLayer.h"
#include "Utils.h"
#include "FontConstant.h"
#include "SoundPlayer.h"
#include "GameUtils.h"
#include "Node/MenuItemImage.h"

BetSlider::BetSlider(){}

BetSlider::~BetSlider(){}

bool BetSlider::init()
{
	if (!Layer::init())
	{
		return false;
	}

	//initWithColor(ccc4(255,0,0,125));
	setContentSize(CCSizeMake(230,614));
	//setTouchPenetrate(true);
	initSlider();
	initBtns();

	return true;
}

void BetSlider::initSlider()
{
	m_ptrSlider = ControlSlider::create("slider_bg.png","slider_fg.png","slider_thumb.png");
	m_ptrSlider->addTargetWithActionForControlEvents(this,cccontrol_selector(BetSlider::sliderValueChangedCallback),CCControlEventValueChanged);
	addChild(m_ptrSlider);

	CCSprite *m_arrow = ResourceManager::getInstance()->createSprite(this, "slider_arrow.png",true);
	CCSprite *m_tail = ResourceManager::getInstance()->createSprite(this, "slider_tail.png",true);
	CCSprite *m_halo = ResourceManager::getInstance()->createSprite(this, "slider_halo.png",true);
	m_ptrSlider->addChild(m_halo);
	m_ptrSlider->addChild(m_arrow);
	m_ptrSlider->addChild(m_tail);
	LayoutUtil::layoutParentLeft(m_tail,-76);
	LayoutUtil::layoutParentRight(m_arrow,114);
	LayoutUtil::layoutParentCenter(m_halo,22);

	m_ptrSlider->setRotation(-90);
	LayoutUtil::layoutParentRight(m_ptrSlider, -30, 0);
}

void BetSlider::initBtns()
{
	CCNode *node = CCNode::create();
	CCSprite *sprite = CREATE_SPRITE(this, "btn_allin.png", false);
	node->setContentSize(CCSizeMake(160, 120));
	node->addChild(sprite);
	LayoutUtil::layoutParentBottom(sprite);

	CCNode *nodeDown = CCNode::create();
	CCSprite *spriteDown = CREATE_SPRITE(this, "btn_allin.png", false);
	CCSprite *spriteLight = CREATE_SPRITE(this, "btn_allin_light.png", false);
	spriteDown->addChild(spriteLight);
	LayoutUtil::layoutParentCenter(spriteLight);
	nodeDown->setContentSize(CCSizeMake(160, 120));
	nodeDown->addChild(spriteDown);
	LayoutUtil::layoutParentBottom(spriteDown);

	CCMenuItemSprite* ptrBtnAllIn = CCMenuItemSprite::create(node, nodeDown,
		this,menu_selector(BetSlider::allInBtnCallback));
	addChild(ptrBtnAllIn);
	LayoutUtil::layoutParentTop(ptrBtnAllIn,35,2);
	registerMenuItem(ptrBtnAllIn);


	MenuItemImage* ptrBtnAdd = MenuItemImage::create("btn_add.png","btn_add_down.png",
		this,menu_selector(BetSlider::addBtnCallback));
	addChild(ptrBtnAdd);
	LayoutUtil::layoutParentLeftBottom(ptrBtnAdd,20,160);
	registerMenuItem(ptrBtnAdd);

	MenuItemImage* ptrBtnSub = MenuItemImage::create("btn_sub.png","btn_sub_down.png",
		this,menu_selector(BetSlider::subBtnCallback));
	addChild(ptrBtnSub);
	LayoutUtil::layoutParentLeftBottom(ptrBtnSub,20,60);
	registerMenuItem(ptrBtnSub);
	
	CCSprite* ptrChipLableBG = ResourceManager::getInstance()->createSprite(this,"raise_chip_bg.png");
	m_ptrChipsLabel = CCLabelTTF::create("", FontConstant::TTF_IMPACT, 30);
	m_ptrChipsLabel->setHorizontalAlignment(kCCTextAlignmentCenter);
	m_ptrChipsLabel->setVerticalAlignment(kCCVerticalTextAlignmentCenter);
	ptrChipLableBG->addChild(m_ptrChipsLabel);
	LayoutUtil::layoutParentCenter(m_ptrChipsLabel,-10,2);
	addChild(ptrChipLableBG);
	LayoutUtil::layoutParentLeftBottom(ptrChipLableBG,-2,115);
}

void BetSlider::reInitSlider()
{
	m_ptrSlider->setMinimumValue(getMinRaise());
	m_minSliderValue = m_ptrSlider->getMinimumValue();
	m_ptrSlider->setMaximumValue(getMaxRaise());
	m_maxSliderValue = m_ptrSlider->getMaximumValue();
}

void BetSlider::sliderValueChangedCallback(CCObject *sender, CCControlEvent controlEvent)
{
	if (!isVisible())
	{
		return;
	}
	BetControler* ptrBetControler = (BetControler*)getParent();
	TableLayer* ptrTableLayer = getTable();
	Player* ptrMySelf = ptrTableLayer->mySelf;
	INT64 blindBet = ptrTableLayer->blindBet;
	INT64 newValue;
	if (m_maxSliderValue == m_ptrSlider->getValue())
	{
		newValue = getMaxRaise();
	}
	else
	{
		INT64 currentValue = m_ptrSlider->getValue();
		newValue = currentValue - currentValue % blindBet;
	}
	if (newValue != ptrBetControler->raiseData)
	{
		SoundPlayer::play(SNDI_BLIND_BET_CHOOSE);
	}
	ptrBetControler->raiseData = newValue;
	
	updateChipLable(ptrBetControler->raiseData);
}

void BetSlider::setValue(float value)
{
	m_ptrSlider->setValue(value);
}

void BetSlider::allInBtnCallback(CCObject* pSender)
{
	if (!isVisible())
	{
		return;
	}
	SoundPlayer::play(SNDI_CLICK);
	BetControler* ptrBetControler = (BetControler*)getParent();
	TableLayer* ptrTableLayer = getTable();
	ptrBetControler->raiseData = ptrTableLayer->mySelf->m_chipsInGame + ptrTableLayer->mySelf->m_betAmount;
	updateChipLable(ptrBetControler->raiseData);
	m_ptrSlider->setValue(m_ptrSlider->getMaximumValue());
}

void BetSlider::addBtnCallback(CCObject* pSender)
{
	if (!isVisible())
	{
		return;
	}
	SoundPlayer::play(SNDI_CLICK);
	BetControler* ptrBetControler = (BetControler*)getParent();
	TableLayer* ptrTableLayer = getTable();
	ptrBetControler->raiseData += ptrTableLayer->blindBet;
	updateChipLable(ptrBetControler->raiseData);
	m_ptrSlider->setValue(ptrBetControler->raiseData);
}

void BetSlider::subBtnCallback(CCObject* pSender)
{
	if (!isVisible())
	{
		return;
	}
	SoundPlayer::play(SNDI_CLICK);
	BetControler* ptrBetControler = (BetControler*)getParent();
	TableLayer* ptrTableLayer = getTable();
	INT64 blindBet = ptrTableLayer->blindBet;
	INT64 minRaise = ptrTableLayer->maxBet + blindBet;
	ptrBetControler->raiseData -= ptrTableLayer->blindBet;
	updateChipLable(ptrBetControler->raiseData);
	m_ptrSlider->setValue(ptrBetControler->raiseData);
}

TableLayer* BetSlider::getTable()
{
	BetControler* ptrBetControler = (BetControler*)getParent();
	GameScreen* ptrGameScreen = ptrBetControler->getGameScreen();
	TableLayer* ptrTableLayer = ptrGameScreen->table;

	return ptrTableLayer;
}

void BetSlider::updateChipLable(INT64 chips)
{
	m_ptrChipsLabel->setString(CCString::createWithFormat("%s",GameUtils::num2tableStr(chips,getTable()->blindBet,6).c_str())->getCString());
	LayoutUtil::layoutParentCenter(m_ptrChipsLabel,-10,2);
}

INT64 BetSlider::getMinRaise()
{
	TableLayer* ptrTableLayer = getTable();
	INT64 maxRaise = ptrTableLayer->mySelf->m_chipsInGame + ptrTableLayer->mySelf->m_betAmount;
	INT64 blindBet = ptrTableLayer->blindBet;
	INT64 minRaise = ptrTableLayer->maxBet + blindBet;
	INT64 raiseData;
	if(minRaise > maxRaise)
	{
		raiseData = maxRaise;
	}
	else
	{
		raiseData = minRaise;
	}

	return raiseData;
}

INT64 BetSlider::getMaxRaise()
{
	TableLayer* ptrTableLayer = getTable();
	INT64 maxRaise = ptrTableLayer->mySelf->m_chipsInGame + ptrTableLayer->mySelf->m_betAmount;	

	return maxRaise;
}

bool BetSlider::ccTouchBegan( cocos2d::CCTouch* pTouch, cocos2d::CCEvent* pEvent )
{
	if (!isVisible())
	{
		return false;
	}
	if (Layer::ccTouchBegan(pTouch,pEvent))
	{
		m_ptrSlider->ccTouchBegan(pTouch,pEvent);
		if (_menuTouchDispatcher.getSelectedMenuItem() == NULL 
			&& convertToNodeSpace(pTouch->getLocation()).y <= 110)
		{
			return false;
		}
	}
	else
	{
		setVisible(false);
		BetControler* ptrBetControler = (BetControler*)getParent();
		ptrBetControler->hideBetSlider();
	}
	return true;
}

void BetSlider::ccTouchMoved( cocos2d::CCTouch* pTouch, cocos2d::CCEvent* pEvent )
{
	m_ptrSlider->ccTouchMoved(pTouch,pEvent);
	Layer::ccTouchMoved(pTouch,pEvent);
}

void BetSlider::ccTouchEnded( cocos2d::CCTouch* pTouch, cocos2d::CCEvent* pEvent )
{
	m_ptrSlider->ccTouchEnded(pTouch,pEvent);
	Layer::ccTouchEnded(pTouch,pEvent);
}

void BetSlider::ccTouchCancelled( cocos2d::CCTouch* pTouch, cocos2d::CCEvent* pEvent )
{
	m_ptrSlider->ccTouchCancelled(pTouch,pEvent);
	Layer::ccTouchCancelled(pTouch,pEvent);
}
