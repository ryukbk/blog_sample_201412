#pragma once

#include <vector>
#include <memory>

#include "cocos2d.h"
#include "cocostudio/CocoStudio.h"

#include "PlayerCharacter.h"

class GameScene : public cocos2d::Node
{
public:
    // there's no 'id' in cpp, so we recommend returning the class instance pointer
    static cocos2d::Scene* createScene();

    // Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
    virtual bool init();
    
    // a selector callback
    void menuCloseCallback(cocos2d::Ref* pSender);
    
    // implement the "static create()" method manually
    CREATE_FUNC(GameScene);

	void onTouchesBegan(const std::vector<cocos2d::Touch*>& touches, cocos2d::Event* event);
	void onTouchesEnded(const std::vector<cocos2d::Touch*>& touches, cocos2d::Event* event);

	CC_SYNTHESIZE(PlayerCharacter*, player1, Player1);
	CC_SYNTHESIZE(PlayerCharacter*, player2, Player2);
};

