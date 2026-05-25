#pragma once
#include "Packets.h"

struct WorldPos {
    float x;
    float y;
};

struct ScreenPos {
    float x;
    float y;
};

class Camera {
public:
    int screenWidth;
    int screenHeight;
    float x;
    float y;
    struct { float x, y; } offset;
    float scale;
    float targetScale;
    float minScale;
    float maxScale;

    Camera(int screenWidth, int screenHeight);

    void zoom(float deltaY);
    void update(float dtMs, bool hasTarget, WorldPos targetWorldPos, bool isLocalPlayer = false);
    ScreenPos worldToScreen(float worldX, float worldY) const;
    void followPlayer(float dt, bool hasTarget, WorldPos pos, bool isLocalPlayer = false);

private:
    bool hasTargetObj = false;
};
