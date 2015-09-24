#ifndef __SEND_GIFT_LIST_LAYER_H__
#define __SEND_GIFT_LIST_LAYER_H__

#include "VerticalButtonTableView.h"

class SendGiftListLayer : public VerticalButtonTableView
{
public:
	CREATE_FUNC(SendGiftListLayer);
	virtual bool init();
	virtual void initParams();
	void setSelectors(Layer* parent,SEL_MenuHandler btnSelector);
	virtual void setVisible(bool visible);

protected:
	void createButtons();

private:
	Layer* m_ptrParent;
	SEL_MenuHandler m_pfnBtnSelector;
	static const int MAX_GIFT_LAYER = 3;
};
#endif