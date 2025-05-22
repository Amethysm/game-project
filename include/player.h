// used ai to reformat this file
// honestly it looks terrible won't do it again

#pragma once

// SDL
#include <SDL.h>
#include <SDL_image.h>

// STL
#include <map>
#include <string>
#include <vector>

// project
#include "entities.h"
#include "utils.h"
#include "window.h"

class GameManager; // Add this forward declaration

class Player : public GameObject {
private:
    // constants
    static constexpr char texturePath[]    = "player.png";
    static constexpr Uint8 color[4]        = { 255, 255, 255, 255 }; // RGBA

    // state
    int health                              = 20;
    int maxHealth                           = 20;
    int score                               = 0;
    Vector2D knockbackVelocity{ 0, 0 };
    Vector2D rapidKnockbackVelocity{ 0, 0 }; // For the rapid phase
    
    // --- knockback parameters (for easier fine tuning) ---
    // phase 1 parameters
    float phaseOneMultiplier               = 0.2f;   // 20% immediate

    // phase 2 parameters
    float phaseTwoMultiplier               = 16.0f;  // Knockback multiplier for rapid phase
    float rapidKnockbackDuration           = 0.04f;  // Duration of rapid phase in seconds

    // phase 3 parameters
    float phaseThreeMultiplier             = 0.1f;   // 10% (less trailing)
    float rapidKnockbackTimer              = 0.0f;   // helper timer
    float knockbackDecay                   = 12.0f;   // Decay rate for phase three (higher = faster decay)
    float knockbackMinThreshold            = 0.1f;   // minimum threshold before resetting to zero
    /// --- end of knockback parameters ---

    // uhhh
    float gracePeriod                      = 0.2f;   // seconds 
    float lastHitTime                      = 0.0f;
    bool isDying                           = false;  // death animation flag
    float deathTimer                       = 0.0f;   // time since death
    float deathAnimationDuration           = 1.0f;   // self explanatory
    float redFlashDuration                = 0.05f;   // seconds

    // timing
    float projectileSpeed                  = 1500.0f; // pps
    float fireRate                         = 3.0f;    // shots/sec
    Uint32 lastShot                        = 0;       // last shot time

    // external refs
    GameManager* gameManager                = nullptr; // back pointer
    Window* window                          = nullptr; // yes

    SDL_Texture* texture                    = nullptr;

public:
    // construction
    explicit Player(const Vector2D& pos, float radius, float speed, Window* window);
    ~Player() override = default;

    // configuration
    void setSpeed(float speed);
    void setHealth(int hp)          { health = hp; }
    int  getHealth() const          { return health; }
    void changeHealthBy(int delta);
    void resetHealth()              { health = maxHealth; } // Reset health to max
    void resetDeathState()          { isDying = false; } // Reset death animation state
    void setHitTime(float time) { lastHitTime = time; }
    bool isDead() const         { return health <= 0; }
    bool isInDeathAnimation() const { return isDying; }
    void setScore(int score)         { this->score = score; }
    void addScore(int score)         { this->score += score; }
    void resetScore()                { score = 0; } // Reset score to zero
    int  getScore() const            { return score; } 
    
    // death handling
    void startDeathSequence();
    void updateDeathAnimation(float deltaTime);
    void reinitializeCollision(); // New method to reinitialize collision after restart

    // gameplay
    void updateMovement(const std::map<std::string, bool>& keys,
                        const SDL_Rect& bounds, float dt);
    void shoot(const std::map<std::string, bool>& mouse,
               std::vector<Projectile>& projectiles,
               int mouseX, int mouseY);
    void applyKnockback(const Vector2D& impulse);

    // overrides
    void draw(SDL_Renderer* renderer) override;
    void update(float deltaTime) override;
    GameObject::ObjectType getType() const override { return GameObject::ObjectType::Player; }

    // something
    void processEvent(const SDL_Event& event);
    void setGameManager(GameManager* gm) { gameManager = gm; }
    void drawCollisionVertices(SDL_Renderer* renderer) const;
};
