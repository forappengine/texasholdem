#ifndef __BET_SLIDER_H__
#define __BET_SLIDER_H__

#include "ControlSlider.h"
#include "GameConstants.h"
#include "Layer.h"

class TableLayer;

class BetSlider : public Layer
{
public:
	BetSlider();
	~BetSlider();
	CREATE_FUNC(BetSlider);
	virtual bool init();
	void setValue(float value);
	void updateChipLable(INT64 chips);
	void reInitSlider();

	CCLabelTTF *m_ptrChipsLabel;

private:
	void initBtns();
	void initSlider();
	void sliderValueChangedCallback(CCObject *sender, CCControlEvent controlEvent);
	void allInBtnCallback(CCObject* pSender);
	void addBtnCallback(CCObject* pSender);
	void subBtnCallback(CCObject* pSender);
	TableLayer* getTable();
	INT64 getMinRaise();
	INT64 getMaxRaise();
	virtual bool ccTouchBegan( cocos2d::CCTouch* pTouch, cocos2d::CCEvent* pEvent );
	virtual void ccTouchMoved( cocos2d::CCTouch* pTouch, cocos2d::CCEvent* pEvent );
	virtual void ccTouchEnded( cocos2d::CCTouch* pTouch, cocos2d::CCEvent* pEvent );
	virtual void ccTouchCancelled( cocos2d::CCTouch* pTouch, cocos2d::CCEvent* pEvent );

	ControlSlider* m_ptrSlider;
	float m_maxSliderValue;
	float m_minSliderValue;
};

#endif//__BET_SLIDER_H__