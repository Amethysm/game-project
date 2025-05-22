#include "../include/globals.h"
#include "../include/utils.h"
#include <map>
#include <string>

// general convention
// objects: middle coords
// window: top left coords

int windowWidth = 800;
int windowHeight = 800;
int screenFPS = 60;
int screenTicksPerFrame = 1000 / screenFPS;
int displayWidth = 0;
int displayHeight = 0;

std::map<std::string, bool> keyState = {
    {"up", false},
    {"down", false},
    {"left", false},
    {"right", false},
    {"j", false}  // test key
};

// bool mouseState[3] = {false, false, false}; // Left, Middle, Right
map<string, bool> mouseState = {
    {"left", false},
    {"middle", false},
    {"right", false}
};

int mouseX, mouseY;