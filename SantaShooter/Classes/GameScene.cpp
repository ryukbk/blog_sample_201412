#include "GameScene.h"
#include "PlayerCharacter.h"

USING_NS_CC;

using namespace cocostudio::timeline;
using namespace cocos2d::ui;

Scene* GameScene::createScene()
{
    // 'scene' is an autorelease object
	auto scene = Scene::createWithPhysics();
#ifndef NDEBUG
	scene->getPhysicsWorld()->setDebugDrawMask(PhysicsWorld::DEBUGDRAW_ALL);
#endif

	scene->getPhysicsWorld()->setGravity(Vec2::ZERO);

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

	auto console = CSLoader::createNode("Console.csb");
	this->addChild(console, 0, "Console");
	console->setPosition(Point(100, 100));

	auto connectButton = dynamic_cast<Button*>(console->getChildByName("ConnectButton"));
	connectButton->addTouchEventListener(this, static_cast<SEL_TouchEvent>(&GameScene::onConnectButtonPressed));

	auto spriteFrameCache = SpriteFrameCache::getInstance();
	spriteFrameCache->addSpriteFramesWithFile("sprites.plist");

	player1 = PlayerCharacter::create();
	player1->setPosition(Vec2(PLAYER_X_POS, visibleSize.height / 2));
	player1->setName("player1");
	this->addChild(player1);

	player2 = PlayerCharacter::create();
	player2->setPosition(Vec2(visibleSize.width - PLAYER_X_POS, visibleSize.height / 2));
	player2->stayIdle(true);
	player2->setName("player2");
	this->addChild(player2);

	player1->getPhysicsBody()->setCategoryBitmask(~player2->getContactBitMask());
	player2->getPhysicsBody()->setCategoryBitmask(~player1->getContactBitMask());

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

	getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, this);

	auto touchListener = EventListenerTouchAllAtOnce::create();
	touchListener->onTouchesBegan = CC_CALLBACK_2(GameScene::onTouchesBegan, this);
	touchListener->onTouchesEnded = CC_CALLBACK_2(GameScene::onTouchesEnded, this);
	getEventDispatcher()->addEventListenerWithSceneGraphPriority(touchListener, this);

	auto contactListener = EventListenerPhysicsContact::create();
	contactListener->onContactBegin = CC_CALLBACK_1(GameScene::onContactBegin, this);
	getEventDispatcher()->addEventListenerWithSceneGraphPriority(contactListener, this);

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
			player1->attack(this, touch, spriteFrameCache, visibleSize, player2->getContactBitMask());
		} else {
			CCLOG("Player 2 attack");
			player2->attack(this, touch, spriteFrameCache, visibleSize, player1->getContactBitMask());
		}
	}
}

void GameScene::onTouchesEnded(const std::vector<cocos2d::Touch*>& touches, cocos2d::Event* event)
{
	
}

bool GameScene::onContactBegin(const PhysicsContact& contact)
{
	std::string log;
	log += "onContactBegin: ";
	log += contact.getShapeA()->getBody()->getNode()->getName();
	log += " - ";
	log += contact.getShapeB()->getBody()->getNode()->getName();

	CCLOG(log.c_str());
	return true;
}

void GameScene::onConnectButtonPressed(Ref* pSender, TouchEventType type)
{
	switch (type) {
	case TouchEventType::TOUCH_EVENT_BEGAN:
		auto parent = dynamic_cast<Button*>(pSender)->getParent();
		auto ipAddressBox = dynamic_cast<TextField*>(parent->getChildByName("IPAddress"));
		std::string a = ipAddressBox->getString();
		std::string::size_type index = a.find_first_of(":");
		std::string port = "31337";
		std::string ipAddress = "127.0.0.1";
		if (index != std::string::npos) {
			ipAddress = a.substr(0, index);
			port = a.substr(index + 1);
		}

		std::string finalDest = "ws://";
		finalDest += ipAddress;
		finalDest += ':';
		finalDest += port;

		//std::string s("Connecting to ");
		//s += ipAddress->getString();

		if (websocket == nullptr) {
			websocket = new network::WebSocket();
		}

		std::string log = "Connecting to ";
		log += finalDest;
		CCLOG(log.c_str());

		websocket->init(*this, finalDest);
		break;
	}
}

void GameScene::onOpen(cocos2d::network::WebSocket* ws)
{
	CCLOG("websocket open");
}

void GameScene::onMessage(cocos2d::network::WebSocket* ws, const cocos2d::network::WebSocket::Data& data)
{

}

void GameScene::onClose(cocos2d::network::WebSocket* ws)
{

}

void GameScene::onError(cocos2d::network::WebSocket* ws, const cocos2d::network::WebSocket::ErrorCode& error)
{
	CCLOG("websocket error code: %d", error);
}

