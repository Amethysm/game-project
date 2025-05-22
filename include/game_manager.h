#ifndef GAME_MANAGER_H
#define GAME_MANAGER_H

#include <vector>
#include <memory>
#include <random>
#include "entities.h"
#include "window.h"
#include "collision_manager.h"

class Player;

// Game state enum to track current game state
enum class GameState {
    RUNNING,
    PAUSED,
    GAME_OVER,
    MENU
};

class GameManager {
private:
    Window* window;
    std::vector<std::unique_ptr<GameObject>> gameObjects;
    std::mt19937 rng;
    float spawnTimer;
    float spawnInterval;
    float beamTimer; 
    float beamInterval;
    
    // Game state management
    GameState gameState = GameState::RUNNING;
    float gameOverDelay = 2.0f;
    float gameOverTimer = 0.0f;

    // Collision manager
    CollisionManager collisionManager;

    // Pentagon spawn variables
    float pentagonTimer = 0.0f;
    float pentagonInterval = 15.0f;

public:
    GameManager(Window* window);
    ~GameManager();

    void pauseGame();
    void resumeGame();
    void restartGame();
    void triggerGameOver();
    GameState getGameState() const { return gameState; }
    bool isPaused() const { return gameState == GameState::PAUSED || gameState == GameState::GAME_OVER; }
    
    void addObject(std::unique_ptr<GameObject> obj);
    void handleInput(const SDL_Event& event, Player* player);

    // Spawn methods
    void spawnTriangle(Vector2D pos, Vector2D dir, Player* target = nullptr);
    void spawnProjectile(Vector2D pos, Vector2D dir, int speed);
    void spawnRandomEnemy(Player* target = nullptr);
    void spawnBeam(Player* target);
    void spawnPentagon(Vector2D pos, Player* player);
    void spawnPentagonGroup(Player* player);

    // Game loop methods
    void update(float deltaTime);
    void draw(SDL_Renderer* renderer);
    void cleanupInactiveObjects();
    
    // Collision handling
    void checkCollisions();
    void handleCollision(GameObject* a, GameObject* b);
    
    // getters
    std::vector<std::unique_ptr<GameObject>>& getGameObjects() { return gameObjects; }
};
#endif