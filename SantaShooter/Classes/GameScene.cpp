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

	lastAckTickFromServer = 0;
	pingTime = 0;

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

	setupPlayers();

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
			if (role == Role::UNINITIALIZED || role == Role::CLIENT1) {
				player1->setKeyInput(KeyInput::UP);
			}
			break;
		case EventKeyboard::KeyCode::KEY_S:
			if (role == Role::UNINITIALIZED || role == Role::CLIENT1) {
				player1->setKeyInput(KeyInput::DOWN);
			}
			break;
		case EventKeyboard::KeyCode::KEY_UP_ARROW:
			if (role == Role::UNINITIALIZED || role == Role::CLIENT2) {
				player2->setKeyInput(KeyInput::UP);
			}
			break;
		case EventKeyboard::KeyCode::KEY_DOWN_ARROW:
			if (role == Role::UNINITIALIZED || role == Role::CLIENT2) {
				player2->setKeyInput(KeyInput::DOWN);
			}
			break;
		}
	};

	listener->onKeyReleased = [=](EventKeyboard::KeyCode keyCode, Event* event) {
		switch (keyCode) {
		case EventKeyboard::KeyCode::KEY_W:
		case EventKeyboard::KeyCode::KEY_S:
			player1->setKeyInput(KeyInput::STOP);
			break;
		case EventKeyboard::KeyCode::KEY_UP_ARROW:
		case EventKeyboard::KeyCode::KEY_DOWN_ARROW:
			player2->setKeyInput(KeyInput::STOP);
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
	switch (player1->getKeyInput()) {
	case KeyInput::UP:
		player1->playWalkUp();
		break;
	case KeyInput::DOWN:
		player1->playWalkDown();
		break;
	case KeyInput::STOP:
		player1->stayIdle(false);
		break;
	}

	switch (player2->getKeyInput()) {
	case KeyInput::UP:
		player2->playWalkUp();
		break;
	case KeyInput::DOWN:
		player2->playWalkDown();
		break;
	case KeyInput::STOP:
		player2->stayIdle(true);
		break;
	}

	Director::getInstance()->getRunningScene()->getPhysicsWorld()->step(deltaTime);

	if (role == Role::CLIENT1) {
		if (player1->getKeyInput() != KeyInput::IDLE) {
			sendKeyInput(role, player1->getKeyInput());
		}

		addClientActionLog(player1);
	}

	player1->setKeyInput(KeyInput::IDLE);

	if (role == Role::CLIENT2) {
		if (player2->getKeyInput() != KeyInput::IDLE) {
			sendKeyInput(role, player2->getKeyInput());
		}

		addClientActionLog(player2);
	}

	player2->setKeyInput(KeyInput::IDLE);

	if (role == Role::SERVER) {
		if (player1->getHandShakeDone()) {
			sendWorldState(Role::CLIENT1, player1->getLastAckTickSequence());

			auto& positionHistory = player1->getPositionHistory();
			positionHistory.push_front(std::make_pair(lastMessageCreatedTimestamp, player1->getPosition()));
			if (positionHistory.size() > SERVER_POSITION_LOG_CAPACITY) {
				positionHistory.pop_back();
			}
		}

		if (player2->getHandShakeDone()) {
			sendWorldState(Role::CLIENT2, player2->getLastAckTickSequence());

			auto& positionHistory = player2->getPositionHistory();
			positionHistory.push_front(std::make_pair(lastMessageCreatedTimestamp, player2->getPosition()));
			if (positionHistory.size() > SERVER_POSITION_LOG_CAPACITY) {
				positionHistory.pop_back();
			}
		}
	}

	++tickSequence;
}

void GameScene::addConsoleText(std::string text)
{
	if (bailout) {
		return;
	}

	CCLOG(text.c_str());

	consoleLines.push_back(text);
	if (consoleLines.size() > CONSOLE_LOG_LINES) {
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

	if (websocket != nullptr) {
		websocket->close();
		websocket = nullptr;
	}

	bailout = true;

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
			if (role == Role::UNINITIALIZED || role == Role::CLIENT1) {
				player1->attack(this, touch->getLocation(), spriteFrameCache, visibleSize, player2->getContactBitMask(), nullptr);
			}
			if (role == Role::CLIENT1) {
				sendFire(role, touch->getLocation());
			}
		} else {
			CCLOG("Player 2 attack");
			if (role == Role::UNINITIALIZED || role == Role::CLIENT2) {
				player2->attack(this, touch->getLocation(), spriteFrameCache, visibleSize, player1->getContactBitMask(), nullptr);
			}
			if (role == Role::CLIENT2) {
				sendFire(role, touch->getLocation());
			}
		}

		// TODO: add proper support for multitouch
		break;
	}
}

void GameScene::onTouchesEnded(const std::vector<cocos2d::Touch*>& touches, cocos2d::Event* event)
{
	
}

bool GameScene::onContactBegin(const PhysicsContact& contact)
{
	if (role != Role::SERVER && role != Role::UNINITIALIZED) {
		return true;
	}

	if (contact.getShapeA()->getBody()->getNode() == nullptr || contact.getShapeB()->getBody()->getNode() == nullptr) {
		return false;
	}

	std::string nameA = contact.getShapeA()->getBody()->getNode()->getName();
	std::string nameB = contact.getShapeB()->getBody()->getNode()->getName();

	std::string log;
	log += "onContactBegin: ";
	log += nameA;
	log += " - ";
	log += nameB;

	CCLOG(log.c_str());

	if (nameA == "giftbox") {
		Node* giftbox = contact.getShapeA()->getBody()->getNode();
		if (nameB == "player2") {
			player1->setScore(player1->getScore() + 10);
			updateScore();
			player1->removeFromGiftboxes(giftbox);
			player2->removeFromGiftboxes(giftbox);
			giftbox->removeFromParent();
		} else if (nameB == "player1") {
			player2->setScore(player2->getScore() + 10);
			updateScore();
			player1->removeFromGiftboxes(giftbox);
			player2->removeFromGiftboxes(giftbox);
			giftbox->removeFromParent();
		}
	} else if (nameB == "giftbox") {
		Node* giftbox = contact.getShapeB()->getBody()->getNode();
		if (nameA == "player2") {
			player1->setScore(player1->getScore() + 10);
			updateScore();
			player1->removeFromGiftboxes(giftbox);
			player2->removeFromGiftboxes(giftbox);
			giftbox->removeFromParent();
		} else if (nameA == "player1") {
			player2->setScore(player2->getScore() + 10);
			updateScore();
			player1->removeFromGiftboxes(giftbox);
			player2->removeFromGiftboxes(giftbox);
			giftbox->removeFromParent();
		}
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

			std::string log("JSON GetParseError: ");
			log += json.GetParseError();
			log += " ";
			log += data.bytes;
			addConsoleText(log);
			return;
		}

		auto opcode = (Opcode)json["o"].GetInt();
		Role origin = Role(json["d"].GetInt());
		int64_t tick = json["t"].GetInt64();
		int64_t ackTickSequence = json["a"].GetInt64();

		if (role == Role::SERVER && opcode == Opcode::KEY_INPUT) {
			if (origin == Role::CLIENT1) {
				player1->setLastAckTickSequence(tick);
			} else if (origin == Role::CLIENT2) {
				player2->setLastAckTickSequence(tick);
			}
		}

		if (role == Role::CLIENT1 || role == Role::CLIENT2) {
			lastAckTickFromServer = tick;
		}

		switch (opcode) {
		case Opcode::HELLO:
			role = origin;

			updateStatus();

			gameStartTime = std::chrono::high_resolution_clock::now();

			if (role != Role::SERVER) {
				if (role == Role::CLIENT1) {
					player2->removePhysics();
				} else if (role == Role::CLIENT2) {
					player1->removePhysics();
				}

				sendPing();
			}
			break;
		case Opcode::PING:
			if (origin == Role::CLIENT1) {
				player1->setPingStartTime(std::chrono::high_resolution_clock::now());
			} else if (origin == Role::CLIENT2) {
				player2->setPingStartTime(std::chrono::high_resolution_clock::now());
			}

			sendPong(origin, tick);
			break;
		case Opcode::PONG:
			pingTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - pingStartTime).count();
			updateStatus();
			sendHandshakeAck();
			break;
		case Opcode::HANDSHAKE_ACK:
			if (origin == Role::CLIENT1) {
				player1->setPingTime(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - player1->getPingStartTime()).count());
				player1->setHandShakeDone(true);
				std::string log("Player 1 ping: ");
				std::stringstream ss;
				ss << player1->getPingTime();
				addConsoleText(log + ss.str());
			} else if (origin == Role::CLIENT2) {
				player2->setPingTime(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - player2->getPingStartTime()).count());
				player2->setHandShakeDone(true);
				std::string log("Player 2 ping: ");
				std::stringstream ss;
				ss << player2->getPingTime();
				addConsoleText(log + ss.str());
			}
			break;
		case Opcode::WORLD_STATE:
			{
				acceptAuthoritativeWorldState(
					ackTickSequence,
					Point(json["a0"].GetDouble(), json["a1"].GetDouble()),
					Point(json["a2"].GetDouble(), json["a3"].GetDouble()),
					json["a4"].GetInt(),
					Point(json["a5"].GetDouble(), json["a6"].GetDouble()),
					Point(json["a7"].GetDouble(), json["a8"].GetDouble()),
					json["a9"].GetInt()
				);
			}
			break;
		case Opcode::KEY_INPUT:
			if (origin == Role::CLIENT1) {
				player1->setKeyInput((KeyInput)(json["a0"].GetInt()));
			} else if (origin == Role::CLIENT2) {
				player2->setKeyInput((KeyInput)(json["a0"].GetInt()));
			}
			break;
		case Opcode::FIRE:
			{
				Size visibleSize = Director::getInstance()->getVisibleSize();
				auto spriteFrameCache = SpriteFrameCache::getInstance();
				if (role == Role::SERVER) {
					Sprite* hitbox = nullptr;
					if (origin == Role::CLIENT1) {
						hitbox = createLagCompensationHitbox(player1, player2);
						player1->attack(this, Point(json["a0"].GetDouble(), json["a1"].GetDouble()), spriteFrameCache, visibleSize, player2->getContactBitMask(), hitbox);
						sendFire(Role::CLIENT2, Point(json["a0"].GetDouble(), json["a1"].GetDouble()));
					} else if (origin == Role::CLIENT2) {
						hitbox = createLagCompensationHitbox(player2, player1);
						player2->attack(this, Point(json["a0"].GetDouble(), json["a1"].GetDouble()), spriteFrameCache, visibleSize, player1->getContactBitMask(), hitbox);
						sendFire(Role::CLIENT1, Point(json["a0"].GetDouble(), json["a1"].GetDouble()));
					}
				} else if (role == Role::CLIENT1) {
					player2->attack(this, Point(json["a0"].GetDouble(), json["a1"].GetDouble()), spriteFrameCache, visibleSize, player1->getContactBitMask(), nullptr);
				} else if (role == Role::CLIENT2) {
					player1->attack(this, Point(json["a0"].GetDouble(), json["a1"].GetDouble()), spriteFrameCache, visibleSize, player2->getContactBitMask(), nullptr);
				}
			}
			break;
		}
	}
}

cocos2d::Sprite* GameScene::createLagCompensationHitbox(PlayerCharacter* player, PlayerCharacter* opponent)
{
	Sprite* hitbox = nullptr;
	int64_t fireTime = getCurrentTimestamp() - player->getPingTime() / 2;
	auto positionHistory = opponent->getPositionHistory();
	for (auto history : positionHistory) {
		if (history.first < fireTime) {
			auto spriteFrameCache = SpriteFrameCache::getInstance();
			hitbox = Sprite::createWithSpriteFrame(spriteFrameCache->getSpriteFrameByName("slice12_12.png"));
			hitbox->setVisible(false);

			opponent->addChild(hitbox, 0, "hitbox");
			hitbox->setPosition(history.second - opponent->getPosition());

			break;
		}
	}

	return hitbox;
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
	if (role == Role::UNINITIALIZED) {
		return;
	}

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

	if (role != Role::SERVER) {
		std::stringstream ss;
		ss << pingTime;

		text += "Ping: ";
		text += ss.str();
		text += "\n";
	}

	status->setString(text);
}

void GameScene::updateScore()
{
	auto score1text = dynamic_cast<Text*>(this->getChildByName("Console")->getChildByName("Score1"));
	auto score2text = dynamic_cast<Text*>(this->getChildByName("Console")->getChildByName("Score2"));

	std::stringstream ss1;
	ss1 << player1->getScore();

	score1text->setString(ss1.str());

	std::stringstream ss2;
	ss2 << player2->getScore();

	score2text->setString(ss2.str());
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
	send(createMessage(Opcode::PING, lastAckTickFromServer, role));
}

void GameScene::sendKeyInput(Role origin, KeyInput keyInput)
{
	send(createMessage(Opcode::KEY_INPUT, lastAckTickFromServer, origin, (int)keyInput));
}

void GameScene::sendPong(Role target, int64_t ackTickSequence)
{
	send(createMessage(Opcode::PONG, ackTickSequence, target));
}

void GameScene::sendHandshakeAck()
{
	send(createMessage(Opcode::HANDSHAKE_ACK, lastAckTickFromServer, role));
}

void GameScene::sendWorldState(Role target, int64_t ackTickSequence)
{
	// [world state]
	// * player1 : position, velocity, score
	// * player2 : position, velocity, score
	send(createMessage(
		Opcode::WORLD_STATE,
		ackTickSequence,
		target,
		double(player1->getPosition().x),
		double(player1->getPosition().y),
		double(player1->getPhysicsBody()->getVelocity().x),
		double(player1->getPhysicsBody()->getVelocity().y),
		player1->getScore(),
		double(player2->getPosition().x),
		double(player2->getPosition().y),
		double(player2->getPhysicsBody()->getVelocity().x),
		double(player2->getPhysicsBody()->getVelocity().y),
		player2->getScore()
	));
}

void GameScene::sendFire(Role origin, Point point)
{
	if (origin == Role::CLIENT1) {
		send(createMessage(Opcode::FIRE, player1->getLastAckTickSequence(), origin, point.x, point.y));
	} else if (origin == Role::CLIENT2) {
		send(createMessage(Opcode::FIRE, player2->getLastAckTickSequence(), origin, point.x, point.y));
	}
}

void GameScene::addClientActionLog(PlayerCharacter* player)
{
	clientActionLog.push_back(std::make_tuple(tickSequence, player->getKeyInput(), player->getPosition(), player->getPhysicsBody()->getVelocity()));
	if (clientActionLog.size() > CLIENT_ACTION_LOG_CAPACITY) {
		clientActionLog.pop_front();
	}
}

void GameScene::acceptAuthoritativeWorldState(
	int64_t lastAckTickSequence, Point player1Position, Point player1Velocity, int player1Score, Point player2Position, Point player2Velocity, int player2Score)
{
	if (player1->getScore() != player1Score || player2->getScore() != player2Score) {
		player1->setScore(player1Score);
		player2->setScore(player2Score);

		updateScore();
	}

	if (role == Role::CLIENT1) {
		player2->setPosition(player2Position);

		if (player2Velocity.y > 0) {
			player2->playWalkUp();
		} else if (player2Velocity.y == 0) {
			player2->stayIdle(true);
		} else if (player2Velocity.y < 0) {
			player2->playWalkDown();
		}

		rewindAndReplayClientWorldState(player1, player1Position, player1Velocity, lastAckTickSequence);
	}

	if (role == Role::CLIENT2) {
		player1->setPosition(player1Position);

		if (player1Velocity.y > 0) {
			player1->playWalkUp();
		} else if (player1Velocity.y == 0) {
			player1->stayIdle(false);
		} else if (player1Velocity.y < 0) {
			player1->playWalkDown();
		}

		rewindAndReplayClientWorldState(player2, player2Position, player2Velocity, lastAckTickSequence);
	}

}

void GameScene::rewindAndReplayClientWorldState(
	PlayerCharacter* player, Point authoritativePlayerPosition, Point authoritativePlayerVelocity, int64_t lastAckTickSequence)
{
	if (clientActionLog.empty()) {
		player->setPosition(authoritativePlayerPosition);
		return;
	}

	float distance = 0;
	for (auto action : clientActionLog) {
		if (std::get<0>(action) == lastAckTickSequence) {
			distance = std::get<2>(action).distance(authoritativePlayerPosition);
			if (distance != 0) {
				std::stringstream ss;
				ss << distance << " at " << lastAckTickSequence;
				std::string log("Discrepancy: ");
				addConsoleText(log + ss.str());
			}
			break;
		}
	}

	if (distance > REWIND_DISTANCE_THRESHOLD) {
		while (!clientActionLog.empty() && std::get<0>(clientActionLog.front()) <= lastAckTickSequence) {
			clientActionLog.pop_front();
		}

		if (clientActionLog.empty()) {
			std::string log("Rewind with zero replay");
			addConsoleText(log);

			player->setPosition(authoritativePlayerPosition);
			player->getPhysicsBody()->setVelocity(authoritativePlayerVelocity);
		} else {
			std::string log("Rewind for ");
			std::stringstream ss;
			ss << distance << " diff with " << clientActionLog.size() << " logs replay";
			log += ss.str();
			addConsoleText(log);

			player->setPosition(authoritativePlayerPosition);
			player->getPhysicsBody()->setVelocity(authoritativePlayerVelocity);

			for (auto action : clientActionLog) {
				switch (std::get<1>(action)) {
				case KeyInput::UP:
					player->move(true);
					break;
				case KeyInput::DOWN:
					player->move(false);
					break;
				case KeyInput::STOP:
					player->stop();
					break;
				}

				Director::getInstance()->getRunningScene()->getPhysicsWorld()->step(1.0f / 60.0f);
			}
		}
	}
}

