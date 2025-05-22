#include "../include/entities.h"
#include "../include/utils.h"
#include "../include/player.h"
#include "../include/window.h"
#include <iostream>
#include <algorithm>

void Triangle::initTriangleCollision() {
    vertices.clear();
    localVertices.clear();

    // triangle vertices
    // if im writing a init collision function for every shape i might as well just hardcode it in
    localVertices.push_back(Vector2D(0, -dimensions.y / 2.0f)); // Top vertex (pointing up)
    localVertices.push_back(Vector2D(-dimensions.x / 2.0f, dimensions.y / 2.0f)); // Bottom left vertex
    localVertices.push_back(Vector2D(dimensions.x / 2.0f, dimensions.y / 2.0f)); // Bottom right vertex

    updateCollisionVertices();
}

Triangle::Triangle(
    Vector2D pos,
    Vector2D dims,
    Vector2D vel,
    Scope scope,
    Uint8 r, Uint8 g, Uint8 b, Uint8 a,
    int speed,
    Window* window,
    Player* target,
    float health
):
    GameObject(pos, dims, vel, scope, r, g, b, a, speed),
    window(window),
    health(health),
    maxHealth(health),
    score(3.0f),
    homingTarget(target)
{
    texture = TextureManager::getTexture(fetchResourcePath("triangle.png"), window->renderer);
    spinDirection = (rand() % 2) ? 1 : -1;
    initTriangleCollision();
}

Triangle::~Triangle() {
    if (texture) {
        texture = nullptr;
    }
}

void Triangle::draw(SDL_Renderer* renderer) {
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

    // tinting
    SDL_SetTextureColorMod(texture, color[0], color[1], color[2]);

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

void Triangle::update(float deltaTime) {
    if (!isActive) return;
    if (getHealth() <= 0) {
        setActive(false);
        return;
    }

    if (homingTarget) {
        Vector2D targetPos = homingTarget->getPosition();
        Vector2D targetDir = (targetPos - position).normalize();
        if (targetDir.lengthSquared() > 1e-6f) { // epsilon, anything less is not meaningful
            float deviationStrength = 0.2;
            float deviationX = (((float)rand() / RAND_MAX) * 2.0f - 1.0f) * deviationStrength;
            float deviationY = (((float)rand() / RAND_MAX) * 2.0f - 1.0f) * deviationStrength;

            Vector2D randomDeviationVector(deviationX, deviationY);

            targetDir = (targetDir + randomDeviationVector).normalize();
            setDirection(targetDir);
        }
    }

    Vector2D vel = getDirection() * speed * deltaTime;
    // rotate around for fun why not
    float angleRotate = 60.0f;
    float dAngle = (float)spinDirection * angleRotate * M_PI / 180.0f * deltaTime;
    rotate(dAngle);

    // check flashing
    float currentTime = SDL_GetTicks() / 1000.0f;
    if (currentTime - lastHitTime < whiteFlashDuration) {
        setColor(255, 255, 255, 255); // white flash
    } else {
        // reset to yellow
        setColor(255, 255, 0, 255);
    }

    // move
    GameObject::move(vel);

    // does this need any movement restrictions? i'm not sure
}

void Triangle::drawCollisionVertices(SDL_Renderer* renderer) const {
    if (!isActive) return;

    SDL_Rect windowBounds = window->getBounds();

    // draw vertices at runtime
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
    // draw center
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    SDL_RenderDrawPoint(renderer, int(position.x - window->x), int(position.y - window->y));
}

void Triangle::drawHealthBar(SDL_Renderer* renderer) const {
    // runtime drawing
    if (!isActive) return;
    SDL_Rect windowBounds = window->getBounds();

    Vector2D pos = getPosition(); 
    Vector2D dim = getDimensions();

    // move the health bar to the top of the dimension
    SDL_Rect healthBarRect = {
        int(pos.x - windowBounds.x - dim.x / 2),
        int(pos.y - windowBounds.y - dim.y / 2) - 10,
        int(dim.x),
        5
    };

    // max health bar
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, &healthBarRect);

    // current health bar
    float healthRatio = health / maxHealth;
    SDL_Rect currentHealthBarRect = {
        healthBarRect.x,
        healthBarRect.y,
        int(healthBarRect.w * healthRatio),
        healthBarRect.h
    };
    // color transition: green -> yellow -> red
    if (healthRatio > 0.5f) {
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // green
    } else if (healthRatio > 0.25f) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255); // yellow
    } else {
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // red
    }
    SDL_RenderFillRect(renderer, &currentHealthBarRect);
}

void Triangle::changeHealthBy(float delta) {
    health += delta;
}