#pragma once

#include "Common.h"
#include "Scene.h"
#include <map>
#include "EntityManager.h"
#include <queue>

class Scene_Play : public Scene
{
	struct SpawnPoint {
		std::string     type;
		float           y;
		auto operator<=>(const SpawnPoint& other) const {
			return  y <=> other.y;
		}
	};

	struct EnemyConfig
	{
		float X{ 0.f }, Y{ 0.f }, CW{ 0.f }, CH{ 0.f };
		float SPEED{ 0.f }, MAXSPEED{ 0.f }, JUMP{ 0.f }, GRAVITY{ 0.f };
		float DETECTION_RANGE{ 0.f }, ATTACK_RANGE{ 0.f };
		std::string WEAPON;
		float platformStartX{ 0.f };
		float platformEndX{ 0.f };
	};

	struct PlayerConfig
	{
		float X{ 0.f }, Y{ 0.f }, CW{ 0.f }, CH{ 0.f };
		float SPEED{ 0.f }, MAXSPEED{ 0.f }, JUMP{ 0.f }, GRAVITY{ 0.f };
		std::string WEAPON;
	};

protected:

	std::shared_ptr<Entity>		m_player;
	std::shared_ptr<Entity>		m_key;
	std::shared_ptr<Entity>		m_door;
	std::string					m_levelPath;
	PlayerConfig				m_playerConfig;
	std::vector<EnemyConfig> m_enemyConfigs;
	EnemyConfig					m_enemyConfig;
	std::priority_queue<SpawnPoint>     _spawnPoints;
	bool						m_drawTextures{true};						
	bool						m_drawCollision{true}; 
	bool						m_drawGrid{false};
	int                         collectedCoins{ 0 };
	int                         m_playerArrows{ 5 };
	int                         totalCoins{ 29 };
	bool						m_hasKey{ false };
	const Vec2					m_gridSize{ 50,50 };
	sf::Text					m_gridText;
	sf::Sprite                  m_backgroundSprite;
	Animation m_coinAnimation; 
	Animation m_arrowAnimation;
	float m_ovalAnimationTime{ 0.0f };
	sf::Shader m_glowShader;
	const float POWER_UP_DROP_PROBABILITY = 0.7f; // 30% chance to drop a power-up
	std::map<std::shared_ptr<Entity>, Vec2> m_enemyRespawnPoints; // Store respawn points for enemies


	void	init(const std::string& levelPath);
	void	registerActions();
	void	onEnd() override;


public:
	Scene_Play(GameEngine* gameEngine, const std::string& levelPath);
	
	void update() override;
	void sRender() override;
	void sDoAction(const Action& action) override;
	void updateView();
	void updateBackground();

	void sMovement();
	void sAnimation();
	void sLifespan();
	
	void sCollision();
	void createGround();
	
	void sDebug();
	void drawLine();
	void drawHP(std::shared_ptr<Entity> e);
	void drawCoinsCounter();
	void drawWinScreen();
	void drawLifeSpan();
	void drawArrowsCounter();

	void playerCheckState();
	void respawnPlayer(std::shared_ptr<Entity> player);
	void respawnEnemy(std::shared_ptr<Entity> enemy);


	Vec2 gridToMidPixel(float gridX, float gridY, std::shared_ptr<Entity> entity);
	void loadLevel(const std::string& filename);
	void loadFromFile(const std::string& filename);
	void spawnPlayer();
	void spawnBullet(std::shared_ptr<Entity>);

	void spawnEnemy(const std::vector<EnemyConfig>& configs);
	void sEnemyBehavior();
	void checkWinCondition();
	void checkLoseCondition();

	void meleeAttack(std::shared_ptr<Entity> enemy);
	void rangedAttack(std::shared_ptr<Entity> enemy);
	bool checkPlatformEdge(std::shared_ptr<Entity> enemy);
	void spawnPowerUp(const Vec2& position, const std::string& type);
	void spawnKey(const Vec2& position);
	void spawnDoor(const Vec2& position);

};




