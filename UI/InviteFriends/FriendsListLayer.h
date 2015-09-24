#ifndef __FRIENDS_LIST_LAYER_H__
#define __FRIENDS_LIST_LAYER_H__

#include "VerticalButtonTableView.h"

USING_NS_CC;
USING_NS_CC_EXT;

//////////////////////////////////
//just for test
struct FriendInfoInFriendInvite
{
	string name;
	int level;
	int chip;
	int diamond;
	bool canInvited;
	bool isInvited;
};
//////////////////////////////////

class FriendsListLayer : public VerticalButtonTableView
{
public:
	CREATE_FUNC(FriendsListLayer);
	virtual bool init();
	virtual void initParams();
	void onEnter();
	void setSelectors(CCLayer* parent, vector<FriendInfoInFriendInvite>* ptrFriendsInfo, SEL_MenuHandler btnInviteSelector);
	virtual void setVisible(bool visible);

protected:
	void createButtons();

private:
	vector<FriendInfoInFriendInvite>* m_ptrFriendsInfoVec;
	CCLayer* m_ptrParent;
	SEL_MenuHandler m_pfnBtnInviteSelector;
	const static int VIEW_BTN_ZORDER = 10;

};


#endif