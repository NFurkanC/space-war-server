#pragma once
#include <vector>
#include "Packets.h"

struct AsteroidInstance {
    float wx;
    float wy;
    float z;
    float scale;
    int templateIndex;
};

class AsteroidField {
public:
    AsteroidField(int count = 250, float spread = 500.0f, float viewRadius = 80.0f);
    std::vector<std::vector<LinePacket>> getStaticAsteroids(float camX, float camY) const;

private:
    std::vector<AsteroidInstance> instances;
    float viewRadius;
};
