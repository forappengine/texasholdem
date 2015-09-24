#ifndef __PLAYER_INFO_LAYER_H__
#define __PLAYER_INFO_LAYER_H__

#include "Layer.h"
#include "cocos-ext.h"
#include "PlayerInfo.h"

class Icon;
class TableLayer;
class MenuItemImage;

USING_NS_CC;
USING_NS_CC_EXT;
using namespace std;

class PlayerInfoLayer : public Layer
{
public:
	PlayerInfoLayer(TableLayer* ptrTableLayer,PlayerInfo playerInfo,int index);
	static PlayerInfoLayer* create(TableLayer* ptrTableLayer,PlayerInfo playerInfo,int index);
	void refreshValue(PlayerInfo playerInfo);
	void animateToShow(CCPoint startPoint);
	void refreshPhoto();
	virtual bool init();
	virtual void onExit();
	virtual bool onKeyBackClicked();
	virtual bool ccTouchBegan(cocos2d::CCTouch* pTouch, cocos2d::CCEvent* pEvent);

	PlayerInfo m_playerInfo;

private:
	void initBG();
	void initPhoto();
	void initInfo();
	void initBtns();
	void initSendGiftListLayer(bool isSendEveryone);
	void btnCloseCallback(CCObject* pSender);
	void btnAddFriendCallback(CCObject* pSender);
	void btnReportCallback(CCObject* pSender);
	void btnSendGiftCallback(CCObject* pSender);
	void btnSendEveryoneCallback(CCObject* pSender);
	void btnClickGiftCallback(CCObject* pSender);
	void btnPhotoCallback(CCObject* pSender);
	void animateHide();
	void revertIsLayerSwitching();
	void onAddFriendNetworkCallback(CCObject* pSender);
	bool canISendGift();
	void showMsgBox(const char* msg);

	CCScale9Sprite* m_ptrBg;
	CCScale9Sprite* m_ptrSendGiftListBG;
	Icon* m_ptrPhoto;
	CCMenuItem* m_ptrBtnPhoto;
	CCLabelTTF* m_ptrLbName;
	CCLabelTTF* m_ptrLbLevel;
	CCSprite* m_ptrCard[5];
	MenuItemImage* m_ptrBtnSendGift;
	MenuItemImage* m_ptrBtnSendEvery;
	bool m_bHasInitRight;
	bool m_bIsLayerSwitching;
	CCPoint m_startPoint;
	int m_sendGiftToWho;
	CCLabelBMFont* m_ptrLbSendGiftTitle;
	int m_playerIndex;
	TableLayer* m_ptrTableLayer;
};

//BigPhotoLayer
class BigPhotoLayer : public Layer
{
public:
	BigPhotoLayer(PlayerInfo playerInfo);
	static BigPhotoLayer* create(PlayerInfo playerInfo);
	virtual bool init();
	virtual bool ccTouchBegan(cocos2d::CCTouch* pTouch, cocos2d::CCEvent* pEvent);

private:
	PlayerInfo m_playerInfo;
};
#endif