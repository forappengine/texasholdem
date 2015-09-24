#include "BetControler.h"
#include "LayoutUtil.h"
#include "ResourceManager.h"
#include "Player.h"
#include "cocos2d.h"
#include "TableLayer.h"
#include "GameSocket.h"
#include "JSONCommand.h"
#include "FontConstant.h"
#include "LogTag.h"
#include "Utils.h"
#include "BetSlider.h"
#include "SoundPlayer.h"
#include "GameUtils.h"
#define MODULE_LOG_TAG "BetControler.cpp"

USING_NS_CC;

void BetControler::onEnter()
{
	Layer::onEnter();
	//initWithColor(ccc4(200,10,20,60));
	setContentSize(CCSizeMake(LAYER_WIDTH,LAYER_HEIGHT));
	LayoutUtil::layoutParentCenter(this,280,-240);

	m_foldButton=createSelectButton("fold.png","fold_down.png");
	LayoutUtil::layoutParentLeftBottom(m_foldButton);
	m_foldButton->setUserData((void*)(Player::DECISION_FOLD));
	m_ptrFoldTimer = CCProgressTimer::create(ResourceManager::getInstance()->createSprite(this, "fold_timer.png"));
	m_ptrFoldTimer->setType(kCCProgressTimerTypeRadial);
	m_ptrFoldTimer->setReverseProgress(true);
	m_ptrFoldTimer->setPercentage(0);
	m_foldButton->addChild(m_ptrFoldTimer);
	LayoutUtil::layoutParentCenter(m_ptrFoldTimer);

	m_checkButton=createSelectButton("check.png","check_down.png");
	LayoutUtil::layoutParentLeftBottom(m_checkButton,160);
	m_checkButton->setUserData((void*)(Player::DECISION_CHECK));
	m_ptrCheckTimer = CCProgressTimer::create(ResourceManager::getInstance()->createSprite(this, "check_timer.png"));
	m_ptrCheckTimer->setType(kCCProgressTimerTypeRadial);
	m_ptrCheckTimer->setReverseProgress(true);
	m_ptrCheckTimer->setPercentage(0);
	m_checkButton->addChild(m_ptrCheckTimer);
	LayoutUtil::layoutParentCenter(m_ptrCheckTimer);

	m_callButton=createSelectButton("call.png","call_down.png");
	LayoutUtil::layoutParentLeftBottom(m_callButton,160);
	m_callButton->setUserData((void*)(Player::DECISION_CALL));
	m_ptrLbCallAmount = CCLabelTTF::create("",FontConstant::TTF_IMPACT,20);
	m_callButton->addChild(m_ptrLbCallAmount);
	LayoutUtil::layoutParentCenter(m_ptrLbCallAmount);

	m_allinButton=createSelectButton("allin.png","allin_down.png");
	LayoutUtil::layoutParentLeftBottom(m_allinButton,160);
	m_allinButton->setUserData((void*)(Player::DECISION_ALLIN));

	m_raiseButton=createSelectButton("raise.png","raise_down.png","raise_disabled.png");
	LayoutUtil::layoutParentLeftBottom(m_raiseButton,320);
	m_raiseButton->setUserData((void*)(Player::DECISION_RAISE));
	
	m_okButton=createSelectButton("ok.png","ok_down.png");
	m_okButton->setUserData((void*)(Player::DECISION_CONFIRM));
	LayoutUtil::layoutParentLeftBottom(m_okButton,322);
	//-------------------------------
	
	m_checkFoldRadioButton=createRadioButton("label_checkfold.png","radio_checkfold.png","radio_checkfold_down.png");
	LayoutUtil::layoutParentLeftBottom(m_checkFoldRadioButton);
	m_checkFoldRadioButton->setUserData((void*)(Player::DECISION_CHECK_FOLD));

	m_checkRadioButton=createRadioButton("label_check.png","radio_check.png","radio_check_down.png");
	LayoutUtil::layoutParentLeftBottom(m_checkRadioButton,160);
	m_checkRadioButton->setUserData((void*)(Player::DECISION_CHECK));

	m_callRadioButton=createRadioButton("label_call.png","radio_call.png","radio_call_down.png");
	LayoutUtil::layoutParentLeftBottom(m_callRadioButton,160);
	m_callRadioButton->setUserData((void*)(Player::DECISION_CALL));

	m_allinRadioButton=createRadioButton("label_allin.png","radio_allin.png","radio_allin_down.png");
	LayoutUtil::layoutParentLeftBottom(m_allinRadioButton,160);
	m_allinRadioButton->setUserData((void*)(Player::DECISION_ALLIN));

	m_callanyRadioButton=createRadioButton("label_callany.png","radio_callany.png","radio_callany_down.png",
		"radio_callany_disabled.png","label_callany_disabled.png");
	LayoutUtil::layoutParentLeftBottom(m_callanyRadioButton,285);
	m_callanyRadioButton->setUserData((void*)(Player::DECISION_CALL_ANY));

	createSliderControler();
}

void BetControler::createSliderControler()
{
	m_slider = BetSlider::create(); 	
	addChild(m_slider);
	LayoutUtil::layoutParentLeftBottom(m_slider,222);
	m_slider->setVisible(false);
}                          

CCMenuItemSprite* BetControler::createSelectButton(const char* normal,const char* selected,const char* disabled/*=NULL*/){
	if(normal==NULL){
		return NULL;
	}

	CCNode* node = CCNode::create();
	node->setContentSize(CCSizeMake(104, 104));
	CCSprite* sprite = ResourceManager::getInstance()->createSprite(this, normal);
	node->addChild(sprite);
	LayoutUtil::layoutParentCenter(sprite);
	CCNode* downNode=node;
	if(selected!=NULL){
		downNode = CCNode::create();
		downNode->setContentSize(CCSizeMake(104, 104));
		CCSprite* downSprite = ResourceManager::getInstance()->createSprite(this, selected);
		downNode->addChild(downSprite);
		LayoutUtil::layoutParentCenter(downSprite);
	}

	CCNode* disabledNode=NULL;
	if(disabled!=NULL){
		disabledNode = CCNode::create();
		disabledNode->setContentSize(CCSizeMake(104, 104));
		CCSprite* disabledSprite = ResourceManager::getInstance()->createSprite(this, disabled);
		disabledNode->addChild(disabledSprite);
		LayoutUtil::layoutParentCenter(disabledSprite);
	}
	CCMenuItemSprite* button=NULL;
	if(disabledNode==NULL){
		button=CCMenuItemSprite::create(node,downNode, this, menu_selector(BetControler::selectButtonsCallback));
	}else{
		button=CCMenuItemSprite::create(node,downNode,disabledNode, this, menu_selector(BetControler::selectButtonsCallback));
	}
	
	registerMenuItem(button);
	this->addChild(button);
	//button->unselected();
	button->setVisible(false);
	return button;
}


CCMenuItemSprite* BetControler::createRadioButton(const char* label,const char* normal,const char* selected,const char* disabled/*=NULL*/,const char* disabledLabel/*=NULL*/){
	if(normal==NULL){
		return NULL;
	}

	CCNode* node = CCNode::create();
	CCSprite* sprite = ResourceManager::getInstance()->createSprite(this, normal);
	CCSprite* labelSprite = ResourceManager::getInstance()->createSprite(this, label);
	node->setContentSize(CCSizeMake(90+labelSprite->getContentSize().width, 110));
	node->addChild(sprite);
	node->addChild(labelSprite);
	LayoutUtil::layoutParentLeft(sprite);
	LayoutUtil::layoutParentLeft(labelSprite,90);
	CCNode* downNode=node;
	if(selected!=NULL){
		downNode = CCNode::create();
		CCSprite* downSprite = ResourceManager::getInstance()->createSprite(this, selected);
		CCSprite* labeldownSprite = ResourceManager::getInstance()->createSprite(this, label);
		downNode->setContentSize(CCSizeMake(90+labeldownSprite->getContentSize().width, 110));
		downNode->addChild(downSprite);
		downNode->addChild(labeldownSprite);
		LayoutUtil::layoutParentLeft(downSprite);
		LayoutUtil::layoutParentLeft(labeldownSprite,90);
	}

	CCNode* disabledNode=NULL;
	if(disabled!=NULL){
		disabledNode = CCNode::create();
		disabledNode->setContentSize(CCSizeMake(104, 104));
		CCSprite* disabledSprite = ResourceManager::getInstance()->createSprite(this, disabled);
		CCSprite* labelDisabledSprite = ResourceManager::getInstance()->createSprite(this, disabledLabel);
		disabledNode->addChild(disabledSprite);
		disabledNode->addChild(labelDisabledSprite);
		LayoutUtil::layoutParentLeft(disabledSprite);
		LayoutUtil::layoutParentLeft(labelDisabledSprite,90);
	}

	CCMenuItemSprite* button=NULL;
	if(disabledNode==NULL){
		button=CCMenuItemSprite::create(node,downNode, this, menu_selector(BetControler::radioButtonsCallback));
	}else{
		button=CCMenuItemSprite::create(node,downNode,disabledNode, this, menu_selector(BetControler::radioButtonsCallback));
	}

	registerMenuItem(button);
	this->addChild(button);
	//button->unselected();
	button->setVisible(false);
	return button;
}

void BetControler::focusButton( int index){
	switch(index){
	case Player::DECISION_CHECK_FOLD:
		m_checkFoldRadioButton->selected();
		break;
	case Player::DECISION_CHECK:
		m_checkRadioButton->selected();
		break;
	case Player::DECISION_CALL:
		m_callRadioButton->selected();
		break;
	case Player::DECISION_ALLIN:
		m_allinRadioButton->selected();
		break;
	case Player::DECISION_CALL_ANY:
		m_callanyRadioButton->selected();
		break;
	}
}

void BetControler::unFocusButton( int index){
	switch(index){
	case Player::DECISION_CHECK_FOLD:
		m_checkFoldRadioButton->unselected();
		break;
	case Player::DECISION_CHECK:
		m_checkRadioButton->unselected();
		break;
	case Player::DECISION_CALL:
		m_callRadioButton->unselected();
		break;
	case Player::DECISION_ALLIN:
		m_allinRadioButton->unselected();
		break;
	case Player::DECISION_CALL_ANY:
		m_callanyRadioButton->unselected();
		break;
	}
}

void BetControler::clearButton(){
	m_checkFoldRadioButton->unselected();
	m_checkRadioButton->unselected();
	m_callRadioButton->unselected();
	m_allinRadioButton->unselected();
	if (m_callanyRadioButton->isEnabled())
	{
		m_callanyRadioButton->unselected();
	}
}

void BetControler::radioButtonsCallback( CCObject* pSender )
{
	SoundPlayer::play(SNDI_CLICK);
	int data=(int)(static_cast<CCMenuItemSprite*>(pSender)->getUserData());
	if(preSelection==data){
		unFocusButton(data);
		preSelection=Player::DECISION_NONE;
	}else{
		clearButton();
		focusButton(data);
		preSelection=data;
	}
}


void BetControler::selectButtonsCallback( CCObject* pSender )
{
	int data=(int)(static_cast<CCMenuItemSprite*>(pSender)->getUserData());
	actionID=data;
	
	if(actionID==Player::DECISION_RAISE){
		SoundPlayer::play(SNDI_CLICK);
		TableLayer* table=gameScreen->table;
		Player* mySelf=table->mySelf;
		INT64 maxRaise=mySelf->m_chipsInGame+mySelf->m_betAmount;
		INT64 blindBet=table->blindBet;
		INT64 minRaise=table->maxBet+blindBet;
		if(minRaise>maxRaise){
			raiseData=maxRaise;
		}else{
			raiseData=minRaise;
		}
		m_raiseButton->setVisible(false);
		m_slider->reInitSlider();
		m_slider->setVisible(true);
		m_slider->setValue(raiseData);
		m_okButton->setVisible(true);
	}else if(actionID==Player::DECISION_CONFIRM){
		hideAllBtns();
		actionID=Player::DECISION_RAISE;
		//raiseData=raiseData;
		JSONCommand* lcmd=JSONCommand::createJSONCMD(JSONCommand::CMD_U_COMMIT);
		char* msg=lcmd->encode(gameScreen);
		gameScreen->gameSocket->sendMsg(msg);

		delete lcmd;
	}else{
		hideAllBtns();
		raiseData=0;
		JSONCommand* lcmd=JSONCommand::createJSONCMD(JSONCommand::CMD_U_COMMIT);
		char* msg=lcmd->encode(gameScreen);
		gameScreen->gameSocket->sendMsg(msg);

		delete lcmd;
	}
}

//whether has new card released,  we need to clear MaxBet for new circle at this point
void BetControler::updateButtons( GameScreen* gameScreen,const JSONNode& node,float delayTime/*=0*/ )
{	
	this->gameScreen=gameScreen;
	TableLayer* table=gameScreen->table;
	Player* mySelf=table->mySelf;

	INT64 betAmount=0,chipsInGame=0;
	if(mySelf != NULL)
	{
		betAmount=mySelf->m_betAmount;
		chipsInGame=mySelf->m_chipsInGame;
	}
	
	INT64 maxBet=table->maxBet;
	//hide all buttons first
	hideAllBtns();
	
	int next=node["next"].as_int();
	int currentDecision = JsonHelper::optInt(node,"decision");
	if(preSelection!=Player::DECISION_NONE){
		switch(preSelection){
		case Player::DECISION_CHECK_FOLD:
			if(next==table->myPosition&&table->isSitDown){
				if(betAmount<maxBet){
					preSelection=Player::DECISION_FOLD;
				}else {
					preSelection=Player::DECISION_CHECK;
				}
			}
			break;
		case Player::DECISION_CALL_ANY:
			if(next==table->myPosition&&table->isSitDown){				
				if(betAmount==maxBet){
					preSelection=Player::DECISION_CHECK;
				}else{
					preSelection=Player::DECISION_CALL;
				}
			}
			break;
		case Player::DECISION_ALLIN:
			break;
		case Player::DECISION_CHECK:
			if(betAmount < maxBet)
			{
				preSelection=Player::DECISION_NONE;
				clearButton();
			}
			break;
		case Player::DECISION_CALL:
			if (currentDecision == Player::DECISION_RAISE
				&& betAmount < maxBet)
			{
				clearButton();
				preSelection=Player::DECISION_NONE;
			}
			break;			
		}
	}

	if(preSelection!=Player::DECISION_NONE&&next==table->myPosition&&table->isSitDown){
		actionID=preSelection;
		raiseData=0;

		JSONCommand* lcmd=JSONCommand::createJSONCMD(JSONCommand::CMD_U_COMMIT);
		char* msg=lcmd->encode(gameScreen);
		gameScreen->gameSocket->sendMsg(msg);
		delete lcmd;

		clearButton();
		preSelection=Player::DECISION_NONE;//comments this out to turn the auto debug on
		return;
	}

	if(next==table->myPosition&&table->isSitDown)//big select button
	{	
		m_ptrCheckTimer->stopAllActions();
		m_ptrCheckTimer->setVisible(false);
		m_ptrFoldTimer->stopAllActions();
		m_ptrFoldTimer->setVisible(false);
		m_foldButton->setVisible(true);
		m_raiseButton->setVisible(true);
		m_raiseButton->setEnabled(true);
		m_checkButton->setVisible(true);
		if(betAmount<maxBet){
			if(chipsInGame+betAmount>maxBet){
				m_callButton->setVisible(true);
				m_checkButton->setVisible(false);
				m_ptrLbCallAmount->setString(
					CCString::createWithFormat("%s",GameUtils::num2tableStr(maxBet-betAmount,table->blindBet,6).c_str())->getCString());
			}else{
				m_allinButton->setVisible(true);
				m_raiseButton->setEnabled(false);
				m_checkButton->setVisible(false);
			}

		}else if(betAmount==maxBet){
			if(chipsInGame+betAmount>=maxBet){
				m_checkButton->setVisible(true);
				m_ptrCheckTimer->setVisible(true);
				m_ptrCheckTimer->runAction(CCSequence::create(
					CCDelayTime::create(delayTime),
					CCProgressFromTo::create(((TableLayer*)getParent())->thinkTime, 100,0),NULL));
			}else{//shouldn't happen
				m_checkButton->setVisible(true);
				m_raiseButton->setEnabled(false);
			}
		}
		if (!m_checkButton->isVisible())
		{
			m_ptrFoldTimer->setVisible(true);
			m_ptrFoldTimer->runAction(CCSequence::create(
				CCDelayTime::create(delayTime),
				CCProgressFromTo::create(((TableLayer*)getParent())->thinkTime, 100,0),NULL));
		}
	}else{//small radio button
		if(mySelf && mySelf->m_state==Player::GAME_PLAYING&&table->isSitDown){
			m_checkFoldRadioButton->setVisible(true);
			m_callanyRadioButton->setVisible(true);
			m_callanyRadioButton->setEnabled(true);
			m_checkRadioButton->setVisible(true);

			if(betAmount<maxBet){//not check
				if(chipsInGame+betAmount>maxBet){
					m_callRadioButton->setVisible(true);
					m_checkRadioButton->setVisible(false);
				}else{
					m_allinRadioButton->setVisible(true);
					m_checkRadioButton->setVisible(false);
					m_callanyRadioButton->setEnabled(false);
				}
			}else if(betAmount==maxBet){
				m_checkRadioButton->setVisible(true);
			}
		}
	}
}

void BetControler::clear(){
	actionID=Player::DECISION_NONE;
	raiseData=0;
	preSelection=Player::DECISION_NONE;
	clearButton();
}

void BetControler::onExit()
{
	Layer::onExit();
}

// on "init" you need to initialize your instance
bool BetControler::init()
{
	if ( !Layer::init() )
    {
        return false;
    }

	//setTouchPenetrate(true);

	actionID=Player::DECISION_NONE;
	raiseData=0;
	preSelection=Player::DECISION_NONE;
	m_slider=NULL;
    return true;
}

void BetControler::hideAllBtns()
{
	m_foldButton->setVisible(false);
	m_raiseButton->setVisible(false);
	m_checkButton->setVisible(false);
	m_callButton->setVisible(false);
	m_allinButton->setVisible(false);

	m_checkFoldRadioButton->setVisible(false);
	m_checkRadioButton->setVisible(false);
	m_callRadioButton->setVisible(false);
	m_allinRadioButton->setVisible(false);
	m_callanyRadioButton->setVisible(false);

	m_okButton->setVisible(false);
	m_slider->setVisible(false);
}

GameScreen* BetControler::getGameScreen()
{
	return gameScreen;
}

void BetControler::hideBetSlider()
{
	m_okButton->setVisible(false);
	m_raiseButton->setVisible(true);
}
