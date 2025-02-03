#include "Animation.h"
#include <cmath>

Animation::Animation()
{}

Animation::Animation(const std::string& name, const sf::Texture& t)
	: Animation(name, t, 1, 0)
{}

Animation::Animation(const std::string& name, const sf::Texture& t, size_t frameCount, size_t speed)
	: m_name(name)
	, m_sprite(t)
	, m_frameCount(frameCount)
	, m_currentFrame(0)
	, m_speed(speed)
{
	m_size = Vec2(static_cast<float>(t.getSize().x) / frameCount, static_cast<float>(t.getSize().y));
	m_sprite.setOrigin(m_size.x / 2.f, m_size.y / 2.f);
	m_sprite.setTextureRect(sf::IntRect(std::floor(m_currentFrame * m_size.x), 0, m_size.x, m_size.y));
}

void Animation::update(bool repeat)
{

	// increament Frame
	m_currentFrame += 1;

	size_t frame{ 0 };
	if (m_speed > 0) // has animation
	{
		frame = (m_currentFrame / m_speed);   // new frame in animation
		if (frame >= m_frameCount)
		{
			if (repeat)
			{
				frame %= m_frameCount;
			}
			else
			{
				m_hasEnded = true;
				frame = m_frameCount - 1;
			}
		}
	}

	// set frame rect
	sf::IntRect frameRect(static_cast<int>(m_size.x * frame), static_cast<int>(0), static_cast<int>(m_size.x), static_cast<int>(m_size.y)); // left, top, width, height
	m_sprite.setTextureRect(frameRect);
}

bool Animation::hasEnded() const
{
	return m_hasEnded;
}

const std::string& Animation::getName() const
{
	// // O: insert return statement here
	return m_name;
}

const Vec2& Animation::getSize() const
{
	return m_size;
}

sf::Sprite& Animation::getSprite()
{
	return m_sprite;
}
