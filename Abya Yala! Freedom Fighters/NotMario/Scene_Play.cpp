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

    sf::Texture& backgroundTexture = const_cast<sf::Texture&>(m_game->assets().getTexture("Background"));
    backgroundTexture.setRepeated(true);
    m_backgroundSprite.setTexture(backgroundTexture);

    // Initialize the coin animation
    m_coinAnimation = m_game->assets().getAnimation("SmallCoin");
    m_arrowAnimation = m_game->assets().getAnimation("Arrow");

    loadLevel(levelPath);
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

    // Increment the elapsed time for the oval animation
    m_ovalAnimationTime += m_game->deltaTime();

    // Update the coin animation
    m_coinAnimation.update();

    // TODO pause function

    sMovement();
    sLifespan();
    sCollision();
    sAnimation();
    sEnemyBehavior();

    playerCheckState();
    checkWinCondition();
    //updateBackground();
    checkLoseCondition();
}


void Scene_Play::sRender() {
    // Background color (only visible if there's transparency)
    static const sf::Color background(100, 100, 255);
    static const sf::Color pauseBackground(50, 50, 150);
    m_game->window().clear((m_isPaused ? pauseBackground : background));

    auto& pPos = m_player->getComponent<CTransform>().pos;
    float centerX = std::max(m_game->window().getSize().x / 2.f, pPos.x);

    // Calculate the maximum centerX value
    int textureWidth = m_backgroundSprite.getTexture()->getSize().x;
    int numTiles = 2; // Number of tiles to draw
    float maxCenterX = textureWidth * numTiles - m_game->window().getSize().x / 2.f;

    // Clamp the centerX value
    centerX = std::min(centerX, maxCenterX);

    sf::View view = m_game->window().getView();
    view.setCenter(centerX, m_game->window().getSize().y - view.getCenter().y);
    m_game->window().setView(view);

    // Draw the background image multiple times to create a tiling effect
    int windowWidth = m_game->window().getSize().x;
    int numTilesToDraw = (windowWidth / textureWidth) + 2.5; // Number of tiles to draw

    for (int i = -1; i < numTilesToDraw; ++i) {
        m_backgroundSprite.setPosition(i * textureWidth - (static_cast<int>(pPos.x * 0.5f) % textureWidth), 0);
        m_game->window().draw(m_backgroundSprite);
    }

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
            }
        }
    }

    // Draw health bars for enemies
    for (auto e : m_entityManager.getEntities("enemy")) {
        drawHP(e);
    }


    drawLifeSpan();
    drawCoinsCounter();
	drawArrowsCounter();

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

    // gravity for player
    pt.vel.y += m_playerConfig.GRAVITY;
    pt.vel.x = pt.vel.x * m_playerConfig.SPEED;

    // facing direction
    if (pt.vel.x < -0.1)
        m_player->getComponent<CState>().set(CState::isFacingLeft);
    if (pt.vel.x > 0.1)
        m_player->getComponent<CState>().unSet(CState::isFacingLeft);

    // move all entities
    for (auto e : m_entityManager.getEntities()) {
        auto& tx = e->getComponent<CTransform>();
        tx.prevPos = tx.pos;
        tx.pos += tx.vel;
    }

    // apply gravity to enemies
    for (auto e : m_entityManager.getEntities("enemy")) {
        auto& tx = e->getComponent<CTransform>();
        tx.vel.y += m_playerConfig.GRAVITY;
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

void Scene_Play::respawnPlayer(std::shared_ptr<Entity> player) {
    // Reset player position to the starting point
    player->getComponent<CTransform>().pos = gridToMidPixel(m_playerConfig.X, m_playerConfig.Y, player);
    player->getComponent<CTransform>().vel = Vec2(0.f, 0.f);

    // Reset player state
    player->getComponent<CState>().unSet(CState::isGrounded);
    player->getComponent<CInput>().canJump = true;
    player->getComponent<CInput>().canShoot = true;
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
    auto enemyBullets = m_entityManager.getEntities("enemy_bullet");
    auto coins = m_entityManager.getEntities("coin");
    auto powerUps = m_entityManager.getEntities("Bottle");
    auto fruits = m_entityManager.getEntities("Fruit");


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

        for (auto p : players) {
            for (auto pu : powerUps) {
                auto overlap = Physics::getOverlap(p, pu);
                if (overlap.x > 0 && overlap.y > 0) {
                    // Collect bottle (increase arrows)
                    p->getComponent<CInput>().canShoot = true; // Allow shooting
                    pu->destroy(); // Destroy the power-up
                }
            }

            for (auto f : fruits) {
                auto overlap = Physics::getOverlap(p, f);
                if (overlap.x > 0 && overlap.y > 0) {
                    // Collect fruit (increase life)
                    auto& playerLifespan = p->getComponent<CLifespan>();
                    if (playerLifespan.remaining < 5) {
                        playerLifespan.remaining++;
                    }
                    f->destroy(); // Destroy the power-up
                }
            }
        }

        // Check collision with enemy bullets
        for (auto eb : enemyBullets) {
            auto overlap = Physics::getOverlap(p, eb);
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
                eb->destroy(); // Destroy the enemy bullet
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

        for (auto p : players) {
            for (auto pu : powerUps) {
                auto overlap = Physics::getOverlap(p, pu);
                if (overlap.x > 0 && overlap.y > 0) {
                    // Collect bottle (increase arrows)
                    m_playerArrows += 5; // Increase arrows by 5 (or any other value)
                    pu->destroy(); // Destroy the power-up
                }
            }

            for (auto f : fruits) {
                auto overlap = Physics::getOverlap(p, f);
                if (overlap.x > 0 && overlap.y > 0) {
                    // Collect fruit (increase life)
                    auto& playerLifespan = p->getComponent<CLifespan>();
                    if (playerLifespan.remaining < 5) {
                        playerLifespan.remaining++;
                    }
                    f->destroy(); // Destroy the power-up
                }
            }
        }

        // Check collision with bullets
        for (auto b : bullets) {
            for (auto e : enemies) {
                auto overlap = Physics::getOverlap(e, b);
                if (overlap.x > 0 && overlap.y > 0) {
                    auto& enemyHealth = e->getComponent<CHealth>();
                    enemyHealth.remaining -= 20; // Reduce health
                    if (enemyHealth.remaining <= 0) {
                        // Enemy dies
                        e->destroy();

                        // Spawn power-ups
                        Vec2 position = e->getComponent<CTransform>().pos;
                        if (rand() % 2 == 0) {
                            spawnPowerUp(position, "Bottle");
                        }
                        else {
                            spawnPowerUp(position, "Fruit");
                        }
                    }
                    else {
                        e->getComponent<CAnimation>().animation = m_game->assets().getAnimation("Hurt");
                    }
                    b->destroy(); // Destroy the bullet
                }
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
    coinText.setFont(m_game->assets().getFont("Bungee"));
    coinText.setString(std::to_string(collectedCoins));
    coinText.setCharacterSize(30);
    coinText.setFillColor(sf::Color(255, 223, 63));


    // Get the current view's center
    sf::Vector2f viewCenter = m_game->window().getView().getCenter();
    sf::Vector2f viewSize = m_game->window().getView().getSize();

    // Set the position relative to the view's center and lower the Y-coordinate by 20 pixels
    coinText.setPosition(viewCenter.x - viewSize.x / 2 + 55, viewCenter.y - viewSize.y / 2 + 100);

    // Get the coin animation
    sf::Sprite coinSprite = m_coinAnimation.getSprite();
    coinSprite.setPosition(viewCenter.x - viewSize.x / 2 + 30, viewCenter.y - viewSize.y / 2 + 120);

    // Draw the coin sprite and coin text
    m_game->window().draw(coinSprite);
    m_game->window().draw(coinText);
}

void Scene_Play::drawLifeSpan() {
    // Get the current view's center
    sf::Vector2f viewCenter = m_game->window().getView().getCenter();
    sf::Vector2f viewSize = m_game->window().getView().getSize();

    // Draw hearts for health
    int totalHealth = m_player->getComponent<CLifespan>().total;
    int remainingHealth = m_player->getComponent<CLifespan>().remaining;

    // Adjust the spacing between hearts
    float heartSpacing = 40.0f; // Increase this value to add more space between hearts

    for (int i = 0; i < totalHealth; ++i) {
        sf::Sprite heartSprite;
        if (i < remainingHealth) {
            heartSprite.setTexture(m_game->assets().getTexture("Heart"));
        }
        else {
            heartSprite.setTexture(m_game->assets().getTexture("EmptyHeart"));
        }
        heartSprite.setPosition(viewCenter.x - viewSize.x / 2 + 10 + i * heartSpacing, viewCenter.y - viewSize.y / 2 + 10);
        m_game->window().draw(heartSprite);
    }
}

void Scene_Play::drawArrowsCounter()
{
    sf::Text arrowText;
    arrowText.setFont(m_game->assets().getFont("Bungee"));
    arrowText.setString(std::to_string(m_playerArrows));
    arrowText.setCharacterSize(30);
    arrowText.setFillColor(sf::Color(255, 223, 63));

    // Get the current view's center
    sf::Vector2f viewCenter = m_game->window().getView().getCenter();
    sf::Vector2f viewSize = m_game->window().getView().getSize();

    // Set the position relative to the view's center and lower the Y-coordinate by 20 pixels
    arrowText.setPosition(viewCenter.x - viewSize.x / 2 + 80, viewCenter.y - viewSize.y / 2 + 55);

    // Get the arrow sprite from the assets
    sf::Sprite arrowSprite = m_arrowAnimation.getSprite();
    arrowSprite.setPosition(viewCenter.x - viewSize.x / 2 + 40, viewCenter.y - viewSize.y / 2 + 75);

    // Draw the arrow sprite and the arrows counter
    m_game->window().draw(arrowSprite);
    m_game->window().draw(arrowText);
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
    spawnPowerUp(Vec2(100, 100), "Bottle"); // Example position
    spawnPowerUp(Vec2(200, 200), "Fruit");
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
		else if (token == "Arrow") {
			float gx, gy;
			confFile >> gx >> gy;
			auto arrow = m_entityManager.addEntity("arrow");
			arrow->addComponent<CAnimation>(m_game->assets().getAnimation("Arrow"), true);
			arrow->addComponent<CTransform>(gridToMidPixel(gx, gy, arrow));
		}
		else if (token == "Bottle") {
			float gx, gy;
			confFile >> gx >> gy;
			auto bottle = m_entityManager.addEntity("Bottle");
			bottle->addComponent<CAnimation>(m_game->assets().getAnimation("Bottle"), true);
			bottle->addComponent<CTransform>(gridToMidPixel(gx, gy, bottle));
		}
		else if (token == "Fruit") {
			float gx, gy;
			confFile >> gx >> gy;
			auto fruit = m_entityManager.addEntity("Fruit");
			fruit->addComponent<CAnimation>(m_game->assets().getAnimation("Fruit"), true);
			fruit->addComponent<CTransform>(gridToMidPixel(gx, gy, fruit));
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
    if (m_playerArrows > 0) { // Check if the player has arrows
        auto tx = e->getComponent<CTransform>();

        if (tx.has) {
            auto bullet = m_entityManager.addEntity("bullet");
            bullet->addComponent<CAnimation>(m_game->assets().getAnimation(m_playerConfig.WEAPON), true);
            bullet->addComponent<CTransform>(tx.pos);
            bullet->addComponent<CBoundingBox>(m_game->assets().getAnimation(m_playerConfig.WEAPON).getSize());
            bullet->addComponent<CLifespan>(10);

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

            m_playerArrows--; // Decrease the number of arrows
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
        enemy->addComponent<CAttackTimer>(2.0f);

        Vec2 pos = gridToMidPixel(config.X, config.Y, enemy);
        std::cout << "Converted position: " << pos.x << ", " << pos.y << std::endl;
        enemy->addComponent<CTransform>(pos);

        auto& transform = enemy->getComponent<CTransform>();
        transform.vel.x = config.SPEED;
        transform.vel.y = config.GRAVITY;

        std::cout << "Spawned enemy at: " << config.X << ", " << config.Y
            << " with weapon: " << config.WEAPON << std::endl;
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

void Scene_Play::checkLoseCondition() {
    if (m_hasEnded) return;

    auto players = m_entityManager.getEntities("player");
    for (auto& player : players) {
        auto& playerTransform = player->getComponent<CTransform>();

        // Check if the player has fallen off the screen
        if (playerTransform.pos.y > m_game->window().getSize().y) {
            auto& playerLifespan = player->getComponent<CLifespan>();
            playerLifespan.remaining--;

            if (playerLifespan.remaining <= 0) {
                m_hasEnded = true;
                onEnd();
            }
            else {
                respawnPlayer(player);
            }
        }
    }
}

void Scene_Play::meleeAttack(std::shared_ptr<Entity> enemy) {
    // Implement melee attack logic
    std::cout << "Enemy performs melee attack!" << std::endl;
    // Example: Reduce player's health
    auto players = m_entityManager.getEntities("player");
    for (auto p : players) {
        auto& ptx = p->getComponent<CTransform>();
        auto& etx = enemy->getComponent<CTransform>();
        float distance = std::abs(etx.pos.x - ptx.pos.x);
        if (distance < 50) { // Example melee range
            auto& playerHealth = p->getComponent<CHealth>();
            playerHealth.remaining -= 10; // Reduce player's health
        }
    }
}

void Scene_Play::rangedAttack(std::shared_ptr<Entity> enemy) {
    // Implement ranged attack logic
    std::cout << "Enemy performs ranged attack!" << std::endl;
    // Example: Spawn an enemy bullet entity
    auto& etx = enemy->getComponent<CTransform>();
    auto enemyBullet = m_entityManager.addEntity("enemy_bullet");
    enemyBullet->addComponent<CAnimation>(m_game->assets().getAnimation("Arrow"), true);
    enemyBullet->addComponent<CTransform>(etx.pos);
    enemyBullet->addComponent<CBoundingBox>(Vec2(10, 10)); // Example size
    enemyBullet->addComponent<CLifespan>(50);

    bool isFacingLeft = enemy->getComponent<CState>().test(CState::isFacingLeft);
    enemyBullet->getComponent<CAnimation>().setFlipped(isFacingLeft);

    enemyBullet->getComponent<CTransform>().vel.x = 5 * (isFacingLeft ? -1 : 1);
    enemyBullet->getComponent<CTransform>().vel.y = 0;

    // Set the scale based on the velocity
    if (std::abs(enemyBullet->getComponent<CTransform>().vel.x) > 0.1f) {
        enemyBullet->getComponent<CTransform>().scale.x = (enemyBullet->getComponent<CTransform>().vel.x > 0) ? 1 : -1;
    }
}

void Scene_Play::sEnemyBehavior() {
    auto enemies = m_entityManager.getEntities("enemy");
    for (auto enemy : enemies) {
        if (!enemy->hasComponent<CAttackTimer>()) continue;

        auto& transform = enemy->getComponent<CTransform>();
        auto& attackTimer = enemy->getComponent<CAttackTimer>();

        // Decrease the timeLeft by deltaTime
        attackTimer.timeLeft -= m_game->deltaTime();

        bool playerNearby = false;
        bool attacking = false;

        // Check for players
        auto players = m_entityManager.getEntities("player");
        for (auto player : players) {
            auto& playerTransform = player->getComponent<CTransform>();
            float distance = std::abs(transform.pos.x - playerTransform.pos.x);

            // **Always face the player**
            if (playerTransform.pos.x < transform.pos.x) {
                transform.scale.x = -1; // Face left
                enemy->getComponent<CState>().set(CState::isFacingLeft);
            }
            else {
                transform.scale.x = 1; // Face right
                enemy->getComponent<CState>().unSet(CState::isFacingLeft);
            }

            if (distance < 200) {
                playerNearby = true;

                // Attack only if cooldown is over
                if (attackTimer.timeLeft <= 0) {
                    attackTimer.timeLeft = 2.0f;  // **Reset cooldown BEFORE attacking**

                    if (distance < 50) {
                        meleeAttack(enemy);
                    }
                    else {
                        rangedAttack(enemy);
                    }

                    attacking = true;  // Mark that the enemy is attacking
                }
            }
        }

        // If no player is nearby, patrol
        if (!playerNearby) {
            if (checkPlatformEdge(enemy)) {
                transform.vel.x *= -1;  // Reverse direction
                transform.scale.x *= -1;  // Flip sprite direction

                if (transform.vel.x < 0) {
                    enemy->getComponent<CState>().set(CState::isFacingLeft);
                }
                else {
                    enemy->getComponent<CState>().unSet(CState::isFacingLeft);
                }
            }
        }
        else if (!attacking) {
            // If player is nearby but not attacking, move slightly toward them
            transform.vel.x = (transform.scale.x == 1) ? 0.5f : -0.5f;  // Slow approach
        }
        else {
            transform.vel.x = 0;  // Stop movement while attacking
        }

        // Move enemy
        transform.pos.x += transform.vel.x * m_game->deltaTime();
        transform.pos.y += transform.vel.y;
    }
}

void Scene_Play::spawnPowerUp(const Vec2& position, const std::string& type) {
    auto powerUp = m_entityManager.addEntity(type);
    powerUp->addComponent<CAnimation>(m_game->assets().getAnimation(type), true);
    powerUp->addComponent<CTransform>(position);
    powerUp->addComponent<CBoundingBox>(Vec2(20, 20)); // Adjust the size as needed
}

