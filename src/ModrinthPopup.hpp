#pragma once

class ModrinthPopup final : public geode::Popup<const std::string&> {
protected:
	cocos2d::CCMenu* m_buttons {};
	geode::TextInput* m_inputBx{};
	bool setup(const std::string &) override;
	bool setup();
public:
	static ModrinthPopup* create();
	void onSubmit(CCObject*);
};