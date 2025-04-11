// Created by Karla Serrano on 06/04/2025

#include "Scene_Menu.h"
#include "Scene_Play.h"
#include <memory>
#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <iostream>
#include "Scene_Instructions.h"

void Scene_Menu::onEnd()
{
    m_backgroundMusic.stop(); // Stop the background music
    m_game->window().close();
}

Scene_Menu::Scene_Menu(GameEngine* gameEngine)
    : Scene(gameEngine), m_transitionEffect(5.0f) // fade speed
{
    init();
}

void Scene_Menu::init()
{
    registerAction(sf::Keyboard::W, "UP");
    registerAction(sf::Keyboard::Up, "UP");
    registerAction(sf::Keyboard::S, "DOWN");
    registerAction(sf::Keyboard::Down, "DOWN");
    registerAction(sf::Keyboard::Enter, "PLAY");
    registerAction(sf::Keyboard::Escape, "QUIT");

    m_title = "Abya Yala!"; // Freedom Fighters
    m_subtitle = "Freedom Fighters";
    m_menuStrings.push_back("Level 1");
    m_menuStrings.push_back("Level 2");
    m_menuStrings.push_back("Instructions"); 

    m_levelPaths.push_back("../assets/level1.txt");
    m_levelPaths.push_back("../assets/level2.txt");
    m_levelPaths.push_back("../assets/instructions.txt"); 

    m_menuText.setFont(m_game->assets().getFont("Bungee"));

    const size_t CHAR_SIZE{ 64 };
    m_menuText.setCharacterSize(CHAR_SIZE);

    m_menuText.setFillColor(sf::Color::White);
    m_menuText.setOutlineThickness(2);
    m_menuText.setOutlineColor(sf::Color::Black);
    m_menuText.setPosition(m_game->window().getSize().x / 2.f, m_game->window().getSize().y / 2.f);
    m_menuText.setOrigin(m_menuText.getLocalBounds().width / 2.f, m_menuText.getLocalBounds().height / 2.f);
    m_menuIndex = 0;
    m_backgroundSprite.setTextureRect(sf::IntRect(0, 0, m_game->window().getSize().x, m_game->window().getSize().y));

    // Load sounds from assets
     m_hoverSound.setBuffer(m_game->assets().getSound("Hover"));
     m_selectSound.setBuffer(m_game->assets().getSound("Select"));

    if (!m_backgroundMusic.openFromFile(m_game->assets().getMusic("Menu"))) {
        std::cerr << "ERROR: Failed to load background music!" << std::endl;
    }
    else {
        m_backgroundMusic.setLoop(true); // Loop the music
        m_backgroundMusic.play(); // Play the music
    }

    // Set the size of the transition effect rectangle
    m_transitionEffect.setSize(sf::Vector2f(m_game->window().getSize().x, m_game->window().getSize().y));

    // Load images for sliding
    m_backgroundSprite.setTexture(m_game->assets().getTexture("Background"));
    m_Anim2.setTexture(m_game->assets().getTexture("Anim2"));
    m_Anim3.setTexture(m_game->assets().getTexture("Anim3"));

    // Set initial positions
    m_backgroundSprite.setPosition(0, 0);
    m_Anim2.setPosition(m_game->window().getSize().x, 0);
    m_Anim3.setPosition(m_game->window().getSize().x * 2, 0);

    for (size_t i = 0; i < m_menuStrings.size(); ++i) {
        Animation coinAnimationStart = m_game->assets().getAnimation("Coin");
        Animation coinAnimationEnd = m_game->assets().getAnimation("Coin");
        m_coinAnimations.push_back(coinAnimationStart);
        m_coinAnimations.push_back(coinAnimationEnd);
    }


    loadMenu();
}

void Scene_Menu::sDoAction(const Action& action)
{
    if (action.type() == "START")
    {
        if (action.name() == "UP")
        {
            m_menuIndex = (m_menuIndex + m_menuStrings.size() - 1) % m_menuStrings.size();
            m_hoverSound.play();
        }
        else if (action.name() == "DOWN")
        {
            m_menuIndex = (m_menuIndex + 1) % m_menuStrings.size();
            m_hoverSound.play();
        }
        else if (action.name() == "PLAY")
        {
            m_selectSound.play();
            if (m_menuStrings[m_menuIndex] == "Instructions") {
                m_game->changeScene("INSTRUCTIONS", std::make_shared<Scene_Instructions>(m_game));
            }
            else {
                m_backgroundMusic.stop(); // Stop the background music
                m_transitionEffect.startFadeOut();
                m_sceneChangePending = true;
                m_applyTransition = true;
            }
        }
        else if (action.name() == "QUIT")
        {
            onEnd();
        }
    }
}


void Scene_Menu::update()
{
    m_entityManager.update();
    m_transitionEffect.update();
    for (auto& coinAnimation : m_coinAnimations) {
        coinAnimation.update();
    }

    // Update positions of sliding images
    float speed = 1.0f;
    m_backgroundSprite.move(-speed, 0);
    m_Anim2.move(-speed, 0);
    m_Anim3.move(-speed, 0);

    if (m_backgroundSprite.getPosition().x + m_backgroundSprite.getGlobalBounds().width < 0) {
        m_backgroundSprite.setPosition(m_game->window().getSize().x * 2, 0);
    }
    if (m_Anim2.getPosition().x + m_Anim2.getGlobalBounds().width < 0) {
        m_Anim2.setPosition(m_game->window().getSize().x * 2, 0);
    }
    if (m_Anim3.getPosition().x + m_Anim3.getGlobalBounds().width < 0) {
        m_Anim3.setPosition(m_game->window().getSize().x * 2, 0);
    }

    if (m_sceneChangePending && !m_transitionEffect.isFadingOut())
    {
        if (m_applyTransition) {
            m_backgroundMusic.stop(); // Stop the background music
            m_game->changeScene("PLAY", std::make_shared<Scene_Play>(m_game, m_levelPaths[m_menuIndex]));
        }
        m_sceneChangePending = false;
        m_applyTransition = false;
    }

    sRender();
}

void Scene_Menu::loadMenu()
{
    if (m_game->assets().getTexture("TexMenu").getSize().x > 0)
    {
        m_backgroundSprite.setTexture(m_game->assets().getTexture("TexMenu")); // Set texture directly
    }

    const sf::Texture& backgroundTexture = m_game->assets().getTexture("TexMenu");

    if (backgroundTexture.getSize().x == 0)  
    {
        std::cerr << "ERROR: 'TexMenu' texture failed to load or is empty!" << std::endl;
        return;
    }

    m_menuBackground = backgroundTexture;
    m_backgroundSprite.setTexture(m_menuBackground);
    std::cout << "SUCCESS: Background texture loaded correctly!" << std::endl;
}

static void drawGradientText(sf::RenderWindow& window, sf::Text& text, const sf::Color& gradientTop, const sf::Color& gradientBottom, sf::Shader& shader) {
    sf::String string = text.getString();
    float charHeight = text.getCharacterSize();
    sf::Vector2f position = text.getPosition();

    for (size_t i = 0; i < string.getSize(); ++i) {
        sf::Text character = text;
        character.setString(string[i]);

        // Set the gradient colors
        shader.setUniform("topColor", sf::Glsl::Vec4(gradientTop));
        shader.setUniform("bottomColor", sf::Glsl::Vec4(gradientBottom));

  
        character.setPosition(position.x + text.findCharacterPos(i).x - text.findCharacterPos(0).x, position.y);

        window.draw(character, &shader);
    }
}

void Scene_Menu::sRender()
{
    const size_t CHAR_SIZE{ 55 }; // Define CHAR_SIZE within the scope of sRender
    const size_t TITLE_SIZE{ 80 }; // Define TITLE_SIZE for the title
    const size_t SUBTITLE_SIZE{ 70 }; // Define SUBTITLE_SIZE for the subtitle

    sf::View view = m_game->window().getView();
    view.setCenter(m_game->window().getSize().x / 2.f, m_game->window().getSize().y / 2.f);
    m_game->window().setView(view);

    m_game->window().clear();

    // Draw the background sprite
    m_game->window().draw(m_backgroundSprite);

    // Draw sliding images
    m_game->window().draw(m_Anim2);
    m_game->window().draw(m_Anim3);

    static const sf::Color selectedColor(255, 255, 255); // White color for selected option
    static const sf::Color normalColor(255, 200, 0); // Gold color for normal options
    static const sf::Color redColor(255, 0, 0); // Red color for text

    sf::Text footer("UP: W    DOWN: S   SELECT: ENTER    QUIT: ESC",
        m_game->assets().getFont("Bungee"), 20);
    footer.setFillColor(normalColor);
    footer.setPosition(32, 700);

    // Retrieve the shader from the assets
    sf::Shader& shader = const_cast<sf::Shader&>(m_game->assets().getShader("Gradient"));

    // Center the title text
    m_menuText.setCharacterSize(TITLE_SIZE); // Set title text size
    m_menuText.setFillColor(redColor);
    m_menuText.setString(m_title);
    sf::FloatRect titleBounds = m_menuText.getLocalBounds();
    m_menuText.setOrigin(titleBounds.left + titleBounds.width / 2.0f, titleBounds.top + titleBounds.height / 2.0f);
    m_menuText.setPosition(m_game->window().getSize().x / 2.0f, 100); // Move title down
    drawGradientText(m_game->window(), m_menuText, redColor, normalColor, shader);

    // Center the subtitle text
    m_menuText.setCharacterSize(SUBTITLE_SIZE); // Set subtitle text size
    m_menuText.setFillColor(redColor);
    m_menuText.setString(m_subtitle);
    sf::FloatRect subtitleBounds = m_menuText.getLocalBounds();
    m_menuText.setOrigin(subtitleBounds.left + subtitleBounds.width / 2.0f, subtitleBounds.top + subtitleBounds.height / 2.0f);
    m_menuText.setPosition(m_game->window().getSize().x / 2.0f, 200); // space between title and subtitle
    drawGradientText(m_game->window(), m_menuText, redColor, normalColor, shader);

    // Draw menu options and coin animations
    for (size_t i{ 0 }; i < m_menuStrings.size(); ++i) {
        if (i == m_menuIndex) {
            m_menuText.setCharacterSize(CHAR_SIZE + 10); // Increase font size for selected option
            m_menuText.setFillColor(selectedColor); // Change color for selected option
        }
        else {
            m_menuText.setCharacterSize(CHAR_SIZE); // Normal font size for other options
            m_menuText.setFillColor(redColor); // Normal color for other options
        }

        m_menuText.setString(m_menuStrings.at(i));
        sf::FloatRect menuBounds = m_menuText.getLocalBounds();
        m_menuText.setOrigin(menuBounds.left + menuBounds.width / 2.0f, menuBounds.top + menuBounds.height / 2.0f);
        m_menuText.setPosition(m_game->window().getSize().x / 2.0f, 320 + i * 100); // Adjust position of menu options
        drawGradientText(m_game->window(), m_menuText, redColor, normalColor, shader);

        // the position of the coin animations relative to the menu string
        float coinOffsetXStart = -menuBounds.width / 2.0f - 30.0f; 
        float coinOffsetXEnd = menuBounds.width / 2.0f + 30.0f; 
        float coinOffsetY = 0.0f; 

        m_coinAnimations[i * 2].getSprite().setPosition(m_menuText.getPosition().x + coinOffsetXStart, m_menuText.getPosition().y + coinOffsetY);
        m_game->window().draw(m_coinAnimations[i * 2].getSprite());

        m_coinAnimations[i * 2 + 1].getSprite().setPosition(m_menuText.getPosition().x + coinOffsetXEnd, m_menuText.getPosition().y + coinOffsetY);
        m_game->window().draw(m_coinAnimations[i * 2 + 1].getSprite());
    }

    m_game->window().draw(footer);
    m_transitionEffect.render(m_game->window());
    m_game->window().display();
}
