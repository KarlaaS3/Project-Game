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
}


void Scene_Play::update() {
    m_entityManager.update();

    // TODO pause function

    sMovement();
    sLifespan();
    sCollision();
    sAnimation();

    playerCheckState();
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


    // move all entities
    for (auto e : m_entityManager.getEntities()) {
        auto& tx = e->getComponent<CTransform>();
        tx.prevPos = tx.pos;
        tx.pos += tx.vel;
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

void Scene_Play::sEnemyBehavior() {
    auto players = m_entityManager.getEntities("player");
    auto enemies = m_entityManager.getEntities("enemy");

    for (auto e : enemies) {
        auto& etx = e->getComponent<CTransform>();
        auto& estate = e->getComponent<CState>();

        // Apply gravity if not grounded
        if (!estate.test(CState::isGrounded)) {
            etx.vel.y += m_enemyConfig.GRAVITY;
        }

        // Move left or right within platform bounds
        if (estate.test(CState::isFacingLeft)) {
            etx.vel.x = -m_enemyConfig.SPEED;
        }
        else {
            etx.vel.x = m_enemyConfig.SPEED;
        }

        // Ensure the enemy stays within platform boundaries (x:2,y:7 to x:9,y:7)
        if (etx.pos.x <= gridToMidPixel(2, 7, e).x) {
            estate.unSet(CState::isFacingLeft);
        }
        else if (etx.pos.x >= gridToMidPixel(9, 7, e).x) {
            estate.set(CState::isFacingLeft);
        }

        // Check if the enemy is on the same platform as the player
        bool playerOnSamePlatform = false;
        for (auto p : players) {
            auto& ptx = p->getComponent<CTransform>();

            if (abs(etx.pos.y - ptx.pos.y) < 5 && ptx.pos.x >= gridToMidPixel(2, 7, p).x && ptx.pos.x <= gridToMidPixel(9, 7, p).x) {
                // Player is on the same platform
                playerOnSamePlatform = true;
                break;
            }
        }

        if (playerOnSamePlatform) {
            estate.set(CState::isAttacking);
            std::cout << "Enemy starts attacking!" << std::endl;
        }
        else {
            estate.unSet(CState::isAttacking);
        }

        // Update position
        etx.pos += etx.vel;
    }
}


void Scene_Play::sCollision() {
    // player with tile
    auto players = m_entityManager.getEntities("player");
    auto tiles = m_entityManager.getEntities("tile");
    auto ground = m_entityManager.getEntities("ground");
    auto enemies = m_entityManager.getEntities("enemy");

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
    }
}



void Scene_Play::sDoAction(const Action& action) {
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
	spawnEnemy(m_enemyConfig);
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
				m_enemyConfig.X >>
				m_enemyConfig.Y >>
				m_enemyConfig.CW >>
				m_enemyConfig.CH >>
				m_enemyConfig.SPEED >>
				m_enemyConfig.JUMP >>
				m_enemyConfig.MAXSPEED >>
				m_enemyConfig.GRAVITY >>
				m_enemyConfig.WEAPON;
		}
        else if (token == "#") {
            ; // ignore comments
            std::string tmp;
            std::getline(confFile, tmp);
            std::cout << "# " << tmp << "\n";
        }
        else {
            std::cerr << "Unkown asset type: " << token << std::endl;
        }

        confFile >> token;
    }
}


void Scene_Play::spawnPlayer() {
    m_player = m_entityManager.addEntity("player");
    m_player->addComponent<CAnimation>(m_game->assets().getAnimation("Run"), true);
    m_player->addComponent<CTransform>(gridToMidPixel(m_playerConfig.X, m_playerConfig.Y, m_player));
    m_player->addComponent<CBoundingBox>(Vec2(m_playerConfig.CW, m_playerConfig.CH));
    m_player->addComponent<CState>();
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
    }
}

void Scene_Play::spawnEnemy(const EnemyConfig& config)
{
    auto enemy = m_entityManager.addEntity("enemy");
    enemy->addComponent<CAnimation>(m_game->assets().getAnimation("Enemy"), true);
    enemy->addComponent<CTransform>(gridToMidPixel(config.X, config.Y, enemy));
    enemy->addComponent<CBoundingBox>(Vec2(config.CW, config.CH));
    enemy->addComponent<CState>();

    // Set additional enemy properties like speed, gravity, etc.
    auto& transform = enemy->getComponent<CTransform>();
    transform.vel.x = config.SPEED;
    transform.vel.y = config.GRAVITY;
    

    std::cout << "Spawned enemy at: " << config.X << ", " << config.Y << " with weapon: " << config.WEAPON << std::endl;
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
