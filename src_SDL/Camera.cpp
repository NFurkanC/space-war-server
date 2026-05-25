#include "Camera.h"
#include <algorithm>
#include <cmath>

Camera::Camera(int screenW, int screenH) 
    : screenWidth(screenW), screenHeight(screenH),
      x(0), y(0), offset{0, 0}, scale(10), targetScale(10),
      minScale(5), maxScale(40), hasTargetObj(false) {}

void Camera::zoom(float deltaY) {
    const float zoomSensitivity = 0.1f;
    targetScale *= (deltaY > 0) ? (1.0f - zoomSensitivity) : (1.0f + zoomSensitivity);
    targetScale = std::max(minScale, std::min(maxScale, targetScale));
}

void Camera::update(float dtMs, bool hasTarget, WorldPos targetWorldPos, bool isLocalPlayer) {
    float dt = dtMs / 1000.0f;
    float trackK = isLocalPlayer ? 5.0f : 15.0f; 
    float zoomK = 5.0f;

    float zoomFactor = 1.0f - std::exp(-zoomK * dt);
    float trackFactor = 1.0f - std::exp(-trackK * dt);

    scale += (targetScale - scale) * zoomFactor;

    if (hasTarget) {
        if (!hasTargetObj) {
            x = targetWorldPos.x;
            y = targetWorldPos.y;
            hasTargetObj = true;
        }
        x += (targetWorldPos.x - x) * trackFactor;
        y += (targetWorldPos.y - y) * trackFactor;
    }

    offset.x = (screenWidth / 2.0f) - (x * scale);
    offset.y = (screenHeight / 2.0f) - (y * scale);
}

ScreenPos Camera::worldToScreen(float worldX, float worldY) const {
    return {
        (worldX * scale) + offset.x,
        (worldY * scale) + offset.y
    };
}

void Camera::followPlayer(float dt, bool hasTarget, WorldPos targetPos, bool isLocalPlayer) {
    update(dt, hasTarget, targetPos, isLocalPlayer);
}
