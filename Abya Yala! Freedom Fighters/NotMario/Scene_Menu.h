#pragma once

#include "Scene.h"

class Scene_Menu : public Scene
{
private:
	std::vector<std::string>	m_menuStrings;
	sf::Text					m_menuText;
	std::vector<std::string>	m_levelPaths;
	int							m_menuIndex{0};
	std::string					m_title;
	std::string                 m_subtitle;
	sf::Texture m_menuBackground; // Path to background image
	sf::Sprite  m_backgroundSprite;

	void loadMenu();


	void init();
	void onEnd() override;
public:

	Scene_Menu(GameEngine* gameEngine);

	void update() override;

	void sRender() override;
	void sDoAction(const Action& action) override;
	

};

