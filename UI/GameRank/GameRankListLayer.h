#ifndef __GAME_RANK_LIST_LAYER_H__
#define __GAME_RANK_LIST_LAYER_H__

#include "VerticalButtonTableView.h"
#include "GameConstants.h"

class Layer;

class GameRankListLayer : public VerticalButtonTableView
{
public:
	CREATE_FUNC(GameRankListLayer);
	virtual bool init();
	virtual void initParams();
	void setSelectors(Layer* parent, SEL_MenuHandler btnJoinSelector, bool isFriend);
	virtual void setVisible(bool visible);

protected:
	void createButtons();

private:
	Layer* m_ptrParent;
	const static int VIEW_BTN_ZORDER = 10;
	SEL_MenuHandler m_pfnBtnJoinSelector;
	CCLabelTTF* m_ptrLbEmptyInfo;
	bool m_bIsToShowFriend;
};
#endif