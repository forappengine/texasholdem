#ifndef __COMMON_USE_LIST_LAYER_H__
#define __COMMON_USE_LIST_LAYER_H__

#include "VerticalButtonTableView.h"

class CommonUseListLayer : public VerticalButtonTableView
{
public:
	CREATE_FUNC(CommonUseListLayer);
	virtual bool init();
	virtual void initParams();
	void setSelectors(Layer* parent, vector<const char*>* ptrCommonUseVec, SEL_MenuHandler pfnCommonUseSelector);
	virtual void setVisible(bool visible);

protected:
	void createButtons();

private:
	SEL_MenuHandler m_pfnCommonUseSelector;
	vector<const char*>* m_ptrCommonUseVec;
	Layer* m_ptrParent;
};
#endif