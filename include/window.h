#pragma once

#include <SDL.h>
#include <string>
#include <vector>

using namespace std;

struct ResizeRequest {
    int topTarget, bottomTarget, leftTarget, rightTarget;
    bool active;
    float animationDuration; // seconds
    float animationStartTime;
    float elapsed;
    int initialSpeed; // pixel per second, for animation
    float lastProgress; // [0..1] for frame by frame delta
    // Constructor for convenience
    ResizeRequest(int top, int bottom, int left, int right, 
                    float duration, 
                    float start, 
                    float elapsed,
                    int initSpeed = 0)
        : topTarget(top), bottomTarget(bottom), leftTarget(left), rightTarget(right),
          active(true), 
          animationDuration(duration), 
          animationStartTime(start), 
          elapsed(elapsed),
          initialSpeed(initSpeed),
          lastProgress(0.0f)
        {}
};

// Window class to handle window properties and rendering
class Window {
private:
    vector<ResizeRequest> resizeRequests;
    
    void applyResize(int top, int bottom, int left, int right);

    // bunch of predefined easing functions
    float easeOutCubic(float progress);
    float easeOutQuart(float progress);
    float easeOutExpo(float progress);
    float easeOutQuad(float progress);

    void clampTargets(ResizeRequest& req);
    void applyImmediateResize(ResizeRequest& req);
    bool applyAnimatedResize(ResizeRequest& req, float deltaTime);
    std::tuple<int,int,int,int> computeDeltas(ResizeRequest& req, float progress, float deltaTime);

public:
    enum class CollisionEdge {
        TOP,
        BOTTOM,
        LEFT,
        RIGHT
    };

    int width, height, x, y; // x,y is top left coords
    float shrinkSpeed; 
    std::string title;
    bool isOnTop = false, isBorderless = false, isTransparent = false;
    SDL_Window* window;
    CollisionEdge collisionEdge;
    SDL_Renderer* renderer;
    int screenWidth, screenHeight;
    bool screenEdges[4] = {false, false, false, false}; // top bot left right
    const int MIN_SIZE = 150;

    // Constructor
    Window(int x, int y, int width, int height, float shrinkSpeed, std::string title, 
           bool isOnTop = false, bool isBorderless = false, bool isTransparent = false);

    // Destructor
    ~Window();

    // Window management methods
    void setTitle(const std::string& newTitle);
    void setPos(int x, int y);
    void setSize(int width, int height);

    void createResizeRequest(int top, int bottom, int left, int right, int initialSpeed, float duration);
    void applyResizeRequest(ResizeRequest& request, float deltaTime);
    void clearResizeRequests() { resizeRequests.clear(); }

    void update(float deltaTime);
    void naturalShrinking(const float &deltaTime);
    SDL_Rect getBounds();
    
    // Helper to get current dimensions
    void getDimensions(int& x, int& y, int& width, int& height);
    void checkScreenEdgesCollision();
};
