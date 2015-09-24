#ifndef __SIDE_POOL__
#define __SIDE_POOL__

#include "Layer.h"
#include "GameConstants.h"

USING_NS_CC;

class SidePool : public Layer 
{
public:
    CREATE_FUNC(SidePool);
	virtual bool init();
	void updatePool(INT64 count,bool needShowUpdateAction);
	INT64 getBetChipAmount();

private:
	CCSprite* m_ptrIconChip;
	CCLabelTTF* m_ptrLbBetChipAmount;
	CCSprite* m_ptrBetLabelBG;
	CCSpriteFrame* m_ptrChipImages[17];
	static const int LAYER_WIDTH = 260;
	static const int LAYER_HEIGHT = 100;
	INT64 m_betChipAmount;

	void showUpdateActions();
	void updateBetChipAmountLabel();
};
#endif 
