#include <SDL.h>
#include <iostream>
#include <map>
#include <memory>
#include <unordered_set>
#include "Config.h"
#include "Renderer.h"
#include "Camera.h"
#include "InputHandler.h"
#include "AsteroidField.h"
#include "Network.h"
#include "PacketHandler.h"
#include "Entity.h"

int main(int argc, char* argv[]) {
    int initialWidth = 1280;
    int initialHeight = 720;
    
    Renderer renderer("Space War", initialWidth, initialHeight);
    Camera camera(initialWidth, initialHeight);
    InputHandler input;
    AsteroidField asteroidField;
    NetworkManager network(CONFIG::WS_URL);
    PacketHandler packetHandler;

    network.onData([&](const std::vector<uint8_t>& data) {
        packetHandler.handleIncomingData(data);
    });

    network.connect();

    std::unique_ptr<GameEntity> localPlayer;
    std::map<uint64_t, std::unique_ptr<GameEntity>> otherEntities;

    packetHandler.onPacket(PacketType::WORLD_STATE, [&](const void* data) {
        if (!data) return;
        const WorldState* ws = static_cast<const WorldState*>(data);

        if (ws->hasLocalPlayer) {
            if (!localPlayer || localPlayer->id != ws->localPlayer.id) {
                localPlayer = std::make_unique<GameEntity>(ws->localPlayer);
            } else {
                localPlayer->applyServerUpdate(ws->localPlayer);
            }
        }

        std::unordered_set<uint64_t> currentIds;
        for (const auto& obj : ws->nearObjects) {
            currentIds.insert(obj.id);
            auto it = otherEntities.find(obj.id);
            if (it == otherEntities.end()) {
                otherEntities[obj.id] = std::make_unique<GameEntity>(obj);
            } else {
                it->second->applyServerUpdate(obj);
            }
        }

        for (auto it = otherEntities.begin(); it != otherEntities.end(); ) {
            if (currentIds.find(it->first) == currentIds.end()) {
                it = otherEntities.erase(it);
            } else {
                ++it;
            }
        }
    });

    uint32_t lastTime = SDL_GetTicks();
    uint32_t lastActionSendTime = lastTime;

    while (!input.quit) {
        uint32_t currentTime = SDL_GetTicks();
        float dtMs = (float)(currentTime - lastTime);
        lastTime = currentTime;

        network.poll();
        input.processEvents();

        if (input.windowResized) {
            renderer.resize(input.newWidth, input.newHeight);
            camera.screenWidth = input.newWidth;
            camera.screenHeight = input.newHeight;
        }
        if (input.zoomDelta != 0.0f) {
            camera.zoom(-input.zoomDelta); 
        }

        if (currentTime - lastActionSendTime >= CONFIG::SERVER_TICK_INTERVAL) {
            ActionPacket action = input.createActionPacket(renderer.windowWidth, renderer.windowHeight);
            std::vector<uint8_t> buffer = packetHandler.encodeAction(action);
            network.send(buffer);
            lastActionSendTime = currentTime;
        }

        // Update
        float dx = 0, dy = 0;
        if (localPlayer) {
            if (input.state.w) dy -= 1;
            if (input.state.s) dy += 1;
            if (input.state.a) dx -= 1;
            if (input.state.d) dx += 1;
            float len = std::sqrt(dx*dx + dy*dy);
            if (len > 0) { dx /= len; dy /= len; }
            
            localPlayer->update(dtMs, true, dx, dy);
        }

        for (auto& pair : otherEntities) {
            pair.second->update(dtMs);
        }

        // Camera
        if (localPlayer) {
            WorldPos wp{localPlayer->getVisualPosition().x, localPlayer->getVisualPosition().y};
            camera.followPlayer(dtMs, true, wp, true);
        } else {
            camera.update(dtMs, false, {0,0}, true);
        }

        // Draw
        std::vector<std::vector<LinePacket>> allPolygons = asteroidField.getStaticAsteroids(camera.x, camera.y);
        
        if (localPlayer) {
            allPolygons.push_back(localPlayer->getVisualPolygon());
        }
        for (const auto& pair : otherEntities) {
            allPolygons.push_back(pair.second->getVisualPolygon());
        }

        renderer.drawScene(allPolygons, camera);
        
        SDL_Delay(1); // Small delay to avoid 100% CPU
    }

    return 0;
}
