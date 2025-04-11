#pragma once
#include "Scene.h"
#include "GameEngine.h"
#include <SFML/Graphics.hpp>

class Scene_Instructions : public Scene
{
private:
    sf::Text m_additionalInstructionsText;
    sf::Sprite m_backgroundSprite;
    sf::View m_view;
    void init();
    void onEnd() override; 

public:
    Scene_Instructions(GameEngine* gameEngine);

    void update() override;
    void sRender() override;
    void sDoAction(const Action& action) override;
};

