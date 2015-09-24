#ifndef __LOST_CONNECTION_LAYER_H__
#define __LOST_CONNECTION_LAYER_H__

#include "Layer.h"

USING_NS_CC;

class LostConnectionLayer : public Layer
{
public:
	CREATE_FUNC(LostConnectionLayer);
	virtual bool init();

private:
	void initBG();
	void initTitle();
	void initContent();
	void initBtns();
	void btnCancelCallback(CCObject* pSender);
	void btnRetryCallback(CCObject* pSender);
	virtual bool onKeyBackClicked();
	void enterTableNetworkCallback(CCObject* pSender);

};

#endif//__LOST_CONNECTION_LAYER_H__