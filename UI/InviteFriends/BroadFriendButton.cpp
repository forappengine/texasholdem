#include "BroadFriendButton.h"
#include "LayoutUtil.h"
#include "Icon.h"
#include "Utils.h"
#include "FontConstant.h"
#include "LocalizationManager.h"
#include "GameUtils.h"

BroadFriendButton::BroadFriendButton(CCSize cellSize):
	m_ptrNormal(NULL),
	m_ptrSelected(NULL)
{
	m_ptrNormal = Layer::create();
	m_ptrNormal->setContentSize(cellSize);

	m_ptrSelected = Layer::create();
	m_ptrSelected ->setContentSize(cellSize);
}

void BroadFriendButton::init(CCLayer* parentLayer, FriendInfoInFriendInvite* ptrFriendInfo)
{
	initWithNormalSprite(m_ptrNormal, m_ptrSelected, NULL, parentLayer, NULL);
/*

	//bg
	CCScale9Sprite* ptrCellBg = CCScale9Sprite::create("blue_bg.png",CCRectMake(0,0,54,54),CCRectMake(20,20,15,15));
	ptrCellBg->setContentSize(CCSizeMake(387, 107));
	addChild(ptrCellBg);
	LayoutUtil::layoutParentCenter(ptrCellBg);
*/

	//bg
	CCSprite* ptrCellBg = CREATE_SPRITE(this,"broad_bg.png",true);
	addChild(ptrCellBg);
	LayoutUtil::layoutParentCenter(ptrCellBg);

	//head portrait
	string photoPath = GameUtils::LOCAL_PHOTO_PATH + "0.jpg";
	CCSprite* ptrHeadSprite = CREATE_SPRITE(this,photoPath.c_str(),true);
	Icon * ptrIcon = Icon::create(86.0f);
	addChild(ptrIcon);
	ptrIcon->setContent(ptrHeadSprite);
	ptrIcon->setStencil(CREATE_SPRITE(this,"photo_mask_clip.png",true));
	LayoutUtil::layoutParentLeft(ptrIcon,15,0);
	ptrIcon->setCover(CREATE_SPRITE(this,"photo_mask.png",true));

	//name
	CCLabelTTF* ptrLbName = CCLabelTTF::create(ptrFriendInfo->name.c_str(),FontConstant::TTF_IMPACT,20);
	addChild(ptrLbName);
	LayoutUtil::layoutParentLeftTop(ptrLbName,113,-7);

	//level
	CCLabelTTF* ptrLbLevel = CCLabelTTF::create(CCString::createWithFormat(
		LOCALIZED_STRING("level_with_format"), ptrFriendInfo->level)->getCString(), 
		FontConstant::TTF_IMPACT,20);
	ptrLbLevel->setColor(ccc3(0x00,0xDA,0xC0));
	addChild(ptrLbLevel);
	LayoutUtil::layoutParentLeftTop(ptrLbLevel,113,-30);

	//Chip
	CCSprite* ptrChipIcon = CREATE_SPRITE(this,"assets_chip_icon.png",true);
	this->addChild(ptrChipIcon);
	LayoutUtil::layoutParentLeftTop(ptrChipIcon,113,-56);

	// Chip Count
	CCLabelTTF* ptrLbChipCount = CCLabelTTF::create(getCountString(ptrFriendInfo->chip).c_str(),FontConstant::TTF_IMPACT,20);
	this->addChild(ptrLbChipCount);
	LayoutUtil::layoutRight(ptrLbChipCount, ptrChipIcon);

	//Diamond
	CCSprite* ptrDiamond = CREATE_SPRITE(this,"assets_diamond_icon.png",true);
	this->addChild(ptrDiamond);
	LayoutUtil::layoutParentLeftTop(ptrDiamond,113,-80);

	// Diamond Count
	CCLabelTTF* ptrLbDiamondCount = CCLabelTTF::create(getCountString(ptrFriendInfo->diamond).c_str(), FontConstant::TTF_IMPACT, 20);
	this->addChild(ptrLbDiamondCount);
	LayoutUtil::layoutRight(ptrLbDiamondCount, ptrDiamond);
}

string BroadFriendButton::getCountString(int count)
{
	string strChipCount = " : ";
	string s = Utils::itoa(count).c_str();
	for(int i = 0; i < s.size();i++)
	{
		int size = s.size();
		strChipCount += s.at(i);
		if(i+1 != size && (size + 2 - i)%3 == 0)
		{
			strChipCount += ',';
		}
	}
	return strChipCount;
}