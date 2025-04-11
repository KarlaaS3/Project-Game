#include "Scene_Instructions.h"
#include "Scene_Play.h"
#include <iostream>

Scene_Instructions::Scene_Instructions(GameEngine* gameEngine)
    : Scene(gameEngine)
{
    init();
}

void Scene_Instructions::init()
{
    registerAction(sf::Keyboard::Escape, "QUIT");
    registerAction(sf::Keyboard::Enter, "PLAY"); // Register the "PLAY" action

    // Set the background texture to "Instructions"
    if (m_game->assets().getTexture("Instructions").getSize().x > 0)
    {
        m_backgroundSprite.setTexture(m_game->assets().getTexture("Instructions"));

        // Customize the background sprite
        m_backgroundSprite.setPosition(0, 0); // Set the position
        m_backgroundSprite.setScale(
            static_cast<float>(m_game->window().getSize().x) / m_backgroundSprite.getTexture()->getSize().x,
            static_cast<float>(m_game->window().getSize().y) / m_backgroundSprite.getTexture()->getSize().y
        ); // Scale to fit the window
    }

    // Customize the additional instructions text
    m_additionalInstructionsText.setFont(m_game->assets().getFont("Bungee")); // Set the font
    m_additionalInstructionsText.setCharacterSize(20); // Set the character size
    m_additionalInstructionsText.setFillColor(sf::Color::White); // Set the fill color
    m_additionalInstructionsText.setOutlineThickness(1); // Set the outline thickness
    m_additionalInstructionsText.setOutlineColor(sf::Color::Black); 
    m_additionalInstructionsText.setString("ESC: Go back    Enter: Start game");
    m_additionalInstructionsText.setPosition(50, m_game->window().getSize().y - 50); 
}

void Scene_Instructions::update()
{
    m_entityManager.update();
}

void Scene_Instructions::sRender()
{
    sf::View view = m_game->window().getView();
    view.setCenter(m_game->window().getSize().x / 2.f, m_game->window().getSize().y / 2.f);
    m_game->window().setView(view);

    m_game->window().clear();
    m_game->window().draw(m_backgroundSprite); // Draw the customized background sprite
    m_game->window().draw(m_additionalInstructionsText); // Draw the additional instructions text
    m_game->window().display();
}

void Scene_Instructions::sDoAction(const Action& action)
{
    if (action.type() == "START")
    {
        if (action.name() == "QUIT")
        {
            m_game->changeScene("MENU", nullptr, true);
        }
        else if (action.name() == "PLAY")
        {
            m_game->changeScene("PLAY", std::make_shared<Scene_Play>(m_game, "../assets/level1.txt"));
        }
    }
}

void Scene_Instructions::onEnd()
{
    m_game->changeScene("MENU", nullptr, true);
}
