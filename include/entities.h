#ifndef ENTITIES_H
#define ENTITIES_H

#include "utils.h"
#include <SDL.h>
#include <vector>
#include <string>
#include <algorithm>

class Window;
class Player; // Forward declaration

class GameObject {
    private:
        static std::vector<Vector2D> getAxes(const std::vector<Vector2D>& vertices);
        static void project(
            const std::vector<Vector2D>& vertices,
            const Vector2D& axis,
            float& minOut, float& maxOut
        );

    public:
        enum class Scope {
            LOCAL,
            GLOBAL
        };
    
    protected:
        Vector2D position; // center coords btw
        Vector2D dimensions;
        Vector2D direction;
        std::vector<Vector2D> vertices;
        std::vector<Vector2D> localVertices;
        float angle; // rad
        Uint8 color[4]; // RGBA 
                        // textures are plain white, color is used for tinting
        SDL_Texture* texture;
        Scope scope;
        bool isActive;
        int speed; // pixels per second
    
    public:
        enum class ObjectType {
            Generic,
            Player,
            Projectile,
            Triangle,
            Beam,
            Pentagon
        };
        GameObject(Vector2D pos, Vector2D dims, Vector2D dir, Scope scope,
                   Uint8 r, Uint8 g, Uint8 b, Uint8 a, int speed
                   );
        
        virtual ~GameObject();

        // core functionalities
        virtual void draw(SDL_Renderer* renderer) = 0;
        virtual void update(float deltaTime) = 0;

        // positions and movement
        virtual void setPosition(const Vector2D& pos);
        virtual void setDirection(const Vector2D& dir);
        virtual void move(Vector2D delta);
        virtual void setSpeed(int speed) {this->speed = speed;}

        // dimensions and rotation
        virtual void setDimensions(const Vector2D& dims);
        virtual void setAngle(float angle);
        virtual void rotate(float dAngle); 

        // state
        virtual void setActive(bool active);
        virtual void setScope(Scope scope);

        // bounds detection
        virtual bool isOutOfBounds(const SDL_Rect& bounds) const;
        virtual bool isInWindow(const SDL_Rect& bounds) const;

        // getters
        virtual Vector2D getPosition() const;
        virtual Vector2D getDirection() const;
        virtual Vector2D getDimensions() const;
        virtual float getAngle() const;
        virtual Scope getScope() const;
        virtual bool getActive() const;
        virtual ObjectType getType() const {return ObjectType::Generic;}

        // specifically for circular objects
        // not using for now
        virtual bool isCircular() const {return false;}
        virtual float getRadius() const {return 0.0f;}

        // for collision detection
        virtual std::vector<Vector2D> getCollisionVertices() const;
        virtual void setCollisionVertices(const std::vector<Vector2D>& vertices);
        virtual void initRectangleCollision();
        virtual void initCircleCollision();
        
        // sat api
        bool checkCollision(const GameObject& other);
        void updateCollisionVertices();
        bool checkSATCollision(
            const std::vector<Vector2D>& vertices1,
            const std::vector<Vector2D>& vertices2
        );

        virtual void setColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
            color[0] = r; color[1] = g; color[2] = b; color[3] = a;
        }
};

// ---- projectile ------------------------------------------
class Projectile : public GameObject {
protected:
    Window* window;
    float damage;

    Player* player; // back pointer again yay
                    // i do like this
    
public:
    Projectile(Vector2D pos,
               Vector2D dims,
               Vector2D dir,
               Scope scope,
               Uint8 r, Uint8 g, Uint8 b, Uint8 a,
               int speed,
               Window* window);
    ~Projectile();

    void draw(SDL_Renderer* renderer) override;
    void update(float deltaTime) override;
    ObjectType getType() const override {return ObjectType::Projectile;}

    void setDamage(float damage) {this->damage = damage;}
    float getDamage() const {return damage;}
};

// ---- triangle ------------------------------------------
class Triangle : public GameObject {
    protected:
        Window* window;
        Player* homingTarget; // back pointer
        // this is a bit of a mess
        float health, maxHealth, score;
        float lastHitTime = 0.0f;
        float whiteFlashDuration = 0.05f; // seconds
        void initTriangleCollision();
        int spinDirection; // 1: clockwise, -1: anticlockwise
    
    public:
        Triangle(Vector2D pos,
                 Vector2D dims,
                 Vector2D dir,
                 Scope scope,
                 Uint8 r, Uint8 g, Uint8 b, Uint8 a,
                 int speed,
                 Window* window,
                 Player* target = nullptr,
                 float health = 50.0f);
        ~Triangle();

        void draw(SDL_Renderer* renderer) override;
        void update(float deltaTime) override;
        ObjectType getType() const override {return ObjectType::Triangle;}

        void setHealth(float health) {this->health = std::clamp(health, 0.0f, maxHealth);}
        void setScore(float score) {this->score = score;}
        void setLastHitTime(float time) {lastHitTime = time;}

        void changeHealthBy(float delta);

        float getHealth() const {return health;}
        float getScore() const {return score;}

        void drawCollisionVertices(SDL_Renderer* renderer) const;
        void drawHealthBar(SDL_Renderer* renderer) const;
};

// ---- beam -------------------------------------------------
class Beam : public GameObject {
    public:
        enum class BeamState {
            WARNING,    // Initial warning state
            EXPANDING,  // Beam is expanding
            ACTIVE,     // Fully expanded
            FADING      // Disappearing
        };

    private:
        Window* window;
        Player* target;
        
        BeamState state;
        float stateTimer;       // Time spent in current state
        float warningDuration;  // How long the warning shows
        bool hasExpandedWarning; // Whether the warning has been shown
                                // (used to prevent double expansion)

        float expandDuration;   // How long it takes to fully expand
                                // (expands until touching the other edge)

        float activeDuration;   // How long the beam stays at full size
        float fadeDuration;     // How long the beam takes to fade out
        
        int direction; // 0=bottom, 1=top, 2=right, 3=left
        int startEdge; // 0=top, 1=bottom, 2=left, 3=right
        // this is confusing
        float beamProgress;
        
        float beamWidth;
        float damage;
        
        // initrectanglecollision already exist, since the beam is just a long rectangle, might as well use that

    
    public:
        Beam(Vector2D pos, Vector2D dims, /*Vector2D vel,*/ Scope scope,
             Uint8 r, Uint8 g, Uint8 b, Uint8 a, int speed, int startEdge,
             Window* window, Player* target, float beamWidth = 200.0f);
             
        ~Beam();
        
        // Override virtual methods
        void update(float deltaTime) override;
        void draw(SDL_Renderer* renderer) override;
        void drawCollisionVertices(SDL_Renderer* renderer) const;

        // helper
        void expandTop(float delta);
        void expandBottom(float delta);
        void expandLeft(float delta);
        void expandRight(float delta);
        void expandByDirection(int direction, float delta, float compensate);
        
        // Type identification
        GameObject::ObjectType getType() const override { return ObjectType::Beam; }
        
        // State access
        BeamState getState() const { return state; }
};

// ---- Pentagon -------------------------------------------------
class Pentagon : public GameObject {
protected:
    Window* window;
    Player* player; // Reference to player for collision handling
    float health, maxHealth, score;
    float lastHitTime = 0.0f;
    float whiteFlashDuration = 0.05f; // seconds
    void initPentagonCollision();

public:
    Pentagon(Vector2D pos,
             Vector2D dims,
             Scope scope,
             Window* window,
             Player* player,
             float health = 500.0f);
    ~Pentagon();

    void draw(SDL_Renderer* renderer) override;
    void update(float deltaTime) override;
    ObjectType getType() const override {return ObjectType::Pentagon;}

    void setHealth(float health) {this->health = std::clamp(health, 0.0f, maxHealth);}
    void setLastHitTime(float time) {lastHitTime = time;}
    void changeHealthBy(float delta);

    float getScore() {return score;}

    float getHealth() const {return health;}
    void drawCollisionVertices(SDL_Renderer* renderer) const;
    void drawHealthBar(SDL_Renderer* renderer) const;
};

#endif // ENTITIES_H
