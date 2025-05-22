#include "../include/player.h"
#include "../include/game_manager.h"
#include "../include/utils.h"
#include "../include/window.h"
#include "../include/entities.h"
#include <SDL.h>
#include <iostream>
#include <map>
#include <algorithm>

Player::Player(const Vector2D& pos,
               float radius,
               float speed,
               Window* window) :
     GameObject(
        pos,
        Vector2D(radius*2, radius*2), // dimensions
        Vector2D(0, 0), // direction (likely don't need here)
        GameObject::Scope::LOCAL,
        255, 255, 255, 255,
        speed
    ),
    window(window),
    texture(TextureManager::getTexture(fetchResourcePath(texturePath), window->renderer)),
    gameManager(nullptr),
    knockbackVelocity(Vector2D(0, 0)),
    knockbackDecay(1.0f),
    lastShot(0)
{
    if (!texture) {
        std::cerr << "Failed to load player texture: " << IMG_GetError() << '\n';
        return;
    }
    cerr << "Initial position (before init): " << position.x << ", " << position.y << endl; 
    initCircleCollision();
    cerr << "Initial position: " << position.x << ", " << position.y << endl;
}

void Player::setSpeed(float s) {
    GameObject::speed = s;
}

void Player::draw(SDL_Renderer* renderer) {
    Vector2D p = getPosition();
    Vector2D d = getDimensions();
    SDL_Rect rect = {
        int(p.x - window->x - d.x/2),
        int(p.y - window->y - d.y/2),
        int(d.x),
        int(d.y)
    };
    
    // draw death animation
    if (isDying) {
        SDL_SetTextureColorMod(texture, color[0], color[1], color[2]);
        SDL_SetTextureAlphaMod(texture, color[3]);
        
        float angleDeg = angle * 180.0f / M_PI;
        SDL_Point center = {rect.w/2, rect.h/2};
        
        SDL_RenderCopyEx(
            renderer,
            texture,
            nullptr,
            &rect,
            angleDeg,
            &center,
            SDL_FLIP_NONE
        );
    } else {
        SDL_RenderCopy(renderer, texture, nullptr, &rect);
    }
    
    // Get the window dimensions to position the text
    int windowWidth, windowHeight;
    SDL_GetRendererOutputSize(renderer, &windowWidth, &windowHeight);
    
    int fontSize = 16;
    
    // Draw health text at the bottom right of the screen
    std::string healthText = std::to_string(health) + "/" + std::to_string(maxHealth) + " HP";
    
    // Position text at bottom right with some padding
    int textX = windowWidth - (healthText.length() * (fontSize/2 + 1)) - 10;
    int textY = windowHeight - fontSize - 10;
    
    // Draw health text
    SDL_Color healthColor = {255, 255, 255, 255}; // White by default
    
    // Change color based on health percentage
    float healthPercent = float(health) / float(maxHealth);
    if (healthPercent <= 0.25f) {
        // Red for low health (25% or below)
        healthColor = {255, 0, 0, 255};
    } else if (healthPercent <= 0.5f) {
        // Yellow for medium health (50% or below)
        healthColor = {255, 255, 0, 255};
    } else {
        // Green for good health (above 50%)
        healthColor = {0, 255, 0, 255};
    }
    
    renderText(renderer, healthText, textX, textY, fontSize, healthColor);
    
    // Draw score text at the top left of the screen
    std::string scoreText = "Score: " + std::to_string(score);
    int scoreTextX = 10;
    int scoreTextY = 10;
    SDL_Color scoreColor = {255, 255, 255, 255}; // White color for score
    
    renderText(renderer, scoreText, scoreTextX, scoreTextY, fontSize, scoreColor);
    
    // uncomment if debug
    // drawCollisionVertices(renderer);
}

void Player::update(float deltaTime) {
    // Check if player is in death animation
    if (isDying) {
        updateDeathAnimation(deltaTime);
        return; // Skip normal update when dying
    }
    
    // Handle rapid knockback phase first (60% of knockback - fast but brief)
    if (rapidKnockbackTimer > 0) {
        // Apply rapid movement
        Vector2D rapidVel = rapidKnockbackVelocity * deltaTime;
        GameObject::move(rapidVel);
        
        // Decrement timer
        rapidKnockbackTimer -= deltaTime;
        
        // Transfer any remaining velocity to the regular knockback when time is up
        if (rapidKnockbackTimer <= 0) {
            // Add any remaining rapid velocity to regular knockback for smooth transition
            if (rapidKnockbackTimer < 0) {
                float remainingTimeFraction = -rapidKnockbackTimer / rapidKnockbackDuration;
                knockbackVelocity += rapidKnockbackVelocity * remainingTimeFraction;
            }
            rapidKnockbackVelocity = Vector2D(0, 0);
        }
    }
    
    // Handle regular knockback phase
    if (knockbackVelocity.x != 0 || knockbackVelocity.y != 0) {
        Vector2D vel = knockbackVelocity * deltaTime;
        GameObject::move(vel);
        
        // decay
        float f = std::max(0.0f, 1.0f - knockbackDecay * deltaTime);
        knockbackVelocity *= f;
        
        // reset when below minimum threshold
        if (knockbackVelocity.lengthSquared() < knockbackMinThreshold * knockbackMinThreshold) {
            knockbackVelocity = Vector2D(0, 0);
        }
    }
    
    std::map<std::string, bool> keyStates = getKeyState();
    SDL_Rect bounds = window->getBounds();
    updateMovement(keyStates, bounds, deltaTime);

    // shooting
    if (isDying || !isActive) {
        return;
    }
    int mouseX, mouseY;
    Uint32 m = SDL_GetMouseState(&mouseX, &mouseY);
    std::map<std::string, bool> mouseState = {
        {"left", m & SDL_BUTTON(SDL_BUTTON_LEFT)},
        {"right", m & SDL_BUTTON(SDL_BUTTON_RIGHT)}
    };
    
    initCircleCollision();
}

void Player::updateMovement(const std::map<std::string,bool>& keyStates, const SDL_Rect& bounds, float deltaTime) {
    Vector2D delta(0, 0);
    if (keyStates.at("up")) delta.y -= 1;
    if (keyStates.at("down")) delta.y += 1;
    if (keyStates.at("left")) delta.x -= 1;
    if (keyStates.at("right")) delta.x += 1;

    if (delta.x && delta.y) delta *= 0.7071f; // diagonal
    Vector2D vel = delta * speed * deltaTime;
    GameObject::move(vel);

    // clamp
    Vector2D pos = getPosition();
    float halfWidth = getDimensions().x / 2.0f;
    float halfHeight = getDimensions().y / 2.0f;

    pos.x = std::clamp(pos.x, 
                       float(bounds.x) + halfWidth, 
                       float(bounds.x + bounds.w) - halfWidth
                      );

    pos.y = std::clamp(pos.y, 
                       float(bounds.y) + halfHeight, 
                       float(bounds.y + bounds.h) - halfHeight
                      );
    
    GameObject::setPosition(pos);
}

void Player::changeHealthBy(int delta) {
    int current = SDL_GetTicks();
    if (current - lastHitTime < static_cast<int>(gracePeriod * 1000)) return; // ignore hits during grace period
    
    // Record the hit time for grace period
    lastHitTime = current;
    
    // Update health with clamp at 0
    health = std::max(0, health + delta);
    
    // If health reaches 0, start death sequence instead of immediately deactivating
    if (health == 0 && !isDying) {
        startDeathSequence();
    }
}

void Player::startDeathSequence() {
    std::cerr << "Player death sequence started" << std::endl;
    isDying = true;
    deathTimer = 0.0f;
    // let the death animation play out
}

void Player::reinitializeCollision() {
    std::cerr << "Reinitializing collision for player at position: " << position.x << ", " << position.y << std::endl;
    vertices.clear();
    initCircleCollision();
}

void Player::updateDeathAnimation(float deltaTime) {
    if (!isDying) return;
    
    deathTimer += deltaTime;
    
    // During death animation, flash the player and rotate
    float flashRate = 10.0f; // flashes per second
    bool flashVisible = (static_cast<int>(deathTimer * flashRate) % 2 == 0);
    
    if (flashVisible) {
        setColor(255, 0, 0, 255); // Red flash
    } else {
        setColor(255, 255, 255, 100); // Fade out
    }

    // Shrink the player
    float shrinkFactor = 1.0f - (deathTimer / deathAnimationDuration);
    Vector2D originalDimensions(getDimensions().x, getDimensions().y);
    setDimensions(originalDimensions * std::max(0.1f, shrinkFactor));
    
    // When animation completes, actually set to inactive
    if (deathTimer >= deathAnimationDuration) {
        std::cerr << "Player death animation complete, setting inactive" << std::endl;
        isActive = false;
        
        // Trigger game over state
        if (gameManager) {
            gameManager->triggerGameOver();
        }
    }
}

void Player::processEvent(const SDL_Event& event) {
    if (isDying || !isActive) {
        return; // ded, not handling inputs
    }
    // only concerns left mouse click for shooting
    // for now
    if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
        Vector2D pos = getPosition();
        Vector2D target((float)event.button.x, (float)event.button.y);
        Vector2D dir = (target - pos).normalize();

        gameManager->spawnProjectile(pos, dir, projectileSpeed);
    }
}

// yeet
/*
void Player::shoot(const std::map<std::string,bool>& mouseState,
                   std::vector<Projectile>& projectiles,
                   int mouseX, int mouseY)
{
    if (!mouseState.at("left")) return;
    Uint32 now = SDL_GetTicks();
    if (now - lastShot < 1000.0f / fireRate) return;
    lastShot = now;

    Vector2D p = getPosition();
    Vector2D target((float)mouseX, (float)mouseY);
    Vector2D dir = (target - p).normalize();
    Vector2D dim(15,15);

    Projectile proj(p, dim, dir, GameObject::Scope::LOCAL, 255, 255, 255, 255, projectileSpeed, window);
    projectiles.push_back(proj);
}
*/
void Player::applyKnockback(const Vector2D& impulse) {
    // phase 1
    Vector2D immediateMove = impulse * phaseOneMultiplier;
    GameObject::move(immediateMove);
    
    // phase 2
    rapidKnockbackVelocity = impulse * phaseTwoMultiplier;
    rapidKnockbackTimer = rapidKnockbackDuration;
    
    // phase 3
    knockbackVelocity = impulse * phaseThreeMultiplier;
}

void Player::drawCollisionVertices(SDL_Renderer* renderer) const {
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