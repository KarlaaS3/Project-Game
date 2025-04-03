#include "Scene_Menu.h"
#include "Scene_Play.h"
#include <memory>
#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <iostream>
#include "Scene_Instructions.h"

void Scene_Menu::onEnd()
{
    m_game->window().close();
}

Scene_Menu::Scene_Menu(GameEngine* gameEngine)
    : Scene(gameEngine), m_transitionEffect(5.0f) // Adjust fade speed as needed
{
    init();
}

void Scene_Menu::init()
{
    registerAction(sf::Keyboard::W, "UP");
    registerAction(sf::Keyboard::Up, "UP");
    registerAction(sf::Keyboard::S, "DOWN");
    registerAction(sf::Keyboard::Down, "DOWN");
    registerAction(sf::Keyboard::D, "PLAY");
    registerAction(sf::Keyboard::Escape, "QUIT");

    m_title = "Abya Yala!"; // Freedom Fighters
    m_subtitle = "Freedom Fighters";
    m_menuStrings.push_back("Level 1");
    m_menuStrings.push_back("Level 2");
    m_menuStrings.push_back("Instructions"); // Replace "Level 3" with "Instructions"

    m_levelPaths.push_back("../assets/level1.txt");
    m_levelPaths.push_back("../assets/level2.txt");
    m_levelPaths.push_back("../assets/instructions.txt"); // Add a placeholder path for instructions

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

    // Load sounds
    m_hoverSoundBuffer.loadFromFile("../assets/sounds/hover.wav");
    m_selectSoundBuffer.loadFromFile("../assets/sounds/select.wav");
    m_hoverSound.setBuffer(m_hoverSoundBuffer);
    m_selectSound.setBuffer(m_selectSoundBuffer);

    // Set the size of the transition effect rectangle
    m_transitionEffect.setSize(sf::Vector2f(m_game->window().getSize().x, m_game->window().getSize().y));

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
                m_transitionEffect.startFadeOut();
                m_sceneChangePending = true;
                m_applyTransition = true; // Set the flag to apply transition
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

    if (m_sceneChangePending && !m_transitionEffect.isFadingOut())
    {
        if (m_applyTransition) {
            m_game->changeScene("PLAY", std::make_shared<Scene_Play>(m_game, m_levelPaths[m_menuIndex]));
        }
        m_sceneChangePending = false;
        m_applyTransition = false; // Reset the flag
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

    if (backgroundTexture.getSize().x == 0)  // Check if texture loaded
    {
        std::cerr << "ERROR: 'TexMenu' texture failed to load or is empty!" << std::endl;
        return;
    }

    m_menuBackground = backgroundTexture;
    m_backgroundSprite.setTexture(m_menuBackground);
    std::cout << "SUCCESS: Background texture loaded correctly!" << std::endl;
}

void drawGradientText(sf::RenderWindow& window, sf::Text& text, const sf::Color& gradientTop, const sf::Color& gradientBottom, sf::Shader& shader) {
    sf::String string = text.getString();
    float charHeight = text.getCharacterSize();
    sf::Vector2f position = text.getPosition();

    for (size_t i = 0; i < string.getSize(); ++i) {
        sf::Text character = text;
        character.setString(string[i]);

        // Set the gradient colors
        shader.setUniform("topColor", sf::Glsl::Vec4(gradientTop));
        shader.setUniform("bottomColor", sf::Glsl::Vec4(gradientBottom));

        // Position the character
        character.setPosition(position.x + text.findCharacterPos(i).x - text.findCharacterPos(0).x, position.y);

        // Draw the character with the shader
        window.draw(character, &shader);
    }
}

void Scene_Menu::sRender() {
    const size_t CHAR_SIZE{ 64 }; // Define CHAR_SIZE within the scope of sRender

    sf::View view = m_game->window().getView();
    view.setCenter(m_game->window().getSize().x / 2.f, m_game->window().getSize().y / 2.f);
    m_game->window().setView(view);

    m_game->window().clear();

    // Draw the background sprite
    m_game->window().draw(m_backgroundSprite);

    static const sf::Color selectedColor(255, 255, 255); // White color for selected option
    static const sf::Color normalColor(255, 210, 0); // Gold color for normal options
    static const sf::Color redColor(255, 0, 0); // Red color for text

    sf::Text footer("UP: W    DOWN: S   PLAY:D    QUIT: ESC",
        m_game->assets().getFont("Megaman"), 20);
    footer.setFillColor(normalColor);
    footer.setPosition(32, 700);

    // Retrieve the shader from the assets
    sf::Shader& shader = const_cast<sf::Shader&>(m_game->assets().getShader("Gradient"));

    // Center the title text
    m_menuText.setCharacterSize(CHAR_SIZE); // Ensure title text has normal size
    m_menuText.setFillColor(redColor);
    m_menuText.setString(m_title);
    sf::FloatRect titleBounds = m_menuText.getLocalBounds();
    m_menuText.setOrigin(titleBounds.left + titleBounds.width / 2.0f, titleBounds.top + titleBounds.height / 2.0f);
    m_menuText.setPosition(m_game->window().getSize().x / 2.0f, 50);
    drawGradientText(m_game->window(), m_menuText, redColor, normalColor, shader);

    // Center the subtitle text
    m_menuText.setCharacterSize(CHAR_SIZE); // Ensure subtitle text has normal size
    m_menuText.setFillColor(redColor);
    m_menuText.setString(m_subtitle);
    sf::FloatRect subtitleBounds = m_menuText.getLocalBounds();
    m_menuText.setOrigin(subtitleBounds.left + subtitleBounds.width / 2.0f, subtitleBounds.top + subtitleBounds.height / 2.0f);
    m_menuText.setPosition(m_game->window().getSize().x / 2.0f, 150);
    drawGradientText(m_game->window(), m_menuText, redColor, normalColor, shader);

    // Draw menu options
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
        m_menuText.setPosition(m_game->window().getSize().x / 2.0f, 250 + i * 100);
        drawGradientText(m_game->window(), m_menuText, redColor, normalColor, shader);
    }

    m_game->window().draw(footer);
    m_transitionEffect.render(m_game->window());
    m_game->window().display();
}

