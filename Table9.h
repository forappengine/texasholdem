#ifndef __TABLE9_H__
#define __TABLE9_H__

#include "cocos2d.h"
#include "Layer.h"
#include "TableLayer.h"

USING_NS_CC;

class Table9 : public TableLayer{
public:
	virtual Player* createAndAddPlayer(GameScreen* gameScreen,const JSONNode& node);
	virtual void layoutSitDownBtn(int index);
	virtual void onEnter();
	virtual void layoutPlayer(int index);
	CREATE_FUNC(Table9);
};

#endif 