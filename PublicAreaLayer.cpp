#include "PublicAreaLayer.h"
#include "LayoutUtil.h"
#include "ResourceManager.h"
#include "cocos2d.h"
#include "TableLayer.h"
#include "LogTag.h"
#include "GameUtils.h"
#include "FontConstant.h"
#include "SoundPlayer.h"

#define MODULE_LOG_TAG "PublicAreaLayer.cpp"

USING_NS_CC;

const char* PublicAreaLayer::WIN_CARDS_TYPE_DESC[WIN_CARDS_TYPE_COUNT] = 
{
	"high_card",
	"one_pair",
	"two_pairs",
	"three_of_a_kind",
	"straight",
	"flush",
	"full_house",
	"four_of_a_kind",
	"straight_flush",
	"royal_flush"
};

//whether has new card released,  we need to clear MaxBet for new circle at this point
bool PublicAreaLayer::updateArea(const JSONNode& node)
{
	bool newCardReleased = false;
	bool isRunning = ((TableLayer*)getParent())->getIsRunning();
	
	if(JsonHelper::optExist(node,"pc") && pclength < node["pc"].size())
	{
		int len = node["pc"].size();
		if (len - pclength == 1)
		{
			SoundPlayer::play(SNDI_SINGLE_FLIP);
		}
		else
		{
			SoundPlayer::play(SNDI_MULTI_FLIP);
		}

		for(int i = pclength;i < len;i++)
		{
			m_cardsIndex[i] = node["pc"][i].as_int();
			CCSprite* ptrNewCard = ResourceManager::getInstance()->createSprite(this, 
				CCString::createWithFormat("card_%02d.png", node["pc"][i].as_int())->getCString());
			int w = ptrNewCard->getContentSize().width;
			int h = ptrNewCard->getContentSize().height;
			ptrNewCard->setPosition(CCPointMake((w / 2 + 2) + (w + 5) * i + 7,h / 2));
			addChild(ptrNewCard);
			m_cardsImgs[i] = ptrNewCard;

			if(!isRunning)
			{
				CCSprite* ptrTempCard = ResourceManager::getInstance()->createSprite(this, "card_00.png");			
				ptrTempCard->setPosition(CCPointMake((w/2+2)+(w+5)*i,h/2));		
				addChild(ptrTempCard);			
				ptrNewCard->setVisible(false);
				ptrTempCard->runAction(CCSequence::create(
					CCOrbitCamera::create(0.4f,1,0,0,90,0,0),
					CCCallFunc::create(ptrTempCard, callfunc_selector(CCNode::removeFromParent)),
					NULL));
				ptrNewCard->runAction(CCSequence::create(
					CCDelayTime::create(0.4f),
					CCShow::create(),
					CCOrbitCamera::create(0.4f,1,0,270,90,0,0),
					NULL));
			}			
		}
		pclength = len;
		newCardReleased = true;
	}

	if(JsonHelper::optExist(node,"winners") || newCardReleased)
	{
		INT64 count = 0;
		stringstream sval(node["pool"].as_string());
		sval>>count;
		if(count > 0)
		{
			if(sidePool == NULL)
			{
				sidePool = SidePool::create();
				addChild(sidePool);
				LayoutUtil::layoutParentCenter(sidePool,0,120);
			}
			sidePool->updatePool(count,!isRunning);
		}
	}

	if(isRunning)
	{
		((TableLayer*)getParent())->setIsRunning(false);
	}

	return newCardReleased;
}

void PublicAreaLayer::clear()
{
	pclength = 0;
	this->removeAllChildren();
	sidePool = NULL;
	memset(m_cardsIndex,0,sizeof(int)*5);
	memset(m_cardsImgs,0,sizeof(CCSprite*)*5);
}

// on "init" you need to initialize your instance
bool PublicAreaLayer::init()
{
	if ( !Layer::init() )
    {
        return false;
    }
	
	memset(m_cardsIndex,0,sizeof(int)*5);
	memset(m_cardsImgs,0,sizeof(CCSprite*)*5);
	sidePool = NULL;

	setTouchPenetrate(true);
	//initWithColor(ccc4(200,10,20,125));
	setContentSize(CCSizeMake(LAYER_WIDTH,LAYER_HEIGHT));

    return true;
}

void PublicAreaLayer::highlightWinCards(CCNode* node,void *ptrVoid)
{
	set<int> *winCards = (set<int> *)ptrVoid;
	for(int i = 0; i < 5; i++)
	{
		if (m_cardsIndex[i] == 0)
		{
			continue;
		}
		set<int>::iterator itr = winCards->find(m_cardsIndex[i]);
		if(itr == winCards->end())
		{
			GameUtils::setMask(m_cardsImgs[i],0,0,0,255*0.5);
			m_cardsImgs[i]->setScale(0.85f);
		}
		else
		{
			winCards->erase(itr);
		}
	}
}

void PublicAreaLayer::showWinCardsType(WIN_CARDS_TYPE type)
{
	if(type < TYPE_HIGH_CARD)
	{
		return;
	}
	CCLOG("PublicAreaLayer::showWinCardsType:type:%s_%d",WIN_CARDS_TYPE_DESC[type],type);

	CCSprite* ptrDescBG = ResourceManager::getInstance()->createSprite(this,"win_cards_type_bg.png");
	CCProgressTimer* ptrProgTimer = CCProgressTimer::create(ptrDescBG);
	ptrProgTimer->setType(kCCProgressTimerTypeBar);
	ptrProgTimer->setMidpoint(ccp(0.5, 0.5));
	ptrProgTimer->setBarChangeRate(ccp(1, 0));
	this->addChild(ptrProgTimer,1);
	LayoutUtil::layoutParentBottom(ptrProgTimer,0,-75);

	CCSprite* ptrWinCardsType = ResourceManager::getInstance()->createSprite(this,
		CCString::createWithFormat("text_%s.png",WIN_CARDS_TYPE_DESC[type])->getCString());
	ptrProgTimer->addChild(ptrWinCardsType,5);
	LayoutUtil::layoutParentCenter(ptrWinCardsType);
	ptrWinCardsType->setVisible(false);
	
	ptrWinCardsType->runAction(CCSequence::create(
		CCDelayTime::create(1.0f),
		CCShow::create(),
		NULL));
	ptrProgTimer->runAction(CCSequence::create(
		CCDelayTime::create(1.0f),
		CCProgressTo::create(0.5f, 100),
		CCDelayTime::create(6.0f),
		CCCallFunc::create(ptrProgTimer, callfunc_selector(CCNode::removeFromParent)),
		NULL));

	for (int i = 0; i < 4; i++)
	{
		CCParticleSystemQuad* ptrSuitParticle = CCParticleSystemQuad::create("Particle/suit_win_cards_type.plist");
		ptrSuitParticle->setTexture(CCTextureCache::sharedTextureCache()->addImage(
			CCString::createWithFormat("Particle/suit_%d.png",i)->getCString()));
		ptrSuitParticle->setVisible(false);
		addChild(ptrSuitParticle);
		LayoutUtil::layoutParentCenter(ptrSuitParticle,0,-30);
		ptrSuitParticle->runAction(CCSequence::create(
			CCDelayTime::create(1.0f),
			CCShow::create(),
			CCDelayTime::create(ptrSuitParticle->getDuration() + 1.0f),
			CCCallFunc::create(ptrSuitParticle,callfunc_selector(CCNode::removeFromParent)),
			NULL));
	}
}

SidePool* PublicAreaLayer::getSidePool()
{
	return sidePool;
}

vector<int> PublicAreaLayer::getPublicCardsVec()
{
	vector<int> cards;
	for (int i = 0; i < 5; i++)
	{
		if (m_cardsIndex[i] != 0)
		{
			cards.push_back(m_cardsIndex[i]);
		}
	}
	return cards;
}
