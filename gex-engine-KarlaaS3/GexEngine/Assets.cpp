#include "Assets.h"
#include "MusicPlayer.h"
#include <iostream>
#include <cassert>
#include <fstream>

Assets::Assets() {}

Assets& Assets::getInstance() {
    static Assets instance;
    return instance;
}

void Assets::loadFromFile(const std::string path)
{
	loadFonts(path);
	loadTextures(path);
	loadSounds(path);
	loadAnimations(path);
}

void Assets::addFont(const std::string& fontName, const std::string& path) {
    std::unique_ptr<sf::Font> font(new sf::Font);
    if (!font->loadFromFile(path))
        throw std::runtime_error("Load failed - " + path);

    auto rc = _fontMap.insert(std::make_pair(fontName, std::move(font)));
    if (!rc.second) assert(0);

    std::cout << "Loaded font: " << path << std::endl;
}

void Assets::addSound(const std::string& soundName, const std::string& path) {
    std::unique_ptr<sf::SoundBuffer> sb(new sf::SoundBuffer);
    if (!sb->loadFromFile(path))
        throw std::runtime_error("Load failed - " + path);

    auto rc = _soundEffects.insert(std::make_pair(soundName, std::move(sb)));
    if (!rc.second) assert(0);

    std::cout << "Loaded sound effect: " << path << std::endl;
}

void Assets::addTexture(const std::string& textureName, const std::string& path, bool smooth) {
    _textures[textureName] = sf::Texture();
    if (!_textures[textureName].loadFromFile(path)) {
        std::cerr << "Could not load texture file: " << path << std::endl;
        _textures.erase(textureName);
    } else {
        _textures.at(textureName).setSmooth(smooth);
        std::cout << "Loaded texture: " << path << std::endl;
    }
}

const sf::Font& Assets::getFont(const std::string& fontName) const {
    auto found = _fontMap.find(fontName);
    assert(found != _fontMap.end());
    return *found->second;
}

const sf::SoundBuffer& Assets::getSound(const std::string& soundName) const {
    auto found = _soundEffects.find(soundName);
    assert(found != _soundEffects.end());
    return *found->second;
}

const sf::Texture& Assets::getTexture(const std::string& textureName) const {
    return _textures.at(textureName);
}

void Assets::loadFonts(const std::string& path) {
    std::ifstream confFile(path);
    if (!confFile) {
        std::cerr << "Failed to open font file: " << path << std::endl;
        return;
    }

    std::string token;
    while (confFile >> token) {
        if (token == "Font") {
            std::string name, filePath;
            confFile >> name >> filePath;
            addFont(name, filePath);
        } else {
            std::string buffer;
            std::getline(confFile, buffer);
        }
    }
}

void Assets::loadTextures(const std::string& path) {
    std::ifstream confFile(path);
    if (!confFile) {
        std::cerr << "Failed to open texture file: " << path << std::endl;
        return;
    }

    std::string token;
    while (confFile >> token) {
        if (token == "Texture") {
            std::string name, filePath;
            confFile >> name >> filePath;
            addTexture(name, filePath);
        } else {
            std::string buffer;
            std::getline(confFile, buffer);
        }
    }
}

void Assets::loadSounds(const std::string& path) {
    std::ifstream confFile(path);
    if (!confFile) {
        std::cerr << "Failed to open sound file: " << path << std::endl;
        return;
    }

    std::string token;
    while (confFile >> token) {
        if (token == "Sound") {
            std::string name, filePath;
            confFile >> name >> filePath;
            addSound(name, filePath);
        } else {
            std::string buffer;
            std::getline(confFile, buffer);
        }
    }
}

void Assets::loadAnimations(const std::string& path) {
    std::ifstream confFile(path);
    if (!confFile) {
        std::cerr << "Failed to open animation file: " << path << std::endl;
        return;
    }

    std::string token;
    while (confFile >> token) {
        if (token == "Animation") {
            std::string name, texture, repeat;
            float speed;
            confFile >> name >> texture >> speed >> repeat;

            Animation a(name, getTexture(texture), _frameSets[name], sf::seconds(1 / speed), (repeat == "yes"));
            _animationMap[name] = a;
        } else {
            std::string buffer;
            std::getline(confFile, buffer);
        }
    }
}

const Animation& Assets::getAnimation(const std::string& name) const {
    return _animationMap.at(name);
}