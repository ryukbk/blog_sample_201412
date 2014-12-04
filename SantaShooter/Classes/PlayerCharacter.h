#pragma once

#include "cocos2d.h"
#include "cocostudio/CocoStudio.h"

static const float PLAYER_X_POS = 80.0f;
static const float PLAYER_MOVE_SPEED = 150.0f;

class PlayerCharacter : public cocos2d::Node
{
private:
	void Move(bool up);

public:
	// Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
	virtual bool init();

	// implement the "static create()" method manually
	CREATE_FUNC(PlayerCharacter);

	virtual ~PlayerCharacter();

	CC_SYNTHESIZE(Node*, walkUp, WalkUp);
	CC_SYNTHESIZE(Node*, walkDown, WalkDown);
	CC_SYNTHESIZE(cocostudio::timeline::ActionTimeline*, walkUpAnimation, WalkUpAnimation);
	CC_SYNTHESIZE(cocostudio::timeline::ActionTimeline*, walkDownAnimation, WalkDownAnimation);

	void playWalkUp();
	void playWalkDown();
	void stayIdle(bool flipped);
};
