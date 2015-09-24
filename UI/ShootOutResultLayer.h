#ifndef __SHOOT_OUT_RESULT_LAYER_H__
#define __SHOOT_OUT_RESULT_LAYER_H__

#include "Layer.h"
#include "cocos-ext.h"
#include "GameConstants.h"

USING_NS_CC;
USING_NS_CC_EXT;

class ShootOutResultLayer : public Layer
{
public:
	static ShootOutResultLayer* create(int rank,INT64 bonus);
	ShootOutResultLayer();
	~ShootOutResultLayer();
	virtual bool init(int rank,INT64 bonus);
	void animateShow();

private:
	void initBG();
	void initContent(INT64 bonus);
	void initBtns();
	void btnCloseCallback(CCObject* pSender);
	void btnPlayCallback(CCObject* pSender);
	void enterShootOutCmdNetworkCallback(CCObject* pSender);
	virtual bool onKeyBackClicked();

	CCScale9Sprite* m_ptrBg;
	int m_rank;
};
#endif