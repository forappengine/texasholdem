#ifndef __PLAYER5_H__
#define __PLAYER5_H__

#include "cocos2d.h"
#include "Layer.h"
#include "Player.h"

USING_NS_CC;

class Player5 : public Player
{
public:
	virtual void layoutChildren();
	static Player5* create(TableLayer* ptrTableLayer, int seat, const JSONNode& node);
};

#endif 