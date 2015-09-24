#include "CommonUseListLayer.h" 
#include "LayoutUtil.h"
#include "CellButtonContainer.h"
#include "FontConstant.h"
#include "BroadCommonUseButton.h"

bool CommonUseListLayer::init()
{
	if(!VerticalButtonTableView::init())
	{
		return false;
	}

	m_ptrParent = NULL;

	return true;
}

void CommonUseListLayer::setSelectors( Layer* parent, vector<const char*>* ptrCommonUseVec, SEL_MenuHandler pfnCommonUseSelector )
{
	m_ptrParent = parent;
	m_ptrCommonUseVec = ptrCommonUseVec;
	m_pfnCommonUseSelector = pfnCommonUseSelector;
}

void CommonUseListLayer::createButtons()
{
	if(m_ptrParent == NULL)
	{
		return;
	}

	for(int i = 0; i < m_ptrCommonUseVec->size(); i++)
	{
		const char* ptrStrCommonUse = m_ptrCommonUseVec->at(i);
		CellButtonContainer* ptrContainer = new CellButtonContainer();
		ptrContainer->setContentSize(mCellSize);

		BroadCommonUseButton* ptrBroad = new BroadCommonUseButton(mCellSize);
		ptrBroad->init(m_ptrParent, ptrStrCommonUse, m_pfnCommonUseSelector);
		ptrBroad->autorelease();
		ptrBroad->setAnchorPoint(CCPointZero);
		ptrBroad->setPosition(CCPointZero);
		ptrBroad->setUserData((void*)ptrStrCommonUse);
		ptrContainer->addChild(ptrBroad);

		m_buttonContainerVec.push_back(ptrContainer);
	}
}

void CommonUseListLayer::initParams()
{
	mTableSize = CCSizeMake(151, 398);
	mCellSize = CCSizeMake(151, 40);
}

void CommonUseListLayer::setVisible( bool visible )
{
	VerticalButtonTableView::setVisible(visible);
	m_ptrTableView->setVisible(visible);
}
