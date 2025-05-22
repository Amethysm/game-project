#include "../include/entities.h"
#include "../include/utils.h"
#include "../include/player.h"
#include "../include/window.h"
#include <iostream>

Projectile::Projectile(
    Vector2D pos,
    Vector2D dims,
    Vector2D vel,
    Scope scope,
    Uint8 r, Uint8 g, Uint8 b, Uint8 a,
    int speed,
    Window* window
):
    GameObject(pos, dims, vel, scope, r, g, b, a, speed),
    window(window)
{
    texture = TextureManager::getTexture(fetchResourcePath("projectile.png"), window->renderer);
    initCircleCollision();
}

Projectile::~Projectile() {
    if (texture) {
        texture = nullptr;
    }
}

void Projectile::draw(SDL_Renderer* renderer) {
    Vector2D p = getPosition();
    Vector2D d = getDimensions();
    SDL_Rect rect = {
        int(p.x - window->x - d.x/2),
        int(p.y - window->y - d.y/2),
        int(d.x),
        int(d.y)
    };
    SDL_Point center = {rect.w/2, rect.h/2};
    float angleDeg = angle * 180.0f / M_PI;
    SDL_RenderCopyEx(
        renderer, 
        texture, 
        nullptr, 
        &rect, 
        angleDeg, 
        &center, 
        SDL_FLIP_NONE);
}

// todo update this to handle global bounds (if needed)
void Projectile::update(float deltaTime) {
    if (!isActive) return;

    // handle window edge collision
    // this is likely the only object that will have this interaction, so for now it makes sense to put it exclusively here
    // might change if i add anything else, but unlikely
    Vector2D pos = getPosition(); 
    Vector2D vel = getDirection() * speed * deltaTime;
    Vector2D newPos = pos + vel;

    SDL_Rect windowBounds = window->getBounds();

    int expandAmount = 100; // pixels
                           // might overshoot a little due to the way window size is calculated
    int edgeHit = -1; // 0 = top, 1 = bottom, 2 = left, 3 = right, -1 = none
                      // we need to use int because screenEdge is currently a bool array, will change in the future
    
    // left right top bot
    if (newPos.x - dimensions.x / 2 <= windowBounds.x) edgeHit = 2;
    if (newPos.x + dimensions.x / 2 >= windowBounds.x + windowBounds.w) edgeHit = 3;
    if (newPos.y - dimensions.y / 2 <= windowBounds.y) edgeHit = 0;
    if (newPos.y + dimensions.y / 2 >= windowBounds.y + windowBounds.h) edgeHit = 1;

    if (edgeHit != -1) {
        int deltaAmount[4] = {0, 0, 0, 0};
        deltaAmount[edgeHit] = expandAmount;
        window->createResizeRequest(
            deltaAmount[0], deltaAmount[1], 
            deltaAmount[2], deltaAmount[3], 
            this->speed / 100,
            0.3f
        );
        setActive(false); // deactivate the projectile
        return; // exit
    }
    GameObject::move(vel); // move as normal otherwise
}