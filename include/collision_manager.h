#pragma once
#include "entities.h"
#include "utils.h"
#include <vector>

class GameManager; // forward declaration

class CollisionManager {
private:
    std::vector<GameObject*> objects;
    GameManager* gameManager;
    
public:
    CollisionManager();

    void setGameManager(GameManager* gm) { gameManager = gm; }
    void addObject(GameObject* obj) { objects.push_back(obj); }
    void removeObject(GameObject* obj);
    void clear() { objects.clear(); } // Add method to clear all objects
    void checkCollisions();
    void handleCollision(GameObject* obj1, GameObject* obj2);
};