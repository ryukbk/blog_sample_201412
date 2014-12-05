#include "PlayerCharacter.h"

USING_NS_CC;
using namespace cocostudio::timeline;

bool PlayerCharacter::init()
{
	if (!Node::init())
	{
		return false;
	}

	auto spriteFrameCache = SpriteFrameCache::getInstance();
	auto idleRight = Sprite::createWithSpriteFrame(spriteFrameCache->getSpriteFrameByName("slice12_12.png"));
	this->addChild(idleRight, 0, "IDLE_RIGHT");

	walkUp = CSLoader::createNode("WalkUp.csb");
	walkDown = CSLoader::createNode("WalkDown.csb");

	walkUpAnimation = CSLoader::createTimeline("WalkUp.csb");
	walkUpAnimation->retain();

	walkDownAnimation = CSLoader::createTimeline("WalkDown.csb");
	walkDownAnimation->retain();

	this->addChild(walkUp, 0, "WALK_UP");
	walkUp->runAction(walkUpAnimation);
	walkUp->setVisible(false);

	this->addChild(walkDown, 0, "WALK_DOWN");
	walkDown->runAction(walkDownAnimation);
	walkDown->setVisible(false);

	setScale(PLAYER_SCALE);

	return true;
}

PlayerCharacter::~PlayerCharacter()
{
	walkUpAnimation->release();
	walkDownAnimation->release();
}

void PlayerCharacter::Move(bool up)
{
	this->stopAllActions();

	Point start = this->getPosition();
	Size visibleSize = Director::getInstance()->getVisibleSize();
	Point end(start.x, up ? visibleSize.height : 0);
	float duration = start.getDistance(end) / PLAYER_MOVE_SPEED;
	this->runAction(MoveTo::create(duration, end));
}

void PlayerCharacter::playWalkUp()
{
	auto idleRight = this->getChildByName("IDLE_RIGHT");
	idleRight->setVisible(false);

	walkDownAnimation->pause();
	walkDown->setVisible(false);

	walkUp->setVisible(true);
	walkUpAnimation->gotoFrameAndPlay(0, 40, true);

	Move(true);
}

void PlayerCharacter::playWalkDown()
{
	auto idleRight = this->getChildByName("IDLE_RIGHT");
	idleRight->setVisible(false);

	walkUpAnimation->pause();
	walkUp->setVisible(false);

	walkDown->setVisible(true);
	walkDownAnimation->gotoFrameAndPlay(0, 40, true);

	Move(false);
}

void PlayerCharacter::stayIdle(bool flipped)
{
	this->stopAllActions();

	walkUpAnimation->pause();
	walkUp->setVisible(false);

	walkDownAnimation->pause();
	walkDown->setVisible(false);

	auto idleRight = this->getChildByName("IDLE_RIGHT");
	if (flipped) {
		static_cast<Sprite*>(idleRight)->setFlippedX(true);
	}

	idleRight->setVisible(true);
}

void PlayerCharacter::attack(cocos2d::Node* scene, cocos2d::Touch* touch, cocos2d::SpriteFrameCache* spriteFrameCache, const cocos2d::Size& visibleSize)
{
	auto giftbox = Sprite::createWithSpriteFrame(spriteFrameCache->getSpriteFrameByName("giftbox.png"));
	giftbox->setScale(3.0f);
	scene->addChild(giftbox, 0);

	giftbox->setPosition(getPosition());

	Point start = giftbox->getPosition();
	Point end = touch->getLocation();
	float angle = Point(end - start).getAngle();

	end.add(Point(cos(angle) * 200, sin(angle) * 200));

	auto callback = CallFunc::create([giftbox]() {
		giftbox->removeFromParent();
	});

	float duration = start.getDistance(end) / 2500;
	auto sequence = Sequence::create(MoveTo::create(duration, end), callback, NULL);
	giftbox->runAction(sequence);
}
