#include "BroadGameRankButton.h"
#include "LayoutUtil.h"
#include "Icon.h"
#include "Utils.h"
#include "FontConstant.h"
#include "GameUtils.h"
#include "PlayerInfo.h"
#include "CmdOutputs.h"
#include "LocalizationManager.h"
#include "ResourceManager.h"

BroadGameRankButton::BroadGameRankButton(CCSize cellSize):
	m_ptrNormal(NULL),
	m_ptrSelected(NULL)
{
	m_ptrNormal = CCNode::create();
	m_ptrNormal->setContentSize(cellSize);

	m_ptrSelected = CCNode::create();
	m_ptrSelected ->setContentSize(cellSize);
}

void BroadGameRankButton::init( Layer* parentLayer, PokerPlayerRankInfo* ptrFriendInfo, int index, bool isShowFriend )
{
	initWithNormalSprite(m_ptrNormal, m_ptrSelected, NULL, parentLayer, NULL);
	
	PokerPlayerRankInfo friendInfo;
	if(ptrFriendInfo->account_id == GameUtils::s_myInfo.getAccountID())
	{
		friendInfo = *ptrFriendInfo;
		friendInfo.onlineType = OFF_LINE;
		ptrFriendInfo = &friendInfo;
	}

	if (ptrFriendInfo->account_id == GameUtils::s_myInfo.getAccountID())
	{
		CCScale9Sprite* ptrCellBgCover = GameUtils::createScale9("light_blue_bg.png",
			CCRectMake(0,0,88,88),CCRectMake(30,30,25,25));
		ptrCellBgCover->setContentSize(CCSizeMake(434,128));
		ptrCellBgCover->setOpacity(0.5 * 255);
		addChild(ptrCellBgCover);
		LayoutUtil::layoutParentCenter(ptrCellBgCover);
	}

	// Rank
	if(index <= 2 && index >= 0 && ptrFriendInfo->winCount > 0)
	{
		char buf[40] ;
		sprintf(buf,"rank_%d.png",index + 1);
		CCSprite* ptrCupSprite = CREATE_SPRITE(this,buf,true);
		addChild(ptrCupSprite);
		LayoutUtil::layoutParentCenter(ptrCupSprite, -123);
	}
	else if(index <= -1 || ptrFriendInfo->winCount == 0)
	{
		CCLabelTTF* ptrLbRank = CCLabelTTF::create("---",FontConstant::TTF_IMPACT,30);
		addChild(ptrLbRank);
		LayoutUtil::layoutParentCenter(ptrLbRank, -123);
	}
	else
	{
		char buf[10];
		sprintf(buf, "%d", index + 1);
		CCLabelTTF* ptrLbRank = CCLabelTTF::create(buf,FontConstant::TTF_IMPACT,30);
		addChild(ptrLbRank);
		LayoutUtil::layoutParentCenter(ptrLbRank, -123);
	}

	// Head Portrait
	string photoPath;
	GameUtils::getPhotoPath(photoPath,ptrFriendInfo->account_id,ptrFriendInfo->photo);
	CCSprite* ptrHeadSprite = CREATE_SPRITE(this,photoPath.c_str(),true);
	Icon* ptrIcon = Icon::create(86.0f);
	addChild(ptrIcon);
	ptrIcon->setContent(ptrHeadSprite);
	ptrIcon->setStencil(CREATE_SPRITE(this,"photo_mask_clip.png",true));
	LayoutUtil::layoutParentLeft(ptrIcon, 72);
	ptrIcon->setCover(CREATE_SPRITE(this,"photo_mask.png",true));

	// Name
	string name = ptrFriendInfo->account_id == GameUtils::s_myInfo.getAccountID() 
		? GameUtils::s_myInfo.getNickName() : ptrFriendInfo->nickName;
	name = GameUtils::cutName(name,12);
	CCLabelTTF* ptrLbName = CCLabelTTF::create(name.c_str(), FontConstant::TTF_IMPACT, 22);
	ptrLbName->setDimensions(CCSizeMake(145,ptrLbName->getContentSize().height));
	ptrLbName->setHorizontalAlignment(kCCTextAlignmentLeft);
	addChild(ptrLbName);
	LayoutUtil::layoutParentLeftTop(ptrLbName, 172, -10);

	if(!isShowFriend)
	{
		// Win
		CCLabelTTF* ptrLbWin = CCLabelTTF::create(LOCALIZED_STRING("win_label"),
			FontConstant::TTF_IMPACT,24);
		ptrLbWin->setColor(ccc3(255,234,0));
		addChild(ptrLbWin);
		LayoutUtil::layoutParentLeftTop(ptrLbWin, 172, -43);

		// Win Count
		CCLabelTTF* ptrLbWinCount = CCLabelTTF::create(
			Utils::num2strF(ptrFriendInfo->winCount,8,",",false).c_str(), 
			FontConstant::TTF_IMPACT, 22);
		ptrLbWinCount->setColor(ccc3(0,255,255));
		addChild(ptrLbWinCount);
		LayoutUtil::layoutRight(ptrLbWinCount, ptrLbWin);
	}
	else
	{
		if(ptrFriendInfo->onlineType == ON_LINE_NORMAL)
		{
			// Icon
			CCSprite* ptrGreenIcon = CREATE_SPRITE(this, "ico_green.png", true);
			addChild(ptrGreenIcon);
			LayoutUtil::layoutParentLeftTop(ptrGreenIcon, 166, -43);
		}
		else
		{
			CCLabelTTF* ptrLbWin = CCLabelTTF::create(LOCALIZED_STRING("win_label"),
				FontConstant::TTF_IMPACT,24);
			ptrLbWin->setColor(ccc3(255,234,0));
			addChild(ptrLbWin);
			LayoutUtil::layoutParentLeftTop(ptrLbWin, 172, -43);

			CCLabelTTF* ptrLbWinCount = CCLabelTTF::create(
				Utils::num2strF(ptrFriendInfo->winCount,8,",",false).c_str(), 
				FontConstant::TTF_IMPACT,22);
			ptrLbWinCount->setColor(ccc3(0,255,255));
			addChild(ptrLbWinCount);
			LayoutUtil::layoutRight(ptrLbWinCount, ptrLbWin);

			if (ptrFriendInfo->onlineType > ON_LINE_NORMAL)
			{
				CCSprite* ptrStatusBg = CREATE_SPRITE(this,"status_bg.png",false);
				addChild(ptrStatusBg);
				LayoutUtil::layoutBottom(ptrStatusBg,ptrIcon,0,24);

				string path = "in_sng.png";
				if (ptrFriendInfo->onlineType == ON_LINE_SO)
				{
					path = "in_shootout.png";
				}
				CCSprite* ptrOnlineStatus = CREATE_SPRITE(this,path.c_str(),false);	
				ptrStatusBg->addChild(ptrOnlineStatus);
				LayoutUtil::layoutParentCenter(ptrOnlineStatus);
			}
		}
	}

	//line
	CCSprite* ptrBottomLine = CREATE_SPRITE(this,"Images/separator_line_thin.png",true);
	ptrBottomLine->setScaleX(0.328f);
	addChild(ptrBottomLine);
	LayoutUtil::layoutParentBottom(ptrBottomLine,0,-17);
}
