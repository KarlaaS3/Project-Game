#pragma once

#include "EntityManager.h"
#include "Common.h"
#include "GameEngine.h"
#include "Action.h"
#include <map>
#include <string>


using ActionMap = std::map<int, std::string>;

class GameEngine;

class Scene
{

protected:

	GameEngine		*m_game;
	EntityManager	m_entityManager;
	ActionMap		m_actions;
	bool			m_isPaused{false};
	bool			m_hasEnded{false};
	size_t			m_currentFrame{ 0 };

	virtual void	onEnd() = 0;
	void			setPaused(bool paused);

public:
	Scene(GameEngine* gameEngine);
    virtual ~Scene();

	virtual void		update() = 0;
	virtual void		sDoAction(const Action& action) = 0;
	virtual void		sRender() = 0;

	void				simulate(int);
	void				doAction(Action);
	void				registerAction(int, std::string);
	const ActionMap		getActionMap() const;
};

