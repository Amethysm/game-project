#include <vector>
#include <algorithm>
#include "../include/entities.h"
#include "../include/collision_manager.h"
#include "../include/game_manager.h"

CollisionManager::CollisionManager() : gameManager(nullptr) {}

void CollisionManager::removeObject(GameObject* obj) {
    auto it = std::find(objects.begin(), objects.end(), obj);
    if (it != objects.end()) {
        objects.erase(it);
    }
}

void CollisionManager::checkCollisions() {
    // simple n^2
    // grid a bit too difficult and the number of objects isn't too large anyway
    int size = objects.size();
    for (int i = 0; i < size; i++) {
        GameObject* objA = objects[i];
        if (!objA->getActive()) continue;
        
        for (int j = i + 1; j < size; j++) {
            GameObject* objB = objects[j];
            if (!objB->getActive()) continue;
            
            if (objA->checkCollision(*objB)) {
                handleCollision(objA, objB);
            }
        }
    }
}

void CollisionManager::handleCollision(GameObject* obj1, GameObject* obj2) {
    // delegate to game manager
    if (gameManager) {
        gameManager->handleCollision(obj1, obj2);
    }
}