#include "BroadCommonUseButton.h"
#include "LayoutUtil.h"
#include "FontConstant.h"
#include "ResourceManager.h"

BroadCommonUseButton::BroadCommonUseButton(CCSize cellSize):
	m_ptrNormal(NULL),
	m_ptrSelected(NULL)
{
	m_ptrNormal = CCNode::create();
	m_ptrNormal->setContentSize(cellSize);

	m_ptrSelected = CCNode::create();
	m_ptrSelected ->setContentSize(cellSize);
}

void BroadCommonUseButton::init( Layer *parentLayer,const char* ptrCommonUseTxt, SEL_MenuHandler menuSelector )
{
	initWithNormalSprite(m_ptrNormal, m_ptrSelected, NULL, parentLayer, menuSelector);

	CCLabelTTF* ptrLbCommonUse = CCLabelTTF::create(ptrCommonUseTxt,FontConstant::TTF_IMPACT,18);
	addChild(ptrLbCommonUse);
	LayoutUtil::layoutParentCenter(ptrLbCommonUse);

	CCSprite* ptrBottomLine = CREATE_SPRITE(this,"Images/separator_line_thin.png",true);
	ptrBottomLine->setScaleX(0.146f);
	addChild(ptrBottomLine);
	LayoutUtil::layoutParentBottom(ptrBottomLine,0,-17);
}
