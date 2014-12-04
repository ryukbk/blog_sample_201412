#pragma once

#include "cocos2d.h"
#include "cocostudio/CocoStudio.h"

class PlayerCharacter : public cocos2d::Node
{
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
	void stayIdle();
};
