#ifndef __TABLE5_H__
#define __TABLE5_H__

#include "cocos2d.h"
#include "Layer.h"
#include "TableLayer.h"

USING_NS_CC;

class Table5 : public TableLayer
{
public:
	virtual Player* createAndAddPlayer(GameScreen* gameScreen,const JSONNode& node);
	virtual void layoutSitDownBtn(int index);
	virtual void onEnter();
	virtual void layoutPlayer(int index);
	CREATE_FUNC(Table5);
};

#endif 