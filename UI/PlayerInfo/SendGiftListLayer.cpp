#include "SendGiftListLayer.h"
#include "CellButtonContainer.h"
#include "LayoutUtil.h"
#include "FontConstant.h"
#include "Utils.h"
#include "GameConstants.h"
#include "cocos-ext.h"
#include "GameUtils.h"
#include "Node/MenuItemImage.h"

USING_NS_CC_EXT;

bool SendGiftListLayer::init()
{
	if(!VerticalButtonTableView::init())
	{
		return false;
	}

	m_ptrParent = NULL;
	m_pfnBtnSelector = NULL;

	return true;
}

void SendGiftListLayer::setSelectors( Layer* parent,SEL_MenuHandler btnSelector )
{
	m_ptrParent = parent;
	m_pfnBtnSelector = btnSelector;
}

void SendGiftListLayer::createButtons()
{
	if(m_ptrParent == NULL)
	{
		return;
	}

	set<string> fileNames;
	GameUtils::getAllFileNamesFromDir(GameUtils::WRITABLE_GIFTS_PATH,fileNames,false);
	CCSprite* giftSprites[30][3][MAX_GIFT_LAYER] = {0};
	INT64 giftPrices[30][3] = {0};
	int giftTag[30][3] = {0};
	set<string>::iterator it;
	for (it = fileNames.begin(); it != fileNames.end(); it++)
	{
		string fileName = *it;
		if (fileName.substr(0,5) == "gift_")
		{
			continue;
		}
		int layerIndex = fileName[4] - 48;//char to int
		int col = fileName[6] - 48;
		int row = fileName[7] - 48;
		int realCol = col % 3;
		int realRow = col / 3 * 10 + row;
		string filePath = GameUtils::WRITABLE_GIFTS_PATH + fileName;
		giftSprites[realRow][realCol][layerIndex] = ResourceManager::getInstance()->createSprite(this,filePath.c_str());
		stringstream price(fileName.substr(9,fileName.size()-13));
		price>>giftPrices[realRow][realCol];
		giftTag[realRow][realCol] = Utils::atoi(string(fileName.substr(6,2)),giftTag[realRow][realCol]);
	}

	for (int i = 0; i < 30; i++)
	{
		if (giftSprites[i][0][0] == NULL)
		{
			break;
		}
		CellButtonContainer* ptrContainer = new CellButtonContainer();
		ptrContainer->setContentSize(mCellSize);

		CCMenuItem* ptrBtns[3];
		for (int j = 0; j < 3; j++)
		{
			// menu handler
			CCMenuItem* ptrBtn = MenuItemImage::create("bg_gift.png", 
				"bg_gift.png",m_ptrParent,m_pfnBtnSelector);
			ptrBtn->setTag(giftTag[i][j]);
			ptrContainer->addChild(ptrBtn);
			ptrBtns[j] = ptrBtn;

			// gift images
			for (int k = 0; k < MAX_GIFT_LAYER; k++)
			{
				if (giftSprites[i][j][k] == NULL)
				{
					break;
				}
				ptrBtn->addChild(giftSprites[i][j][k]);
				LayoutUtil::layoutParentCenter(giftSprites[i][j][k]);
			}

			// price
			CCSprite* ptrPriceBg = ResourceManager::getInstance()->createSprite(this,"gift_price_bg.png");
			ptrBtn->addChild(ptrPriceBg);
			LayoutUtil::layoutParentBottom(ptrPriceBg, 1, 2);
			CCLabelTTF* ptrLbPrice = CCLabelTTF::create(Utils::itoa(giftPrices[i][j],10).c_str(), 
				FontConstant::TTF_IMPACT, 20);
			ptrPriceBg->addChild(ptrLbPrice);
			LayoutUtil::layoutParentCenter(ptrLbPrice, 15, 0);

			string iconPath;
			if (j < 2)//chip icon
			{
				iconPath = "assets_chip_icon.png";
			}
			else//diamond icon
			{
				iconPath = "assets_diamond_icon.png";
			}
			CCSprite* ptrIcon = CREATE_SPRITE(this, iconPath.c_str(), true);
			ptrIcon->setScale(0.7f);
			ptrPriceBg->addChild(ptrIcon);
			LayoutUtil::layoutLeft(ptrIcon, ptrLbPrice, -10, 0);
		}
		LayoutUtil::layoutParentLeft(ptrBtns[0],2);
		LayoutUtil::layoutRight(ptrBtns[1], ptrBtns[0],9);
		LayoutUtil::layoutRight(ptrBtns[2], ptrBtns[1],9);
		m_buttonContainerVec.push_back(ptrContainer);
	}
}

void SendGiftListLayer::initParams()
{
	mTableSize = CCSizeMake(402, 518);
	mCellSize = CCSizeMake(402, 135);
}

void SendGiftListLayer::setVisible(bool visible)
{
	VerticalButtonTableView::setVisible(visible);
	m_ptrTableView->setVisible(visible);
}
