#ifndef __GAME_MORE_MENU_LAYER_H__
#define __GAME_MORE_MENU_LAYER_H__

#include "Layer.h"
#include "cocos2d.h"
#include "cocos-ext.h"

USING_NS_CC;
USING_NS_CC_EXT;

class GameMoreMenuLayer:public Layer
{
public:
	GameMoreMenuLayer();
	virtual ~GameMoreMenuLayer();
	CREATE_FUNC(GameMoreMenuLayer);
	virtual bool init();
	virtual bool ccTouchBegan(cocos2d::CCTouch* pTouch, cocos2d::CCEvent* pEvent);
	virtual bool onKeyBackClicked();
	void hideMenu();
	void showMenu();

	static const int UD_LEAVE = 0xC1;
	static const int UD_SETTINGS = 0xC2;
	static const int UD_HAND_RANKS = 0xC3;
	static const int UD_CHANGE_TABLE = 0xC4;

private:
	void initMenuButtons();
	void onMenuItem(CCObject* pSender);
	void enterTableNetworkCallback(CCObject* pSender);
	void backToUI(CCObject *pSender);
	void showErrorMsg(string msg);
	void revertIsLayerSwitching();
	CCMenuItemSprite* createMenuItem(SEL_MenuHandler selector,void* pUserData);

	CCScale9Sprite* m_ptrMenuBg;
	bool m_bIsLayerSwitching;
};
#endif