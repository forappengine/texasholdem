#include "FriendsListLayer.h"
#include "CellButtonContainer.h"
#include "LayoutUtil.h"
#include "FontConstant.h"
#include "BroadFriendButton.h"
#include "Node/MenuItemImage.h"

bool FriendsListLayer::init()
{
	if(!VerticalButtonTableView::init())
	{
		return false;
	}

	m_ptrParent = NULL;
	m_pfnBtnInviteSelector = NULL;

	//setTouchPenetrate(true);

	return true;
}

void FriendsListLayer::onEnter()
{
	VerticalButtonTableView::onEnter();
}

void FriendsListLayer::setSelectors(CCLayer* parent, vector<FriendInfoInFriendInvite>* ptrFriendsInfo, SEL_MenuHandler btnInviteSelector)
{
	m_ptrParent = parent;
	m_ptrFriendsInfoVec = ptrFriendsInfo;
	m_pfnBtnInviteSelector = btnInviteSelector;
}

void FriendsListLayer::createButtons()
{
	if(m_ptrParent == NULL)
	{
		return ;
	}

	// Create Button
	for(int i = 0; i < m_ptrFriendsInfoVec->size(); i++)
	{
		FriendInfoInFriendInvite* ptrFriendInfo = &m_ptrFriendsInfoVec->at(i);
		CellButtonContainer* ptrContainer = new CellButtonContainer();
		ptrContainer->setContentSize(mCellSize);

		BroadFriendButton* ptrBroadFriendButton = new BroadFriendButton(mCellSize);
		ptrBroadFriendButton->init(m_ptrParent,ptrFriendInfo);
		ptrBroadFriendButton->autorelease();
		ptrBroadFriendButton->setAnchorPoint(CCPointZero);
		ptrBroadFriendButton->setPosition(CCPointZero);
		ptrBroadFriendButton->setUserData(ptrFriendInfo);
		ptrContainer->addChild(ptrBroadFriendButton);

		// Invite Button
		if(ptrFriendInfo->canInvited)
		{
			CCMenuItem* ptrBtnInvite = MenuItemImage::create("btn_invite.png","btn_invite_down.png", m_ptrParent, m_pfnBtnInviteSelector);
			ptrBtnInvite->setUserData(ptrFriendInfo);
			ptrBtnInvite->setTag(i);
			ptrContainer->addChild(ptrBtnInvite);
			LayoutUtil::layoutParentRight(ptrBtnInvite, -26, 0);
			if(ptrFriendInfo->isInvited)
			{
				ptrBtnInvite->setEnabled(false);
			}
		}
		m_buttonContainerVec.push_back(ptrContainer);
	}
}

void FriendsListLayer::initParams()
{
	mTableSize = CCSizeMake(400, 434);
	mCellSize = CCSizeMake(400,118);
}

void FriendsListLayer::setVisible( bool visible )
{
	VerticalButtonTableView::setVisible(visible);
	m_ptrTableView->setVisible(visible);
}