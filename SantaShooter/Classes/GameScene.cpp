#include "GameScene.h"
#include "PlayerCharacter.h"

USING_NS_CC;

using namespace cocostudio::timeline;
using namespace cocos2d::ui;

Scene* GameScene::createScene()
{
	auto scene = Scene::createWithPhysics();
	auto physicsWorld = scene->getPhysicsWorld();

#ifndef NDEBUG
	physicsWorld->setDebugDrawMask(PhysicsWorld::DEBUGDRAW_ALL);
#endif

	physicsWorld->setAutoStep(false);
	physicsWorld->setGravity(Vec2::ZERO);

	scene->addChild(GameScene::create());

	return scene;
}

bool GameScene::init()
{
	if ( !Node::init() )
	{
		return false;
	}

	this->scheduleUpdate();

	std::string fullPath = FileUtils::getInstance()->fullPathForFilename("CloseNormal.png");
	if (!FileUtils::getInstance()->isFileExist(fullPath)) {
		return true;
	}

	Size visibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();
	auto closeItem = MenuItemImage::create("CloseNormal.png", "CloseSelected.png", CC_CALLBACK_1(GameScene::menuCloseCallback, this));

	closeItem->setPosition(Vec2(origin.x + visibleSize.width - closeItem->getContentSize().width/2, origin.y + closeItem->getContentSize().height/2));

	auto menu = Menu::create(closeItem, NULL);
	menu->setPosition(Vec2::ZERO);
	this->addChild(menu, 1);

	auto console = CSLoader::createNode("Console.csb");
	this->addChild(console, 0, "Console");
	console->setPosition(Point(100, 100));

	auto connectButton = dynamic_cast<Button*>(console->getChildByName("ConnectButton"));
	connectButton->addTouchEventListener(std::bind(&GameScene::onConnectButtonPressed, this, std::placeholders::_1, std::placeholders::_2));

	return true;
}

void GameScene::setupPlayers()
{
	auto spriteFrameCache = SpriteFrameCache::getInstance();
	spriteFrameCache->addSpriteFramesWithFile("sprites.plist");

	player1 = PlayerCharacter::create();
	player1->addPhysics();

	Size visibleSize = Director::getInstance()->getVisibleSize();
	player1->setPosition(Vec2(PLAYER_X_POS, visibleSize.height / 2));
	player1->setName("player1");
	this->addChild(player1);

	player2 = PlayerCharacter::create();
	player2->addPhysics();

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
}

void GameScene::update(float deltaTime)
{
	Director::getInstance()->getRunningScene()->getPhysicsWorld()->step(deltaTime);
}

void GameScene::addConsoleText(std::string text)
{
	CCLOG(text.c_str());

	consoleLines.push_back(text);
	if (consoleLines.size() > 8) {
		consoleLines.pop_front();
	}

	auto textBox = dynamic_cast<Text*>(this->getChildByName("Console")->getChildByName("ConsoleLines"));
	std::string finalText;
	for (auto line: consoleLines) {
		finalText += line;
		finalText += "\n";
	}

	textBox->setString(finalText);
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
	std::string nameA = contact.getShapeA()->getBody()->getNode()->getName();
	std::string nameB = contact.getShapeB()->getBody()->getNode()->getName();

	std::string log;
	log += "onContactBegin: ";
	log += nameA;
	log += " - ";
	log += nameB;

	CCLOG(log.c_str());

	if ((nameA == "giftbox" && nameB == "player2") || (nameB == "giftbox" && nameA == "player2")) {
		score1 += 10;
		updateScore();
	} else if ((nameA == "giftbox" && nameB == "player1") || (nameB == "giftbox" && nameA == "player1")) {
		score2 += 10;
		updateScore();
	}

	return true;
}

void GameScene::onConnectButtonPressed(Ref* pSender, cocos2d::ui::Widget::TouchEventType type)
{
	if (websocket != nullptr) {
		return;
	}

	switch (type) {
	case cocos2d::ui::Widget::TouchEventType::BEGAN:
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

		if (websocket == nullptr) {
			websocket = new network::WebSocket();
		}

		std::string log = "Connecting to ";
		log += finalDest;

		addConsoleText(log);

		websocket->init(*this, finalDest);
		break;
	}
}

void GameScene::onOpen(cocos2d::network::WebSocket* ws)
{
	addConsoleText("websocket open");
}

void GameScene::onMessage(cocos2d::network::WebSocket* ws, const cocos2d::network::WebSocket::Data& data)
{
	if (ws == websocket) {
		rapidjson::Document json;
		json.Parse<0>(data.bytes);
		if (json.HasParseError())
		{
			CCLOG("GetParseError %s\n", json.GetParseError());

			std::string log("GetParseError: ");
			log += json.GetParseError();
			log += " ";
			log += data.bytes;
			addConsoleText(log);
			return;
		}

		auto opcode = (Opcode)json["o"].GetInt();
		switch (opcode) {
		case Opcode::HELLO:
			role = Role(json["role"].GetInt());

			updateStatus();

			if (role == Role::SERVER) {
				setupPlayers();
			} else {
				sendPing();
			}
			break;
		case Opcode::PING:
			{
				Role target = Role(json["d"].GetInt());
				sendPong(target);
			}
			break;
		case Opcode::PONG:
			pingTime = std::chrono::duration_cast< std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - pingStartTime).count();
			updateStatus();
			break;
		}
	}
}

void GameScene::onClose(cocos2d::network::WebSocket* ws)
{
	addConsoleText("websocket close");

	if (ws == websocket && websocket != nullptr) {
		delete websocket;
		websocket = nullptr;
	}
}

void GameScene::onError(cocos2d::network::WebSocket* ws, const cocos2d::network::WebSocket::ErrorCode& error)
{
	std::string log("websocket error code: ");
	std::stringstream ss;
	ss << log << (int)error;
	addConsoleText(ss.str());

	if (ws == websocket && websocket != nullptr) {
		delete websocket;
		websocket = nullptr;
	}
}

void GameScene::updateStatus()
{
	auto status = dynamic_cast<Text*>(this->getChildByName("Console")->getChildByName("Status"));
	std::string text("Role: ");
	if (role == Role::SERVER) {
		text += " Server";
	} else if (role == Role::CLIENT1) {
		text += " Player 1";
	} else if (role == Role::CLIENT2) {
		text += " Player 2";
	}

	text += "\n";

	std::stringstream ss;
	ss << pingTime;

	text += "Ping: ";
	text += ss.str();
	text += "\n";

	status->setString(text);
}

void GameScene::updateScore()
{
	auto score1text = dynamic_cast<Text*>(this->getChildByName("Console")->getChildByName("Score1"));
	auto score2text = dynamic_cast<Text*>(this->getChildByName("Console")->getChildByName("Score2"));

	std::stringstream ss1;
	ss1 << score1;

	score1text->setString(ss1.str());

	std::stringstream ss2;
	ss2 << score2;
	score2text->setString(ss2.str());
}

std::string GameScene::createMessage(Opcode opcode, Role target)
{
	rapidjson::Document json;
	json.SetObject();

	json.AddMember("o", opcode, json.GetAllocator());
	json.AddMember("d", target, json.GetAllocator());

	rapidjson::StringBuffer sb;
	rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
	json.Accept(writer);

	return std::string(sb.GetString());
}

void GameScene::send(const std::string& message)
{
	if (websocket == nullptr) {
		return;
	}

	websocket->send(message);
}

void GameScene::sendPing()
{
	pingStartTime = std::chrono::high_resolution_clock::now();
	send(createMessage(Opcode::PING, role));
}

void GameScene::sendPong(Role target)
{
	send(createMessage(Opcode::PONG, target));
}
