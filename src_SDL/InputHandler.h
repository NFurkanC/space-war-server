#pragma once
#include "Packets.h"

struct InputStateStruct {
    bool w, a, s, d;
    int mouseX, mouseY;
    bool attack;
    bool boost;
};

class InputHandler {
public:
    InputStateStruct state = {false, false, false, false, 0, 0, false, false};
    float zoomDelta = 0.0f;
    bool quit = false;
    bool windowResized = false;
    int newWidth = 0;
    int newHeight = 0;

    void processEvents();
    ActionPacket createActionPacket(int windowWidth, int windowHeight);
};
