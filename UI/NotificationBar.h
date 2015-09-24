#ifndef __NOTIFICATION_BAR_H__
#define __NOTIFICATION_BAR_H__

#include "Layer.h"
#include "cocos2d.h"
#include "cocos-ext.h"

USING_NS_CC;
USING_NS_CC_EXT;

class NotificationBar : public Layer
{
public:
	static NotificationBar* getInstance();
	void setNotificationMsg(string str);
	void animateShow();
	void setAutoHide(float duration);

protected:
	NotificationBar();
	~NotificationBar();
	virtual bool init();
	virtual void initNotificationMsg();
	virtual void initBG();
	void autoHide(float dlt);

	CCLabelTTF* m_ptrLbNotificationMsg;
	CCScale9Sprite* m_ptrBg;

private:
	static NotificationBar* s_ptrInstance;

	void initBtnClose();
	void btnCloseCallback(CCObject* pSender);
};

#endif//__NOTIFICATION_BAR_H__