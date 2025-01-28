//
// Created by David Burchill on 2023-09-27.
//

#include "Game.h"
#include "Components.h"
#include <fstream>
#include <iostream>


const sf::Time GameEngine::TIME_PER_FRAME = sf::seconds((1.f / 60.f));

void GameEngine::run() {
    sf::Clock clock;
    sf::Time timeSinceLastUpdate = sf::Time::Zero;

    while (isRunning) {

        // **********************
        // handle user input
        // **********************
        sUserInput();


        // **********************
        // update the world
        // in fixed time steps 
        // **********************
        sf::Time elapsedTime = clock.restart();
        timeSinceLastUpdate += elapsedTime;
        while (timeSinceLastUpdate > TIME_PER_FRAME) {
            timeSinceLastUpdate -= TIME_PER_FRAME;
            sUserInput();
            sUpdate(TIME_PER_FRAME);
        }

        updateStatistics(elapsedTime);  // times per second world is rendered

        // **********************
        // render the next frome
        // **********************
        sRender();
    }
}

void GameEngine::sMovement(sf::Time dt) {

    for (auto &e: entityManager.getEntities()) {
        auto &tfm = e->getComponent<CTransform>();
        tfm.pos += tfm.vel * dt.asSeconds();

        keepInBounds(*e);
    }
}


void GameEngine::sRender() {
    window.clear(sf::Color(100, 100, 255));

    for (auto &e: entityManager.getEntities()) {
        auto &tfm = e->getComponent<CTransform>();
        auto &shape = e->getComponent<CShape>().circle;
        shape.setPosition(tfm.pos);
        window.draw(shape);
    }

    window.draw(statisticsText);
    window.display();
}


void GameEngine::sUpdate(sf::Time dt) {

    // update entity manager
    entityManager.update();

    if (isPaused)
        return;


    // move everything
    sMovement(dt);
    sCollision();

}


void GameEngine::sUserInput() {
 
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            window.close();
            isRunning = false;
        }

    }
}


void GameEngine::init(const std::string &path) {
    //
    loadConfigFromFile(path);
    window.create(sf::VideoMode(windowSize.x, windowSize.y), "GEX Engine");

    statisticsText.setFont(font);
    statisticsText.setPosition(15.0f, 15.0f);
    statisticsText.setCharacterSize(15);
}
 


sf::FloatRect GameEngine::getViewBounds() {
    auto view = window.getView();
    return sf::FloatRect(
            (view.getCenter().x - view.getSize().x / 2.f), (view.getCenter().y - view.getSize().y / 2.f),
            view.getSize().x, view.getSize().y);
}


void GameEngine::sCollision() {

   
}


void GameEngine::keepInBounds(Entity& e)
{
    if (e.hasComponent<CCollision>()) {
        auto  cr = e.getComponent<CCollision>().radius;
        auto& tfm = e.getComponent<CTransform>();
        auto wbounds = window.getSize();

        if (tfm.pos.x < cr || tfm.pos.x >(wbounds.x - cr)) {
            tfm.vel.x *= -1;
            tfm.pos.x = (tfm.pos.x < cr) ? cr : wbounds.x - cr;
        }

        if (tfm.pos.y < cr || tfm.pos.y >(wbounds.y - cr)) {
            tfm.vel.y *= -1;
            tfm.pos.y = (tfm.pos.y < cr) ? cr : wbounds.y - cr;
        }
    }
}


void GameEngine::adjustPlayerPosition() {
    auto vb = getViewBounds();

    auto &player_pos = player->getComponent<CTransform>().pos;
    auto player_cr = player->getComponent<CCollision>().radius;

    // keep player in bounds
    player_pos.x = std::max(player_pos.x, vb.left + player_cr + 5);
    player_pos.x = std::min(player_pos.x, (vb.left + vb.width) - player_cr - 5);
    player_pos.y = std::max(player_pos.y, vb.top + player_cr + 5);
    player_pos.y = std::min(player_pos.y, (vb.top + vb.height) - player_cr - 5);
}

void GameEngine::loadConfigFromFile(const std::string &path) {
    std::ifstream config(path);
    if (config.fail()) {
        std::cerr << "Open file " << path << " failed\n";
        config.close();
        exit(1);
    }

    std::string token{""};
    config >> token;
    while (!config.eof()) {
        if (token == "Window") {
            config >> windowSize.x >> windowSize.y;

        } else if (token == "Font") {
            std::string path;
            config >> path;
            if (!font.loadFromFile(path)) {
                std::cerr << "Failed to load font " << path << "\n";
                exit(-1);
            }
        } else if (token == "Player") {
            auto &pcf = playerConfig;

            int fr{255}, fg{255}, fb{0};
            int olr{255}, olg{255}, olb{255};
            config >> pcf.radius >> pcf.speed
                   >> fr >> fg >> fb
                   >> olr >> olg >> olb
                   >> pcf.outlineThickness
                   >> pcf.nVerticies;

            pcf.fillColor = sf::Color(fr, fg, fb);
            pcf.outlineColor = sf::Color(olr, olg, olb);

        } else if (token[0] == '#') {
            std::string tmp;
            std::getline(config, tmp);
            std::cout << tmp << "\n";
        } 

        config >> token;
    }

    config.close();
}


GameEngine::GameEngine(
        const std::string &path) {
    init(path);
}


void GameEngine::updateStatistics(sf::Time dt) {
    statisticsUpdateTime += dt;
    statisticsNumFrames += 1;
    if (statisticsUpdateTime >= sf::seconds(1.0f)) {
        statisticsText.setString("FPS: " + std::to_string(statisticsNumFrames));
        statisticsUpdateTime -= sf::seconds(1.0f);
        statisticsNumFrames = 0;
    }

}