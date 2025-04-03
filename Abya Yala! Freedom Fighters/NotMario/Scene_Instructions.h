#pragma once

#include "Scene.h"
#include "GameEngine.h"
#include <SFML/Graphics.hpp>

class Scene_Instructions : public Scene
{
private:
    sf::Text m_instructionsText;
    sf::Sprite m_backgroundSprite;
    void init();
    void onEnd() override; // Declare the onEnd method

public:
    Scene_Instructions(GameEngine* gameEngine);

    void update() override;
    void sRender() override;
    void sDoAction(const Action& action) override;
};

