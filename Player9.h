#ifndef __PLAYER9_H__
#define __PLAYER9_H__

#include "cocos2d.h"
#include "Layer.h"
#include "Player.h"

USING_NS_CC;

class Player9 : public Player
{
public:
	virtual void layoutChildren();
	static Player9* create(TableLayer* ptrTableLayer, int seat, const JSONNode& node);
};

#endif 