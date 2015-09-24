#ifndef __GAME_SCREEN_H__
#define __GAME_SCREEN_H__

#include "GameConstants.h"
#include "GameSocket.h"
#include "cocos2d.h"
#include "Layer.h"
#include "UI/ParticalDemoLayer.h"

class TableLayer;

USING_NS_CC;
class GameScreen : public Layer 
{
public:
	GameScreen();
	virtual ~GameScreen();
	virtual void onExit();
    virtual bool init();
	void initBg();
	void update(float dt);
    static cocos2d::CCScene* scene();
    CREATE_FUNC(GameScreen);
	void removeLoadingLayer(float dt);
	void lostConnection();
	void backToUI();

private:
	bool isStarted;

	void sendFailToEnterTableCmd(CCObject* pSender);
	void failToEnterTableCmdCallback(CCObject* pSender);
	void showNetProblemMessageBox();

public:
	GameSocket* gameSocket;
	TableLayer *table;
	static string phpSessionID;
	static string host;
	static int port;
	ParticleDemoLayer* m_ptrLoadingLayer;
};

#endif 
