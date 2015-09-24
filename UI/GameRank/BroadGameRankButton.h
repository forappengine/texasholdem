#ifndef __BROAD_GAME_RANK_BUTTON_H__
#define __BROAD_GAME_RANK_BUTTON_H__

#include "cocos2d.h"

struct PokerPlayerRankInfo;
class Layer;

USING_NS_CC;

class BroadGameRankButton : public CCMenuItemSprite
{
public:
	BroadGameRankButton(CCSize cellSize);
	void init(Layer* parentLayer, PokerPlayerRankInfo* ptrFriendInfo, int index, bool isShowFriend);

private:
	CCNode* m_ptrNormal;
	CCNode* m_ptrSelected;
};
#endif