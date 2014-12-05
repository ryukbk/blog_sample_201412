#include "GameScene.h"
#include "PlayerCharacter.h"

USING_NS_CC;

using namespace cocostudio::timeline;

Scene* GameScene::createScene()
{
    // 'scene' is an autorelease object
	auto scene = Scene::createWithPhysics();
	scene->getPhysicsWorld()->setDebugDrawMask(PhysicsWorld::DEBUGDRAW_ALL);

    // 'layer' is an autorelease object
    auto layer = GameScene::create();

    // add layer as a child to scene
    scene->addChild(layer);

    // return the scene
    return scene;
}

// on "init" you need to initialize your instance
bool GameScene::init()
{
    //////////////////////////////
    // 1. super init first
    if ( !Node::init() )
    {
        return false;
    }
    
    Size visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    /////////////////////////////
    // 2. add a menu item with "X" image, which is clicked to quit the program
    //    you may modify it.

    // add a "close" icon to exit the progress. it's an autorelease object
    auto closeItem = MenuItemImage::create(
                                           "CloseNormal.png",
                                           "CloseSelected.png",
                                           CC_CALLBACK_1(GameScene::menuCloseCallback, this));
    
	closeItem->setPosition(Vec2(origin.x + visibleSize.width - closeItem->getContentSize().width/2 ,
                                origin.y + closeItem->getContentSize().height/2));

    // create menu, it's an autorelease object
    auto menu = Menu::create(closeItem, NULL);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 1);

    /////////////////////////////
    // 3. add your codes below...

    // add a label shows "Hello World"
    // create and initialize a label
    
    auto label = Label::createWithTTF("Santa Shooter", "fonts/Marker Felt.ttf", 24);
    
    // position the label on the center of the screen
    label->setPosition(Vec2(origin.x + visibleSize.width/2,
                            origin.y + visibleSize.height - label->getContentSize().height));

    // add the label as a child to this layer
    this->addChild(label, 1);

	auto spriteFrameCache = SpriteFrameCache::getInstance();
	spriteFrameCache->addSpriteFramesWithFile("sprites.plist");

	player1 = PlayerCharacter::create();
	player1->setPosition(Vec2(PLAYER_X_POS, visibleSize.height / 2));

	this->addChild(player1);

	player2 = PlayerCharacter::create();
	player2->setPosition(Vec2(visibleSize.width - PLAYER_X_POS, visibleSize.height / 2));
	player2->stayIdle(true);

	this->addChild(player2);

	auto dispatcher = Director::getInstance()->getEventDispatcher();
	auto listener = EventListenerKeyboard::create();

	listener->onKeyPressed = [=](EventKeyboard::KeyCode keyCode, Event* event) {
		switch (keyCode) {
		case EventKeyboard::KeyCode::KEY_W:
			player1->playWalkUp();
			break;
		case EventKeyboard::KeyCode::KEY_S:
			player1->playWalkDown();
			break;
		case EventKeyboard::KeyCode::KEY_UP_ARROW:
			player2->playWalkUp();
			break;
		case EventKeyboard::KeyCode::KEY_DOWN_ARROW:
			player2->playWalkDown();
			break;
		}
	};

	listener->onKeyReleased = [=](EventKeyboard::KeyCode keyCode, Event* event) {
		switch (keyCode) {
		case EventKeyboard::KeyCode::KEY_W:
		case EventKeyboard::KeyCode::KEY_S:
			player1->stayIdle(false);
			break;
		case EventKeyboard::KeyCode::KEY_UP_ARROW:
		case EventKeyboard::KeyCode::KEY_DOWN_ARROW:
			player2->stayIdle(true);
			break;
		}
	};

	dispatcher->addEventListenerWithSceneGraphPriority(listener, this);

	auto touchListener = EventListenerTouchAllAtOnce::create();
	touchListener->onTouchesBegan = CC_CALLBACK_2(GameScene::onTouchesBegan, this);
	touchListener->onTouchesEnded = CC_CALLBACK_2(GameScene::onTouchesEnded, this);
	dispatcher->addEventListenerWithSceneGraphPriority(touchListener, this);

	auto contactListener = EventListenerPhysicsContact::create();
	contactListener->onContactBegin = CC_CALLBACK_1(GameScene::onContactBegin, this);
	dispatcher->addEventListenerWithSceneGraphPriority(contactListener, this);

    return true;
}

void GameScene::menuCloseCallback(Ref* pSender)
{
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WP8) || (CC_TARGET_PLATFORM == CC_PLATFORM_WINRT)
	MessageBox("You pressed the close button. Windows Store Apps do not implement a close button.","Alert");
    return;
#endif

    Director::getInstance()->end();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    exit(0);
#endif
}

void GameScene::onTouchesBegan(const std::vector<cocos2d::Touch*>& touches, cocos2d::Event* event)
{
	Size visibleSize = Director::getInstance()->getVisibleSize();
	auto spriteFrameCache = SpriteFrameCache::getInstance();
	for (auto touch: touches) {
		Point touchPosition = touch->getLocation();
		if (touchPosition.x > visibleSize.width / 2) {
			CCLOG("Player 1 attack");
			player1->attack(this, touch, spriteFrameCache, visibleSize);
		} else {
			CCLOG("Player 2 attack");
			player2->attack(this, touch, spriteFrameCache, visibleSize);
		}
	}
}

void GameScene::onTouchesEnded(const std::vector<cocos2d::Touch*>& touches, cocos2d::Event* event)
{
	
}

bool GameScene::onContactBegin(const PhysicsContact& contact)
{
	CCLOG("onContactBegin");
	return true;
}

