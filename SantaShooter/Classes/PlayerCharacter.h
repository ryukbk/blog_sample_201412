#pragma once

#include <string>
#include <list>
#include <deque>
#include <utility>
#include <algorithm>

#include "cocos2d.h"
#include "cocostudio/CocoStudio.h"

#define MOVE_WITH_PHYSICS 1

static const float PLAYER_X_POS = 80.0f;
static const float PLAYER_SCALE = 5.0f;
static const float PLAYER_MOVE_SPEED_WITH_PHYSICS = 200.0f;
static const float PLAYER_MOVE_SPEED_WITHOUT_PHYSICS = 200.0f;

static const float PLAYER_GIFTBOX_SCALE = 3.0f;
static const float PLAYER_GIFTBOX_SPEED_WITHOUT_PHYSICS = 2500.0f;
static const float PLAYER_GIFTBOX_SPEED_WITH_PHYSICS = 600.0f;

enum class KeyInput
{
	IDLE,
	UP,
	DOWN,
	STOP
};

class PlayerCharacter : public cocos2d::Node
{
private:
	std::list<std::pair<cocos2d::Node*, cocos2d::Node*>> giftboxes;
	std::deque<std::pair<int64_t, cocos2d::Point>> positionHistory;

	static int currentContactBitMask;
	static int getNewContactBitMask()
	{
		int v = currentContactBitMask;
		currentContactBitMask <<= 1;
		return v;
	}

	enum class AnimationState
	{
		IDLE,
		UP,
		DOWN,
	};

public:
	// Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
	virtual bool init();

	// implement the "static create()" method manually
	CREATE_FUNC(PlayerCharacter);

	virtual ~PlayerCharacter();

	void addPhysics();
	void removePhysics();

	void cleanupGiftbox(float deltaTime);
	void removeFromGiftboxes(Node* giftbox);

	CC_SYNTHESIZE(cocos2d::Sprite*, idleRight, IdleRight);
	CC_SYNTHESIZE(cocos2d::Node*, walkUp, WalkUp);
	CC_SYNTHESIZE(cocos2d::Node*, walkDown, WalkDown);
	CC_SYNTHESIZE(cocostudio::timeline::ActionTimeline*, walkUpAnimation, WalkUpAnimation);
	CC_SYNTHESIZE(cocostudio::timeline::ActionTimeline*, walkDownAnimation, WalkDownAnimation);
	CC_SYNTHESIZE(int, contactBitMask, ContactBitMask);
	CC_SYNTHESIZE(int, score, Score);
	CC_SYNTHESIZE(int64_t, lastAckTimestamp, LastAckTimestamp);
	CC_SYNTHESIZE(KeyInput, keyInput, KeyInput);
	CC_SYNTHESIZE(AnimationState, animationState, AnimationState);
	CC_SYNTHESIZE(bool, handShakeDone, HandShakeDone);
	CC_SYNTHESIZE(int64_t, pingTime, PingTime);
	CC_SYNTHESIZE(std::chrono::high_resolution_clock::time_point, pingStartTime, PingStartTime);

	void playWalkUp();
	void playWalkDown();
	void move(bool up);
	void stop();
	void stayIdle(bool flipped);
	void attack(
		cocos2d::Node* scene,
		cocos2d::Point touchPoint,
		cocos2d::SpriteFrameCache* spriteFrameCache,
		const cocos2d::Size& visibleSize,
		int targetContactBitMask,
		cocos2d::Sprite* hitbox
	);

	inline std::deque<std::pair<int64_t, cocos2d::Point>>& getPositionHistory()
	{
		return positionHistory;
	}
	void toggleGiftboxPhysics(bool enabled);
};
