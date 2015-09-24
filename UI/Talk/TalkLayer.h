#ifndef __TALK_LAYER_H__
#define __TALK_LAYER_H__

#include "Layer.h"
#include "cocos-ext.h"

class SendMailInputLayer;
class CommonUseListLayer;

USING_NS_CC;
USING_NS_CC_EXT;

struct Talk
{
	string name;
	string content;
};

class TalkLayer : public Layer, public CCScrollViewDelegate, public CCEditBoxDelegate
{
public:
	CREATE_FUNC(TalkLayer);
	void updateTalkContent();
	static void pushNewMsg(string name, string content);
	static void clearNewMsg();
	void animateShow();
	virtual bool init();
	virtual void onExit();
	virtual bool onKeyBackClicked();
	virtual bool ccTouchBegan(cocos2d::CCTouch* pTouch, cocos2d::CCEvent* pEvent);
	virtual void ccTouchMoved(cocos2d::CCTouch* pTouch, cocos2d::CCEvent* pEvent);
	virtual void ccTouchEnded(cocos2d::CCTouch* pTouch, cocos2d::CCEvent* pEvent);
	virtual void ccTouchCancelled(cocos2d::CCTouch* pTouch, cocos2d::CCEvent* pEvent);

	// CCScrollViewDelegate
	virtual void scrollViewDidScroll(CCScrollView* view);
	virtual void scrollViewDidZoom(CCScrollView* view);

private:
	void initScrollView();
	void initCommonUseList();
	void onBtnCommonUseListCallback(CCObject* pSender);
	void animateHide();
	virtual void editBoxReturn( CCEditBox* editBox );

	CCScale9Sprite* m_ptrBG;
	SendMailInputLayer* m_ptrInput;
	static std::vector<Talk> s_talkVec;
	std::vector<const char*> m_commonUseVec;
	CommonUseListLayer* m_ptrCommonUseList;
	static const int TALK_COMMON_USE_COUNT = 18;
	CCScrollView* m_ptrTalkList;
};
#endif