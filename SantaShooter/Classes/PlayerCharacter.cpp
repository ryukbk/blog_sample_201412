#include "PlayerCharacter.h"

USING_NS_CC;
using namespace cocostudio::timeline;

int PlayerCharacter::currentContactBitMask = 0x00000001;

bool PlayerCharacter::init()
{
	if (!Node::init())
	{
		return false;
	}

	animationState = AnimationState::IDLE;
	lastAckTimestamp = 0;
	handShakeDone = false;
	score = 0;

	auto spriteFrameCache = SpriteFrameCache::getInstance();
	idleRight = Sprite::createWithSpriteFrame(spriteFrameCache->getSpriteFrameByName("slice12_12.png"));
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

	schedule(CC_SCHEDULE_SELECTOR(PlayerCharacter::cleanupGiftbox), 2.0f);

	return true;
}

PlayerCharacter::~PlayerCharacter()
{
	walkUpAnimation->release();
	walkDownAnimation->release();
}

void PlayerCharacter::addPhysics()
{
	Size originalSize = idleRight->getContentSize();
	auto boxBody = PhysicsBody::createBox(Size(originalSize.width * PLAYER_SCALE, originalSize.height * PLAYER_SCALE));
	boxBody->setDynamic(true);
	boxBody->setRotationEnable(false);
	boxBody->setMass(0);

	contactBitMask = getNewContactBitMask();
	boxBody->setContactTestBitmask(contactBitMask);
	setPhysicsBody(boxBody);
}

void PlayerCharacter::removePhysics()
{
	if (getPhysicsBody() != nullptr) {
		getPhysicsBody()->removeFromWorld();
		setPhysicsBody(nullptr);
	}
}

void PlayerCharacter::move(bool up)
{
#ifdef MOVE_WITH_PHYSICS
	if (this->getPhysicsBody() != nullptr) {
		this->getPhysicsBody()->setVelocity(Vec2(0, up ? PLAYER_MOVE_SPEED_WITH_PHYSICS : - PLAYER_MOVE_SPEED_WITH_PHYSICS));
	}

	auto hitbox = getChildByName("hitbox");
	if (hitbox != nullptr) {
		hitbox->getPhysicsBody()->setVelocity(Vec2(0, up ? PLAYER_MOVE_SPEED_WITH_PHYSICS : - PLAYER_MOVE_SPEED_WITH_PHYSICS));
	}
#else
	this->stopAllActions();

	Point start = this->getPosition();
	Size visibleSize = Director::getInstance()->getVisibleSize();
	Point end(start.x, up ? visibleSize.height : 0);
	float duration = start.getDistance(end) / PLAYER_MOVE_SPEED_WITHOUT_PHYSICS;
	this->runAction(MoveTo::create(duration, end));
#endif
}

void PlayerCharacter::playWalkUp()
{
	if (animationState == AnimationState::UP) {
		return;
	}

	idleRight->setVisible(false);

	walkDownAnimation->pause();
	walkDown->setVisible(false);

	walkUp->setVisible(true);
	walkUpAnimation->gotoFrameAndPlay(0, true);

	animationState = AnimationState::UP;

	move(true);
}

void PlayerCharacter::playWalkDown()
{
	if (animationState == AnimationState::DOWN) {
		return;
	}

	idleRight->setVisible(false);

	walkUpAnimation->pause();
	walkUp->setVisible(false);

	walkDown->setVisible(true);
	walkDownAnimation->gotoFrameAndPlay(0, true);

	animationState = AnimationState::DOWN;

	move(false);
}

void PlayerCharacter::stop()
{
#ifdef MOVE_WITH_PHYSICS
	if (this->getPhysicsBody() != nullptr) {
		this->getPhysicsBody()->setVelocity(Vec2::ZERO);
	}

	auto hitbox = getChildByName("hitbox");
	if (hitbox != nullptr) {
		this->getPhysicsBody()->setVelocity(Vec2::ZERO);
	}
#endif
}

void PlayerCharacter::stayIdle(bool flipped)
{
	animationState = AnimationState::IDLE;

	stop();

	this->stopAllActions();

	walkUpAnimation->pause();
	walkUp->setVisible(false);

	walkDownAnimation->pause();
	walkDown->setVisible(false);

	if (flipped) {
		static_cast<Sprite*>(idleRight)->setFlippedX(true);
	}

	idleRight->setVisible(true);
}

void PlayerCharacter::attack(
	cocos2d::Node* scene,
	cocos2d::Point touchPoint,
	cocos2d::SpriteFrameCache* spriteFrameCache,
	const cocos2d::Size& visibleSize,
	int targetContactBitMask,
	cocos2d::Sprite* hitbox
)
{
	if (hitbox != nullptr) {
		auto originalPhysicsBody = getPhysicsBody();
		Size originalSize = hitbox->getContentSize();
		auto boxBody = PhysicsBody::createBox(Size(originalSize.width, originalSize.height));
		boxBody->setDynamic(true);
		boxBody->setRotationEnable(false);
		boxBody->setMass(PHYSICS_INFINITY);
		boxBody->setCollisionBitmask(0);
		boxBody->setContactTestBitmask(originalPhysicsBody->getContactTestBitmask());
		originalPhysicsBody->setCategoryBitmask(0);
		hitbox->setPhysicsBody(boxBody);
	}

	auto giftbox = Sprite::createWithSpriteFrame(spriteFrameCache->getSpriteFrameByName("giftbox.png"));
	giftbox->setScale(PLAYER_GIFTBOX_SCALE);

	Size originalSize = giftbox->getContentSize();
	auto boxBody = PhysicsBody::createBox(Size(originalSize.width * PLAYER_GIFTBOX_SCALE, originalSize.height * PLAYER_GIFTBOX_SCALE));
	// If setDynamic(false) collision is not detected
	boxBody->setDynamic(true);
	boxBody->setRotationEnable(false);
	boxBody->setCollisionBitmask(0);
	boxBody->setContactTestBitmask(targetContactBitMask);
	giftbox->setPhysicsBody(boxBody);

	giftbox->setName("giftbox");

	Point start = getPosition();
	
	if (touchPoint.x > visibleSize.width / 2) {
		start.x += 100.0f;
	} else {
		start.x -= 100.0f;
	}

	giftbox->setPosition(start);

	scene->addChild(giftbox, 0);

	float angle = Point(touchPoint - start).getAngle();

#ifdef MOVE_WITH_PHYSICS
	giftbox->getPhysicsBody()->setVelocity(Vec2(cos(angle) * PLAYER_GIFTBOX_SPEED_WITH_PHYSICS, sin(angle) * PLAYER_GIFTBOX_SPEED_WITH_PHYSICS));
#else
	touchPoint.add(Point(cos(angle) * 300, sin(angle) * 300));

	auto callback = CallFunc::create([giftbox]() {
		giftbox->removeFromParent();
	});

	float duration = start.getDistance(touchPoint) / PLAYER_GIFTBOX_SPEED_WITHOUT_PHYSICS;
	auto sequence = Sequence::create(MoveTo::create(duration, touchPoint), callback, NULL);
	giftbox->runAction(sequence);
#endif

	giftboxes.push_back(std::make_pair(giftbox, hitbox));
}

void PlayerCharacter::cleanupGiftbox(float deltaTime)
{
	Size visibleSize = Director::getInstance()->getVisibleSize();
	giftboxes.erase(std::remove_if(giftboxes.begin(), giftboxes.end(), [visibleSize](std::pair<Node*, Node*> g)
	{
		Node* giftbox = g.first;
		Point pos = giftbox->getPosition();
		bool offscreen = (pos.x > visibleSize.width || pos.x < 0 || pos.y < 0 || pos.y > visibleSize.height);
		if (offscreen) {
			if (g.second != nullptr) {
				g.second->removeFromParent();
			}
			giftbox->removeFromParent();
		}

		return offscreen;
	}), giftboxes.end());
}

void PlayerCharacter::removeFromGiftboxes(Node* giftbox)
{
	giftboxes.erase(std::remove_if(giftboxes.begin(), giftboxes.end(), [giftbox](std::pair<Node*, Node*> g) {
		if (g.first == giftbox) {
			if (g.second != nullptr) {
				g.second->removeFromParent();
			}
		}

		return g.first == giftbox;
	}), giftboxes.end());
}

void PlayerCharacter::toggleGiftboxPhysics(bool enabled)
{
	for (auto g: giftboxes) {
		g.first->getPhysicsBody()->setEnable(enabled);
	}
}
