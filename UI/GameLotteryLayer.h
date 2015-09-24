#ifndef __GAME_LOTTERY_LAYER_H__
#define __GAME_LOTTERY_LAYER_H__

#include "Layer.h"

class MenuItemImage;

USING_NS_CC;

class GameLotteryLayer : public Layer
{
public:
	CREATE_FUNC(GameLotteryLayer);
	GameLotteryLayer();
	virtual ~GameLotteryLayer();
	virtual bool init();
	virtual void onEnter();
	virtual bool onKeyBackClicked();
	virtual bool ccTouchBegan(cocos2d::CCTouch* pTouch, cocos2d::CCEvent* pEvent);

private:
	void initUI();
	void initCircleContent();
	void initBackBtn();
	void initBtnGo();
	void chargeNewDay();

	void animateToShow();
	void animateToExit();
	void onAnimateEnd();

	void onBtnMinusCallback(CCObject* pSender);
	void onBtnPlusCallback(CCObject* pSender);
	void onBtnGoCallback(CCObject* pSender);
	void onBtnBackCallback(CCObject* pSender);
	void onBtnRewardCallback(CCObject* pSender);
	void onGetLotteryNetworkCallback(CCObject* pSender);
	void onCircleStop();

	void selectBtnByIndex(int index);
	void setGoResult(int);
	void setIsShowing(bool);
	void animateToShowHint();
	void removeHint(float);
	void enableBtns();
	void disableBtns();
	int decideDefaultSelect();

private:
	static const int DEFAULT_SELECTED_INDEX;
	static const int BACKGROUND_MOVE_HEIGHT = 345;
	static const int DIAMOND_BTN_COUNT = 6;
	static const int MUlTIPLE_COUNT[DIAMOND_BTN_COUNT+1];
	static const int CIRCLE_CONTENT_COUNT = 8;
	static const int CircleContent[CIRCLE_CONTENT_COUNT];
	static const int ChipAmountPositionOffsetX[CIRCLE_CONTENT_COUNT];
	static const int ChipAmountPositionOffsetY[CIRCLE_CONTENT_COUNT];
	static const int CircleHeightLightRotation[CIRCLE_CONTENT_COUNT];
	static const int CircleHeightLightPositionX[CIRCLE_CONTENT_COUNT];
	static const int CircleHeightLightPositionY[CIRCLE_CONTENT_COUNT];
	static const int LotteryProbability[CIRCLE_CONTENT_COUNT];

	CCSprite* m_ptrBg;
	CCSprite* m_ptrCircle;
	CCMenuItemSprite* m_ptrMenuGo;
	CCMenuItem* m_ptrMenuBack;
	CCMenuItemSprite* m_ptrMenuHint;
	MenuItemImage* m_ptrBtnMinus;
	MenuItemImage* m_ptrBtnPlus;
	CCLabelTTF* m_ptrLbMutilply[CIRCLE_CONTENT_COUNT];
	CCLabelTTF* m_ptrLbBtnGoDiamond[3];
	bool m_bIsRunning;
	int m_iCurLottery;
	int m_iChoiceDiamond;
	bool m_bIsLotteryNormally;
	bool m_bIsFree;
	bool m_bIsShowing;
	bool m_bTempSoundOn;

private:
	enum backBtnPic
	{
		ARROW_UP = 0,
		ARROW_DOWN
	};
	enum lotteryResultType
	{
		DIAMOND_NOT_ENOUGH,
		WIN_CHIPS
	};
};

#endif//!__GAME_LOTTERY_LAYER_H__