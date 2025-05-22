#include "../include/game_manager.h"
#include "../include/player.h"
#include "../include/utils.h"
#include "../include/globals.h"
#include <random>
#include <algorithm>
#include <ctime>

GameManager::GameManager(Window* window) : 
    window(window), 
    spawnTimer(0.0f), 
    spawnInterval(5.0f), 
    beamTimer(0.0f), 
    beamInterval(5.0f), 
    pentagonTimer(0.0f), 
    pentagonInterval(5.0f), 
    gameState(GameState::RUNNING), 
    gameOverTimer(0.0f) {
    
    std::random_device rd;
    rng = std::mt19937(rd());
    
    collisionManager.setGameManager(this);
}

GameManager::~GameManager() {}

void GameManager::spawnTriangle(Vector2D pos, Vector2D dir, Player* target) {
    Vector2D dims(60, 51);
    GameObject::Scope scope = GameObject::Scope::GLOBAL;

    Uint8 r = 255, g = 255, b = 0, a = 255;
    
    int speedBase = 100;
    int speedRange = 50;
    std::uniform_int_distribution<int> speedDist(speedBase - speedRange, speedBase + speedRange);
    int speed = speedDist(rng);
    
    float health = 50.0f;
    float score = 10.0f;
    
    auto triangle = std::make_unique<Triangle>(
        pos, dims, dir, scope, r, g, b, a, speed, window, target, health
    );
    
    triangle->setScore(score);

    std::uniform_real_distribution<float> angleDist(0.0f, 2 * M_PI);
    float angle = angleDist(rng);
    triangle->setAngle(angle);
    
    collisionManager.addObject(triangle.get());
    gameObjects.push_back(std::move(triangle));
}

void GameManager::spawnBeam(Player* target) {
    if (!target || !target->getActive()) return;
    
    SDL_Rect bounds = window->getBounds();
    
    std::uniform_int_distribution<int> edgeDist(0, 3); // 0: top, 1: bottom, 2: left, 3: right
    int startEdge = edgeDist(rng);
    
    Vector2D targetPos = target->getPosition();
    Vector2D beamPos;
    float beamWidth = 200.0f; // Width of the beam
    
    // Calculate beam position along the selected edge
    switch (startEdge) {
        case 0: // Top edge
            beamPos.x = targetPos.x - (beamWidth / 2);
            beamPos.y = bounds.y;
            break;
        case 1: // Bottom edge
            beamPos.x = targetPos.x - (beamWidth / 2);
            beamPos.y = bounds.y + bounds.h;
            break;
        case 2: // Left edge
            beamPos.x = bounds.x;
            beamPos.y = targetPos.y - (beamWidth / 2);
            break;
        case 3: // Right edge
            beamPos.x = bounds.x + bounds.w;
            beamPos.y = targetPos.y - (beamWidth / 2);
            break;
    }
    
    // Beam parameters
    Vector2D dims(20, 20); // Initial warning size
    GameObject::Scope scope = GameObject::Scope::GLOBAL;
    Uint8 r = 255, g = 0, b = 0, a = 255; // Red beam
    int speed = 0;
    
    auto beam = std::make_unique<Beam>(
        beamPos, dims, scope, r, g, b, a, speed, startEdge, window, target, beamWidth
    );
    
    // Add to game objects collection
    collisionManager.addObject(beam.get());
    gameObjects.push_back(std::move(beam));
}

void GameManager::spawnProjectile(Vector2D pos, Vector2D dir, int speed) {
    // Projectile parameters
    Vector2D dims(10, 10);
    GameObject::Scope scope = GameObject::Scope::LOCAL;
    Uint8 r = 0, g = 255, b = 255, a = 255;
    
    auto projectile = std::make_unique<Projectile>(
        pos, dims, dir, scope, r, g, b, a, speed, window
    );
    
    collisionManager.addObject(projectile.get());
    gameObjects.push_back(std::move(projectile));
}

void GameManager::spawnRandomEnemy(Player* target) {
    SDL_Rect bounds = window->getBounds();
    
    std::uniform_int_distribution<int> marginDist(50, 100);
    int spawnMargin = marginDist(rng);

    std::uniform_int_distribution<int> numEnemiesDist(1, 4);
    int numEnemies = numEnemiesDist(rng);

    for (int i = 0; i < numEnemies; ++i) {
        Vector2D spawnPos;

        std::uniform_int_distribution<int> edgeDist(0, 3);
        int edge = edgeDist(rng); // Randomly select an edge (0: top, 1: bottom, 2: left, 3: right)

        switch (edge) {
            case 0: // Top edge
                spawnPos.x = std::uniform_int_distribution<int>(bounds.x, bounds.x + bounds.w)(rng);
                spawnPos.y = bounds.y - spawnMargin;
                break;

            case 1: // Bottom edge
                spawnPos.x = std::uniform_int_distribution<int>(bounds.x, bounds.x + bounds.w)(rng);
                spawnPos.y = bounds.y + bounds.h + spawnMargin;
                break;
            
            case 2: // Left edge
                spawnPos.x = bounds.x - spawnMargin;
                spawnPos.y = std::uniform_int_distribution<int>(bounds.y, bounds.y + bounds.h)(rng);
                break;

            case 3: // Right edge
                spawnPos.x = bounds.x + bounds.w + spawnMargin;
                spawnPos.y = std::uniform_int_distribution<int>(bounds.y, bounds.y + bounds.h)(rng);
                break;
            }
        
            Vector2D spawnDir = (target->getPosition() - spawnPos).normalize();
            spawnTriangle(spawnPos, spawnDir, target);
        }
}

void GameManager::spawnPentagon(Vector2D pos, Player* player) {
    Vector2D dims(100, 100);
    GameObject::Scope scope = GameObject::Scope::GLOBAL;
    float health = 500.0f;
    
    auto pentagon = std::make_unique<Pentagon>(
        pos, dims, scope, window, player, health
    );
    
    collisionManager.addObject(pentagon.get());
    gameObjects.push_back(std::move(pentagon));
}

void GameManager::spawnPentagonGroup(Player* player) {
    if (!player || !player->getActive()) return;
    
    std::uniform_int_distribution<int> numPentagonsDist(1, 3);
    int numPentagons = numPentagonsDist(rng);
    
    SDL_Rect bounds = window->getBounds();
    auto [screenW, screenH] = getResolution();
    Vector2D playerPos = player->getPosition();
    
    for (int i = 0; i < numPentagons; ++i) {
        Vector2D spawnPos;
        float distance = 0.0f;
        
        // randomly select a position until find one 500 pixels away
        do {
            std::uniform_int_distribution<int> xDist(0, screenW);
            std::uniform_int_distribution<int> yDist(0, screenH);
            
            spawnPos.x = xDist(rng);
            spawnPos.y = yDist(rng);
            
            distance = (spawnPos - playerPos).magnitude();
        } while (distance < 500.0f);
        
        spawnPentagon(spawnPos, player);
    }
}

void GameManager::update(float deltaTime) {
    if (gameState == GameState::PAUSED || gameState == GameState::GAME_OVER) {
        return;
    }

    // search for player object
    Player* playerTarget = nullptr;
    for (auto& obj : gameObjects) {
        if (obj->getType() == GameObject::ObjectType::Player) {
            playerTarget = static_cast<Player*>(obj.get());
            break;
        }
    }

    // triangle spawn logic
    spawnTimer += deltaTime;
    if (spawnTimer >= spawnInterval) {
        if (playerTarget) {
            spawnRandomEnemy(playerTarget);
        }
        spawnTimer = 0.0f;
    }
    
    // beam spawn logic
    beamTimer += deltaTime;
    if (beamTimer >= beamInterval) {
        if (playerTarget) {
            spawnBeam(playerTarget);
        }
        beamTimer = 0.0f;
    }
    
    // pentagon spawn logic
    pentagonTimer += deltaTime;
    if (pentagonTimer >= pentagonInterval) {
        if (playerTarget) {
            spawnPentagonGroup(playerTarget);
        }
        pentagonTimer = 0.0f;
    }
    
    for (auto& obj : gameObjects) {
        if (obj->getActive()) {
            obj->update(deltaTime);
        }
    }
    
    checkCollisions();
    cleanupInactiveObjects();
}

void GameManager::draw(SDL_Renderer* renderer) {
    for (auto& obj : gameObjects) {
        if (obj->getActive()) {
            obj->draw(renderer);
        }
    }
}

void GameManager::cleanupInactiveObjects() {
    auto it = gameObjects.begin();
    while (it != gameObjects.end()) {
        if (!(*it)->getActive()) {
            collisionManager.removeObject(it->get());
        }
        ++it;
    }
    
    gameObjects.erase(
        std::remove_if(gameObjects.begin(), gameObjects.end(), 
            [](const std::unique_ptr<GameObject>& obj) { return !obj->getActive(); }
        ),
        gameObjects.end()
    );
}

void GameManager::checkCollisions() {
    collisionManager.checkCollisions();
}

void GameManager::handleCollision(GameObject* a, GameObject* b) {
    // get object types
    GameObject::ObjectType typeA = a->getType();
    GameObject::ObjectType typeB = b->getType();
    
    // Projectile hits Triangle
    if ((typeA == GameObject::ObjectType::Projectile && typeB == GameObject::ObjectType::Triangle) ||
        (typeB == GameObject::ObjectType::Projectile && typeA == GameObject::ObjectType::Triangle)) {
        
        Projectile* projectile = (typeA == GameObject::ObjectType::Projectile) ? 
            static_cast<Projectile*>(a) : static_cast<Projectile*>(b);
        
        Triangle* triangle = (typeA == GameObject::ObjectType::Triangle) ? 
            static_cast<Triangle*>(a) : static_cast<Triangle*>(b);
        
        if (projectile->getActive() && triangle->getActive()) {
            triangle->changeHealthBy(-10.0f);
            triangle->setLastHitTime(SDL_GetTicks() / 1000.0f);
            triangle->setColor(255, 255, 255, 255);
            
            projectile->setActive(false);
            
            if (triangle->getHealth() <= 0) {
                Player* player = nullptr;
                for (auto& obj : gameObjects) {
                    if (obj->getType() == GameObject::ObjectType::Player) {
                        player = static_cast<Player*>(obj.get());
                        break;
                    }
                }
                
                if (player) {
                    int scoreValue = static_cast<int>(triangle->getScore());
                    player->addScore(scoreValue);
                    std::cerr << "Triangle destroyed! Add " << scoreValue << " points to player." << std::endl;
                }
                
                triangle->setActive(false);
            }
        }
    }
    
    // CASE: Projectile hits Pentagon
    else if ((typeA == GameObject::ObjectType::Projectile && typeB == GameObject::ObjectType::Pentagon) ||
             (typeB == GameObject::ObjectType::Projectile && typeA == GameObject::ObjectType::Pentagon)) {
        
        Projectile* projectile = (typeA == GameObject::ObjectType::Projectile) ? 
            static_cast<Projectile*>(a) : static_cast<Projectile*>(b);
        
        Pentagon* pentagon = (typeA == GameObject::ObjectType::Pentagon) ? 
            static_cast<Pentagon*>(a) : static_cast<Pentagon*>(b);
        
        if (projectile->getActive() && pentagon->getActive()) {
            pentagon->changeHealthBy(-10.0f);
            pentagon->setLastHitTime(SDL_GetTicks() / 1000.0f);
            
            projectile->setActive(false);
            
            if (pentagon->getHealth() <= 0) {
                // Find the player to award score
                Player* player = nullptr;
                for (auto& obj : gameObjects) {
                    if (obj->getType() == GameObject::ObjectType::Player) {
                        player = static_cast<Player*>(obj.get());
                        break;
                    }
                }
                
                if (player) {
                    int scoreValue = static_cast<int>(pentagon->getScore());
                    player->addScore(scoreValue);
                    std::cerr << "Pentagon destroyed! Add " << scoreValue << " points to player." << std::endl;
                }
                
                pentagon->setActive(false);
            }
        }
    }
    
    // CASE: Triangle hits Player
    else if ((typeA == GameObject::ObjectType::Triangle && typeB == GameObject::ObjectType::Player) ||
             (typeB == GameObject::ObjectType::Triangle && typeA == GameObject::ObjectType::Player)) {
        
        Triangle* triangle = (typeA == GameObject::ObjectType::Triangle) ? 
            static_cast<Triangle*>(a) : static_cast<Triangle*>(b);
            
        Player* player = (typeA == GameObject::ObjectType::Player) ? 
            static_cast<Player*>(a) : static_cast<Player*>(b);
        
        if (triangle->getActive() && player->getActive()) {
            if (player->isInDeathAnimation()) {
                return;
            }
            
            // Deal damage to player
            player->changeHealthBy(-1);
            
            // Calculate and apply knockback
            float knockbackFactor = 50.0f;
            Vector2D playerPos = player->getPosition();
            Vector2D trianglePos = triangle->getPosition();
            
            Vector2D knockbackDirection = (playerPos - trianglePos).normalize();
            Vector2D knockbackVelocity = knockbackDirection * knockbackFactor;
            
            player->applyKnockback(knockbackVelocity);
            
            std::cerr << "Triangle hit player! Player health: " << player->getHealth() << std::endl;
        }
    }
    
    // CASE: Pentagon hits Player
    else if ((typeA == GameObject::ObjectType::Pentagon && typeB == GameObject::ObjectType::Player) ||
             (typeB == GameObject::ObjectType::Pentagon && typeA == GameObject::ObjectType::Player)) {
        
        Pentagon* pentagon = (typeA == GameObject::ObjectType::Pentagon) ? 
            static_cast<Pentagon*>(a) : static_cast<Pentagon*>(b);
            
        Player* player = (typeA == GameObject::ObjectType::Player) ? 
            static_cast<Player*>(a) : static_cast<Player*>(b);
        
        if (pentagon->getActive() && player->getActive()) {
            if (player->isInDeathAnimation()) {
                return;
            }
            
            player->changeHealthBy(-1);
            
            float knockbackFactor = 50.0f;
            Vector2D playerPos = player->getPosition();
            Vector2D pentagonPos = pentagon->getPosition();
            
            Vector2D knockbackDirection = (playerPos - pentagonPos).normalize();
            Vector2D knockbackVelocity = knockbackDirection * knockbackFactor;
            
            player->applyKnockback(knockbackVelocity);
        }
    }
    
    // CASE: Beam hits Player
    else if ((typeA == GameObject::ObjectType::Beam && typeB == GameObject::ObjectType::Player) ||
             (typeB == GameObject::ObjectType::Beam && typeA == GameObject::ObjectType::Player)) {
        
        Beam* beam = (typeA == GameObject::ObjectType::Beam) ? 
            static_cast<Beam*>(a) : static_cast<Beam*>(b);
            
        Player* player = (typeA == GameObject::ObjectType::Player) ? 
            static_cast<Player*>(a) : static_cast<Player*>(b);
        
        if (beam->getActive() && player->getActive()) {
            if (player->isInDeathAnimation()) {
                return;
            }
            
            Beam::BeamState beamState = beam->getState();
            if (beamState == Beam::BeamState::ACTIVE || beamState == Beam::BeamState::EXPANDING) {
                player->changeHealthBy(-2);
            }
        }
    }
    
    // CASE: Triangle hits Triangle or Pentagon - they can pass through each other
    else if ((typeA == GameObject::ObjectType::Triangle && typeB == GameObject::ObjectType::Triangle) ||
             (typeA == GameObject::ObjectType::Pentagon && typeB == GameObject::ObjectType::Pentagon) ||
             (typeA == GameObject::ObjectType::Triangle && typeB == GameObject::ObjectType::Pentagon) ||
             (typeB == GameObject::ObjectType::Triangle && typeA == GameObject::ObjectType::Pentagon)) {
        // No action needed, these objects should be able to pass through each other
        return;
    }
    
    // CASE: Player hits Projectile - ignore as projectiles are from the player
    else if ((typeA == GameObject::ObjectType::Player && typeB == GameObject::ObjectType::Projectile) ||
             (typeB == GameObject::ObjectType::Player && typeA == GameObject::ObjectType::Projectile)) {
        // player shouldn't collide with their own projectiles
        return;
    }
}

void GameManager::addObject(std::unique_ptr<GameObject> obj) {
    collisionManager.addObject(obj.get());
    gameObjects.push_back(std::move(obj));
}

void GameManager::handleInput(const SDL_Event& event, Player* player) {
    player->processEvent(event);
}

// Game state methods
void GameManager::pauseGame() {
    if (gameState == GameState::RUNNING) {
        gameState = GameState::PAUSED;
        std::cerr << "Game paused" << std::endl;
    }
}

void GameManager::resumeGame() {
    if (gameState == GameState::PAUSED) {
        gameState = GameState::RUNNING;
        std::cerr << "Game resumed" << std::endl;
    }
}

void GameManager::restartGame() {
    gameState = GameState::RUNNING;
    
    // Reset window to original dimensions and position
    std::pair<int,int> resolution = getResolution();
    int centerX = (resolution.first - windowWidth) / 2;
    int centerY = (resolution.second - windowHeight) / 2;
    
    // Reset window to original size and position
    if (window) {
        window->setSize(windowWidth, windowHeight);
        window->setPos(centerX, centerY);
        
        window->clearResizeRequests();
    }
    
    std::unique_ptr<GameObject> playerObj = nullptr;

    // find player
    for (auto it = gameObjects.begin(); it != gameObjects.end(); ++it) {
        if ((*it)->getType() == GameObject::ObjectType::Player) {
            playerObj = std::move(*it);
            gameObjects.erase(it);
            break;
        }
    }
    
    gameObjects.clear();
    collisionManager.clear();
    
    spawnTimer = 0.0f;
    beamTimer = 0.0f;
    pentagonTimer = 0.0f;
    gameOverTimer = 0.0f;
    
    if (playerObj) {
        Player* player = static_cast<Player*>(playerObj.get());
        
        SDL_Rect bounds = window->getBounds();
        int startX = bounds.x + bounds.w/2;
        int startY = bounds.y + bounds.h/2;
        
        player->setPosition(Vector2D(startX, startY));
        player->resetHealth();
        player->resetScore();
        player->resetDeathState();
        player->setActive(true);
        
        player->setDimensions(Vector2D(50, 50));
        player->setColor(255, 255, 255, 255);
        
        player->reinitializeCollision();
        
        collisionManager.addObject(playerObj.get());
        gameObjects.push_back(std::move(playerObj));

    } else { 
        std::cerr << "No player found, creating a new one" << std::endl;
        
        SDL_Rect bounds = window->getBounds();
        int startX = bounds.x + bounds.w/2;
        int startY = bounds.y + bounds.h/2;
        float playerSpeed = 500.0f;
        int playerRadius = 25;
        
        auto newPlayer = std::make_unique<Player>(
            Vector2D(startX, startY),
            playerRadius,
            playerSpeed,
            window
        );
        
        Player* newPlayerPtr = newPlayer.get();
        newPlayerPtr->setGameManager(this);

        // just to be sure
        newPlayerPtr->setDimensions(Vector2D(50, 50));
        newPlayerPtr->setPosition(Vector2D(startX, startY));
        
        collisionManager.addObject(newPlayer.get());
        gameObjects.push_back(std::move(newPlayer));
    }
    
    std::cerr << "Game restarted" << std::endl;
}

void GameManager::triggerGameOver() {
    if (gameState != GameState::GAME_OVER) {
        gameState = GameState::GAME_OVER;
        gameOverTimer = 0.0f;
        std::cerr << "Game over triggered" << std::endl;
    }
}