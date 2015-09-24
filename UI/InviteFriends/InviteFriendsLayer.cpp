#include "InviteFriendsLayer.h"
#include "ResourceManager.h"
#include "LayoutUtil.h"
#include "FontConstant.h"
#include "Utils.h"
#include "LocalizationManager.h"
#include "Node/MenuItemImage.h"

bool InviteFriendsLayer::init()
{
	if(!Layer::init())
	{
		return false;
	}

	m_ptrBg = CREATE_SPRITE(this,"Images/invite_bg.png",false);
	this->addChild(m_ptrBg);
	LayoutUtil::layoutParentCenter(m_ptrBg);

	initLb();
	initBtn();
	initFriendsListLayer();

	//setTouchPenetrate(true);

	return true;
}

void InviteFriendsLayer::onEnter()
{
	Layer::onEnter();
	// Animate
	CCAction* ptrAct = CCSequence::create(
		CCEaseBackOut::create(CCScaleTo::create(0.4f,1.0f)),
		NULL
		);
	m_ptrBg->setScale(0.1f);
	m_ptrBg->runAction(ptrAct);

	// disable parent
	lockParent();
}

void InviteFriendsLayer::initLb()
{
	// Invite friends Label
	CCLabelTTF* ptrLbInvite = CCLabelTTF::create(LOCALIZED_STRING("title_invite_friends"),FontConstant::TTF_IMPACT,30);
	m_ptrBg->addChild(ptrLbInvite);
	LayoutUtil::layoutParentTop(ptrLbInvite,0,-18);
}

void InviteFriendsLayer::initBtn()
{
	// Back Button
	CCMenuItem* ptrBtnBack = MenuItemImage::create("btn_close.png","btn_close_down.png",
		this,menu_selector(InviteFriendsLayer::onBtnBackCallback));
	ptrBtnBack->setRotation(-90.0f);
	m_ptrBg->addChild(ptrBtnBack);
	LayoutUtil::layoutParentRightTop(ptrBtnBack,-14,-14);
	registerMenuItem(ptrBtnBack);
}

void InviteFriendsLayer::initFriendsListLayer()
{
	m_friendsInfoVec.clear();
	for(int i=0; i<9; i++)
	{
		FriendInfoInFriendInvite friendInfo;
		friendInfo.canInvited = true;
		friendInfo.name = CCString::createWithFormat(LOCALIZED_STRING("word_name"), Utils::itoa(i).c_str())->getCString();
		friendInfo.level = 2 * i;
		friendInfo.chip = 500 * i;
		friendInfo.diamond = 100 * i;
		friendInfo.isInvited = true;
		m_friendsInfoVec.push_back(friendInfo);
	}

	m_ptrFriendsListLayer = FriendsListLayer::create();
	m_ptrFriendsListLayer->setSelectors(this, &m_friendsInfoVec, menu_selector(InviteFriendsLayer::onBtnInviteCallback));
	m_ptrBg->addChild(m_ptrFriendsListLayer);
	LayoutUtil::layoutParentCenter(m_ptrFriendsListLayer, 0, -6);

}

void InviteFriendsLayer::onBtnBackCallback(CCObject* pSender)
{
	CCLOG("InviteFriendsLayer::onBtnBackCallback");
	CCAction* ptrAct = CCSequence::create(
		CCEaseBackIn::create(CCScaleTo::create(0.4f,0.1f)),
		CCCallFunc::create(this,callfunc_selector(InviteFriendsLayer::unlockParentAndCleanup)),
		NULL
		);
	m_ptrBg->runAction(ptrAct);
}

void InviteFriendsLayer::onBtnInviteCallback(CCObject* pSender)
{
	CCNode* sender = dynamic_cast<CCNode*>(pSender);
	if(sender)
	{
		void* ptrUserData = sender->getUserData();
		FriendInfoInFriendInvite* friendInfo = static_cast<FriendInfoInFriendInvite*>(ptrUserData);
		CCLOG("InviteFriendsLayer::onBtnInviteCallback + %s", friendInfo->name.c_str());
	}
}

bool InviteFriendsLayer::onKeyBackClicked()
{
	onBtnBackCallback(NULL);
	return true;
}

void InviteFriendsLayer::lockParent()
{
	Layer* parent = dynamic_cast<Layer*>(this->getParent());
	if(parent != NULL)
		parent->setTouchEnabled(false);
}

void InviteFriendsLayer::unlockParentAndCleanup()
{
	Layer* parent = dynamic_cast<Layer*>(this->getParent());
	if(parent != NULL)
		parent->setTouchEnabled(true);
	removeFromParent();
}


