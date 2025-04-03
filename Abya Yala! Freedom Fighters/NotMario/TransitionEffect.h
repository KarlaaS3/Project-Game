#pragma once

#include <SFML/Graphics.hpp>

class TransitionEffect
{
private:
    sf::RectangleShape m_fadeRect;
    float m_alpha;
    float m_fadeSpeed;
    bool m_fadingIn;
    bool m_fadingOut;

public:
    TransitionEffect(float fadeSpeed = 1.0f);

    void startFadeIn();
    void startFadeOut();
    void update();
    void render(sf::RenderWindow& window);
    bool isFading() const;
    bool isFadingOut() const;
    void setSize(const sf::Vector2f& size); // Add this method
};
