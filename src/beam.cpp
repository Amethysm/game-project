#include "../include/entities.h"
#include "../include/utils.h"
#include "../include/player.h"
#include "../include/window.h"
#include <iostream>
#include <algorithm>

Beam::Beam(
    Vector2D pos, // initial pos, preferably already set at the edge
    Vector2D dims, // dimensions, for rendering? should be initial size, will change
    //Vector2D dir, i don't use this
    Scope scope, // honestly never used this so far
    Uint8 r, Uint8 g, Uint8 b, Uint8 a, // color for tinting
    int speed, // ...
    int startEdge,
    Window* window, // bounds, and render
    Player* target, // target for homing
    float beamWidth
) :
    GameObject(pos, dims, Vector2D(0,0), scope, r, g, b, a, speed),
    window(window),
    target(target),
    state(BeamState::WARNING),
    stateTimer(0.0f),
    warningDuration(1.5f),
    expandDuration(0.2f),
    activeDuration(1.0f),
    fadeDuration(1.0f),
    beamWidth(beamWidth),
    damage(2.0f),
    beamProgress(0.0f)
{
    // direction is opposite to startEdge
    switch (startEdge) {
        case 0: // top
            direction = 1; // down
            break;
        case 1: // bottom
            direction = 0; // up
            break;
        case 2: // left
            direction = 3; // right
            break;
        case 3: // right
            direction = 2; // left
            break;
        default:
            std::cerr << "Invalid start edge for beam." << std::endl;
            direction = -1; // invalid
    }

    angle = 0.0f;
    initRectangleCollision();
}

Beam::~Beam() {
    if (texture) {
        texture = nullptr;
    }
}

void Beam::draw(SDL_Renderer* renderer) {
    if (!isActive) return;

    SDL_Rect windowBounds = window->getBounds();
    /*if (!isInWindow(windowBounds)) {
        return;
    }
        */
    // runtime drawing

    Vector2D pos = getPosition();
    Vector2D dim = getDimensions();
    SDL_Rect rect = {
        int(pos.x - window->x - dim.x/2),
        int(pos.y - window->y - dim.y/2),
        int(dim.x),
        int(dim.y)
    };

    // color tinting
    // yellow: warning stage, white: active

    Uint8 r = 255, g = 255, b = 255, a = 255;

    switch (state) {
        case BeamState::WARNING:
            r = 255; g = 255; b = 0; a = 255; // yellow
            break;
        case BeamState::EXPANDING:
            r = 255; g = 255; b = 255; a = 255; // white
            break;
        case BeamState::ACTIVE:
            r = 255; g = 255; b = 255; a = 255; // white
            break;
        case BeamState::FADING:
            r = 255; g = 255; b = 255;
            float fadeProgress = std::min(stateTimer / fadeDuration, 1.0f);
            a = Uint8(255 * (1.0f - fadeProgress)); // fade out
            break;
    }

    // Save the current blend mode
    SDL_BlendMode oldBlendMode;
    SDL_GetRenderDrawBlendMode(renderer, &oldBlendMode);
    
    // Set blend mode to enable alpha blending
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    
    // Set color with alpha
    SDL_SetRenderDrawColor(renderer, r, g, b, a);
    SDL_RenderFillRect(renderer, &rect);
    
    // Restore the previous blend mode
    SDL_SetRenderDrawBlendMode(renderer, oldBlendMode);

    // uncomment if debug
    // drawCollisionVertices(renderer);
}

void Beam::drawCollisionVertices(SDL_Renderer* renderer) const {
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

void Beam::expandTop(float delta) {
    float originalHeight = dimensions.y;
    dimensions.y += delta;
    position.y -= delta / 2.0f;
    initRectangleCollision();
}

void Beam::expandBottom(float delta) {
    dimensions.y += delta;
    position.y += delta / 2.0f;
    initRectangleCollision();
}

void Beam::expandLeft(float delta) {
    dimensions.x += delta;
    position.x -= delta / 2.0f;
    initRectangleCollision();
}

void Beam::expandRight(float delta) {
    dimensions.x += delta;
    position.x += delta / 2.0f;
    initRectangleCollision();
}

void Beam::expandByDirection(int direction, float delta, float compensate) {
    switch (direction) {
        case 0:  // Bottom
            expandBottom(delta);
            expandTop(compensate);
            break;
        case 1:  // Top
            expandTop(delta);
            expandBottom(compensate);
            break;
        case 2:  // Right
            expandRight(delta);
            expandLeft(compensate);
            break;
        case 3:  // Left
            expandLeft(delta);
            expandRight(compensate);
            break;
        default:
            // Invalid direction, do nothing
            break;
    }
}
 
void Beam::update(float deltaTime) {
    if (!isActive) return;
    stateTimer += deltaTime;

    SDL_Rect windowBounds = window->getBounds();
    std::pair<int, int> screenResolution = getResolution();

    Vector2D pos = getPosition();
    Vector2D dim = getDimensions();

    float targetExpandUp = screenResolution.second, targetExpandDown = screenResolution.second, targetExpandLeft= screenResolution.first, targetExpandRight = screenResolution.first;
    // so realistically it'll expand by 2x screen resolution both ways
    // not optimal but it works
    
    switch (state) {
        case BeamState::WARNING:
            if (!hasExpandedWarning) {
                float warningExpandMultiplier = 0.05f;
                switch (direction) {
                    case 0: // Bottom
                        expandBottom(targetExpandDown * warningExpandMultiplier);
                        expandTop(targetExpandUp * warningExpandMultiplier);
                        setDimensions(Vector2D(beamWidth, dim.y));
                        break;
                    case 1: // Top
                        expandTop(targetExpandUp * warningExpandMultiplier);
                        expandBottom(targetExpandDown * warningExpandMultiplier);
                        setDimensions(Vector2D(beamWidth, dim.y));
                        break;
                    case 2: // Right
                        expandRight(targetExpandRight * warningExpandMultiplier);
                        expandLeft(targetExpandLeft * warningExpandMultiplier);
                        setDimensions(Vector2D(dim.x, beamWidth));
                        break;
                    case 3: // Left
                        expandLeft(targetExpandLeft * warningExpandMultiplier);
                        expandRight(targetExpandRight * warningExpandMultiplier);
                        setDimensions(Vector2D(dim.x, beamWidth));
                        break;
                }
                hasExpandedWarning = true;
            }
            // attach to corresponding edge
            switch (direction) {
                case 0: setPosition(Vector2D(pos.x, windowBounds.y - dim.y + 20)); break;
                case 1: setPosition(Vector2D(pos.x, windowBounds.y + windowBounds.h - 20)); break;
                case 2: setPosition(Vector2D(windowBounds.x - dim.x + 20, pos.y)); break;
                case 3: setPosition(Vector2D(windowBounds.x + windowBounds.w - 20, pos.y)); break;
            }
            if (stateTimer >= warningDuration) {
                state = BeamState::EXPANDING;
                cerr << "Duration in warning state: " << stateTimer << endl;
                stateTimer = 0.0f;
            }
        break;
    
        case BeamState::EXPANDING:
            if (stateTimer >= expandDuration) {
                state = BeamState::ACTIVE;
                cerr << "Duration in expanding state: " << stateTimer << endl;
                stateTimer = 0.0f;
            } else {
                // proportional expansion based on stateTimer
                switch (direction) {
                    case 0: // Bottom
                        expandBottom(targetExpandDown * (stateTimer / expandDuration));
                        expandTop(targetExpandUp * (stateTimer / expandDuration));
                        break;
                    case 1: // Top
                        expandTop(targetExpandUp * (stateTimer / expandDuration));
                        expandBottom(targetExpandDown * (stateTimer / expandDuration));
                        break;
                    case 2: // Right
                        expandRight(targetExpandRight * (stateTimer / expandDuration));
                        expandLeft(targetExpandLeft * (stateTimer / expandDuration));
                        break;
                    case 3: // Left
                        expandLeft(targetExpandLeft * (stateTimer / expandDuration));
                        expandRight(targetExpandRight * (stateTimer / expandDuration));
                        break;
                }
            }
        break;

        case BeamState::ACTIVE:
            if (stateTimer >= activeDuration) {
                state = BeamState::FADING;
                cerr << "Duration in active state: " << stateTimer << endl;
                stateTimer = 0.0f;
            }
        break;
        
        case BeamState::FADING:
            if (stateTimer >= fadeDuration) {
                isActive = false;
            }
        break;
        }

}