#include "Assets.h"
#include <cassert>
#include <iostream>
#include <fstream>


Assets::Assets()
{
}

void Assets::loadFromFile(const std::string& path) {
    // Read Config file 
    std::ifstream confFile(path);
    if (confFile.fail()) {
        std::cerr << "Open file: " << path << " failed\n";
        confFile.close();
        exit(1);
    }

    std::string token{ "" };
    confFile >> token;
    while (confFile) {
        if (token == "Texture") {
            std::string name, path;
            confFile >> name >> path;
            addTexture(name, path);
        }
        else if (token == "Animation") {
            std::string name, texture;
            size_t frames, speed;
            confFile >> name >> texture >> frames >> speed;
            addAnimation(name, texture, frames, speed);
        }
        else if (token == "Font") {
            std::string name, path;
            confFile >> name >> path;
            addFont(name, path);
        }
        else if (token == "Shader") {
            std::string name, path;
            confFile >> name >> path;
            addShader(name, path);
        }
        else if (token == "Sound") { 
            std::string name, path;
            confFile >> name >> path;
            addSound(name, path);
        }
		else if (token == "Music") {
			std::string name, path;
			confFile >> name >> path;
			addMusic(name, path);
		}
        else if (token[0] == '#') {
            ; // ignore comments
        }
        else {
            std::cerr << "Unknown asset type: " << token << std::endl;
        }

        confFile >> token;
    }
    confFile.close();
}

void Assets::addTexture(const std::string& textureName, const std::string& path, bool smooth)
{
    m_textureMap[textureName] = sf::Texture();
    if (!m_textureMap[textureName].loadFromFile(path))
    {
        std::cerr << "Could not load texture file: " << path << std::endl;
        m_textureMap.erase(textureName);
    }
    else
    {
        m_textureMap.at(textureName).setSmooth(smooth);
        std::cout << "Loaded texture: " << textureName << " from " << path << std::endl;
    }
}

void Assets::addAnimation(const std::string& animationName, const std::string& textureName, size_t frameCount, size_t speed)
{
    m_animatioMap[animationName] = Animation(animationName, getTexture(textureName), frameCount, speed);
}

void Assets::addFont(const std::string& fontName, const std::string& path)
{
    m_fontMap[fontName] = sf::Font();
    if (!m_fontMap[fontName].loadFromFile(path))
    {
        std::cerr << "Could not load Font from file: " << path << std::endl;
        m_fontMap.erase(fontName);
    }
    else
    {
        std::cout << "Loaded font: " << path << std::endl;
    }
}


void Assets::addShader(const std::string& shaderName, const std::string& path) {
    auto shader = std::make_unique<sf::Shader>();
    if (!shader->loadFromFile(path, sf::Shader::Fragment)) {
        std::cerr << "Could not load shader file: " << path << std::endl;
    }
    else {
        m_shaderMap[shaderName] = std::move(shader);
        std::cout << "Loaded shader: " << shaderName << " from " << path << std::endl;
    }
}

const sf::Texture& Assets::getTexture(const std::string& textureName) const
{
    auto it = m_textureMap.find(textureName);
    if (it != m_textureMap.end()) {
        return it->second;
    }
    else {
        std::cerr << "Texture not found: " << textureName << std::endl;
        throw std::out_of_range("Texture not found: " + textureName);
    }
}

const Animation& Assets::getAnimation(const std::string& animationName) const {
    auto it = m_animatioMap.find(animationName);
    if (it != m_animatioMap.end()) {
        return it->second;
    }
    else {
        std::cerr << "Animation not found: " << animationName << std::endl;
        throw std::out_of_range("Animation not found: " + animationName);
    }
}

const sf::Font& Assets::getFont(const std::string& fontName) const {
    auto it = m_fontMap.find(fontName);
    if (it != m_fontMap.end()) {
        return it->second;
    }
    else {
        std::cerr << "Font not found: " << fontName << std::endl;
        throw std::out_of_range("Font not found: " + fontName);
    }
}

void Assets::addSound(const std::string& soundEffectName, const std::string& path) {
    auto soundBuffer = std::make_unique<sf::SoundBuffer>();
    if (!soundBuffer->loadFromFile(path)) {
        std::cerr << "Could not load sound file: " << path << std::endl;
    }
    else {
        m_soundMap[soundEffectName] = std::move(soundBuffer);
        std::cout << "Loaded sound: " << soundEffectName << " from " << path << std::endl;
    }
}

const sf::SoundBuffer& Assets::getSound(const std::string& soundEffectName) const {
    auto it = m_soundMap.find(soundEffectName);
    if (it != m_soundMap.end()) {
        return *(it->second);
    }
    else {
        std::cerr << "Sound not found: " << soundEffectName << std::endl;
        throw std::out_of_range("Sound not found: " + soundEffectName);
    }
}

void Assets::addMusic(const std::string& musicName, const std::string& path) {
    m_musicMap[musicName] = path;
}

const std::string& Assets::getMusic(const std::string& musicName) const {
    auto it = m_musicMap.find(musicName);
    if (it != m_musicMap.end()) {
        return it->second;
    }
    else {
        std::cerr << "Music not found: " << musicName << std::endl;
        throw std::out_of_range("Music not found: " + musicName);
    }
}


const sf::Shader& Assets::getShader(const std::string& shaderName) const {
    auto it = m_shaderMap.find(shaderName);
    if (it != m_shaderMap.end()) {
        return *(it->second);
    }
    else {
        std::cerr << "Shader not found: " << shaderName << std::endl;
        throw std::out_of_range("Shader not found: " + shaderName);
    }
}