#pragma once

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <cmath>
#include "window.h"
#include "globals.h"

using namespace std;
 
SDL_Texture* loadTexture(const std::string& path, SDL_Renderer* renderer);
std::string fetchResourcePath(const std::string& filename);

// Simple text rendering utility
void renderText(SDL_Renderer* renderer, const std::string& text, int x, int y, int fontSize = 16, SDL_Color color = {255, 255, 255, 255});

std::pair<int, int> getResolution();
Window* init();
Window* createOverlayWindow();
void destroy(Window* window);
void cleanup(map<string, Window*> windows);
void checkMovement(SDL_Event event, std::map<std::string, bool>& keyState);
void checkMouseMovement(SDL_Event event, std::map<std::string, bool>& mouseState);
void waitForFrame(int fps, Uint32 frameStart);
map<string, bool> getKeyState();
map<string, bool> getMouseState();

struct Vector2D {
    float x, y;
    Vector2D(float x = 0, float y = 0) : x(x), y(y) {}
    Vector2D operator+(const Vector2D& other) const {return Vector2D(x + other.x, y + other.y);}
    Vector2D operator-(const Vector2D& other) const {return Vector2D(x - other.x, y - other.y);}
    Vector2D operator*(float a) const {return Vector2D(x * a, y * a);}
    Vector2D operator/(float a) const {return Vector2D(x / a, y / a);}
    Vector2D& operator*=(float a) {x *= a; y *= a; return *this;}
    Vector2D& operator/=(float a) {x /= a; y /= a; return *this;}
    Vector2D& operator+=(const Vector2D& other) {x += other.x; y += other.y; return *this;}
    Vector2D& operator-=(const Vector2D& other) {x -= other.x; y -= other.y; return *this;}
    Vector2D& operator=(const Vector2D& other) {
        if (this == &other) return *this;
        x = other.x;
        y = other.y;
        return *this;
    }
    bool operator==(const Vector2D& other) const {return x == other.x && y == other.y;}
    float lengthSquared() const {return x * x + y * y;} // for performance
    float magnitude() const {return sqrt(x * x + y * y);}
    float distance(const Vector2D& other) const {return (*this - other).magnitude();}
    Vector2D normalize() const {
        float mag = magnitude();
        if (mag == 0) return Vector2D(0, 0);
        return Vector2D(x / mag, y / mag);
    }
    float dot(const Vector2D& other) const {return x * other.x + y * other.y;}
};

struct Point2D {
    float x, y;
    Point2D(float x = 0, float y = 0) : x(x), y(y) {}
};
// i think i've used this like.... once

class TextureManager {
    public:
        static SDL_Texture* getTexture(const std::string& path, SDL_Renderer* renderer);
        static void cleanup();
    private:
        static std::map<std::string, SDL_Texture*> textures;
    };