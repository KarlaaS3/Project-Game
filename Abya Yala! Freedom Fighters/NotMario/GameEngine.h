#pragma once

#include "Common.h"
 
#include "Assets.h"

#include <memory>
#include <map>

class Scene;

using SceneMap = std::map<std::string, std::shared_ptr<Scene>>;

class GameEngine
{

public:
	sf::RenderWindow	m_window;
	Assets				m_assets;
	std::string			m_currentScene;
	SceneMap			m_sceneMap;
	size_t				m_simulationSpeed{ 1 };
	bool				m_running{ true };
	float               m_deltaTime = 0.0f;
	sf::Clock m_clock;


public:
	void init(const std::string& path);
	void update();

	void sUserInput();

	std::shared_ptr<Scene> currentScene();

public:

	GameEngine(const std::string& path);
	void changeScene(const std::string& sceneName,
                     std::shared_ptr<Scene> scene,
                     bool endCurrentScene = false);

	void quit();
	void run();

	sf::RenderWindow& window();
	const Assets& assets() const;
	bool isRunning();

	void updateDeltaTime() {
		m_deltaTime = m_clock.restart().asSeconds();
	}

	float deltaTime() const { return m_deltaTime; }

};

