#include "Player5.h"
#include "Player.h"
#include "LayoutUtil.h"
#include "Icon.h"

void Player5::layoutChildren()
{
	m_isSeatRight = false;
	if(m_seatPosition > 2)
	{
		m_isSeatRight = true;
	}

	switch(m_seatPosition)
	{
	case 0:
		LayoutUtil::layoutParentBottom(m_ptrHalo,0,-10);
		m_ptrBetLabelBG->setPosition(ccpAdd(m_ptrHalo->getPosition(),ccp(0,85)));
		break;
	case 1:
		LayoutUtil::layoutParentBottom(m_ptrHalo,0,-10);
		m_ptrBetLabelBG->setPosition(ccpAdd(m_ptrHalo->getPosition(),ccp(25,85)));
		break;
	case 2:
	case 3:
		LayoutUtil::layoutParentTop(m_ptrHalo,0,10);
		m_ptrBetLabelBG->setPosition(ccpAdd(m_ptrHalo->getPosition(),ccp(0,-86)));
		break;
	case 4:
		LayoutUtil::layoutParentBottom(m_ptrHalo,0,-10);
		m_ptrBetLabelBG->setPosition(ccpAdd(m_ptrHalo->getPosition(),ccp(-25,85)));
		break;
	}
	m_ptrNormalBg->setPosition(m_ptrHalo->getPosition());
	LayoutUtil::layoutCenter(m_progressTimer,m_ptrHalo,0,17);
	LayoutUtil::layoutCenter(m_ptrPhoto,m_progressTimer,0,1);
	LayoutUtil::layoutBottom(m_ptrLbChipsInGame->getParent(),m_ptrPhoto,0,-4);
	LayoutUtil::layoutBottom(m_ptrLbState,m_ptrLbChipsInGame->getParent(),0,2);
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
	LayoutUtil::layoutBottom(m_ptrLbState,m_ptrLbChipsInGame->getParent(),0,4);
#endif
	m_ptrWinBg->setPosition(m_ptrHalo->getPosition());
	m_ptrFailBg->setPosition(m_ptrHalo->getPosition());
	m_progressTimerBg->setPosition(m_progressTimer->getPosition());
}

Player5* Player5::create( TableLayer* ptrTableLayer, int seat, const JSONNode& node )
{
	Player5* pRet = new Player5();
	if (pRet && pRet->init(ptrTableLayer,seat,node))
	{
		pRet->autorelease();
		return pRet;
	}
	CC_SAFE_DELETE(pRet);
	return NULL;
}
