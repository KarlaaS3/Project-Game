#include "Scene_Play.h"
#include "Entity.h"
#include "Components.h"
#include "Physics.h"

#include <string>

Scene_Play::Scene_Play(GameEngine* gameEngine, const std::string& levelPath)
    : Scene(gameEngine)
    , m_levelPath(levelPath) {
    init(m_levelPath);
}

void Scene_Play::init(const std::string& levelPath) {
    registerActions();


    m_gridText.setCharacterSize(12);
    m_gridText.setFont(m_game->assets().getFont("Arial"));

    const sf::Texture& backgroundTexture = m_game->assets().getTexture("Background");
    m_backgroundSprite.setTexture(backgroundTexture);

    loadLevel(levelPath);
    createGround();
}

void Scene_Play::registerActions() {
    registerAction(sf::Keyboard::P, "PAUSE");
    registerAction(sf::Keyboard::Escape, "QUIT");
    registerAction(sf::Keyboard::T, "TOGGLE_TEXTURE");
    registerAction(sf::Keyboard::C, "TOGGLE_COLLISION");
    registerAction(sf::Keyboard::G, "TOGGLE_GRID");

    registerAction(sf::Keyboard::A, "LEFT");
    registerAction(sf::Keyboard::Left, "LEFT");
    registerAction(sf::Keyboard::D, "RIGHT");
    registerAction(sf::Keyboard::Right, "RIGHT");

    registerAction(sf::Keyboard::W, "JUMP");
    registerAction(sf::Keyboard::Up, "JUMP");

    registerAction(sf::Keyboard::Space, "SHOOT");

    registerAction(sf::Keyboard::Num1, "LEVEL2"); 
    registerAction(sf::Keyboard::Num2, "MENU");    
    registerAction(sf::Keyboard::Num3, "RESTART");
}

void Scene_Play::update() {
    if (m_hasEnded) return;
    m_entityManager.update();

    // TODO pause function

    sMovement();
    sLifespan();
    sCollision();
    sAnimation();
	sEnemyBehavior();

    playerCheckState();
	checkWinCondition();
}

void Scene_Play::sRender() {
        // Background color (only visible if there's transparency)
        static const sf::Color background(100, 100, 255);
        static const sf::Color pauseBackground(50, 50, 150);
        m_game->window().clear((m_isPaused ? pauseBackground : background));

        // **Always use the default view to keep everything static**
        m_game->window().setView(m_game->window().getDefaultView());

        // Draw the background image (this will stay fixed)
        m_game->window().draw(m_backgroundSprite);

        if (m_hasEnded) {
            drawWinScreen();  
            return; 
        }

        // Draw all entities
        if (m_drawTextures) {
            for (auto e : m_entityManager.getEntities()) {
                if (e->hasComponent<CAnimation>()) {
                    auto& transform = e->getComponent<CTransform>();
                    auto& animation = e->getComponent<CAnimation>().animation;
                    animation.getSprite().setRotation(transform.angle);
                    animation.getSprite().setPosition(transform.pos.x, transform.pos.y);
                    animation.getSprite().setScale(transform.scale.x, transform.scale.y);
                    m_game->window().draw(animation.getSprite());
                }
            }
        }

        // Draw collision boxes (debugging)
        if (m_drawCollision) {
            for (auto e : m_entityManager.getEntities()) {
                if (e->hasComponent<CBoundingBox>()) {
                    auto& box = e->getComponent<CBoundingBox>();
                    auto& transform = e->getComponent<CTransform>();
                    sf::RectangleShape rect;
                    rect.setSize(sf::Vector2f(box.size.x, box.size.y));
                    rect.setOrigin(sf::Vector2f(box.halfSize.x, box.halfSize.y));
                    rect.setPosition(transform.pos.x, transform.pos.y);
                    rect.setFillColor(sf::Color(0, 0, 0, 0));
                    rect.setOutlineColor(sf::Color(0, 255, 0));
                    rect.setOutlineThickness(1.f);
                    m_game->window().draw(rect);
                }
            }
        }

        // Draw health bars for enemies
        for (auto e : m_entityManager.getEntities("enemy")) {
            drawHP(e);
        }

        drawCoinsCounter();
		drawLifeSpan();


        // Draw grid (optional debugging)
        if (m_drawGrid) {
            sf::VertexArray lines(sf::Lines);
            sf::Text gridText;
            gridText.setFont(m_game->assets().getFont("Arial"));
            gridText.setCharacterSize(10);

            float left = 0;
            float right = m_game->window().getSize().x;
            float top = 0;
            float bot = m_game->window().getSize().y;

            int nCols = static_cast<int>(m_game->window().getSize().x) / m_gridSize.x;
            int nRows = static_cast<int>(m_game->window().getSize().y) / m_gridSize.y;

            lines.clear();

            // Vertical lines
            for (int x = 0; x <= nCols; ++x) {
                lines.append(sf::Vector2f(x * m_gridSize.x, top));
                lines.append(sf::Vector2f(x * m_gridSize.x, bot));
            }

            // Horizontal lines
            for (int y = 0; y <= nRows; ++y) {
                lines.append(sf::Vector2f(left, y * m_gridSize.y));
                lines.append(sf::Vector2f(right, y * m_gridSize.y));
            }

            // Grid coordinates
            for (int x = 0; x <= nCols; ++x) {
                for (int y = 0; y <= nRows; ++y) {
                    std::string label = "(" + std::to_string(x) + ", " + std::to_string(y) + ")";
                    gridText.setString(label);
                    gridText.setPosition(x * m_gridSize.x, y * m_gridSize.y);
                    m_game->window().draw(gridText);
                }
            }

            m_game->window().draw(lines);
        }

        m_game->window().display();
   }

void Scene_Play::sMovement() {
    // player movement
    auto& pt = m_player->getComponent<CTransform>();
    pt.vel.x = 0.f;
    if (m_player->getComponent<CInput>().left)
        pt.vel.x -= 1;

    if (m_player->getComponent<CInput>().right)
        pt.vel.x += 1;

    if (m_player->getComponent<CInput>().up) {
        m_player->getComponent<CInput>().up = false;
        pt.vel.y = -m_playerConfig.JUMP;
    }

    // gravity
    pt.vel.y += m_playerConfig.GRAVITY;
    pt.vel.x = pt.vel.x * m_playerConfig.SPEED;

    // facing direction
    if (pt.vel.x < -0.1)
        m_player->getComponent<CState>().set(CState::isFacingLeft);
    if (pt.vel.x > 0.1)
        m_player->getComponent<CState>().unSet(CState::isFacingLeft);

    // Define the boundaries of the window
    const float leftBoundary = 0.0f;
    const float rightBoundary = static_cast<float>(m_game->window().getSize().x);
    const float topBoundary = 0.0f;
    const float bottomBoundary = static_cast<float>(m_game->window().getSize().y);

    // move all entities
    for (auto e : m_entityManager.getEntities()) {
        auto& tx = e->getComponent<CTransform>();
        tx.prevPos = tx.pos;
        tx.pos += tx.vel;

        // Check collision with window boundaries
        if (tx.pos.x < leftBoundary) {
            tx.pos.x = leftBoundary;
            tx.vel.x = 0;
        }
        if (tx.pos.x > rightBoundary) {
            tx.pos.x = rightBoundary;
            tx.vel.x = 0;
        }
        if (tx.pos.y < topBoundary) {
            tx.pos.y = topBoundary;
            tx.vel.y = 0;
        }
        if (tx.pos.y > bottomBoundary) {
            tx.pos.y = bottomBoundary;
            tx.vel.y = 0;
        }
    }
}

void Scene_Play::playerCheckState() {
    auto& tx = m_player->getComponent<CTransform>();
    auto& state = m_player->getComponent<CState>();

    // face the right way
    if (std::abs(tx.vel.x) > 0.1f)
        tx.scale.x = (tx.vel.x > 0) ? 1 : -1;

    if (!state.test(CState::isGrounded)) {
        m_player->getComponent<CAnimation>().animation = m_game->assets().getAnimation("Air");
    }
    else {
        // if grounded
        if (std::abs(tx.vel.x) > 0.1f) {
            if (!state.test(CState::isRunning)) // wasn't running
            {
                // change to running animation
                m_player->addComponent<CAnimation>(m_game->assets().getAnimation("Run"), true);
                state.set(CState::isRunning);
            }
        }
        else {
            m_player->addComponent<CAnimation>(m_game->assets().getAnimation("Stand"), true);
            state.unSet(CState::isRunning);
        }
    }
}

void Scene_Play::sLifespan() {
    // move all entities
    for (auto e : m_entityManager.getEntities("bullet")) {
        auto& lifespan = e->getComponent<CLifespan>();
        if (lifespan.has) {
            lifespan.remaining -= 1;
            if (lifespan.remaining < 0) {
                e->addComponent<CAnimation>(m_game->assets().getAnimation("Explosion"), false);
                e->getComponent<CLifespan>().has = false;
                e->getComponent<CTransform>().vel.x *= 0.1f;
            }
        }
    }
}

void Scene_Play::sCollision() {
    // player with tile
    auto players = m_entityManager.getEntities("player");
    auto tiles = m_entityManager.getEntities("tile");
    auto ground = m_entityManager.getEntities("ground");
    auto enemies = m_entityManager.getEntities("enemy");
    auto arrows = m_entityManager.getEntities("arrow");
    auto bullets = m_entityManager.getEntities("bullet");
    auto coins = m_entityManager.getEntities("coin");

    for (auto p : players) {
        p->getComponent<CState>().unSet(CState::isGrounded); // not grounded
        for (auto t : tiles) {
            auto overlap = Physics::getOverlap(p, t);
            if (overlap.x > 0 && overlap.y > 0) // +ve overlap in both x and y means collision
            {
                auto prevOverlap = Physics::getPreviousOverlap(p, t);
                auto& ptx = p->getComponent<CTransform>();
                auto ttx = t->getComponent<CTransform>();

                // collision is in the y direction
                if (prevOverlap.x > 0) {
                    if (ptx.prevPos.y < ttx.prevPos.y) {
                        // player standing on something isGrounded
                        p->getComponent<CTransform>().pos.y -= overlap.y;
                        p->getComponent<CInput>().canJump = true;
                        p->getComponent<CState>().set(CState::isGrounded);
                    }
                    else {
                        // player hit something from below
                        p->getComponent<CTransform>().pos.y += overlap.y;
                    }
                    p->getComponent<CTransform>().vel.y = 0.f;
                }

                // collision is in the x direction
                if (prevOverlap.y > 0) {
                    if (ptx.prevPos.x < ttx.prevPos.x) // player left of tile
                        p->getComponent<CTransform>().pos.x -= overlap.x;
                    else
                        p->getComponent<CTransform>().pos.x += overlap.x;
                }
            }
        }

        // Check collision with the ground
        for (auto g : ground) {
            auto overlap = Physics::getOverlap(p, g);
            if (overlap.x > 0 && overlap.y > 0) {
                auto prevOverlap = Physics::getPreviousOverlap(p, g);
                auto& ptx = p->getComponent<CTransform>();
                auto& gtx = g->getComponent<CTransform>();

                // Y-axis collision (ground)
                if (prevOverlap.x > 0) {
                    if (ptx.prevPos.y < gtx.prevPos.y) {  // Player is above ground
                        ptx.pos.y -= overlap.y;
                        p->getComponent<CInput>().canJump = true;
                        p->getComponent<CState>().set(CState::isGrounded);
                    }
                    ptx.vel.y = 0.f;
                }
            }
        }

        // Check collision with coins
        for (auto c : coins) {
            auto overlap = Physics::getOverlap(p, c);
            if (overlap.x > 0 && overlap.y > 0) {
                c->destroy(); // Destroy the coin
                collectedCoins++; 
            }
        }
    }

    for (auto e : enemies) {
        e->getComponent<CState>().unSet(CState::isGrounded);
        for (auto t : tiles) {
            auto overlap = Physics::getOverlap(e, t);
            if (overlap.x > 0 && overlap.y > 0) {
                auto prevOverlap = Physics::getPreviousOverlap(e, t);
                auto& etx = e->getComponent<CTransform>();
                auto ttx = t->getComponent<CTransform>();

                if (prevOverlap.x > 0) {
                    if (etx.prevPos.y < ttx.prevPos.y) {
                        etx.pos.y -= overlap.y;
                        e->getComponent<CState>().set(CState::isGrounded);
                    }
                    else {
                        etx.pos.y += overlap.y;
                    }
                    etx.vel.y = 0.f;
                }
            }
        }

        // Check collision with the ground
        for (auto g : ground) {
            auto overlap = Physics::getOverlap(e, g);
            if (overlap.x > 0 && overlap.y > 0) {
                auto prevOverlap = Physics::getPreviousOverlap(e, g);
                auto& etx = e->getComponent<CTransform>();
                auto& gtx = g->getComponent<CTransform>();

                if (prevOverlap.x > 0) {
                    if (etx.prevPos.y < gtx.prevPos.y) {
                        etx.pos.y -= overlap.y;
                        e->getComponent<CState>().set(CState::isGrounded);
                    }
                    else {
                        etx.pos.y += overlap.y;
                    }
                    etx.vel.y = 0.f;
                }
            }
        }

        // Check collision with bullets
        for (auto b : bullets) {
            auto overlap = Physics::getOverlap(e, b);
            if (overlap.x > 0 && overlap.y > 0) {
                auto& enemyHealth = e->getComponent<CHealth>();
                enemyHealth.remaining -= 20; // Reduce health
                if (enemyHealth.remaining <= 0) {
                    e->destroy(); // Enemy dies
                }
                else {
                    e->getComponent<CAnimation>().animation = m_game->assets().getAnimation("Hurt");
                }
                b->destroy(); // Destroy the bullet
            }
        }
    }

    // Player collision with enemies
    for (auto p : players) {
        for (auto e : enemies) {
            auto overlap = Physics::getOverlap(p, e);
            if (overlap.x > 0 && overlap.y > 0) {  
                auto& playerLifespan = p->getComponent<CLifespan>();
                playerLifespan.remaining--;  

                if (playerLifespan.remaining <= 0) {
                    p->destroy(); 
                    onEnd(); 
                }
                else {
					p->getComponent<CTransform>().vel.y = 5.f;
                    p->getComponent<CAnimation>().animation = m_game->assets().getAnimation("PlayerHurt");
                }
            }
        }
    }


    // Enemy hit by an arrow
    for (auto e : enemies) {
        for (auto a : arrows) {
            auto overlap = Physics::getOverlap(e, a);
            if (overlap.x > 0 && overlap.y > 0) {
                auto& enemyHealth = e->getComponent<CHealth>();
                enemyHealth.remaining--; // Reduce health
                if (enemyHealth.remaining <= 0) {
                    e->getComponent<CAnimation>().animation = m_game->assets().getAnimation("Death");
                    e->destroy(); // Enemy dies
					onEnd(); // Call game over function
                }
                else {
                    e->getComponent<CState>().unSet(CState::isGrounded);
                    e->getComponent<CTransform>().vel.y = 5.f; // Make the enemy fall
                    e->getComponent<CAnimation>().animation = m_game->assets().getAnimation("Hurt");
                }
                a->destroy(); // Destroy the arrow
            }
        }
    }
}

void Scene_Play::sDoAction(const Action& action) {

    if (m_hasEnded) {
        if (action.type() == "START") {
            if (action.name() == "LEVEL2") {
                m_game->changeScene("PLAY_LEVEL2", std::make_shared<Scene_Play>(m_game, "level2.txt"));
            }
            else if (action.name() == "MENU") {
                m_game->changeScene("MENU", nullptr, true);
            }
            else if (action.name() == "RESTART") {
                m_game->changeScene("PLAY", std::make_shared<Scene_Play>(m_game, "level1.txt"));
            }
        }
        return; 
    }

    // On Key Press
    if (action.type() == "START") {
        if (action.name() == "TOGGLE_TEXTURE") { m_drawTextures = !m_drawTextures; }
        else if (action.name() == "TOGGLE_COLLISION") { m_drawCollision = !m_drawCollision; }
        else if (action.name() == "TOGGLE_GRID") { m_drawGrid = !m_drawGrid; }
        else if (action.name() == "PAUSE") { setPaused(!m_isPaused); }
        else if (action.name() == "QUIT") { onEnd(); }


        // Player control
        else if (action.name() == "LEFT") { m_player->getComponent<CInput>().left = true; }
        else if (action.name() == "RIGHT") { m_player->getComponent<CInput>().right = true; }


        else if (action.name() == "JUMP") {
            if (m_player->getComponent<CInput>().canJump && m_player->getComponent<CState>().test(CState::isGrounded)) {
                m_player->getComponent<CInput>().up = true;
                m_player->getComponent<CInput>().canJump = false;
            }
        }

        else if (action.name() == "SHOOT") {
            if (m_player->getComponent<CInput>().canShoot) {
                spawnBullet(m_player);
                m_player->getComponent<CInput>().shoot = true;
                m_player->getComponent<CInput>().canShoot = false;
            }
        }
    }


    // on Key Release
    else if (action.type() == "END") {
        if (action.name() == "LEFT") { m_player->getComponent<CInput>().left = false; }
        else if (action.name() == "RIGHT") { m_player->getComponent<CInput>().right = false; }
        else if (action.name() == "JUMP") { m_player->getComponent<CInput>().up = false; }
        else if (action.name() == "SHOOT") { m_player->getComponent<CInput>().canShoot = true; }
    }
}

void Scene_Play::sAnimation() {
    // m_player->getComponent<CAnimation>().animation.update();

    for (auto e : m_entityManager.getEntities()) {
        auto& anim = e->getComponent<CAnimation>();
        if (anim.has) {
            anim.animation.update(anim.repeat);
            if (anim.animation.hasEnded())
                e->destroy();
        }
    }
}

void Scene_Play::onEnd() {
    m_game->changeScene("MENU", nullptr, true);
}

void Scene_Play::drawLine() {
}

void Scene_Play::drawHP(std::shared_ptr<Entity> e) {
    auto& health = e->getComponent<CHealth>();
    auto& tx = e->getComponent<CTransform>();

    // Create the health bar
    sf::RectangleShape hpBar;
    hpBar.setSize(sf::Vector2f(health.remaining * 0.5f, 5)); 
    hpBar.setFillColor(sf::Color::Red);
    hpBar.setPosition(tx.pos.x - 25, tx.pos.y - 40); // Higher position

    // Create the health text
    sf::Text hpText;
    hpText.setFont(m_game->assets().getFont("Arial")); 
    hpText.setString(std::to_string(health.remaining) + "/100");
    hpText.setCharacterSize(10); 
    hpText.setFillColor(sf::Color::White);
    hpText.setPosition(tx.pos.x - 25, tx.pos.y - 50); // Higher position

    // Draw the health bar and text
    m_game->window().draw(hpBar);
    m_game->window().draw(hpText);
}

void Scene_Play::drawCoinsCounter() {
    sf::Text coinText;
    coinText.setFont(m_game->assets().getFont("Arial")); 
    coinText.setString("Coins: " + std::to_string(collectedCoins));
    coinText.setCharacterSize(20); 
    coinText.setFillColor(sf::Color::Yellow);
    coinText.setPosition(10, 10); 

    m_game->window().draw(coinText);
}

void Scene_Play::drawWinScreen()
{
    sf::Text gameOverText;
    gameOverText.setFont(m_game->assets().getFont("Arial"));
    gameOverText.setCharacterSize(50);
    gameOverText.setFillColor(sf::Color::White);
    gameOverText.setString("YOU WIN!");
    gameOverText.setPosition(m_game->window().getSize().x / 2 - 100, 100);
    m_game->window().draw(gameOverText);

    
    sf::Text options;
    options.setFont(m_game->assets().getFont("Arial"));
    options.setCharacterSize(30);
    options.setFillColor(sf::Color::Yellow);
    options.setString("1 - Level 2\n2 - Menu\n3 - Restart Level 1");
    options.setPosition(m_game->window().getSize().x / 2 - 100, 200);
    m_game->window().draw(options);

    m_game->window().display();
}

void Scene_Play::drawLifeSpan()
{
	sf::Text lifeSpanText;
	lifeSpanText.setFont(m_game->assets().getFont("Arial"));
	lifeSpanText.setString("Life Span: " + std::to_string(m_player->getComponent<CLifespan>().remaining));
    lifeSpanText.setCharacterSize(20);
    lifeSpanText.setFillColor(sf::Color::White);
	lifeSpanText.setPosition(10, 40);
	m_game->window().draw(lifeSpanText);
}

void Scene_Play::sDebug() {
}

Vec2 Scene_Play::gridToMidPixel(float gridX, float gridY, std::shared_ptr<Entity> entity) {
    // (left, bot) of grix,gidy)

    // this is for side scroll, and based on window height being the same as world height
    // to be more generic and support scrolling up and down as well as left and right it
    // should be based on world size not window size
    float x = 0.f + gridX * m_gridSize.x;
    float y = 768.f - gridY * m_gridSize.y;

    Vec2 spriteSize = entity->getComponent<CAnimation>().animation.getSize();

    return Vec2(x + spriteSize.x / 2.f, y - spriteSize.y / 2.f);
}

void Scene_Play::loadLevel(const std::string& path) {
    m_entityManager = EntityManager(); // get a new entity manager

    // TODO read in level file
    loadFromFile(path);

    spawnPlayer();
	spawnEnemy(m_enemyConfigs);
}

void Scene_Play::loadFromFile(const std::string& path) {
    // Read Config file
    std::ifstream confFile(path);
    if (confFile.fail()) {
        std::cerr << "Open file: " << path << " failed\n";
        confFile.close();
        exit(1);
    }

    std::string token{ "" };
    std::vector<EnemyConfig> enemyConfigs;
    confFile >> token;
    while (confFile) {
        if (token == "Tile") {
            std::string name;
            float gx, gy;
            confFile >> name >> gx >> gy;

            auto e = m_entityManager.addEntity("tile");
            e->addComponent<CAnimation>(m_game->assets().getAnimation(name), true);
            e->addComponent<CBoundingBox>(m_game->assets().getAnimation(name).getSize());
            e->addComponent<CTransform>(gridToMidPixel(gx, gy, e));
        }
        else if (token == "Dec") {
            std::string name;
            float gx, gy;
            confFile >> name >> gx >> gy;

            auto e = m_entityManager.addEntity("dec");
            e->addComponent<CAnimation>(m_game->assets().getAnimation(name), true);
            e->addComponent<CTransform>(gridToMidPixel(gx, gy, e));
        }
        else if (token == "Player") {
            confFile >>
                m_playerConfig.X >>
                m_playerConfig.Y >>
                m_playerConfig.CW >>
                m_playerConfig.CH >>
                m_playerConfig.SPEED >>
                m_playerConfig.JUMP >>
                m_playerConfig.MAXSPEED >>
                m_playerConfig.GRAVITY >>
                m_playerConfig.WEAPON;
        }
		else if (token == "Enemy") {
            EnemyConfig enemyConfig;
            confFile >>
                enemyConfig.X >>
                enemyConfig.Y >>
                enemyConfig.CW >>
                enemyConfig.CH >>
                enemyConfig.SPEED >>
                enemyConfig.JUMP >>
                enemyConfig.MAXSPEED >>
                enemyConfig.GRAVITY >>
                enemyConfig.DETECTION_RANGE >>
                enemyConfig.ATTACK_RANGE >>
                enemyConfig.platformStartX >>
                enemyConfig.platformEndX >>
                enemyConfig.WEAPON;
            enemyConfigs.push_back(enemyConfig);
		}
        else if (token == "Coin") {
            float gx, gy;
            confFile >> gx >> gy;

            auto coin = m_entityManager.addEntity("coin");
            coin->addComponent<CAnimation>(m_game->assets().getAnimation("Coin"), true);
            coin->addComponent<CTransform>(gridToMidPixel(gx, gy, coin));
            coin->addComponent<CBoundingBox>(Vec2(20, 20)); // Adjust the size as needed
        }
        else if (token == "#") {
            std::string tmp;
            std::getline(confFile, tmp);
            std::cout << "# " << tmp << "\n";
        }
        else {
            std::cerr << "Unkown asset type: " << token << std::endl;
        }

        confFile >> token;
    }

    m_enemyConfigs = enemyConfigs;
}

void Scene_Play::spawnPlayer() {
    m_player = m_entityManager.addEntity("player");
    m_player->addComponent<CAnimation>(m_game->assets().getAnimation("Run"), true);
    m_player->addComponent<CTransform>(gridToMidPixel(m_playerConfig.X, m_playerConfig.Y, m_player));
    m_player->addComponent<CBoundingBox>(Vec2(m_playerConfig.CW, m_playerConfig.CH));
    m_player->addComponent<CState>();
    m_player->addComponent<CLifespan>(3);
}

void Scene_Play::spawnBullet(std::shared_ptr<Entity> e) {
    auto tx = e->getComponent<CTransform>();

    if (tx.has) {
        auto bullet = m_entityManager.addEntity("bullet");
        bullet->addComponent<CAnimation>(m_game->assets().getAnimation(m_playerConfig.WEAPON), true);
        bullet->addComponent<CTransform>(tx.pos);
        bullet->addComponent<CBoundingBox>(m_game->assets().getAnimation(m_playerConfig.WEAPON).getSize());
        bullet->addComponent<CLifespan>(50);

        bool isFacingLeft = e->getComponent<CState>().test(CState::isFacingLeft);
        std::cout << "Bullet facing left: " << isFacingLeft << std::endl;

        // Flip the animation
        bullet->getComponent<CAnimation>().setFlipped(isFacingLeft);

        bullet->getComponent<CTransform>().vel.x = 10 * (isFacingLeft ? -1 : 1);
        bullet->getComponent<CTransform>().vel.y = 0;

        // Set the scale based on the velocity
        if (std::abs(bullet->getComponent<CTransform>().vel.x) > 0.1f) {
            bullet->getComponent<CTransform>().scale.x = (bullet->getComponent<CTransform>().vel.x > 0) ? 1 : -1;
        }
    }
}

void Scene_Play::spawnEnemy(const std::vector<EnemyConfig>& configs) {
    for (const auto& config : configs) {
        auto enemy = m_entityManager.addEntity("enemy");
        enemy->addComponent<CAnimation>(m_game->assets().getAnimation("Enemy"), true);
        enemy->addComponent<CBoundingBox>(Vec2(config.CW, config.CH));
        enemy->addComponent<CState>();
        enemy->addComponent<CPlatformInfo>(config.platformStartX, config.platformEndX);
        enemy->addComponent<CHealth>(100);
        Vec2 pos = gridToMidPixel(config.X, config.Y, enemy);
        std::cout << "Converted position: " << pos.x << ", " << pos.y << std::endl;
        enemy->addComponent<CTransform>(pos);

        auto& transform = enemy->getComponent<CTransform>();
        transform.vel.x = config.SPEED;
        transform.vel.y = config.GRAVITY;

        std::cout << "Spawned enemy at: " << config.X << ", " << config.Y << " with weapon: " << config.WEAPON << std::endl;
    }
}

void Scene_Play::sEnemyBehavior() {
    auto players = m_entityManager.getEntities("player");
    auto enemies = m_entityManager.getEntities("enemy");

    for (auto e : enemies) {
        auto& etx = e->getComponent<CTransform>();
        auto& estate = e->getComponent<CState>();
        auto& enemyConfig = m_enemyConfig;

        bool playerDetected = false;
        bool playerInAttackRange = false;

        for (auto p : players) {
            auto& ptx = p->getComponent<CTransform>();
            float distance = std::abs(etx.pos.x - ptx.pos.x);
            // If player is within detection range, enemy gets active
            if (distance < enemyConfig.DETECTION_RANGE) {
                playerDetected = true;
            }
            // If within attack range, start attacking instead of moving towards the player
            if (distance < enemyConfig.ATTACK_RANGE) {
                playerInAttackRange = true;
                estate.set(CState::isAttacking);
                std::cout << "Enemy attacking!" << std::endl;
            }
            else {
                estate.unSet(CState::isAttacking);
            }
        }
        // If the player is in detection range but not in attack range,
        if (playerDetected && !playerInAttackRange) {
            if (estate.test(CState::isFacingLeft)) {
                etx.vel.x = -enemyConfig.SPEED;
            }
            else {
                etx.vel.x = enemyConfig.SPEED;
            }
            
            if (checkPlatformEdge(e)) {
                if (estate.test(CState::isFacingLeft)) {
                    estate.unSet(CState::isFacingLeft);
                }
                else {
                    estate.set(CState::isFacingLeft);
                }
            }
        }
        // If attacking, move left and right instead of following the player
        else if (playerInAttackRange) {
            if (estate.test(CState::isFacingLeft)) {
                etx.vel.x = -enemyConfig.SPEED; 
            }
            else {
                etx.vel.x = enemyConfig.SPEED; 
            }
            // Change direction randomly to simulate enemy attacking motion
            if (rand() % 100 < 3) { // 3% chance to change direction each frame
                if (estate.test(CState::isFacingLeft)) {
                    estate.unSet(CState::isFacingLeft);
                }
                else {
                    estate.set(CState::isFacingLeft);
                }
            }
            e->getComponent<CAnimation>().animation = m_game->assets().getAnimation("Attack");
        }
        else {
            etx.vel.x = 0; // If no player is nearby, enemy stays still
        }
        // Apply gravity if not grounded
        if (!estate.test(CState::isGrounded)) {
            etx.vel.y += enemyConfig.GRAVITY;
        }
        // Update enemy position
        etx.pos += etx.vel;

        if (std::abs(etx.vel.x) > 0.1f) {
            etx.scale.x = (etx.vel.x > 0) ? 1 : -1;
            e->getComponent<CAnimation>().setFlipped(etx.vel.x < 0);
        }
    }
}

bool Scene_Play::checkPlatformEdge(std::shared_ptr<Entity> enemy) {
    auto& transform = enemy->getComponent<CTransform>();
    auto& boundingBox = enemy->getComponent<CBoundingBox>();
    auto& platformInfo = enemy->getComponent<CPlatformInfo>();

    float edgeThreshold = 5.0f;

    // Check if the enemy is near the left or right edge of its assigned platform
    float leftEdge = transform.pos.x - boundingBox.halfSize.x;
    float rightEdge = transform.pos.x + boundingBox.halfSize.x;

    if (leftEdge <= platformInfo.platformStartX + edgeThreshold ||
        rightEdge >= platformInfo.platformEndX - edgeThreshold) {
        return true;  // Enemy is near the edge, should turn around
    }

    return false;
}

void Scene_Play::createGround() {

    float groundWidth = m_game->window().getSize().x;  // Full screen width
    float groundHeight = 30.0f;  // Thickness of the collision area

    // Define ground position (bottom of the screen)
    float groundX = groundWidth / 2;
    float groundY = m_game->window().getSize().y - (groundHeight / 2);

    // Create the ground entity
    auto ground = m_entityManager.addEntity("ground");

    // Add transform component
    ground->addComponent<CTransform>(Vec2(groundX, groundY));

    // Add bounding box for collision detection
    ground->addComponent<CBoundingBox>(Vec2(groundWidth, groundHeight));
}

void Scene_Play::checkWinCondition() {
    if (m_hasEnded) return;

    auto enemies = m_entityManager.getEntities("enemy");

    
    bool allEnemiesDefeated = true;
    for (auto& enemy : enemies) {
        if (enemy->getComponent<CHealth>().remaining > 0) {
            allEnemiesDefeated = false;
            break; 
        }
    }

    if (allEnemiesDefeated && collectedCoins >= totalCoins) {
        m_hasEnded = true;
        drawWinScreen();
    }
}

void Scene_Play::checkLoseCondition()
{
	if (m_hasEnded) return;
	auto players = m_entityManager.getEntities("player");
	for (auto& player : players) {
		if (player->getComponent<CLifespan>().remaining <= 0) {
			m_hasEnded = true;
			onEnd();
		}
	}
}





