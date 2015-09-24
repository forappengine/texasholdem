#ifndef __BROAD_SEND_GIFT_BUTTON_H__
#define __BROAD_SEND_GIFT_BUTTON_H__

#include "cocos2d.h"

USING_NS_CC;
using namespace std;

class BroadSendGiftButton : public CCMenuItemSprite
{
	BroadSendGiftButton(CCSize cellSize);
	void init(CCLayer* parentLayer);

private:
	CCLayerColor* m_ptrNormal;
	CCLayerColor* m_ptrSelected;
};

#endif