#ifndef __TABLE_LAYER_H__
#define __TABLE_LAYER_H__

#include "GameConstants.h"
#include "libjson.h"
#include <string>
#include "GameScreen.h"
#include "cocos2d.h"
#include "Layer.h"
#include "Player.h"

class PublicAreaLayer;
class BetControler;
class TalkLayer;
class PlayerInfoLayer;
struct TableInfo;
class PlayerInfoCmd;

USING_NS_CC;
using namespace std;

enum TABLE_TYPE
{
	TABLE_TYPE_NORMAL = 0,
	TABLE_TYPE_SNG,
	TABLE_TYPE_SHOOT_OUT
};

class TableLayer : public Layer {
public:
	TableLayer();
	virtual ~TableLayer();
	virtual void onEnter();
	virtual void onExit();
	virtual Player* createAndAddPlayer(GameScreen* gameScreen,const JSONNode& node)=0;
	virtual void showSitDownBtn(int index);
	void hideSitDownBtn(int index);
	void createSitDownBtn(int index);
	virtual void layoutSitDownBtn(int index)=0;
	void onNotifyCommand(GameScreen* gameScreen,const JSONNode& node);
	void onUserChangeCommand(GameScreen* gameScreen,const JSONNode& node);
	void onMSGCommand(GameScreen* gameScreen,const JSONNode& node);
	void onGiftCommand(GameScreen* gameScreen,const JSONNode& node);
	void onDecisionUpdateCommand(GameScreen* gameScreen,const JSONNode& node);
	void onStartGameCommand(GameScreen* gameScreen,const JSONNode& node);
	void setTable(int capacity,int myPosition,INT64 blindBet,int thinkTime);
	int getCapacity();
	INT64 getMaxRaiseReference();
    virtual bool init();
	Player* getPlayer(int index);
	void setIsRunning(bool isRunning);
	bool getIsRunning();
	virtual void talk(string content, int index);
	void sendTalkMsg(string content);
	void setMyPosition(int myPosition);
	void sendSitDownCmd(INT64 buyIn);
	void backToUI();
	void sendGiftCmd(string code);
	int getPlayerNum();
	void sendGotoTableCmd(string fID);
	bool addGagByAccountId(string accountID,int index);
	bool isInGagSet(string accountID);
	bool deleteGagByAccountId(string accountID,int index);
	void showSNGLayer();
	void showQucikBuyLayer();
	void setBtnLotteryVisible(bool);
	void setBtnRankVisible(bool);
	void setBtnTalkVisible(bool val);
	PublicAreaLayer* getPublicAreaLayer();
	void showPlayerInfoLayer(int index, CCPoint startPoint);
	virtual void layoutPlayer(int index) = 0;
	CCPoint getLilyPostion();
	void showShopLayer();

protected:
	Player** players;
	Player* m_thinker;
	PublicAreaLayer* publicArea;
	int capacity;
	bool m_isRunning;
	CCMenuItemSprite** m_prtBtnSitDown;
	bool m_isGameStarted;
	CCLabelTTF* m_ptrLbHint;
	JSONNode m_startGameNode;
	CCMenuItemSprite* m_ptrBtnLily;
	list<string> m_needGetPhotoIDs;
	set<string> m_gagPlayerIdx;
	CCLabelTTF* m_ptrPlayerLevel;
	CCProgressTimer* m_ptrExpProgress;
	bool m_hasSetRemoveLoadingSchedule;
	set<int> m_winCards;
	CCMenuItem* m_ptrBtnTalk;
	bool m_bIsLotteryShowing;
	bool m_bHasSendSitDownCmd;
	CCMenuItem *m_ptrBtnLottery;
	CCMenuItem *m_ptrBtnSpeicalOffer;
	CCLabelTTF *m_ptrLbCountDown;
	CCMenuItem *m_ptrBtnShop;
	CCMenuItem *m_ptrBtnRank;
	INT64 m_tempChipsInGame;
	vector<pair<int,vector<INT64> > > m_tempWinnerChipsVec;

public:
	INT64 blindBet;
	int thinkTime;
	Player* mySelf;
	int myPosition;
	bool isSitDown;
	INT64 maxBet;
	BetControler* betCtrl;
	PlayerInfoLayer* m_ptrPlayerInfoLayer;
	TalkLayer* m_ptrTalkLayer;
	INT64 m_newBuyIn;
	static TableInfo s_tableInfo;
	static TABLE_TYPE s_tableType;
	static string s_tableId;
	static bool s_bIsJoinNearby;
	bool m_isShowSNGOrSTOLayer;
	bool m_isSocketClosedByServer;
	int m_sitDownBtnIndex;
	static const int FLAG_TABLE_ID_NEAR_BY = 98;

	//Z Order
	static const int Z_TABLE = 1;
	static const int Z_PUBLIC_AREA = 4;
	static const int Z_PLAYER_INFO_BG = 4;
	static const int Z_BTNS = 7;
	static const int Z_HINT = 10;
	static const int Z_PLAYER = 16;
	static const int Z_BTN_SIT_DOWN = 16;
	static const int Z_BET_CONTROL = 19;
	static const int Z_BTN_LILY = 19;
	static const int Z_UPDATE_SIDE_POOL_ACTION = 19;
	static const int Z_TIP_ACTION = 19;
	static const int Z_GIFT = 20;
	static const int Z_WIN_CHIPS_ACTION = 20;
	static const int Z_PLAYER_INFO_LAYER = 22;
	static const int Z_GAME_RANK_LAYER = 22;
	static const int Z_GAME_MORE_MENU_LAYER = 22;
	static const int Z_TALK_LAYER = 22;
	static const int Z_NOTIFICATION_BAR = 23;
	static const int Z_LOTTERY_LAYER = 23;
	static const int Z_WIN_CHIPS_PARTICLE = 24;
	static const int Z_UPDATE_EXP_PARTICLE = 25;
	static const int Z_SNG_LAYER = 28;
	static const int Z_SNG_RESULT_LAYER = 28;
	static const int Z_SHOOT_OUT_RESULT_LAYER = 28;
	static const int Z_HOLDEM_TABLE_LAYER = 31;
	static const int Z_TUTORIAL_LAYER = 33;
	static const int Z_QUICK_BUY_LAYER = 34;
	static const int Z_SPECIAL_OFFER_LAYER = 34;
	static const int Z_MESSAGE_BOX = 37;
	static const int Z_SETTING_LAYER = 38;
	static const int Z_SHOP = 40;
	static const int Z_GUIDE = 45;

private:
	void initMoreBtn();
	void initShopBtn();
	void initTalkBtn();
	void initPlayerInfo();
	void onMoreBtnCallback(CCObject* pSender);
	void onShopBtnCallback(CCObject* pSender);
	void onSpecialOfferBtnCallback(CCObject* pSender);
	void onPhotoBtnCallback(CCObject* pSender);
	void onTalkBtnCallback(CCObject* pSender);
	void onLilyBtnCallback(CCObject* pSender);
	void getPlayerInfoByNetwork(vector<string>& ids);
	void getPlayerInfoNetworkCallback(CCObject* pSender);
	void startGame(float dt);
	void getPhotoNetworkCallback(CCObject* pSender);
	void onRecharge(float dlt = 0.0);
	virtual bool onKeyBackClicked();
	void sendGiftNetworkCallback(CCObject* pSender);
	void showTipActions(int index);
	void gotoTableNetworkCallback(CCObject* pSender);
	void removePlayer(int index);
	void onSitDownBtnCallback(CCObject* pSender);
	void updateExp(int level, int exp);
	float getDealCardsTime();
	void showSNGResult(int rank,INT64 bonus);
	void showNotificationBar(string msg, float autoHideTime=0);
	void showShootOutResult(int rank, INT64 bonus);
	void showHint();
	void hideHint();
	void enterBackground(CCObject* pSender);
	void enterForeground(CCObject* pSender);
	void showGuide();
	void showWinParticleChips(WIN_CARDS_TYPE type);
	void playSoundGetExp();
	void resetLilyAfterDeal(float);
	void initLotteryBtn();
	void onLotteryBtnCallback(CCObject *pSender);
	void initSpecialOfferCountDown();
	void updateSpecialOfferCountDown(float dt);
	void initRankBtn();
	void onRankBtnCalback(CCObject *pSender);
	void updateGameRankList(INT64 winAmount);
	void showWinChipsActions(vector<pair<int,vector<INT64> > > winnerInfos, 
		vector<pair<int,vector<int> > > winnerCardsVec);
	CCNode* createChipAmountWithBG(INT64 chipAmount);
	string getLilyImgName(string imgName);
	void layoutAllPlayers();
	void updateSidePool(CCNode* pSender);
	void doBanCmdNetworkCallback(CCObject* pSender);
	void dealCards(float dealCardsTime);
};
#endif 
