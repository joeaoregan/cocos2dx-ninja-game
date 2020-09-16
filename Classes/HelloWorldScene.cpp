/****************************************************************************
 Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.
 
 http://www.cocos2d-x.org
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#include "HelloWorldScene.h"
#include "SimpleAudioEngine.h"

using namespace CocosDenshion;
USING_NS_CC;

#define BACKGROUND_MUSIC_SFX	"audio/background-music-aac.mp3"
#define PEW_PEW_SFX				"audio/pew-pew-lei.mp3"

enum class PhysicsCategory {
	None = 0,
	Monster = (1 << 0), // 1
	Projectile = (1 << 1), // 2
	All = PhysicsCategory::Monster | PhysicsCategory::Projectile // 3
};

Scene* HelloWorld::createScene()
{
	auto scene = Scene::createWithPhysics();
	scene->getPhysicsWorld()->setGravity(Vec2(0, 0));
	scene->getPhysicsWorld()->setDebugDrawMask(PhysicsWorld::DEBUGDRAW_ALL);
	auto layer = HelloWorld::create();
	scene->addChild(layer);
	return scene;
}

// Print useful error message instead of segfaulting when files are not there.
static void problemLoading(const char* filename)
{
    printf("Error while loading: %s\n", filename);
    printf("Depending on how you compiled you might have to add 'Resources/' in front of filenames in HelloWorldScene.cpp\n");
}

// on "init" you need to initialize your instance
bool HelloWorld::init()
{
    //////////////////////////////
    // 1. super init first
    if ( !Scene::init() )
    {
        return false;
    }

	// 2
	auto origin = Director::getInstance()->getVisibleOrigin();
	auto winSize = Director::getInstance()->getVisibleSize();

	// 3 Create a DrawNode to draw a grey rectangle that fills the screen, for game background
	auto background = DrawNode::create();
	background->drawSolidRect(origin, winSize, Color4F(0.6F, 0.6F, 0.6F, 1.0F));
	this->addChild(background);

	// 4 Create the player sprite by passing the image's name
	_player = Sprite::create("sprites/player.png");
	_player->setPosition(Vec2(winSize.width * 0.1, winSize.height * 0.5));
	this->addChild(_player);

	// Add & move monsters
	srand((unsigned int)time(nullptr)); // seed random number generator
	this->schedule(schedule_selector(HelloWorld::addMonster), 1.5); // add monster every 1.5 seconds

	// Add projectiles
	auto eventListener = EventListenerTouchOneByOne::create();
	eventListener->onTouchBegan = CC_CALLBACK_2(HelloWorld::onTouchBegan, this);
	this->getEventDispatcher()->addEventListenerWithSceneGraphPriority(eventListener, _player);

	// Receive contact notifications
	auto contactListener = EventListenerPhysicsContact::create();
	contactListener->onContactBegin = CC_CALLBACK_1(HelloWorld::onContactBegan, this);
	this->getEventDispatcher()->addEventListenerWithSceneGraphPriority(contactListener, this);

	SimpleAudioEngine::getInstance()->playBackgroundMusic(BACKGROUND_MUSIC_SFX, true);

    return true;
}


void HelloWorld::menuCloseCallback(Ref* pSender)
{
    //Close the cocos2d-x game scene and quit the application
    Director::getInstance()->end();

    /*To navigate back to native iOS screen(if present) without quitting the application  ,do not use Director::getInstance()->end() as given above,instead trigger a custom event created in RootViewController.mm as below*/

    //EventCustom customEndEvent("game_scene_close_event");
    //_eventDispatcher->dispatchEvent(&customEndEvent);
}

void HelloWorld::addMonster(float dt) {
	auto monster = Sprite::create("sprites/monster.png");

	// 1
	auto monsterSize = monster->getContentSize();
	auto physicsBody = PhysicsBody::createBox(Size(monsterSize.width, monsterSize.height),
		PhysicsMaterial(0.1f, 1.0f, 0.0f));
	// 2
	physicsBody->setDynamic(true);
	// 3
	physicsBody->setCategoryBitmask((int)PhysicsCategory::Monster);
	physicsBody->setCollisionBitmask((int)PhysicsCategory::None);
	physicsBody->setContactTestBitmask((int)PhysicsCategory::Projectile);
	monster->setPhysicsBody(physicsBody);

	// 1
	auto monsterContentSize = monster->getContentSize();
	auto selfContentSize = this->getContentSize();
	int minY = monsterContentSize.height / 2;
	int maxY = selfContentSize.height - monsterContentSize.height / 2;
	int rangeY = maxY - minY;
	int randomY = (rand() % rangeY) + minY;
	monster->setPosition(Vec2(selfContentSize.width + monsterContentSize.width / 2, randomY));	// random position off screen
	this->addChild(monster);
	// 2
	int minDuration = 2.0;
	int maxDuration = 4.0;
	int rangeDuration = maxDuration - minDuration;
	int randomDuration = (rand() % rangeDuration) + minDuration;
	// 3
	auto actionMove = MoveTo::create(randomDuration, Vec2(-monsterContentSize.width / 2, randomY));
	auto actionRemove = RemoveSelf::create();
	monster->runAction(Sequence::create(actionMove, actionRemove, nullptr));
}

bool HelloWorld::onTouchBegan(Touch *touch, Event *unused_event) {
	// 1 - Just an example for how to get the _player object
	//auto node = unused_event->getCurrentTarget();

	// 2 get the coordinate of the touch, calculate the offset point
	Vec2 touchLocation = touch->getLocation();
	Vec2 offset = touchLocation - _player->getPosition();

	// 3 Player not let shoot backwards
	if (offset.x < 0) {
		return true;
	}

	// 4 Create a projectile
	auto projectile = Sprite::create("sprites/projectile.png");
	projectile->setPosition(_player->getPosition());

	auto projectileSize = projectile->getContentSize();
	auto physicsBody = PhysicsBody::createCircle(projectileSize.width / 2);
	physicsBody->setDynamic(true);
	physicsBody->setCategoryBitmask((int)PhysicsCategory::Projectile);
	physicsBody->setCollisionBitmask((int)PhysicsCategory::None);
	physicsBody->setContactTestBitmask((int)PhysicsCategory::Monster);
	projectile->setPhysicsBody(physicsBody);

	this->addChild(projectile);

	// 5 length enough to extend past screen at current resolution
	offset.normalize();
	auto shootAmount = offset * 1000;

	// 6 Add vector to projectile's position gives target position
	auto realDest = shootAmount + projectile->getPosition();

	// 7 Move the projectile to the target position over 2 seconds
	auto actionMove = MoveTo::create(2.0f, realDest);
	auto actionRemove = RemoveSelf::create();
	projectile->runAction(Sequence::create(actionMove, actionRemove, nullptr));
	
	SimpleAudioEngine::getInstance()->playEffect(PEW_PEW_SFX);

	return true;
}

bool HelloWorld::onContactBegan(PhysicsContact &contact) {
	cocos2d::Node* nodeA = contact.getShapeA()->getBody()->getNode();
	cocos2d::Node* nodeB = contact.getShapeB()->getBody()->getNode();

	if (nodeA != NULL) { // Added check for null
		nodeA->removeFromParent();

		if (nodeB != NULL) {
			nodeB->removeFromParent();
		}
	}

	return true;
}
