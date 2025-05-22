#include "../include/utils.h"
#include "../include/globals.h"
#include <iostream>

using namespace std;

std::map<std::string, SDL_Texture*> TextureManager::textures;

SDL_Texture* TextureManager::getTexture(const std::string& path, SDL_Renderer* renderer) {
    if (textures.find(path) != textures.end()) {
        return textures[path];
    }
    
    SDL_Texture* newTexture = loadTexture(path, renderer);
    if (newTexture) {
        textures[path] = newTexture;
        SDL_SetTextureBlendMode(newTexture, SDL_BLENDMODE_BLEND);
        std::cerr << "Texture loaded and cached: " << path << endl;
    }
    return newTexture;
}

void TextureManager::cleanup() {
    for (auto& pair : textures) {
        if (pair.second) {
            SDL_DestroyTexture(pair.second);
        }
    }
    textures.clear();
}

SDL_Texture* loadTexture(const string& path, SDL_Renderer* renderer) {
    SDL_Texture* texture = IMG_LoadTexture(renderer, path.c_str());
    if (texture == nullptr) {
        cerr << "Failed to load texture: " << IMG_GetError() << '\n';
    }
    return texture;
}

string fetchResourcePath(const string& filename) {
    char* basePath = SDL_GetBasePath();
    if (basePath) {
        string path = string(basePath) + "..\\assets\\" + filename;
        SDL_free(basePath);
        return path;
    }
    else {
        cerr << "Failed to get base path: " << SDL_GetError() << '\n';
        return "..\\assets\\" + filename; // fallback
    }
}

std::pair<int, int> getResolution() {
    SDL_DisplayMode d;
    if (SDL_GetCurrentDisplayMode(0, &d) != 0) {
        std::cerr << "SDL_GetCurrentDisplayMode failed: " << SDL_GetError() << '\n';
        return {windowWidth, windowHeight};
    }
    return {d.w, d.h - 50}; // 45 is the height of the taskbar
                            // todo get the taskbar height dynamically
                            // otherwise will have to change this (maybe just delete it entirely on linux, testing on windows for now)
}

Window* init() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << '\n';
        return nullptr;
    }

    if (!IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG)) {
        std::cerr << "SDL_image could not initialize! SDL_image Error: " << IMG_GetError() << '\n';
        SDL_Quit();
        return nullptr;
    }
    
    // Initialize SDL_ttf
    if (TTF_Init() == -1) {
        std::cerr << "SDL_ttf could not initialize! SDL_ttf Error: " << TTF_GetError() << '\n';
        IMG_Quit();
        SDL_Quit();
        return nullptr;
    }

    pair<int,int> resolution = getResolution();
    int x = (resolution.first - windowWidth) / 2;
    int y = (resolution.second - windowHeight) / 2;

    Window* window = new Window(x, y, windowWidth, windowHeight, 0.25f, "SDL2 Player Movement", false, false, false);

    if (window->window == nullptr) {
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        return nullptr;
    }

    displayWidth = resolution.first;
    displayHeight = resolution.second;

    return window;
}

Window* createOverlayWindow() {
    std::pair<int, int> resolution = getResolution();

    Window* overlayWindow = new Window(
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_UNDEFINED, // ??
        resolution.first, resolution.second, // window size
        0.0f,
        "Overlay",
        false, // 
        true, // borderless
        true // transparent
    );

    SDL_SetWindowOpacity(overlayWindow->window, 0.01f); // nearly invisible
    //SDL_SetWindowAlwaysOnTop(overlayWindow->window, SDL_TRUE); // always on top

    return overlayWindow;
}

void destroy(Window* window) {
    if (window) {
        delete window;
        window = nullptr;
    }
}

void cleanup(map<string, Window*> windows) {
    for (auto& pair : windows) {
        if (pair.second) {
            destroy(pair.second);
            pair.second = nullptr;
        }
    }
    TextureManager::cleanup();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

void waitForFrame(int fps, Uint32 frameStart) {
    if (fps <= 0) return;

    Uint32 frameTime = SDL_GetTicks() - frameStart;
    int ticks = 1000 / fps;
    if (frameTime < ticks) {
        SDL_Delay(ticks - frameTime);
    }
}

void checkMovement(SDL_Event event, std::map<std::string, bool>& keyState) {
    if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {
            case SDLK_UP: keyState["up"] = true; break;
            case SDLK_DOWN: keyState["down"] = true; break;
            case SDLK_LEFT: keyState["left"] = true; break;
            case SDLK_RIGHT: keyState["right"] = true; break;

            // WASD
            case SDLK_w: keyState["up"] = true; break;
            case SDLK_s: keyState["down"] = true; break;
            case SDLK_a: keyState["left"] = true; break;
            case SDLK_d: keyState["right"] = true; break;
            
            case SDLK_j: keyState["j"] = true; break;
        }
    } else if (event.type == SDL_KEYUP) {
        switch (event.key.keysym.sym) {
            case SDLK_UP: keyState["up"] = false; break;
            case SDLK_DOWN: keyState["down"] = false; break;
            case SDLK_LEFT: keyState["left"] = false; break;
            case SDLK_RIGHT: keyState["right"] = false; break;

            // WASD
            case SDLK_w: keyState["up"] = false; break;
            case SDLK_s: keyState["down"] = false; break;
            case SDLK_a: keyState["left"] = false; break;
            case SDLK_d: keyState["right"] = false; break;
            
            case SDLK_j: keyState["j"] = false; break;
        }
    }
}

void checkMouseMovement(SDL_Event event, std::map<std::string, bool>& mouseState) {
    if (event.type == SDL_MOUSEMOTION) {
        mouseX = event.motion.x;
        mouseY = event.motion.y;
    } else if (event.type == SDL_MOUSEBUTTONDOWN) {
        switch (event.button.button) {
            case SDL_BUTTON_LEFT: mouseState["left"] = true; break;
            case SDL_BUTTON_MIDDLE: mouseState["middle"] = true; break;
            case SDL_BUTTON_RIGHT: mouseState["right"] = true; break;
        }
    }
    else if (event.type == SDL_MOUSEBUTTONUP) {
        switch (event.button.button) {
            case SDL_BUTTON_LEFT: mouseState["left"] = false; break;
            case SDL_BUTTON_MIDDLE: mouseState["middle"] = false; break;
            case SDL_BUTTON_RIGHT: mouseState["right"] = false; break;
        }
    }
}

void renderText(SDL_Renderer* renderer, const std::string& text, int x, int y, int fontSize, SDL_Color color) {
    static TTF_Font* font = nullptr;
    static int cachedFontSize = 0;
    static bool ttfInitialized = false;
    
    if (!ttfInitialized) {
        if (TTF_Init() == -1) {
            std::cerr << "SDL_ttf could not initialize! SDL_ttf Error: " << TTF_GetError() << std::endl;
            return;
        }
        ttfInitialized = true;
    }
    
    if (!font || cachedFontSize != fontSize) {
        if (font) {
            TTF_CloseFont(font);
            font = nullptr;
        }
        
        // this is gonna die hard on non-windows machine
        font = TTF_OpenFont("C:/Windows/Fonts/Arial.ttf", fontSize);
        
        if (!font) {
            std::cerr << "Failed to load font! SDL_ttf Error: " << TTF_GetError() << std::endl;
            return;
        }
        
        cachedFontSize = fontSize;
    }
    
    SDL_Surface* textSurface = TTF_RenderText_Blended(font, text.c_str(), color);
    if (!textSurface) {
        std::cerr << "Unable to render text surface! SDL_ttf Error: " << TTF_GetError() << std::endl;
        return;
    }
    
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    if (!textTexture) {
        std::cerr << "Unable to create texture from rendered text! SDL Error: " << SDL_GetError() << std::endl;
        SDL_FreeSurface(textSurface);
        return;
    }
    
    int textWidth = textSurface->w;
    int textHeight = textSurface->h;
    
    SDL_FreeSurface(textSurface);
    
    SDL_Rect renderQuad = {x, y, textWidth, textHeight};
    SDL_RenderCopy(renderer, textTexture, NULL, &renderQuad);
    
    SDL_DestroyTexture(textTexture);
}

map<string, bool> getKeyState() {
    return keyState;
}

map<string, bool> getMouseState() {
    return mouseState;
}