#ifndef __BET_CONTROLER__
#define __BET_CONTROLER__

#include "GameScreen.h"
#include "JsonHelper.h"
#include "Layer.h"
#include "SidePool.h"
#include "BetSlider.h"
#include "GameConstants.h"
#include "cocos-ext.h"

USING_NS_CC_EXT;

class BetControler : public Layer {
public:
	void updateButtons(GameScreen* gameScreen,const JSONNode& node,float delayTime=0);
	void clear();
	virtual void onEnter();
	virtual void onExit();
    virtual bool init();
    CREATE_FUNC(BetControler);
	void hideAllBtns();
	GameScreen* getGameScreen();
	void hideBetSlider();

	int actionID;
	INT64 raiseData;
	BetSlider *m_slider;

private:
	void selectButtonsCallback( CCObject* pSender );
	void radioButtonsCallback( CCObject* pSender );
	CCMenuItemSprite* createSelectButton(const char* normal,const char* selected,const char* disabled=NULL);
	CCMenuItemSprite* createRadioButton(const char* label,const char* normal,const char* selected,const char* disabled=NULL,const char* disabledLabel=NULL);
	void createSliderControler();
	void focusButton( int index);
	void unFocusButton( int index);
	void clearButton();

	int preSelection;
	static const int LAYER_WIDTH=460;
	static const int LAYER_HEIGHT=130;
	CCMenuItemSprite* m_foldButton;
	CCMenuItemSprite* m_checkButton;
	CCMenuItemSprite* m_callButton;
	CCMenuItemSprite* m_allinButton;
	CCMenuItemSprite* m_raiseButton;
	CCMenuItemSprite* m_okButton;
	CCMenuItemSprite* m_checkFoldRadioButton;
	CCMenuItemSprite* m_checkRadioButton;
	CCMenuItemSprite* m_callRadioButton;
	CCMenuItemSprite* m_allinRadioButton;
	CCMenuItemSprite* m_callanyRadioButton;
	CCLabelTTF *m_chipsLabel;
	GameScreen* gameScreen;
	CCLabelTTF* m_ptrLbCallAmount;
	CCProgressTimer* m_ptrCheckTimer;
	CCProgressTimer* m_ptrFoldTimer;
};

#endif 
