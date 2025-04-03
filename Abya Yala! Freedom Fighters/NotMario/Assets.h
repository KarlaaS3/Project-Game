#pragma once

#include "Common.h"
#include "Animation.h"
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <map>
#include <memory>
#include <string>

class Assets
{
private:
    std::map<std::string, sf::Texture> m_textureMap;
    std::map<std::string, Animation> m_animatioMap;
    std::map<std::string, sf::Font> m_fontMap;
    std::map<std::string, std::unique_ptr<sf::SoundBuffer>> _soundEffects;
    std::map<std::string, std::unique_ptr<sf::Shader>> m_shaderMap; // Use unique_ptr

    void addTexture(const std::string& textureName, const std::string& path, bool smooth = true);
    void addAnimation(const std::string& animationName, const std::string& textureName, size_t frameCount, size_t speed);
    void addFont(const std::string& fontName, const std::string& path);
    void addSound(const std::string& soundEffectName, const std::string& path);
    void addShader(const std::string& shaderName, const std::string& path); // Updated

public:
    Assets();
    void loadFromFile(const std::string& path);

    const sf::Texture& getTexture(const std::string& textureName) const;
    const Animation& getAnimation(const std::string& animationName) const;
    const sf::Font& getFont(const std::string& fontName) const;
    const sf::SoundBuffer& getSound(const std::string& soundEffectName) const;
    const sf::Shader& getShader(const std::string& shaderName) const;
};

