#include "../include/window.h"
#include "../include/utils.h"
#include <iostream>

using namespace std;

Window::Window(int x, int y, int width, int height, float shrinkSpeed, string title, bool isOnTop, bool isBorderless, bool isTransparent) : 
                x(x), y(y), // top left
                width(width), height(height), shrinkSpeed(shrinkSpeed),
                title(title), 
                isOnTop(isOnTop), isBorderless(isBorderless), isTransparent(isTransparent) {
    Uint32 flags = SDL_WINDOW_SHOWN;
    Uint32 rendererFlags = SDL_RENDERER_ACCELERATED;  
    if (isTransparent) isBorderless = true;      
    if (isOnTop) flags |= SDL_WINDOW_ALWAYS_ON_TOP;
    if (isBorderless) flags |= SDL_WINDOW_BORDERLESS;

    window = SDL_CreateWindow(title.c_str(), x, y, width, height, flags);
    if (window == nullptr) {
        cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << '\n';
        return;
    }

    if (isTransparent) {
        SDL_SetWindowOpacity(window, 1.0f);
        SDL_SetWindowBordered(window, SDL_FALSE);
    }

    renderer = SDL_CreateRenderer(window, -1, rendererFlags);
    if (renderer == nullptr) {
        cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << '\n';
        SDL_DestroyWindow(window);
        window = nullptr;
        return;
    }

    pair<int, int> resolution = getResolution();
    screenWidth = resolution.first;
    screenHeight = resolution.second;
}

Window::~Window() {
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }
    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
}

void Window::setTitle(const string& newTitle) {
    title = newTitle;
    SDL_SetWindowTitle(window, title.c_str());
}

void Window::checkScreenEdgesCollision() {
    // Reset screen edges
    screenEdges[0] = screenEdges[1] = screenEdges[2] = screenEdges[3] = false;
    
    // Check and handle collisions with screen edges
    if (y <= 0) {
        screenEdges[0] = true;
        y = 0;
    }
    if (y + height >= screenHeight) {
        screenEdges[1] = true;
        y = screenHeight - height;
    }
    if (x <= 0) {
        screenEdges[2] = true;
        x = 0;
    }
    if (x + width >= screenWidth) {
        screenEdges[3] = true;
        x = screenWidth - width;
    }
    
    // Update window position
    SDL_SetWindowPosition(window, x, y);
}


// setpos is called every frame, so this might as well be update() (not anymore)
void Window::setPos(int x, int y) {
    this->x = x;
    this->y = y;
    SDL_SetWindowPosition(window, x, y);
}

void Window::setSize(int width, int height) {
    this->width = width;
    this->height = height;
    SDL_SetWindowSize(window, width, height);
}

// helper clamp (std::clamp doesn't work for some reason)
int clamp(int value, int min, int max) {
    return (value < min) ? min : (value > max) ? max : value;
}

// helper to get current window dimensions
void Window::getDimensions(int& x, int& y, int& width, int& height) {
    x = this->x;
    y = this->y;
    width = this->width;
    height = this->height;
}

// resize-by
void Window::createResizeRequest(int top, int bottom, int left, int right, int initialSpeed, float duration) {
    float now = SDL_GetTicks() / 1000.0f;

    ResizeRequest request(top, bottom, left, right, duration, now, 0.0f, initialSpeed);

    resizeRequests.push_back(request);
}

// reminder that request with 0 duration = flat resize
// otherwise apply ease out cubic
// positive -> expand, negative -> shrink
void Window::applyResizeRequest(ResizeRequest& request, float deltaTime) {
    // clamping
    if (x - request.leftTarget < 0) request.leftTarget = x;
    if (y - request.topTarget < 0) request.topTarget = y;
    if (x + width + request.rightTarget > screenWidth) request.rightTarget = screenWidth - (x + width);
    if (y + height + request.bottomTarget > screenHeight) request.bottomTarget = screenHeight - (y + height);

    if (request.animationDuration <= 0.0f) { // immediate resize
        int newX = std::max(x - request.leftTarget, 0);
        int newY = std::max(y - request.topTarget, 0);
        int newWidth = std::max(width + request.leftTarget + request.rightTarget, MIN_SIZE);
        int newHeight = std::max(height + request.topTarget + request.bottomTarget, MIN_SIZE);

        // apply
        setPos(newX, newY);
        setSize(newWidth, newHeight);

        request.active = false; // Mark the request as inactive (completed).
    }

    else { // animated resize
           // there's some predefined easing functions declared below, plug whatever is needed in
           // maybe i'll work on it more in the future

        request.elapsed = (SDL_GetTicks() / 1000.0f) - request.animationStartTime;
        float progress = std::min(request.elapsed / request.animationDuration, 1.0f);

        /*
        if (progress >= 1.0f) { // apply final state
            int newX = x - request.leftTarget;
            int newY = y - request.topTarget;
            int newWidth = std::min(std::max(width + request.leftTarget + request.rightTarget, MIN_SIZE), screenWidth);
            int newHeight = std::min(std::max(height + request.topTarget + request.bottomTarget, MIN_SIZE), screenHeight);

            setSize(newWidth, newHeight);
            setPos(newX, newY);

            cerr << "New pos: " << newX << ", " << newY << ", new size: " << newWidth << ", " << newHeight << endl;

            progress = 1.0f;
            request.active = false; // mark as inactive
            return; // exit
        }
            */
        
        // definitely gonna have some undershoot, but that's better than the alternative (potential overshoot)
        if (progress >= 1.0f) {
            progress = 1.0f;
            request.active = false;
            return;
        }

        else { // apply easing for the current frame
            // calculate progress difference
            float easingNow = easeOutQuad(progress);
            float easingLast = easeOutQuad(request.lastProgress);

            // speed delta
            float speedFactor = 1.0f - progress;
            float speedDelta = request.initialSpeed * deltaTime * speedFactor;

            float leftSpeedBoost = 0.0f; float rightSpeedBoost = 0.0f; float topSpeedBoost = 0.0f; float bottomSpeedBoost = 0.0f;

            // apply speed boost (and direction) based on target
            if (request.leftTarget > 0) leftSpeedBoost = speedDelta; if (request.leftTarget < 0) leftSpeedBoost = -speedDelta;
            if (request.rightTarget > 0) rightSpeedBoost = speedDelta; if (request.rightTarget < 0) rightSpeedBoost = -speedDelta;
            if (request.topTarget > 0) topSpeedBoost = speedDelta; if (request.topTarget < 0) topSpeedBoost = -speedDelta;
            if (request.bottomTarget > 0) bottomSpeedBoost = speedDelta; if (request.bottomTarget < 0) bottomSpeedBoost = -speedDelta;

            // calculate deltas
            int leftDelta = std::max(0,static_cast<int>(request.leftTarget * (easingNow - easingLast) + leftSpeedBoost));
            int rightDelta = static_cast<int>(request.rightTarget * (easingNow - easingLast) + rightSpeedBoost);
            int topDelta = std::max(0,static_cast<int>(request.topTarget * (easingNow - easingLast) + topSpeedBoost));
            int bottomDelta = static_cast<int>(request.bottomTarget * (easingNow - easingLast) + bottomSpeedBoost);

            // adjust new position based on deltas
            int newX = x - leftDelta;
            int newY = y - topDelta;
            int newWidth = std::min(std::max(width + leftDelta + rightDelta, MIN_SIZE), screenWidth);
            int newHeight = std::min(std::max(height + topDelta + bottomDelta, MIN_SIZE), screenHeight);

            setPos(newX, newY);
            setSize(newWidth, newHeight);

            request.lastProgress = progress;
        }
    }
}

void Window::update(float deltaTime) {
    naturalShrinking(deltaTime); // might as well
    auto it = resizeRequests.begin();
    while (it != resizeRequests.end()) {
        if (it->active) {
            applyResizeRequest(*it, deltaTime);
            if (!it->active) {
                it = resizeRequests.erase(it); // remove completed request
            } else {
                it++; // move to next request
            }
        } else {
            it = resizeRequests.erase(it); // remove inactive request
        }
    }
}

SDL_Rect Window::getBounds() {
    SDL_Rect bounds = {x, y, width, height};
    return bounds;
}

float Window::easeOutQuad(float progress) {
    // clamp
    progress = std::min(1.0f, std::max(0.0f, progress));

    // ease out quad function
    // f(t) = 1 - (1 - t)^2
    return 1.0f - std::pow(1.0f - progress, 2);
}

float Window::easeOutCubic(float progress) {
    // clamp
    progress = std::min(1.0f, std::max(0.0f, progress));

    // ease out cubic function
    // f(t) = 1 - (1 - t)^3
    return 1.0f - std::pow(1.0f - progress, 3);
}

float Window::easeOutQuart(float progress) {
    // clamp
    progress = std::min(1.0f, std::max(0.0f, progress));

    // ease out quart function
    // f(t) = 1 - (1 - t)^4
    return 1.0f - std::pow(1.0f - progress, 4);
}

float Window::easeOutExpo(float progress) {
    // clamp
    progress = std::min(1.0f, std::max(0.0f, progress));

    // ease out expo function
    // f(t) = 1 - 2^(-10 * t)
    return 1.0f - std::pow(2.0f, -10.0f * progress);
}

void Window::naturalShrinking(const float &deltaTime) {
    int centerX = x + width / 2;
    int centerY = y + height / 2;
    float effectiveSpeed = shrinkSpeed * deltaTime;

    int topDistance = centerY - y;
    int bottomDistance = (y + height) - centerY;
    int leftDistance = centerX - x;
    int rightDistance = (x + width) - centerX;

    float f_topDelta = topDistance * effectiveSpeed;
    float f_bottomDelta = bottomDistance * effectiveSpeed;
    float f_leftDelta = leftDistance * effectiveSpeed;
    float f_rightDelta = rightDistance * effectiveSpeed;

    int topDelta = static_cast<int>(f_topDelta);
    int bottomDelta = static_cast<int>(f_bottomDelta);
    int leftDelta = static_cast<int>(f_leftDelta);
    int rightDelta = static_cast<int>(f_rightDelta);


    if (height > MIN_SIZE) {
        if (topDelta == 0 && f_topDelta > 0.0f) {
            topDelta = 1;
        }
        if (bottomDelta == 0 && f_bottomDelta > 0.0f) {
            bottomDelta = 1;
        }

    } else { // height <= MIN_SIZE
        topDelta = 0;
        bottomDelta = 0;
    }

    if (width > MIN_SIZE) {
        if (leftDelta == 0 && f_leftDelta > 0.0f) {
            leftDelta = 1;
        }
        if (rightDelta == 0 && f_rightDelta > 0.0f) {
            rightDelta = 1;
        }
    } else { // width <= MIN_SIZE
        leftDelta = 0;
        rightDelta = 0;
    }

    // create request if there's any actual shrinking to do
    if (topDelta > 0 || bottomDelta > 0 || leftDelta > 0 || rightDelta > 0) {
        createResizeRequest(-topDelta, -bottomDelta, -leftDelta, -rightDelta, 0, 0.0f);
    }
}
