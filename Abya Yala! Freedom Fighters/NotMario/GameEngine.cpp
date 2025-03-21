#include "GameEngine.h"
#include "Assets.h"
#include "Scene_Play.h"
#include "Scene_Menu.h"
#include "Action.h"



GameEngine::GameEngine(const std::string& path)
{
	init(path);
}


void GameEngine::init(const std::string& path)
{
	m_assets.loadFromFile(path);

    m_window.create(sf::VideoMode(1280, 768), "Not Mario");
    //m_window.create(sf::VideoMode(2560, 1536), "Not Mario");
	//m_window.setFramerateLimit(60);

	changeScene("MENU", std::make_shared<Scene_Menu>(this));
}

void GameEngine::update()
{

	//sUserInput();
	//currentScene()->update();

}

void GameEngine::sUserInput()
{
	sf::Event event;
	while (m_window.pollEvent(event))
	{
		if (event.type == sf::Event::Closed)  
			quit();  

		if (event.type == sf::Event::KeyPressed || event.type == sf::Event::KeyReleased)
		{
			if (currentScene()->getActionMap().contains(event.key.code))
			{
				std::string actionType = (event.type == sf::Event::KeyPressed) ? "START" : "END";
				currentScene()->doAction(Action(currentScene()->getActionMap().at(event.key.code), actionType));
			}
		}
	}
}

std::shared_ptr<Scene> GameEngine::currentScene()
{
	return m_sceneMap.at(m_currentScene);
}

void GameEngine::changeScene(const std::string& sceneName, std::shared_ptr<Scene> scene, bool endCurrentScene)
{
	if (endCurrentScene)
		m_sceneMap.erase(m_currentScene);

	if (!m_sceneMap.contains(sceneName))
		m_sceneMap[sceneName] = scene;

	m_currentScene = sceneName;
}


void GameEngine::quit()
{
	m_window.close();
}


void GameEngine::run()
{

	const sf::Time SPF = sf::seconds(1.0f / 60.f);  // Fixed update time step (60 FPS)

	sf::Clock clock;
	sf::Time timeSinceLastUpdate = sf::Time::Zero;

	while (isRunning())
	{
		timeSinceLastUpdate += clock.restart(); // Get time elapsed since last frame

		while (timeSinceLastUpdate > SPF)  // Ensure fixed time step
		{
			updateDeltaTime();  // Update deltaTime properly here

			sUserInput();       // Get user input
			currentScene()->update(); // Update world logic

			timeSinceLastUpdate -= SPF;
		}

		currentScene()->sRender();  // Render world
	}
}


sf::RenderWindow& GameEngine::window()
{
	return m_window;
}


const Assets& GameEngine::assets() const
{
	return m_assets;
}


bool GameEngine::isRunning()
{
	return (m_running && m_window.isOpen());
}
