#include "Scene_Menu.h"
#include "Scene_Play.h"
#include <memory>

void Scene_Menu::onEnd()
{
	m_game->window().close();
}

Scene_Menu::Scene_Menu(GameEngine* gameEngine)
	: Scene(gameEngine)
{
	init();
}



void Scene_Menu::init()
{
    registerAction(sf::Keyboard::W,			"UP");
    registerAction(sf::Keyboard::Up,		"UP");
    registerAction(sf::Keyboard::S,			"DOWN");
    registerAction(sf::Keyboard::Down,	 	"DOWN");
	registerAction(sf::Keyboard::D,			"PLAY");
	registerAction(sf::Keyboard::Escape,	"QUIT");

	m_title = "Abya Yala!"; // Freedom Fighters
	m_subtitle = "Freedom Fighters";
	m_menuStrings.push_back("Level 1");
	m_menuStrings.push_back("Level 2");
	m_menuStrings.push_back("Level 3");

	m_levelPaths.push_back("../assets/level1.txt");
	m_levelPaths.push_back("../assets/level2.txt");
	m_levelPaths.push_back("../assets/level3.txt");

	m_menuText.setFont(m_game->assets().getFont("Megaman"));

	const size_t CHAR_SIZE{ 64 };
	m_menuText.setCharacterSize(CHAR_SIZE);

	loadMenu();

}

void Scene_Menu::update()
{
	m_entityManager.update();
	//sRender();
}

void Scene_Menu::sRender()
{
	sf::View view = m_game->window().getView();
	view.setCenter(m_game->window().getSize().x / 2.f, m_game->window().getSize().y / 2.f);
	m_game->window().setView(view);

	m_game->window().clear();

	// Draw the background sprite
	m_game->window().draw(m_backgroundSprite);

	static const sf::Color selectedColor(255, 255, 255);
	static const sf::Color normalColor(255, 215, 0);

	sf::Text footer("UP: W    DOWN: S   PLAY:D    QUIT: ESC",
		m_game->assets().getFont("Megaman"), 20);
	footer.setFillColor(normalColor);
	footer.setPosition(32, 700);

	// Center the title text
	m_menuText.setFillColor(normalColor);
	m_menuText.setString(m_title);
	sf::FloatRect titleBounds = m_menuText.getLocalBounds();
	m_menuText.setOrigin(titleBounds.left + titleBounds.width / 2.0f, titleBounds.top + titleBounds.height / 2.0f);
	m_menuText.setPosition(m_game->window().getSize().x / 2.0f, 50);
	m_game->window().draw(m_menuText);

	// Center the subtitle text
	m_menuText.setFillColor(normalColor);
	m_menuText.setString(m_subtitle);
	sf::FloatRect subtitleBounds = m_menuText.getLocalBounds();
	m_menuText.setOrigin(subtitleBounds.left + subtitleBounds.width / 2.0f, subtitleBounds.top + subtitleBounds.height / 2.0f);
	m_menuText.setPosition(m_game->window().getSize().x / 2.0f, 150);
	m_game->window().draw(m_menuText);

	// Draw menu options
	for (size_t i{ 0 }; i < m_menuStrings.size(); ++i)
	{
		m_menuText.setFillColor((i == m_menuIndex ? selectedColor : normalColor));
		m_menuText.setString(m_menuStrings.at(i));
		sf::FloatRect menuBounds = m_menuText.getLocalBounds();
		m_menuText.setOrigin(menuBounds.left + menuBounds.width / 2.0f, menuBounds.top + menuBounds.height / 2.0f);
		m_menuText.setPosition(m_game->window().getSize().x / 2.0f, 250 + i * 100);
		m_game->window().draw(m_menuText);
	}

	m_game->window().draw(footer);
	m_game->window().display();
}


void Scene_Menu::sDoAction(const Action& action)
{
	if (action.type() == "START")
	{
		if (action.name() == "UP")
		{
			m_menuIndex = (m_menuIndex + m_menuStrings.size() - 1) % m_menuStrings.size();
		} 
		else if (action.name() == "DOWN")
		{
			m_menuIndex = (m_menuIndex + 1) % m_menuStrings.size();
		}
		else if (action.name() == "PLAY")
		{
			m_game->changeScene("PLAY", std::make_shared<Scene_Play>(m_game, m_levelPaths[m_menuIndex]));
		}
		else if (action.name() == "QUIT")
		{
			onEnd();
		}
	}

}

void Scene_Menu::loadMenu()
{
	if (m_game->assets().getTexture("TexMenu").getSize().x > 0)
	{
		m_backgroundSprite.setTexture(m_game->assets().getTexture("TexMenu")); // Set texture directly
	}
	
	const sf::Texture& backgroundTexture = m_game->assets().getTexture("TexMenu");

	if (backgroundTexture.getSize().x == 0)  // Check if texture loaded
	{
		std::cerr << "ERROR: 'TexMenu' texture failed to load or is empty!" << std::endl;
		return;
	}

	m_menuBackground = backgroundTexture;
	m_backgroundSprite.setTexture(m_menuBackground);
	std::cout << "SUCCESS: Background texture loaded correctly!" << std::endl;
}