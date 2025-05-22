#include <SDL.h>
#include <SDL_image.h>
#include <iostream>
#include <random>
#include <memory>
#include "include/globals.h"
#include "include/utils.h"
#include "include/window.h"
#include "include/player.h"
#include "include/game_manager.h"

int main(int argc, char* argv[]) {
    // initialize SDL + windows
    Window* mainWindow = init();
    if (!mainWindow) return -1;

    // sdl2 doesn't support getting global mouse state
    // my little hack around this is to create a (nearly) full-screen transparent overlay window
    // that will be on top of the main window
    // alternatively i can use some specific windows api but that's more difficult
    // the downside is that it blocks everything, but i guess that could be a safeguard for potential misclick
    Window* overlay = createOverlayWindow();
    
    GameManager gameManager(mainWindow);

    // create & register Player
    SDL_Rect bounds = mainWindow->getBounds();
    int startX = bounds.x + bounds.w/2;
    int startY = bounds.y + bounds.h/2;
    float playerSpeed = 500.0f;
    int   playerRadius = 25;
    
    // allocate Player on the heap and hand to manager
    auto player = std::make_unique<Player>(
        Vector2D(startX, startY),
        playerRadius,
        playerSpeed,
        mainWindow
    );
    Player* playerPtr = player.get();
    playerPtr->setGameManager(&gameManager);
    gameManager.addObject(std::move(player)); 

    bool quit = false;
    SDL_Event event;
    Uint32 lastTick = SDL_GetTicks();
    auto [screenW, screenH] = getResolution();
    
    while (!quit) {
        // ——— frame timing ———
        Uint32 now = SDL_GetTicks();
        float  dt = (now - lastTick) / 1000.0f;
        lastTick = now;

        // ——— handle input ———
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            
            // pause/resume
            } else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
                if (gameManager.getGameState() == GameState::RUNNING) {
                    gameManager.pauseGame();
                } else if (gameManager.getGameState() == GameState::PAUSED) {
                    gameManager.resumeGame();
                }
            // restart
            } else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_r) {
                if (gameManager.getGameState() == GameState::PAUSED || 
                    gameManager.getGameState() == GameState::GAME_OVER) {
                    gameManager.restartGame();
                    
                    // Find the active player after restart
                    playerPtr = nullptr;
                    for (auto& obj : gameManager.getGameObjects()) {
                        if (obj->getType() == GameObject::ObjectType::Player) {
                            playerPtr = static_cast<Player*>(obj.get());
                            break;
                        }
                    }
                }
            // ragequit
            } else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_p) {
                quit = true;
            } else {
                checkMovement(event, keyState);
                
                // Only process player input if game is running and player pointer is valid
                if (gameManager.getGameState() == GameState::RUNNING && playerPtr && playerPtr->getActive()) {
                    gameManager.handleInput(event, playerPtr);
                }
            }
        }
        
        // ——— update & collision ———
        if (!gameManager.isPaused()) {
            mainWindow->update(dt);
        }
        gameManager.update(dt);
        
        // ——— render ———
        SDL_SetRenderDrawColor(mainWindow->renderer, 0, 0, 0, 255);
        SDL_RenderClear(mainWindow->renderer);
        
        // Draw game objects
        gameManager.draw(mainWindow->renderer);
        
        if (gameManager.isPaused()) {
            // draw an overlay to indicate pause (and maybe some text)
            SDL_SetRenderDrawBlendMode(mainWindow->renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(mainWindow->renderer, 0, 0, 0, 128);
            SDL_Rect fullscreen = {0, 0, mainWindow->width, mainWindow->height};
            SDL_RenderFillRect(mainWindow->renderer, &fullscreen);
            
            SDL_Color pausedTextColor = {255, 255, 255, 255};
            int fontSize = 12;
            renderText(mainWindow->renderer, "PAUSED", 20, 20, fontSize, pausedTextColor);
            renderText(mainWindow->renderer, "Press R to restart", 20, 40, fontSize, pausedTextColor);
            renderText(mainWindow->renderer, "Press P to ragequit", 20, 60, fontSize, pausedTextColor);
        }
        
        SDL_RenderPresent(mainWindow->renderer);

        // ——— cap to ~60 FPS ———
        waitForFrame(screenFPS, lastTick);
    }

    cleanup({{"main",mainWindow},{"overlay",overlay}});
    return 0;
}