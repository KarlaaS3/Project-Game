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

    // Customize the instructions text
    m_instructionsText.setFont(m_game->assets().getFont("Bungee")); // Set the font
    m_instructionsText.setCharacterSize(30); // Set the character size
    m_instructionsText.setFillColor(sf::Color::Yellow); // Set the fill color
    m_instructionsText.setOutlineThickness(2); // Set the outline thickness
    m_instructionsText.setOutlineColor(sf::Color::Black); // Set the outline color
    m_instructionsText.setString(
        "Instructions:\n\n"
        "Use W or Up Arrow to move up\n"
        "Use S or Down Arrow to move down\n"
        "Press D to play\n"
        "Press ESC to quit\n"
        "Press Enter to start the game"
    );
    m_instructionsText.setPosition(50, 50); // Set the position

    // Set the background texture
    if (m_game->assets().getTexture("TexMenu").getSize().x > 0)
    {
        m_backgroundSprite.setTexture(m_game->assets().getTexture("TexMenu"));

        // Customize the background sprite
        m_backgroundSprite.setPosition(0, 0); // Set the position
        m_backgroundSprite.setScale(
            static_cast<float>(m_game->window().getSize().x) / m_backgroundSprite.getTexture()->getSize().x,
            static_cast<float>(m_game->window().getSize().y) / m_backgroundSprite.getTexture()->getSize().y
        ); // Scale to fit the window
    }
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
    m_game->window().draw(m_instructionsText); // Draw the instructions text
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