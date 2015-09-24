#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "GameConstants.h"
#include "libjson.h"
#include <string>
#include "GameScreen.h"
#include "Layer.h"
#include "PublicAreaLayer.h"
#include "cocos-ext.h"
#include "PlayerInfo.h"

USING_NS_CC;
USING_NS_CC_EXT;

class TableLayer;
class Icon;

class Player : public Layer 
{
public:
	Player();
	virtual ~Player();
	virtual void onExit();
	virtual void layoutChildren() = 0;
	void addBaseCards(const JSONNode& cardArray, bool isRunning);
	void flipCardsInHands(const JSONNode& cardArray);
	void updateState(const JSONNode& node);
	void clearState();
	virtual bool init(TableLayer* ptrTableLayer, int seat, const JSONNode& node);
	void startThinking(float delayTime=0);
	void stopThinking(bool calledByServer);
	void highlightWinCards(CCNode* node,void *ptrVoid);
	virtual bool ccTouchBegan(cocos2d::CCTouch* pTouch, cocos2d::CCEvent* pEvent);
	CCSprite* getBetLabelBG();
	void dealCard(bool isFirstOne, bool needFlipCard, int order, int all);
	int getRole();
	void talk(string content);
	void refreshPhoto();
	void setDecision(int decision);
	void setThinkTime(int thinkTime);
	void reInitState(const JSONNode& node);
	void showSendGiftActions(Player* sender,string giftCode);
	void showWinTwinkleParticle();
	void showWin();
	void updateHandIndicator();
	WIN_CARDS_TYPE getMaxCardsType();
	void initState();

private:
	void removeBaseCards();
	void progressTimerDone(CCNode* node);
	void showBetAction(INT64 currentBetAmount);
	void updateChipImg(INT64 amount);
	void showFail(bool throwCards);
	void showWinActions();
	void showAllIn();
	void vibrate();
	void playSoundStartThinking();
	void playSoundTickTock();
	void playSoundDealCard();
	void playSoundBet(INT64 betChipAmount);
	void removeAllInEffect();
	void removeBlackCover();
	void removeWinActions();
	void createHandIndicator(WIN_CARDS_TYPE cardsType);
	void removeHandIndicator(bool removeBg);

public:
	static const int REASON_NONE = 0;//for new player
	static const int REASON_KICKOUT = 2;
	static const int REASON_RECHARGE = 3;
	static const int REASON_NEW_JOIN = 4;//no use for now
	static const int REASON_STANDS_UP = 5;
	static const int REASON_SIT_DOWN_FILURE = 6;
	static const int REASON_TOURNAMENT_LOST = 7;
	static const int REASON_BLIND_BET_CHANGE = 8;
	static const int REASON_FRIEND_IN = 10;

	static const int ROLE_DEALER = 0;
	static const int ROLE_SMALL_BLIND = 1;
	static const int ROLE_BIG_BLIND = 2;
	static const int ROLE_COMMON = 3;
	static const int ROLE_NEW_JOIN =4 ;

	static const int DECISION_NONE = 0;
	static const int DECISION_CHECK = 1;
	static const int DECISION_CALL = 2;
	static const int DECISION_ALLIN = 3;
	static const int DECISION_FOLD = 4;
	static const int DECISION_RAISE = 5;
	static const int DECISION_SIT_DOWN =6 ;
	static const int DECISION_CONFIRM = 10;
	//for bet control use
	static const int DECISION_CHECK_FOLD = 14;
	static const int DECISION_CALL_ANY = 15;

	static const int GAME_PLAYING = 1;
	static const int GAME_FOLDED = 2;
	static const int GAME_ALLIN = 3;
	static const int GAME_WAIT = 4;
	static const int GAME_STANDS_UP = 5;

	static const int OP_DENIED = 204;
	static const int UNSUPPORTED_BUYIN = 210;
	static const int CHIP_NOT_ENOUGH = 213;
	static const int SEAT_FULLED = 214;

	INT64 m_betAmount;
	INT64 m_chipsInGame;
	int m_state;
	PlayerInfo m_playerInfo;
	int m_index;
	bool m_amIWin;
	bool m_isDealer;
	int m_seatPosition;
	bool m_isSeatRight;
	static const int CHIP_NUM_FOR_IMAGES[17];

protected:
	Icon* m_ptrPhoto;
	CCSprite* m_ptrWinBg;
	CCSprite* m_ptrFailBg;
	CCSprite* m_ptrHalo;
	CCSprite* m_ptrNormalBg;
	CCSprite* m_progressTimerBg;
	CCProgressTimer* m_progressTimer;
	CCLabelTTF *m_ptrLbState;
	CCSprite* m_ptrBetLabelBG;
	CCLabelTTF *m_ptrLbChipsInGame;

	//Z Order
	static const int Z_HALO = 1;
	static const int Z_BET_LABEL_BG = 1;
	static const int Z_NORMAL_BG = 4;
	static const int Z_WIN_BG = 7;
	static const int Z_FAIL_BG = 7;
	static const int Z_WIN_ACTIONS = 10;
	static const int Z_PHOTO = 13;
	static const int Z_STATE_LABEL = 13;
	static const int Z_CHIPS_IN_GAME_BG = 14;
	static const int Z_PROGRESS_TIMER_BG = 15;
	static const int Z_PROGRESS_TIMER = 16;
	static const int Z_BLACK_COVER = 17;
	static const int Z_GIFT = 18;
	static const int Z_BASE_IMG_0 = 19;
	static const int Z_BASE_IMG_1 = 22;
	static const int Z_HAND_INDICATOR = 23;
	static const int Z_WIN_LABEL = 23;
	static const int Z_DEALER = 25;
	static const int Z_BET_ACTION = 25;
	static const int Z_POP = 28;
	static const int Z_ALL_IN_EFFECT = 32;
	static const int Z_TWINKLE_PARTICLE = 34;
	static const int Z_WIN_CHIPS_ACTION = 37;

private:
	static const int PAD_WIDTH=200;
	static const int PAD_HEIGHT=180;

	CCSpriteFrame* m_ptrChipImages[17];
	INT64 m_preBetAmount;
	CCSprite* m_ptrDealer;
	CCSprite* m_ptrGiftSprite;
	CCSprite* m_ptrBlackCover;
	CCNode* m_ptrWinActions;
	CCSprite* m_ptrWinLabel;
	CCSprite* m_ptrAllInEffect;
	CCSprite* m_ptrHandIndicatorBg;
	CCScale9Sprite* m_ptrHandIndicator;
	CCLabelBMFont* m_ptrLbHandCardsType;
	TableLayer* m_ptrTableLayer;
	CCSprite* m_ptrChipImg;
	CCLabelTTF *m_ptrLbBet;
	int m_baseImgIndex0;
	int m_baseImgIndex1;
	CCSprite* m_baseImg0;
	CCSprite* m_baseImg1;
	int m_role;
	int m_decision;
	int m_thinkTime;
};

#endif 
