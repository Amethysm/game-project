#pragma once
#include <map>
#include <string>

// Global variables for window settings and display
extern int windowWidth;
extern int windowHeight;
extern int screenFPS;
extern int screenTicksPerFrame;
extern int displayWidth;
extern int displayHeight;

// Keyboard state
extern std::map<std::string, bool> keyState;

// Mouse state
extern std::map<std::string, bool> mouseState;
extern int mouseX, mouseY;