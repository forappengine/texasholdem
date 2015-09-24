#ifndef __PUBLIC_AREA_LAYER__
#define __PUBLIC_AREA_LAYER__

#include "JsonHelper.h"
#include "Layer.h"
#include "SidePool.h"

enum WIN_CARDS_TYPE
{
	TYPE_HIGH_CARD = 0,
	TYPE_ONE_PAIR,
	TYPE_TWO_PAIRS,
	TYPE_THREE_OF_A_KIND,
	TYPE_STRAIGHT,
	TYPE_FLUSH,
	TYPE_FULL_HOUSE,
	TYPE_FOUR_OF_A_KIND,
	TYPE_STRAIGHT_FLUSH,
	TYPE_ROYAL_FLUSH,
	WIN_CARDS_TYPE_COUNT
};

class PublicAreaLayer : public Layer 
{
public:
	bool updateArea(const JSONNode& node);
	void clear();
    virtual bool init();
    CREATE_FUNC(PublicAreaLayer);
	void highlightWinCards(CCNode* node,void *ptrVoid);
	void showWinCardsType(WIN_CARDS_TYPE type);
	SidePool* getSidePool();
	vector<int> getPublicCardsVec();

private:
	static const int LAYER_WIDTH = 460;
	static const int LAYER_HEIGHT = 115;
	int pclength;
	SidePool* sidePool;
	int m_cardsIndex[5];
	CCSprite* m_cardsImgs[5];
	static const char* WIN_CARDS_TYPE_DESC[WIN_CARDS_TYPE_COUNT];
};
#endif 
