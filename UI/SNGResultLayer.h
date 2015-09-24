#ifndef __SNG_RESULT_LAYER_H__
#define __SNG_RESULT_LAYER_H__

#include "Layer.h"
#include "cocos-ext.h"
#include "GameConstants.h"

USING_NS_CC;
USING_NS_CC_EXT;

class SNGResultLayer : public Layer
{
public:
	static SNGResultLayer* create(int rank,INT64 bonus);
	SNGResultLayer();
	~SNGResultLayer();
	virtual bool init(int rank,INT64 bonus);
	void animateShow();

private:
	void initBG();
	void initContent(INT64 bonus);
	void initBtns();
	void btnCloseCallback(CCObject* pSender);
	void btnPlayCallback(CCObject* pSender);
	void btnBuyInCallback(CCObject* pSender);
	void enterSitAndGoCmdNetworkCallback(CCObject* pSender);
	virtual bool onKeyBackClicked();

	CCScale9Sprite* m_ptrBg;
	CCMenuItem* m_ptrBtnBuyIn;
	int m_rank;
};
#endif//__SNG_RESULT_LAYER_H__