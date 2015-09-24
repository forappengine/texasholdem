#include "GameRankListLayer.h"
#include "CellButtonContainer.h"
#include "LayoutUtil.h"
#include "FontConstant.h"
#include "BroadGameRankButton.h"
#include "MainMenuLayer.h"
#include "GameUtils.h"
#include "CmdOutputs.h"
#include "LocalizationManager.h"
#include "Node/MenuItemImage.h"

bool GameRankListLayer::init()
{
	if(!VerticalButtonTableView::init())
	{
		return false;
	}

	m_ptrParent = NULL;
	m_ptrLbEmptyInfo = NULL;

	return true;
}

void GameRankListLayer::setSelectors( Layer* parent, SEL_MenuHandler btnJoinSelector, bool isFriend )
{
	m_ptrParent = parent;
	m_pfnBtnJoinSelector = btnJoinSelector;
	m_bIsToShowFriend = isFriend;
}

void GameRankListLayer::createButtons()
{
	if(m_ptrParent == NULL)
	{
		return;
	}

	CCSprite* ptrTopLine = CREATE_SPRITE(this,"Images/separator_line_thin.png",false);
	ptrTopLine->setScaleX(0.328f);
	addChild(ptrTopLine,2);
	LayoutUtil::layoutParentTop(ptrTopLine,0,17);

	vector<PokerPlayerRankInfo>* ptrFriendInfoVec = m_bIsToShowFriend ? &MainMenuLayer::s_friendInfosVec : &MainMenuLayer::s_globalInfosVec;

	// if the list is empty
	if(ptrFriendInfoVec->size() == 0)
	{
		m_ptrLbEmptyInfo = CCLabelTTF::create(LOCALIZED_STRING("info_list_empty"), FontConstant::TTF_IMPACT, 25);
		m_ptrLbEmptyInfo->setDimensions(CCSizeMake(240, 80));
		addChild(m_ptrLbEmptyInfo);
		LayoutUtil::layoutParentCenter(m_ptrLbEmptyInfo);
	}
	else
	{
		if(m_ptrLbEmptyInfo)
		{
			m_ptrLbEmptyInfo->removeFromParent();
			m_ptrLbEmptyInfo = NULL;
		}
	}

	// the list 
	for(int i = 0; i < ptrFriendInfoVec->size(); ++i)
	{
		PokerPlayerRankInfo* ptrFriendInfo = &ptrFriendInfoVec->at(i);
		CellButtonContainer* ptrContainer = new CellButtonContainer();
		ptrContainer->setContentSize(mCellSize);

		BroadGameRankButton* ptrBroadGameRankButton = new BroadGameRankButton(mCellSize);

		if(i == ptrFriendInfoVec->size() - 1 && ptrFriendInfo->account_id == GameUtils::s_myInfo.getAccountID() && !m_bIsToShowFriend)
		{
			ptrBroadGameRankButton->init(m_ptrParent, ptrFriendInfo, -1, m_bIsToShowFriend);
		}
		else
		{
			ptrBroadGameRankButton->init(m_ptrParent, ptrFriendInfo, i, m_bIsToShowFriend);
		}

		ptrBroadGameRankButton->autorelease();
		ptrBroadGameRankButton->setAnchorPoint(CCPointZero);
		ptrBroadGameRankButton->setPosition(CCPointZero);
		ptrBroadGameRankButton->setUserData(ptrFriendInfo);
		ptrContainer->addChild(ptrBroadGameRankButton);

		// Join Button
		if(m_bIsToShowFriend && ptrFriendInfo->onlineType == ON_LINE_NORMAL 
			&& (ptrFriendInfo->account_id != GameUtils::s_myInfo.getAccountID()))
		{
			CCMenuItem* ptrBtnJoin = MenuItemImage::create("btn_join_game.png", 
				"btn_join_down_game.png", m_ptrParent, m_pfnBtnJoinSelector);
			ptrBtnJoin->setUserData(ptrFriendInfo);
			ptrBtnJoin->setTag(i);
			ptrContainer->addChild(ptrBtnJoin,VIEW_BTN_ZORDER);
			LayoutUtil::layoutParentLeftTop(ptrBtnJoin, 182, -30);

			CCLabelTTF* ptrLbJoin = CCLabelTTF::create(LOCALIZED_STRING("word_join"), 
				FontConstant::TTF_IMPACT, 22);
			ptrBtnJoin->addChild(ptrLbJoin);
			LayoutUtil::layoutParentCenter(ptrLbJoin);
		}		m_buttonContainerVec.push_back(ptrContainer);
	}
}

void GameRankListLayer::initParams()
{
	mTableSize = CCSizeMake(319, getContentSize().height - 37);
	mCellSize = CCSizeMake(319,90);
}

void GameRankListLayer::setVisible( bool visible )
{
	VerticalButtonTableView::setVisible(visible);
	m_ptrTableView->setVisible(visible);
}