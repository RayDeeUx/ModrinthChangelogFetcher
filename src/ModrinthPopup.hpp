#pragma once

class ModrinthPopup final : public geode::Popup {
protected:
	cocos2d::CCMenu* m_buttons {};
	geode::TextInput* m_inputBx{};
	bool init() override;
public:
	static ModrinthPopup* create();
	void onSubmit(CCObject*);
};