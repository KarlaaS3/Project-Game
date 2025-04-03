#include "TransitionEffect.h"

TransitionEffect::TransitionEffect(float fadeSpeed)
    : m_alpha(0), m_fadeSpeed(fadeSpeed), m_fadingIn(false), m_fadingOut(false)
{
    m_fadeRect.setFillColor(sf::Color(0, 0, 0, static_cast<sf::Uint8>(m_alpha)));
}

void TransitionEffect::startFadeIn()
{
    m_fadingIn = true;
    m_fadingOut = false;
    m_alpha = 255;
    m_fadeRect.setFillColor(sf::Color(0, 0, 0, static_cast<sf::Uint8>(m_alpha)));
}

void TransitionEffect::startFadeOut()
{
    m_fadingIn = false;
    m_fadingOut = true;
    m_alpha = 0;
    m_fadeRect.setFillColor(sf::Color(0, 0, 0, static_cast<sf::Uint8>(m_alpha)));
}

void TransitionEffect::update()
{
    if (m_fadingIn)
    {
        m_alpha -= m_fadeSpeed;
        if (m_alpha <= 0)
        {
            m_alpha = 0;
            m_fadingIn = false;
        }
    }
    else if (m_fadingOut)
    {
        m_alpha += m_fadeSpeed;
        if (m_alpha >= 255)
        {
            m_alpha = 255;
            m_fadingOut = false;
        }
    }
    m_fadeRect.setFillColor(sf::Color(0, 0, 0, static_cast<sf::Uint8>(m_alpha)));
}

void TransitionEffect::render(sf::RenderWindow& window)
{
    if (m_fadingIn || m_fadingOut)
    {
        window.draw(m_fadeRect);
    }
}

bool TransitionEffect::isFading() const
{
    return m_fadingIn || m_fadingOut;
}

bool TransitionEffect::isFadingOut() const
{
    return m_fadingOut;
}

void TransitionEffect::setSize(const sf::Vector2f& size)
{
    m_fadeRect.setSize(size);
}
