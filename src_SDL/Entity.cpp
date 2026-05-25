#include "Entity.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

GameEntity::GameEntity(const PlayerState& state) {
    id = state.id;
    position = state.position;
    velocity = state.velocity;
    rotation = state.rotation;
    polygon = state.polygon;

    visualPosition = position;
    visualRotation = rotation;

    updateBasePolygon(state.polygon, state.position, state.rotation);
}

void GameEntity::updateBasePolygon(const std::vector<LinePacket>& worldPolygon, Vector2Packet center, float rot) {
    float cosA = std::cos(-rot);
    float sinA = std::sin(-rot);

    basePolygon.clear();
    for (const auto& line : worldPolygon) {
        auto transformPoint = [&](const Vector2Packet& p) -> Vector2Packet {
            float dx = p.x - center.x;
            float dy = p.y - center.y;
            return {
                dx * cosA - dy * sinA,
                dx * sinA + dy * cosA
            };
        };
        basePolygon.push_back({transformPoint(line.a), transformPoint(line.b)});
    }
}

void GameEntity::applyServerUpdate(const PlayerState& state) {
    position = state.position;
    velocity = state.velocity;
    rotation = state.rotation;

    if (state.polygon.size() != basePolygon.size()) {
        updateBasePolygon(state.polygon, state.position, state.rotation);
    }
    polygon = state.polygon;
}

void GameEntity::update(float dtMs, bool hasInput, float dx, float dy) {
    float dt = dtMs / 1000.0f;

    if (hasInput) {
        velocity.x += dx * acceleration * dt;
        velocity.y += dy * acceleration * dt;
    }

    float frictionMultiplier = std::pow(1.0f - frictionRate, dt);
    velocity.x *= frictionMultiplier;
    velocity.y *= frictionMultiplier;

    position.x += velocity.x * dt;
    position.y += velocity.y * dt;

    float lerpFactor = 1.0f - std::exp(-lerpSpeed * dt);
    
    visualPosition.x += (position.x - visualPosition.x) * lerpFactor;
    visualPosition.y += (position.y - visualPosition.y) * lerpFactor;

    float rotationDiff = rotation - visualRotation;
    while (rotationDiff > (float)M_PI) rotationDiff -= (float)M_PI * 2.0f;
    while (rotationDiff < -(float)M_PI) rotationDiff += (float)M_PI * 2.0f;
    visualRotation += rotationDiff * lerpFactor;
}

std::vector<LinePacket> GameEntity::getVisualPolygon() const {
    if (basePolygon.empty()) return {};

    float cosA = std::cos(visualRotation);
    float sinA = std::sin(visualRotation);
    std::vector<LinePacket> result;

    for (const auto& line : basePolygon) {
        auto transformPoint = [&](const Vector2Packet& p) -> Vector2Packet {
            float rx = p.x * cosA - p.y * sinA;
            float ry = p.x * sinA + p.y * cosA;
            return {
                rx + visualPosition.x,
                ry + visualPosition.y
            };
        };
        result.push_back({transformPoint(line.a), transformPoint(line.b)});
    }
    return result;
}

Vector2Packet GameEntity::getVisualPosition() const {
    return visualPosition;
}

float GameEntity::getVisualRotation() const {
    return visualRotation;
}
