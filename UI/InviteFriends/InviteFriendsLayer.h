#ifndef __INVITE_FRIENDS_LAYER_H__
#define __INVITE_FRIENDS_LAYER_H__

#include "Layer.h"
#include "FriendsListLayer.h"

USING_NS_CC;

class InviteFriendsLayer : public Layer
{
public:
	CREATE_FUNC(InviteFriendsLayer);
	virtual bool init();
	virtual void onEnter();
	virtual bool onKeyBackClicked();

private:
	void initLb();
	void initBtn();
	void initFriendsListLayer();
	void onBtnBackCallback(CCObject* pSender);
	void onBtnInviteCallback(CCObject* pSender);
	void lockParent();
	void unlockParentAndCleanup();

private:
	CCSprite* m_ptrBg;
	vector<FriendInfoInFriendInvite> m_friendsInfoVec;
	FriendsListLayer* m_ptrFriendsListLayer;
};


#endif