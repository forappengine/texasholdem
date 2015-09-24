#include"SidePool.h"
#include "LayoutUtil.h"
#include "ResourceManager.h"
#include "FontConstant.h"
#include "Utils.h"
#include "TableLayer.h"
#include "GameUtils.h"

bool SidePool::init()
{
	if ( !Layer::init() )
	{
		return false;
	}

	setTouchPenetrate(true);
	//initWithColor(ccc4(200,10,20,125));
	setContentSize(CCSizeMake(LAYER_WIDTH,LAYER_HEIGHT));

	m_ptrLbBetChipAmount = NULL;
	m_ptrIconChip = NULL;
	m_ptrBetLabelBG = NULL;
	for (int i = 0; i < 17; i++)
	{
		m_ptrChipImages[i] = ResourceManager::getInstance()->createFrame(this,
			CCString::createWithFormat("chip_%d.png",Player::CHIP_NUM_FOR_IMAGES[i])->getCString());
	}

	return true;
}

void SidePool::updatePool( INT64 count,bool needShowUpdateAction )
{
	CCLOG("SidePool::updatePool:count:%lld,needShowUpdateAction:%d",count,needShowUpdateAction);
	m_betChipAmount = count;
	if(m_ptrLbBetChipAmount == NULL)
	{
		m_ptrBetLabelBG = ResourceManager::getInstance()->createSprite(this,"bet_chip_bg.png");
		if(needShowUpdateAction)
		{
			m_ptrBetLabelBG->setVisible(false);
		}
		addChild(m_ptrBetLabelBG);
		LayoutUtil::layoutParentCenter(m_ptrBetLabelBG);

		TableLayer* ptrTable = (TableLayer*)(getParent()->getParent());
		m_ptrLbBetChipAmount = CCLabelTTF::create(GameUtils::num2tableStr(m_betChipAmount,ptrTable->blindBet,6).c_str(),
			FontConstant::TTF_IMPACT, 20);
		m_ptrBetLabelBG->addChild(m_ptrLbBetChipAmount);
		LayoutUtil::layoutParentCenter(m_ptrLbBetChipAmount,8,1);

		m_ptrIconChip = CCSprite::createWithSpriteFrame(m_ptrChipImages[0]);	
		m_ptrBetLabelBG->addChild(m_ptrIconChip);
		LayoutUtil::layoutParentLeft(m_ptrIconChip);
	}

	if(needShowUpdateAction)
	{
		showUpdateActions();
	}
	else
	{
		updateBetChipAmountLabel();
	}
}

void SidePool::showUpdateActions()
{
	m_ptrBetLabelBG->runAction(CCSequence::create(
		CCDelayTime::create(1.8f),
		CCCallFunc::create(this,callfunc_selector(SidePool::updateBetChipAmountLabel)),
		CCShow::create(),
		NULL));

	TableLayer* ptrTable = (TableLayer*)(getParent()->getParent());
	for (int i = 0; i < ptrTable->getCapacity(); i++)
	{
		Player* ptrPlayer = ptrTable->getPlayer(i);
		if(!ptrPlayer || !ptrPlayer->getBetLabelBG()->isVisible())
		{
			continue;
		}

		CCSprite* ptrBetLable = ResourceManager::getInstance()->createSprite(this,"bet_chip_bg.png");
		ptrTable->addChild(ptrBetLable,TableLayer::Z_UPDATE_SIDE_POOL_ACTION);

		CCSprite* ptrChipImage = CCSprite::createWithSpriteFrame(m_ptrChipImages[16]);
		for (int j = 1; j < 17; j++)
		{
			if (ptrPlayer->m_betAmount < Player::CHIP_NUM_FOR_IMAGES[j])
			{
				ptrChipImage->setDisplayFrame(m_ptrChipImages[j - 1]);
				break;
			}
		}
		ptrBetLable->addChild(ptrChipImage);
		LayoutUtil::layoutParentLeft(ptrChipImage);

		CCLabelTTF* ptrBetLabel = CCLabelTTF::create(GameUtils::num2tableStr(ptrPlayer->m_betAmount,ptrTable->blindBet,6).c_str(),
			FontConstant::TTF_IMPACT, 20);
		ptrBetLable->addChild(ptrBetLabel);
		LayoutUtil::layoutParentCenter(ptrBetLabel,8,1);

		CCPoint orig = ptrTable->convertToNodeSpace(ptrPlayer->convertToWorldSpace(ptrPlayer->getBetLabelBG()->getPosition()));
		CCPoint dest = ptrTable->convertToNodeSpace(this->convertToWorldSpace(m_ptrBetLabelBG->getPosition()));
		ptrBetLable->setPosition(orig);		
		ptrBetLable->runAction(CCSequence::create(
			CCDelayTime::create(0.8f),
			CCMoveTo::create(1.0f,dest),
			CCCallFunc::create(ptrBetLable, callfunc_selector(CCNode::removeFromParent)),
			NULL));
		CCLOG("SidePool::showUpdateActions:seatPosition:%d",ptrPlayer->m_seatPosition);
	}
}

void SidePool::updateBetChipAmountLabel()
{
	TableLayer* ptrTable = (TableLayer*)(getParent()->getParent());
	m_ptrLbBetChipAmount->setString(GameUtils::num2tableStr(m_betChipAmount,ptrTable->blindBet,6).c_str());
	m_ptrIconChip->setDisplayFrame(m_ptrChipImages[16]);
	for (int i = 1; i < 17; i++)
	{
		if (m_betChipAmount < Player::CHIP_NUM_FOR_IMAGES[i])
		{
			m_ptrIconChip->setDisplayFrame(m_ptrChipImages[i - 1]);
			break;
		}
	}
}

INT64 SidePool::getBetChipAmount()
{
	return m_betChipAmount;
}
