#pragma once

#include <string>

#include "cocos2d.h"
#include "cocostudio/CocoStudio.h"

static const float PLAYER_X_POS = 80.0f;
static const float PLAYER_SCALE = 5.0f;
static const float PLAYER_MOVE_SPEED = 200.0f;

static const float PLAYER_GIFTBOX_SCALE = 3.0f;

class PlayerCharacter : public cocos2d::Node
{
private:
	void Move(bool up);

	static int currentContactBitMask;
	static int getNewContactBitMask()
	{
		int v = currentContactBitMask;
		currentContactBitMask <<= 1;
		return v;
	}

public:
	// Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
	virtual bool init();

	// implement the "static create()" method manually
	CREATE_FUNC(PlayerCharacter);

	virtual ~PlayerCharacter();

	CC_SYNTHESIZE(cocos2d::Node*, walkUp, WalkUp);
	CC_SYNTHESIZE(cocos2d::Node*, walkDown, WalkDown);
	CC_SYNTHESIZE(cocostudio::timeline::ActionTimeline*, walkUpAnimation, WalkUpAnimation);
	CC_SYNTHESIZE(cocostudio::timeline::ActionTimeline*, walkDownAnimation, WalkDownAnimation);
	CC_SYNTHESIZE(int, contactBitMask, ContactBitMask);

	void playWalkUp();
	void playWalkDown();
	void stayIdle(bool flipped);
	void attack(
		cocos2d::Node* scene,
		cocos2d::Touch* touch,
		cocos2d::SpriteFrameCache* spriteFrameCache,
		const cocos2d::Size& visibleSize,
		int targetContactBitMask
	);
};
