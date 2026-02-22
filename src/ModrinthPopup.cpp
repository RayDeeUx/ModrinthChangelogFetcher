// thank you flingus for the base code

#include "ModrinthPopup.hpp"
#include <Geode/utils/web.hpp>

using namespace geode::prelude;

bool ModrinthPopup::init() {
	if (!geode::Popup::init(330.f, 120.f, "GJ_square01.png")) return false;

	this->setID("ModrinthPopup"_spr);
	this->setTitle("Enter Modrinth Mod ID");

	m_inputBx = TextInput::create(250.f, "Modrinth Mod ID", "bigFont.fnt");
	m_inputBx->setScale(1.2);
	m_inputBx->setMaxCharCount(99);
	m_inputBx->setFilter("1234567890!@#$%^&*()qwertyuiopasdfghjklzxcvbnmMNBVCXZLKJHGFDSAPOIUYTREWQ[]\\{}|;':\",./<>?-_=+");
	m_mainLayer->addChildAtPosition(m_inputBx, Anchor::Center, {0, 7});

	CCMenu* m_buttons = CCMenu::create();
	m_mainLayer->addChildAtPosition(m_buttons, geode::Anchor::Bottom, {0, 25});
	CCMenuItemSpriteExtra* submitBtn = CCMenuItemSpriteExtra::create(
		ButtonSprite::create("Submit", "goldFont.fnt", "GJ_button_01.png", 0.8f), this, menu_selector(ModrinthPopup::onSubmit)
	);
	submitBtn->setID("submit-button"_spr);
	m_buttons->addChildAtPosition(submitBtn, Anchor::Bottom, {0, 25});
	m_buttons->setLayout(AxisLayout::create());
	return true;
}

void ModrinthPopup::onSubmit(CCObject* sender) {
	if (!sender || !typeinfo_cast<CCNode*>(sender) || static_cast<CCNode*>(sender)->getID() != "submit-button"_spr) return;
	const std::string& modIDMaybe = m_inputBx->getString();
	auto req = web::WebRequest();

	req.userAgent("Mozilla/5.0 (Macintosh; Intel Mac OS X 10.15; rv:128.0) Gecko/20100101 Firefox/128.0");

	req.header("Content-Type", "application/json");
	req.timeout(std::chrono::seconds(5));

	const std::string& formattedURL = fmt::format("https://api.modrinth.com/v2/project/{}/version", modIDMaybe);

	async::TaskHolder<WebResponse> listener;

	listener.spawn(
		req.get(formattedURL),
		[] (web::WebResponse* res) {
		if (!res->ok()) return geode::Notification::create("Request failed! (Response was not OK)", NotificationIcon::Error, 5.f)->show();
		log::info("success?");

		auto resStr = res->string();
		if (resStr.isErr()) return geode::Notification::create("Request failed! (Response was not a string)", NotificationIcon::Error, 5.f)->show();

		const std::string& str = resStr.unwrap();
		if (str.empty()) return geode::Notification::create("Request failed! (Response was an empty string)", NotificationIcon::Error, 5.f)->show();

		auto parsedJson = matjson::parse(str);
		if (parsedJson.isErr()) return geode::Notification::create("Request failed! (Response could not be parsed as JSON)", NotificationIcon::Error, 5.f)->show();

		const auto json = parsedJson.unwrap();

		if (json.contains("error")) {
			if (json.contains("description")) return geode::Notification::create(fmt::format("Request failed! ({})", json["description"].asString().unwrap()), NotificationIcon::Error, 5.f)->show();
			return geode::Notification::create(fmt::format("Request failed! ({})", json["error"].asString().unwrap()), NotificationIcon::Error, 5.f)->show();
		}

		const auto jsonAsArray = json.asArray();
		if (jsonAsArray.isErr()) {
			log::info("————— JSON IS NOT ARRAY —————\n\n{}\n\n————— JSON IS NOT ARRAY —————", str);
			return geode::Notification::create("Request failed! JSON recieved but is not array", NotificationIcon::Error, 5.f)->show();
		}

		bool foundLatestVersion = false;
		for (const auto entry : jsonAsArray.unwrap()) {
			if (!entry.contains("game_versions") || !entry.contains("loaders") || !entry.contains("version_number") || !entry.contains("changelog") || !entry.contains("name")) {
				log::info("————— STUFF WENT WRONG —————\n\n{}\n\n————— STUFF WENT WRONG —————", str);
				return geode::Notification::create("Request failed! Check logs.", NotificationIcon::Error, 5.f)->show();
			}

			const auto gameVersArray = entry["game_versions"].asArray();
			const auto loadersArray = entry["loaders"].asArray();

			if (gameVersArray.isErr() || loadersArray.isErr()) {
				log::info("————— STUFF WENT WRONG —————\n\n{}\n\n————— STUFF WENT WRONG —————", str);
				return geode::Notification::create("Request failed! Check logs.", NotificationIcon::Error, 5.f)->show();
			}

			bool foundPreferredGameVersion = false;
			for (const auto version : gameVersArray.unwrap()) {
				if (utils::string::toLower(version.asString().unwrap()) == utils::string::toLower(Mod::get()->getSettingValue<std::string>("gameVersion"))) {
					foundPreferredGameVersion = true;
					break;
				}
			}

			if (!foundPreferredGameVersion) continue;

			bool foundPreferredLoader = false;
			for (const auto loader : loadersArray.unwrap()) {
				if (utils::string::toLower(loader.asString().unwrap()) == utils::string::toLower(Mod::get()->getSettingValue<std::string>("modLoader"))) {
					foundPreferredLoader = true;
					break;
				}
			}

			if (!foundPreferredLoader) continue;

			foundLatestVersion = foundPreferredGameVersion && foundPreferredLoader;

			if (foundLatestVersion) {
				const std::string& changelogAsString = entry["changelog"].dump();
				geode::MDPopup::create(entry["name"].asString().unwrap(), changelogAsString, "Close", "Copy", [changelogAsString](bool buttonTwo) {
					if (buttonTwo) geode::utils::clipboard::write(changelogAsString);
				})->show();
				break;
			}
		}
		if (!foundLatestVersion) return geode::Notification::create("There is no latest changelog available.", NotificationIcon::Error, 5.f)->show();
	});
}

ModrinthPopup* ModrinthPopup::create() {
	auto ret = new ModrinthPopup();
	if (ret->init()) {
		ret->autorelease();
		return ret;
	}
	delete ret;
	return nullptr;
}