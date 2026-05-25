#include "AsteroidField.h"
#include <cstdlib>

static const std::vector<std::vector<Vector2Packet>> TEMPLATES = {
    { {-2.0, -0.8}, {-0.5, -1.4}, {1.2, -1.0}, {2.0, 0.2}, {1.0, 1.1}, {-0.8, 1.3}, {-2.0, 0.5} },
    { {-1.0, -2.0}, {0.8, -1.5}, {1.8, -0.3}, {1.2, 1.4}, {-0.3, 1.8}, {-1.5, 0.6}, {-1.8, -0.8} },
    { {0.0, -1.5}, {1.0, -0.5}, {1.3, 0.8}, {0.0, 1.5}, {-1.3, 0.8}, {-1.0, -0.5} },
    { {-2.2, -0.5}, {-1.0, -1.5}, {1.0, -1.5}, {2.2, -0.3}, {2.0, 0.8}, {0.5, 1.5}, {-1.2, 1.2}, {-2.2, 0.3} }
};

std::vector<LinePacket> templateToLines(const std::vector<Vector2Packet>& temp, float scale, float ox, float oy) {
    std::vector<LinePacket> lines;
    int n = temp.size();
    for (int i = 0; i < n; i++) {
        const auto& curr = temp[i];
        const auto& next = temp[(i + 1) % n];
        lines.push_back({
            {ox + curr.x * scale, oy + curr.y * scale},
            {ox + next.x * scale, oy + next.y * scale}
        });
    }
    return lines;
}

AsteroidField::AsteroidField(int count, float spread, float viewRadius) : viewRadius(viewRadius) {
    for (int i = 0; i < count; i++) {
        float z = static_cast<float>(rand()) / RAND_MAX;
        instances.push_back({
            (static_cast<float>(rand()) / RAND_MAX - 0.5f) * spread,
            (static_cast<float>(rand()) / RAND_MAX - 0.5f) * spread,
            z,
            0.4f + z * 1.6f,
            rand() % static_cast<int>(TEMPLATES.size())
        });
    }
}

std::vector<std::vector<LinePacket>> AsteroidField::getStaticAsteroids(float camX, float camY) const {
    std::vector<std::vector<LinePacket>> result;
    for (const auto& a : instances) {
        float parallaxFactor = 0.05f + a.z * 0.75f;
        float worldX = a.wx + camX * (1.0f - parallaxFactor);
        float worldY = a.wy + camY * (1.0f - parallaxFactor);

        float dx = worldX - camX;
        float dy = worldY - camY;
        if (dx * dx + dy * dy > viewRadius * viewRadius) continue;

        result.push_back(templateToLines(TEMPLATES[a.templateIndex], a.scale, worldX, worldY));
    }
    return result;
}
