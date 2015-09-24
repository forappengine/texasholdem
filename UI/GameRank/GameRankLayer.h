#ifndef __GAME_RANK_LAYER_H__
#define __GAME_RANK_LAYER_H__

#include "Layer.h"
#include "GameRankListLayer.h"

USING_NS_CC;

class GameRankLayer : public Layer
{
public:
	GameRankLayer();
	CREATE_FUNC(GameRankLayer);
	virtual bool init();
	virtual void onEnter();
	bool onKeyBackClicked();
	virtual bool ccTouchBegan(cocos2d::CCTouch* pTouch, cocos2d::CCEvent* pEvent);

private:
	void initBG();
	void initTabs();
	void initBtnBack();

	void btnBackCallback(CCObject* pSender);
	void tabFriendsCallback(CCObject* pSender);
	void tabGlobalCallback(CCObject* pSender);
	void btnJoinCallback(CCObject* pSender);

	void refreshGameRankList(bool isToShowFriends);
	void animateToShow();
	void animateToHide();
	void onAnimateEnd();

private:
	bool m_bIsLayerSwitching;
	CCScale9Sprite* m_ptrBg;
	CCMenuItem* m_ptrTabFriends;
	CCLabelBMFont* m_ptrLbFriends;
	CCMenuItem* m_ptrTabGolbal;
	CCLabelBMFont* m_ptrLbGlobal;
	CCMenuItemSprite* m_ptrBtnBack;
	GameRankListLayer* m_ptrGameRankList;
	bool m_bNeedRemoveFromParent;
};
#endif