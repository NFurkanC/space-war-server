#include "InputHandler.h"
#include <SDL.h>
#include <cmath>

void InputHandler::processEvents() {
    zoomDelta = 0.0f;
    windowResized = false;
    
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) quit = true;
        else if (e.type == SDL_WINDOWEVENT) {
            if (e.window.event == SDL_WINDOWEVENT_RESIZED) {
                windowResized = true;
                newWidth = e.window.data1;
                newHeight = e.window.data2;
            }
        } else if (e.type == SDL_KEYDOWN || e.type == SDL_KEYUP) {
            bool isDown = (e.type == SDL_KEYDOWN);
            switch (e.key.keysym.sym) {
                case SDLK_w: state.w = isDown; break;
                case SDLK_s: state.s = isDown; break;
                case SDLK_a: state.a = isDown; break;
                case SDLK_d: state.d = isDown; break;
                case SDLK_SPACE: if (isDown) state.attack = true; break;
                case SDLK_LSHIFT:
                case SDLK_RSHIFT: if (isDown) state.boost = true; break;
            }
        } else if (e.type == SDL_MOUSEMOTION) {
            state.mouseX = e.motion.x;
            state.mouseY = e.motion.y;
        } else if (e.type == SDL_MOUSEBUTTONDOWN) {
            if (e.button.button == SDL_BUTTON_LEFT) {
                state.attack = true;
            }
        } else if (e.type == SDL_MOUSEWHEEL) {
            zoomDelta = (float)e.wheel.y; // Simplified
        }
    }
}

ActionPacket InputHandler::createActionPacket(int windowWidth, int windowHeight) {
    float dx = 0, dy = 0;
    if (state.w) dy -= 1;
    if (state.s) dy += 1;
    if (state.a) dx -= 1;
    if (state.d) dx += 1;

    float len = std::sqrt(dx*dx + dy*dy);
    if (len > 0) {
        dx /= len;
        dy /= len;
    }

    ActionPacket packet;
    if (dx != 0 || dy != 0) {
        packet.hasForce = true;
        packet.force = {dx, dy};
    }

    float centerX = windowWidth / 2.0f;
    float centerY = windowHeight / 2.0f;
    packet.hasRotation = true;
    packet.rotation.targetAngle = std::atan2(state.mouseY - centerY, state.mouseX - centerX);
    
    packet.attack = state.attack;
    packet.boost = state.boost;

    // Reset non-continuous
    state.attack = false;
    state.boost = false;

    return packet;
}
