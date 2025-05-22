#include "../include/entities.h"
#include "../include/utils.h"
#include "../include/player.h"
#include "../include/window.h"
#include <iostream>
#include <algorithm>

void Pentagon::initPentagonCollision() {
    vertices.clear();
    localVertices.clear();

    // Define vertices in clockwise order around the perimeter
    // Top vertex
    localVertices.push_back(Vector2D(0, -dimensions.y / 2.0f)); 
    // Top right vertex
    localVertices.push_back(Vector2D(dimensions.x / 2.0f, -dimensions.y / 6.0f)); 
    // Bottom right vertex
    localVertices.push_back(Vector2D(dimensions.x / 2.0f - 18.0f, dimensions.y / 2.0f - 4.0f)); 
    // Bottom left vertex
    localVertices.push_back(Vector2D(-dimensions.x / 2.0f + 18.0f, dimensions.y / 2.0f - 4.0f)); 
    // Top left vertex
    localVertices.push_back(Vector2D(-dimensions.x / 2.0f, -dimensions.y / 6.0f)); 

    updateCollisionVertices();
}

Pentagon::Pentagon(
    Vector2D pos,
    Vector2D dims,
    Scope scope,
    Window* window,
    Player* player,
    float health
):
    GameObject(pos, dims, Vector2D(0, 0), scope, 0, 255, 255, 255, 0), // Cyan color (0, 255, 255)
    window(window),
    player(player),
    health(health),
    maxHealth(health),
    score(50.0f)
{
    texture = TextureManager::getTexture(fetchResourcePath("pentagon.png"), window->renderer);
    // random angle at init
    angle = (rand() % 360) * M_PI / 180.0f;
    initPentagonCollision();
}

Pentagon::~Pentagon() {
    if (texture) {
        texture = nullptr;
    }
}

void Pentagon::draw(SDL_Renderer* renderer) {
    if (!isActive) return;
    SDL_Rect windowBounds = window->getBounds();
    if (!isInWindow(windowBounds)) {
        return;
    }

    Vector2D p = getPosition();
    Vector2D d = getDimensions();
    SDL_Rect rect = {
        int(p.x - window->x - d.x/2),
        int(p.y - window->y - d.y/2),
        int(d.x),
        int(d.y)
    };

    // Apply tinting effect
    float currentTime = SDL_GetTicks() / 1000.0f;
    if (currentTime - lastHitTime < whiteFlashDuration) {
        SDL_SetTextureColorMod(texture, 255, 255, 255); // White flash when hit
    } else {
        SDL_SetTextureColorMod(texture, color[0], color[1], color[2]); // Cyan color
    }

    SDL_Point center = {rect.w/2, rect.h/2};
    float angleDeg = angle * 180.0f / M_PI;
    SDL_RenderCopyEx(
        renderer, 
        texture, 
        nullptr, 
        &rect, 
        angleDeg, 
        &center, 
        SDL_FLIP_NONE
    );

    // uncomment if debug
    // drawCollisionVertices(renderer);
    drawHealthBar(renderer);
}

void Pentagon::update(float deltaTime) {
    if (!isActive) return;
    if (getHealth() <= 0) {
        setActive(false);
        return;
    }

    // rotate very slowly
    float angleRotate = 10.0f;
    float dAngle = angleRotate * M_PI / 180.0f * deltaTime;
    rotate(dAngle);

    // Reset color after white flash
    float currentTime = SDL_GetTicks() / 1000.0f;
    if (currentTime - lastHitTime < whiteFlashDuration) {
        setColor(255, 255, 255, 255); // White flash
    } else {
        setColor(0, 255, 255, 255); // Reset to cyan
    }

}

void Pentagon::drawCollisionVertices(SDL_Renderer* renderer) const {
    if (!isActive) return;

    // Draw vertices for debugging purposes
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    for (size_t i = 0; i < vertices.size(); i++) {
        const Vector2D& v1 = vertices[i];
        const Vector2D& v2 = vertices[(i + 1) % vertices.size()];
        SDL_RenderDrawLine(
            renderer,
            int(v1.x - window->x), int(v1.y - window->y),
            int(v2.x - window->x), int(v2.y - window->y)
        );
    }
    // Draw center point
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    SDL_RenderDrawPoint(renderer, int(position.x - window->x), int(position.y - window->y));
}

void Pentagon::drawHealthBar(SDL_Renderer* renderer) const {
    if (!isActive) return;
    SDL_Rect windowBounds = window->getBounds();

    Vector2D pos = getPosition(); 
    Vector2D dim = getDimensions();

    // Position health bar above the pentagon
    SDL_Rect healthBarRect = {
        int(pos.x - windowBounds.x - dim.x / 2),
        int(pos.y - windowBounds.y - dim.y / 2) - 10,
        int(dim.x),
        5
    };

    // draw health container
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, &healthBarRect);

    // draw health ba
    float healthRatio = health / maxHealth;
    SDL_Rect currentHealthBarRect = {
        healthBarRect.x,
        healthBarRect.y,
        int(healthBarRect.w * healthRatio),
        healthBarRect.h
    };
    
    // color transition based on health
    if (healthRatio > 0.5f) {
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // Green
    } else if (healthRatio > 0.25f) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255); // Yellow
    } else {
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Red
    }
    SDL_RenderFillRect(renderer, &currentHealthBarRect);
}

void Pentagon::changeHealthBy(float delta) {
    health = std::clamp(health + delta, 0.0f, maxHealth);
    
    if (delta < 0) {
        lastHitTime = SDL_GetTicks() / 1000.0f;
    }
}