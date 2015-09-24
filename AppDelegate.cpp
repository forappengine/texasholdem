#include "AppDelegate.h"
#include "cocos2d.h"
//#include "CSArmatureDataManager.h"
#include "GameConstants.h"
#include "MTNotificationQueue.h"
#include "CoverLayer.h"
#include "Screen.h"
#include "LayerTouchDispatcher.h"
#include "LayerKeyDispatcher.h"
#include "SimpleAudioEngine.h"
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
//#include "StatisticsManager.h"
#include "JniUtils.h"
#endif
#include "GameUtils.h"

USING_NS_CC;
using namespace CocosDenshion;


AppDelegate::AppDelegate() {

}

AppDelegate::~AppDelegate() 
{
	//cs::ArmatureDataManager::sharedArmatureDataManager()->purgeArmatureSystem();
	SoundPlayer::unloadAll();
	delete ptrSoundPlayer;
	delete Screen::getCurrentScreen();
	GameUtils::deleteAllFilesFromDir(GameUtils::WRITABLE_PHOTO_PATH);
}

bool AppDelegate::applicationDidFinishLaunching()
{
    // initialize director
    CCDirector* pDirector = CCDirector::sharedDirector();
    CCEGLView* pEGLView = CCEGLView::sharedOpenGLView();

    pDirector->setOpenGLView(pEGLView);
    pDirector->setProjection(kCCDirectorProjection2D);

    // Set the design resolution
    CCSize winSize = pDirector->getWinSize();
	pDirector->getScheduler()->scheduleSelector(  
		schedule_selector(MTNotificationQueue::postNotifications),  
		MTNotificationQueue::sharedNotificationQueue(),  
		1.0 / 60.0,false);
	Screen* currentScreen = Screen::init(winSize.width, winSize.height,
		GameConstants::DESIGN_FRAME_WIDTH,GameConstants::DESIGN_FRAME_HEIGHT);

	float width = currentScreen->getScaledDesignedWidth();
	float height = currentScreen->getScaledDesignedHeight();
    pEGLView->setDesignResolutionSize(width, height, kResolutionNoBorder);

    LayerTouchDispatcher::getInstance()->registerTouch();

    LayerKeyDispatcher::getInstance()->registerUIKeyDispatcher();
/*
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
    StatisticsManager::getInstance()->init("log.tdgame.biz:8080");
    StatisticsManager::getInstance()->onStart(JniUtil::getDeviceID().c_str(),
    		Preferences::sharedPreferences()->getStringForKey("ref").c_str(),
    		Player::Player::getCurrentPlayer().m_nRank);
#endif
*/

	// turn on display FPS
	//pDirector->setDisplayStats(true);

    // set FPS. the default value is 1.0/60 if you don't call this
    pDirector->setAnimationInterval(1.0 / 60);

    // create a scene. it's an autorelease object
    CCScene *pScene = CoverLayer::scene();
    // run
    pDirector->runWithScene(pScene);
	ptrSoundPlayer = new SoundPlayer();
	CCDirector::sharedDirector()->getScheduler()->scheduleUpdateForTarget(ptrSoundPlayer, 0, false);
	SoundPlayer::preloadAll();

    return true;
}



// This function will be called when the app is inactive. When comes a phone call,it's be invoked too
void AppDelegate::applicationDidEnterBackground()
{
	CCLOG("AppDelegate::applicationDidEnterBackground");
	CCNotificationCenter::sharedNotificationCenter()->postNotification("enterBackground");
	//AudioUtil::stopAll();
    //CCDirector::sharedDirector()->pause();
    //CCDirector::sharedDirector()->stopAnimation();
    //DataVersion::getInstance().pauseDownload();
    // if you use SimpleAudioEngine, it must be pause
	SimpleAudioEngine::sharedEngine()->stopAllEffects();
    SimpleAudioEngine::sharedEngine()->pauseBackgroundMusic();
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
	JniUtils::s_bBgMusicPaused = true;
#endif
}

// this function will be called when the app is active again
void AppDelegate::applicationWillEnterForeground()
{
	CCLOG("AppDelegate::applicationWillEnterForeground");
	CCNotificationCenter::sharedNotificationCenter()->postNotification("enterForeground");
	//CCDirector::sharedDirector()->startAnimation();
    //CCDirector::sharedDirector()->resume();
    //DataVersion::getInstance().resumeDownload();
    // if you use SimpleAudioEngine, it must resume here
    //if (JniUtil::s_bWindowHasFocus)
    //{
    //	SimpleAudioEngine::sharedEngine()->resumeBackgroundMusic();
    //}
}


