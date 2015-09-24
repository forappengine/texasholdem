#include "TalkLayer.h"
#include "ResourceManager.h"
#include "LayoutUtil.h"
#include "FontConstant.h"
#include "Utils.h"
#include "MailBox/SendMailInputLayer.h"
#include "GameUtils.h"
#include "TableLayer.h"
#include "SoundPlayer.h"
#include "LocalizationManager.h"
#include "CommonUseListLayer.h"

vector<Talk> TalkLayer::s_talkVec;

void TalkLayer::pushNewMsg(string name, string content)
{
	Talk temp = {name,content};
	s_talkVec.push_back(temp);
}

void TalkLayer::clearNewMsg()
{
	s_talkVec.clear();
}

bool TalkLayer::init()
{
	if(!Layer::init())
	{
		return false;
	}

	m_ptrTalkList = NULL;

	//initWithColor(ccc4(125,0,0,125));

	initScrollView();
	initCommonUseList();

	return true;
}

void TalkLayer::initScrollView()
{
	m_ptrBG = GameUtils::createScale9("popup_bg_1.png",
		CCRectMake(0,0,100,100),CCRectMake(40,40,20,20));
	m_ptrBG->setContentSize(CCSizeMake(454,426));
	setContentSize(m_ptrBG->getContentSize());
	addChild(m_ptrBG);
	LayoutUtil::layoutParentLeftBottom(m_ptrBG);

	CCScale9Sprite* ptrInputBg = GameUtils::createScale9("bg_input.png",
		CCRectMake(0,0,194,40),CCRectMake(50,0,20,40));
	ptrInputBg->setContentSize(CCSizeMake(255,40));
	m_ptrBG->addChild(ptrInputBg, 1);
	LayoutUtil::layoutParentLeftBottom(ptrInputBg,23,25);

	m_ptrInput = SendMailInputLayer::create(CCSizeMake(250,40));
	m_ptrInput->m_ptrEditBox->setDelegate(this);
	ptrInputBg->addChild(m_ptrInput);
	LayoutUtil::layoutParentCenter(m_ptrInput);

	updateTalkContent();
}

void TalkLayer::initCommonUseList()
{
	// Bg
	CCScale9Sprite* ptrListBg = GameUtils::createScale9("common_use_list_bg.png",
		CCRectMake(0,0,78,78),CCRectMake(20,20,35,35));
	ptrListBg->setContentSize(CCSizeMake(151,398));
	ptrListBg->setOpacity(0.2 * 255);
	m_ptrBG->addChild(ptrListBg);
	LayoutUtil::layoutParentRight(ptrListBg,-14);

	// List
	m_commonUseVec.clear();
	for(int i = 0; i < TALK_COMMON_USE_COUNT; ++i)
	{
		m_commonUseVec.push_back(LOCALIZED_STRING(CCString::createWithFormat("talk_common_use_%d", i)->getCString()));
	}

	m_ptrCommonUseList = CommonUseListLayer::create();
	m_ptrCommonUseList->setSelectors(this, &m_commonUseVec, menu_selector(TalkLayer::onBtnCommonUseListCallback));
	ptrListBg->addChild(m_ptrCommonUseList);
	LayoutUtil::layoutParentCenter(m_ptrCommonUseList);
}

void TalkLayer::updateTalkContent()
{
	// this is a long test for message
	CCNode* ptrNodeTalkContent = CCNode::create();
	int temp_all_content_size_height = 0;

	// create Name and Content and calculate all elements' contentsize.height
	for(int i = 0; i < s_talkVec.size(); i++)
	{
		Talk talk = s_talkVec[i];
		CCLabelTTF* ptrLbName = CCLabelTTF::create(GameUtils::cutName(talk.name,18).c_str(),
			FontConstant::TTF_IMPACT, 20);
		ptrLbName->setColor(ccc3(0,255,255));
		ptrLbName->setDimensions(CCSizeMake(250,ptrLbName->getContentSize().height));
		ptrLbName->setTag(1000 + i);
		ptrLbName->setAnchorPoint(ccp(0, 0.5f));
		ptrLbName->setHorizontalAlignment(kCCTextAlignmentLeft);
		ptrNodeTalkContent->addChild(ptrLbName);

		CCLabelTTF* ptrLbContent = GameUtils::getMultiLinesLabelTTF(talk.content,
			FontConstant::TTF_IMPACT,18,205);
		ptrLbContent->setHorizontalAlignment(kCCTextAlignmentLeft);
		ptrLbContent->setAnchorPoint(ccp(0, 1.0f));
		ptrLbContent->setTag(i);
		ptrNodeTalkContent->addChild(ptrLbContent);

		temp_all_content_size_height += ptrLbName->getContentSize().height + ptrLbContent->getContentSize().height + 5;
	}

	// set position with all elements' height
	if(s_talkVec.size() > 0)
	{
		// set position of the 1st element pair
		ptrNodeTalkContent->getChildByTag(1000)->setPosition(0, temp_all_content_size_height - 20);
		ptrNodeTalkContent->getChildByTag(0)->setPosition(40, ptrNodeTalkContent->getChildByTag(1000)->getPositionY() - 10);
		for(int i = 1; i < s_talkVec.size(); ++i)
		{
			int temp_height = 0;
			temp_height = ptrNodeTalkContent->getChildByTag(i - 1)->getContentSize().height;

			ptrNodeTalkContent->getChildByTag(1000 + i)->setPosition(0, 
				ptrNodeTalkContent->getChildByTag(i - 1)->getPositionY() - temp_height - 20);
			ptrNodeTalkContent->getChildByTag(i)->setPosition(40, 
				ptrNodeTalkContent->getChildByTag(1000 + i)->getPositionY() - 10);
		}
	}

	if(m_ptrTalkList)
	{
		m_ptrTalkList->removeFromParent();
		m_ptrTalkList = NULL;
	}

	ptrNodeTalkContent->setAnchorPoint(ccp(0, 0));
	ptrNodeTalkContent->setPosition(0 ,0);
	ptrNodeTalkContent->setContentSize(CCSizeMake(260, temp_all_content_size_height));

	m_ptrTalkList = CCScrollView::create(CCSizeMake(250, temp_all_content_size_height), ptrNodeTalkContent);
	m_ptrTalkList->setViewSize(CCSizeMake(250, 325));
	m_ptrTalkList->setContentSize(CCSizeMake(250, temp_all_content_size_height));
	m_ptrTalkList->setDelegate(this);
	m_ptrTalkList->setAnchorPoint(ccp(0, 0));
	m_ptrTalkList->setDirection(kCCScrollViewDirectionVertical);
	m_ptrTalkList->setContentOffset(temp_all_content_size_height > 325 
		? m_ptrTalkList->maxContainerOffset() : m_ptrTalkList->minContainerOffset());
	m_ptrTalkList->setTouchEnabled(false);
	m_ptrTalkList->setAnchorPoint(ccp(0, 1));

	m_ptrBG->addChild(m_ptrTalkList);
	m_ptrTalkList->setPosition(30, 68);
}

void TalkLayer::onBtnCommonUseListCallback(CCObject* pSender)
{
	SoundPlayer::play(SNDI_CLICK);
	CCNode* ptrBroad = dynamic_cast<CCNode*>(pSender);
	if(ptrBroad)
	{
		const char* str = (const char*)ptrBroad->getUserData();
		TableLayer* ptrTableLayer = dynamic_cast<TableLayer*>(getParent());
		if(ptrTableLayer)
		{
			ptrTableLayer->sendTalkMsg(str);
		}
	}
	animateHide();
}

void TalkLayer::animateHide()
{
	setTouchEnabled(false);
	CCPoint postion = m_ptrBG->getPosition();
	CCPoint destPostion = ccp(postion.x - m_ptrBG->getContentSize().width / 2,
		postion.y - m_ptrBG->getContentSize().height / 2);
	m_ptrBG->runAction(CCSequence::create(
		CCSpawn::create(CCEaseOut::create(CCScaleTo::create(0.3f, 0.01f), 0.2f),
			CCEaseOut::create(CCMoveTo::create(0.3f, destPostion), 0.2f),
			NULL),
		CCCallFunc::create(this, callfunc_selector(TalkLayer::removeFromParent)),
		NULL));
}

void TalkLayer::editBoxReturn( CCEditBox* editBox )
{
	string content = m_ptrInput->m_ptrEditBox->getText();
	if(!content.compare(""))
		return;

	if (content.size() > 45)
	{
		content = GameUtils::getStringByMaxBytes(content,45);
		content.append(". . .");
	}

	m_ptrInput->clear();

	TableLayer* ptrTableLayer = dynamic_cast<TableLayer*>(this->getParent());
	if(ptrTableLayer)
	{
		ptrTableLayer->sendTalkMsg(content);
	}
	animateHide();
}

void TalkLayer::animateShow()
{
	CCPoint prePostion = m_ptrBG->getPosition();
	m_ptrBG->setPosition(prePostion.x - m_ptrBG->getContentSize().width / 2,
		prePostion.y - m_ptrBG->getContentSize().height / 2);
	m_ptrBG->setScale(0.01f);
	m_ptrBG->runAction(CCSpawn::create(
		CCEaseIn::create(CCScaleTo::create(0.2f, 1.0f), 0.2f),
		CCEaseIn::create(CCMoveTo::create(0.2f, prePostion), 0.2f),
		NULL));
}

void TalkLayer::onExit()
{
	TableLayer* ptrTableLayer = dynamic_cast<TableLayer*>(getParent());
	if (ptrTableLayer)
	{
		ptrTableLayer->m_ptrTalkLayer = NULL;
		ptrTableLayer->setBtnTalkVisible(true);
	}
	CCNotificationCenter::sharedNotificationCenter()->removeAllObservers(this);
	Layer::onExit();
}

bool TalkLayer::onKeyBackClicked()
{
	SoundPlayer::play(SNDI_CLICK);
	if(!isTouchEnabled())
	{
		return true;
	}
	animateHide();
	return true;
}

bool TalkLayer::ccTouchBegan(cocos2d::CCTouch* pTouch, cocos2d::CCEvent* pEvent)
{
	if (!(Layer::ccTouchBegan(pTouch, pEvent) 
		| (m_ptrTalkList && m_ptrTalkList->ccTouchBegan(pTouch, pEvent))))
	{
		animateHide();
	}
	
	return true;
}

void TalkLayer::ccTouchMoved(cocos2d::CCTouch* pTouch, cocos2d::CCEvent* pEvent)
{
	m_ptrTalkList->ccTouchMoved(pTouch, pEvent);
	Layer::ccTouchMoved(pTouch, pEvent);
}

void TalkLayer::ccTouchEnded(cocos2d::CCTouch* pTouch, cocos2d::CCEvent* pEvent)
{
	m_ptrTalkList->ccTouchEnded(pTouch, pEvent);
	Layer::ccTouchEnded(pTouch, pEvent);
}

void TalkLayer::ccTouchCancelled(cocos2d::CCTouch* pTouch, cocos2d::CCEvent* pEvent)
{
	m_ptrTalkList->ccTouchCancelled(pTouch, pEvent);
	Layer::ccTouchCancelled(pTouch, pEvent);
}

void TalkLayer::scrollViewDidScroll(CCScrollView* view)
{

}

void TalkLayer::scrollViewDidZoom(CCScrollView* view)
{

}