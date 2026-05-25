#pragma once
#include <string>
#include <vector>
#include <cstdint>

enum class PacketType : uint8_t {
    UNKNOWN = 0,
    VECTOR2 = 1,
    LINE = 2,
    BREAKABLE_LINE = 3,
    POLYGON = 4,
    NOTIFICATION = 5,
    STRING = 6,
    ARRAY = 7,
    UNIFORM_ARRAY = 8,
    ACTION = 9,
    WORLD_STATE = 10,
    PLAYER_STATE = 11,
    ENTITY_STATE = 12
};

struct Vector2Packet {
    float x;
    float y;
};

struct LinePacket {
    Vector2Packet a;
    Vector2Packet b;
};

struct BreakableLinePacket : public LinePacket {
    int health;
};

struct PolygonPacket {
    std::vector<LinePacket> lines;
};

struct NotificationPacket {
    std::string sender;
    std::string message;
    std::string datetime;
};

struct PlayerState {
    uint64_t id;
    Vector2Packet position;
    Vector2Packet velocity;
    float rotation;
    std::vector<LinePacket> polygon;
};

using EntityState = PlayerState;

struct WorldState {
    PlayerState localPlayer;
    std::vector<EntityState> nearObjects;
    bool hasLocalPlayer = false;
};

enum ActionType {
    FORCE = 1,
    ROTATION = 2,
    ATTACK = 4,
    BOOST = 8
};

struct ForceActionData {
    float dx;
    float dy;
};

struct RotationActionData {
    float targetAngle;
};

struct ActionPacket {
    bool hasForce = false;
    ForceActionData force = {0,0};
    
    bool hasRotation = false;
    RotationActionData rotation = {0};
    
    bool attack = false;
    bool boost = false;
};
