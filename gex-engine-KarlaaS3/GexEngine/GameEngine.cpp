#include "GameEngine.h"
#include "Assets.h"
#include "Scene_Menu.h"
#include "Scene_Level1.h"
#include "Command.h"
#include <fstream>
#include <memory>
#include <cstdlib>
#include <iostream>

void GameEngine::init(const std::string& path)
{
	unsigned int width = 800;
	unsigned int height = 600;
	loadConfigFromFile(path, width, height);
	_window.create(sf::VideoMode(width, height), "Game Engine");
	_window.setFramerateLimit(60);
	_sceneMap["MENU"] = std::make_shared<Scene_Menu>(this);
	_sceneMap["LEVEL1"] = std::make_shared<Scene_Level1>(this);
	_currentScene = "MENU";
	

}

void GameEngine::update()
{
	//sUserInput();
	//currentScene()->update();
}

void GameEngine::sUserInput()
{
	sf::Event event;
	while (_window.pollEvent(event))
	{
		if (event.type == sf::Event::Closed)
			quit();

		if (event.type == sf::Event::KeyPressed || event.type == sf::Event::KeyReleased)
		{
			if (currentScene()->getActionMap().contains(event.key.code))
			{
				const std::string actionType = (event.type == sf::Event::KeyPressed) ? "START" : "END";
				currentScene()->doAction(Command{ actionType, currentScene()->getActionMap().at(event.key.code) });
			}
		}	
	}

}

std::shared_ptr<Scene> GameEngine::currentScene()
{
	return _sceneMap.at(_currentScene);
}

GameEngine::GameEngine(const std::string& path)
{
	Assets::getInstance().loadFromFile("../config.txt");
	init(path);

}

void GameEngine::changeScene(const std::string& sceneName, std::shared_ptr<Scene> scene, bool endCurrentScene)
{
	if (endCurrentScene)
	{
		_sceneMap.erase(_currentScene);
	}
	if(!_sceneMap.contains(sceneName))
		_sceneMap[sceneName] = scene;
	_currentScene = sceneName;
}

void GameEngine::quit()
{
	_window.close();
}

void GameEngine::run()
{
	sf::Clock clock;
	sf::Time timeSinceLastUpdate = sf::Time::Zero;
	sf::Time timePerFrame = sf::seconds(1.f / 60.f);

	while (isRunning())
	{
		sf::Time dt = clock.restart();
		timeSinceLastUpdate += dt;
		while (timeSinceLastUpdate > timePerFrame)
		{
			timeSinceLastUpdate -= timePerFrame;
			sUserInput();
			update();
		}
		_window.clear();
		currentScene()->sRender();
		_window.display();
	}
	quit();
}


void GameEngine::quitLevel()
{
	changeScene("MENU", nullptr, true);
}

void GameEngine::backLevel()
{
	changeScene("MENU", nullptr, false);	
}

sf::RenderWindow& GameEngine::window()
{
	return _window;
}

sf::Vector2f GameEngine::windowSize() const
{
	return sf::Vector2f{ _window.getSize() };
}

bool GameEngine::isRunning()
{

	return (_running && _window.isOpen());
}

void GameEngine::loadConfigFromFile(const std::string& path, unsigned int& width, unsigned int& height) const
{
	std::ifstream config(path);
	if (config.fail()) {
		std::cerr << "Open file " << path << " failed\n";
		exit(1);
	}

	std::string token;
	while (config >> token) {
		if (token == "Window") {
			config >> width >> height;
		}
		else if (token == "Font") {
			std::string name, path;
			config >> name >> path;
			Assets::getInstance().addFont(name, path);
		}
		else if (token == "Sound") {
			std::string name, path;
			config >> name >> path;
			Assets::getInstance().addSound(name, path);
		}
		else if (token == "Texture") {
			std::string name, path;
			config >> name >> path;
			Assets::getInstance().addTexture(name, path);
		}
		else if (token[0] == '#') {
			config.ignore(std::numeric_limits<std::streamsize>::max(), '\n');  // Ignore comments
		}
		else {
			std::cerr << "Unknown token in config file: " << token << "\n";
		}
	}

	config.close();
}
