#pragma once

#include <vector>
#include <deque>
#include <memory>
#include <chrono>
#include <string>
#include <sstream>

#include "cocos2d.h"
#include "cocostudio/CocoStudio.h"
#include "ui/CocosGUI.h"
#include "network/WebSocket.h"

#include "../external/json/document.h"
#include "../external/json/writer.h"
#include "../external/json/stringbuffer.h"

#include "PlayerCharacter.h"

enum class Role
{
	UNINITIALIZED = 0,
	SERVER,
	CLIENT1,
	CLIENT2
};

enum class Opcode
{
	HELLO = 0,
	PING = 1,
	PONG = 2,
};

class GameScene : public cocos2d::Node, public cocos2d::network::WebSocket::Delegate
{
private:
	int score1 = 0;
	int score2 = 0;

	cocos2d::network::WebSocket* websocket = nullptr;

	Role role = Role::UNINITIALIZED;
	std::deque<std::string> consoleLines;
	std::chrono::system_clock::time_point pingStartTime;
	std::chrono::system_clock::duration pingTime;

	void setupPlayers();
	void updateStatus();
	void updateScore();

	void addConsoleText(std::string text);
	void sendPing();
	void sendPong(Role target);

public:
    // there's no 'id' in cpp, so we recommend returning the class instance pointer
    static cocos2d::Scene* createScene();

    // Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
    virtual bool init();
    
	virtual void update(float deltaTime);

    // a selector callback
    void menuCloseCallback(cocos2d::Ref* pSender);
    
    // implement the "static create()" method manually
    CREATE_FUNC(GameScene);

	virtual void onOpen(cocos2d::network::WebSocket* ws);
	virtual void onMessage(cocos2d::network::WebSocket* ws, const cocos2d::network::WebSocket::Data& data);
	virtual void onClose(cocos2d::network::WebSocket* ws);
	virtual void onError(cocos2d::network::WebSocket* ws, const cocos2d::network::WebSocket::ErrorCode& error);

	void onTouchesBegan(const std::vector<cocos2d::Touch*>& touches, cocos2d::Event* event);
	void onTouchesEnded(const std::vector<cocos2d::Touch*>& touches, cocos2d::Event* event);
	bool onContactBegin(const cocos2d::PhysicsContact& contact);
	void onConnectButtonPressed(Ref* pSender, cocos2d::ui::Widget::TouchEventType type);

	CC_SYNTHESIZE(PlayerCharacter*, player1, Player1);
	CC_SYNTHESIZE(PlayerCharacter*, player2, Player2);
};

