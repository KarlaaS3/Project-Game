#pragma once

#include "Scene.h"
#include "GameEngine.h"
#include "TransitionEffect.h"
#include <SFML/Graphics.hpp>

class Scene_Menu : public Scene
{
private:
    std::vector<std::string> m_menuStrings;
    sf::Text m_menuText;
    std::vector<std::string> m_levelPaths;
    int m_menuIndex{ 0 };
    std::string m_title;
    std::string m_subtitle;
    sf::Texture m_menuBackground;
    sf::Sprite m_backgroundSprite;
    sf::RectangleShape m_highlightRect;
    sf::SoundBuffer m_hoverSoundBuffer;
    sf::SoundBuffer m_selectSoundBuffer;
    sf::Sound m_hoverSound;
    sf::Sound m_selectSound;
    TransitionEffect m_transitionEffect;
    bool m_sceneChangePending{ false };
    bool m_applyTransition{ false }; 
    sf::Music m_backgroundMusic;
    sf::Sprite m_animationSprite;
	sf::Sprite m_Anim2;
	sf::Sprite m_Anim3;
    std::vector<Animation> m_coinAnimations;
    sf::Color m_gradientTop;
    sf::Color m_gradientBottom;

    void loadMenu();
    void init();
    void onEnd() override;

public:
    Scene_Menu(GameEngine* gameEngine);
    void update() override;
    void sRender() override;
    void sDoAction(const Action& action) override;
};
