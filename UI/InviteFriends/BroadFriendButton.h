#ifndef __BROAD_FRIEND_BUTTON_H__
#define __BROAD_FRIEND_BUTTON_H__

#include "cocos2d.h"
#include "FriendsListLayer.h"

USING_NS_CC;

class BroadFriendButton : public CCMenuItemSprite
{
public:
	BroadFriendButton(CCSize cellSize);
	void init(CCLayer* parentLayer, FriendInfoInFriendInvite* ptrFriendInfo);

private:
	string getCountString(int count);

private:
	CCLayerColor* m_ptrNormal;
	CCLayerColor* m_ptrSelected;
};


#endif