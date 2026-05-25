#pragma once
#include "Packets.h"
#include <vector>

class GameEntity {
public:
    uint64_t id;
    
    // Server State
    Vector2Packet position;
    Vector2Packet velocity;
    float rotation;
    std::vector<LinePacket> polygon;

    GameEntity(const PlayerState& state);
    
    void applyServerUpdate(const PlayerState& state);
    void update(float dtMs, bool hasInput = false, float dx = 0.0f, float dy = 0.0f);
    
    std::vector<LinePacket> getVisualPolygon() const;
    Vector2Packet getVisualPosition() const;
    float getVisualRotation() const;

private:
    // Visual State
    Vector2Packet visualPosition;
    float visualRotation;
    std::vector<LinePacket> basePolygon;
    
    float lerpSpeed = 7.0f;
    float frictionRate = 0.4f;
    float acceleration = 100.0f;

    void updateBasePolygon(const std::vector<LinePacket>& worldPolygon, Vector2Packet center, float rot);
};
