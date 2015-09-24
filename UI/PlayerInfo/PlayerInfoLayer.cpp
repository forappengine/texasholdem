#include "PlayerInfoLayer.h"
#include "ResourceManager.h"
#include "LayoutUtil.h"
#include "FontConstant.h"
#include "Icon.h"
#include "SendGiftListLayer.h"
#include "Utils.h"
#include "PlayerInfo.h"
#include "TableLayer.h"
#include "GameUtils.h"
#include "CmdSender.h"
#include "PKMessageBox.h"
#include "MainMenuLayer.h"
#include "SoundPlayer.h"
#include "LocalizationManager.h"
#include "Node/MenuItemImage.h"

PlayerInfoLayer::PlayerInfoLayer( TableLayer* ptrTableLayer,PlayerInfo playerInfo,int index ):
	m_ptrTableLayer(ptrTableLayer),
	m_playerInfo(playerInfo),
	m_playerIndex(index),
	m_bIsLayerSwitching(false),
	m_ptrSendGiftListBG(NULL),
	m_ptrBg(NULL),
	m_ptrLbSendGiftTitle(NULL)
{}

PlayerInfoLayer* PlayerInfoLayer::create( TableLayer* ptrTableLayer,PlayerInfo playerInfo,int index )
{
	PlayerInfoLayer* pRet = new PlayerInfoLayer(ptrTableLayer,playerInfo,index);
	if (pRet && pRet->init())
	{
		pRet->autorelease();
		return pRet;
	}
	CC_SAFE_DELETE(pRet);
	return NULL;
}

bool PlayerInfoLayer::init()
{
	if(!Layer::init())
	{
		return false;
	}

	initBG();
	initPhoto();
	initInfo();
	refreshValue(m_playerInfo);
	initBtns();

	return true;
}

void PlayerInfoLayer::initBG()
{
	m_ptrBg = GameUtils::createScale9("popup_bg_1.png",
		CCRectMake(0,0,100,100),CCRectMake(40,40,20,20));
	m_ptrBg->setContentSize(CCSizeMake(298,614));
	addChild(m_ptrBg);
	LayoutUtil::layoutParentLeftBottom(m_ptrBg, 5);
}

void PlayerInfoLayer::initPhoto()
{
	//head portrait
	string photoPath = GameUtils::LOCAL_PHOTO_PATH + "0.jpg";
	if (m_ptrTableLayer->isInGagSet(m_playerInfo.getAccountID()))
	{
		photoPath = "ban.png";
	}
	else
	{
		m_playerInfo.getPhotoPath(photoPath);
	}
	CCSprite* ptrHeadSprite = CREATE_SPRITE(this,photoPath.c_str(),false);
	m_ptrPhoto = Icon::create(86.0f);
	m_ptrPhoto->setContent(ptrHeadSprite);
	m_ptrPhoto->setStencil(CREATE_SPRITE(this,"photo_mask_clip.png",false));
	m_ptrPhoto->setCover(CREATE_SPRITE(this,"photo_mask.png",false));

	m_ptrBtnPhoto = CCMenuItemSprite::create(m_ptrPhoto,m_ptrPhoto,
		this,menu_selector(PlayerInfoLayer::btnPhotoCallback));
	m_ptrBg->addChild(m_ptrBtnPhoto);
	m_ptrBtnPhoto->setScale(1.3f);
	LayoutUtil::layoutParentLeftTop(m_ptrBtnPhoto, 25, -26);
	registerMenuItem(m_ptrBtnPhoto);
}

void PlayerInfoLayer::initInfo()
{
	if ((MainMenuLayer::isFriend(m_playerInfo.getAccountID()))
		|| m_playerInfo.getAccountID() == GameUtils::s_myInfo.getAccountID())
	{
		CCLabelTTF* ptrLbTitle = CCLabelTTF::create(LOCALIZED_STRING("total_assets"), 
			FontConstant::TTF_IMPACT,22);
		ptrLbTitle->setColor(ccc3(0,255,255));
		m_ptrBg->addChild(ptrLbTitle);
		LayoutUtil::layoutRight(ptrLbTitle,m_ptrBtnPhoto,6,-1);

		CCSprite* ptrChipIcon = CREATE_SPRITE(this,"assets_chip_icon.png",false);
		ptrChipIcon->setScale(0.665f);
		m_ptrBg->addChild(ptrChipIcon);
		LayoutUtil::layoutRight(ptrChipIcon,m_ptrBtnPhoto,6,-24);
		CCLabelTTF* ptrChipLabel = CCLabelTTF::create(
			CCString::createWithFormat("%s",Utils::num2strF(m_playerInfo.getChip(),11).c_str())->getCString(),
			FontConstant::TTF_IMPACT,20);
		m_ptrBg->addChild(ptrChipLabel);
		LayoutUtil::layoutRight(ptrChipLabel,ptrChipIcon,5);

		if (m_playerInfo.getAccountID() == GameUtils::s_myInfo.getAccountID())
		{
			INT64 chipsNum = GameUtils::s_myInfo.getChip();
			if (TableLayer::s_tableType == TABLE_TYPE_NORMAL)
			{
				chipsNum += m_ptrTableLayer->getPlayer(m_playerIndex)->m_chipsInGame;
			}
			ptrChipLabel->setString(CCString::createWithFormat("%s",
				Utils::num2strF(chipsNum,11).c_str())->getCString());
			LayoutUtil::layoutRight(ptrChipLabel,ptrChipIcon,5);

			CCSprite* ptrDiamondIcon = CREATE_SPRITE(this,"assets_diamond_icon.png",false);
			ptrDiamondIcon->setScale(0.665f);
			m_ptrBg->addChild(ptrDiamondIcon);
			LayoutUtil::layoutBottom(ptrDiamondIcon,ptrChipIcon,0,-2);
			CCLabelTTF* ptrDiamondLabel = CCLabelTTF::create(
				CCString::createWithFormat("%s",Utils::num2strF(GameUtils::s_myInfo.getDiamond(),11).c_str())->getCString(),
				FontConstant::TTF_IMPACT,20);
			m_ptrBg->addChild(ptrDiamondLabel);
			LayoutUtil::layoutRight(ptrDiamondLabel,ptrDiamondIcon,5);
		}
	}

	// Bg
	CCLayerColor* ptrInfoBg = CCLayerColor::create(ccc4(0,200,200,0.2 * 255));
	ptrInfoBg->setAnchorPoint(ccp(0,0));
	ptrInfoBg->setContentSize(CCSizeMake(270,179));
	m_ptrBg->addChild(ptrInfoBg);
	LayoutUtil::layoutParentTop(ptrInfoBg, 0, -147);

	// Name Label
	m_ptrLbName = CCLabelTTF::create(LOCALIZED_STRING("name_label"), 
		FontConstant::TTF_IMPACT, 22);
	m_ptrLbName->setAnchorPoint(ccp(0,0.5));
	m_ptrLbName->setDimensions(CCSizeMake(250,m_ptrLbName->getContentSize().height));
	m_ptrLbName->setHorizontalAlignment(kCCTextAlignmentLeft);
	ptrInfoBg->addChild(m_ptrLbName);
	LayoutUtil::layoutParentLeftTop(m_ptrLbName, 13, -9);

	// Level Label
	m_ptrLbLevel = CCLabelTTF::create(CCString::createWithFormat(
		LOCALIZED_STRING("level_with_format"), 0)->getCString(), 
		FontConstant::TTF_IMPACT, 22);
	m_ptrLbLevel->setAnchorPoint(ccp(0,0.5));
	m_ptrLbLevel->setColor(ccc3(0,255,255));
	ptrInfoBg->addChild(m_ptrLbLevel);
	LayoutUtil::layoutParentLeftTop(m_ptrLbLevel, 13, -40);

	// Best Suite Label
	CCLabelTTF* ptrLbBestHand = CCLabelTTF::create(LOCALIZED_STRING("best_hand"), 
		FontConstant::TTF_IMPACT, 22);
	ptrInfoBg->addChild(ptrLbBestHand);
	LayoutUtil::layoutParentLeftTop(ptrLbBestHand, 13, -71);

	// card
	for(int i = 0; i < 5; i++)
	{
		m_ptrCard[i] = CREATE_SPRITE(this, "card_00.png", false);
		m_ptrCard[i]->setScale(0.511f);
		ptrInfoBg->addChild(m_ptrCard[i]);
		LayoutUtil::layoutParentLeftTop(m_ptrCard[i], 14 + 50 * i, -107);
	}
}

void PlayerInfoLayer::initBtns()
{
	// Close Button
	CCMenuItem* ptrBtnClose = MenuItemImage::create("btn_close.png","btn_close_down.png",
		this,menu_selector(PlayerInfoLayer::btnCloseCallback));
	m_ptrBg->addChild(ptrBtnClose);
	registerMenuItem(ptrBtnClose);
	LayoutUtil::layoutParentRightTop(ptrBtnClose,-18,-18);

	if ((!MainMenuLayer::isFriend(m_playerInfo.getAccountID()))
		&& m_playerInfo.getAccountID() != GameUtils::s_myInfo.getAccountID())
	{
		// Add Friend Button
		CCMenuItem* ptrBtnAddFriend = MenuItemImage::create("btn_add_friend.png",
			"btn_add_friend_down.png",this,menu_selector(PlayerInfoLayer::btnAddFriendCallback));
		m_ptrBg->addChild(ptrBtnAddFriend);
		registerMenuItem(ptrBtnAddFriend);
		LayoutUtil::layoutParentLeftTop(ptrBtnAddFriend, 145, -90);
	}

	// Ban Button
	MenuItemImage* ptrBtnBan;
	// judge has added to gag set  如果是自己，enable设置为false
	if(m_playerInfo.getAccountID() == GameUtils::s_myInfo.getAccountID()
		|| !m_ptrTableLayer->isSitDown)
	{
		ptrBtnBan = MenuItemImage::create("btn_report.png","btn_report_down.png",
			"btn_report.png",this,menu_selector(PlayerInfoLayer::btnReportCallback));
		((CCSprite*)(ptrBtnBan->getDisabledImage()))->setOpacity(0.4 * 255);
		ptrBtnBan->setEnabled(false);
	}
	else if (m_ptrTableLayer->isInGagSet(m_playerInfo.getAccountID()))
	{
		ptrBtnBan = MenuItemImage::create("btn_report_down.png","btn_report_down.png",
			"btn_report_down.png",this,menu_selector(PlayerInfoLayer::btnReportCallback));
	}
	else
	{
		ptrBtnBan = MenuItemImage::create("btn_report.png","btn_report_down.png",
			"btn_report_down.png",this,menu_selector(PlayerInfoLayer::btnReportCallback));
	}
	m_ptrBg->addChild(ptrBtnBan);
	registerMenuItem(ptrBtnBan);
	LayoutUtil::layoutParentBottom(ptrBtnBan, 0, 187);

	if (!m_ptrTableLayer->isSitDown)
	{
		// Send a Gift Button
		m_ptrBtnSendGift = MenuItemImage::create("btn_send_gift.png",
			"btn_send_gift_down.png", "btn_send_gift.png", 
			this,menu_selector(PlayerInfoLayer::btnSendGiftCallback));
		((CCSprite*)(m_ptrBtnSendGift->getDisabledImage()))->setOpacity(0.4 * 255);
		m_ptrBtnSendGift->setEnabled(false);

		// Send Everyone Button
		m_ptrBtnSendEvery = MenuItemImage::create("btn_send_everyone.png",
			"btn_send_everyone_down.png", "btn_send_everyone.png", 
			this,menu_selector(PlayerInfoLayer::btnSendEveryoneCallback));
		((CCSprite*)(m_ptrBtnSendEvery->getDisabledImage()))->setOpacity(0.4 * 255);
		m_ptrBtnSendEvery->setEnabled(false);
	}
	else
	{
		// Send a Gift Button
		m_ptrBtnSendGift = MenuItemImage::create("btn_send_gift.png",
			"btn_send_gift_down.png", "btn_send_gift_down.png", 
			this,menu_selector(PlayerInfoLayer::btnSendGiftCallback));

		// Send Everyone Button
		m_ptrBtnSendEvery = MenuItemImage::create("btn_send_everyone.png",
			"btn_send_everyone_down.png", "btn_send_everyone_down.png", 
			this,menu_selector(PlayerInfoLayer::btnSendEveryoneCallback));
	}
	m_ptrBg->addChild(m_ptrBtnSendGift);
	registerMenuItem(m_ptrBtnSendGift);
	LayoutUtil::layoutParentBottom(m_ptrBtnSendGift, 0, 103);

	m_ptrBg->addChild(m_ptrBtnSendEvery);
	registerMenuItem(m_ptrBtnSendEvery);
	LayoutUtil::layoutParentBottom(m_ptrBtnSendEvery, 0, 20);
}

void PlayerInfoLayer::initSendGiftListLayer( bool isSendEveryone )
{
	if(m_ptrLbSendGiftTitle)
	{
		if (isSendEveryone)
		{
			m_ptrLbSendGiftTitle->setString(LOCALIZED_STRING("title_send_gifts"));
		}
		else
		{
			m_ptrLbSendGiftTitle->setString(LOCALIZED_STRING("title_send_a_gift"));
		}
		return;
	}

	// Bg
	m_ptrSendGiftListBG = GameUtils::createScale9("popup_bg_1.png",
		CCRectMake(0,0,100,100),CCRectMake(40,40,20,20));
	m_ptrSendGiftListBG->setContentSize(CCSizeMake(436, 615));
	addChild(m_ptrSendGiftListBG);
	LayoutUtil::layoutRight(m_ptrSendGiftListBG,m_ptrBg,-14);

	// Send a gift Label
	string title = LOCALIZED_STRING("title_send_a_gift");
	if (isSendEveryone)
	{
		title = LOCALIZED_STRING("title_send_gifts");
	}
	m_ptrLbSendGiftTitle = CCLabelBMFont::create(title.c_str(), 
		FontConstant::BMFONT_GOTHIC725_BLK_BT_BLUE);
	m_ptrLbSendGiftTitle->setScale(0.62);
	m_ptrSendGiftListBG->addChild(m_ptrLbSendGiftTitle);
	LayoutUtil::layoutParentTop(m_ptrLbSendGiftTitle, 0, -12);

	SendGiftListLayer* ptrSendGiftListLayer = SendGiftListLayer::create();
	ptrSendGiftListLayer->setSelectors(this, menu_selector(PlayerInfoLayer::btnClickGiftCallback));
	m_ptrSendGiftListBG->addChild(ptrSendGiftListLayer);
	LayoutUtil::layoutParentCenter(ptrSendGiftListLayer,0,-4);

	CCSprite* ptrTopLine = CREATE_SPRITE(this,"Images/separator_line_thin.png",false);
	ptrTopLine->setScaleX(0.397f);
	m_ptrSendGiftListBG->addChild(ptrTopLine);
	LayoutUtil::layoutTop(ptrTopLine,ptrSendGiftListLayer,0,-17);

	CCSprite* ptrBottomLine = CREATE_SPRITE(this,"Images/separator_line_thin.png",false);
	ptrBottomLine->setScaleX(0.397f);
	m_ptrSendGiftListBG->addChild(ptrBottomLine);
	LayoutUtil::layoutBottom(ptrBottomLine,ptrSendGiftListLayer,0,17);
}

void PlayerInfoLayer::btnCloseCallback(CCObject* pSender)
{
	SoundPlayer::play(SNDI_CLICK);
	animateHide();
}

void PlayerInfoLayer::revertIsLayerSwitching()
{
	m_bIsLayerSwitching = !m_bIsLayerSwitching;
}

void PlayerInfoLayer::btnAddFriendCallback(CCObject* pSender)
{
	SoundPlayer::play(SNDI_CLICK);
	if(m_bIsLayerSwitching)
	{
		return;
	}
	string msg = "";
	if (MainMenuLayer::isFriend(m_playerInfo.getAccountID()))
	{
		msg = "reminder_already_added";
	}
	else if (m_playerInfo.getAccountID() == GameUtils::s_myInfo.getAccountID())
	{
		msg = "reminder_cant_add_self";
	}
	if (msg != "")
	{
		showMsgBox(msg.c_str());
		return;
	}

	PokerAddFriendCmd* ptrAddFriendCmd = CmdSender::getInstance()->getPokerAddFriendCmd(m_playerInfo.getAccountID().c_str());
	ptrAddFriendCmd->m_needShowWaitingPanel = false;
	CCNotificationCenter::sharedNotificationCenter()->addObserver(this, 
		callfuncO_selector(PlayerInfoLayer::onAddFriendNetworkCallback), 
		ptrAddFriendCmd->m_cmdName.c_str(), NULL);
	CmdSender::getInstance()->sendCommands(1, ptrAddFriendCmd);
}

void PlayerInfoLayer::btnReportCallback(CCObject* pSender)
{
	SoundPlayer::play(SNDI_CLICK);
	if(m_bIsLayerSwitching)
	{
		return;
	}
	CCLOG("PlayerInfoLayer::onBtnReportCallback");
	// judge has added to gag set
	MenuItemImage* ptrBtnBan = (MenuItemImage*)pSender;
	if(!m_ptrTableLayer->isInGagSet(m_playerInfo.getAccountID()))
	{
		m_ptrTableLayer->addGagByAccountId(m_playerInfo.getAccountID(),m_playerIndex);
		ptrBtnBan->setNormalImage(CREATE_SPRITE(this,"btn_report_down.png",false));
	}
	else
	{
		m_ptrTableLayer->deleteGagByAccountId(m_playerInfo.getAccountID(),m_playerIndex);
		ptrBtnBan->setNormalImage(CREATE_SPRITE(this,"btn_report.png",false));
	}
	refreshPhoto();
}

void PlayerInfoLayer::btnSendGiftCallback(CCObject* pSender)
{
	if(m_bIsLayerSwitching)
	{
		return;
	}
	if (!canISendGift())
	{
		animateHide();
		return;
	}

	SoundPlayer::play(SNDI_CLICK);
	m_ptrBtnSendGift->setEnabled(false);
	m_ptrBtnSendEvery->setEnabled(true);
	m_sendGiftToWho = m_playerIndex;
	initSendGiftListLayer(false);
}

void PlayerInfoLayer::btnSendEveryoneCallback(CCObject* pSender)
{
	if(m_bIsLayerSwitching)
	{
		return;
	}
	if (!canISendGift())
	{
		animateHide();
		return;
	}

	SoundPlayer::play(SNDI_CLICK);
	m_ptrBtnSendGift->setEnabled(true);
	m_ptrBtnSendEvery->setEnabled(false);
	m_sendGiftToWho = 9;//9 means everyone
	initSendGiftListLayer(true);
}

void PlayerInfoLayer::btnClickGiftCallback(CCObject* pSender)
{
	if(m_bIsLayerSwitching)
	{
		return;
	}
	if (!canISendGift())
	{
		animateHide();
		return;
	}

	SoundPlayer::play(SNDI_CLICK);
	CCNode* ptrMenuItem = dynamic_cast<CCNode*>(pSender);
	if(ptrMenuItem)
	{
		m_ptrTableLayer->sendGiftCmd(CCString::createWithFormat("%d%02d%d",m_sendGiftToWho,ptrMenuItem->getTag(),
			m_sendGiftToWho == 9 ? m_ptrTableLayer->getPlayerNum():1)->getCString());
	}
	animateHide();
}

void PlayerInfoLayer::btnPhotoCallback(CCObject* pSender)
{
	SoundPlayer::play(SNDI_CLICK);
	BigPhotoLayer* ptrBigPhotoLayer = BigPhotoLayer::create(m_playerInfo);
	getParent()->addChild(ptrBigPhotoLayer,TableLayer::Z_MESSAGE_BOX);
	LayoutUtil::layoutParentCenter(ptrBigPhotoLayer);
}

void PlayerInfoLayer::refreshValue( PlayerInfo playerInfo )
{
	m_playerInfo = playerInfo;
	if (m_playerInfo.getAccountID() == GameUtils::s_myInfo.getAccountID())
	{
		m_playerInfo.setLevel(GameUtils::s_myInfo.getLevel());
	}
	string name = GameUtils::cutName(m_playerInfo.getNickName(),15);
	m_ptrLbName->setString((LOCALIZED_STRING("name_label") + name).c_str());
	m_ptrLbLevel->setString(CCString::createWithFormat(
		LOCALIZED_STRING("level_with_format"),m_playerInfo.getLevel())->getCString());
	const int *bestHand = m_playerInfo.getBestHand();
	for(int i = 0; i < 5; i++)
	{
		CCNode* ptrParent = m_ptrCard[i]->getParent();
		float scale = m_ptrCard[i]->getScale();
		CCPoint postion = m_ptrCard[i]->getPosition();
		m_ptrCard[i]->removeFromParent();
		m_ptrCard[i] = CREATE_SPRITE(this, 
			CCString::createWithFormat("card_%02d.png",bestHand[i])->getCString(), false);
		m_ptrCard[i]->setScale(scale);
		ptrParent->addChild(m_ptrCard[i]);
		m_ptrCard[i]->setPosition(postion);
	}
}

void PlayerInfoLayer::animateToShow(CCPoint startPoint)
{
	if(m_bIsLayerSwitching)
	{
		return;
	}
	m_bIsLayerSwitching = true;
	CCPoint endPoint = m_ptrBg->getPosition();
	m_ptrBg->setPosition(startPoint);
	m_startPoint = startPoint;
	m_ptrBg->setScale(0.1f);
	CCAction* ptrAct = CCSequence::create(
		CCSpawn::create(
			CCEaseSineIn::create(CCScaleTo::create(0.3f, 1.0f)),
			CCEaseSineIn::create(CCMoveTo::create(0.3f, endPoint)),
			NULL),
		CCCallFunc::create(this, callfunc_selector(PlayerInfoLayer::revertIsLayerSwitching)),
		NULL);
	m_ptrBg->runAction(ptrAct);
}

void PlayerInfoLayer::refreshPhoto()
{
	CCLOG("PlayerInfoLayer::refreshPhoto");
	string photoPath;
	if (m_ptrTableLayer->isInGagSet(m_playerInfo.getAccountID()))
	{
		photoPath = "ban.png";
	}
	else
	{
		m_playerInfo.getPhotoPath(photoPath);
	}
	CCSprite* p_img = CREATE_SPRITE(this,photoPath.c_str(),false);
	m_ptrPhoto->setContent(p_img);
}

void PlayerInfoLayer::onAddFriendNetworkCallback(CCObject* pSender)
{
	PokerAddFriendCmd* ptrAddFriendCmd = (PokerAddFriendCmd*) pSender;
	int errorNo = ptrAddFriendCmd->m_ptrOutput->errorInfo.errorNO;

	string msg = "";
	if(errorNo == RespHandler::OK)
	{
		// Requested
		msg = LOCALIZED_STRING("reminder_requested");
	}
	else if(errorNo == RespHandler::OP_DENIED)
	{
		msg = LOCALIZED_STRING("reminder_cant_add_self");
	}
	else if(errorNo == RespHandler::USER_NOT_FOUND)
	{
		msg = LOCALIZED_STRING("reminder_player_not_found");
	}
	else if (errorNo == RespHandler::ALREADY_ADDED)
	{
		msg = LOCALIZED_STRING("reminder_already_added");
	}
	else if (errorNo == RespHandler::ALREADY_REQUESTED)
	{
		msg = LOCALIZED_STRING("reminder_already_requested");
	}

	if (msg != "")
	{
		showMsgBox(msg.c_str());
		RespHandler::getInstance()->setLastErrorNO(RespHandler::OK);
		CCNotificationCenter::sharedNotificationCenter()->removeObserver(this,
			ptrAddFriendCmd->m_cmdName.c_str());
		CC_SAFE_DELETE(ptrAddFriendCmd);
	}
}

void PlayerInfoLayer::animateHide()
{
	if(m_bIsLayerSwitching)
	{
		return;
	}
	m_bIsLayerSwitching = true;
	CCAction* ptrAct = CCSequence::create(
		CCSpawn::create(
			CCEaseSineIn::create(CCScaleTo::create(0.3f, 0.1f)),
			CCEaseSineIn::create(CCMoveTo::create(0.3f, m_startPoint)),
			NULL),
		CCCallFunc::create(this,callfunc_selector(PlayerInfoLayer::removeFromParent)),
		NULL);
	runAction(ptrAct);
}

bool PlayerInfoLayer::canISendGift()
{
	if (!m_ptrTableLayer->isSitDown)
	{
		showMsgBox(LOCALIZED_STRING("reminder_send_gift"));	
		return false;
	}
	return true;
}

void PlayerInfoLayer::onExit()
{
	m_ptrTableLayer->m_ptrPlayerInfoLayer = NULL;
	CCNotificationCenter::sharedNotificationCenter()->removeAllObservers(this);
	Layer::onExit();
}

bool PlayerInfoLayer::onKeyBackClicked()
{
	SoundPlayer::play(SNDI_CLICK);
	animateHide();
	return true;
}

bool PlayerInfoLayer::ccTouchBegan(cocos2d::CCTouch* pTouch, cocos2d::CCEvent* pEvent)
{
	if (Layer::ccTouchBegan(pTouch, pEvent))
	{
		if (!(m_ptrBg->boundingBox().containsPoint(pTouch->getLocation())
			|| (m_ptrSendGiftListBG && m_ptrSendGiftListBG->boundingBox().containsPoint(pTouch->getLocation()))))
		{
			animateHide();
		}
	}
	return true;
}

void PlayerInfoLayer::showMsgBox( const char* msg )
{
	PKMessageBox* ptrMessageBox = PKMessageBox::create(this, 
		PKMsgTypeOk,msg, NULL, NULL);
	m_ptrTableLayer->addChild(ptrMessageBox, TableLayer::Z_MESSAGE_BOX);
	LayoutUtil::layoutParentCenter(ptrMessageBox);
}

//BigPhotoLayer

BigPhotoLayer::BigPhotoLayer( PlayerInfo playerInfo ):
	m_playerInfo(playerInfo)
{}

BigPhotoLayer* BigPhotoLayer::create( PlayerInfo playerInfo )
{
	BigPhotoLayer* pRet = new BigPhotoLayer(playerInfo);
	if (pRet && pRet->init())
	{
		pRet->autorelease();
		return pRet;
	}
	CC_SAFE_DELETE(pRet);
	return NULL;
}

bool BigPhotoLayer::init()
{
	if (!Layer::init())
	{
		return false;
	}

	initWithColor(ccc4(0,0,0,255 * 0.5));
	CCScale9Sprite* ptrPhotoBg = GameUtils::createScale9("dark_blue_bg.png",
		CCRectMake(0,0,154,200),CCRectMake(50,50,50,90));
	ptrPhotoBg->setContentSize(CCSizeMake(310,310));
	addChild(ptrPhotoBg);
	LayoutUtil::layoutParentCenter(ptrPhotoBg);

	string strPath;
	m_playerInfo.getPhotoPath(strPath);
	CCSprite* ptrPhoto = CREATE_SPRITE(this,strPath.c_str(),false);
	ptrPhoto->setScaleX(236.0 / ptrPhoto->getContentSize().width);
	ptrPhoto->setScaleY(236.0 / ptrPhoto->getContentSize().height);
	ptrPhotoBg->addChild(ptrPhoto);
	LayoutUtil::layoutParentCenter(ptrPhoto,-1);

	CCLabelTTF* ptrLbName = CCLabelTTF::create(GameUtils::cutName(m_playerInfo.getNickName(),30).c_str(), 
		FontConstant::TTF_IMPACT, 40);
	ptrLbName->setDimensions(CCSizeMake(400,ptrLbName->getContentSize().height));
	ptrLbName->setHorizontalAlignment(kCCTextAlignmentCenter);
	addChild(ptrLbName);
	LayoutUtil::layoutCenter(ptrLbName,ptrPhotoBg,0,-155);

	return true;
}

bool BigPhotoLayer::ccTouchBegan( cocos2d::CCTouch* pTouch, cocos2d::CCEvent* pEvent )
{
	Layer::ccTouchBegan(pTouch,pEvent);
	runAction(CCCallFunc::create(this,callfunc_selector(CCNode::removeFromParent)));
	return true;
}
