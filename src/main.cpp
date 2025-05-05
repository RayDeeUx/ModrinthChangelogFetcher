#include <Geode/modify/MenuLayer.hpp>
#include "ModrinthPopup.hpp"

using namespace geode::prelude;

class $modify(MyMenuLayer, MenuLayer) {
	bool init() {
		if (!MenuLayer::init()) return false;

		CCNode* rightSideMenu = this->getChildByID("right-side-menu");
		if (!rightSideMenu) return true;

		CCMenuItemSpriteExtra* modrinthButton = CCMenuItemSpriteExtra::create(CircleButtonSprite::create(CCSprite::create("modrinth.png"_spr), CircleBaseColor::DarkPurple), this, menu_selector(MyMenuLayer::onModrinth));
		modrinthButton->setID("modrinth-button"_spr);
		rightSideMenu->addChild(modrinthButton);
		rightSideMenu->updateLayout();

		return true;
	}

	void onModrinth(CCObject* sender) {
		if (!sender || !typeinfo_cast<CCNode*>(sender) || static_cast<CCNode*>(sender)->getID() != "modrinth-button"_spr) return;
		ModrinthPopup::create()->show();
	}
};