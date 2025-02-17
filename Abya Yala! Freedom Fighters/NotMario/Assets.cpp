#include "Assets.h"
#include <cassert>

Assets::Assets()
{
}


void Assets::loadFromFile(const std::string& path) 
{
    // Read Config file 
    std::ifstream confFile(path);
    if (confFile.fail())
    {
        std::cerr << "Open file: " << path << " failed\n";
        confFile.close();
        exit(1);
    }

    std::string token{ "" };
    confFile >> token;
    while (confFile)
    {
        if (token == "Texture")
        {
            std::string name, path;
            confFile >> name >> path;
            addTexture(name, path);
        }
        else if (token == "Animation")
        {
            std::string name, texture;
            size_t frames, speed;
            confFile >> name >> texture >> frames >> speed;
            addAnimation(name, texture, frames, speed);
        }
        else if (token == "Font")
        {
            std::string name, path;
            confFile >> name >> path;
            addFont(name, path);
        }
        else if (token[0] == '#')
        {
            ; // ignore comments
        }
        else
        {
            std::cerr << "Unkown asset type: " << token << std::endl;
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

/*
const sf::Texture& Assets::getTexture(const std::string& textureName) const
{
    // assert(m_textureMap.contains(textureName)); not required .at() throws out_of_range excpt
    return m_textureMap.at(textureName);
}*/

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


/*const Animation& Assets::getAnimation(const std::string& animationName) const
{
    return m_animatioMap.at(animationName);
}*/

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

const sf::Font& Assets::getFont(const std::string& fontName) const
{
    return m_fontMap.at(fontName);
}
