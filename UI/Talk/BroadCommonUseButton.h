#ifndef __BROAD_COMMON_USE_BUTTON_H__
#define __BROAD_COMMON_USE_BUTTON_H__

#include "cocos2d.h"

class Layer;

USING_NS_CC;

class BroadCommonUseButton : public CCMenuItemSprite
{
public:
	BroadCommonUseButton(CCSize cellSize);
	void init(Layer *parentLayer,const char* ptrCommonUseTxt, SEL_MenuHandler menuSelector);

private:
	CCNode* m_ptrNormal;
	CCNode* m_ptrSelected;
};
#endif